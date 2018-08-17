/*
 * this header define the basic data type 
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#define TRUE							1
#define FALSE							0
#define	true							TRUE
#define	false							FALSE


#ifndef NULL
#define NULL							((void *) 0)
#endif


typedef unsigned char 			u8;
typedef unsigned short			u16;
typedef unsigned int			u32;
typedef unsigned long long		u64;

typedef signed char 			s8;
typedef signed short			s16;
typedef signed int				s32;
typedef signed long long 		s64;

typedef u64 time64_t;
typedef u32 time32_t;


#endif
