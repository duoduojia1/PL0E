#include "limits.h"
#include "symtab.h"
#include "global.h"
#include "common.h"
#include "debug.h"
#include "syntax.h"
#include "util.h"
symtab_t *top = NULL;

int depth = 0;
int tidcnt = 0;

syment_t *syments[MAXSYMENT];
int sidcnt = 0;

symtab_t *scope_entry(char *nspace){
    symtab_t *t;
    NEWSTAB(t);
    t->tid = ++tidcnt;
    strcopy(t->nspace, nspace);
    t->varoff = 1;
    t->outer = top;
    if(top){
        top->inner = t;
    }
    top = t;
    return t;
}

symtab_t *scope_exit(void){
    nevernil(top);
    symtab_t *t  = top;
    top = t->outer;
    
    if(top){
        top->inner = NULL;
    }
    depth--;
    return t;
}

symtab_t *scope_top(void){
    nevernil(top);
    return top;
}

const int HASHSIZE = 211;
const int HASHSHIFT = 4;
static inline int hash(char *key){
    if(!key){
        panic("Bad key!")
    }
    int h,i;
    for(i = h =0;key[i] != '\0'; i++){
        h = ((h << HASHSHIFT) + key[i]) % HASHSIZE; 
    }
    return h;
}

static syment_t *getsym(symtab_t *stab, char *name){
    syment_t *hair, *p;
    hair = &stab->buckets[hash(name) % MAXBUCKETS];
    for(p = hair->next; p ; p = p->next){
        if(!strcmp(name,p->name)){
            return p;
        }
    }
    return NULL;
}

static void putsym(symtab_t *stab, syment_t *e){
    syment_t *hair = &stab->buckets[hash(e->name) % MAXBUCKETS];
    e->next = hair->next;
    hair->next = e;

    if(e->sid + 1 >= MAXSYMENT){
        panic("TOO_MANY_SYMENT");
    }
    syments[e->sid] = e;
}

syment_t *symget(char *name) { // find syment from  top
    nevernil(top);
    return getsym(top, name);
}

syment_t *symget2(symtab_t *stab,char *name){ //find syment from symtab
    return getsym(stab, name);
}

syment_t *symfind(char *name){ // find syment from all
    symtab_t *t;
    syment_t *p = NULL;
    for(t = top; t; t = t->outer){
        p = getsym(t, name);
        if(p) return p;
    }
    return NULL;
}


void symadd(syment_t *entry){
    symadd2(top, entry);
}

void symadd2(symtab_t *stab, syment_t *entry){
    nevernil(stab);
    putsym(stab, entry);
    entry->stab = stab;
}


syment_t *syminit(ident_node_t *idp){
    return syminit2(top, idp, idp->name);
}
syment_t *syminit2(symtab_t *stab, ident_node_t *idp, char *key){
    syment_t *e;
    NEWENTRY(e);
    e->sid = ++sidcnt;
    strcopy(e->name, key);
    e->initval = idp->value;
    e->arrlen = idp->length;
    e->lineno = idp->line;

    switch(idp->kind){
        case PROC_IDENT:
            e->cate = PROC_OBJ;
            break;
        case INT_FUN_IDENT:
        case CHAR_FUN_IDENT:
            e->cate = FUN_OBJ;
            break;
        case INT_CONST_IDENT:
        case CHAR_CONST_IDENT:
            e->cate = CONST_OBJ;
            break;
        case INT_ARRVAR_IDENT:
        case CHAR_ARRVAR_IDENT:
            e->cate = ARRAY_OBJ;
            break;
        case INT_BYADR_IDENT:
        case CHAR_BYADR_IDENT:
            e->cate = BYVAL_OBJ;
            break;
        default:
            e->cate = NOP_OBJ;
    }

    switch(idp->kind){
        case INT_FUN_IDENT:
        case INT_CONST_IDENT:
        case INT_VAR_IDENT:
        case INT_ARRVAR_IDENT:
	    case INT_BYVAL_IDENT:
	    case INT_BYADR_IDENT:
            e->type = INT_TYPE;
            break;
        case CHAR_FUN_IDENT:
	    case CHAR_CONST_IDENT:
	    case CHAR_VAR_IDENT:
	    case CHAR_ARRVAR_IDENT:
	    case CHAR_BYVAL_IDENT:
	    case CHAR_BYADR_IDENT:
            e->type = CHAR_TYPE;
            break;
        default:
            e->type = VOID_TYPE;
    }
    switch(e->cate) {
        case NOP_OBJ:
        case CONST_OBJ:
            break;
        case VAR_OBJ:
        case PROC_OBJ:
        case FUN_OBJ:
            e->off = stab->varoff;
            stab->varoff++;
            break;
        case BYVAL_OBJ:
        case BYREF_OBJ:
            e->off = stab->argoff;
            stab->argoff++;
            break;
        case ARRAY_OBJ:
            e->off = stab->varoff;
            stab->varoff +=e->arrlen;
            break;
        default:
            unlikely();
    }
    symadd2(stab, e);
    return e;
}

syment_t *symalloc(symtab_t *stab, char *name, cate_t cate, type_t type){
    syment_t *e;
    NEWENTRY(e);
    strcopy(e->name, name);
    e->sid = ++sidcnt;
    e->cate = cate;
    e->type = type;
    switch(e->cate){
        case TMP_OBJ:
            e->off = stab->varoff + stab->tmpoff;
            stab->tmpoff++;
            break;
        case LABEL_OBJ:
        case NUM_OBJ:
        case STR_OBJ:
            break;
        default:
            unlikely();
    }
    e->stab = stab;
    putsym(stab, e);
    return e;
}








