#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "archke_logs.h"

void logError(char* prefix) {
    perror(prefix);
}

