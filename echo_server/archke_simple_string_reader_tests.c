#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "archke_simple_string_reader.h"

#define BUFFER_SIZE 256

void test1() {
	char* input = "+hello\r\n";
        char* expected = "hello";	
		
	rchk_ssr_status status = { .code = 0 };

	rchk_ssr* reader = rchk_ssr_new(BUFFER_SIZE, &status);

	rchk_ssr_process(reader, input, strlen(input), &status);

	char* result = rchk_ssr_str(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #1 failed\n");
		exit(-1);
	} 

	rchk_ssr_free(reader);
	
	printf("Test #1 passed\n");
}

void test2() {
	char* input1 = "+hel";
	char* input2 = "lo\r\n";
        char* expected = "hello";	
		
	rchk_ssr_status status = { .code = 0 };

	rchk_ssr* reader = rchk_ssr_new(BUFFER_SIZE, &status);

	rchk_ssr_process(reader, input1, strlen(input1), &status);
	rchk_ssr_process(reader, input2, strlen(input2), &status);

	char* result = rchk_ssr_str(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #2 failed\n");
		exit(-1);
	} 

	rchk_ssr_free(reader);
	
	printf("Test #2 passed\n");
}

void test3() {
	char* input1 = "+Catch m";
	char* input2 = "e if y";
	char* input3 = "ou can, Mr. Holme";
	char* input4 = "s\r";
	char* input5 = "\n";
        
	char* expected = "Catch me if you can, Mr. Holmes";	
		
	rchk_ssr_status status = { .code = 0 };

	rchk_ssr* reader = rchk_ssr_new(BUFFER_SIZE, &status);

	rchk_ssr_process(reader, input1, strlen(input1), &status);
	rchk_ssr_process(reader, input2, strlen(input2), &status);
	rchk_ssr_process(reader, input3, strlen(input3), &status);
	rchk_ssr_process(reader, input4, strlen(input4), &status);
	rchk_ssr_process(reader, input5, strlen(input5), &status);

	char* result = rchk_ssr_str(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #3 failed\n");
		exit(-1);
	} 

	rchk_ssr_free(reader);
	
	printf("Test #3 passed\n");
}

void test4() {
	char* input = "+\r\n";
        char* expected = "";	
		
	rchk_ssr_status status = { .code = 0 };

	rchk_ssr* reader = rchk_ssr_new(BUFFER_SIZE, &status);

	rchk_ssr_process(reader, input, strlen(input), &status);

	char* result = rchk_ssr_str(reader);

	if (strcmp(result, expected) != 0) {
		printf("Test #4 failed\n");
		exit(-1);
	} 

	rchk_ssr_free(reader);
	
	printf("Test #4 passed\n");
}

int main(void) {
	test1();
	test2();
	test3();
	test4();	
}

