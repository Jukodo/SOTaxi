#include "CenDLL.h"
#include "Service.h"

int _tmain(int argc, LPTSTR argv[]) {
	_tprintf(TEXT("%d"), MAIN_A());
	_tprintf(TEXT("%d"), TAXI_A());

	Application app;

	if(!Setup_Application(&app)){
		wprintf(TEXT("Central is offline..."));
		return false;
	}

	_tprintf(TEXT("%d"), ((Taxi*) app.shmHandles.lpTestMem)->IdPassenger);

	getchar();

	Setup_CloseAllHandles(&app);

	return 0;
}