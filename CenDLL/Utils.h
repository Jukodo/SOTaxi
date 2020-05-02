#pragma once
#include "CenDLL.h"

CENDLL_API void Utils_CleanString(TCHAR* str);
CENDLL_API bool Utils_StringIsNumber(TCHAR* str);
CENDLL_API bool Utils_StringIsEmpty(TCHAR* str);
CENDLL_API TCHAR* Utils_NewLine();
CENDLL_API TCHAR* Utils_NewSubLine();