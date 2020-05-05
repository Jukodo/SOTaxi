#include "CenDLL.h"
#include "CTService.h"
#include "CTCommunication.h"

int _tmain(int argc, LPTSTR argv[]) {
	#pragma region Initialization
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

			if(_tolower(retryOpt[0]) == 'n' || retryOpt[0] == 'n'){
				return false;
			}
		}
	} while(flagOfflineCentral);
	_tprintf(TEXT("%sConnection to central successful!"), Utils_NewLine());
	#pragma endregion

	_tprintf(TEXT("%sPress any key to start the application..."), Utils_NewLine());
	getchar();

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
	getchar();
	#pragma endregion

	#pragma region Closing
	Setup_CloseAllHandles(&app);
	#pragma endregion

	_tprintf(TEXT("%sPress any key to close the application..."), Utils_NewLine());
	getchar();

	return 0;
}