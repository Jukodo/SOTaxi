#include "Communication.h"

DWORD WINAPI Thread_SendRequests(LPVOID request){
	SendRequest* req = (SendRequest*) request;
	_tprintf(TEXT("\n%d"), &req->app->syncHandles.mutex_SendRequest);
	WaitForSingleObject(req->app->syncHandles.mutex_SendRequest, INFINITE);
	_tprintf(TEXT("Thread_SendRequests is working now"));

	return 0;
}