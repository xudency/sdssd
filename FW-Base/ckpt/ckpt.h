/*
 * Copyright (C) 2018-2020 NET-Swift.
 * Initial release: Dengcai Xu <dengcaixu@net-swift.com>
 *
 * ALL RIGHTS RESERVED. These coded instructions and program statements are
 * copyrighted works and confidential proprietary information of NET-Swift Corp.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part.
 *
 * function: 
 * checkpoint for fast crash recovery
 *
 */

#ifndef __CKPT_H__
#define __CKPT_H__


#define OP_PERCENTAGE	10		//Over-Provision space

//Byte
#define SSD_RAW_CAPACITY (FPA_SIZE*PG_NUM*BLK_NUM*PLANE_NUM*LUN_NUM*CH_NUM)
#define SSD_USER_CAPACITY SSD_RAW_CAPACITY*(100-OP_PERCENTAGE)/100

#define MAX_LBA (SSD_USER_CAPACITY/0x1000)	//0x1000:4KB,as FTL table is 4KB granularity

#define FTL_TABLE_SIZE  (MAX_LBA*4)				 	//store in open blk
#define FTL_TABLE_L1_SIZE (FTL_TABLE_SIZE/1024)	 	//store in open blk
#define FTL_TABLE_L2_SIZE (FTL_TABLE_L1_SIZE/1024)	//store in boot blk in primary_page



typedef union 
{
	struct 
	{
		u8 lpo;
		u8 lp;
		u8 up1;
		u8 up2;
	}mlc;

	struct
	{
		u8 lpo;
		u8 lp;
		u8 xp1;
		u8 up1;
		u8 xp2;
		u8 xp3;
		u8 up2;
		u8 xp4;
	}tlc;

} read_retry_para;



// recovery need Scan-Merge LOG Pages
typedef struct recov_log_pages {
	ppa_t log_ppa;
	u16 pos;		// scan from here rather than start0, version0.1, No use
} recov_logs_t;



// CKPT Control Block
typedef struct ckpt_ctl_ctx {
	bool ongoing;
	u16 log_pages_num;

	//u32 dirty_sgmt_cnt;		// MAP segment

	//Ping-Pong design
	//u32 sgmt_dirty_count;
	//u32 sgmt_dirty_count
	u64 sgmt_state1[MAP_SGMT_NUM];		// bitmap 0:clean    1-dirty 
	u64 sgmt_state2[MAP_SGMT_NUM];
	u64 *psgmt_ckpt_lock;
	u64 *psgmt_path_mutate;
	
} ckpt_cb;




#endif


