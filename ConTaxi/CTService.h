#pragma once
#include "CTDLL.h"

bool isLoggedIn(Application* app);
bool isValid_LicensePlate(TCHAR* sLicensePlate);
bool isValid_Coordinates(TCHAR* sCoordinates);

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y);
TaxiCommands Service_UseCommand(Application* app, TCHAR* command);
void Service_RegisterInterest(Application* app, TCHAR* idPassenger);
void Service_CloseApp(Application* app);

bool Command_DefineCDN(Application* app, TCHAR* value);
bool Command_Speed(Application* app, bool speedUp);
void Command_AutoResp(Application* app, bool autoResp);