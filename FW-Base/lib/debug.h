#ifndef _FW_DEBUG_H_
#define _FW_DEBUG_H_


#define DEBUG_ENABLE   // when ship out, to get better performance comment this line


#ifdef DEBUG_ENABLE
#define assert(expr) \
do { \
	if (unlikely(!(expr))) { \
		printk(KERN_ERR "Assertion failed! %s,%s,%s,line=%d\n", \
		#expr, __FILE__, __func__, __LINE__); \
	} \
} while (0)
#else
#define assert(expr) do {} while (0)
#endif


#endif
