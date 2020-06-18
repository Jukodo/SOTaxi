#pragma once
#include "DLLUtils.h"
#include "DLLPath.h"
#include "DLLNodeList.h"

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
	if(namedPipe == NULL || namedPipe == INVALID_HANDLE_VALUE)
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

XY* Utils_GetNeighbors4(Map* map, XY pointA){
	XY* neighbor4List = calloc(MAX_NEIGHBORS, sizeof(XY));
	
	XY xyNeighbor;
	for(int i = 0; i < MAX_NEIGHBORS; i++){
		neighbor4List[i].x = -1;
		neighbor4List[i].y = -1;
		xyNeighbor = pointA;

		switch(i){
		case 0://Left
			xyNeighbor.x--;
			if(xyNeighbor.x < 0)//If invalid, Skip to the next iteration
				continue;
			break;
		case 1://Right
			xyNeighbor.x++;
			if(xyNeighbor.x >= map->width)//If invalid, Skip to the next iteration
				continue;
			break;
		case 2://Up
			xyNeighbor.y--;
			if(xyNeighbor.y < 0)//If invalid, Skip to the next iteration
				continue;
			break;
		case 3://Down
			xyNeighbor.y++;
			if(xyNeighbor.y >= map->height)//If invalid, Skip to the next iteration
				continue;
			break;
		default:
			_tprintf(TEXT("CE: Utils_GetNeighbors4 MAX_NEIGHBORS > 4 (Not implemented)"));
			return NULL;
		}

		if(map->cellArray[(map->width * (int) xyNeighbor.y) + (int) xyNeighbor.x] == MAP_ROAD_CHAR)
			neighbor4List[i] = xyNeighbor;
	}

	return neighbor4List;
}

/*Get closest path from point A to point B
  Using BFS (Wide-Search)
  Returns a malloc created Path struct
  Recommended to be freed after usage
*/
Path* Utils_GetPath(Map* map, XY pointA, XY pointB){
	if(map->cellArray[(map->width * (int) pointA.y) + (int) pointA.x] == MAP_STRUCTURE_CHAR ||
		map->cellArray[(map->width * (int) pointB.y) + (int) pointB.x] == MAP_STRUCTURE_CHAR){

		_tprintf(TEXT("%sPointA (X:%.2lf Y:%.2lf) or PointB (X:%.2lf Y:%.2lf) are on a structure!"),
			Utils_NewSubLine(),
			pointA.x,
			pointA.y,
			pointB.x,
			pointB.y);
		return;
	}

	//PointA and PointB cannot be the same
	if(pointA.x == pointB.x && pointA.y == pointB.y){
		_tprintf(TEXT("%sOrigin (X:%.2lf Y:%.2lf) is same as destination (X:%.2lf Y:%.2lf)!"),
			Utils_NewSubLine(),
			pointA.x,
			pointA.y,
			pointB.x,
			pointB.y);
		return NULL;
	}

	NodeList* nlNodesSeen = List_Init(DEFAULT_SEEN_SIZE);
	NodeList* nlNodesQueue = List_Init(DEFAULT_QUEUE_SIZE);
	NodeList* nlNodesNewQueue = List_Init(DEFAULT_QUEUE_SIZE);
	
	_tprintf(TEXT("%sQuant items%snlNS:%d%snlNQ:%d%snlNNQ:%d\n"),
		Utils_NewSubLine(),
		Utils_NewSubLine(),
		nlNodesSeen->quantItems,
		Utils_NewSubLine(),
		nlNodesQueue->quantItems,
		Utils_NewSubLine(), 
		nlNodesNewQueue->quantItems);

	Node* root = malloc(sizeof(Node));
	root->xyPosition = pointA;
	root->depth = 1;

	List_Add(nlNodesQueue, root);
	_tprintf(TEXT("%s[Create Path] Added [X:%.2lf Y:%.2lf] as a queue item"), 
		Utils_NewSubLine(),
		root->xyPosition.x,
		root->xyPosition.y);

	XY* neighbors4;
	Node* newNode;

	//remove later and following prints
	bool print = false;

	bool keepRunning = true;
	Node* finalNode = NULL;
	while(keepRunning){
		if(print)
			_tprintf(TEXT("%sStarting to read nlNodesQueue with %d nodes"), Utils_NewSubLine(), nlNodesQueue->quantItems);
		for(int i = 0; i < nlNodesQueue->quantItems; i++){
			if(nlNodesQueue->itemList[i] == NULL){
				if(print)
					_tprintf(TEXT("%snlNodesQueue item #%d is NULL"), Utils_NewSubLine(), i);
				break;
			}

			if(List_Contains(nlNodesSeen, nlNodesQueue->itemList[i]))
				continue;

			//Add item being checked into seen items list
			List_Add(nlNodesSeen, nlNodesQueue->itemList[i]);

			if(print)
				_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] Reading as a queue item"),
				Utils_NewSubLine(), 
				nlNodesQueue->itemList[i]->xyPosition.x,
				nlNodesQueue->itemList[i]->xyPosition.y);

			neighbors4 = Utils_GetNeighbors4(map, nlNodesQueue->itemList[i]->xyPosition);
			if(neighbors4 == NULL){
				if(print)
					_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] Does not have any valid neighbors"),
					Utils_NewSubLine(),
					nlNodesQueue->itemList[i]->xyPosition.x,
					nlNodesQueue->itemList[i]->xyPosition.y);
				continue;
			}

			for(int n = 0; n < MAX_NEIGHBORS; n++){
				if(print)
					_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] Checking neighbor X:%.2lf Y:%.2lf"),
					Utils_NewSubLine(),
					nlNodesQueue->itemList[i]->xyPosition.x,
					nlNodesQueue->itemList[i]->xyPosition.y,
					neighbors4[n].x,
					neighbors4[n].y);

				if(neighbors4[n].x == -1 || neighbors4[n].y == -1){ //Invalid neighbor (out of bounds or structure)
					if(print)
						_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] (X:%.2lf Y:%.2lf) Invalid neighbor"),
						Utils_NewSubLine(),
						nlNodesQueue->itemList[i]->xyPosition.x,
						nlNodesQueue->itemList[i]->xyPosition.y,
						neighbors4[n].x,
						neighbors4[n].y);
					continue;
				}

				newNode = malloc(sizeof(Node));
				if(newNode == NULL){
					if(print)
						_tprintf(TEXT("%s newNode malloc failed! GLE: %d"), Utils_NewSubLine(), GetLastError());
					return NULL;
				}
				newNode->xyPosition = neighbors4[n];
				newNode->parent = nlNodesQueue->itemList[i];
				newNode->depth = newNode->parent->depth + 1;

				//Skip already seen positions
				if(List_Contains(nlNodesSeen, newNode)){
					if(print)
						_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] (X:%.2lf Y:%.2lf) Already seen cell"),
						Utils_NewLine(),
						nlNodesQueue->itemList[i]->xyPosition.x,
						nlNodesQueue->itemList[i]->xyPosition.y,
						neighbors4[n].x,
						neighbors4[n].y);

					free(newNode);
					continue;
				}

				if(List_Contains(nlNodesNewQueue, newNode)){
					if(print)
						_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] (X:%.2lf Y:%.2lf) Already in queue cell"),
						Utils_NewLine(),
						nlNodesQueue->itemList[i]->xyPosition.x,
						nlNodesQueue->itemList[i]->xyPosition.y,
						neighbors4[n].x,
						neighbors4[n].y);

					free(newNode);
					continue;
				}

				//Found PointB
				if(newNode->xyPosition.x == pointB.x &&
					newNode->xyPosition.y == pointB.y){
					if(print)
						_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] (X:%.2lf Y:%.2lf) Found PointB! (X:%.2lf Y:%.2lf)"),
						Utils_NewSubLine(),
						nlNodesQueue->itemList[i]->xyPosition.x,
						nlNodesQueue->itemList[i]->xyPosition.y,
						neighbors4[n].x,
						neighbors4[n].y,
						pointB.x,
						pointB.y);

					finalNode = newNode;
					keepRunning = false;
					break;
				}

				if(!List_Add(nlNodesNewQueue, newNode)){
					if(print)
						_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] (X:%.2lf Y:%.2lf) Adding failed"),
						Utils_NewSubLine(),
						nlNodesQueue->itemList[i]->xyPosition.x,
						nlNodesQueue->itemList[i]->xyPosition.y,
						neighbors4[n].x,
						neighbors4[n].y);
					return NULL;
				}
				if(print)
					_tprintf(TEXT("%s[X:%.2lf Y:%.2lf] (X:%.2lf Y:%.2lf) Added to new queue list"),
					Utils_NewSubLine(),
					nlNodesQueue->itemList[i]->xyPosition.x,
					nlNodesQueue->itemList[i]->xyPosition.y,
					neighbors4[n].x,
					neighbors4[n].y);
			}
		}

		if(nlNodesSeen->quantItems > (map->height * map->width) || nlNodesQueue->quantItems <= 0)
			keepRunning = false;

		if(print)
			_tprintf(TEXT("%sBEFORE Quant items%snlNS:%d%snlNQ:%d%snlNNQ:%d\n"),
			Utils_NewSubLine(),
			Utils_NewSubLine(),
			nlNodesSeen->quantItems,
			Utils_NewSubLine(),
			nlNodesQueue->quantItems,
			Utils_NewSubLine(),
			nlNodesNewQueue->quantItems);

		List_Close(nlNodesQueue);
		nlNodesQueue = nlNodesNewQueue;
		nlNodesNewQueue = List_Init(DEFAULT_QUEUE_SIZE);

		if(print)
			if(print)
				_tprintf(TEXT("%sAFTER Quant items%snlNS:%d%snlNQ:%d%snlNNQ:%d\n"),
			Utils_NewSubLine(),
			Utils_NewSubLine(),
			nlNodesSeen->quantItems,
			Utils_NewSubLine(),
			nlNodesQueue->quantItems,
			Utils_NewSubLine(),
			nlNodesNewQueue->quantItems);
	}

	if(finalNode != NULL)
		_tprintf(TEXT("%sWE FOUND IT with depth of %d"), Utils_NewSubLine(), finalNode->depth);

	if(nlNodesSeen->quantItems > (map->height * map->width))
		_tprintf(TEXT("%sUtils_GetPath Seen quantItems > %d"), Utils_NewSubLine(), (map->height * map->width));

	if(nlNodesQueue->quantItems <= 0)
		_tprintf(TEXT("%sUtils_GetPath Queue quantItems <= 0"), Utils_NewSubLine());

	return NULL;
}