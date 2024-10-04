#ifndef _SYNTAX_H_
#define _SYNTAX_H_
#include "parse.h"
#include "limits.h"
/*program*/
struct _pgm_node{
    int nid;
    block_node_t *bp;//block 块，可能指向一个代码块或者作用域，函数体，循环体，条件语句块
    ident_node_t *entry;
};

struct _block_node{
    int nid;
    const_dec_node_t *cdp; //常量声明
    var_dec_node_t *vdp; //变量声明
    pf_dec_list_node_t *pfdlp; //函数列表声明
    comp_stmt_node_t*csp;
};

struct _const_dec_node{
    int nid;
    const_def_node_t *cdp;//常量定义
    const_dec_node_t *next; //下一个声明节点
};

struct _const_def_node{
    int nid;
    ident_node_t *idp; //这里就指向具体的内容
};

struct _var_dec_node{
    int nid;
    var_def_node_t *vdp;
    var_dec_node_t *next;
};

struct _var_def_node{
    int nid;
    ident_node_t *idp;
    var_def_node_t *next;//存疑,这里总不可能定义多个值吧  
};

typedef enum _pf_dec_enum { FUN_PFDEC, PROC_PFDEC } pf_dec_t;

struct _pf_dec_list_node{
    int nid;
    pf_dec_t kind;
    proc_dec_node_t *pdp;
    fun_dec_node_t *fdp;
    pf_dec_list_node_t *next;
};

struct _proc_dec_node{
    int nid;
    proc_def_node_t *pdp;
    proc_dec_node_t *next;
};
struct _proc_def_node {
	int nid;
	proc_head_node_t *php; //头部声明部分
	block_node_t *bp;//主体部分
};

struct _proc_head_node{
    int nid;
    ident_node_t *idp;//标注是什么类型
    para_list_node_t *plp;// 标注参数列表
};

struct  _fun_dec_node{
    int nid;
    fun_def_node_t *fdp;
    fun_dec_node_t *next;
};

struct _fun_def_node{
    int nid;
    fun_head_node_t *fhp;
    block_node_t *bp;
};

struct _fun_head_node{
    int nid;
    ident_node_t *idp;
    para_list_node_t *plp;
};

typedef enum _stmt_enum{
    ASSGIN_STMT,
    IF_STMT,
    REPEAT_STMT,
    PCALL_STMT,
    COMP_STMT, //这里存疑
    READ_STMT,
    WRITE_STMT,
    FOR_STMT,
    NULL_STMT,
}stmt_t;

struct _stmt_node{
    int nid;
    stmt_t kind;
	assign_stmt_node_t *asp;
	if_stmt_node_t *ifp;
	repe_stmt_node_t *rpp;
	for_stmt_node_t *frp;
	pcall_stmt_node_t *pcp;
	comp_stmt_node_t *cpp;
	read_stmt_node_t *rdp;
	write_stmt_node_t *wtp;
};
/*一种是普通赋值，一种是通过函数返回值赋值，一种是数组赋值*/
typedef enum _assign_enum {NORM_ASSGIN, FUN_ASSIGN, ARRAY_ASSIGN } assign_t;

struct _assign_stmt_node{
    int nid;
    assign_t kind;
    ident_node_t *idp;//这里应该是指向最终的节点的值 a = b + c
    expr_node_t *lep;
    expr_node_t *rep;
};

struct _if_stmt_node{
    int nid;
    cond_node_t *cp;
    /*then*/
    stmt_node_t *tp;
    /*else*/
    stmt_node_t *ep;
};

struct _repe_stmt_node {
    int nid;
    stmt_node_t *sp; // 表达式
    cond_node_t *cp; //条件语句
};

typedef enum _for_enum { TO_FOR ,DOWNTO_FOR } for_t;

struct _for_stmt_node {
    int nid;
    for_t kind;
    ident_node_t *idp;
    expr_node_t *lep;
    expr_node_t *rep;
    stmt_node_t *sp; //类比for
};

struct _pcall_stmt_node {
    int nid;
    ident_node_t *idp;
    arg_list_node_t *alp;
};

struct _fcall_stmt_node {
    int nid;
    ident_node_t *idp;
    arg_list_node_t *alp;
};

struct _comp_stmt_node {
    int nid;
    stmt_node_t *sp;
    comp_stmt_node_t *next;
};

struct _read_stmt_node {
    int nid;
    ident_node_t *idp;
    read_stmt_node_t *next;
};

typedef enum _write_enum { STRID_WRITE, STR_WRITE, ID_WRITE} write_t;

struct _write_stmt_node {
    int nid;
    write_t type;
    char sp[MAXSTRLEN];
    expr_node_t *ep;
};


typedef enum _addop_enum {
    NOP_ADDOP,
    NEG_ADDOP,
    ADD_ADDOP,
    MINUS_ADDOP,
}addop_t;

struct _expr_node {
    int nid;
    addop_t kind;
    term_node_t *tp;
    expr_node_t *next;
};

typedef enum multop_enum { NOP_MULTOP, MULT_MULTOP, DIV_MULTOP} multop_t;

struct _term_node {
    int nid;
    multop_t kind;
    factor_node_t *fp;
    term_node_t *next;
};

typedef enum _factor_enum {
    ID_FACTOR,
    ARRAY_FACTOR,
    UNSIGN_FACTOR,
    CHAR_FACTOR,
    EXPR_FACTOR,
    FUNCALL_FACTOR,
}factor_t;

struct _factor_node {
    int nid;
    factor_t kind;
    ident_node_t *idp;
    expr_node_t *ep;
    int value;
    fcall_stmt_node_t *fcsp;
};

typedef enum _rela_enum {
    EQU_RELA,
    NEQ_RELA,
    GTT_RELA,
    GEQ_RELA,
    LST_RELA,
    LEQ_RELA,
}rela_t;

struct _cond_node {
    int nid;
    expr_node_t *lep;
    rela_t kind;
    expr_node_t *rep;
};

typedef enum _ident_enum {
    INIT_IDENT,
    PROC_IDENT,
    INT_FUN_IDENT,
    CHAR_FUN_IDENT,
    INT_CONST_IDENT,
    CHAR_CONST_IDENT,
    INT_VAR_IDENT,
    CHAR_VAR_IDENT,
    INT_ARRVAR_IDENT,
    CHAR_ARRVAR_IDENT,
    INT_BYVAL_IDENT,
    CHAR_BYVAL_IDENT,
    INT_BYADR_IDENT,
    CHAR_BYADR_IDENT,
}idekind_t;

struct _ident_node {
	int nid;
	idekind_t kind;
	char name[MAXSTRLEN];
	int value;
	int length;
	int line;
};

struct _para_list_node {
	int nid;
	para_def_node_t *pdp;
	para_list_node_t *next;
};
struct _para_def_node {
	int nid;
	ident_node_t *idp;
	para_def_node_t *next;
};


struct _arg_list_node {
	int nid;
	expr_node_t *ep;
	arg_list_node_t *next;
	// link to referred variable or array
	expr_node_t *idx; // array reference index
};
#endif