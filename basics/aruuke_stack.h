#ifndef ARUUKE_STACK_H
#define ARUUKE_STACK_H

struct AruukeStackItem {
	struct AruukeStackItem* next;
	int value; 
};

struct AruukeStack {
	struct AruukeStackItem* head;
	int size;
};

typedef struct AruukeStackItem* AruukeStackItem;
typedef struct AruukeStack* AruukeStack;

AruukeStack 	AruukeStackNew();
void 		AruukeStackPush(AruukeStack arst, int value);
int  		AruukeStackPop(AruukeStack arst);
int		AruukeStackSize(AruukeStack arst);
void 		AruukeStackFree(AruukeStack arst);

#endif

