#include "CenDLL.h"
#include "CenService.h"

int _tmain(int argc, LPTSTR argv[]) {
	#pragma region OneInstanceLock
	HANDLE hMutex_OneInstanceLock = CreateMutex(NULL, TRUE, NAME_MUTEX_OneInstance_CEN);
	if(GetLastError() == ERROR_ALREADY_EXISTS){
		_tprintf(TEXT("%sError!%sThere is already and instance of this application running...%sOnly one instance is allowed!"), Utils_NewLine(), Utils_NewSubLine(), Utils_NewSubLine());
		getchar();
		return 0;
	}
	#pragma endregion

	#pragma region ArgumentsSetup
	//argv[0] - Executable file path (default)
	//argv[1] - Max Taxis
	//argv[2] - Max Passengers
	if(argc != 3){
		_tprintf(TEXT("%sArguments error! Please be sure to include [MaxTaxi] and [MaxPassenger], respectively!"), Utils_NewLine());
		getchar();
		return false;
	}
	if(!Utils_StringIsNumber(argv[1])){
		_tprintf(TEXT("%sArguments error! [MaxTaxi] is not a number!"), Utils_NewLine());
		getchar();
		return false;
	}
	if(!Utils_StringIsNumber(argv[2])){
		_tprintf(TEXT("%sArguments error! [MaxPassenger] is not a number!"), Utils_NewLine());
		getchar();
		return false;
	}
	int maxTaxi = _wtoi(argv[1]);
	int maxPassenger = _wtoi(argv[2]);
	#pragma endregion

	#pragma region ApplicationSetup
	Application app;

	if(!Setup_Application(&app, maxTaxi, maxPassenger)){
		_tprintf(TEXT("%sError trying to set up central..."), Utils_NewLine());
		getchar();
		return false;
	}
	#pragma endregion

	#pragma region Commands
	_tprintf(TEXT("%sThe application is ready to accept commands...%s\"%s\" to display all of the commands!"), Utils_NewLine(), Utils_NewSubLine(), CMD_HELP);

	TCHAR sCommand[STRING_MEDIUM];
	TCHAR sArgument[STRING_SMALL];
	CentralCommands command;
	bool flagReadyToLeave = false;
	do{
		ZeroMemory(sCommand, STRING_MEDIUM);
		ZeroMemory(sArgument, STRING_SMALL);
		_tprintf(TEXT("%s[Central] Logged in as an admin!%sWaiting for command:"), Utils_NewLine(), Utils_NewSubLine());
		_tprintf(TEXT("%s-> "), Utils_NewSubLine());
		_tscanf_s(TEXT(" %[^\n]"), sCommand, _countof(sCommand));
		Utils_CleanStdin();

		command = Service_UseCommand(&app, sCommand);
		if(command == CC_HELP){
			_tprintf(TEXT("%s%s:\t\tShows a list of available commands"), Utils_NewSubLine(), CMD_HELP);
			_tprintf(TEXT("%s%s:\tShows list of logged in taxis"), Utils_NewSubLine(), CMD_LIST_TAXIS);
			_tprintf(TEXT("%s%s:\tChange timeout of taxi assignment to a transport request"), Utils_NewSubLine(), CMD_SET_TIMEOUT);
			_tprintf(TEXT("%s%s:\tAllow taxi logins"), Utils_NewSubLine(), CMD_TAXI_LOGIN_ON);
			_tprintf(TEXT("%s%s:\tDeny taxi logins"), Utils_NewSubLine(), CMD_TAXI_LOGIN_OFF);
			_tprintf(TEXT("%s%s:\tKick a specific taxi"), Utils_NewSubLine(), CMD_KICK_TAXI);
			_tprintf(TEXT("%s%s:\tSimulate a new transport request notification"), Utils_NewSubLine(), CMD_SIMULATE_NTR);
			_tprintf(TEXT("%s%s:\tCloses the application"), Utils_NewSubLine(), CMD_CLOSEAPP);
		}

		switch(command){
			case CC_LIST_TAXIS:
				_tprintf(TEXT("%sLogged Taxis List:"), Utils_NewSubLine());
				for(int i = 0; i < app.maxTaxis; i++){
					if(!app.taxiList[i].empty)
						_tprintf(TEXT("%s[%02d] - %s located at (%.2lf, %.2lf)"), 
							Utils_NewSubLine(), 
							i+1,
							app.taxiList[i].LicensePlate,
							app.taxiList[i].object.coordX,
							app.taxiList[i].object.coordY);
					else
						_tprintf(TEXT("%s[%02d] - Empty slot"), Utils_NewSubLine(), i+1);
				}
				break;
			case CC_SET_TIMEOUT:
				_tprintf(TEXT("%sInsert value of assignment timeout:"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), sArgument, _countof(sArgument));
				Utils_CleanStdin();

				if(!Utils_StringIsNumber(sArgument) || !Command_SetAssignmentTimeout(&app, sArgument)){
					_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
				}
				break;
			case CC_KICK_TAXI:
				_tprintf(TEXT("%sInsert license plate of taxi to kick:"), Utils_NewSubLine());
				_tprintf(TEXT("%s-> "), Utils_NewSubLine());
				_tscanf_s(TEXT(" %[^\n]"), sArgument, _countof(sArgument));
				Utils_CleanStdin();

				if(!Utils_StringIsNumber(sArgument) || !Service_KickTaxi(&app, sArgument)){
					_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
				}
				break;
			case CC_CLOSEAPP:
				flagReadyToLeave = true;
				break;
			case CC_UNDEFINED:
				_tprintf(TEXT("%sCommand doesn't follow input rules or doesn't exist..."), Utils_NewSubLine());
				continue;
		}

	} while(!flagReadyToLeave);
	#pragma endregion

	getchar();
	return 0;
}