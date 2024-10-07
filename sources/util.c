#include <string.h>
#include "limits.h"
void strcopy(char *d, char *s){
    strncpy(d, s, MAXSTRLEN);
}