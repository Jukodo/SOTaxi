#include "CenDLL.h"

void WINAPI Thread_Main(){
	while(1){
		wprintf(TEXT("1"));
	}
}

int _tmain(int argc, LPTSTR argv[]) {
	_tprintf(TEXT("%d"), MAIN_A());
	_tprintf(TEXT("%d"), TAXI_A());

	HANDLE hMapObj = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(Taxi),
		SHM_Testing
	);
	if(hMapObj == NULL)
	{
		_tprintf(TEXT("ERROR: hMapObj = NULL"));
		return FALSE;
	}
	LPVOID lpSharedMem = MapViewOfFile(
		hMapObj,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(Taxi)
	);
	if(lpSharedMem == NULL)
	{
		_tprintf(TEXT("ERROR: lpSharedMem = NULL"));
		return FALSE;
	}
	TAXI_A();
	TCHAR ssString[STRING_MEDIUM];

	HANDLE hTestingEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("TESTING"));
	if(hTestingEvent == NULL){
		_tprintf(TEXT("ERROR: hTestingEvent = NULL"));
		return FALSE;
	}
	//int idThreadMain;
	//HANDLE hThreadMain = CreateThread(
	//	NULL,
	//	0,
	//	Thread_Main,	//nome da funçao
	//	NULL,					//Argumento a ser passado
	//	0,						//Flags de criaçao
	//	&idThreadMain //idThread
	//);
	ZeroMemory(ssString, STRING_MEDIUM);
	while(/*_tcscmp(ssString, TEXT("exit")) != 0*/Utils_StringIsEmpty(ssString)){
		_tprintf(TEXT("\n\nInsert a number (1 - %d):"), _countof(ssString));
		_tprintf(TEXT("\n\t-> "));
		
		wscanf_s(TEXT(" %[^\n]"), ssString, _countof(ssString));

		int c;
		while((c = fgetc(stdin)) != '\n' && c != EOF); /* Flush stdin */

		if(!Utils_StringIsNumber(ssString) || Utils_StringIsEmpty(ssString)){
			_tprintf(TEXT("\nERROR: input doesn't follow rules"));
			ZeroMemory(ssString, STRING_MEDIUM);
			continue;
		}
		
		_tprintf(TEXT("\nInput: %s with lenght of %d characters"), ssString, _tcslen(ssString));
	}
	Taxi a;
	a.IdPassenger = _ttoi(ssString);
	_tprintf(TEXT("\nInput: %d"), a.IdPassenger);
	CopyMemory((LPVOID) lpSharedMem, &a, sizeof(Taxi));
	_getch();

	UnmapViewOfFile(lpSharedMem);
	CloseHandle(hMapObj);
	return 0;
}