#ifndef _SCAN_H_
#define _SCAN_H_
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "limits.h"
#include "init.h"
#include "debug.h"
#include "lexical.h"
extern int lineno;
extern int colmnno;
extern char linebuf[MAXLINEBUF];
extern char tokenbuf[MAXTOKSIZE + 1];
extern bool fileend;
extern int bufsize;
extern int readc(bool peek);
extern void unreadc(void);
extern token_t gettok(void);
extern int toklineno;


typedef enum _state_enum {
    /*0*/ START,
    /*1*/ INSTR,
    /*2*/ INUNS,
    /*3*/ INIDE,
    /*4*/ INLES,
    /*5*/ INCOM,
    /*6*/ INGRE,
    /*7*/ INCHA,
    /*8*/ INCMT,
    /*9*/ DONE
}state_t;


#endif