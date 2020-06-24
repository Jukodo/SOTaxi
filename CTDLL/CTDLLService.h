#pragma once
#include "CTDLL.h"

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;
typedef struct StepRoutine StepRoutine;

typedef enum TaxiCommands TaxiCommands;

struct ThreadHandles{
	HANDLE hQnARequests;
	DWORD dwIdQnARequests;
	HANDLE hNotificationReceiver_NewTransport;
	DWORD dwIdNotificationReceiver_NewTransport;
	HANDLE hNotificationReceiver_NamedPipe;
	DWORD dwIdNotificationReceiver_NamedPipe;
	HANDLE hTossRequests;
	DWORD dwIdTossRequests;
	HANDLE hDestinationChanger;
	DWORD dwIdDestinationChanger;
};

struct SyncHandles{
	HANDLE hMutex_QnARequest_CanAccess;
	HANDLE hMutex_TossRequest_CanAccess;
	HANDLE hMutex_CommsTaxiCentral_CanAccess;
	HANDLE hMutex_StepRoutine;
	HANDLE hEvent_DestinationChanged;
	HANDLE hEvent_QnARequest_Read;
	HANDLE hEvent_QnARequest_Write;
	HANDLE hEvent_Notify_T_NewTranspReq;
	HANDLE hEvent_NewTaxiSpot;
	HANDLE hSemaphore_HasTossRequest;
};

struct ShmHandles{
	HANDLE hSHM_QnARequest;
	LPVOID lpSHM_QnARequest;
	HANDLE hSHM_NTBuffer;
	LPVOID lpSHM_NTBuffer;
	HANDLE hSHM_Map;
	LPVOID lpSHM_Map;
	HANDLE hSHM_TossReqBuffer;
	LPVOID lpSHM_TossReqBuffer;
};

struct StepRoutine{
	Path* path;
	double currentStep; //Double because each step can move by X.5
	HANDLE hStepRoutine;
	DWORD dwIdStepRoutine;
};

struct Application{
	Settings settings;
	CTTaxi loggedInTaxi;
	HANDLE taxiMovementRoutine;
	Map map;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	StepRoutine stepRoutine;
	int maxTaxis;
	int maxPassengers;
	int NTBuffer_Tail; //Used as a tail for the new transport request buffer (each taxi has a tail)
	bool keepRunning; //Used to identify if app should keep running, when FALSE threads that are infinitely looping will end
};

#define CMD_HELP TEXT("/help")
#define CMD_SPEED_UP TEXT("/speedUp")
#define CMD_SPEED_DOWN TEXT("/speedDown")
#define CMD_AUTORESP_ON TEXT("/autoRespOn")
#define CMD_AUTORESP_OFF TEXT("/autoRespOff")
#define CMD_DEFINE_CDN TEXT("/defineCDN")
#define CMD_REQUEST_INTEREST TEXT("/reqInterest")
#define CMD_SHOW_MAP TEXT("/showMap")
#define CMD_CLOSEAPP TEXT("/closeApp")

#define TIMEOUT_TaxiLoginQueue_Seconds 30 //30 seconds
#define TIMEOUT_TaxiLoginQueue SECOND_IN_MILI * TIMEOUT_TaxiLoginQueue_Seconds //30 seconds

enum TaxiCommands{
	TC_HELP,
	TC_SPEED_UP,
	TC_SPEED_DOWN,
	TC_AUTORESP_ON,
	TC_AUTORESP_OFF,
	TC_DEFINE_CDN,
	TC_REQUEST_INTEREST,
	TC_SHOW_MAP,
	TC_CLOSEAPP,
	TC_UNDEFINED
};

CTDLL_API bool Setup_Application(Application* app);
CTDLL_API bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
CTDLL_API bool Setup_OpenSmhHandles(Application* app);
CTDLL_API bool Setup_OpenThreadHandles(Application* app);

CTDLL_API void Setup_CloseAllHandles(Application* app);
CTDLL_API void Setup_CloseThreadHandles(ThreadHandles* threadHandles);
CTDLL_API void Setup_CloseSyncHandles(SyncHandles* syncHandles);
CTDLL_API void Setup_CloseSmhHandles(ShmHandles* shmHandles);

CTDLL_API bool Service_ConnectToCentralNamedPipe(Application* app);
CTDLL_API bool Service_GetMap(Application* app);

CTDLL_API void Communication_SendTossRequest(Application* app, TossRequest tossRequest);