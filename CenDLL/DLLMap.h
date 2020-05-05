#pragma once
#include "CenDLL.h"

typedef struct XYObject{
	float coordX;	//Horizontal position
	float coordY; //Vertical position
	float speedX; //1 = (Moving Right) | -1 = (Moving Left) | 0 = (Horizontally Stationary)
	float speedY; //1 = (Moving Down) | -1 = (Moving Up) | 0 = (Vertically Stationary)
	float speedMultiplier; //Speed * Multiplier
}XYObject;