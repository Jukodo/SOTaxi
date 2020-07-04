#pragma once
#include "MIService.h"

bool Setup_Application(Application* app, HWND hWnd){
	ZeroMemory(app, sizeof(Application));
	srand((unsigned int) time(NULL));

	if(!Setup_OpenSyncHandles(&app->syncHandles)){
		return false;
	}
	if(!Setup_OpenShmHandles(app)){
		return false;
	}
	if(!Setup_OpenThreadHandles(app)){
		return false;
	}

	app->refreshRoutine.xMapOffset = 25; //Let initial space on left side for coordinates indicators
	app->refreshRoutine.yMapOffset = 25; //Let initial space on top side for coordinates indicators

	RECT clientSize;
	if(!GetClientRect(hWnd, &clientSize))
		return false;

	app->refreshRoutine.mapRelativeHeight = clientSize.bottom - clientSize.top;
	app->refreshRoutine.mapRelativeWidth = app->refreshRoutine.mapRelativeHeight;

	app->refreshRoutine.cellWidth = (app->refreshRoutine.mapRelativeWidth / app->map.width)+1;
	app->refreshRoutine.cellHeight = (app->refreshRoutine.mapRelativeHeight /app->map.height)+1;

	return true;
}

bool Setup_OpenThreadHandles(Application* app){
	return true;
}

bool Setup_OpenThreadHandles_RefreshRoutine(ThreadHandles* threadHandles, HANDLE hWnd){
	#pragma region Refresh Routine
	TParam_RefreshRoutine* rrParam = (TParam_RefreshRoutine*) malloc(sizeof(TParam_RefreshRoutine));
	rrParam->hWnd = hWnd;

	threadHandles->hRefreshRoutine = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_RefreshRoutine,				//Function
		(LPVOID) rrParam,					//Param
		0,									//Creation flags
		&threadHandles->dwIdRefreshRoutine	//Thread Id
	);
	#pragma endregion

	return !(threadHandles->hRefreshRoutine == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hMutex_QnARequest_CanAccess = CreateMutex(//This mutex is only created on and for ConTaxi
		NULL,							//Security attributes
		FALSE,							//Initial owner (TRUE = Locked from the creation)
		NAME_MUTEX_QnARequest);			//Mutex name
	Utils_DLL_Register(NAME_MUTEX_QnARequest, DLL_TYPE_MUTEX);

	syncHandles->hEvent_TaxiList = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_TaxiList);			//Event name
	Utils_DLL_Register(NAME_EVENT_TaxiList, DLL_TYPE_EVENT);

	syncHandles->hEvent_PassengerList = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_PassengerList);		//Event name
	Utils_DLL_Register(NAME_EVENT_PassengerList, DLL_TYPE_EVENT);

	syncHandles->hEvent_QnARequest_Read = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Read);	//Event name
	Utils_DLL_Register(NAME_EVENT_QnARequest_Read, DLL_TYPE_EVENT);

	syncHandles->hEvent_QnARequest_Write = OpenEvent(//This event is already created with CenTaxi
		EVENT_ALL_ACCESS,				//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_EVENT_QnARequest_Write);	//Event name
	Utils_DLL_Register(NAME_EVENT_QnARequest_Write, DLL_TYPE_EVENT);

	return !(syncHandles->hMutex_QnARequest_CanAccess == NULL ||
		syncHandles->hEvent_TaxiList == NULL ||
		syncHandles->hEvent_PassengerList == NULL ||
		syncHandles->hEvent_QnARequest_Read == NULL ||
		syncHandles->hEvent_QnARequest_Write == NULL);
}
bool Setup_OpenShmHandles(Application* app){
	#pragma region QnARequest
	app->shmHandles.hSHM_QnARequest = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	//Desired access flag
		FALSE,					//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_QnARequest	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_QnARequest == NULL){
		return false;
	}

	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_QnARequest == NULL){
		return false;
}
	#pragma endregion

	//Now that QnARequest SHM is mapped, we can request vars (maxTaxis, maxPassengers, mapWidth and mapHeight) from CenTaxi to open the next SHM (taxiList, passList, map)
	if(!Service_RequestMaxVars(app)){
		return false;
	}

	#pragma region Map
	app->shmHandles.hSHM_Map = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_Map	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_Map == NULL){
		return false;
	}

	app->shmHandles.lpSHM_Map = MapViewOfFile(
		app->shmHandles.hSHM_Map,	//File mapping object handle
		FILE_MAP_READ,				//Desired access flag
		0,							//DWORD high-order of the file offset where the view begins
		0,							//DWORD low-order of the file offset where the view begins
		(app->map.width * app->map.height * sizeof(char))	//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_Map == NULL){
		return false;
	}

	//Since nothing went wrong from openning the map from CenTaxi, it creates a static array and closes map shared memory, since it will not be needed anymore
	app->map.cellArray = calloc(app->map.width * app->map.height, sizeof(char));
	CopyMemory(app->map.cellArray, app->shmHandles.lpSHM_Map, (app->map.width * app->map.height) * sizeof(char));
	UnmapViewOfFile(app->shmHandles.lpSHM_Map);
	CloseHandle(app->shmHandles.hSHM_Map);
	app->shmHandles.lpSHM_Map = NULL;
	app->shmHandles.hSHM_Map = NULL;
	#pragma endregion

	#pragma region TaxiList
	app->shmHandles.hSHM_TaxiList = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_TaxiList);				//File mapping object name
	Utils_DLL_Register(NAME_SHM_TaxiList, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_TaxiList == NULL)
		return false;

	app->shmHandles.lpSHM_TaxiList = MapViewOfFile(
		app->shmHandles.hSHM_TaxiList,	//File mapping object handle
		FILE_MAP_READ,					//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(Taxi) * app->maxTaxis);	//Number of bytes to map
	Utils_DLL_Register(NAME_SHM_TaxiList, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_TaxiList == NULL)
		return false;
	#pragma endregion

	#pragma region PassengerList
	app->shmHandles.hSHM_PassengerList = OpenFileMapping(
		FILE_MAP_READ,					//Desired access flag
		FALSE,							//Inherit handle (child processes can inherit the handle)(?)
		NAME_SHM_PassengerList);		//File mapping object name
	Utils_DLL_Register(NAME_SHM_PassengerList, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_PassengerList == NULL)
		return false;

	app->shmHandles.lpSHM_PassengerList = MapViewOfFile(
		app->shmHandles.hSHM_PassengerList,		//File mapping object handle
		FILE_MAP_READ,							//Desired access flag
		0,										//DWORD high-order of the file offset where the view begins
		0,										//DWORD low-order of the file offset where the view begins
		sizeof(Passenger) * app->maxPassengers);//Number of bytes to map
	Utils_DLL_Register(NAME_SHM_TaxiList, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_PassengerList == NULL)
		return false;
	#pragma endregion
	return true;
}

void Setup_CloseAllHandles(Application* app){
}
void Setup_CloseSyncHandles(SyncHandles* syncHandles){
}
void Setup_CloseShmHandles(ShmHandles* shmHandles){
}

bool Service_RequestMaxVars(Application* app){
	QnARequest* shm = app->shmHandles.lpSHM_QnARequest;
	if(shm == NULL){
		return false;
	}

	WaitForSingleObject(app->syncHandles.hMutex_QnARequest_CanAccess, INFINITE);
	WaitForSingleObject(app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	QnARequest maxVarsRequest;
	maxVarsRequest.requestType = QnART_MAX_VARS;

	CopyMemory(shm, &maxVarsRequest, sizeof(QnARequest));
	SetEvent(app->syncHandles.hEvent_QnARequest_Read);

	WaitForSingleObject(app->syncHandles.hEvent_QnARequest_Write, INFINITE);

	app->maxTaxis = shm->maxVarsResponse.maxTaxis;
	app->maxPassengers = shm->maxVarsResponse.maxPassengers;
	app->map.width = shm->maxVarsResponse.mapWidth;
	app->map.height = shm->maxVarsResponse.mapHeight;

	ReleaseMutex(app->syncHandles.hMutex_QnARequest_CanAccess);
	SetEvent(app->syncHandles.hEvent_QnARequest_Write);

	if(app->maxTaxis < 0 ||
		app->maxPassengers < 0 ||
		app->map.width < 0 ||
		app->map.height < 0){
		return false;
	}

	return true;
}

void Paint_DrawMap(Application* app, HDC hdc, HBRUSH roadBrush, HBRUSH structureBrush, HBRUSH cellBorderBrush){
	RECT drawingRect;
	
	for(int w = 0; w < app->map.width; w++){
		for(int h = 0; h < app->map.height; h++){
			drawingRect.left = ((app->refreshRoutine.cellWidth) *w)-w + app->refreshRoutine.xMapOffset;
			drawingRect.top = (app->refreshRoutine.cellHeight*h)-h + app->refreshRoutine.yMapOffset;
			drawingRect.right = (app->refreshRoutine.cellWidth*(w+1))-w + app->refreshRoutine.xMapOffset;
			drawingRect.bottom = (app->refreshRoutine.cellHeight*(h+1))-h + app->refreshRoutine.yMapOffset;

			if(app->map.cellArray[((int) h * app->map.height) + (int) w] == MAP_STRUCTURE_CHAR){
				FillRect(hdc, &drawingRect, structureBrush);
			} else{
				FillRect(hdc, &drawingRect, roadBrush);
			}

			FrameRect(hdc, &drawingRect, cellBorderBrush);
		}
	}
}

void Paint_MapCoordinates(Application* app, HDC hdc, HBRUSH borderBrush){
	RECT drawingRect;

	TCHAR cellPlaceholder[3];

	drawingRect.left = 0;
	drawingRect.top = 0;
	drawingRect.right = app->refreshRoutine.xMapOffset;
	drawingRect.bottom = app->refreshRoutine.yMapOffset;
	FrameRect(hdc, &drawingRect, borderBrush);

	for(int i = 0; i < app->map.width; i++){
		drawingRect.left = (app->refreshRoutine.cellWidth*i) - i + app->refreshRoutine.xMapOffset;
		drawingRect.top = 0;
		drawingRect.right = (app->refreshRoutine.cellWidth*(i+1)) - i + app->refreshRoutine.xMapOffset;
		drawingRect.bottom = app->refreshRoutine.yMapOffset;
		FrameRect(hdc, &drawingRect, borderBrush);

		swprintf_s(cellPlaceholder, 3, TEXT("%d\0"), i);
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, cellPlaceholder, -1, &drawingRect, DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(hdc, OPAQUE);
	}

	for(int i = 0; i < app->map.height; i++){
		drawingRect.left = 0;
		drawingRect.top = (app->refreshRoutine.cellHeight*i) - i + app->refreshRoutine.yMapOffset;
		drawingRect.right = app->refreshRoutine.xMapOffset;
		drawingRect.bottom = (app->refreshRoutine.cellHeight*(i+1)) - i + app->refreshRoutine.yMapOffset;
		FrameRect(hdc, &drawingRect, borderBrush);

		swprintf_s(cellPlaceholder, 3, TEXT("%d\0"), i);
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, cellPlaceholder, -1, &drawingRect, DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(hdc, OPAQUE);
	}
}

void Paint_Taxis(Application* app, HDC hdc, HBRUSH taxiBrush){
	Taxi* taxiList = (Taxi*) app->shmHandles.lpSHM_TaxiList;

	if(taxiList == NULL)
		return;

	TCHAR mapId[3];
	RECT drawingRect;
	for(int i = 0; i < app->maxTaxis; i++){
		if(taxiList[i].empty)
			continue;

		Get_RectFromXY(app, &drawingRect, (int) taxiList[i].object.xyPosition.x, (int) taxiList[i].object.xyPosition.y);

		if(Utils_StringIsEmpty(taxiList[i].transporting.passId))
			swprintf_s(mapId, 3, TEXT("T%d\0"), i+1);
		else if(!Utils_StringIsEmpty(taxiList[i].transporting.passId) && taxiList[i].state == TS_OTW_PASS)
			swprintf_s(mapId, 3, TEXT("Y%d\0"), i+1);
		else
			swprintf_s(mapId, 3, TEXT("B%d\0"), i+1);
		FillRect(hdc, &drawingRect, taxiBrush);
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, mapId, -1, &drawingRect, DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(hdc, OPAQUE);
	}
}

void Paint_Passengers(Application* app, HDC hdc, HBRUSH passengerBrush){
	Passenger* passengerList = (Passenger*) app->shmHandles.lpSHM_PassengerList;

	if(passengerList == NULL)
		return;

	TCHAR mapId[3];
	RECT drawingRect;
	for(int i = 0; i < app->maxTaxis; i++){
		if(passengerList[i].empty)
			continue;

		Get_RectFromXY(app, &drawingRect, (int) passengerList[i].object.xyPosition.x, (int) passengerList[i].object.xyPosition.y);

		swprintf_s(mapId, 3, TEXT("P%d\0"), i+1);
		FillRect(hdc, &drawingRect, passengerBrush);
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, mapId, -1, &drawingRect, DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(hdc, OPAQUE);
	}

	//taxi = (Taxi*) Get_TaxiAt(app, w, h, &taxiIndex);
	//passenger = (Passenger*) Get_PassengerAt(app, w, h, &passengerIndex);
	//if(taxi != NULL){
	//	swprintf_s(taxiId, 3, TEXT("T%d\0"), taxiIndex+1);
	//	FillRect(hdc, &drawingRect, taxiBrush);
	//	SetBkMode(hdc, TRANSPARENT);
	//	DrawText(hdc, taxiId, -1, &drawingRect, DT_CENTER | DT_NOCLIP);
	//	SetBkMode(hdc, OPAQUE);
	//} else if(passenger != NULL){
	//	swprintf_s(passengerId, 3, TEXT("P%d\0"), passengerIndex+1);
	//	FillRect(hdc, &drawingRect, passengerBrush);
	//	SetBkMode(hdc, TRANSPARENT);
	//	DrawText(hdc, passengerId, -1, &drawingRect, DT_CENTER | DT_NOCLIP);
	//	SetBkMode(hdc, OPAQUE);
	//}
}

void Get_RectFromXY(Application* app, RECT* rRect, int x, int y){
	rRect->left = ((app->refreshRoutine.cellWidth) *x)-x + app->refreshRoutine.xMapOffset;
	rRect->top = (app->refreshRoutine.cellHeight*y)-y + app->refreshRoutine.yMapOffset;
	rRect->right = (app->refreshRoutine.cellWidth*(x+1))-x + app->refreshRoutine.xMapOffset;
	rRect->bottom = (app->refreshRoutine.cellHeight*(y+1))-y + app->refreshRoutine.yMapOffset;
}

Taxi* Get_TaxiAt(Application* app, int X, int Y, int* taxiIndex){
	Taxi* taxiList = (Taxi*) app->shmHandles.lpSHM_TaxiList;

	for(int i = 0; i < app->maxTaxis; i++){
		if(taxiList[i].empty)
			continue;

		if(taxiList[i].object.xyPosition.x == X &&
			taxiList[i].object.xyPosition.y == Y){
			*taxiIndex = i;
			return &taxiList[i];
		}
	}

	return NULL;
}

Passenger* Get_PassengerAt(Application* app, int X, int Y, int* passengerIndex){
	Passenger* passengerList = (Passenger*) app->shmHandles.lpSHM_PassengerList;

	for(int i = 0; i < app->maxPassengers; i++){
		if(passengerList[i].empty)
			continue;

		if(passengerList[i].object.xyPosition.x == X &&
			passengerList[i].object.xyPosition.y == Y){
			*passengerIndex = i;
			return &passengerList[i];
		}
	}

	return NULL;
}