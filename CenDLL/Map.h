#pragma once
#include "CenDLL.h"

typedef struct XYObject{
	int coordX;	//Horizontal position
	int coordY; //Vertical position
	int speedX; //1 = (Moving Right) | -1 = (Moving Left) | 0 = (Horizontally Stationary)
	int speedY; //1 = (Moving Down) | -1 = (Moving Up) | 0 = (Vertically Stationary)
	float speedMultiplier; //Speed * Multiplier
}XYObject;