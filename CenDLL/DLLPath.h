#pragma once
#include "CenDLL.h"

#define DEFAULT_SEEN_SIZE 50

typedef struct Node Node;
typedef struct Path Path;

struct Node{
	int x;
	int y;

	Node* parent;
};

struct Path{
	int steps;
	XY* path;
};