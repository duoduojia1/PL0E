#ifndef _DEBUG_H_
#define _DEBUG_H_
#include "error.h"



void quit(char *file,int line,const char *func, int errno,char *msg);
#define dbg(fmt, args...)                                                      \
	do {                                                                  \
		if (echo) {                                                    \
			printf("%s:%d %s(): " fmt, __FILE__, __LINE__,         \
			       __func__, ##args);                              \
		}                                                              \
	} while (0)


#define msg(fmt, args...)                      \
	do{                                         \
		if (!silent) {                            \
			printf(fmt, ##args);                 \
		}                                       \
	}while(0);          						\
	
	
#define panic(msg) quit(__FILE__, __LINE__, __func__, EPANIC, msg);
#define unlikely() quit(__FILE__,__LINE__,__func__, EABORT, "unlikely case");
#endif