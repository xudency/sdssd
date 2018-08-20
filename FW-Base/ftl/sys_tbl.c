/*
 *
 */
 
#include "first_page.h"

// each band has 1 first_page in memory, the sizeis 4KB/Band 
// this is not access frequently, reside in DDR
first_page_t *g_first_page[BAND_NUM] = {NULL};

// each band has 2 ftl_log_page in memory(Ping-Pong switch), 36KB*2/band
log_page_t *g_ftl_log_page[BAND_NUM][LOG_PAGES_PER_BAND] = {NULL};

// a fix pattern data for padding, all 0x0000, assign a special 4KB for it 
void *dummy_date_buff = NULL;

// RAIF is implement by HW, the result is saved in a fixed address in CBUFF, 
// the address is depend on HW design, for fw, we only need assign the base address 
void *raif1_buff = NULL;
void *raif2_buff = NULL;


void first_page_buff_init(void)
{
	u8 band;

	for_each_band(band) {
		memset(g_first_page[band], 0x00, FIRST_PAGE_SIZE);
	}
}


// when this R-Block Open, init it from bmi.
void update_first_page(u32 block, band_type_e band)
{
	ASSERT_DEBUG(band < MAX_BAND_CNT);
    first_page_t * firstpage = (first_page_t *)(g_ftl_firstpage_base[band]);
    //state_table_t* state_tbl = &gt_state_table;
    bmi_item_t* bmi;
    u32 lun;

    if (block >= CFG_MAX_RAID_BLOCK_NUM)
    {
        //print_err(" the block (0x%x) is out of range", block);
        return;
    }

    bmi = ((bmi_item_t*)g_ftl_bmitbl_base) + block;

    firstpage->bad_block_flag = 0xffffffff;
    firstpage->sequence = gt_state_table.current_seq;
    for (lun = 0; lun < CFG_DRIVE_LUN_NUM; lun++)
    {
        firstpage->bad_block[lun] = bmi->bad_block[lun];
        firstpage->write_fail_block[lun] = bmi->write_fail_block[lun];
    }
    firstpage->pecycle = bmi->pecycle;
    //firstpage->cr_time = bmi->cr_time;
    firstpage->block = block;
    firstpage->gat_bpc_mode = bmi->gat_bpc_mode;
    return;
}


static inline band_info_t *get_band_info(u8 band)
{
	return &(g_primary_page->page.bandinfo[band]);
}

pg_type sys_tbl_get_pg_type(ppa_t ppa)
{
	// bbt, particular pos
}

pg_mode sys_tbl_get_pg_mode(ppa_t ppa)
{
	// LP UP XP
}


