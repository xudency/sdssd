#ifndef _FW_DEBUG_H_
#define _FW_DEBUG_H_


#define DEBUG_ENABLE   // when ship out, to get better performance comment this line
 

#define print_err(fmt, arg...)  printk("[%s][%d]"fmt"\n", __FUNCTION__, __LINE__, ##arg)


#ifdef DEBUG_ENABLE

#define print_dbg(fmt, arg...)  printk("[%s][%d]"fmt"\n", __FUNCTION__, __LINE__, ##arg)

#define assert(expr) \
do { \
	if (unlikely(!(expr))) { \
		printk(KERN_ERR "Assertion failed! %s,%s,%s,line=%d\n", \
		#expr, __FILE__, __func__, __LINE__); \
	} \
} while (0)

///////////////////////////////
#else

#define print_dbg(fmt, arg...) 

#define assert(expr) do {} while (0)

#endif
///////////////////////////////

#endif
