#include "DLLUtils.h"

void Utils_CleanString(TCHAR* str){
	if(str[_tcslen(str) - 1] == '\n')
		str[_tcslen(str) - 1] = '\0';
}

bool Utils_StringIsNumber(TCHAR* str){
	if(Utils_StringIsEmpty(str))
		return false;

	for(unsigned int i = 0; i < _tcslen(str); i++){
		if(!iswdigit(str[i]))
			return false;
	}
	return true;
}

bool Utils_StringIsEmpty(TCHAR* str){
	if(str == NULL || str[0] == '\0' || str[0] == '\n' || _tcslen(str) == 0)
		return true;

	return false;
}

TCHAR* Utils_NewLine(){
	return TEXT("\n\n\t");
}

TCHAR* Utils_NewSubLine(){
	return TEXT("\n\t\t");
}

void Utils_CleanStdin(){
	int c;
	while((c = fgetc(stdin)) != '\n' && c != EOF);
}

bool Utils_IsValid_LicensePlate(TCHAR* sLicensePlate){
	if(_tcslen(sLicensePlate) != 8)
		return false;

	for(unsigned int i = 0; i < _tcslen(sLicensePlate); i++){
		if((i+1)%3 == 0){ //every third character it is supposed to be a '-'
			if(sLicensePlate[i] == '-')
				continue;
			else
				return false;
		}

		if(!iswalpha(sLicensePlate[i]) && !iswdigit(sLicensePlate[i]))
			return false;
	}

	return true;
}

bool Utils_IsValid_Id(TCHAR* sId){
	if(_tcslen(sId) != 8)
		return false;

	for(unsigned int i = 0; i < _tcslen(sId); i++){
		if(!iswalpha(sId[i]) && !iswdigit(sId[i]))
			return false;
	}

	return true;
}

bool Utils_IsValid_Coordinates(TCHAR* sCoordinates){
	if(_tcslen(sCoordinates) <= 0 || _tcslen(sCoordinates) > 2)
		return false;

	for(unsigned int i = 0; i < _tcslen(sCoordinates); i++){
		if(!iswdigit(sCoordinates[i]))
			return false;
	}

	return true;
}

bool Utils_CloseNamedPipe(HANDLE namedPipe){
	if(namedPipe == NULL)
		return false;

	FlushFileBuffers(namedPipe); //Flush the pipe to allow the client to read the pipe's contents before disconnecting. 
	DisconnectNamedPipe(namedPipe); //Then disconnect the pipe
	CloseHandle(namedPipe); //Close the handle to this pipe instance

	return true;
}

void Utils_DLL_Register(TCHAR* name, int type){
	if(!ALLOW_PROF_DLL)
		return;

	if(hLib == NULL){
		_tprintf(TEXT("%sLoadLibrary failed! Error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	dll_register hLib_register = (dll_register) GetProcAddress(hLib, "dll_register");
	if(hLib_register == NULL){
		_tprintf(TEXT("%sGetProcAddress failed! Error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	hLib_register(name, type);
}

void Utils_DLL_Log(TCHAR* text){
	if(!ALLOW_PROF_DLL)
		return;

	if(hLib == NULL){
		_tprintf(TEXT("%sLoadLibrary failed! Error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	dll_log hLib_log = (dll_log) GetProcAddress(hLib, "dll_log");
	if(hLib_log == NULL){
		_tprintf(TEXT("%sGetProcAddress failed! Error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	hLib_log(text);
}

void Utils_DLL_Test(){
	if(!ALLOW_PROF_DLL)
		return;

	if(hLib == NULL){
		_tprintf(TEXT("%sLoadLibrary failed! Error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	dll_test hLib_test = (dll_test) GetProcAddress(hLib, "dll_test");
	if(hLib_test == NULL){
		_tprintf(TEXT("%sGetProcAddress failed! Error: %d"), Utils_NewLine(), GetLastError());
		return;
	}

	hLib_test();
}

void Utils_GenerateNewRand(){
	srand(time(NULL));
}

TCHAR Utils_GetRandomLetter(){
	return 'A' + (rand() % 26);
}