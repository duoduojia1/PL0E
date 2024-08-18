#include "scan.h"
int lineno = 0;
int colmno = 0;
char linebuf[MAXLINEBUF]; 
char tokenbuf[MAXTOKSIZE + 1];
bool fileend = false;
int bufsize=0;
int readc(bool peek)
{
	if (colmno < bufsize) {
		goto ready;
	}
	lineno++;
	if (fgets(linebuf, MAXLINEBUF - 1, source) == NULL) {
		fileend = true; 
		return EOF;
	}
	dbg("source L%03d: %s", lineno, linebuf);
	bufsize = strlen(linebuf);
	colmno = 0;
	goto ready;

ready:
	return (peek) ? linebuf[colmno] : linebuf[colmno++];
}


//在分词中有些情况是要回退的
void unreadc(void){
	if(!fileend){
		colmno--;
	}
}


static struct _pl0e_keywords_struct{
	char *str;
	token_t tok;	
}PL0E_KEYWORDS[]={
	{"array",KW_ARRAY},
	{"begin",KW_BGGIN},
	{"char",KW_CHAR},
	{"const",KW_CONST},
	{ "do", KW_DO },
	{ "downto", KW_DOWNTO },
	{ "else", KW_ELSE },
	{ "end", KW_END },
	{ "for", KW_FOR },
	{ "function", KW_FUNCTION },
	{ "if", KW_IF },
	{ "integer", KW_INTEGER },
	{ "of", KW_OF },
	{ "procedure", KW_PROCEDURE },
	{ "read", KW_READ },
	{ "repeat", KW_REPEAT },
	{ "then", KW_THEN },
	{ "to", KW_TO },
	{ "until", KW_UNTIL },
	{ "var", KW_VAR },
	{ "write", KW_WRITE }
};

static token_t getkw(char *s){
	for(int i = 0;i <= 20;i++){
		if(!strcmp(s,PL0E_KEYWORDS[i].str)){
			return PL0E_KEYWORDS[i].tok;
		}
	}
	return MC_ID;
}

token_t gettok(void){
	int i = 0; //存储每一个字符的位置
	token_t curr; //主要是用来判断是不是关键字
	bool save;
	state_t state = START;


	while(state != DONE){
		int ch = readc(0);
		save = true; //特判后面需不需要保存
		switch (state)
		{
		case START:
			if(isspace(ch)){
				save = false;
			}
			else if(ch == '"'){
				save = false;
				state = INSTR;
			}
			else if(isdigit(ch)){
				state = INUNS;
			}
			else if(isalpha(ch)){
				state = INIDE;
			}
			else if(ch == '<'){
				state = INLES;
			}
			else if(ch == ':'){
				state = INCOM;
			}
			else if(ch == '\''){
				save = false;
				state = INCHA;
			}
			else if(ch == '{'){
				save = false;
				state = INCMT;
			}
			else{
				state = DONE; //转到结束或者是运算符
				switch(ch)
				{
				case EOF:
					save = false;
					curr = ENDFILE;
					break;
				case '.':
					curr = SS_DOT;
					break;
				case '+':
					curr = SS_PLUS;
					break;
				case '-':
					curr = SS_MINUS;
					break;
				case '*':
					curr = SS_STAR;
					break;
				case '/':
					curr = SS_OVER;
					break;
				case '=':
					curr = SS_EQU;
					break;
				case ',':
					curr = SS_COMMA;
					break;
				case ';':
					curr = SS_SEMI;
					break;
				case '(':
					curr = SS_LPAR;
					break;
				case ')':
					curr = SS_RPAR;
					break;
				case '[':
					curr = SS_LBRA;
					break;
				case ']':
					curr = SS_RBBR;
					break;
				case '{': //不知道什么时候会跳到这里
					curr = SS_LBBR;
					break;
				case '}':
					curr = SS_RBBR;
					break;
				default:
					curr = ERROR;
					break;
				}
			}
			break;
		
		case INSTR:
			if(ch == '"'){
				state = DONE;
				save = false;
				curr = MC_STR;
			}
			else if(isprint(ch)){ //看是否是能打印的
			}
			else if(!isprint(ch)){
			}
			else{
				state = DONE;
				if(ch == EOF){
					save = false;
					i = 0;
					curr = ENDFILE;
				}
			}
			break;
		case INUNS:
			if(!isdigit(ch)){
				unreadc();
				save = false;
				state = DONE;
				curr = MC_UNS;
			}
			break;
		case INIDE:
			if(!(isdigit(ch)||isalpha(ch))){
				unreadc();
				save = false;
				state = DONE;
				curr = MC_ID;
			}
			break;
		case INLES:
			state = DONE;
			if(ch == '='){
				curr = SS_LEQ;
			}
			else if(ch == '>'){
				curr = SS_NEQ;
			}
			else{
				unreadc();
				save = false;
				curr = SS_LST;
			}
			break;
		case INCOM:
			state = DONE;
			if(ch == '='){
				curr = SS_ASGN;
			}
			else{ // 这里感觉有点问题,单纯一个：是用来解决什么问题？
				unreadc();
				save = false;
				curr = SS_COLON;
			}
			break;
		case INGRE:
			state = DONE;
			if(ch == '='){
				curr = SS_GEQ;
			}
			else{
				unreadc();
				save = false;
				curr = SS_GTT;
			}
			break;
		case INCHA:
			if(ch == '\''){
				state = DONE;
				save = false;
				curr = MC_CH;
			}
			else if((isalpha(ch)||isdigit(ch))){
			}
			else{
				if(ch == EOF){
					save =false;
					i = 0;
					curr = ENDFILE;
					state = DONE;
				}
			}
			break;
		case INCMT:
			save = false;
			if(ch == EOF){
				state = DONE;
				curr = ENDFILE;
			}
			else if(ch == '}'){
				state = START;
			}
			break;
		default:
			state = DONE;
			curr = ERROR;
			break;
		}
		if((save) && (i <= MAXTOKSIZE)){
			tokenbuf[i++] = (char)ch;
			tokenbuf[i]='\0';
		}
		else if(i > MAXTOKSIZE){//之后在处理
		}
		if(state == DONE){
			tokenbuf[i]='\0';//可能save=0，但是直接done了，但是没有结束符。
			if(curr == MC_ID){
				curr = getkw(tokenbuf);
			}
		}
	}
	return curr;
}