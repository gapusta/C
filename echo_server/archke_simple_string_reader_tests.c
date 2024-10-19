#include <stdio.h>
#include <string.h>
#include "archke_simple_string_reader.h"

#define BUFFER_SIZE 256

void test1() {
	rchk_ssr_status status = { .code = 0 };

	rchk_ssr* reader = rchk_ssr_new(BUFFER_SIZE, &status);

	char* input = "+hello\r\n";

	rchk_ssr_process(reader, input, strlen(input), &status);

	printf("%s\n", rchk_ssr_str(reader));

	rchk_ssr_free(reader);
}

int main(void) {
	test1();	
}

