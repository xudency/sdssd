#ifndef _UTILS_H_
#define _UTILS_H_


//bit ops
// n is 64bit
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((u32)(n))
#define bit_test(f, bitnum)   			(0 != ((f)&(1<<bitnum)))
#define bit_set(f, bitnum)    			((f) |= (1<<bitnum))
#define bit_clear(f, bitnum)  			((f) &= ~(1<<bitnum))
#define bit_mask(width) 				((1 << (width)) - 1)

//word dword qword
#define BYTE_BITS					8
#define WORD_BITS					16
#define DWORD_BITS					32
#define QWORD_BITS					64

#define WORD_BYTES					(WORD_BITS/BYTE_BITS)
#define DWORD_BYTES					(DWORD_BITS/BYTE_BITS)
#define QWORD_BYTES					(QWORD_BITS/BYTE_BITS)


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

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

//#define forever for(;;)

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
