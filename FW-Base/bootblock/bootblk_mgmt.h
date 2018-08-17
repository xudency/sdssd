#ifndef __BOOT_BLK_MGMT__
#define __BOOT_BLK_MGMT__

#define BOOT_BLK_COPIES   (CFG_NAND_PL_NUM*CFG_NAND_LUN_NUM)
#define BOOT_BLK0  		  0
#define BOOT_BLK1  		  1

#define BOOT_LUN_PRIMARY_PING 		0
#define BOOT_LUN_PRIMARY_PONG 		1
#define BOOT_LUN_BBT				2
#define BOOT_LUN_FW_IMAGE			3
#define BOOT_LUN_BOOTLOADER			4
#define BOOT_LUN_TEMPORY			5		// this is shared by fw-image/bbt/bootloader

#define BOOT_LUN_NUM 		CFG_NAND_LUN_NUM

#define MW_DWORD_NUM 	  4

#define BOOT_BBT_PRIMARY_MDW0 0x6E657875
#define BOOT_BBT_PRIMARY_MDW1 0x73707269
#define BOOT_BBT_PRIMARY_MDW2 0x6d617279
#define BOOT_BBT_PRIMARY_MDW3 0X626F6F74

#define BOOT_BBT_PAGE_MDW0 0x6261646c
#define BOOT_BBT_PAGE_MDW1 0x6c6f636b
#define BOOT_BBT_PAGE_MDW2 0x73626974
#define BOOT_BBT_PAGE_MDW3 0x6d617073


typedef enum _POWER_DOWN_STATE {
	POWER_DOWN_STATE_SAFE,		//Power down normally,load  mapping table from Nand
	POWER_DOWN_STATE_PFAIL, 	//Power-Fail, need rebuild mapping table
	POWER_DOWN_STATE_MAX
}POWER_DOWN_STATE;


typedef struct {
	u8 type;
	u16 open_blk;
	
	// rbtree is sort by pecycle &&vpc&&sequence, this is the list_head, sort by sequence
	dlist closed_blk_list;
	
	ppa_t current_ppa;
	u32 current_seq;

	recov_logs_t last_log_page;    // pfail and pdown, it save the last LOG Page

	// fast crash recovery, Scan-Merge LOG Pages Window
	recov_logs_t base_log_page;
	recov_logs_t anchor_log_page;

	///////////// below only exist in memory
	log_page_t *current_log_page;   // updating
	WPB *wpb[2];
	WPB *wpb_hist[SB_NUM];			// buffer UP until XP WPB is rdy, program both together
} band_info_t;

typedef struct
{
	u32 magic_word[MW_DWORD_NUM];  // for sanity check
	u32 sequence_id;   //max is newest
	u32 left_space;    // when new bb grown, decrease the new bb space
	u16 bbt[blk][lun][pl];
} boot_blk_bbt_page;

struct device_cfg
{
	u8 cp_num;
	u8 pl_num;
	u8 ch_num;
	u8 lun_num;
	u16 pg_num;
	u16 blk_num;
	u16 cp_size;
	u16 pg_size;
	u32 max_pe;
	u32 max_rdc;
	u32 max_lba;
	u32 min_space_need;    // when bb grow, space derease below this threshold, FW WARN!!!
};

struct boot_page_pointer {

	// the BLUN is fixed original, when a BLUN wear out due to pecycle beyond
	// FW will get the B-LUN from reserves_list.
	u8 blun_primary_ping;
	u8 blun_primary_pong;
	u8 blun_bbt;
	u8 blun_fw_image;
	u8 blun_bootloader;
	u8 blun_temporary;
	u16 blun_reserves_list;  	// 0xffc0   1-reserves   0-broken or using 

	ppa_t primary_page;  	/* last primary page location */
	ppa_t bbt_page;			/* b-page0 is manu bbt, only update grown version */
	ppa_t fw_image_page;
	ppa_t bootloader_page;
	ppa_t tempory_page;     /* shared, may not in b-page0 */
};


//track the B-LUN use
struct boot_lun_info {
	u32 rdc;
	u32 cr_time;
	u32 pecycle;
};

typedef struct {
	u32 magic_word[MW_DWORD_NUM];

	// the max_sequence_id is the last one.
	u32 sequence_id;		
	POWER_DOWN_STATE	power_down_state;
	
	//Last full mapping table saved in last safe power down
	u32 map_l2_index[FTL_TABLE_L2_SIZE/4];
	u32 rdc_index[];
	u32 bmi_index[];

	// after flush bmitbl, FW will flush Log Pages to system band, 
	// during this process, the last sys block may closed and open the next Rblk
	// so the bmitbl previous flush is a bit stale
	// when power on we need merge it to bmi_index->bmitbl.
	bmi dirty_bmi[2];
	u32 vpc[CFG_NAND_BLK_NUM];
	struct boot_lun_info blun_status[BOOT_LUN_NUM];

	struct device_cfg dev_cfg;
	struct boot_page_pointer pointer;

	band_info_t bandinfo[BAND_NUM];

#if 1
	// dlist O(N)
	dlist free_blk_list;
	dlist closed_blk_list;
	dlist obsolete_blk_list;
#else
	rb-tree O(logN)
	rb_tree_t free_blk_rbtree;
	rb_tree_t close_blk_rbtree[BAND_NUM];  // sort by pecycle&&vpc
	rb_tree_t obsolete_blk_rbtree;
	recycleing_blk;
#endif
}primary_page_info;
	

typedef struct {
	primary_page_info page;
	u8 rsv[PRIMARY_BOOT_PAGE_SIZE - sizeof(primary_page_info)];   // 16KB align
} boot_blk_primary_page;


// this is only in memory, don't need flush to NAND. 
typedef struct {
	u8 bb_grown;    		// flag,  1:bbt-page need flushed when Pfail.
	u16 blun_wait_erase;  	// bitmap 1:these B-LUN need erase.
	//u8 ......... 
} boot_blk_ctx;



static inline boot_blk_primary_page *get_primary_page(void)
{
	return g_primary_page;
}

#endif
