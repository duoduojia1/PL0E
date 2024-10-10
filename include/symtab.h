#ifndef _SYMTAB_H_
#define _SYMTAB_H_
#include "limits.h"
#include "parse.h"


#define MAXBUCKETS 16 
typedef struct _sym_param_struct param_t;
typedef struct _sym_entry_struct syment_t;
typedef struct _sym_table_struct symtab_t;

// symbol category
typedef enum _sym_cate_enum{
    NOP_OBJ,
    CONST_OBJ,
    VAR_OBJ,
    PROC_OBJ,
    FUN_OBJ,
    ARRAY_OBJ,
    BYVAL_OBJ,
    BYREF_OBJ,
    TMP_OBJ,
    LABEL_OBJ,
    NUM_OBJ,
    STR_OBJ,
} cate_t;


typedef enum _sym_type_enum {
    VOID_TYPE,
    INT_TYPE,
    CHAR_TYPE,
    STR_TYPE,
} type_t;


struct _sym_param_struct {
    syment_t *symbol;
    param_t *next;
};

struct _sym_entry_struct {
    int sid;
    char name[MAXSTRLEN];
    cate_t cate;
    type_t type;
    int initval;
    int arrlen;
    char str[MAXSTRLEN];
    param_t *phead;
    symtab_t *scope;

    char label[MAXSTRLEN];
    int off;
    int lineno;

    symtab_t *stab;
    syment_t *next;
};

struct _sym_table_struct {
    int tid;
    int depth;
    char nspace[MAXSTRLEN];

    syment_t *funcsym; //current scope
    symtab_t *inner;
    symtab_t *outer;


    int argoff;
    int varoff;
    int tmpoff;

    syment_t buckets[MAXBUCKETS];
};

#define NEWPARAM(v) INITMEM(param_t, v)
#define NEWENTRY(v) INITMEM(syment_t, v)
#define NEWSTAB(v) INITMEM(symtab_t, v)


extern syment_t *syments[MAXSYMENT];
extern int sidcnt;


symtab_t *scope_entry(char *nspace);
symtab_t *scope_exit(void);
symtab_t *scope_top(void);

void symadd(syment_t *entry);
void symadd2(symtab_t *stab, syment_t *entry);

syment_t *symget(char *name);
syment_t *symget2(symtab_t *stab, char *name);
syment_t *symfind(char *name);

void stabdump();
static void dumptab(symtab_t *stab);
syment_t *syminit(ident_node_t *idp);
syment_t *syminit2(symtab_t *stab, ident_node_t *idp, char *key);
syment_t *symalloc(symtab_t *stab, char *name, cate_t cate, type_t type);



#endif

