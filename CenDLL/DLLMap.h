#pragma once
#include "CenDLL.h"

typedef struct XYObject{
	double coordX;	//Horizontal position
	double coordY; //Vertical position
	double speedX; //1 = (Moving Right) | -1 = (Moving Left) | 0 = (Horizontally Stationary)
	double speedY; //1 = (Moving Down) | -1 = (Moving Up) | 0 = (Vertically Stationary)
	double speedMultiplier; //Speed * Multiplier
}XYObject;