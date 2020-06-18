#include "DLLNodeList.h"

NodeList* List_Init(int baseSize){
	NodeList* list;

	list = malloc(sizeof(NodeList));
	list->itemList = calloc(baseSize, sizeof(Node*));
	if(list->itemList == NULL){
		_tprintf(TEXT("%s calloc failed! GLE: %d"), Utils_NewSubLine(), GetLastError());
	}
	list->maxItems = baseSize;
	list->baseSize = baseSize;
	list->quantItems = 0;

	return list;
}

bool List_Increase(NodeList* list){
	_tprintf(TEXT("%sTrying to increase from %d to %d"), 
		Utils_NewLine(),
		list->maxItems,
		list->maxItems + list->baseSize);

	list->maxItems += list->baseSize;


	Node** oldList = list->itemList;
	list->itemList = (Node**) realloc(list->itemList, list->maxItems * sizeof(Node*)); //New buffer
	
	if(list->itemList == NULL)
		free(oldList);

	return true;
}

bool List_Add(NodeList* list, Node* node){
	if(list->quantItems >= list->maxItems)
		if(!List_Increase(list))
			return false;

	if(list->itemList == NULL)
		return false;

	list->itemList[list->quantItems] = node;
	list->quantItems++;

	return true;
}

bool List_Remove(NodeList* list, int index){
	if(list->itemList[index] == NULL)
		return false;

	while(index < list->maxItems){
		index++;

		list->itemList[index-1] = list->itemList[index];
		/*Only check after first iteration
		**Even if it is null, it will remove the intended node
		*/
		if(list->itemList[index] == NULL)
			break;
	}

	list->quantItems--;
	return true;
}

bool List_Contains(NodeList* list, Node* node){
	if(!list || !node)
		return false;

	for(int i = list->quantItems-1; i >= 0; i--){
		if(list->itemList[i] == NULL)
			break;

		if(list->itemList[i]->xyPosition.x == node->xyPosition.x && 
			list->itemList[i]->xyPosition.y == node->xyPosition.y)
			return true;
	}

	return false;
}

void List_Close(NodeList* list){
	if(list){
		if(list->itemList){
			free(list->itemList);
		}
		free(list);
	}
}