

/*
 * main.c
 *
 *  Created on: 2018年9月13日
 *      Author: dengcaixu
 */

#include <stdio.h>

// CPU Endian test
typedef union
{
	u64 ppa;

	struct
	{
		u8 byte0;
		u8 byte1;
		u8 byte2;
		u8 byte3;
		u8 byte4;
		u8 byte5;
		u8 byte6;
		u8 byte7;
	}bytes;
} mycmd;


typedef union
{
	u64 all;

	struct
	{
		u64 cnt  	:4;
		u64 cal  	:3;
		u64 rsvd	:1;
		u64 opcode	:8;
		u64 dir		:1;
		u64 tag		:8;
		u64 ep		:5;
		u64	ch		:5;
		u64	cpa		:16;
		u64 rsvd2	:13;
	}qw0;
} aplo_msg_t;

/*
int main()
{
	//int *p = NULL;
	//mycmd tmp;
	//tmp.ppa = 0x0123456789abcdef;

	//u64 a = 0x0123456789abcdef;


	//printf("char:%d  short:%d int:%d long:%d longlong:%d pointer:%d\n",
			//sizeof(char), sizeof(short), sizeof(int), sizeof(long),
			//sizeof(long long), sizeof(p));


	printf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			tmp.bytes.byte0, tmp.bytes.byte1, tmp.bytes.byte2,
			tmp.bytes.byte3, tmp.bytes.byte4, tmp.bytes.byte5,
			tmp.bytes.byte6, tmp.bytes.byte7);

	return 0;
}
*/

int main()
{
	aplo_msg_t req;
	req.all = 0;

	req.qw0.cnt = 5;
	req.qw0.cal = 2;
	req.qw0.opcode = 0x7f;
	req.qw0.dir = 0;
	req.qw0.tag = 0x89;
	req.qw0.ep = 19;
	req.qw0.ch = 22;
	req.qw0.cpa = 0xeda8;

	printf("req.all=0x%llx\n", req.all);
	printf("req.qw0.cnt=%d req.qw0.cal=%d req.qw0.opcode=0x%x "
			"req.qw0.tag=0x%x, req.qw0.ep=%d req.qw0.ch=%d req.qw0.cpa=0x%x\n",
			req.qw0.cnt, req.qw0.cal, req.qw0.opcode, req.qw0.tag, req.qw0.ep,
			req.qw0.ch, req.qw0.cpa);

	return 0;
}

