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





typedef enum _phase_enum {
	/* 0 */ INIT,
	/* 1 */ LEXICAL,
	/* 2 */ SYNTAX,
	/* 3 */ SEMANTIC,
	/* 4 */ IR,
	/* 5 */ CODE_GEN,
	/* 6 */ ASSEMBLE,
	/* 7 */ LINK,
	/* 8 */ SUCCESS
} phase_t;






#endif