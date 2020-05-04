#include "CenDLL.h"
#include "CenService.h"

int _tmain(int argc, LPTSTR argv[]) {
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
	Application app;

	if(!Setup_Application(&app, maxTaxi, maxPassenger)){
		_tprintf(TEXT("Error trying to set up central..."));
		getchar();
		return false;
	}

	getchar();
	return 0;
}