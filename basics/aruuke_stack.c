#include <stdlib.h>
#include "aruuke_stack.h"

AruukeStack AruukeStackNew(){ 
	AruukeStack arst = (AruukeStack) malloc(sizeof(AruukeStack));
	arst->size = 0;
	return arst;
}

void AruukeStackPush(AruukeStack arst, int value) {
	AruukeStackItem it = (AruukeStackItem) malloc(sizeof(AruukeStackItem));
	
	it->value = value;
	it->next = arst->head;
	
	arst->head = it;
	arst->size++;
}

int AruukeStackPop(AruukeStack arst) {
	AruukeStackItem head = arst->head;
	
	int value = head->value;
	
	arst->head = head->next;
	arst->size--;

	free(head);
	
	return value;
}

int AruukeStackSize(AruukeStack arst) {
	return arst->size;
}

void AruukeStackFree(AruukeStack arst) {
	free(arst);
}

