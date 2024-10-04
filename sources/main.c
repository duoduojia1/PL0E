#include "parse.h"
#include "init.h"
int main(){
    source=fopen("/home/suzichuan/PL01/PL0E/sources/test.txt","r");
    pgm_node_t *pgm = parse();
    return 0;
}