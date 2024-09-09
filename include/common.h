#ifndef _COMMON_H_
#define _COMMON_H_

#define INITMEM(s, v)                            \
    do {                                         \
        v = (s*)malloc(sizeof(s));              \
        if(v == NULL){                          \
            panic("OUT_OF_MEMORY");             \
        };                                      \
        memset(v, 0, sizeof(s));                \
    } while(0)                                   \








#endif