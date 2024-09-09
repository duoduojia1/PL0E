#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void quit(char *file ,int line ,const char *func ,int errno,char *msg){
    char *prefix = "QUIT";

    if(errno == 997) {
        prefix = "ABORT";
    }
    else if(errno == 996){
        prefix = "PANIC";
    }
    fprintf(stderr, "%s:%s:%d %s(): %s\n", prefix, file, line, func, msg);
}
