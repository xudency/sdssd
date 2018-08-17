#ifndef _FSCFTL_UTILIS_H_
#define _FSCFTL_UTILIS_H_

#define TRACE_TAG(fmt, arg...) \
	printk("%s-%d "fmt"\n", __FUNCTION__, __LINE__, ##arg)


#define BIT_TEST(f, bitnum)   			(0 != ((f)&(1<<bitnum)))
#define BIT_SET(f, bitnum)    			((f) |= (1<<bitnum))
#define BIT_CLEAR(f, bitnum)  			((f) &= ~(1<<bitnum))
#define BIT2MASK(width) 			((1UL<<(width))-1)


#define for_each_ch(ch) \
	for(ch= 0; ch < CFG_NAND_CHANNEL_NUM; ch++)

#define for_each_sec(sec) \
		for(sec= 0; sec < CFG_NAND_EP_NUM; sec++)
		
#define for_each_pl(pl) \
	for(pl= 0; pl < CFG_NAND_PLANE_NUM; pl++)
		
#define for_each_lun(lun) \
	for(lun= 0; lun < CFG_NAND_LUN_NUM; lun++)
		
#define for_each_blk(blk) \
	for(blk= 0; blk < CFG_NAND_BLOCK_NUM; blk++)

// BEWARE, if use reverse to trasveral blk, blk type MUST int,u16 u32 alway >=0
#define for_each_blk_reverse(blk) \
	for(blk = CFG_NAND_BLOCK_NUM-1; blk >= 0 ; blk--)

#endif

