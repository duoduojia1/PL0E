#ifndef _ERROR_H_
#define _ERROR_H_

#define EPANIC 996
#define EABORT 997



#define chkerr(fmt)                                                            \
	do {                                                                   \
		if (errnum > 0) {                                              \
			printf(fmt);                                           \
			printf("\n");                                          \
			exit(errnum);                                          \
		}                                                              \
	} while (0)


    
#endif