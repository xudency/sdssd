
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
 * bad block managemt
 *
 */


// first PL, first LUN band block bitmap
u16 *get_blk_bbt_base(u16 blk)
{
	assert(blk < CFG_NAND_BLK_NUM);
	
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	return &(bbt_pg->bbt[blk][0]);
}

bool __get_bbt(u16 blk, u8 lun, u8 ch)
{
	u16 b = 0;
	
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	b = bbt_pg->bbt[blk][lun];

	return ((b >> ch) & 0x0001);
}

void get_bbt(ppa_t ppa)
{
	return __get_bbt(ppa.nand.blk, ppa.nand.lun, ppa.nand.ch);
}

bool __is_bad_block(u16 blk, u8 lun, u8 ch)
{
	return __get_bbt(blk, lun, ch);
}

bool is_bad_block(ppa_t ppa)
{
	return get_bbt(ppa);
}

void __set_bbt(u16 blk, u8 lun, u8 ch)
{
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	bbt_pg->bbt[blk][lun] |= (1<<ch);
	//bit_set(bbt_pg->bbt[blk][lun], ch)
}

void set_bbt(ppa_t ppa)
{
	__set_bbt(ppa.nand.blk, ppa.nand.lun, ppa.nand.ch);
}

void __clear_bbt(u16 blk, u8 lun, u8 ch)
{
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	bbt_pg->bbt[blk][lun] &= ~(1<<ch);
}

void clear_bbt(ppa_t ppa)
{
	__clear_bbt(ppa.nand.blk, ppa.nand.lun, ppa.nand.ch);
}


// last n Good block in a R-Block, it is resv for LOG Page(raif, if enable)
// last1 if raif2, last2 is raif1, last3 is log page
// this is in all dies [0 - 191]
/*bool get_lastn_good_die(u16 blk, u16 n, ppa_t *result)
{
    int counter = 0;
    u16 bitmap;
    int lun, ch; // MUST use int rather than u8
    ppa_t ppa = 0; 
	
	u16 *b = get_blk_bbt_base(blk);

	for (lun = CFG_NAND_LUN_NUM-1; lun >= 0; lun--) {
        for(ch = CFG_NAND_CH_NUM-1; ch >= 0; ch--) {
            bitmap = b[lun];
            //bitmap = *(b + pl + lun*CFG_NAND_PL_NUM);
            if (bit_test(ch)) {
                // bad block  
                continue;
            } else {
                // find a good block
                ppa.all = 0;
                ppa.nand.cp = 0;
                ppa.nand.ch = ch;
                ppa.nand.lun = lun;
                ppa.nand.pg = 0;
                ppa.nand.blk = blk;
                result[counter++] = ppa;
                if (counter == n)
                    break;
            }
		}
	}

    if (counter == n) {
        // find it, the last n good block's pl lun ch
        return true;
    } else {
        // Don't find due to no enough Good Block(less than n)
        return false;
    }
}*/

/*
 *last n Good block in a R-Block, it is resv for LOG Page(raif, if enable)
 *get lastn good dies in a given die range, die is compose of LUN|CH
 *BEWARE: res[0] is last1, res[1] is last2
 *if you want search in all Dies, you can call this fn as
 * 		get_lastn_good_die_within_range(blk, 0, 0, CFG_NAND_LUN_NUM-1, CFG_NAND_CH_NUM-1, n, &dies)
 */
bool get_lastn_good_die_within_range(u16 blk, u8 start_lun, u8 start_ch, 
											  u8 end_lun, u8 end_ch, u16 n, u8 *dies)
{
	int counter = 0;
	u16 bitmap;
	int lun, ch; // MUST use int rather than u8
	
	u16 *b = get_blk_bbt_base(blk);

	for (lun = end_lun; lun >= start_lun; lun--) {
		for(ch = end_ch; ch >= start_ch; ch--) {
			bitmap = b[lun];
			//bitmap = *(b + pl + lun*CFG_NAND_PL_NUM);
			if (bit_test(bitmap, ch)) {
				// bad block  
				continue;
			} else {
				// find a good block;
				dies[counter++] = LUNCH_TO_DIE(lun, ch)
				if (counter == n)
					break;
			}
		}
	}

	if (counter == n) {
		// find it, the last n good block's pl lun ch
		return true;
	} else {
		// Don't find due to no enough Good Block(less than n)
		return false;
	}
}


