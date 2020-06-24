#pragma once
#include "CenThreads.h"
#include "CenService.h"

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers){
	ZeroMemory(app, sizeof(Application));
	srand((unsigned int) time(NULL));
	app->keepRunning = true;
	app->settings.secAssignmentTimeout = DEFAULT_ASSIGNMENT_TIMEOUT;
	app->settings.allowTaxiLogins = DEFAULT_ALLOW_TAXI_LOGINS;

	app->maxTaxis = maxTaxis;
	app->maxPassengers = maxPassengers;

	app->taxiList = calloc(maxTaxis, sizeof(Taxi));
	app->passengerList = calloc(maxPassengers, sizeof(CenPassenger));
	app->transportList = calloc(NTBUFFER_MAX, sizeof(CenTransportRequest));
	
	int i;
	for(i = 0; i < maxTaxis; i++){
		ZeroMemory(&app->taxiList[i], sizeof(Taxi));
		app->taxiList[i].taxiInfo.empty = true;
	}

	for(i = 0; i < maxPassengers; i++){
		ZeroMemory(&app->passengerList[i], sizeof(CenPassenger));
		app->passengerList[i].passengerInfo.empty = true;
	}

	for(i = 0; i < NTBUFFER_MAX; i++){
		ZeroMemory(&app->transportList[i], sizeof(CenTransportRequest));
		app->transportList[i].interestedTaxis = calloc(maxTaxis, sizeof(int));

		for(int a = 0; a < maxTaxis; a++)
			app->transportList[i].interestedTaxis[a] = -1;
	}

	if((app->passengerList == NULL))
		return false;
	if((app->taxiList == NULL))
		return false;
	if((app->transportList == NULL))
		return false;
	if(!Setup_OpenSyncHandles(app))
		return false;
	if(!Setup_OpenShmHandles(app))
		return false;
	if(!Setup_OpenMap(app))
		return false;
	if(!Setup_OpenNamedPipes(&app->namedPipeHandles))
		return false;
	if(!Setup_OpenThreadHandles(app)) //Has to be called at the end, because it will use Sync and SMH
		return false;

	return true;
}

bool Setup_OpenNamedPipes(NamedPipeHandles* namedPipeHandles){
	HANDLE hPipe_Read = CreateNamedPipe(
		NAME_NAMEDPIPE_CommsP2C,	//Named Pipe name
		PIPE_ACCESS_INBOUND,		//Access to read
		PIPE_TYPE_MESSAGE |			//Message type
		PIPE_WAIT,					//Blocking mode
		1,							//Max instances
		0,							//Buffer size of output
		sizeof(CommsP2C),			//Buffer size of input
		0,							//Client timeout
		NULL);						//Security attributes

	//If the CreateNamedPipe return invalid handle check if busy or unexpected error
	if(hPipe_Read == INVALID_HANDLE_VALUE){
		if(GetLastError() == ERROR_PIPE_BUSY)
			_tprintf(TEXT("%sFailed while trying to connect to ConPass! There is already one ConPass connected..."), Utils_NewSubLine());
		else
			_tprintf(TEXT("%sCreateNamedPipe failed! Error: %d"), Utils_NewSubLine(), GetLastError());

		return false;
	}
	namedPipeHandles->hCPRead = hPipe_Read;

	HANDLE hPipe_Write = CreateNamedPipe(
		NAME_NAMEDPIPE_CommsC2P,	//Named Pipe name
		PIPE_ACCESS_OUTBOUND,		//Access to write
		PIPE_TYPE_MESSAGE |			//Message type
		PIPE_WAIT,					//Blocking mode
		1,							//Max instances
		sizeof(CommsC2P),			//Buffer size of output
		0,							//Buffer size of input
		0,							//Client timeout
		NULL);						//Security attributes

	//If the CreateNamedPipe return invalid handle check if busy or unexpected error
	if(hPipe_Write == INVALID_HANDLE_VALUE){
		if(GetLastError() == ERROR_PIPE_BUSY)
			_tprintf(TEXT("%sFailed while trying to connect to ConPass! There is already one ConPass connected..."), Utils_NewSubLine());
		else
			_tprintf(TEXT("%sCreateNamedPipe failed! Error: %d"), Utils_NewSubLine(), GetLastError());

		return false;
	}
	namedPipeHandles->hCPWrite = hPipe_Write;

	HANDLE hPipe_QnA = CreateNamedPipe(
		NAME_NAMEDPIPE_CommsPCQnA,	//Named Pipe name
		PIPE_ACCESS_DUPLEX,			//Access to read and write
		PIPE_TYPE_MESSAGE |			//Message type
		PIPE_WAIT,					//Blocking mode
		1,							//Max instances
		sizeof(CommsC2P),			//Buffer size of output
		0,							//Buffer size of input
		0,							//Client timeout
		NULL);						//Security attributes

	//If the CreateNamedPipe return invalid handle check if busy or unexpected error
	if(hPipe_QnA == INVALID_HANDLE_VALUE){
		if(GetLastError() == ERROR_PIPE_BUSY)
			_tprintf(TEXT("%sFailed while trying to connect to ConPass! There is already one ConPass connected..."), Utils_NewSubLine());
		else
			_tprintf(TEXT("%sCreateNamedPipe failed! Error: %d"), Utils_NewSubLine(), GetLastError());

		return false;
	}
	namedPipeHandles->hCPQnA = hPipe_QnA;

	return true;
}

bool Setup_OpenThreadHandles(Application* app){
	#pragma region QnARequest
	TParam_QnARequest* qnarParam = (TParam_QnARequest*) malloc(sizeof(TParam_QnARequest));
	qnarParam->app = app;

	app->threadHandles.hQnARequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_ReceiveQnARequests,			//Function
		(LPVOID) qnarParam,						//Param
		0,									//Creation flags
		&app->threadHandles.dwIdQnARequests //Thread Id
	);
	#pragma endregion

	#pragma region TossRequest
	TParam_ConsumeTossRequests* ctrParam = (TParam_ConsumeTossRequests*) malloc(sizeof(TParam_ConsumeTossRequests));
	ctrParam->app = app;

	app->threadHandles.hTossRequests = CreateThread(
		NULL,								//Security Attributes
		0,									//Stack Size (0 = default)
		Thread_ConsumeTossRequests,			//Function
		(LPVOID) ctrParam,						//Param
		0,									//Creation flags
		&app->threadHandles.dwIdTossRequests //Thread Id
	);
	#pragma endregion

	#pragma region ConnectingTaxiPipes
	TParam_ConnectingTaxiPipes* ctpParam = (TParam_ConnectingTaxiPipes*) malloc(sizeof(TParam_ConnectingTaxiPipes));
	ctpParam->app = app;

	app->threadHandles.hConnectingTaxiPipes = CreateThread(
		NULL,										//Security Attributes
		0,											//Stack Size (0 = default)
		Thread_ConnectingTaxiPipes,					//Function
		(LPVOID) ctpParam,							//Param
		0,											//Creation flags
		&app->threadHandles.dwIdConnectingTaxiPipes	//Thread Id
	);
	#pragma endregion

	#pragma region ReadingConPassNamedPipeQnA
	TParam_ReadConPassNPQnA* qnaParam = (TParam_ReadConPassNPQnA*) malloc(sizeof(TParam_ReadConPassNPQnA));
	qnaParam->app = app;

	app->threadHandles.hReadConPassNPQnA = CreateThread(
		NULL,										//Security Attributes
		0,											//Stack Size (0 = default)
		Thread_ReadConPassNPQnA,					//Function
		(LPVOID) qnaParam,							//Param
		0,											//Creation flags
		&app->threadHandles.dwIdReadConPassNPQnA	//Thread Id
	);
	#pragma endregion

	#pragma region ReadingConPassNamedPipeToss
	TParam_ReadConPassNPToss* tossParam = (TParam_ReadConPassNPToss*) malloc(sizeof(TParam_ReadConPassNPToss));
	tossParam->app = app;

	app->threadHandles.hReadConPassNPToss = CreateThread(
		NULL,										//Security Attributes
		0,											//Stack Size (0 = default)
		Thread_ReadConPassNPToss,					//Function
		(LPVOID) tossParam,							//Param
		0,											//Creation flags
		&app->threadHandles.dwIdReadConPassNPToss	//Thread Id
	);
	#pragma endregion

	return !(app->threadHandles.hQnARequests == NULL ||
		app->threadHandles.hTossRequests == NULL ||
		app->threadHandles.hConnectingTaxiPipes == NULL ||
		app->threadHandles.hReadConPassNPQnA == NULL ||
		app->threadHandles.hReadConPassNPToss == NULL);
}
bool Setup_OpenSyncHandles(Application* app){
	app->syncHandles.hEvent_QnARequest_Read = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		FALSE,						//Initial State
		NAME_EVENT_QnARequest_Read	//Event Name
	);

	Utils_DLL_Register(NAME_EVENT_QnARequest_Read, DLL_TYPE_EVENT);
	app->syncHandles.hEvent_QnARequest_Write = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		TRUE,						//Initial State
		NAME_EVENT_QnARequest_Write	//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_QnARequest_Write, DLL_TYPE_EVENT);

	app->syncHandles.hEvent_Notify_T_NewTranspReq = CreateEvent(
		NULL,							//Security Attributes
		TRUE,							//Manual Reset
		FALSE,							//Initial State
		NAME_EVENT_NewTransportRequest	//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_NewTransportRequest, DLL_TYPE_EVENT);

	app->syncHandles.hEvent_NewTaxiSpot = CreateEvent(
		NULL,							//Security Attributes
		FALSE,							//Manual Reset
		TRUE,							//Initial State
		NAME_EVENT_NewTaxiSpot			//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_NewTaxiSpot, DLL_TYPE_EVENT);

	app->syncHandles.hSemaphore_HasTossRequest = CreateSemaphore(
		NULL,							//Security Attributes
		0,								//Initial Count
		TOSSBUFFER_MAX,					//Max Count
		NAME_SEMAPHORE_HasTossRequest	//Semaphore Name
	);
	Utils_DLL_Register(NAME_SEMAPHORE_HasTossRequest, DLL_TYPE_SEMAPHORE);

	app->syncHandles.hSemaphore_TaxiNPSpots = CreateSemaphore(
		NULL,							//Security Attributes
		app->maxTaxis,					//Initial Count
		app->maxTaxis,					//Max Count
		NAME_SEMAPHORE_TaxiNPSpots		//Semaphore Name
	);
	Utils_DLL_Register(NAME_SEMAPHORE_TaxiNPSpots, DLL_TYPE_SEMAPHORE);

	return !(app->syncHandles.hEvent_QnARequest_Read == NULL ||
		app->syncHandles.hEvent_QnARequest_Write == NULL ||
		app->syncHandles.hEvent_Notify_T_NewTranspReq == NULL ||
		app->syncHandles.hEvent_NewTaxiSpot == NULL ||
		app->syncHandles.hSemaphore_HasTossRequest == NULL ||
		app->syncHandles.hSemaphore_TaxiNPSpots == NULL);
}
bool Setup_OpenShmHandles(Application* app){
	#pragma region QnARequest
	app->shmHandles.hSHM_QnARequest = CreateFileMapping(
		INVALID_HANDLE_VALUE,	//File handle
		NULL,					//Security Attributes
		PAGE_READWRITE,			//Protection flags
		0,						//DWORD high-order max size
		sizeof(QnARequest),		//DWORD low-order max size
		NAME_SHM_QnARequest		//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_QnARequest == NULL)
		return false;
	app->shmHandles.lpSHM_QnARequest = MapViewOfFile(
		app->shmHandles.hSHM_QnARequest,//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(QnARequest)				//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_QnARequest, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_QnARequest == NULL)
		return false;
	#pragma endregion
	
	#pragma region NewTransportBuffer
	app->shmHandles.hSHM_NTBuffer = CreateFileMapping(
		INVALID_HANDLE_VALUE,			//File handle
		NULL,							//Security Attributes
		PAGE_READWRITE,					//Protection flags
		0,								//DWORD high-order max size
		sizeof(TransportBuffer),		//DWORD low-order max size
		NAME_SHM_TransportRequestBuffer	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_TransportRequestBuffer, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_NTBuffer == NULL)
		return false;

	app->shmHandles.lpSHM_NTBuffer = MapViewOfFile(
		app->shmHandles.hSHM_NTBuffer,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(TransportBuffer)		//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_TransportRequestBuffer, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_NTBuffer == NULL)
		return false;

	ZeroMemory(app->shmHandles.lpSHM_NTBuffer, sizeof(TransportBuffer)); //Makes sure head starts at 0 (unecessary to zero everything)
	
	TransportBuffer* buffer = app->shmHandles.lpSHM_NTBuffer;
	for(int i = 0; i < NTBUFFER_MAX; i++)
		buffer->transportRequests[i].empty = true;

	#pragma endregion

	#pragma region Map
	/*Is being openned by Setup_OpenShmHandles_Map(), which is being called at OpenMap()
	**	Reason: After openning the map, it must write in on shared memory, hence needing the shared memory
	**	HOWEVER, creating the shared memory needs the width and height of the map
	**	SO, OpenMap() opens the map, gathers info about width and height, 
	**	THEN, creates the shared memory using Setup_OpenShmHandles_Map()
	**	AND, write the map into the memory
	*/
	#pragma endregion

	#pragma region TossRequest
	app->shmHandles.hSHM_TossReqBuffer = CreateFileMapping(
		INVALID_HANDLE_VALUE,			//File handle
		NULL,							//Security Attributes
		PAGE_READWRITE,					//Protection flags
		0,								//DWORD high-order max size
		sizeof(TossRequestsBuffer),		//DWORD low-order max size
		NAME_SHM_TossRequestBuffer		//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_TossRequestBuffer, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_TossReqBuffer == NULL)
		return false;

	app->shmHandles.lpSHM_TossReqBuffer = MapViewOfFile(
		app->shmHandles.hSHM_TossReqBuffer,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof(TossRequestsBuffer)		//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_TossRequestBuffer, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_TossReqBuffer == NULL)
		return false;

	ZeroMemory(app->shmHandles.lpSHM_TossReqBuffer, sizeof(TossRequestsBuffer)); //Makes sure head and tail starts at 0 (unecessary to zero everything)
	#pragma endregion

	return true;
}
bool Setup_OpenShmHandles_Map(Application* app){
	app->shmHandles.hSHM_Map = CreateFileMapping(
		INVALID_HANDLE_VALUE,			//File handle
		NULL,							//Security Attributes
		PAGE_READWRITE,					//Protection flags
		0,								//DWORD high-order max size
		sizeof((app->map.width * app->map.height) * sizeof(char)),		//DWORD low-order max size
		NAME_SHM_Map	//File mapping object name
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_FILEMAPPING);
	if(app->shmHandles.hSHM_Map == NULL)
		return false;

	app->shmHandles.lpSHM_Map = MapViewOfFile(
		app->shmHandles.hSHM_Map,	//File mapping object handle
		FILE_MAP_ALL_ACCESS,			//Desired access flag
		0,								//DWORD high-order of the file offset where the view begins
		0,								//DWORD low-order of the file offset where the view begins
		sizeof((app->map.width * app->map.height) * sizeof(char))		//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_Map, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_Map == NULL)
		return false;

	return true;
}
bool Setup_OpenMap(Application* app){
	FILE* mapFile;
	errno_t mapFileFailed;
	mapFileFailed = fopen_s(&mapFile, "../Maps/map.txt", "r");
	if(mapFileFailed){
		_tprintf(TEXT("%sOh no! The map file could not be loaded!"), Utils_NewLine());
		return false;
	}

	char currentChar;
	int columnCount;
	int lineCount;
	int biggestColumn = 0;

	/*The following FOR uses 2 iterations, the reason is only to shorten the code, but in return it gets a bit messy
	**	Iteration 0: Goes through all the characters; 
	**				 When finds \n it means it is starting a new line;
	**				 Counts how many lines and columns the map has;
	**				 If any line has a different column count then the others the map will be considerated invalid.
	**	Iteration 1: Checks if width and height follow the minimum dimenstion requirement, if not, it is invalid;
	**				 Will create a shared memory with the dimentions gathered on Iteration 0;
	**				 Allocate an array with the size of ((width * height) * sizeof(char)) using calloc (basically ZeroMemory after allocating)
	**				 Jump back to the beginning of the file and reset line and column count vars;
	**				 Verifies each character if it is valid, only 2 characters are allowed (MAP_STRUCTURE_CHAR and MAP_ROAD_CHAR);
	**				 Writes each character into the allocated array.
	*/

	for(int i = 0; i < 2; i++){
		if(i == 1){
			if(app->map.height < MAP_MIN_HEIGHT || app->map.width < MAP_MIN_WIDTH){
				_tprintf(TEXT("%sInvalid map! Needs have the minimum dimensions! (width: %d | height: %d)"), Utils_NewLine(), MAP_MIN_WIDTH, MAP_MIN_HEIGHT);
				return false;
			}
			
			if(!Setup_OpenShmHandles_Map(app))
				return false;

			app->map.cellArray = calloc(app->map.height * app->map.width, sizeof(char));
			if(app->map.cellArray == NULL)
				return false;
		}

		fseek(mapFile, 0, SEEK_SET);
		lineCount = 0;
		columnCount = 0;

		while(currentChar = fgetc(mapFile)){
			if(feof(mapFile))
				break;

			if(currentChar == '\n'){
				if(i == 0){
					if(biggestColumn == 0)
						biggestColumn = columnCount;
					else if(columnCount != biggestColumn){//Different width in diferent lines, map is not square
						_tprintf(TEXT("%sInvalid map! Needs have the same column width in every lines!"), Utils_NewLine());
						return false;
					}
				}
				columnCount = 0;
				lineCount++;
				continue;
			} else if(currentChar != MAP_STRUCTURE_CHAR && currentChar != MAP_ROAD_CHAR){
				_tprintf(TEXT("%sInvalid map! Contains invalid characters!%sCharacter found: %c%sAcceptable characters: %c and %c"),
					Utils_NewLine(),
					Utils_NewSubLine(),
					currentChar,
					Utils_NewSubLine(), 
					MAP_STRUCTURE_CHAR,
					MAP_ROAD_CHAR);

				return false;
			}

			if(i == 1)
				app->map.cellArray[(lineCount * app->map.width) + columnCount] = currentChar;
			columnCount++;
		}
		lineCount++; //Last line doesn't contain \n, so we have to compensate for the line count
		app->map.width = columnCount;
		app->map.height = lineCount;
	}

	CopyMemory(app->shmHandles.lpSHM_Map, app->map.cellArray, (app->map.height * app->map.width) * sizeof(char));

	fclose(mapFile);
	return true;
}

void Setup_CloseAllHandles(Application* app){
	Setup_CloseSyncHandles(&(app->syncHandles));
	Setup_CloseShmHandles(&(app->shmHandles));
}
void Setup_CloseSyncHandles(SyncHandles* syncHandles){
	CloseHandle(syncHandles->hEvent_QnARequest_Read);
	CloseHandle(syncHandles->hEvent_QnARequest_Write);
	CloseHandle(syncHandles->hEvent_Notify_T_NewTranspReq);
	CloseHandle(syncHandles->hSemaphore_HasTossRequest);
}
void Setup_CloseShmHandles(ShmHandles* shmHandles){
	#pragma region QnARequest
	UnmapViewOfFile(shmHandles->lpSHM_QnARequest);
	CloseHandle(shmHandles->hSHM_QnARequest);
	#pragma endregion
	#pragma region NewTransportBuffer
	UnmapViewOfFile(shmHandles->lpSHM_NTBuffer);
	CloseHandle(shmHandles->hSHM_NTBuffer);
	#pragma endregion
	#pragma region Map
	UnmapViewOfFile(shmHandles->lpSHM_Map);
	CloseHandle(shmHandles->hSHM_Map);
	#pragma endregion
	#pragma region TossRequestBuffer
	UnmapViewOfFile(shmHandles->lpSHM_TossReqBuffer);
	CloseHandle(shmHandles->hSHM_TossReqBuffer);
	#pragma endregion
}

bool Add_Taxi(Application* app, TCHAR* licensePlate, XY xyStartingPosition){
	/*No need for more validation...
	**Since it is assumed that this function is only called at Service_LoginTaxi, which validates everything
	*/

	CenTaxi* anchorTaxi = &app->taxiList[Get_FreeIndexTaxiList(app)];
	if(anchorTaxi == NULL)
		return false;

	_tprintf(TEXT("%s[Taxi Login] %s at (%.2lf, %.2lf)"),
		Utils_NewSubLine(),
		licensePlate,
		xyStartingPosition.x,
		xyStartingPosition.y);

	anchorTaxi->taxiInfo.empty = false;
	_tcscpy_s(anchorTaxi->taxiInfo.LicensePlate, _countof(anchorTaxi->taxiInfo.LicensePlate), licensePlate);
	anchorTaxi->taxiInfo.object.xyPosition = xyStartingPosition;
	return true;
}
bool Delete_Taxi(Application* app, int index){
	if(index < 0 || index >= app->maxTaxis)
		return false;

	CenTaxi* anchorTaxi = Get_Taxi(app, index);
	if(anchorTaxi == NULL)
		return false;

	_tprintf(TEXT("%s[Taxi Logout] %s at (%.2lf, %.2lf)"),
		Utils_NewSubLine(),
		anchorTaxi->taxiInfo.LicensePlate,
		anchorTaxi->taxiInfo.object.xyPosition.x,
		anchorTaxi->taxiInfo.object.xyPosition.y);

	anchorTaxi->taxiInfo.empty = true;
	if(anchorTaxi->taxiNamedPipe != NULL){
		ReleaseSemaphore(app->syncHandles.hSemaphore_TaxiNPSpots, 1, NULL);
		Utils_CloseNamedPipe(anchorTaxi->taxiNamedPipe);
	}
	SetEvent(app->syncHandles.hEvent_NewTaxiSpot);

	return true;
}
int Get_QuantLoggedInTaxis(Application* app){
	int quantLoggedInTaxis = 0;

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].taxiInfo.empty)
			quantLoggedInTaxis++;
	}

	return quantLoggedInTaxis;
}
bool isTaxiListFull(Application* app){
	return Get_QuantLoggedInTaxis(app) >= app->maxTaxis;
}
int Get_FreeIndexTaxiList(Application* app){
	if(isTaxiListFull(app))
		return -1;

	for(int i = 0; i < app->maxTaxis; i++){
		if(app->taxiList[i].taxiInfo.empty)
			return i;
	}

	return -1;
}
int Get_TaxiIndex(Application* app, TCHAR* licensePlate){
	if(app->taxiList == NULL)
		return -1;

	for(int i = 0; i < app->maxTaxis; i++){
		if(_tcscmp(app->taxiList[i].taxiInfo.LicensePlate, licensePlate) == 0 && !app->taxiList[i].taxiInfo.empty)
			return i;
	}

	return -1;
}
CenTaxi* Get_Taxi(Application* app, int index){
	if(app->taxiList == NULL)
		return NULL;

	if(!app->taxiList[index].taxiInfo.empty)
		return &app->taxiList[index];

	return NULL;
}
CenTaxi* Get_TaxiAt(Application* app, XY xyPosition){
	if(app->taxiList == NULL)
		return NULL;

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].taxiInfo.empty &&
			((int) app->taxiList[i].taxiInfo.object.xyPosition.x) == xyPosition.x &&
			((int) app->taxiList[i].taxiInfo.object.xyPosition.y) == xyPosition.y)
			return &app->taxiList[i];
	}

	return NULL;
}

bool Add_Passenger(Application* app, TCHAR* id, XY xyStartingPosition, XY xyDestination){
	/*No need for more validation...
	**Since it is assumed that this function is only called at Service_LoginPass, which validates everything
	*/

	CenPassenger* anchorPass = &app->passengerList[Get_FreeIndexPassengerList(app)];
	if(anchorPass == NULL)
		return false;

	TransportRequest newTR;
	newTR.empty = false;
	_tcscpy_s(newTR.passId, _countof(newTR.passId), id);
	newTR.xyStartingPosition = xyStartingPosition;
	newTR.xyDestination = xyDestination;

	if(!Service_NewTransportRequest(app, newTR))
		return false;

	_tprintf(TEXT("%s[Passenger Login] %s at (%.2lf, %.2lf) with intent of going to (%.2lf, %.2lf)"),
		Utils_NewSubLine(),
		id,
		xyStartingPosition.x,
		xyStartingPosition.y,
		xyDestination.x,
		xyDestination.y);

	anchorPass->passengerInfo.empty = false;
	_tcscpy_s(anchorPass->passengerInfo.Id, _countof(anchorPass->passengerInfo.Id), id);
	anchorPass->passengerInfo.object.xyPosition = xyStartingPosition;
	anchorPass->xyDestination = xyDestination;

	return true;
}
bool Delete_Passenger(Application* app, int index){
	if(index < 0 || index >= app->maxPassengers)
		return false;

	CenPassenger* anchorPassenger = Get_Passenger(app, index);
	if(anchorPassenger == NULL)
		return false;

	_tprintf(TEXT("%s[Passenger Logout] %s at (%.2lf, %.2lf) with intent of going to (%.2lf, %.2lf)"),
		Utils_NewSubLine(),
		anchorPassenger->passengerInfo.Id,
		anchorPassenger->passengerInfo.object.xyPosition.x,
		anchorPassenger->passengerInfo.object.xyPosition.y,
		anchorPassenger->xyDestination.x,
		anchorPassenger->xyDestination.y);

	anchorPassenger->passengerInfo.empty = true;

	CommsC2P sendPassNotification;
	CommsC2P_PassRemoval removalPassComms;
	sendPassNotification.commType = C2P_PASS_REMOVAL;
	_tcscpy_s(removalPassComms.passId, _countof(removalPassComms.passId), anchorPassenger->passengerInfo.Id);
	sendPassNotification.removeComm = removalPassComms;

	WriteFile(
		app->namedPipeHandles.hCPWrite,	//Named pipe handle
		&sendPassNotification,			//Write from 
		sizeof(CommsC2P),				//Size being written
		NULL,							//Quantity Bytes written
		NULL);							//Overlapped IO
	return true;
}
int Get_QuantLoggedInPassengers(Application* app){
	int quantLoggedInPassengers = 0;

	for(int i = 0; i < app->maxPassengers; i++){
		if(!app->passengerList[i].passengerInfo.empty)
			quantLoggedInPassengers++;
	}

	return quantLoggedInPassengers;
}
bool isPassengerListFull(Application* app){
	return Get_QuantLoggedInPassengers(app) >= app->maxPassengers;
}
int Get_FreeIndexPassengerList(Application* app){
	if(isPassengerListFull(app))
		return -1;

	for(int i = 0; i < app->maxPassengers; i++){
		if(app->passengerList[i].passengerInfo.empty)
			return i;
	}

	return -1;
}
int Get_PassengerIndex(Application* app, TCHAR* Id){
	if(app->passengerList == NULL)
		return -1;

	for(int i = 0; i < app->maxPassengers; i++){
		if(_tcscmp(app->passengerList[i].passengerInfo.Id, Id) == 0 && !app->passengerList[i].passengerInfo.empty)
			return i;
	}

	return -1;
}
CenPassenger* Get_Passenger(Application* app, int index){
	if(app->passengerList == NULL)
		return NULL;

	if(!app->passengerList[index].passengerInfo.empty)
		return &app->passengerList[index];

	return NULL;
}

int Get_TransportIndex(Application* app, TCHAR* idPassenger){
	TransportBuffer* buffer = app->shmHandles.lpSHM_NTBuffer;

	for(int i = 0; i < NTBUFFER_MAX; i++){
		if(buffer->transportRequests[0].empty)
			continue;

		if(_tcscmp(buffer->transportRequests[i].passId, idPassenger) == 0)
			return i;
	}

	return -1;
}
TransportRequest Get_TransportRequest(Application* app, int index){
	TransportBuffer* buffer = app->shmHandles.lpSHM_NTBuffer;
	
	return buffer->transportRequests[index];
}

bool isValid_ObjectPosition(Application* app, XY xyPosition){
	if(xyPosition.x < 0 || xyPosition.x >= app->map.width || xyPosition.y < 0 || xyPosition.y >= app->map.height)
		return false;

	return true;
}

CentralCommands Service_UseCommand(Application* app, TCHAR* command){
	if(_tcscmp(command, CMD_HELP) == 0){ //Continues on Main (listing commands)
		return CC_HELP;
	} else if(_tcscmp(command, CMD_LIST_TAXIS) == 0){ //Continues on Main (listing logged in taxis)
		return CC_LIST_TAXIS;
	} else if(_tcscmp(command, CMD_LIST_PASS) == 0){ //Continues on Main (listing logged in passengers)
		return CC_LIST_PASS;
	} else if(_tcscmp(command, CMD_SET_TIMEOUT) == 0){ //Continues on Main (asking for a value)
		return CC_SET_TIMEOUT;
	} else if(_tcscmp(command, CMD_TAXI_LOGIN_ON) == 0){
		Command_AllowTaxiLogins(app, true);
		return CC_TAXI_LOGIN_ON;
	} else if(_tcscmp(command, CMD_TAXI_LOGIN_OFF) == 0){
		Command_AllowTaxiLogins(app, false);
		return CC_TAXI_LOGIN_OFF;
	} else if(_tcscmp(command, CMD_KICK_TAXI) == 0){ //Continues on Main (asking for value argument)
		return CC_KICK_TAXI;
	} else if(_tcscmp(command, CMD_SHOW_MAP) == 0){
		Temp_ShowMap(app);
		return CC_SHOW_MAP;
	} else if(_tcscmp(command, CMD_SAVE_REGISTRY) == 0){
		Temp_SaveRegistry(app);
		return CC_SAVE_REGISTRY;
	} else if(_tcscmp(command, CMD_LOAD_REGISTRY) == 0){
		Temp_LoadRegistry(app);
		return CC_LOAD_REGISTRY;
	} else if(_tcscmp(command, CMD_DLL_LOG) == 0){
		Utils_DLL_Test();
		return CC_DLL_LOG;
	} else if(_tcscmp(command, CMD_CREATE_PATH) == 0){
		Temp_CreatePath(app);
		return CC_CREATE_PATH;
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Service_CloseApp(app);
		return CC_CLOSEAPP;
	}

	return CC_UNDEFINED;
}

TaxiLoginResponseType Service_LoginTaxi(Application* app, TaxiLoginRequest* loginRequest){
	if(loginRequest == NULL || Utils_StringIsEmpty(loginRequest->licensePlate))
		return TLR_INVALID_UNDEFINED;

	if(!app->settings.allowTaxiLogins)
		return TLR_INVALID_CLOSED;

	if(!isValid_ObjectPosition(app, loginRequest->xyStartingPosition))
		return TLR_INVALID_POSITION;

	if(Get_TaxiIndex(app, loginRequest->licensePlate) != -1)
		return TLR_INVALID_EXISTS;

	/*Recommened to be last invalid before adding
	**For example:
	** Taxi can already exist and spots are full
	** It's recommended to feedback saying that there is already a taxi with same license plate
	** Than placing taxi in queue only to be rejected after
	*/
	if(isTaxiListFull(app))
		return TLR_INVALID_FULL;

	if(!Add_Taxi(app, loginRequest->licensePlate, loginRequest->xyStartingPosition))
		return TLR_INVALID_UNDEFINED;

	return TLR_SUCCESS;
}
CommsC2P_Resp_Login Service_LoginPass(Application* app, CommsP2C_Login* loginRequest){
	if(loginRequest == NULL ||
		Utils_StringIsEmpty(loginRequest->id))
		return PLR_INVALID_UNDEFINED;

	if(!isValid_ObjectPosition(app, loginRequest->xyStartingPosition)) //Current position is an invalid cell
		return PLR_INVALID_POSITION;

	if(!isValid_ObjectPosition(app, loginRequest->xyDestination) || //Destiny has an invalid cell
		(loginRequest->xyStartingPosition.x == loginRequest->xyDestination.x && 
			loginRequest->xyStartingPosition.y == loginRequest->xyDestination.y)) //Destiny is same as actual position
		return PLR_INVALID_DESTINY;

	if(Get_PassengerIndex(app, loginRequest->id) != -1)
		return PLR_INVALID_EXISTS;

	if(isPassengerListFull(app))
		return PLR_INVALID_FULL;

	if(!Add_Passenger(app, loginRequest->id, loginRequest->xyStartingPosition, loginRequest->xyDestination))
		return PLR_INVALID_TRANSPBUFFER_FULL;

	return PLR_SUCCESS;
}
bool Service_NewTransportRequest(Application* app, TransportRequest transportReq){
	if(transportReq.empty)
		return false;

	TransportBuffer* transportBuffer = (TransportBuffer*) app->shmHandles.lpSHM_NTBuffer;
	
	/*Do not allow overwrite on non-empty spots
	**There can't be more than "NTBUFFER_MAX" of TransportRequests
	**If HEAD (next written spot) has a non empty TransportRequest, means that, that TR is still being assigned to someone
	*/
	if(!transportBuffer->transportRequests[transportBuffer->head].empty)
		return false;

	TParam_TaxiAssignment* param = (TParam_TaxiAssignment*) malloc(sizeof(TParam_TaxiAssignment));
	param->app = app;
	param->myIndex = transportBuffer->head;

	transportBuffer->transportRequests[transportBuffer->head] = transportReq;
	transportBuffer->head = (transportBuffer->head + 1) % NTBUFFER_MAX;

	//Each TransportRequest will have a thread to assign a taxi to the request, belonging to a passenger
	CreateThread(
		NULL,					//Security Attributes
		0,						//Stack Size (0 = default)
		Thread_TaxiAssignment,	//Function
		(LPVOID) param,			//Param
		0,						//Creation flags
		NULL);					//Thread Id

	/*Notify all taxis that a new transportRequest has been registered
	*/
	Service_NotifyTaxisNewTransport(app);

	return true;
}
TransportInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest){
	if(ntiRequest == NULL || Utils_StringIsEmpty(ntiRequest->idPassenger))
		return NTIR_INVALID_UNDEFINED;

	int transportIndex = Get_TransportIndex(app, ntiRequest->idPassenger);
	int taxiIndex = Get_TaxiIndex(app, ntiRequest->licensePlate);

	if(taxiIndex == -1)//Taxi received is invalid (not supposed to ever happen!)
		return NTIR_INVALID_UNDEFINED;

	if(transportIndex == -1)//Transport doesn't exist
		return NTIR_INVALID_ID;

	if(WaitForSingleObject(app->transportList[transportIndex].cpThreadHandles.hTaxiAssignment, 0) == WAIT_OBJECT_0)
		return NTIR_INVALID_CLOSED;
	
	for(int i = 0; i < app->maxTaxis; i++){
		if(app->transportList[transportIndex].interestedTaxis[i] == -1){
			app->transportList[transportIndex].interestedTaxis[i] = taxiIndex;
			break;
		}
	}

	_tprintf(TEXT("%s[ConPass] Taxi (%s) has shown interest to transport Passenger (%s)"), 
		Utils_NewSubLine(),
		Get_Taxi(app, taxiIndex)->taxiInfo.LicensePlate,
		Get_TransportRequest(app, transportIndex).passId);

	return NTIR_SUCCESS;
}
void Service_NotifyTaxisNewTransport(Application* app){
	//Manual reset event
	SetEvent(app->syncHandles.hEvent_Notify_T_NewTranspReq);
	ResetEvent(app->syncHandles.hEvent_Notify_T_NewTranspReq);
	//OR
	//Auto reset event (tested, and no flaws found)
	/*for(int i = 0; i < Get_QuantLoggedInTaxis(app); i++)
		SetEvent(app->syncHandles.hEvent_Notify_T_NP);*/
}
void Service_AssignTaxi2Passenger(Application* app, int taxiIndex, int transportIndex){
	//First: check if transport index is valid, if not, the transport request cannot be accessed (never supposed to happen)
	if(transportIndex < 0 || transportIndex >= NTBUFFER_MAX){
		_tprintf(TEXT("%s[Taxi Assignment] This wasn't supposed to happen! Transport Request cannot be accessed on Service_AssignTaxi2Passenger at CenService.c"), Utils_NewLine());
	} else{
		TransportBuffer* shmBuffer = app->shmHandles.lpSHM_NTBuffer;//List that contains information about the transport request
		TransportRequest* myRequestInfo = &shmBuffer->transportRequests[transportIndex];//Item of list that contains information about the transport request

		if(taxiIndex == -1){
			_tprintf(TEXT("%s[Taxi Assignment] Communicating with ConPass informing that Passenger %s doesn't have any taxi assigned... Also logout passenger..."), Utils_NewSubLine(), myRequestInfo->passId);

			Service_NotifyPassenger(app, myRequestInfo, -1/*-1 Means that it failed*/);
			Delete_Passenger(app, Get_PassengerIndex(app, myRequestInfo->passId));
		} else if(taxiIndex < 0 ||
			taxiIndex >= app->maxTaxis ||
			app->taxiList[taxiIndex].taxiInfo.empty ||
			myRequestInfo->empty){
			_tprintf(TEXT("%s[Taxi Assignment] Cannot notify both parties since one of them seems to be invalid!"), Utils_NewSubLine());
		} else{
			Service_NotifyTaxi(app, myRequestInfo, taxiIndex);
			Service_NotifyPassenger(app, myRequestInfo, taxiIndex);
		}

		/*ZeroMemory on the assigned request
		**Only zero memory inside this condition since it can only access the request if index is valid
		*/
		ZeroMemory(myRequestInfo, sizeof(TransportRequest));
		myRequestInfo->empty = true;
	}
}
void Service_NotifyTaxi(Application* app, TransportRequest* myRequestInfo, int taxiIndex){
	CommsC2T sendTaxiNotification;
	CommsC2T_Assign assignTaxiComms;

	_tcscpy_s(assignTaxiComms.passId, _countof(assignTaxiComms.passId), myRequestInfo->passId);
	assignTaxiComms.xyStartingPosition = myRequestInfo->xyStartingPosition;
	assignTaxiComms.xyDestination = myRequestInfo->xyDestination;

	sendTaxiNotification.assignComm = assignTaxiComms;
	sendTaxiNotification.commType = C2T_ASSIGNED;

	WriteFile(
		app->taxiList[taxiIndex].taxiNamedPipe,	//Named pipe handle
		&sendTaxiNotification,					//Write from 
		sizeof(CommsC2T),						//Size being written
		NULL,									//Quantity Bytes written
		NULL);									//Overlapped IO
}

void Service_NotifyPassenger(Application* app, TransportRequest* myRequestInfo, int taxiIndex){
	CommsC2P sendPassNotification;
	CommsC2P_Assign assignPassComms;

	/*IF no taxi sent interest for transport request
	**THEN Service_NotifyPassenger will received -1 as taxiIndex
	**THEN instead of sending C2P_ASSIGNED it sends C2P_ASSIGNED_FAILED
	**MEANING ConPass will receive a notification saying that the passenger was not assigned to any taxi
	*/
	if(taxiIndex == -1)
		sendPassNotification.commType = C2P_ASSIGNED_FAILED;
	else{
		sendPassNotification.commType = C2P_ASSIGNED;
		_tcscpy_s(assignPassComms.licensePlate, _countof(assignPassComms.licensePlate), app->taxiList[taxiIndex].taxiInfo.LicensePlate);
	}
	_tcscpy_s(assignPassComms.passId, _countof(assignPassComms.passId), myRequestInfo->passId);
	
	/*ToDo (TAG_TODO)
	**Calculate estimated time for taxi to arrive to passenger and set to var
	**assignPassComms.estimatedWaitTime = X;
	*/assignPassComms.estimatedWaitTime = 420;
	sendPassNotification.assignComm = assignPassComms;

	WriteFile(
		app->namedPipeHandles.hCPWrite,	//Named pipe handle
		&sendPassNotification,			//Write from 
		sizeof(CommsC2P),				//Size being written
		NULL,							//Quantity Bytes written
		NULL);							//Overlapped IO
}

bool Service_KickTaxi(Application* app, TCHAR* licensePlate, TCHAR* reason, bool global){
	if(Utils_StringIsEmpty(licensePlate))
		return false;

	int taxiIndex = Get_TaxiIndex(app, licensePlate);
	if(taxiIndex == -1)
		return false;

	CenTaxi* anchorTaxi = Get_Taxi(app, taxiIndex);
	if(anchorTaxi == NULL)
		return false;

	//Doesn't allow taxis carrying passengers to be kicked
	if(anchorTaxi->taxiInfo.state == TS_WITH_PASS)
		return false;

	CommsC2T sendNotification;
	CommsC2T_Shutdown shutdownComms;
	
	if(Utils_StringIsEmpty(reason))
		_tcscpy_s(shutdownComms.message, _countof(shutdownComms.message), TEXT("Undefined reason..."));
	else
		_tcscpy_s(shutdownComms.message, _countof(shutdownComms.message), reason);

	if(global)
		shutdownComms.shutdownType = ST_GLOBAL;
	else
		shutdownComms.shutdownType = ST_KICKED;
	
	sendNotification.shutdownComm = shutdownComms;
	sendNotification.commType = C2T_SHUTDOWN;

	WriteFile(
		anchorTaxi->taxiNamedPipe,	//Named pipe handle
		&sendNotification,			//Write from 
		sizeof(CommsC2T),			//Size being written
		NULL,						//Quantity Bytes written
		NULL);						//Overlapped IO

	Delete_Taxi(app, taxiIndex);
	return true;
}
void Service_CloseApp(Application* app){
	app->keepRunning = false;
	
	for(int i = 0; i < app->maxTaxis; i++){
		if(app->taxiList[i].taxiInfo.empty)
			continue;

		Service_KickTaxi(app, app->taxiList[i].taxiInfo.LicensePlate, SHUTDOWN_REASON_Global, true);
	}

	CommsC2P sendPassNotification;
	CommsC2P_Shutdown shutdownPassComms;
	_tcscpy_s(shutdownPassComms.message, _countof(shutdownPassComms.message), SHUTDOWN_REASON_Global);
	sendPassNotification.shutdownComm = shutdownPassComms;
	sendPassNotification.commType = C2P_SHUTDOWN;

	WriteFile(
		app->namedPipeHandles.hCPWrite,	//Named pipe handle
		&sendPassNotification,			//Write from 
		sizeof(CommsC2P),				//Size being written
		NULL,							//Quantity Bytes written
		NULL);							//Overlapped IO

	Setup_CloseAllHandles(app);
	/*ToDo (TAG_TODO)
	**Close threads and handles properly
	*/
}

bool Command_SetAssignmentTimeout(Application* app, TCHAR* value){
	if(!Utils_StringIsNumber(value))
		return false;

	int timeoutValue = _ttoi(value);
	if(timeoutValue <= 0 || timeoutValue > TOPMAX_ASSIGNMENT_TIMEOUT){
		_tprintf(TEXT("%sAssignment timeout value has to be between 1 and %d! Hence, it remained the same (%d)..."), Utils_NewSubLine(), TOPMAX_ASSIGNMENT_TIMEOUT, app->settings.secAssignmentTimeout);
		return true;
	}

	app->settings.secAssignmentTimeout = timeoutValue;
	_tprintf(TEXT("%sAssignment timeout value has been changed to %d!"), Utils_NewSubLine(), app->settings.secAssignmentTimeout);
	return true;
}
void Command_AllowTaxiLogins(Application* app, bool allow){
	app->settings.allowTaxiLogins = allow;

	if(app->settings.allowTaxiLogins)
		_tprintf(TEXT("%sTaxi logins are now allowed!"), Utils_NewSubLine());
	else
		_tprintf(TEXT("%sTaxi logins are now being denied!"), Utils_NewSubLine());
}

/* ToDo (TAG_REMOVE)
** Remove the following after
** Only used to develop and test few features
*/

void Temp_ShowMap(Application* app){
	int iLine = 0;
	int iColumn = 0;
	for(int i = 0; i < (app->map.height * app->map.width); i++){
		iColumn = i % app->map.width;
		iLine = i / app->map.height;

		if(iColumn == 0)
			_tprintf(TEXT("\n"));

		XY tempPos;
		tempPos.x = iColumn;
		tempPos.y = iLine;
		CenTaxi* taxiFound = Get_TaxiAt(app, tempPos);
		if(taxiFound != NULL){
			_tprintf(TEXT("T"));
			continue;
		}

		_tprintf(TEXT("%c"), app->map.cellArray[i]);
	}
}

void Temp_SaveRegistry(Application* app){
	HKEY hRegKey;
	DWORD createState;

	LSTATUS createRet = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		TEXT("Software\\SOTaxi"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hRegKey,
		&createState);

	if(createRet != ERROR_SUCCESS){
		_tprintf(TEXT("%sError trying to open RegistryKey..."), Utils_NewLine());
		return;
	}

	TCHAR valueA[STRING_SMALL] = TEXT("ValAAA");
	TCHAR valueB[STRING_SMALL] = TEXT("ValBBB");
	DWORD size = STRING_SMALL * sizeof(TCHAR);
	RegSetValueEx(hRegKey, TEXT("ValueA"), 0, REG_SZ, (LPBYTE) valueA, STRING_SMALL * sizeof(TCHAR));
	RegSetValueEx(hRegKey, TEXT("ValueB"), 0, REG_SZ, (LPBYTE) valueB, STRING_SMALL * sizeof(TCHAR));
	_tprintf(TEXT("%sSaved ValueA:%s and ValueB:%s into registry"), Utils_NewSubLine(), valueA, valueB);

	RegCloseKey(hRegKey);
}

void Temp_LoadRegistry(Application* app){
	HKEY hRegKey;
	DWORD createState;

	LSTATUS createRet = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		TEXT("Software\\SOTaxi"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hRegKey,
		&createState);

	if(createRet != ERROR_SUCCESS){
		_tprintf(TEXT("%sError trying to open RegistryKey..."), Utils_NewLine());
		return;
	}

	if(createState == REG_OPENED_EXISTING_KEY){
		TCHAR valueA[STRING_SMALL];
		ZeroMemory(valueA, STRING_SMALL * sizeof(TCHAR));
		TCHAR valueB[STRING_SMALL];
		ZeroMemory(valueB, STRING_SMALL * sizeof(TCHAR));
		DWORD size = STRING_SMALL * sizeof(TCHAR);
		RegQueryValueEx(hRegKey, TEXT("ValueA"), NULL, NULL, (LPBYTE) valueA, &size);
		RegQueryValueEx(hRegKey, TEXT("ValueB"), NULL, NULL, (LPBYTE) valueB, &size);
		_tprintf(TEXT("%sLoaded ValueA:%s and ValueB:%s from registry"), Utils_NewSubLine(), valueA, valueB);
	} else{
		_tprintf(TEXT("%sNo registry key found"), Utils_NewSubLine());
	}

	RegCloseKey(hRegKey);

}

void Temp_CreatePath(Application* app){
	//TCHAR xAt[3];
	//swprintf(xAt, 3, TEXT("%d%d"),
	//	rand()%50,
	//	rand()%50);
	//TCHAR yAt[3];
	//swprintf(yAt, 3, TEXT("%d%d"),
	//	rand()%50,
	//	rand()%50);
	//TCHAR xDestiny[3];
	//swprintf(xDestiny, 3, TEXT("%d%d"),
	//	rand()%50,
	//	rand()%50);
	//TCHAR yDestiny[3];
	//swprintf(yDestiny, 3, TEXT("%d%d"),
	//	rand()%50,
	//	rand()%50);

	XY xyStartingPosition;
	XY xyDestination;

	//xyStartingPosition.x = _ttoi(xAt);
	//xyStartingPosition.y = _ttoi(yAt);
	//xyDestination.x = _ttoi(xDestiny);
	//xyDestination.y = _ttoi(yDestiny);

	xyStartingPosition.x = 1;
	xyStartingPosition.y = 1;
	xyDestination.x = 48;
	xyDestination.y = 48;

	Path* path = Utils_GetPath(&app->map, xyStartingPosition, xyDestination);

	if(path == NULL){
		_tprintf(TEXT("%sFailed to get a path from (%.2lf, %.2lf) to (%.2lf, %.2lf)"), 
			Utils_NewSubLine(), 
			xyStartingPosition.x,
			xyStartingPosition.y,
			xyDestination.x,
			xyDestination.y);

		return;
	}


	_tprintf(TEXT("%sReceived following path: "), Utils_NewSubLine());
	for(int i = 0; i < path->steps; i++){
		_tprintf(TEXT("%sStep %d: Goes to (%.2lf, %.2lf)"), Utils_NewSubLine(), i, path->path[i].x, path->path[i].y);
	}
}