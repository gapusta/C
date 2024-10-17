#include <stdlib.h>
#include <stdio.h>

enum rchk_ssr_state { // Archke simple string reader states
	rchk_ssr_start,
	rchk_ssr_read,
	rchk_ssr_done
};

typedef enum rchk_ssr_state rchk_ssr_state;

struct rchk_ssr { // Archke simple string reader 
	rchk_ssr_state state;
	int idx;
	char* str;
};

typedef struct rchk_ssr rchk_ssr;

rchk_ssr* rchk_ssr_new(int buffer_size) {
	rchk_ssr* new = (rchk_ssr*) malloc(sizeof(rchk_ssr));
	
	new->state = rchk_ssr_start;
	new->idx = 0;
	new->str = (char*) malloc(buffer_size);
	
	return new;	
}

void rchk_ssr_process(rchk_ssr* ssr, char* chunk, int occupied) {
	for (int idx=0; idx<occupied; idx++) {
		char current = chunk[idx];

		switch(ssr->state) {
			case rchk_ssr_start:
				if (current == '+') { ssr->state = rchk_ssr_read; } else { } // handle error "+ expected"
				
				break;
			case rchk_ssr_read:
				if (current == '\r') continue;
				if (current == '\n') {
					ssr->str[ssr->idx] = '\0';
					ssr->state = rchk_ssr_done;
					continue;
				}

				ssr->str[ssr->idx] = current;
				ssr->idx++;

				break;
			case rchk_ssr_done: 
				break;
		
		}
	}	
}

int rchk_sst_done(rchk_ssr* ssr) {
	return ssr->state == rchk_ssr_done; 
}

char* rchk_ssr_str(rchk_ssr* ssr) {
	return ssr->str;
}

void rchk_ssr_free(rchk_ssr* ssr) {
	free(ssr->str);
	free(ssr);
}

int main(void) {
	rchk_ssr* reader = rchk_ssr_new(256);

	char* input = "+heyyyyyy\r\n";

	rchk_ssr_process(reader, input, 11);

	printf("%s\n", rchk_ssr_str(reader));

	rchk_ssr_free(reader);
}

