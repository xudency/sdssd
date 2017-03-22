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

/* one bio most will cross 3 LUN entitys */
#define MAX_USED_WCB_ENTITYS 3


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

	/* begin ppa of this raid lun when pull form empty fifo
	 * assign the value according cur_ppa
	 */
	geo_ppa baddr;


    /* data will be dma_map_sg in NVMe Driver, bio_map_kern
     * so we don't need alloc DMA for it 
     */
	void *data;

    void *meta;
	dma_addr_t meta_dma;

    /* the ppa value in this LUN is fixed
     * ppa = baddr+offt, BUT baddr is get from free_blk_list
     */
    u64 *ppa;
    dma_addr_t ppa_dma;

	/* 
	 * when this Lun push to rd fifo, need update l2p incache to nand 
	 * these represent the lba of this LUN 
	 */
	u32 lba[RAID_LUN_SEC_NUM];
	
	/* next ep to be writed */
	u16 pos;

	/* one bit for one ch, 
	 * 1: this ch is outstanding  
	 * 0: this ch is complete 
	 * when ch_status=0, it indicated this LUN can push to read fifo
	 */
	u16 ch_status;

	/* emptypool readpool[lun] fullpool, submit[lun] */
	u32 next;
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
    spinlock_t l2ptbl_lock;

	geo_ppa curppa;  	/* next available ppa */
	struct fsc_fifo empty_lun;
	struct fsc_fifo full_lun;
	struct fsc_fifo read_lun[CFG_NAND_LUN_NUM];

	struct wcb_lun_entity *lun_entitys;
    struct wcb_lun_entity *partial_entity;
	u32 entitynum;
	
	/* 
	 * 0xdead: this Lun is idle, writer can issue this lun to hw 
	 * else: this Lun has a page outstanding, the val=pagenum
	 */
	u16 ongoing_pg_num[CFG_NAND_LUN_NUM];

	/* this must=0 or 1 */
	u16 ongoing_pg_cnt[CFG_NAND_LUN_NUM];
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

static inline struct wcb_lun_entity *partial_wcb_lun_entity(void)
{
    return g_wcb_lun_ctl->partial_entity;
}

static inline void *wcb_entity_base_data(int index)
{
	return g_wcb_lun_ctl->lun_entitys[index].data;
}

static inline void *wcb_entity_base_meta(int index)
{
	return g_wcb_lun_ctl->lun_entitys[index].meta;
}

static inline dma_addr_t wcb_entity_base_metadma(int index)
{
	return g_wcb_lun_ctl->lun_entitys[index].meta_dma;
}

static inline void *wcb_entity_offt_data(int index, u16 pos)
{
	return (void *)((unsigned long)wcb_entity_base_data + pos * CFG_NAND_EP_SIZE);
}



int write_cache_alloc(struct nvm_exdev *exdev);
void write_cache_free(struct nvm_exdev *exdev);
struct wcb_lun_entity *get_new_lun_entity(geo_ppa curppa);

#endif

