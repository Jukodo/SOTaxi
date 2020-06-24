#pragma once
#include "CenDLL.h"

#define DEFAULT_SEEN_SIZE 50
#define DEFAULT_QUEUE_SIZE 50

typedef struct NodeList NodeList;

struct NodeList{
	Node** itemList;
	int maxItems;
	int quantItems;
	int baseSize;
};

NodeList* List_Init(int baseSize);
bool List_Increase(NodeList* list);
bool List_Add(NodeList* list, Node* node);
bool List_Remove(NodeList* list, int index);
bool List_Contains(NodeList* list, Node* node);
void List_Clear(NodeList* list);
void List_Close(NodeList* list);