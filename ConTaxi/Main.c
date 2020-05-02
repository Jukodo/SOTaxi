#include "CenDLL.h"
#include "Service.h"
#include "Communication.h"

int _tmain(int argc, LPTSTR argv[]) {
	#pragma region Initialization
	Application app;
	if(!Setup_Application(&app)){
		_tprintf(TEXT("Central is offline..."));
		return false;
	} else{
		_tprintf(TEXT("Connection to central successful!"));
	}
	#pragma endregion

	_tprintf(TEXT("%sPress any key to start the application..."), Utils_NewLine());
	getchar();

	#pragma region Login
	//wprintf(TEXT("%d"), ((Taxi*) app.shmHandles.lpTestMem)->IdPassenger);
	TCHAR sLicensePlate[STRING_SMALL];
	TCHAR sCoordinates_X[STRING_SMALL];
	TCHAR sCoordinates_Y[STRING_SMALL];
	TCHAR opt[2];
	do{
		ZeroMemory(opt, 2);
		_tprintf(TEXT("%sLogin"), Utils_NewLine());

		if(isValid_LicensePlate(sLicensePlate)){
			_tprintf(TEXT("%sLicense plate: %s"),  Utils_NewSubLine(), sLicensePlate);
		} else{
			_tprintf(TEXT("%sPlease enter you license plate (XX-XX-XX)"), Utils_NewSubLine());
			_tprintf(TEXT("%s-> "), Utils_NewSubLine());
			_tscanf_s(TEXT(" %[^\n]"), sLicensePlate, _countof(sLicensePlate));
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
			if(!isValid_Coordinates(sCoordinates_Y)){
				_tprintf(TEXT("%sCoordinates don't follow input rules..."), Utils_NewSubLine());
				continue;
			}
		}

		_tprintf(TEXT("%sDo you wish to re-enter the information? (Y/n)"), Utils_NewSubLine());
		_tprintf(TEXT("%s-> "), Utils_NewSubLine());
		_tscanf_s(TEXT(" %[^\n]"), opt, _countof(opt));

		if(_tolower(opt[0]) == 'y' || opt[0] == 'y'){
			ZeroMemory(sLicensePlate, sizeof(sLicensePlate));
			ZeroMemory(sCoordinates_X, sizeof(sCoordinates_X));
			ZeroMemory(sCoordinates_Y, sizeof(sCoordinates_Y));
			continue;
		} else
			break;
	} while(_tolower(opt[0]) == 'n' || opt[0] == 'n');

	_tprintf(TEXT("\n%d"), &app.syncHandles.mutex_SendRequest);
	SendRequest request;
	LoginRequest loginRequest;
	_tcscpy_s(loginRequest.licensePlate, STRING_SMALL, sLicensePlate);
	loginRequest.coordX = _wtoi(sCoordinates_X);
	loginRequest.coordY = _wtoi(sCoordinates_Y);
	request.app = &app;
	request.loginRequest = loginRequest;
	request.requestType = LOGIN;

	app.threadHandles.hSendRequests = CreateThread(
		NULL,
		0,
		Thread_SendRequests,				//Function
		(LPVOID) &request,					//Param
		0,									//Creation Flag
		&app.threadHandles.dwIdSendRequests //idThread
	);
	_tprintf(TEXT("%sTrying to log in. Please wait..."), Utils_NewSubLine());
	WaitForSingleObject(app.threadHandles.hSendRequests, INFINITE);
	getchar();

	#pragma endregion

	#pragma region Closing
	//Setup_CloseAllHandles(&app);
	#pragma endregion

	_tprintf(TEXT("%sPress any key to close the application..."), Utils_NewLine());
	getchar();

	return 0;
}