#ifndef _FSCFTL_UTILIS_H_
#define _FSCFTL_UTILIS_H_

#define TRACE_TAG(fmt, arg...) \
	printk("%s-%d"fmt"\n", __FUNCTION__, __LINE__, ##arg)


#define BIT_TEST(f, bitnum)   			(0 != ((f)&(1<<bitnum)))
#define BIT_SET(f, bitnum)    			((f) |= (1<<bitnum))
#define BIT_CLEAR(f, bitnum)  			((f) &= ~(1<<bitnum))
#define BIT2MASK(width) 			((1UL<<(width))-1)

#endif

