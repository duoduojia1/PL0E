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


static var_dec_node_t *parse_var_def(void){
    var_dec_node_t *t, *p, *q;
    NEWNODE(var_dec_node_t, t);
    match(KW_VAR);
    t->vdp = parse_var_def();
    match(SS_SEMI);
    for(p = t;TOKANY(MC_ID); p = q ){
        p->next = q;
        p->vdp = parse_var_def();
        match(SS_SEMI);
    }
}