#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"
#include "lexical.h"
#include "limits.h"
#include "scan.h"
#include "debug.h"
#include "syntax.h"
#include "common.h"
#include "global.h"

pgm_node_t *pgm;

int nidcnt=0;


static token_t currtok;
static token_t prevtok;
static char prevtokbuf[MAXTOKSIZE+1];
static int prevlineno;

static void match(token_t expected){
    if(currtok != expected){
        char buf[MAXSTRLEN];
        sprintf(buf,"UNEXPECTED_TOKEN:LINE%d [%s]",lineno,tokenbuf);
        panic(buf);
    }
    strcpy(prevtokbuf,tokenbuf);
    prevtok = currtok;
    prevlineno = toklineno;

    currtok = gettok();
}


static pgm_node_t *parse_pgm(void){
    pgm_node_t *t;
    NEWNODE(pgm_node_t, t);
    ident_node_t *entry;
    NEWNODE(ident_node_t, entry);
    entry->kind = INT_FUN_IDENT;
    entry->value = 0;
    entry->line = 0;
    entry->length = 0;
    strcpy(entry->name, MAINFUNC);
    t->entry = entry;
    t->bp = parse_block();
    match(SS_DOT); // 注意这是递归写法，这个已经是识别最后的结束符 .
    return t;
}

static block_node_t *parse_block(void){
    block_node_t *t;
    NEWNODE(block_node_t, t);
    if (TOKANY1(KW_CONST)) {
        t->cdp = parse_const_dec();
    }
    if (TOKANY1(KW_VAR)) {
        t->vdp = parse_var_dec();
    }
    if (TOKANY2(KW_FUNCTION, KW_PROCEDURE)) {
        t->pfdlp = parse_pf_dec_list();
    }
    if (TOKANY1(KW_BEGIN)) {
        t->csp = parse_comp_stmt();
    }
    return t;
}

static const_dec_node_t *parse_const_dec(){
    const_dec_node_t *t, *p, *q;
    NEWNODE(const_dec_node_t, t);
    match(KW_CONST);
    t->cdp = parse_const_def();

    for(p=t; TOKANY(SS_COMMA); p = q) {
        match(SS_COMMA); //识别掉逗号
        NEWNODE(const_dec_node_t, q);
        p->next = q;
        q->cdp = parse_const_def();
    }
    match(SS_SEMI);
    
    return t;

}


static const_def_node_t *parse_const_def(){
    const_def_node_t *t;
    NEWNODE(const_def_node_t, t);

    if(TOKANY1(MC_ID)){
        t->idp = parse_ident(READCURR);
    }

    match(SS_EQU);
    if(TOKANY4(SS_PLUS, SS_MINUS, MC_UNS, MC_CH)){
        switch (currtok)
        {
            case SS_PLUS:
                match(SS_PLUS);
                t->idp->kind = INT_CONST_IDENT;
                t->idp->value = atoi(tokenbuf);
                match(MC_UNS);
                break;

            case SS_MINUS:
                match(SS_MINUS);
                t->idp->kind = INT_CONST_IDENT;
                t->idp->value = -atoi(tokenbuf);
                match(MC_UNS);
                break;
            case MC_UNS:
                t->idp->kind = INT_CONST_IDENT;
                t->idp->value = atoi(tokenbuf);
                match(MC_UNS);
                break;
            case MC_CH:
                t->idp->kind = CHAR_CONST_IDENT;
                t->idp->value = (int)tokenbuf[0];
                match(MC_CH);
                break;
            default:
                unlikely();
        }
    }
    else{
        unlikely();
    }
    return t;
}


static var_dec_node_t *parse_var_dec(void){
    var_dec_node_t *t, *p, *q;
    NEWNODE(var_dec_node_t, t);
    match(KW_VAR);
    t->vdp = parse_var_def();
    match(SS_SEMI); //识别分号
    for(p = t;TOKANY(MC_ID); p = q ){
        p->next = q;
        p->vdp = parse_var_def();
        match(SS_SEMI);
    }
}

// SS_COMMA是逗号，SS_COLON是冒号
// var x,y : interger;
static var_def_node_t *parse_var_def(void){
    var_def_node_t *t, *p, *q;
    NEWNODE(var_def_node_t, t);
    int arrlen = 0;
    t->idp = parse_ident(READCURR);
    for (p = t; TOKANY1(SS_COMMA); p = q) {
        match(SS_COMMA);
        NEWNODE(var_def_node_t, q);
        p->next = q;
        q = parse_ident(READCURR); //这里识别完 x,y,z...
    }
    match(SS_COLON);

    switch (currtok)
    {
    case KW_INTEGER:
        match(KW_INTEGER);
        for (p = t; p; p = q) {
            p->idp->kind = INT_VAR_IDENT;
        }
        break;
    case KW_CHAR:
        match(KW_CHAR);
        for (p = t;p; p = q) {
            p->idp->kind = CHAR_VAR_IDENT;
        }
    // var arr1,arr2 : arr[3] of interger or char
    case KW_ARRAY:
        match(KW_ARRAY);
        match(SS_LBRA); //识别掉左方括号
        if (TOKANY1(MC_UNS)){
            arrlen = atoi(tokenbuf);
            match(MC_UNS);
        }
        else {
            unlikely();
        }
        match(SS_RBRA); //识别掉右方括号
        match(KW_OF); //识别掉 关键词 OF
        if (TOKANY1(KW_INTEGER)){
            match(KW_INTEGER);
            for (p = t;p; p = q) {
                p->idp->kind = INT_ARRVAR_IDENT;
                p->idp->length = arrlen;
            }
        }
        else if (TOKANY1(KW_CHAR)){
            match(KW_CHAR);
            for (p = t;p; p = q) {
                p->idp->kind = CHAR_ARRVAR_IDENT;
                p->idp->length = arrlen;
            }
        }
        else {
            unlikely();
        }
    default:
        unlikely();
    }
}

/*
Pascal

procedure proc:
begin:
    //procedure body
end;

function func : integer;
begin:
    //procedure body
end;

*/

static pf_dec_list_node_t *parse_pf_dec_list(void){
    pf_dec_list_node_t *t,*p,*q;

    for(p = NULL; TOKANY2(KW_PROCEDURE,KW_FUNCTION); p = q){
        NEWNODE(pf_dec_list_node_t, q);
        if(!p){
            p = q;
            t = p;
        }
        else{
            p->next = q;
        }
        switch(currtok){
            case KW_PROCEDURE:
                q->kind = PROC_PFDEC;
                q->pdp = parse_proc_dec();
                break;
            case KW_FUNCTION:
                q->kind = FUN_PFDEC;
                q->fdp = parse_fun_dec();
                break;
            default:
                unlikely();
        }
    }
    return t;
}

static proc_dec_node_t *parse_proc_dec(void){
    proc_dec_node_t *t,*p,*q;
    NEWNODE(proc_dec_node_t, t);
    t->pdp = parse_proc_def();
    match(SS_SEMI);
    for(p = t;TOKANY1(KW_PROCEDURE); p = q){
        NEWNODE(proc_dec_node_t, q);
        q->pdp = parse_proc_def();
        p->next = q;
        match(SS_SEMI); 
    }
    return t;
}


static proc_def_node_t *parse_proc_def(void){
    proc_def_node_t *t;
    NEWNODE(proc_def_node_t, t);
    t->bp = parse_block();
    t->php = parse_proc_head();
    return t;
}


/*
procedure proc:
begin:
    //procedure body
end;
procedure(var x:Integer)表示引用传递
procedure(x:Integer) 表示值传递
*/

static proc_head_node_t *parse_proc_head(void){
    proc_head_node_t  *t;
    NEWNODE(proc_head_node_t, t);
    match(KW_PROCEDURE);
    t->idp = parse_ident(READCURR);
    t->idp->kind = PROC_IDENT;
    match(SS_LPAR);
    if(TOKANY2(KW_VAR, MC_ID)){
        t->plp = parse_para_list();
    }
    match(SS_RPAR);
    match(SS_SEMI);
    return t;
}

static fun_dec_node_t *parse_fun_dec(void){
    fun_dec_node_t *t,*q,*p;
    NEWNODE(fun_dec_node_t ,t);
    t->fdp = parse_fun_def();
    match(SS_SEMI);
    for(p = t; TOKANY1(KW_FUNCTION); p = q){
        NEWNODE(fun_dec_node_t, q);
        p->next = q;
        q->fdp = parse_fun_def();
        match(SS_SEMI);
    }
    return t;
}

static fun_def_node_t *parse_fun_def(void){
    fun_def_node_t *t;
    NEWNODE(fun_def_node_t, t);
    t->fhp = parse_fun_head();
    t->bp = parse_block();
    return t;
}



/*
function func : integer;
begin:
    //procedure body
end;
*/
static fun_head_node_t *parse_fun_head(void){
    fun_head_node_t *t;
    NEWNODE(fun_head_node_t, t);
    match(KW_FUNCTION);
    t->idp = parse_ident(READCURR);
    match(SS_LPAR);
    if(TOKANY2(KW_VAR, MC_ID)){
        t->plp = parse_para_list();
    }
    match(SS_RPAR);
    match(SS_COLON);
    switch(currtok){
        case KW_INTEGER:
            match(KW_INTEGER);
            t->idp->kind = INT_FUN_IDENT;
            break;
        case KW_CHAR:
            match(KW_CHAR);
            t->idp->kind = CHAR_FUN_IDENT;
            break;
        default:
            unlikely();
    }
    match(SS_SEMI);
    return t;
}

static stmt_node_t *parse_stmt(void){
    stmt_node_t *t;
    NEWNODE(stmt_node_t, t);
    switch(currtok){
        case IF_STMT:
            t->kind = IF_STMT;
            t->ifp = parse_if_stmt();
            break;
        case KW_REPEAT:
            t->kind = REPEAT_STMT;
            t->rpp = parse_repe_stmt();
            break;
        case KW_BEGIN:
            t->kind = COMP_STMT;
            t->cpp = parse_comp_stmt();
            break;
        case KW_READ:
            t->kind = READ_STMT;
            t->rdp = parse_read_stmt();
            break;
        case KW_WRITE:
            t->kind = WRITE_STMT;
            t->wtp = parse_write_stmt();
            break;
        case KW_FOR:
            t->kind = FOR_STMT;
            t->frp = parse_for_stmt();
            break;
        case MC_ID:
            match(MC_ID);
            if(TOKANY1(SS_LPAR)){
                t->kind = PCALL_STMT;
                t->pcp = parse_pcall_stmt();
            }
            else if(TOKANY2(SS_ASGN, SS_LBRA)) {
                t->kind = ASSGIN_STMT;
                t->asp = parse_assign_stmt();
            }
            else{
                unlikely();
            }
            break;
        default:
            t->kind = NULL_STMT;
            break;
    }
    return t;
}



