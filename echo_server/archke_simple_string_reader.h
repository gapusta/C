#ifndef ARCHKE_SIMPLE_STRING_READER
#define ARCHKE_SIMPLE_STRING_READER

enum rchk_ssr_state { // Archke simple string reader states
	rchk_ssr_start,
	rchk_ssr_read,
	rchk_ssr_done
};

struct rchk_ssr { // Archke simple string reader 
	enum rchk_ssr_state state;
	int idx;
	int max; // max str size
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

rchk_ssr* rchk_ssr_new(int buffer_size, rchk_ssr_status* status);

int rchk_ssr_process(rchk_ssr* ssr, char* chunk, int occupied, rchk_ssr_status* status);

void rchk_ssr_clear(rchk_ssr* ssr);

int rchk_ssr_is_done(rchk_ssr* ssr);

char* rchk_ssr_str(rchk_ssr* ssr);

int rchk_ssr_str_size(rchk_ssr* ssr);

void rchk_ssr_free(rchk_ssr* ssr);

#endif

