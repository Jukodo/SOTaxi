#pragma once
#include "CTDLL.h"

typedef struct TParam_StepRoutine TParam_StepRoutine;

struct TParam_StepRoutine{
	Application* app;
};

bool isLoggedIn(Application* app);
bool isValid_LicensePlate(TCHAR* sLicensePlate);
bool isValid_Coordinates(TCHAR* sCoordinates);

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y);
bool Service_PosLoginSetup(Application* app);
TaxiCommands Service_UseCommand(Application* app, TCHAR* command);
void Service_RegisterInterest(Application* app, TCHAR* idPassenger);
void Service_CloseApp(Application* app);
void Service_NewPosition(Application* app, double newX, double newY);
void Service_NewState(Application* app, TaxiState newState);
void Service_Step(Application* app);

bool Command_DefineCDN(Application* app, TCHAR* value);
bool Command_Speed(Application* app, bool speedUp);
void Command_AutoResp(Application* app, bool autoResp);

DWORD WINAPI Thread_StepRoutine(LPVOID _param);
bool Movement_NextRandomStep(Application* app, XYObject* object);

void Temp_ShowMap(Application* app);