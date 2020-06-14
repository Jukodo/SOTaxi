#pragma once
#include "CPThreads.h"

DWORD WINAPI Thread_ReadComms(LPVOID _param){
	TParam_ReadComms* param = (TParam_ReadComms*) _param;

	/*ToDo (TAG_TODO)
	**Loop reading incoming comms from central
	*/

	free(param);
	return 1;
}

DWORD WINAPI Thread_SendComm(LPVOID _param){
	TParam_SendComm* param = (TParam_SendComm*) _param;

	

	free(param);
	return 1;
}