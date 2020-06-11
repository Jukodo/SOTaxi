#pragma once
#include "CTDLL.h"

bool isLoggedIn(Application* app);
bool isValid_LicensePlate(TCHAR* sLicensePlate);
bool isValid_Coordinates(TCHAR* sCoordinates);

void Service_Login(Application* app, TCHAR* sLicensePlate, TCHAR* sCoordinates_X, TCHAR* sCoordinates_Y);
bool Service_PosLoginSetup(Application* app);
bool Service_LoginQueue(Application* app);
TaxiCommands Service_UseCommand(Application* app, TCHAR* command);
void Service_RegisterInterest(Application* app, TCHAR* idPassenger);
void Service_CloseApp(Application* app);
void Service_NewPosition(Application* app, double newX, double newY);
void Service_NewState(Application* app, TaxiState newState);

bool Command_DefineCDN(Application* app, TCHAR* value);
bool Command_Speed(Application* app, bool speedUp);
void Command_AutoResp(Application* app, bool autoResp);

bool Movement_NextRandomStep(Application* app, XYObject* object);

/* ToDo (TAG_REMOVE)
** Remove the following after
** Only used to develop and test few features
*/
void Temp_ShowMap(Application* app);