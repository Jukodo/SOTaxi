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
			TCHAR sCoordinates_X[3] = TEXT("0");
			TCHAR sCoordinates_Y[3] = TEXT("48");
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

				if(Utils_IsValid_Coordinates(sCoordinates_X)){
					_tprintf(TEXT("%sX coordinate: %s"), Utils_NewSubLine(), sCoordinates_X);
				} else{
					_tprintf(TEXT("%sPlease enter you starting X coordinates (XX)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), sCoordinates_X, _countof(sCoordinates_X));
					Utils_CleanStdin();
					if(!Utils_IsValid_Coordinates(sCoordinates_X)){
						_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
						continue;
					}
				}
				if(Utils_IsValid_Coordinates(sCoordinates_Y)){
					_tprintf(TEXT("%sY coordinate: %s"), Utils_NewSubLine(), sCoordinates_Y);
				} else{
					_tprintf(TEXT("%sPlease enter you starting Y coordinates (YY)"), Utils_NewSubLine());
					_tprintf(TEXT("%s-> "), Utils_NewSubLine());
					_tscanf_s(TEXT(" %[^\n]"), sCoordinates_Y, _countof(sCoordinates_Y));
					Utils_CleanStdin();
					if(!Utils_IsValid_Coordinates(sCoordinates_Y)){
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
					ZeroMemory(sCoordinates_X, sizeof(sCoordinates_X));
					ZeroMemory(sCoordinates_Y, sizeof(sCoordinates_Y));
					continue;
				}

				_tprintf(TEXT("%sTrying to login. Please wait..."), Utils_NewSubLine());
				if(Command_LoginPassenger(&app, sId, sCoordinates_X, sCoordinates_Y)){
					_tprintf(TEXT("%sLogin successful! Transport of the passenger is being taken care of!"), Utils_NewSubLine());
					break;
				}

				ZeroMemory(opt, 2);
				_tprintf(TEXT("%sLogin failed... Do you wish to try again? (Y/n)"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), opt, _countof(opt));
				Utils_CleanStdin();

				if(_tolower(opt[0]) == 'n' || opt[0] == 'n')
					break;

			} while(true);
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