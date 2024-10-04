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
#include "error.h"

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

    for(p=t; TOKANY1(SS_COMMA); p = q) {
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
    for(p = t;TOKANY1(MC_ID); p = q){
        NEWNODE(var_dec_node_t, q);
        p->next = q;
        q->vdp = parse_var_def();
        match(SS_SEMI);
    }
    return t;
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
        q->idp = parse_ident(READCURR); //这里识别完 x,y,z...
    }
    match(SS_COLON);

    switch (currtok)
    {
    case KW_INTEGER:
        match(KW_INTEGER);
        for (p = t; p; p = p->next) {
            p->idp->kind = INT_VAR_IDENT;
        }
        break;
    case KW_CHAR:
        match(KW_CHAR);
        for (p = t; p; p = p->next) {
            p->idp->kind = CHAR_VAR_IDENT;
            p->next = q;
        }
        break;
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
            for (p = t;p; p = p->next) {
                p->idp->kind = INT_ARRVAR_IDENT;
                p->idp->length = arrlen;
            }
        }
        else if (TOKANY1(KW_CHAR)){
            match(KW_CHAR);
            for (p = t;p; p = p->next) {
                p->idp->kind = CHAR_ARRVAR_IDENT;
                p->idp->length = arrlen;
            }
        }
        else {
            unlikely();
        }
        break;
    default:
        unlikely();
    }
    return t;
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
    t->php = parse_proc_head();
    t->bp = parse_block();
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
        case KW_IF:
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


/*
assignstmt ->
 ident := express | funident := expression | ident '[' expression ']' := expression
*/

static assign_stmt_node_t *parse_assign_stmt(void){
    assign_stmt_node_t *t;
    NEWNODE(assign_stmt_node_t, t);
    switch(currtok){
        case SS_ASGN:
            t->kind = NORM_ASSGIN;
            t->idp = parse_ident(READPREV);
            match(SS_ASGN);
            t->lep = NULL;
            t->rep = parse_expr();
            break;

        case SS_LBRA:
            t->kind = ARRAY_ASSIGN;
            t->idp = parse_ident(READPREV);
            match(SS_LBRA);
            t->lep = parse_expr();
            match(SS_RBRA);
            match(SS_ASGN);
            t->rep = parse_expr();
            break;
        default:
            unlikely();
    }
    return t;
}

/*
ifstmt->
    IF condition THEN statement |
    IF condition THEN statement ELSE statement
*/


static if_stmt_node_t *parse_if_stmt(void){
    if_stmt_node_t *t;
    NEWNODE(if_stmt_node_t, t);
    match(KW_IF);
    t->cp = parse_cond();
    match(KW_THEN);
    t->tp = parse_stmt();
    if(TOKANY1(KW_ELSE)){
        match(KW_ELSE);
        t->ep = parse_stmt();
    }
    return t;
}

/*
REPEAT statment UNTIL condition
*/

static repe_stmt_node_t *parse_repe_stmt(void){
    repe_stmt_node_t *t;
    NEWNODE(repe_stmt_node_t, t);
    match(KW_REPEAT);
    t->sp = parse_stmt();
    match(KW_UNTIL);
    t->cp = parse_cond();
    return t;
}

/*
    forstmt ->
    FOR indent := expression (TO | DOWNTO) expression DO statement
*/
static for_stmt_node_t *parse_for_stmt(void){
    for_stmt_node_t *t;
    NEWNODE(for_stmt_node_t ,t);
    match(KW_FOR);
    t->idp = parse_ident(READCURR);
    match(SS_ASGN);
    t->lep = parse_expr();
    switch(currtok){
        case KW_TO:
            match(KW_TO);
            t->kind = TO_FOR;
            break;
        case KW_DOWNTO:
            match(KW_DOWNTO);
            t->kind = DOWNTO_FOR;
            break;
        default:
            unlikely();
    }
    t->rep = parse_expr();
    match(KW_DO);
    t->sp = parse_stmt();
    return t;
}

/*
    pcallstmt -> ident '(' [argist] ')' 
*/
static pcall_stmt_node_t *parse_pcall_stmt(void){
    pcall_stmt_node_t *t;
    NEWNODE(pcall_stmt_node_t, t);
    t->idp = parse_ident(READPREV);
    match(SS_LPAR);
    if(TOKANY6(MC_ID, MC_CH, SS_PLUS, SS_MINUS, MC_UNS, SS_LPAR)){
        t->alp = parse_arg_list();
    }
    match(SS_RPAR);
    return t;
}

/*
    fcall -> ident '(' [arglist] ')'

*/

static fcall_stmt_node_t *parse_fcall_stmt(void){
    fcall_stmt_node_t *t;
    NEWNODE(fcall_stmt_node_t, t);
    t->idp = parse_ident(READPREV);
    match(SS_LPAR);
    if(TOKANY6(MC_ID, MC_CH, SS_PLUS, SS_MINUS, MC_UNS, SS_LPAR)){
        t->alp = parse_arg_list();
    }
    match(SS_RPAR);
    return t;
}

/*
    compstmt ->
    BGEGIN statement {,statement} END
*/

static comp_stmt_node_t *parse_comp_stmt(void){
    comp_stmt_node_t *t,*p,*q;
    match(KW_BEGIN);
    NEWNODE(comp_stmt_node_t, t);
    t->sp = parse_stmt();
    for(p = t; TOKANY1(SS_SEMI); p = q){
        match(SS_SEMI);
        NEWNODE(comp_stmt_node_t, q);
        q->sp = parse_stmt();
        p->next = q;
    }
    match(KW_END);
    return t;
}


/*
    readstmt->
    READ '(' ident {, ident} ')'
*/

static read_stmt_node_t *parse_read_stmt(void){
    read_stmt_node_t *t,*p,*q;
    NEWNODE(read_stmt_node_t, t);
    match(KW_READ);
    match(SS_LPAR);
    t->idp = parse_ident(READCURR);
    NEWNODE(read_stmt_node_t, t);
    for(p = t; TOKANY1(SS_COLON); p = q){
        match(SS_COLON);
        NEWNODE(read_stmt_node_t, q);
        q->idp = parse_ident(READCURR);
        p->next = q;
    }
    match(SS_RPAR);
    return t;
}



/*
    writestmt->
        WRITE '(' string, expression ')' | WRITE '(' string ')' | WRITE '(' expression ')' 
*/

static write_stmt_node_t *parse_write_stmt(void) {
    write_stmt_node_t *t;
    NEWNODE(write_stmt_node_t, t);
    match(KW_WRITE);
    match(SS_LPAR);
    if(TOKANY1(MC_STR)){
        match(MC_STR);
        t->type = STR_WRITE;
        strcpy(t->sp, tokenbuf);
    }
    else if(TOKANY6(MC_ID, MC_CH, SS_PLUS, SS_MINUS, MC_UNS, SS_LPAR)){
        t->type = ID_WRITE;
        t->ep = parse_expr();
    }
    else{
        unlikely();
    }
    if(TOKANY1(SS_COMMA) && (t->type == STR_WRITE)){
        t->type = STRID_WRITE;
        match(SS_COMMA);
        t->ep = parse_expr();
    }
    match(SS_RPAR);
    return t;
}

/*
    expression -> [+|-] term {addop term}
*/

static expr_node_t *parse_expr(void){
    expr_node_t *t, *p, *q;
    NEWNODE(expr_node_t, t);
    switch(currtok){
        case SS_PLUS:
            match(SS_PLUS);
            t->kind = ADD_ADDOP;
            t->tp = parse_term();
            break;
        case SS_MINUS:
            match(SS_MINUS);
            t->kind = MINUS_ADDOP;
            t->tp = parse_term();
            break;
        case MC_ID:   // no match?
        case MC_UNS:
        case MC_CH:
        case SS_LPAR: // 
            t->kind = NOP_ADDOP;
            t->tp = parse_term();
            break;
        default:
            unlikely();
    }
    for(p = t;TOKANY2(SS_PLUS, SS_MINUS); p = q){
        NEWNODE(expr_node_t, q);
        p->next = q;
        switch(currtok){
            case SS_PLUS:
                match(SS_PLUS);
                t->kind = ADD_ADDOP;
                t->tp = parse_term();
                break;
            case SS_MINUS:
                match(SS_MINUS);
                t->kind = MINUS_ADDOP;
                t->tp = parse_term();
                break;
            default:
                unlikely();
        }
    }
    return t;
}

/*
    term -> factor {multop factor}
*/ 

static term_node_t *parse_term(void) {
    term_node_t *t, *p, *q;
    NEWNODE(term_node_t, t);
    t->kind = NOP_MULTOP;
    t->fp = parse_factor();
    for(p = t; TOKANY2(SS_STAR, SS_OVER); p = q) {
        NEWNODE(term_node_t, q);
        p->next = q;
        switch(currtok){
            case SS_STAR:
                match(SS_STAR);
                t->kind = MULT_MULTOP;
                t->fp = parse_factor();
                break;
            case SS_OVER:
                match(SS_OVER);
                t->kind = DIV_MULTOP;
                t->fp = parse_factor();
                break;
            default:
                unlikely();
        }
    }
    return t;
}


/*
factor -> ident | ident '[' expression ']' | unsign | '(' expression ')' | fcallstmt
*/

static factor_node_t *parse_factor(void) {
    factor_node_t *t;
    NEWNODE(factor_node_t ,t);
    switch(currtok){
        case MC_UNS:
            t->kind = UNSIGN_FACTOR;
            t->value = atoi(tokenbuf);
            match(MC_UNS);
            break;
        case MC_CH:
            t->kind = CHAR_FACTOR;
            t->value = (int)tokenbuf[0];
            match(MC_CH);
            break;
        case SS_LPAR:
            match(SS_LPAR);
            t->kind = EXPR_FACTOR;
            t->ep = parse_expr();
            match(SS_RPAR);
            break;
        case MC_ID:
            match(MC_ID);
            if(TOKANY1(SS_LBRA)) {
                t->kind = ARRAY_FACTOR;
                t->idp = parse_ident(READPREV);
                match(SS_LBRA);
                t->ep = parse_expr();
                match(SS_RBRA);
            }
            else if(TOKANY1(SS_LPAR)){
                t->kind = FUNCALL_FACTOR;
                t->fcsp = parse_fcall_stmt();
            }
            else{
                t->kind = ID_FACTOR;
                t->idp = parse_ident(READPREV);
            }
            break;
        default:
            unlikely();
    }
    return t;
}


/*
    condition -> expression relop expression
*/

static cond_node_t *parse_cond(void) {
    cond_node_t *t;
    NEWNODE(cond_node_t, t);
    t->lep = parse_expr();
    switch(currtok){
        case SS_EQU:
            t->kind = EQU_RELA;
            match(SS_EQU);
            break;
        case SS_LST:
            t->kind = LST_RELA;
            match(SS_LST);
            break;
        case SS_GTT:
            t->kind = GTT_RELA;
            match(SS_GTT);
            break;
        case SS_GEQ:
            t->kind = GEQ_RELA;
            match(SS_GEQ);
            break;
        case SS_LEQ:
            t->kind = LEQ_RELA;
            match(SS_LST);
            break;
        case SS_NEQ:
            t->kind = NEQ_RELA;
            match(SS_NEQ);
            break;
        default:
            unlikely();
    }
    t->rep = parse_expr();
    return t;
}

/*
    construct indentifier
*/

static ident_node_t *parse_ident(idreadmode_t mode){
    ident_node_t *t;
    NEWNODE(ident_node_t, t);
    switch(mode){
        case READCURR:
            t->kind = INIT_IDENT;
            strcpy(t->name, tokenbuf);
            t->value = 0;
            t->length = 0;
            t->line = lineno;
            match(MC_ID);
            break;
        case READPREV:
            t->kind = INIT_IDENT;
            strcpy(t->name, prevtokbuf);
            t->value = 0;
            t->length = 0;
            t->line = prevlineno;
            break;
        default:
            unlikely();
    }
    return t;
}

/*
    paralist -> paradef {; paradef }
*/

static para_list_node_t *parse_para_list(void) {
    para_list_node_t *t,*q,*p;
    NEWNODE(para_list_node_t, t);
    t->pdp = parse_para_def();
    for(p = t;TOKANY1(SS_SEMI);p = q){
        match(SS_SEMI);
        NEWNODE(para_list_node_t, q);
        p->next = q;
        q->pdp = parse_para_def();
    }
    return t;
}


/*
    paradef -> [var] ident {, ident} basictype
*/

static para_def_node_t *parse_para_def(void) {
    para_def_node_t *t, *p, *q;
    NEWNODE(para_def_node_t, t);
    bool is_adr = false;
    if(TOKANY1(KW_VAR)) {
        is_adr = true;
        match(KW_VAR);
    }
    t->idp = parse_ident(READCURR);
    for(p = t;TOKANY1(SS_COMMA);p = q) {
        match(SS_COMMA);
        NEWNODE(para_def_node_t, q);
        q->idp = parse_ident(READCURR);
        p->next = q;
    }
    match(SS_COLON);
    switch(currtok){
        case KW_INTEGER:
            match(KW_INTEGER);
            for(p = t;p;p = p->next){
                p->idp->kind = is_adr ? INT_BYADR_IDENT : INT_BYVAL_IDENT;
            }
            break;
        case KW_CHAR:
            match(KW_CHAR);
            for(p = t;p;p = p->next){
                p->idp->kind = is_adr ? CHAR_BYADR_IDENT : CHAR_BYVAL_IDENT;
            }
            break;
        default:
            unlikely();
    }
    return t;
}

/*
    arglist -> argument {,argument}
    argument ->expression
*/


static arg_list_node_t *parse_arg_list(void) {
    arg_list_node_t *t,*p,*q;
    NEWNODE(arg_list_node_t, t);
    t->ep = parse_expr();
    for(p = t; TOKANY1(SS_COMMA); p = q){
        match(SS_COMMA);
        NEWNODE(arg_list_node_t, q);
        p->ep = parse_expr();
        p->next = q;
    }
    return t;
}


pgm_node_t *parse(void){
    currtok = gettok(); //起始token
    pgm = parse_pgm();
    return pgm;
}






