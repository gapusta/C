#include <stdlib.h>
#include <stdio.h>
#include "aruuke_stack.h"

int main(void) {
	AruukeStack arst = AruukeStackNew();
	
	AruukeStackPush(arst, 1);
	AruukeStackPush(arst, 2);
	AruukeStackPush(arst, 3);

	printf("Size : %d\n", AruukeStackSize(arst));
	printf("%d\n", AruukeStackPop(arst));
	printf("%d\n", AruukeStackPop(arst));
	printf("%d\n", AruukeStackPop(arst));
	printf("Size : %d\n", AruukeStackSize(arst));

	AruukeStackFree(arst);

	return 0;
}

