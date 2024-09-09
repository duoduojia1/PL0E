#include "scan.h"
extern int nidcnt;
extern void match(token_t expected);


int main(){
    source=fopen("/home/duoduo/PL01/Test/test.txt","r");
    token_t tok = KW_PROCEDURE;
    int z=3;
    do{
        match(tok);
    }while(z--);
    fclose(source);
    return 0;
}