#ifndef __SYS_META_H_
#define __SYS_META_H_

#include "../fscftl.h"
#include "../writecache/wcb-mngr.h"

// BMITBL need how many PPAs
#define BMITBL_SEC_NUM  (DIV_ROUND_UP(CFG_NAND_BLOCK_NUM * \
			 sizeof(struct bmi_item), EXP_PPA_SIZE))

#define BMITBL_SIZE   (BMITBL_SEC_NUM * EXP_PPA_SIZE)

enum raidblk_status {
	RAID_BLK_FREE,		// in free pool
	RAID_BLK_OPEN,		// partial used, write pointer
	RAID_BLK_GOING,		// wr_point to next blk, but this blk cqe don't back
	RAID_BLK_CLOSED,	// all cqe of this raidblk has back
	RAID_BLK_TOGC,		// in gc_blk_pool, vpc decrease to 0, can be GC
	RAID_BLK_GCING		// GC this raid blk
};

// flush to primary page
struct sys_status_tbl {
	struct fsc_fifo free_blk_pool;
	struct fsc_fifo closed_blk_pool;	// Full But VPC!=0
	struct fsc_fifo gc_blk_pool;        // Full && VPC=0
};

struct bmi_item {
	u32 sequence;	
	u32 pecycle;
	u16 blknum;
	u16 bbt[CFG_NAND_LUN_NUM]; /* 1: bad; 0: good */
	u16 write_err[CFG_NAND_LUN_NUM];
	u16 read_uecc[CFG_NAND_LUN_NUM];
	u16 rdcount;	
	u16 bmstate;
	
	// fifo link
	u32 prev;
	u32 next;
};

extern struct sys_status_tbl *statetbl;
extern struct bmi_item *bmitbl;

u32 pull_blk_from_pool(struct fsc_fifo *fifo);
void push_blk_to_pool(struct fsc_fifo *fifo, u32 blk);

u32 get_blk_from_free_list(void);
void insert_blk_to_free_list(u32 blk);

int statetbl_init(void);
void statetbl_exit(void);

int bmitbl_init(void);
void bmitbl_exit(void);

int l2ptbl_init(struct nvm_exdev *exdev);
void l2ptbl_exit(struct nvm_exdev *exdev);

#endif

