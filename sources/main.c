#include "scan.h"

int main(){
    source=fopen("/home/duoduo/PL01/Test/test.txt","r");
    
    while(1){
        token_t curr=gettok();
        if(curr == ERROR ||curr == ENDFILE) break;
        printf("%d,%s\n",curr,tokenbuf);
    }
    fclose(source);
    return 0;
}