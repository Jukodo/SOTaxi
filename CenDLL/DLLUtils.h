#pragma once
#include "CenDLL.h"

typedef void(__cdecl* dll_register)(TCHAR*, int);
typedef void(__cdecl* dll_log)(TCHAR*);
typedef void(__cdecl* dll_test)();

#define DLL_TYPE_MUTEX 1
#define DLL_TYPE_CRITSECTION 2
#define DLL_TYPE_SEMAPHORE 3
#define DLL_TYPE_EVENT 4
#define DLL_TYPE_WAITABLETIMER 5
#define DLL_TYPE_FILEMAPPING 6
#define DLL_TYPE_MAPVIEWOFFILE 7
#define DLL_TYPE_NAMEDPIPE 8

CENDLL_API void Utils_CleanString(TCHAR* str);
CENDLL_API bool Utils_StringIsNumber(TCHAR* str);
CENDLL_API bool Utils_StringIsEmpty(TCHAR* str);
CENDLL_API TCHAR* Utils_NewLine();
CENDLL_API TCHAR* Utils_NewSubLine();
CENDLL_API void Utils_CleanStdin();

CENDLL_API void Utils_DLL_Register(TCHAR* name, int type);
CENDLL_API void Utils_DLL_Log(TCHAR* text);
CENDLL_API void Utils_DLL_Test();