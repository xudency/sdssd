/* 
 * R-Block management info
 *
 * bbt not include here, due to
 *     1.bbt need backup multi copies
 *     2.bbt need check frequently(atc_assign_fpa), need reside in sram
 *	   3.dirty bmi, so bmi size should as little as possible
 *
 * vpc(RUS) not include here, due to
 *     1.vpc update frequently(fdc_update_vpc), need reside in sram
 *     2.only vpc in primary,it is correctly, band write, vpc will update
 */

#ifndef __RAID_BLOCK_H__
#define __RAID_BLOCK_H__

typedef enum
{
    RBLK_FREE = 0,
    RBLK_OPEN ,		// 1
    RBLK_CLOSED,		// 2
	RBLK_RECYCLING, // 4	
    RBLK_OBSOLETED,	// 3
    RBLK_ERASING,
    RBLK_OFFLINE,	// 5
} bmi_block_status_e;


typedef struct blk_manage_info {
	u16 blk;
	u16 sequence;			/* band sequence */
	time64_t timestamp;     /* Erase Safe, data retention */
	u8 cri;					/* code rate index */
	u8 band;				/* host/recycle/system */
	u8 state;				/* FREE/OPEN/CLOSED/ERASE */
	u8 bb_grown_flag;   	/* BMI_FLAG_PRG_ERR/BMI_FLAG_UECC GC-P0*/
	u16 pecycle;			/* Program Erase Cycle */
	u16 bb_cnt;				/* MAX is CH*PL*LUN */
	ppa_t log_page; 		/* log page Die(CH X LUN) */
	read_retry_para fthr;   /* optimal read retry */
	rb_node_t rbnode;		/* bmi linked in a RB-Tree */
	u16 prev;
	u16 next;
} bmi_t;//__attribute__(align)

extern bmi_t *g_bmi_tbl;


#define GET_BMI(blk) (bmi_t *)(g_bmi_tbl + blk)
#define SET_BMI_STATE(blk, val) (g_bmi_tbl[blk].state=val)

#define BMI_NEXT_BLK(blk)  (g_bmi_tbl[blk].next)
#define BMI_PREV_BLK(blk)  (g_bmi_tbl[blk].prev)

/* get next Rblk's BMI in dlist*/
static inline bmi_t* get_next_bmi(u16 blk)
{
	u16 next_blk = BMI_NEXT_BLK(blk);

	if (next_blk != 0xffff)
		return GET_BMI(next_blk);
	else
		return NULL;
}

static inline bmi_t* get_prev_bmi(u16 blk)
{
	u16 prev_blk = BMI_PREV_BLK(blk);

	if (prev_blk != 0xffff)
		return GET_BMI(prev_blk);
	else
		return NULL;
}

u16 get_blk_from_free_list(void);
void add_blk_to_free_list(u16 blk);


#endif
