#ifndef __SYS_META_H_
#define __SYS_META_H_

#include "../fscftl.h"
#include "../writecache/wcb-mngr.h"

/* Bootblk+SYSTBL+BB+GC+SpecialPPA(XOR FistPage Ftllog) */
//#define OP_CAPACITY		(CAPACITY - USER_CAPACITY)
/* User data Byte */
#define USER_CAPACITY   ((CAPACITY*8)/11)
#define MAX_USER_LBA	   (USER_CAPACITY/CFG_NAND_EP_SIZE - 1)

#define BMITBL_SEC_NUM  (DIV_ROUND_UP(CFG_NAND_BLOCK_NUM * \
			 sizeof(struct bmi_item), EXP_PPA_SIZE))
#define BMITBL_SIZE     (BMITBL_SEC_NUM * EXP_PPA_SIZE)

#define VPCTBL_SEC_NUM  (DIV_ROUND_UP(CFG_NAND_BLOCK_NUM * 4, EXP_PPA_SIZE))
#define VPCTBL_SIZE     (VPCTBL_SEC_NUM*EXP_PPA_SIZE)

#define USR_FTLTBL_SEC_NUM  (DIV_ROUND_UP(MAX_USER_LBA * 4, EXP_PPA_SIZE))
#define USR_FTLTBL_SIZE     (USR_FTLTBL_SEC_NUM*EXP_PPA_SIZE)

#define L1TBL_NOALIGN_SIZE ((USR_FTLTBL_SEC_NUM+VPCTBL_SEC_NUM+BMITBL_SEC_NUM)*4)
#define L1TBL_SEC_NUM   (DIV_ROUND_UP(L1TBL_NOALIGN_SIZE, EXP_PPA_SIZE))
#define L1TBL_SIZE      (L1TBL_SEC_NUM*EXP_PPA_SIZE)
// LBA zone
//[0 MAX_USER_LBA] //User zone
#define EXTEND_LBA_BASE    (MAX_USER_LBA + 1)
#define EXTEND_LBA_BMITBL  (EXTEND_LBA_BASE)
#define EXTEND_LBA_VPCTBL  (EXTEND_LBA_BMITBL+BMITBL_SEC_NUM)
#define EXTEND_LBA_L1TBL   (EXTEND_LBA_VPCTBL+VPCTBL_SEC_NUM)
//#define XXX  (EXTEND_LBA_L1TBL + L1TBL_SEC_NUM)

//#define EXTEND_LBA_END	   ()

#define INVALID_PAGE     0xffffffff


extern struct sys_status_tbl *statetbl;
extern struct bmi_item *bmitbl;
extern u32 *vpctbl;   // prevent by l2plock


enum raidblk_status {
	RAID_BLK_FREE,		// in free pool
	RAID_BLK_OPEN,		// partial used, write pointer, atmost has 2 OPEN
	RAID_BLK_CLOSED,	// all cqe of this raidblk has back
	RAID_BLK_TOGC,		// in gc_blk_pool, vpc decrease to 0, can be GC
	RAID_BLK_GCING		// GC this raid blk
};

// flush to primary page
struct sys_status_tbl {
	struct fsc_fifo free_blk_pool;
	struct fsc_fifo closed_blk_pool;	// Full But VPC!=0
	struct fsc_fifo gc_blk_pool;        	// Full && VPC=0
};

struct bmi_item {
	u32 sequence;	
	u32 pecycle;
	u16 blknum;
	u16 flag_bitmap;
	u16 bbt[CFG_NAND_LUN_NUM]; /* 1: bad; 0: good */
	u16 write_err[CFG_NAND_LUN_NUM];
	u16 read_uecc[CFG_NAND_LUN_NUM];
	u16 rdcount;	
	u16 bmstate;
        atomic_t cmpl_lun;  // 1 rblk contain PG*LUN LUNs, when all LUNs back set bmstate=Close
        // fifo link
	u32 prev;
	u32 next;
};

/* bad or good block */
enum {
	GOODB,
	BADB,
};

static inline struct bmi_item *get_bmi_item(u16 blk)
{
	BUG_ON(blk >= CFG_NAND_BLOCK_NUM);
	
	return bmitbl + blk;
}

void mark_bbt_tbl(u32 blk, u32 lun, u32 ch, bool status);

u32 pull_blk_from_pool(struct fsc_fifo *fifo);
void push_blk_to_pool(struct fsc_fifo *fifo, u32 blk);

u32 get_blk_from_free_list(void);
void insert_blk_to_free_list(u32 blk);

int statetbl_init(void);
void statetbl_exit(void);

int bmitbl_init(void);
void bmitbl_exit(void);

int vpctbl_init(void);
void vpctbl_exit(void);

int l2ptbl_init(struct nvm_exdev *exdev);
void l2ptbl_exit(struct nvm_exdev *exdev);

#endif

