#pragma once
#include "CenDLL.h"

#define DEFAULT_SEEN_SIZE 50

typedef struct Node Node;

struct Node{
	int x;
	int y;

	Node* parent;
};