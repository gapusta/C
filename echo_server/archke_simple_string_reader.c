#include <stdlib.h>
#include <string.h>
#include "archke_simple_string_reader.h"

#define ARCHKE_SIMPLE_STRING_READER_START 0
#define ARCHKE_SIMPLE_STRING_READER_READ  1
#define ARCHKE_SIMPLE_STRING_READER_DONE  2

RchkStringReader* rchkStringReaderNew(int dataSize) {
	RchkStringReader* new = (RchkStringReader*) malloc(sizeof(RchkStringReader) + dataSize);
	if (new == NULL) {
		return NULL;
	}

	bzero(new, sizeof(RchkStringReader) + dataSize);

	new->state = ARCHKE_SIMPLE_STRING_READER_START;
	new->dataSize = dataSize;
	new->dataIdx = 0;
	new->data = (char*) (new + sizeof(RchkStringReader));
	new->data[0] = '\0';
	
	return new;	
}

int rchkStringReaderProcess(RchkStringReader* ssr, char* chunk, int occupied) {
	for (int idx=0; idx<occupied; idx++) {
		char current = chunk[idx];

		switch(ssr->state) {
			case ARCHKE_SIMPLE_STRING_READER_START:
				if (current == '+') { 
					ssr->state = ARCHKE_SIMPLE_STRING_READER_READ; 
				} else { 
					return -1;		
				}
				
				break;
			case ARCHKE_SIMPLE_STRING_READER_READ:
				if (current == '\r') continue;
				if (current == '\n') {
					ssr->data[ssr->dataIdx] = '\0';
					ssr->state = ARCHKE_SIMPLE_STRING_READER_DONE;
					continue;
				}

				ssr->data[ssr->dataIdx] = current;
				ssr->dataIdx++;

				break;
			case ARCHKE_SIMPLE_STRING_READER_DONE: 
				break;
		
		}
	}

	return 0;	
}

void rchkStringReaderClear(RchkStringReader* reader) {
	reader->state = ARCHKE_SIMPLE_STRING_READER_START;
	reader->dataIdx = 0;
	reader->data[0] = '\0';	
}

int rchkStringReaderIsDone(RchkStringReader* reader) { 
	return reader->state == ARCHKE_SIMPLE_STRING_READER_DONE; 
}

int rchkStringReaderDataSize(RchkStringReader* reader) {
	return reader->dataIdx;
}

int rchkStringReaderDataMaxSize(RchkStringReader* reader) {
	return reader->dataSize;
}

char* rchkStringReaderData(RchkStringReader* reader) { 
	return reader->data; 
}

void rchkStringReaderFree(RchkStringReader* reader) { 
	free(reader); 
}

#undef ARCHKE_SIMPLE_STRING_READER_START
#undef ARCHKE_SIMPLE_STRING_READER_READ
#undef ARCHKE_SIMPLE_STRING_READER_DONE

