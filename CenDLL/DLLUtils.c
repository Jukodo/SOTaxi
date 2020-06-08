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

	HINSTANCE hLib = LoadLibrary(DLLPATH);
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

	HINSTANCE hLib = LoadLibrary(DLLPATH);
	if(!hLib){
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

	HINSTANCE hLib = LoadLibrary(DLLPATH);
	if(!hLib){
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