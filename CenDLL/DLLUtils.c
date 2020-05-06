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