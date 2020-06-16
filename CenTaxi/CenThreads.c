#pragma once
#include "CenThreads.h"

DWORD WINAPI Thread_ReceiveQnARequests(LPVOID _param){
	TParam_QnARequest* param = (TParam_QnARequest*) _param;

	while(param->app->keepRunning){
		WaitForSingleObject(param->app->syncHandles.hEvent_QnARequest_Read, INFINITE);

		QnARequest* shm = param->app->shmHandles.lpSHM_QnARequest;

		switch(shm->requestType){
			case QnART_LOGIN:
			{
				TCHAR log[STRING_XXL];
				swprintf(log, STRING_XXL, TEXT("ConTaxi sent a request to CenTaxi to Login, sending: LicensePlate: %s | X: %.2lf | Y: %.2lf"), shm->taxiLoginRequest.licensePlate, shm->taxiLoginRequest.coordX, shm->taxiLoginRequest.coordY);
				Utils_DLL_Log(log);

				shm->taxiLoginResponse.taxiLoginResponseType = Service_LoginTaxi(param->app, &shm->taxiLoginRequest);

				if(shm->taxiLoginResponse.taxiLoginResponseType == TLR_INVALID_FULL){
					ResetEvent(param->app->syncHandles.hEvent_NewTaxiSpot);
				}

				shm->taxiLoginResponse.mapWidth = param->app->map.width;
				shm->taxiLoginResponse.mapHeight = param->app->map.height;
			}
				break;
			case QnART_NT_INTEREST:
			{
				TCHAR log[STRING_XXL];
				swprintf(log, STRING_XXL, TEXT("ConTaxi sent a request to CenTaxi of Interest in a transport request, sending: LicensePlate: %s | PassengerId: %s"), shm->ntIntRequest.licensePlate, shm->ntIntRequest.idPassenger);
				Utils_DLL_Log(log);
				shm->ntIntResponse = Service_RegisterInterest(param->app, &shm->ntIntRequest);
			}
				break;
		}

		SetEvent(param->app->syncHandles.hEvent_QnARequest_Write);
	}

	free(param);
	return 1;
}

DWORD WINAPI Thread_TaxiAssignment(LPVOID _param){
	TParam_TaxiAssignment* param = (TParam_TaxiAssignment*) _param;

	CenTransportRequest* cenBuffer = param->app->transportList;//List that contains information about interested taxis
	TransportBuffer* shmBuffer = param->app->shmHandles.lpSHM_NTBuffer;//List that contains information about the transport request

	CenTransportRequest* myRequestTech = &cenBuffer[param->myIndex];//Item of list that contains information about interested taxis
	TransportRequest* myRequestInfo = &shmBuffer->transportRequests[param->myIndex];//Item of list that contains information about the transport request

	HANDLE hAssignTimeout = CreateWaitableTimer(
		NULL,	//Security Attributes
		TRUE,	//Manual Reset
		NULL	//WaitableTimer name
	);
	Utils_DLL_Register(TEXT("UnnamedWTimer:line41:cenThreads.c"), DLL_TYPE_WAITABLETIMER);
	if(hAssignTimeout == NULL)
		return -1;

	LARGE_INTEGER liTime;
	liTime.QuadPart = -10000000LL * param->app->settings.secAssignmentTimeout;

	SetWaitableTimer(
		hAssignTimeout,
		&liTime,
		0,
		NULL,
		NULL,
		FALSE
	);

	WaitForSingleObject(hAssignTimeout, INFINITE);

	int numIntTaxis = 0;
	for(int i = 0; i < param->app->maxTaxis; i++){
		if(myRequestTech->interestedTaxis[i] != -1){
			numIntTaxis++;
		}
	}

	_tprintf(TEXT("%s[Taxi Assignment] Analysis:"), Utils_NewSubLine(), myRequestInfo->passId);
	if(numIntTaxis == 0){//No interested taxis
		_tprintf(TEXT("%s\tNo taxi has shown interest towards %s's transport!"), Utils_NewSubLine(), myRequestInfo->passId);
		return 1;
	}

	_tprintf(TEXT("%s\tQuantity of interested taxis: %d"), Utils_NewSubLine(), numIntTaxis);
	int chosenTaxi = (rand() % numIntTaxis);

	_tprintf(TEXT("%s\tChosen taxi: %s"), Utils_NewSubLine(), Get_Taxi(param->app, myRequestTech->interestedTaxis[chosenTaxi])->taxiInfo.LicensePlate);
	Service_AssignTaxi2Passenger(param->app, myRequestTech->interestedTaxis[chosenTaxi], param->myIndex);
	
	free(param);
	return 1;
}

DWORD WINAPI Thread_ConsumeTossRequests(LPVOID _param){
	TParam_ConsumeTossRequests* param = (TParam_ConsumeTossRequests*) _param;
	TossRequestsBuffer* buffer = (TossRequestsBuffer*) param->app->shmHandles.lpSHM_TossReqBuffer;

	while(param->app->keepRunning){
		WaitForSingleObject(param->app->syncHandles.hSemaphore_HasTossRequest, INFINITE);

		if(buffer->tail != buffer->head){
			buffer->tossRequests[buffer->tail];
			switch(buffer->tossRequests[buffer->tail].tossType){
				case TRT_TAXI_POSITION:
					{
						TCHAR log[STRING_XXL];
						swprintf(log, STRING_XXL, TEXT("ConTaxi sent a toss request to CenTaxi of TaxiPosition, sending: LicensePlate: %s | NewX: %.2lf | NewY: %.2lf"),
							buffer->tossRequests[buffer->tail].tossPosition.licensePlate,
							buffer->tossRequests[buffer->tail].tossPosition.newX,
							buffer->tossRequests[buffer->tail].tossPosition.newY);
						Utils_DLL_Log(log);

						CenTaxi* updatingTaxi = Get_Taxi(param->app, Get_TaxiIndex(param->app, buffer->tossRequests[buffer->tail].tossPosition.licensePlate));
						
						if(updatingTaxi != NULL){
							updatingTaxi->taxiInfo.object.coordX = buffer->tossRequests[buffer->tail].tossPosition.newX;
							updatingTaxi->taxiInfo.object.coordY = buffer->tossRequests[buffer->tail].tossPosition.newY;
						}
					}
					break;
				case TRT_TAXI_STATE:
				{
					TCHAR state[STRING_MEDIUM];
					switch(buffer->tossRequests[buffer->tail].tossState.newState){
					case TS_EMPTY:
						_tcscpy_s(state, _countof(state), TEXT("Empty"));
						break;
					case TS_OTW_PASS:
						_tcscpy_s(state, _countof(state), TEXT("PickingUpPassenger"));
						break;
					case TS_WITH_PASS:
						_tcscpy_s(state, _countof(state), TEXT("WithPassenger"));
						break;
					case TS_STATIONARY:
						_tcscpy_s(state, _countof(state), TEXT("Stationary"));
						break;
					default:
						return -1;
					}
					TCHAR log[STRING_XXL];
					swprintf(log, STRING_XXL, TEXT("ConTaxi sent a toss request to CenTaxi of TaxiPosition, sending: LicensePlate: %s | NewStateType: %d (%s)"),
						buffer->tossRequests[buffer->tail].tossState.licensePlate,
						buffer->tossRequests[buffer->tail].tossState.newState,
						state);
					Utils_DLL_Log(log);
				}
					break;
				case TRT_TAXI_LOGOUT:
				{
					TCHAR log[STRING_XXL];
					swprintf(log, STRING_XXL, TEXT("ConTaxi sent a toss request to CenTaxi of TaxiLogout, informing central about %s logout"),
						buffer->tossRequests[buffer->tail].tossPosition.licensePlate);
					Utils_DLL_Log(log);

					Delete_Taxi(param->app, Get_TaxiIndex(param->app, buffer->tossRequests[buffer->tail].tossPosition.licensePlate));
				}
				break;
			}
			buffer->tail = (buffer->tail + 1) % TOSSBUFFER_MAX;
		}
	}

	free(param);
	return 1;
}

DWORD WINAPI Thread_ConnectingTaxiPipes(LPVOID _param){
	TParam_ConnectingTaxiPipes* param = (TParam_ConnectingTaxiPipes*) _param;

	HANDLE hPipe;
	while(param->app->keepRunning){
		WaitForSingleObject(param->app->syncHandles.hSemaphore_TaxiNPSpots, INFINITE);

		hPipe = CreateNamedPipe(
			NAME_NAMEDPIPE_CommsC2T,	//Named Pipe name
			PIPE_ACCESS_DUPLEX,					//Access to read and write
			PIPE_TYPE_MESSAGE |					//Message type
			PIPE_WAIT,							//Blocking mode
			param->app->maxTaxis,				//Max instances
			sizeof(CommsT2C),					//Buffer size of output
			sizeof(CommsT2C_Identity),			//Buffer size of input
			0,									//Client timeout
			NULL);								//Security attributes

		//If the CreateNamedPipe return invalid handle check if busy or unexpected error
		if(hPipe == INVALID_HANDLE_VALUE){
			if(GetLastError() == ERROR_PIPE_BUSY){
				_tprintf(TEXT("%sCentral doesn't have more empty slots for taxis! Try again later..."), Utils_NewSubLine());
				continue;
			} else
				_tprintf(TEXT("%sCreateNamedPipe failed! Error: %d"), Utils_NewSubLine(), GetLastError());
			return -1;
		}

		//The taxi connection failed, hence, closing the pipe
		if(!ConnectNamedPipe(hPipe, NULL)){
			ReleaseSemaphore(param->app->syncHandles.hSemaphore_TaxiNPSpots, 1, NULL);
			CloseHandle(hPipe);
		}

		CommsT2C_Identity taxiIdentity;
		ReadFile(
			hPipe,					//Named pipe handle
			&taxiIdentity,			//Read into
			sizeof(CommsT2C_Identity), //Size being read
			NULL,					//Quantity of bytes read
			NULL);					//Overlapped IO

		int taxiIndex = Get_TaxiIndex(param->app, taxiIdentity.licensePlate);
		if(taxiIndex == -1){ //Taxi is not listed on the central
			Utils_CloseNamedPipe(hPipe);
			_tprintf(TEXT("%sA non logged in taxi (%s) tried to connect"), Utils_NewSubLine(), taxiIdentity.licensePlate);
			return -1;
		}

		param->app->taxiList[taxiIndex].taxiNamedPipe = hPipe;
	}

	free(param);
	return 1;
}

DWORD WINAPI Thread_ReadConPassNPQnA(LPVOID _param){
	TParam_ReadConPassNPQnA* param = (TParam_ReadConPassNPQnA*) _param;

	//Waiting for a ConPass to connect
	ConnectNamedPipe(param->app->namedPipeHandles.hCPQnA, NULL);

	_tprintf(TEXT("%s[ConPass] Connected to QnA!"), Utils_NewSubLine());
	CommsP2C receivedComm;
	CommsC2P sendResponse;
	while(param->app->keepRunning){
		ReadFile(param->app->namedPipeHandles.hCPQnA,	//Named pipe handle
			&receivedComm,							//Read into
			sizeof(CommsP2C),						//Size being read
			NULL,									//Quantity of bytes read
			NULL);									//Overlapped IO

		switch(receivedComm.commType){
			case P2C_LOGIN:
				sendResponse.loginRespComm = Service_LoginPass(param->app, &receivedComm.loginComm);
				sendResponse.commType = C2P_RESP_LOGIN;
				break;
			case P2C_REQMAXPASS:
				sendResponse.maxPassRespComm.maxPass = param->app->maxPassengers;
				sendResponse.commType = C2P_RESP_MAXPASS;
				break;
			default:
				_tprintf(TEXT("%s[ConPass] QnA - Wtf is this"), Utils_NewSubLine());
		}

		WriteFile(
			param->app->namedPipeHandles.hCPQnA,//Named pipe handle
			&sendResponse,						//Write from 
			sizeof(CommsC2P),					//Size being written
			NULL,								//Quantity Bytes written
			NULL);								//Overlapped IO
	}

	free(param);
	return 1;
}

DWORD WINAPI Thread_ReadConPassNPToss(LPVOID _param){
	TParam_ReadConPassNPToss* param = (TParam_ReadConPassNPToss*) _param;

	//Waiting for a ConPass to connect
	ConnectNamedPipe(param->app->namedPipeHandles.hCPRead, NULL);

	//Waiting for a ConPass to connect
	ConnectNamedPipe(param->app->namedPipeHandles.hCPWrite, NULL);

	_tprintf(TEXT("%s[ConPass] Connected to Toss!"), Utils_NewSubLine());

	CommsP2C receivedComm;
	while(param->app->keepRunning){
		ReadFile(
			param->app->namedPipeHandles.hCPRead,	//Named pipe handle
			&receivedComm,							//Read into
			sizeof(CommsP2C),						//Size being read
			NULL,									//Quantity of bytes read
			NULL);									//Overlapped IO

		switch(receivedComm.commType){
			default:
				_tprintf(TEXT("%s[ConPass] Lost connection... Disconnecting!"), Utils_NewSubLine());
			case P2C_DISCONNECT:
				_tprintf(TEXT("%s[ConPass] Disconnected!"), Utils_NewSubLine());
				Utils_CloseNamedPipe(param->app->namedPipeHandles.hCPQnA);
				Utils_CloseNamedPipe(param->app->namedPipeHandles.hCPRead);
				Utils_CloseNamedPipe(param->app->namedPipeHandles.hCPWrite);

				/*Both QnA and Toss Threads have no use now and will end
				*/
				DWORD exitCode;
				GetExitCodeThread(param->app->threadHandles.hReadConPassNPQnA, &exitCode);
				ExitThread(exitCode);
				return 1;
			
		}
	}
	_tprintf(TEXT("%sClosing Thread"), Utils_NewSubLine());

	free(param);
	return 1;
}