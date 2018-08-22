/*
 * Copyright (C) 2018-2020 NET-Swift.
 * Initial release: Dengcai Xu <dengcaixu@net-swift.com>
 *
 * ALL RIGHTS RESERVED. These coded instructions and program statements are
 * copyrighted works and confidential proprietary information of NET-Swift Corp.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part.
 *
 * function: 
 * RAIF(Redundant Arrays of Independent Flashs)
 * RPU: RAIF Protection Unit
 */

//rpu: Raif Protection Unit, cross multi Dies according config paramter @RPU_DIES_NUM_CFG
u8 rpu_start_die(ppa_t ppa)
{
	u8 die = PPA_TO_DIE(ppa);

	die = rounddown(die, RPU_DIES_NUM_CFG);

	// this ppa in a raif unit[die, die+RPU_DIES_NUM_CFG-1]

	return die;
}

u8 rpu_end_die(ppa_t ppa)
{
	u8 die = rpu_start_die(ppa);
	
	return die + RPU_DIES_NUM_CFG - 1;
}

// when raif not enable, never call this
// return value:
//     0:  NOT raif page
//    -1:  error RAIF MODE config
//	   1:  this is a raif1 page
//	   2:  this is a raif2 page
int is_raif_page(ppa_t ppa)
{
#if (RAIF_MODE == RAIF_DISABLE)
	// no any raif page
	return 0;

#elif ((RAIF_MODE == RAIF1_ENABLE) || (RAIF_MODE == RAIF2_ENABLE))
	u8 number = RAIF_MODE;
	u8 last_dies[RAIF_MODE];
	u8 current_die;
	u8 raif1_die = 0xff;
	u8 raif2_die = 0xff;
	
	u8 start_die  = rpu_start_die(ppa);
	u8 end_die = start_die + RPU_DIES_NUM_CFG - 1;
	u8 start_lun = DIE_TO_LUN(start_die);
	u8 start_ch  = DIE_TO_CH(start_die);
	u8 end_lun = DIE_TO_LUN(end_die);
	u8 end_ch  = DIE_TO_CH(end_die);

	//FIXME: check fn return value if false
	get_lastn_good_die_within_range(ppa.nand.blk, start_lun, start_ch, 
									end_lun, end_ch, number, last_dies);

	if (number == 1) {
		// raif1
		raif1_die = last_dies[0];
	} else if (number == 2) {
		// raif 2
		raif2_die = last_dies[0];		
		raif1_die = last_dies[1];	
	} else {
		// NEVER run to here
	}
	
	current_die = PPA_TO_DIE(ppa);
	if (current_die == raif1_die)
		return 1;
	else if (current_die == raif1_die)
		return 2;
	else
		return 0;
#else
	#error "RAIF Mode Invalid"
	return -1;
#endif

}

// when this @blk bbt changed and has update bbt,to recalirate raif die position
// old RAIF_PAGE has overwrite as BADBLK_PAGE, so here don't need care about.
void raif_die_recalibrate(u16 blk)
{
	u8 number = RAIF_MODE;
	u8 last_dies[RAIF_MODE];
	u8 start_die, end_die;
	u8 start_lun, start_ch, end_lun, end_ch;
	u8 ch, lun;
	
	for_each_rpu(start_die) {
		//this RPU[start_die: end_die]
		end_die = start_die + RPU_DIES_NUM_CFG - 1;
		start_lun = DIE_TO_LUN(start_die);
		start_ch  = DIE_TO_CH(start_die);
		end_lun = DIE_TO_LUN(end_die);
		end_ch  = DIE_TO_CH(end_die);

		//bbt has updated
		get_lastn_good_die_within_range(blk, start_lun, start_ch, 
										end_lun, end_ch, number, last_dies);
		if (number == 1) {
			// raif1
			ch = DIE_TO_CH(last_dies[0]);
			lun = DIE_TO_LUN(last_dies[0]);
			__set_page_type(blk, lun, ch, RAIF1_PAGE);
		} else if (number == 2) {
			// raif 2
			ch = DIE_TO_CH(last_dies[0]);
			lun = DIE_TO_LUN(last_dies[0]);
			__set_page_type(blk, lun, ch, RAIF2_PAGE);

			ch = DIE_TO_CH(last_dies[1]);
			lun = DIE_TO_LUN(last_dies[1]);
			__set_page_type(blk, lun, ch, RAIF1_PAGE);			
		} else {
			// NEVER run to here
		}
	}

	return;
}


