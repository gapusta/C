#include <stdlib.h>
#include <stdio.h>

enum rchk_ssr_state { // Archke simple string reader states
	rchk_ssr_start,
	rchk_ssr_read,
	rchk_ssr_done
};

struct rchk_ssr { // Archke simple string reader 
	enum rchk_ssr_state state;
	int idx;
	char* str;
};

// 0 - OK
// 1 - Initialization failed
// 200 - '+' was expected
struct rchk_ssr_status {
	int code;
};

typedef enum rchk_ssr_state rchk_ssr_state;
typedef struct rchk_ssr rchk_ssr;
typedef struct rchk_ssr_status rchk_ssr_status;

rchk_ssr* rchk_ssr_new(int buffer_size, rchk_ssr_status* status) {
	rchk_ssr* new = (rchk_ssr*) malloc(sizeof(rchk_ssr) + (buffer_size + 1)); // '1' is for '\0'
	if (new == NULL) {
		status->code = 1;
		return NULL;
	}

	new->state = rchk_ssr_start;
	new->idx = 0;
	new->str = (char*) (new + sizeof(rchk_ssr));
	status->code = 0;
	
	return new;	
}

int rchk_ssr_process(rchk_ssr* ssr, char* chunk, int occupied, rchk_ssr_status* status) {
	for (int idx=0; idx<occupied; idx++) {
		char current = chunk[idx];

		switch(ssr->state) {
			case rchk_ssr_start:
				if (current == '+') { 
					ssr->state = rchk_ssr_read; 
				} else { 
					status->code = 200;
					return -1;		
				}
				
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

	return 0;	
}

int rchk_sst_done(rchk_ssr* ssr) { return ssr->state == rchk_ssr_done; }

char* rchk_ssr_str(rchk_ssr* ssr) { return ssr->str; }

int rchk_ssr_free(rchk_ssr* ssr) { free(ssr); }

int main(void) {
	rchk_ssr_status status;
	status.code = 0;

	rchk_ssr* reader = rchk_ssr_new(3, &status);

	char* input = "+hey\r\n";

	rchk_ssr_process(reader, input, 6, &status);

	printf("%s\n", rchk_ssr_str(reader));

	rchk_ssr_free(reader);
}

