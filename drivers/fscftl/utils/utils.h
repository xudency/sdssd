#ifndef _FSCFTL_UTILIS_H_
#define _FSCFTL_UTILIS_H_

#define BIT_TEST(f, b)   			(0 != ((f)&(1<<b)))
#define BIT_SET(f, b)    			((f) |= (1<<b))
#define BIT_CLEAR(f, b)  			((f) &= ~(1<<b))
#define BIT2MASK(width) 			((1UL<<(width))-1)

#endif

