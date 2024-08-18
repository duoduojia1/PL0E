#ifndef _DEBUG_H_
#define _DEBUG_H_
#define dbg(fmt, args...)                                                      \
	do {                                                                  \
		if (echo) {                                                    \
			printf("%s:%d %s(): " fmt, __FILE__, __LINE__,         \
			       __func__, ##args);                              \
		}                                                              \
	} while (0)
#endif