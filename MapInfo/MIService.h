#pragma once
#include "CenDLL.h"
#include "MIThreads.h"

#define NAME_MUTEX_QnARequest TEXT("JUSO2TAXI_MUTEX_QNAR")
#define NAME_WTIMER_REFRESH_RATE TEXT("JUSO2TAXI_WTIMER_RR")

#define REFRESH_RATE 1

typedef struct Application Application;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;
typedef struct ShmHandles ShmHandles;
typedef struct RefreshRoutine RefreshRoutine;

struct ThreadHandles{
	HANDLE hRefreshRoutine;
	DWORD dwIdRefreshRoutine;
};

struct SyncHandles{
	HANDLE hEvent_TaxiList;
	HANDLE hEvent_PassengerList;
	HANDLE hMutex_QnARequest_CanAccess;
	HANDLE hEvent_QnARequest_Read;
	HANDLE hEvent_QnARequest_Write;
};

struct ShmHandles{
	HANDLE hSHM_TaxiList;
	LPVOID lpSHM_TaxiList;
	HANDLE hSHM_PassengerList;
	LPVOID lpSHM_PassengerList;
	HANDLE hSHM_QnARequest;
	HANDLE lpSHM_QnARequest;
	HANDLE hSHM_Map;
	HANDLE lpSHM_Map;
};

struct RefreshRoutine{
	int mapRelativeWidth;
	int mapRelativeHeight;
	int cellWidth;
	int cellHeight;
	int xMapOffset;
	int yMapOffset;
};

struct Application{
	Map map;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	ShmHandles shmHandles;
	RefreshRoutine refreshRoutine;
	int maxTaxis;
	int maxPassengers;
};

bool Setup_Application(Application* app, HWND hWnd);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenThreadHandles_RefreshRoutine(ThreadHandles* threadHandles, HANDLE hWnd);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
bool Setup_OpenShmHandles(Application* app);

void Setup_CloseAllHandles(Application* app);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Setup_CloseShmHandles(ShmHandles* shmHandles);

bool Service_RequestMaxVars(Application* app);

void Paint_DrawMap(Application* app, HDC hdc, HBRUSH roadBrush, HBRUSH structureBrush, HBRUSH  cellBorderBrush);
void Paint_MapCoordinates(Application* app, HDC hdc, HBRUSH borderBrush);
void Paint_Taxis(Application* app, HDC hdc, HBRUSH taxiBrush);
void Paint_Passengers(Application* app, HDC hdc, HBRUSH passengerBrush);

void Get_RectFromXY(Application* app, RECT* rRect, int x, int y);

Taxi* Get_TaxiAt(Application* app, int X, int Y, int* taxiIndex);
Passenger* Get_PassengerAt(Application* app, int X, int Y, int* passengerIndex);