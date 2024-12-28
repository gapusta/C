#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "archke_simple_string_reader.h"

#define BUFFER_SIZE 256

void test1() {
	char* input = "+hello\r\n";
	char* expected = "hello";	
		
	RchkStringReader* reader = rchkStringReaderNew(BUFFER_SIZE);

	rchkStringReaderProcess(reader, input, strlen(input));

	char* result = rchkStringReaderData(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #1 failed\n");
		exit(-1);
	} 

	rchkStringReaderFree(reader);
	
	printf("Test #1 passed\n");
}

void test2() {
	char* input1 = "+hel";
	char* input2 = "lo\r\n";
        char* expected = "hello";	
		
	RchkStringReader* reader = rchkStringReaderNew(BUFFER_SIZE);

	rchkStringReaderProcess(reader, input1, strlen(input1));
	rchkStringReaderProcess(reader, input2, strlen(input2));

	char* result = rchkStringReaderData(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #2 failed\n");
		exit(-1);
	} 

	rchkStringReaderFree(reader);
	
	printf("Test #2 passed\n");
}

void test3() {
	char* input1 = "+Catch m";
	char* input2 = "e if y";
	char* input3 = "ou can, Mr. Holme";
	char* input4 = "s\r";
	char* input5 = "\n";
        
	char* expected = "Catch me if you can, Mr. Holmes";	
		
	RchkStringReader* reader = rchkStringReaderNew(BUFFER_SIZE);

	rchkStringReaderProcess(reader, input1, strlen(input1));
	rchkStringReaderProcess(reader, input2, strlen(input2));
	rchkStringReaderProcess(reader, input3, strlen(input3));
	rchkStringReaderProcess(reader, input4, strlen(input4));
	rchkStringReaderProcess(reader, input5, strlen(input5));

	char* result = rchkStringReaderData(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #3 failed\n");
		exit(-1);
	} 

	rchkStringReaderFree(reader);
	
	printf("Test #3 passed\n");
}

void test4() {
	char* input = "+\r\n";
	char* expected = "";	
		
	RchkStringReader* reader = rchkStringReaderNew(BUFFER_SIZE);

	rchkStringReaderProcess(reader, input, strlen(input));

	char* result = rchkStringReaderData(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #4 failed\n");
		exit(-1);
	} 

	rchkStringReaderFree(reader);
	
	printf("Test #4 passed\n");
}

// 'clear' operation test
void test5() {
	char* input1 = "+hello\r\n";
	char* expected1 = "hello";	
	
	char* input2 = "+hey\r\n";
	char* expected2 = "hey";

	RchkStringReader* reader = rchkStringReaderNew(BUFFER_SIZE);

	rchkStringReaderProcess(reader, input1, strlen(input1));

	char* result = rchkStringReaderData(reader);
	if (strcmp(result, expected1) != 0) {
		printf("Test #5 failed\n");
		exit(-1);
	}

	rchkStringReaderClear(reader);

	rchkStringReaderProcess(reader, input2, strlen(input2));

	result = rchkStringReaderData(reader);
	if (strcmp(result, expected2) != 0) {
		printf("Test #5 failed\n");
		exit(-1);
	}

	rchkStringReaderFree(reader);
	
	printf("Test #5 passed\n");
}

int main(void) {
	test1();
	test2();
	test3();
	test4();
	test5();
}

