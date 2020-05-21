#pragma once
#include "CenDLL.h"

#define MAP_MIN_WIDTH 50
#define MAP_MIN_HEIGHT 50
#define MAP_STRUCTURE_CHAR 'X'
#define MAP_ROAD_CHAR '_'

typedef struct XYObject XYObject;
typedef struct Map Map;
typedef struct Cell Cell;

struct Map{
	int width;
	int height;
	char* cellArray;
};

struct XYObject{
	double coordX;	//Horizontal position
	double coordY; //Vertical position
	double speedX; //1 = (Moving Right) | -1 = (Moving Left) | 0 = (Horizontally Stationary)
	double speedY; //1 = (Moving Down) | -1 = (Moving Up) | 0 = (Vertically Stationary)
	double speedMultiplier; //Speed * Multiplier
};