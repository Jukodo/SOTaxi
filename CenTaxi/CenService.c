#pragma once
#include "CenService.h"
#include "CenCommunication.h"

bool Setup_Application(Application* app, int maxTaxis, int maxPassengers){
	ZeroMemory(app, sizeof(Application));
	srand((unsigned int) time(NULL));
	app->settings.secAssignmentTimeout = DEFAULT_ASSIGNMENT_TIMEOUT;
	app->settings.allowTaxiLogins = DEFAULT_ALLOW_TAXI_LOGINS;

	app->maxTaxis = maxTaxis;
	app->maxPassengers = maxPassengers;

	app->taxiList = malloc(maxTaxis * sizeof(Taxi));
	app->passengerList = malloc(maxPassengers * sizeof(CenPassenger));
	
	int i;
	for(i = 0; i < maxTaxis; i++){
		ZeroMemory(&app->taxiList[i], sizeof(Taxi));
		app->taxiList[i].taxiInfo.empty = true;
	}

	for(i = 0; i < maxPassengers; i++){
		ZeroMemory(&app->passengerList[i], sizeof(CenPassenger));
		app->passengerList[i].passengerInfo.empty = true;

		app->passengerList[i].interestedTaxis = malloc(maxTaxis * sizeof(int));
		for(int a = 0; a < maxTaxis; a++){
			app->passengerList[i].interestedTaxis[a] = -1;
		}
	}

	bool ret = true;
	ret = ret && (app->passengerList != NULL);
	ret = ret && (app->taxiList != NULL); 
	ret = ret && Setup_OpenSyncHandles(&app->syncHandles);
	ret = ret && Setup_OpenShmHandles(app);
	ret = ret && Setup_OpenMap(app);
	ret = ret && Setup_OpenThreadHandles(app); //Has to be called at the end, because it will use Sync and SMH

	return ret;
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

	return !(app->threadHandles.hQnARequests == NULL ||
		app->threadHandles.hTossRequests == NULL ||
		app->threadHandles.hConnectingTaxiPipes == NULL);
}

bool Setup_OpenSyncHandles(SyncHandles* syncHandles){
	syncHandles->hEvent_QnARequest_Read = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		FALSE,						//Initial State
		NAME_EVENT_QnARequest_Read	//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_QnARequest_Read, DLL_TYPE_EVENT);
	syncHandles->hEvent_QnARequest_Write = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		TRUE,						//Initial State
		NAME_EVENT_QnARequest_Write	//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_QnARequest_Write, DLL_TYPE_EVENT);

	syncHandles->hEvent_Notify_T_NewTranspReq = CreateEvent(
		NULL,							//Security Attributes
		TRUE,							//Manual Reset
		FALSE,							//Initial State
		NAME_EVENT_NewTransportRequest	//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_NewTransportRequest, DLL_TYPE_EVENT);

	syncHandles->hEvent_TaxiLoggingIn = CreateEvent(
		NULL,						//Security Attributes
		FALSE,						//Manual Reset
		FALSE,						//Initial State
		NAME_EVENT_TaxiLoggingIn	//Event Name
	);
	Utils_DLL_Register(NAME_EVENT_TaxiLoggingIn, DLL_TYPE_EVENT);

	syncHandles->hSemaphore_HasTossRequest = CreateSemaphore(
		NULL,							//Security Attributes
		0,								//Initial Count
		TOSSBUFFER_MAX,					//Max Count
		NAME_SEMAPHORE_HasTossRequest	//Semaphore Name
	);
	Utils_DLL_Register(NAME_SEMAPHORE_HasTossRequest, DLL_TYPE_SEMAPHORE);

	return !(syncHandles->hEvent_QnARequest_Read == NULL ||
		syncHandles->hEvent_QnARequest_Write == NULL ||
		syncHandles->hEvent_Notify_T_NewTranspReq == NULL ||
		syncHandles->hSemaphore_HasTossRequest == NULL);
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
		sizeof(NewTransportBuffer),		//DWORD low-order max size
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
		sizeof(NewTransportBuffer)		//Number of bytes to map
	);
	Utils_DLL_Register(NAME_SHM_TransportRequestBuffer, DLL_TYPE_MAPVIEWOFFILE);
	if(app->shmHandles.lpSHM_NTBuffer == NULL)
		return false;

	ZeroMemory(app->shmHandles.lpSHM_NTBuffer, sizeof(NewTransportBuffer)); //Makes sure head starts at 0 (unecessary to zero everything)
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
	Setup_CloseSyncHandles(&app->syncHandles);
	Setup_CloseShmHandles(&app->shmHandles);
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

bool isTaxiListFull(Application* app){
	return Get_QuantLoggedInTaxis(app) >= app->maxTaxis;
}

int Get_QuantLoggedInTaxis(Application* app){
	int quantLoggedInTaxis = 0;

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].taxiInfo.empty)
			quantLoggedInTaxis++;
	}

	return quantLoggedInTaxis;
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

Taxi* Get_Taxi(Application* app, int index){
	if(app->taxiList == NULL)
		return NULL;

	if(!app->taxiList[index].taxiInfo.empty)
		return &app->taxiList[index];

	return NULL;
}

Taxi* Get_TaxiAt(Application* app, int coordX, int coordY){
	if(app->taxiList == NULL)
		return NULL;

	for(int i = 0; i < app->maxTaxis; i++){
		if(!app->taxiList[i].taxiInfo.empty &&
			((int) app->taxiList[i].taxiInfo.object.coordX) == coordX &&
			((int) app->taxiList[i].taxiInfo.object.coordY) == coordY)
			return &app->taxiList[i];
	}

	return NULL;
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

bool isValid_ObjectPosition(Application* app, float coordX, float coordY){
	//ToDo

	return true;
}

bool Service_NewPassenger(Application* app, Passenger pass){
	if(isPassengerListFull(app))
		return false;

	int freeIndex = Get_FreeIndexPassengerList(app);
	if(freeIndex == -1)
		return false;

	app->passengerList[freeIndex].passengerInfo = pass;

	TParam_TaxiAssignment* param = (TParam_TaxiAssignment*) malloc(sizeof(TParam_TaxiAssignment));
	param->app = app;
	param->myIndex = freeIndex;
	app->passengerList[freeIndex].cpThreadHandles.hTaxiAssignment = CreateThread(//Each passenger will 
		NULL,															 //Security Attributes
		0,																 //Stack Size (0 = default)
		Thread_TaxiAssignment,											 //Function
		(LPVOID) param,													 //Param
		0,																 //Creation flags
		&app->passengerList[freeIndex].cpThreadHandles.dwIdTaxiAssignment//Thread Id
	);

	NewTransportBuffer* transportBuffer = (NewTransportBuffer*) app->shmHandles.lpSHM_NTBuffer;
	transportBuffer->transportRequests[transportBuffer->head] = pass;
	transportBuffer->head = (transportBuffer->head + 1) % NTBUFFER_MAX;
	Service_NotifyTaxisNewTransport(app);

	return true;
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

CentralCommands Service_UseCommand(Application* app, TCHAR* command){
	if(_tcscmp(command, CMD_HELP) == 0){ //Continues on Main (listing commands)
		return CC_HELP;
	} else if(_tcscmp(command, CMD_LIST_TAXIS) == 0){ //Continues on Main (listing logged in taxis)
		return CC_LIST_TAXIS;
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
	} else if(_tcscmp(command, CMD_SIMULATE_NTR) == 0){ //Continues on Main (asking for value argument)
		Simulate_NewTransport(app);
		return CC_SIMULATE_NTR;
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
	} else if(_tcscmp(command, CMD_CLOSEAPP) == 0){
		Service_CloseApp(app);
		return CC_CLOSEAPP;
	}

	return CC_UNDEFINED;
}

LoginResponseType Service_LoginTaxi(Application* app, LoginRequest* loginRequest){
	if(loginRequest == NULL || Utils_StringIsEmpty(loginRequest->licensePlate))
		return LR_INVALID_UNDEFINED;

	if(!app->settings.allowTaxiLogins)
		return LR_INVALID_CLOSED;

	if(isTaxiListFull(app))
		return LR_INVALID_FULL;

	if(!isValid_ObjectPosition(app, loginRequest->coordX, loginRequest->coordY))
		return LR_INVALID_POSITION;

	if(Get_TaxiIndex(app, loginRequest->licensePlate) != -1)
		return LR_INVALID_EXISTS;

	Taxi* anchorTaxi = &app->taxiList[Get_FreeIndexTaxiList(app)];
	anchorTaxi->empty = false;
	_tcscpy_s(anchorTaxi->LicensePlate, _countof(anchorTaxi->LicensePlate), loginRequest->licensePlate);
	anchorTaxi->object.coordX = loginRequest->coordX;
	anchorTaxi->object.coordY = loginRequest->coordY;

	return LR_SUCCESS;
}

NTInterestResponse Service_RegisterInterest(Application* app, NTInterestRequest* ntiRequest){
	if(ntiRequest == NULL || Utils_StringIsEmpty(ntiRequest->idPassenger))
		return NTIR_INVALID_UNDEFINED;

	int passIndex = Get_PassengerIndex(app, ntiRequest->idPassenger);
	int taxiIndex = Get_TaxiIndex(app, ntiRequest->licensePlate);

	if(taxiIndex == -1)//Taxi received is invalid (not supposed to ever happen!)
		return NTIR_INVALID_UNDEFINED;

	if(passIndex == -1)//Passenger doesn't exist
		return NTIR_INVALID_ID;

	if(WaitForSingleObject(app->passengerList[passIndex].cpThreadHandles.hTaxiAssignment, 0) == WAIT_OBJECT_0)
		return NTIR_INVALID_CLOSED;
	
	for(int i = 0; i < app->maxTaxis; i++){
		if(app->passengerList[passIndex].interestedTaxis[i] == -1){
			app->passengerList[passIndex].interestedTaxis[i] = taxiIndex;
			break;
		}
	}
	return NTIR_SUCCESS;
}

bool Service_KickTaxi(Application* app, TCHAR* licensePlate){
	return false;
}

void Service_CloseApp(Application* app){
	/*ToDo (TAG_TODO)
	**Notify taxis and passengers about this shutdown, in order to close everything accordingly
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

void Simulate_NewTransport(Application* app){
	Passenger tempP;
	tempP.empty = false;
	tempP.object.coordX = 1;
	tempP.object.coordY = 1;

	_stprintf_s(tempP.Id, _countof(tempP.Id), TEXT("Pass%d"), rand() % 50 + 10);
	if(!Service_NewPassenger(app, tempP))
		_tprintf(TEXT("%sPassenger limit has been reached... This passenger will be ignored!"), Utils_NewSubLine());
}

void Temp_ShowMap(Application* app){
	int iLine = 0;
	int iColumn = 0;
	for(int i = 0; i < (app->map.height * app->map.width); i++){
		iColumn = i % app->map.width;
		iLine = i / app->map.height;

		if(iColumn == 0)
			_tprintf(TEXT("\n"));

		Taxi* taxiFound = Get_TaxiAt(app, iColumn, iLine);
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