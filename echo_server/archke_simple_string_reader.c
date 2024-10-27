#include <stdlib.h>
#include <string.h>
#include "archke_simple_string_reader.h"

rchk_ssr* rchk_ssr_new(int buffer_size, rchk_ssr_status* status) {
	rchk_ssr* new = (rchk_ssr*) malloc(sizeof(rchk_ssr) + buffer_size); // '1' is for '\0'
	if (new == NULL) {
		status->code = 1;
		return NULL;
	}

	bzero(new, sizeof(rchk_ssr) + buffer_size);

	new->state = rchk_ssr_start;
	new->idx = 0;
	new->str = (char*) (new + sizeof(rchk_ssr));
	new->str[0] = '\0';
	new->max = buffer_size;
	status->code = 0;
	
	return new;	
}

void rchk_ssr_clear(rchk_ssr* ssr) {
	ssr->state = rchk_ssr_start;
	ssr->idx = 0;
	ssr->str[0] = '\0';	
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

int rchk_ssr_is_done(rchk_ssr* ssr) { return ssr->state == rchk_ssr_done; }

char* rchk_ssr_str(rchk_ssr* ssr) { return ssr->str; }

void rchk_ssr_free(rchk_ssr* ssr) { free(ssr); }

