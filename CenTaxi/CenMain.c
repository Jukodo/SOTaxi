#include "CenDLL.h"
#include "CenService.h"

int _tmain(int argc, LPTSTR argv[]) {
	#pragma region OneInstanceLock
	HANDLE hMutex_OneInstanceLock = CreateMutex(NULL, TRUE, NAME_MUTEX_ONEINSTANCE_CEN);
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

	_tprintf(TEXT("%sThe central is ready to be used! Waiting for commands..."), Utils_NewLine());
	#pragma endregion

	#pragma region FakePassengerList
	Passenger newPass;
	for(int i = 0; i < app.maxPassengers; i++){
		newPass.empty = false;
		_stprintf_s(newPass.Id, _countof(newPass.Id), TEXT("Pass%d"), i);
		newPass.object.coordX = (float) i;
		newPass.object.coordY = (float) i;

		Service_NewPassenger(&app, newPass);
	}
	#pragma endregion

	getchar();
	return 0;
}