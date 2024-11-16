#ifndef ARCHKE_SIMPLE_STRING_READER
#define ARCHKE_SIMPLE_STRING_READER

struct RchkStringReader { // Archke simple string reader 
	int state;
	int dataIdx;
	int dataSize; // max str size
	char* data;
};

typedef struct RchkStringReader RchkStringReader;

RchkStringReader* rchkStringReaderNew(int dataSize);
int   rchkStringReaderProcess(RchkStringReader* reader, char* chunk, int occupied);
void  rchkStringReaderClear(RchkStringReader* reader);
int   rchkStringReaderIsDone(RchkStringReader* reader);
char* rchkStringReaderData(RchkStringReader* reader);
int   rchkStringReaderDataSize(RchkStringReader* reader);
int   rchkStringReaderDataMaxSize(RchkStringReader* reader);
void  rchkStringReaderFree(RchkStringReader* reader);

#endif

