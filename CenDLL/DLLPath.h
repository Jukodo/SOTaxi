#pragma once
#include "CenDLL.h"
#include "DLLMap.h"

typedef struct Node Node;
typedef struct Path Path;

struct Node{
	XY xyPosition;
	Node* parent;
	int depth;
};

struct Path{
	int steps;
	XY* path;
};