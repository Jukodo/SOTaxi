#pragma once
#include "CenDLL.h"
#include "CTDLL.h"
#include "CTService.h"

int _tmain(int argc, LPTSTR argv[]) {
	#pragma region ApplicationSetup
	Application app;
	TCHAR retryOpt[2];
	bool flagOfflineCentral = true;
	do{
		ZeroMemory(retryOpt, 2);
		flagOfflineCentral = !Setup_Application(&app);
		if(flagOfflineCentral){
			_tprintf(TEXT("%sCentral is offline... Do you want to try again? (Y/n)"), Utils_NewLine());
			_tprintf(TEXT("%s-> "), Utils_NewSubLine());
			_tscanf_s(TEXT(" %[^\n]"), retryOpt, _countof(retryOpt));
			Utils_CleanStdin();

			if(_tolower(retryOpt[0]) == 'n' || retryOpt[0] == 'n'){
				return false;
			}
		}
	} while(flagOfflineCentral);
	_tprintf(TEXT("%sConnection to central successful!"), Utils_NewLine());
	#pragma endregion

	#pragma region Login
	//wprintf(TEXT("%d"), ((Taxi*) app.shmHandles.lpTestMem)->IdPassenger);
	TCHAR sLicensePlate[9] = TEXT("aa-aa-aa");
	TCHAR sCoordinates_X[3] = TEXT("11");
	TCHAR sCoordinates_Y[3] = TEXT("22");
	TCHAR opt[2];
	bool flagLoginFailed = true;
	do{
		ZeroMemory(opt, 2);
		_tprintf(TEXT("%sLogin"), Utils_NewLine());

		if(isValid_LicensePlate(sLicensePlate)){
			_tprintf(TEXT("%sLicense plate: %s"),  Utils_NewSubLine(), sLicensePlate);
		} else{
			_tprintf(TEXT("%sPlease enter you license plate (XX-XX-XX)"), Utils_NewSubLine());
			_tprintf(TEXT("%s-> "), Utils_NewSubLine());
			_tscanf_s(TEXT(" %[^\n]"), sLicensePlate, _countof(sLicensePlate));
			Utils_CleanStdin();
			if(!isValid_LicensePlate(sLicensePlate)){
				_tprintf(TEXT("%sLicense plate doesn't follow input rules..."), Utils_NewSubLine());
				continue;
			}
		}

		if(isValid_Coordinates(sCoordinates_X)){
			_tprintf(TEXT("%sX coordinate: %s"), Utils_NewSubLine(), sCoordinates_X);
		} else{
			_tprintf(TEXT("%sPlease enter you starting X coordinates (XX)"), Utils_NewSubLine());
			_tprintf(TEXT("%s-> "), Utils_NewSubLine());
			_tscanf_s(TEXT(" %[^\n]"), sCoordinates_X, _countof(sCoordinates_X));
			Utils_CleanStdin();
			if(!isValid_Coordinates(sCoordinates_X)){
				_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
				continue;
			}
		}
		if(isValid_Coordinates(sCoordinates_Y)){
			_tprintf(TEXT("%sY coordinate: %s"), Utils_NewSubLine(), sCoordinates_Y);
		} else{
			_tprintf(TEXT("%sPlease enter you starting Y coordinates (YY)"), Utils_NewSubLine());
			_tprintf(TEXT("%s-> "), Utils_NewSubLine());
			_tscanf_s(TEXT(" %[^\n]"), sCoordinates_Y, _countof(sCoordinates_Y));
			Utils_CleanStdin();
			if(!isValid_Coordinates(sCoordinates_Y)){
				_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
				continue;
			}
		}

		_tprintf(TEXT("%sDo you wish to re-enter the information? (Y/n)"), Utils_NewSubLine());
		_tprintf(TEXT("%s-> "), Utils_NewSubLine());
		_tscanf_s(TEXT(" %[^\n]"), opt, _countof(opt));
		Utils_CleanStdin();

		if(_tolower(opt[0]) == 'y' || opt[0] == 'y'){
			ZeroMemory(sLicensePlate, sizeof(sLicensePlate));
			ZeroMemory(sCoordinates_X, sizeof(sCoordinates_X));
			ZeroMemory(sCoordinates_Y, sizeof(sCoordinates_Y));
			continue;
		} else {
			Service_Login(&app, sLicensePlate, sCoordinates_X, sCoordinates_Y);
			_tprintf(TEXT("%sTrying to log in. Please wait..."), Utils_NewSubLine());
			WaitForSingleObject(app.threadHandles.hLARequests, INFINITE);
		}
		flagLoginFailed = !isLoggedIn(&app);
		if(flagLoginFailed)
			_tprintf(TEXT("%sLog in failed... Please try again!"), Utils_NewSubLine());
	} while(flagLoginFailed);

	_tprintf(TEXT("%sYou are now logged in... Welcome!"), Utils_NewSubLine()); 
	ResumeThread(app.threadHandles.hNotificationReceiver_NP);
	#pragma endregion

	#pragma region Commands
	_tprintf(TEXT("%sThe application is ready to accept commands...%s\"/help\" to display all of the commands!"), Utils_NewLine(), Utils_NewSubLine());

	TCHAR sCommand[STRING_MEDIUM];
	TCHAR sArgument[STRING_SMALL];
	TaxiCommands command;
	bool flagReadyToLeave = false;
	do{
		ZeroMemory(sCommand, STRING_MEDIUM);
		ZeroMemory(sArgument, STRING_SMALL);
		_tprintf(TEXT("%sLogged in as %s%sWaiting for command:"), Utils_NewLine(), app.loggedInTaxi.LicensePlate, Utils_NewSubLine());
		_tprintf(TEXT("%s-> "), Utils_NewSubLine());
		_tscanf_s(TEXT(" %[^\n]"), sCommand, _countof(sCommand));
		Utils_CleanStdin();

		command = Service_UseCommand(&app, sCommand);
		if(command == TC_HELP){
			_tprintf(TEXT("%s/help:\t\tShows a list of available commands"), Utils_NewSubLine());
			_tprintf(TEXT("%s/speedUp:\tSpeeds the taxi up by 0.5cells per second"), Utils_NewSubLine());
			_tprintf(TEXT("%s/speedDown:\tSpeeds the taxi down by 0.5cells per second"), Utils_NewSubLine());
			_tprintf(TEXT("%s/autoRespOn:\tTurn on automatic response to new passenger"), Utils_NewSubLine());
			_tprintf(TEXT("%s/autoRespOff:\tTurn off automatic response to new passenger"), Utils_NewSubLine());
			_tprintf(TEXT("%s/defineCDN:\tDefine new CDN value"), Utils_NewSubLine());
			_tprintf(TEXT("%s/requestPass:\tSend a request to be assigned to a respective passenger"), Utils_NewSubLine());
			_tprintf(TEXT("%s/closeApp:\tCloses the application"), Utils_NewSubLine());
		}

		switch(command){
			case TC_UNDEFINED:
				_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
				continue;
			case TC_CLOSEAPP:
				flagReadyToLeave = true;
				break;
			case TC_DEFINE_CDN:
				_tprintf(TEXT("%sInsert new CDN value:"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), sArgument, _countof(sArgument));
				Utils_CleanStdin();

				if(!Utils_StringIsNumber(sArgument)){
					_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
				}
				if(!Service_DefineCDN(&app, sArgument)){
					_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
				}
				break;
			case TC_REQUEST_PASS:
				_tprintf(TEXT("%sInsert passenger ID:"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), sArgument, _countof(sArgument));
				Utils_CleanStdin();
				Service_RequestPass(&app, sArgument);
				break;
		}

	} while(!flagReadyToLeave);
	#pragma endregion

	#pragma region Closing
	_tprintf(TEXT("%sThe application is closing! Press any enter to continue..."), Utils_NewLine());
	getchar();
	Setup_CloseAllHandles(&app);
	#pragma endregion

	_tprintf(TEXT("%sPress any key to close the application..."), Utils_NewLine());
	getchar();

	return 0;
}