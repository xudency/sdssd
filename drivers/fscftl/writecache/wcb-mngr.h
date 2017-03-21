#ifndef __WCB_MNGR_
#define __WCB_MNGR_
#include "../hwcfg/cfg/flash_cfg.h"

// Write Cache Size=16*4*4*8*10(4KB) = 80MB
#define PADDING_PAGE_NUM  8
#define READ_CB_UNITS  	  PADDING_PAGE_NUM
#define WRITE_CB_UNITS 	  2
#define TOTAL_CB_UNITS	(WRITE_CB_UNITS + READ_CB_UNITS)   // one LUN total
#define CB_ENTITYS_CNT (TOTAL_CB_UNITS * CFG_NAND_LUN_NUM) // all LUN total
#define RAID_LUN_SEC_NUM (CFG_NAND_CHANNEL_NUM * CFG_NAND_EP_NUM * CFG_NAND_PLANE_NUM)
#define RAID_PAGE_SEC_NUM (RAID_LUN_SEC_NUM * CFG_NAND_LUN_NUM)
#define WRITE_CACHE_SEC_NUM (RAID_PAGE_SEC_NUM * TOTAL_CB_UNITS)

#define RAID_LUN_DATA_SIZE (RAID_LUN_SEC_NUM * CFG_NAND_EP_SIZE)
#define RAID_LUN_META_SIZE (RAID_LUN_SEC_NUM * NAND_META_SIZE)

struct fsc_fifo {
	u32 head;	
	u32 tail;
	u32 size;
};

struct wcb_lun_entity {
	u32 index; //idr?

	/* begin ppa of this raid lun when pull form empty fifo
	 * assign the value according cur_ppa
	 */
	struct physical_address baddr;
	void *data;  /* Buffer Vaddr size=LUNSIZE */
	void *meta;	 /* Meta Vaddr start */

	/* 
	 * when this Lun push to rd fifo, need update l2p incache to nand 
	 * these represent the lba of this LUN 
	 */
	u32 lba[RAID_LUN_SEC_NUM];
	//u64 ppa[RAID_LUN_SEC_NUM];
	
	/* next ep to be writed */
	u16 pos;

	/* one bit for one ch, 
	 * 1: this ch is outstanding  
	 * 0:this ch is complete 
	 * when ch_status=0, it indicated this LUN can push to read fifo
	 */
	u16 ch_status;

	/* emptypool readpool[lun] fullpool, submit[lun] */
	u32 next;
};

struct wcb_lun_gctl {
	spinlock_t wcb_fifo_lock;

	struct physical_address curppa;  	/* next available ppa */
	struct fsc_fifo empty_lun;
	struct fsc_fifo full_lun;
	struct fsc_fifo read_lun[CFG_NAND_LUN_NUM];

	struct wcb_lun_entity *lun_entitys;

	/* 
	 * 0xdead: this Lun is idle, writer can issue this lun to hw 
	 * else: this Lun has a page outstanding, the val=pagenum
	 */
	u16 ongoing_pg_num[CFG_NAND_LUN_NUM];

	/* this must=0 or 1 */
	u16 ongoing_pg_cnt[CFG_NAND_LUN_NUM];
};


int write_cache_alloc(struct nvm_exdev *exdev);
void write_cache_free(struct nvm_exdev *exdev);

#endif

