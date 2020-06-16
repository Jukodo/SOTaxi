#pragma once
#include "CenDLL.h"
#include "CPThreads.h"
#include "CPPassenger.h"

typedef struct Application Application;
typedef struct NamedPipeHandles NamedPipeHandles;
typedef struct ThreadHandles ThreadHandles;
typedef struct SyncHandles SyncHandles;

typedef enum PassengerCommands PassengerCommands;

struct NamedPipeHandles{
	HANDLE hRead;
	HANDLE hWrite;
	HANDLE hQnA;
};

struct ThreadHandles{
	HANDLE hNotificationReceiver_NamedPipe;
	DWORD dwIdNotificationReceiver_NamedPipe;
};

struct SyncHandles{
	HANDLE hMutex_QnA;
	HANDLE hMutex_Toss;
};

struct Application{
	NamedPipeHandles namedPipeHandles;
	ThreadHandles threadHandles;
	SyncHandles syncHandles;
	Passenger* passengerList;
	int maxPass;
	bool keepRunning; //Used to identify if app should keep running, when FALSE threads that are infinitely looping will end
};

#define CMD_HELP TEXT("/help")
#define CMD_LOGIN TEXT("/login")
#define CMD_LIST_PASSENGERS TEXT("/listPassengers")
#define CMD_CLOSEAPP TEXT("/closeApp")

#define TIMEOUT_TaxiLoginQueue_Seconds 30 //30 seconds
#define TIMEOUT_TaxiLoginQueue SECOND_IN_MILI * TIMEOUT_TaxiLoginQueue_Seconds //30 seconds

#define NAME_MUTEX_OneInstance_CP TEXT("JUSO2TAXI_MUTEX_OI_CP")
#define NAME_MUTEX_CommQnA TEXT("JUSO2TAXI_MUTEX_CQNA")
#define NAME_MUTEX_CommToss TEXT("JUSO2TAXI_MUTEX_CTOSS")

enum PassengerCommands{
	PC_HELP,
	PC_LOGIN,
	PC_LIST_PASSENGERS,
	PC_CLOSEAPP,
	PC_UNDEFINED
};

bool Setup_Application(Application* app);
bool Setup_OpenNamedPipeHandles(NamedPipeHandles* threadHandles);
bool Setup_OpenThreadHandles(Application* app);
bool Setup_OpenSyncHandles(SyncHandles* syncHandles);
void Setup_CloseAllHandles(Application* app);
void Setup_CloseNamedPipeHandles(NamedPipeHandles* namedPipeHandles);
void Setup_CloseSyncHandles(SyncHandles* syncHandles);
void Service_GetMaxPass(Application* app);
void Service_CloseApp(Application* app);
PassengerCommands Service_UseCommand(Application* app, TCHAR* command);

bool Add_Passenger(Application* app, CommsP2C_Login* loginComm);
bool Delete_Passenger(Application* app, int index);
int Get_QuantLoggedInPassengers(Application* app);
bool isPassengerListFull(Application* app);
int Get_FreeIndexPassengerList(Application* app);
int Get_PassengerIndex(Application* app, TCHAR* Id);
Passenger* Get_Passenger(Application* app, int index);

bool Command_LoginPassenger(Application* app, TCHAR* sId, TCHAR* sAtX, TCHAR* sAtY, TCHAR* sDestinyX, TCHAR* sDestinyY);
void Command_ListPassengers(Application* app);