#pragma once
#include "CenDLL.h"

#define MAP_MIN_WIDTH 50
#define MAP_MIN_HEIGHT 50
#define MAP_STRUCTURE_CHAR 'X'
#define MAP_ROAD_CHAR '_'

typedef struct XY XY;
typedef struct XYObject XYObject;
typedef struct Map Map;

struct Map{
	int width;
	int height;
	char* cellArray;
};

struct XY{
	double x;	//Horizontal
	double y;	//Vertical
};

struct XYObject{
	XY xyPosition; //Position
	/*Speed
	  X: 1 = (Moving Right) | -1 = (Moving Left) | 0 = (Horizontally Stationary)
	  Y: 1 = (Moving Down) | -1 = (Moving Up) | 0 = (Vertically Stationary)*/
	XY speed;				  
	double speedMultiplier; //Speed * Multiplier
};