#pragma once
#include "CenDLL.h"
#include "CPService.h"

int _tmain(int argc, LPTSTR argv[]){
	#pragma region OneInstanceLock
	HANDLE hMutex_OneInstanceLock = CreateMutex(NULL, TRUE, NAME_MUTEX_OneInstance_CP);
	if(GetLastError() == ERROR_ALREADY_EXISTS){
		_tprintf(TEXT("%sError!%sThere is already and instance of this application running...%sOnly one instance is allowed!"), Utils_NewLine(), Utils_NewSubLine(), Utils_NewSubLine());
		_gettchar();
		return 0;
	}
	Utils_DLL_Register(NAME_MUTEX_OneInstance_CP, DLL_TYPE_MUTEX);
	#pragma endregion

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

	#pragma region Commands
	_tprintf(TEXT("%sThe application is ready to accept commands...%s\"%s\" to display all of the commands!"), Utils_NewLine(), Utils_NewSubLine(), CMD_HELP);

	TCHAR sCommand[STRING_MEDIUM];
	TCHAR sArgument[STRING_SMALL];
	do{
		ZeroMemory(sCommand, STRING_MEDIUM);
		ZeroMemory(sArgument, STRING_SMALL);
		_tprintf(TEXT("%s[ConPass] Waiting for command:"), Utils_NewLine());
		_tprintf(TEXT("%s-> "), Utils_NewSubLine());
		_tscanf_s(TEXT(" %[^\n]"), sCommand, _countof(sCommand));
		Utils_CleanStdin();

		if(!app.keepRunning)
			break;

		switch(Service_UseCommand(&app, sCommand)){
		case PC_HELP:
			_tprintf(TEXT("%s%s:\t\t\tShows a list of available commands"), Utils_NewSubLine(), CMD_HELP);
			_tprintf(TEXT("%s%s:\t\t\tLogin a new passenger"), Utils_NewSubLine(), CMD_LOGIN);
			_tprintf(TEXT("%s%s:\tList all passengers logged in"), Utils_NewSubLine(), CMD_LIST_PASSENGERS);
			_tprintf(TEXT("%s%s:\t\tCloses the application"), Utils_NewSubLine(), CMD_CLOSEAPP);
			break;
		case PC_LOGIN:
		{
			TCHAR sId[9];
			swprintf(sId, 9, TEXT("%c%c%c%c%c%c%d%d"),
				Utils_GetRandomLetter(),
				Utils_GetRandomLetter(),
				Utils_GetRandomLetter(),
				Utils_GetRandomLetter(),
				Utils_GetRandomLetter(),
				Utils_GetRandomLetter(),
				rand()%10,
				rand()%10);
			TCHAR xAt[3];
			swprintf(xAt, 3, TEXT("%d%d"),
				rand()%50,
				rand()%50);
			TCHAR yAt[3];
			swprintf(yAt, 3, TEXT("%d%d"),
				rand()%50,
				rand()%50);
			TCHAR xDestiny[3];
			swprintf(xDestiny, 3, TEXT("%d%d"),
				rand()%50,
				rand()%50);
			TCHAR yDestiny[3];
			swprintf(yDestiny, 3, TEXT("%d%d"),
				rand()%50,
				rand()%50);
			TCHAR opt[2];
			bool flagLoginFailed = true;
			do{
				ZeroMemory(opt, 2);
				_tprintf(TEXT("%sLogin new Passenger"), Utils_NewLine());

				if(Utils_IsValid_Id(sId)){
					_tprintf(TEXT("%sLicense plate: %s"), Utils_NewSubLine(), sId);
				} else{
					_tprintf(TEXT("%sPlease enter new passenger ID (must have 8 characters or numbers)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), sId, _countof(sId));
					Utils_CleanStdin();
					if(!Utils_IsValid_Id(sId)){
						_tprintf(TEXT("%sLicense plate doesn't follow input rules..."), Utils_NewSubLine());
						continue;
					}
				}

				if(Utils_IsValid_Coordinates(xAt)){
					_tprintf(TEXT("%sX coordinate: %s"), Utils_NewSubLine(), xAt);
				} else{
					_tprintf(TEXT("%sPlease enter your current X coordinates (XX)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), xAt, _countof(xAt));
					Utils_CleanStdin();
					if(!Utils_IsValid_Coordinates(xAt)){
						_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
						continue;
					}
				}
				if(Utils_IsValid_Coordinates(yAt)){
					_tprintf(TEXT("%sY coordinate: %s"), Utils_NewSubLine(), yAt);
				} else{
					_tprintf(TEXT("%sPlease enter your current Y coordinates (YY)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), yAt, _countof(yAt));
					Utils_CleanStdin();
					if(!Utils_IsValid_Coordinates(yAt)){
						_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
						continue;
					}
				}
				if(Utils_IsValid_Coordinates(xDestiny)){
					_tprintf(TEXT("%sX coordinate: %s"), Utils_NewSubLine(), xDestiny);
				} else{
					_tprintf(TEXT("%sPlease enter your destiny X coordinates (XX)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), xDestiny, _countof(xDestiny));
					Utils_CleanStdin();
					if(!Utils_IsValid_Coordinates(xDestiny)){
						_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
						continue;
					}
				}
				if(Utils_IsValid_Coordinates(yDestiny)){
					_tprintf(TEXT("%sY coordinate: %s"), Utils_NewSubLine(), yDestiny);
				} else{
					_tprintf(TEXT("%sPlease enter your destiny Y coordinates (YY)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), yDestiny, _countof(yDestiny));
					Utils_CleanStdin();
					if(!Utils_IsValid_Coordinates(yDestiny)){
						_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
						continue;
					}
				}

				_tprintf(TEXT("%sDo you wish to re-enter the information? (Y/n)"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), opt, _countof(opt));
				Utils_CleanStdin();

				if(_tolower(opt[0]) == 'y' || opt[0] == 'y'){
					ZeroMemory(sId, sizeof(sId));
					ZeroMemory(xAt, sizeof(xAt));
					ZeroMemory(yAt, sizeof(yAt));
					ZeroMemory(xDestiny, sizeof(xDestiny));
					ZeroMemory(yDestiny, sizeof(yDestiny));
					continue;
				}

				_tprintf(TEXT("%sTrying to login. Please wait..."), Utils_NewSubLine());
				if(Command_LoginPassenger(&app, sId, xAt, yAt, xDestiny, yDestiny))
					break;

				ZeroMemory(opt, 2);
				_tprintf(TEXT("%sInformation is invalid... Do you wish to try again? (Y/n)"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), opt, _countof(opt));
				Utils_CleanStdin();

				if(_tolower(opt[0]) == 'n' || opt[0] == 'n')
					break;

			} while(true);
		}
			break;
		case PC_LIST_PASSENGERS:
			_tprintf(TEXT("%sLogged Passengers List:"), Utils_NewSubLine());
			for(int i = 0; i < app.maxPass; i++){
				if(!app.passengerList[i].passengerInfo.empty)
					_tprintf(TEXT("%s[%02d] - %s starting from (%.2lf, %.2lf) with intent of going to (%.2lf, %.2lf)"),
						Utils_NewSubLine(),
						i+1,
						app.passengerList[i].passengerInfo.Id,
						app.passengerList[i].passengerInfo.object.xyPosition.x,
						app.passengerList[i].passengerInfo.object.xyPosition.y,
						app.passengerList[i].xyDestination.x,
						app.passengerList[i].xyDestination.y);
				else
					_tprintf(TEXT("%s[%02d] - Empty slot"), Utils_NewSubLine(), i+1);
			}
			break;
		case PC_UNDEFINED:
			_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
			continue;
		}

	} while(app.keepRunning);
	#pragma endregion

	#pragma region Closing
	_tprintf(TEXT("%sThe application is closing! Press enter to continue..."), Utils_NewLine());
	_gettchar();
	Setup_CloseAllHandles(&app);
	#pragma endregion

	return 0;
}