#ifndef __WCB_MNGR_
#define __WCB_MNGR_
#include "../hwcfg/cfg/flash_cfg.h"
#include "../fscftl.h"

// Write Cache Size=16*4*4*8*10(4KB) = 80MB
#define PADDING_PAGE_NUM  8
#define READ_CB_UNITS  	  0//PADDING_PAGE_NUM
#define WRITE_CB_UNITS 	  2
#define TOTAL_CB_UNITS	(WRITE_CB_UNITS + READ_CB_UNITS)   // one LUN total
#define CB_ENTITYS_CNT (TOTAL_CB_UNITS * CFG_NAND_LUN_NUM) // all LUN total
#define RAID_LUN_SEC_NUM (CFG_NAND_CHANNEL_NUM * CFG_NAND_EP_NUM * CFG_NAND_PLANE_NUM)
#define RAID_PAGE_SEC_NUM (RAID_LUN_SEC_NUM * CFG_NAND_LUN_NUM)
#define WRITE_CACHE_SEC_NUM (RAID_PAGE_SEC_NUM * TOTAL_CB_UNITS)

#define RAID_LUN_PPA_SIZE  (RAID_LUN_SEC_NUM * sizeof(u64))
#define RAID_LUN_DATA_SIZE (RAID_LUN_SEC_NUM * CFG_NAND_EP_SIZE)
#define RAID_LUN_META_SIZE (RAID_LUN_SEC_NUM * NAND_META_SIZE)

/* one bio most will cross 3 LUN entitys */
#define MAX_USED_WCB_ENTITYS 3

extern void *g_vdata;
extern dma_addr_t g_dmadata;
extern struct wcb_lun_gctl *g_wcb_lun_ctl;

typedef enum {
	USR_DATA,
	DUMMY_DATA,
	BAD_BLK,
	XOR_PARITY,
	FIRST_PAGE,
	FTL_LOG,
} PPA_TYPE;

struct fsc_fifo {
	u32 head;	
	u32 tail;
	u32 size;
};

struct wcb_lun_entity {
	u32 index; //idr?

	/* 
	 * begin ppa of this raid lun when pull form empty fifo
	 * assign the value according cur_ppa
	 */
	geo_ppa baddr;

	/* 
	* data will be dma_map_sg in NVMe Driver, bio_map_kern
	* so we don't need alloc DMA for it 
	*/
	void *data;
	dma_addr_t data_dma;

	void *meta;
	dma_addr_t meta_dma;

	/* 
	* the ppa value in this LUN is fixed
	* ppa = baddr+offt, BUT baddr is get from free_blk_list
	*/
	u64 *ppa;
	dma_addr_t ppa_dma;

	/* 
	 * when this Lun push to rd fifo, need update l2p incache to nand 
	 * these represent the lba of this LUN 
	 */
	u32 lba[RAID_LUN_SEC_NUM];
	
	/* one bit for one ch or line, 
	 * 0: this ch is outstanding  
	 * 1: this ch is complete 
	 * when cqe_flag=BIT2MASK(), it indicated this LUN can push to read fifo
	 */
	u16 cqe_flag;

	/* 
	 * next ep to be writed, it's the pointer by memcpy data from bio 
	 * but, the data is not copyin when pos=RAID_LUN_SEC_NUM
	 * Thus, we can't push it to pull fifo
	 */
	u16 pos;

	/* add new member here, can before or will crash, I don't know Why :( */
	u32 newval;

	/* 
	 * it's the count of this lun entity has be coped data in 
	 * when it=RAID_LUN_SEC_NUM, we can push it to pull fifo
	 */
	atomic_t fill_cnt;

	/* emptypool readpool[lun] fullpool, submit[lun] */
	u32 next;
	u32 prev;
};

struct wcb_ctx {
	struct wcb_lun_entity *entitys;
	u16 start_pos;	// next to copy
	u16 end_pos;	// last to copy
};

struct wcb_bio_ctx {
	struct wcb_ctx bio_wcb[MAX_USED_WCB_ENTITYS];
};

struct wcb_lun_gctl {
	spinlock_t wcb_lock;
	spinlock_t fifo_lock;
	spinlock_t l2ptbl_lock;
	spinlock_t biolist_lock;

	geo_ppa curppa;  	/* next available ppa */
	struct bio_list requeue_wr_bios;

	struct fsc_fifo empty_lun;
	struct fsc_fifo full_lun;
	struct fsc_fifo read_lun[CFG_NAND_LUN_NUM];
	struct fsc_fifo ongoing_lun;   //temp

	struct wcb_lun_entity *lun_entitys;
	struct wcb_lun_entity *partial_entity;

	atomic_t outstanding_lun;
	//u32 entitynum;

#if 1 //TIMESTAMP_CATCH
	ktime_t curr_lun_full_ts;  //push to fullfifo
	ktime_t last_lun_full_ts;
	
	ktime_t curr_lun_subm_ts;   //full Lun submit
	ktime_t last_lun_subm_ts;

	ktime_t curr_lun_cmpl_ts;   // full Lun complete
	ktime_t last_lun_cmpl_ts;
#endif
};
    
/* the next available nand ppa address */
static inline geo_ppa current_ppa(void)
{
	return g_wcb_lun_ctl->curppa;
}

static inline void set_current_ppa(geo_ppa ppa)
{
	g_wcb_lun_ctl->curppa = ppa;
}

static inline void incrs_current_ppa(void)
{
    //IF_CARRY();
}

static inline struct wcb_lun_entity *wcb_lun_entity_idx(int index)
{
	return &g_wcb_lun_ctl->lun_entitys[index];
}

static inline struct wcb_lun_entity *partial_wcb_lun_entity(void)
{
	return g_wcb_lun_ctl->partial_entity;
}

static inline void *wcb_entity_base_data(int index)
{
	return g_wcb_lun_ctl->lun_entitys[index].data;
}

static inline void *wcb_entity_offt_data(int index, u16 pos)
{
	return wcb_entity_base_data(index) + pos*CFG_NAND_EP_SIZE;
}

static inline void *wcb_entity_base_meta(int index)
{
	return g_wcb_lun_ctl->lun_entitys[index].meta;
}


static inline dma_addr_t wcb_entity_base_metadma(int index)
{
	return g_wcb_lun_ctl->lun_entitys[index].meta_dma;
}


void print_lun_entitys_fifo(void);

void fsc_fifo_init(struct fsc_fifo *fifo);
int write_cache_alloc(struct nvm_exdev *exdev);
void write_cache_free(struct nvm_exdev *exdev);
struct wcb_lun_entity *get_lun_entity(geo_ppa startppa);
struct wcb_lun_entity *get_next_lun_entity(geo_ppa curppa);

void push_lun_entity_to_fifo(struct fsc_fifo *fifo, struct wcb_lun_entity *entry);
struct wcb_lun_entity *pull_lun_entity_from_fifo(struct fsc_fifo *fifo);

#endif

