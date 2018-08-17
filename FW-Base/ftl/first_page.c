/*
 *
 */
 
#include "first_page.h"

// 4KB/Band, this is not access frequently, reside in DDR
first_page_t *g_first_page[BAND_NUM] = {NULL,};



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

