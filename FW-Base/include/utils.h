#ifndef _UTILS_H_
#define _UTILS_H_


//bit ops
// n is 64bit
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((u32)(n))
#define bit_test(f, bitnum)   			(0 != ((f)&(1<<bitnum)))
#define bit_set(f, bitnum)    			((f) |= (1<<bitnum))
#define bit_clear(f, bitnum)  			((f) &= ~(1<<bitnum))
// TODO: ported bitops from linux/bitops.h and arch/arc/bitops.h


#define MIN(a, b)						(((a) < (b)) ? (a) : (b))
#define MIN_3(a,b,c)					(MIN(a, MIN(b, c)))
#define MAX(a, b)						(((a) < (b)) ? (b) : (a))
#define MAX_3(a, b, c)					(MAX(a, MAX(b, c)))

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)


#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})


#define roundup(x, y) (					\
{							\
	const typeof(y) __y = y;			\
	(((x) + (__y - 1)) / __y) * __y;		\
}							\
)
#define rounddown(x, y) (				\
{							\
	typeof(x) __x = (x);				\
	__x - (__x % (y));				\
}							\
)

#define _DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))


static void printfmt_ppa(const char * title, ppa_t addr)
{

	printf("%s: ppa blk[%u] pg[%u] lun[%u] pl[%u] sec[%u] ch[%u] \n",
	       title,
	       addr.nand.blk,
	       addr.nand.pg,
	       addr.nand.lun,
	       addr.nand.pl,
	       addr.nand.cp,
	       addr.nand.ch);    
}


#endif
