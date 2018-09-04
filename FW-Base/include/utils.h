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

#define REG32_BITS					32


// SIZE define from Linux/sizes.h
#define SZ_1				0x00000001
#define SZ_2				0x00000002
#define SZ_4				0x00000004
#define SZ_8				0x00000008
#define SZ_16				0x00000010
#define SZ_32				0x00000020
#define SZ_64				0x00000040
#define SZ_128				0x00000080
#define SZ_256				0x00000100
#define SZ_512				0x00000200

#define SZ_1K				0x00000400
#define SZ_2K				0x00000800
#define SZ_4K				0x00001000
#define SZ_8K				0x00002000
#define SZ_16K				0x00004000
#define SZ_32K				0x00008000
#define SZ_64K				0x00010000
#define SZ_128K				0x00020000
#define SZ_256K				0x00040000
#define SZ_512K				0x00080000

#define SZ_1M				0x00100000
#define SZ_2M				0x00200000
#define SZ_4M				0x00400000
#define SZ_8M				0x00800000
#define SZ_16M				0x01000000
#define SZ_32M				0x02000000
#define SZ_64M				0x04000000
#define SZ_128M				0x08000000
#define SZ_256M				0x10000000
#define SZ_512M				0x20000000

#define SZ_1G				0x40000000
#define SZ_2G				0x80000000


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

// offset in 4KB
#define page_offset(v) ((u64)(v) & (SZ_4K - 1))


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
