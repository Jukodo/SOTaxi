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

		_tprintf(TEXT("%s[Path Finding] PointA (X:%.2lf Y:%.2lf) or PointB (X:%.2lf Y:%.2lf) are coordinates of a building!"),
			Utils_NewSubLine(),
			pointA.x,
			pointA.y,
			pointB.x,
			pointB.y);
		return;
	}

	//PointA and PointB cannot be the same
	if(pointA.x == pointB.x && pointA.y == pointB.y){
		_tprintf(TEXT("%s[Path Finding] Origin (X:%.2lf Y:%.2lf) is same as destination (X:%.2lf Y:%.2lf)!"),
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

	Node* root = malloc(sizeof(Node));
	root->xyPosition = pointA;
	root->depth = 0;
	List_Add(nlNodesQueue, root);

	XY* neighbors4;
	Node* newNode;
	Path* returnPath = NULL;

	bool keepRunning = true;
	Node* finalNode = NULL;
	while(keepRunning){
		for(int i = 0; i < nlNodesQueue->quantItems; i++){
			if(nlNodesQueue->itemList[i] == NULL)
				break;

			if(List_Contains(nlNodesSeen, nlNodesQueue->itemList[i]))
				continue;

			//Add item being checked into seen items list
			List_Add(nlNodesSeen, nlNodesQueue->itemList[i]);

			neighbors4 = Utils_GetNeighbors4(map, nlNodesQueue->itemList[i]->xyPosition);
			if(neighbors4 == NULL)//If cell does not have any neighbors, skip to next step
				continue;

			for(int n = 0; n < MAX_NEIGHBORS; n++){
				if(neighbors4[n].x == -1 || neighbors4[n].y == -1) //Invalid neighbor (out of bounds or structure)
					continue;

				newNode = malloc(sizeof(Node));
				if(newNode == NULL){
					keepRunning = false;
					break;
				}
				newNode->xyPosition = neighbors4[n];
				newNode->parent = nlNodesQueue->itemList[i];
				newNode->depth = newNode->parent->depth + 1;

				//Skip already seen positions
				if(List_Contains(nlNodesSeen, newNode)){
					free(newNode);
					continue;
				}

				if(List_Contains(nlNodesNewQueue, newNode)){
					free(newNode);
					continue;
				}

				//Found PointB
				if(newNode->xyPosition.x == pointB.x &&
					newNode->xyPosition.y == pointB.y){
					finalNode = newNode;
					keepRunning = false;
					break;
				}

				if(!List_Add(nlNodesNewQueue, newNode)){
					keepRunning = false;
					break;
				}
			}

			if(!keepRunning)//If keep running was set to false while inside cycle, break the while cycle aswell
				break;
		}
		
		if(!keepRunning)//If keep running was set to false while inside cycle, break the while cycle aswell
			break;

		if(nlNodesSeen->quantItems > (map->height * map->width) || nlNodesQueue->quantItems <= 0)
			break;

		List_Close(nlNodesQueue);
		nlNodesQueue = nlNodesNewQueue;
		nlNodesNewQueue = List_Init(DEFAULT_QUEUE_SIZE);
	}

	if(finalNode != NULL){
		Path* newPath = malloc(sizeof(Path));
		if(newPath != NULL){
			newPath->path = calloc(finalNode->depth, sizeof(XY));
			if(newPath->path != NULL){
				newPath->steps = finalNode->depth;

				Node* nodeIt = finalNode;
				for(int i = finalNode->depth-1; i >= 0; i--){
					newPath->path[i] = nodeIt->xyPosition;

					if(nodeIt == NULL || //Something went wrong
						(nodeIt->xyPosition.x == pointA.x && //Do not add pointA has a path step
							nodeIt->xyPosition.y == pointA.y))
						break;

					nodeIt = nodeIt->parent;
				}

				returnPath = newPath;
			}
		}
	}

	//Free all nodes and lists
	List_Clear(nlNodesSeen);
	List_Clear(nlNodesQueue);
	List_Clear(nlNodesNewQueue);
	List_Close(nlNodesSeen);
	List_Close(nlNodesQueue);
	List_Close(nlNodesNewQueue);

	return returnPath;
}