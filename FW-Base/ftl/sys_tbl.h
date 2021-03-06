
#ifndef __SYS_TBL_H__
#define __SYS_TBL_H__

// each band has 2 log_page in memory, because, if we only has 1 for a band, 
// when it is flushing, the next r-page can't until log_page flushed completely.
// so with another, when the first is flushing, the next r-page can update in the second log_page.
// when the second log_page prepare ready. the previos one should has flushed complete.
#define LOG_PAGES_PER_BAND  	2

// WPB nnumber per band, as 3D TLC pair/share page, we need more WPB.
#define WPB_PER_BAND			6

#define FIRST_PAGE_SIZE			CP_SIZE
#define LOG_PAGE_SIZE			(roundup(PPA_PER_RPAGE*sizeof(cp_log_t), CP_SIZE))    //3K*12B=36KB

#define PPA_PER_LOG_PAGE 		(DIV_ROUND_UP(LOG_PAGE_SIZE, CP_SIZE))			// need 9 PPA

// LOG Page is in the last3 Die(last1-RAIF1  last2-RAIF2), But it don't take all PPAs in this Die
// for exapmle, PPA_PER_LOG_PAGE=9, the other7 PPA can used for host data.
#define LOG_PAGE_START_CPL		(PPA_PER_DIE - PPA_PER_LOG_PAGE)				// PL|CP

#define for_each_band(band) \
        for(band = 0; band < CFG_NAND_CH_NUM; band++)

enum band_type {
	SYSBAND = 0,    // MAP sys in 1 band
	HOSTBAND,		// host, hot-warm
	WLBAND,			// host, cold	
	BAND_NUM,
};

enum pg_type{
	NORMAL_PAGE = 0,
	BADBLK_PAGE,
	FIRST_PAGE,	
	FTL_LOG_PAGE,
	RAIF1_PAGE,
	RAIF2_PAGE,
	INVALID_PAGE    // error, no this type
};

enum pg_mode {
	SINGLE_LP,
	SHARED_UP,
	SHARED_XP
} pg_mode;


// first_page is the R-Block header, most member from bmi(help to restore approximate bmi)
// it is insert in specified FPA: cp=0 pl=0 lun=0 pg=0 backup multi copies in each channels  
typedef struct first_page
{
	u16 blk;
	u8 band;               /* host/recycle/system */
	u8 cri;			       /* code rate index */
	u32 sequence;		   /* band sequence */
	time64_t timestamp;	   /* Erase Safe, data retention */
	u16 pecycle;		   /* Program Erase Cycle */
	u16 bb_cnt;			    /* MAX is CH*PL*LUN */
	
	/* when latest boot block bbt page lost, merge it to previous bbt, 
	 * as we only support up to 12 channel, bit[15:12] is no used
	 */
	u16 bbt[CFG_NAND_LUN_NUM];
	//u8 pg_type[CFG_NAND_LUN_NUM][CFG_NAND_CH_NUM];
	read_retry_para fthr;
} first_page_t;    // 4KB


typedef struct cp_log {
	u32 cpa;    		// this EP's CPA
	time64_t timestamp; // if we only has 1 hband, ts is meaningless.
} cp_log_t;


// log page linked in a BlockChain
typedef struct ftl_log_page {
	cp_log_t cpats[PPA_PER_RPAGE];
} log_page_t; //6*3K=36KB


typedef struct write_prepare_block {
	ppa_t ppa;
	slot_t slot[16];
	u32 cpa[16];
} WPB;


typedef struct {
	u8 type;
	u16 open_blk;
	ppa_t current_ppa;
	u32 current_seq;		 // when open a new blk, assign to it the inc

	recov_logs_t last_log_page;   // pfail and pdown, it save the last LOG Page

	// fast crash recovery, Scan-Merge LOG Pages Window
	recov_logs_t base_log_page;
	recov_logs_t anchor_log_page;

} band_info_t;


// this is only in memory, don't need flush to NAND. 
struct band_ctl_ctx {
	log_page_t *current_log_page;    // 2 log page/band, this is the updating one
	ppa_t ppa;	// this log page flushed ppa position
	u16 offset;	// 3KB ppa a Log Page, [0-3KB)
	u8 band;
	u8 half;  // 2 loage page/band, this indicate the first half or second half
	WPB *wpb[WPB_PER_BAND];
};

/*typedef struct page_type_item {
	u8  pg_type[CFG_NAND_LUN_NUM][CFG_NAND_CH_NUM];
} page_type_item_t;*/


// extern global variable for other .c
extern first_page_t* g_first_page[BAND_NUM];
extern log_page_t *g_wdp_ftl_log[BAND_NUM][LOG_PAGES_PER_BAND];
extern void *dummy_date_buff;
extern void *raif1_buff;
extern void *raif2_buff;

// extern API for other .c
#define GET_FIRST_PAGE(band) \
	(first_page_t *)(g_first_page[band]);


static inline void first_page_reinit(u8 band)
{
	first_page_t *fp = GET_FIRST_PAGE(band);
	memset(fp, 0x00, FIRST_PAGE_SIZE);
}

static inline log_page_t *wdp_get_log_page(u8 band, u8 half)
{
	assert(band<BAND_NUM);
	assert(half<LOG_PAGES_PER_BAND)

	return g_wdp_ftl_log[band][half];
}

// when switch to next R-Page, need re-init ahead
static inline void wdp_ftl_log_page_reinit(u8 band, u8 half)
{
	log_page_t *lp = wdp_get_log_page(band, half);
	
	memset(lp, 0xff, LOG_PAGE_SIZE);
}


static inline band_info_t *get_band_info(u8 band)
{
	return &(g_primary_page->page.bandinfo[band]);
}

#endif
