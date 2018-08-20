
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

	return &(bbt_pg->bbt[blk][0][0]);
}

bool get_bbt(u16 blk, u8 lun, u8 ch, u8 pl)
{
	u16 b = 0;
	
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	b = bbt_pg->bbt[blk][lun][pl];

	return ((b >> ch) & 0x0001);
}

bool is_bad_block(u16 blk, u8 lun, u8 ch, u8 pl)
{
	return get_bbt(blk, lun, ch, pl);
}

void set_bbt(u16 blk, u8 lun, u8 ch, u8 pl)
{
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	bbt_pg->bbt[blk][lun][pl] |= (1<<ch);
	//bit_set(bbt_pg->bbt[blk][lun][pl], ch)
}

void clear_bbt(u16 blk, u8 lun, u8 ch, u8 pl)
{
	boot_blk_bbt_page* bbt_pg = get_bbt_page();

	bbt_pg->bbt[blk][lun][pl] &= ~(1<<ch);
}

// last one Good block in a R-Block, it is resv for LOG Page(raif, if enable)
bool get_lastn_good_blk(u16 blk, u16 n, ppa_t *result)
{
    int counter = 0;
    u16 bitmap;
    int lun, ch, pl; // MUST use int rather than u8
    ppa_t ppa = 0; 
	
	u16 *b = get_blk_bbt_base(blk);

	for (lun = CFG_NAND_LUN_NUM-1; lun >= 0; lun--) {
        for(ch = CFG_NAND_CH_NUM; ch >= 0; ch--) {
	        for (pl = CFG_NAND_PL_NUM; pl >=0; pl--) {
                bitmap = b[lun][pl];
                //bitmap = *(b + pl + lun*CFG_NAND_PL_NUM);
                if (TEST_BIT(ch)) {
                    // bad block  
                    continue;
                } else {
                    // find a good block
                    ppa.all = 0;
                    ppa.nand.cp = 0;
                    ppa.nand.pl = pl;
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
	}

    if (counter == n) {
        // find it, the last n good block's pl lun ch
        return true;
    } else {
        // Don't find due to no enough Good Block(less than n)
        return false;
    }
}

