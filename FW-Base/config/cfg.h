#ifndef __CFG_H__
#define __CFG_H__


//// Move Flash type related to cfg_NANDtype_xxx.h
#define CH_BITS    4
#define CP_BITS    2
#define PL_BITS    2
#define LUN_BITS   2
#define PG_BITS    8
#define BLK_BITS   10

#define RSVD_BITS (32-CH_BITS-CP_BITS-PL_BITS-LUN_BITS-PG_BITS-BLK_BITS)

#define CFG_NAND_CH_NUM    12
#define CFG_NAND_CP_NUM    4
#define CFG_NAND_PL_NUM    4
#define CFG_NAND_LUN_NUM   16
#define CFG_NAND_PG_NUM    2304
#define CFG_NAND_BLK_NUM   504


// The defiCPnition of PPA address
typedef union
{
    unsigned int all;

	// ppa format in nand
	struct
	{
		unsigned int cp			: CP_BITS;  	// this is PAGE Offset
		unsigned int pl        	: PL_BITS;  
		unsigned int ch        	: CH_BITS;  
		unsigned int lun       	: LUN_BITS;  
		unsigned int pg        	: PG_BITS;  
		unsigned int blk     	: BLK_BITS; 
		unsigned int rsvd		: RSVD_BITS;
	}nand;

	// ppa format in ddr    
	struct
	{
		unsigned int	srid		    : CACHE_BUF_BITS;   // buf index
		unsigned int    seid		    : CACHE_BLK_BITS;   //block num for VPC update
		unsigned int    slotid      	: CACHE_HIT_CNT_BITS; // write cache hit cnt
    } slot;

} ppa_t;


#define for_each_ch(ch) \
        for(ch= 0; ch < CFG_NAND_CH_NUM; ch++)

#define for_each_cp(cp) \
		for(cp = 0; cp < CFG_NAND_CP_NUM; cp++)
				
#define for_each_pl(pl) \
        for(pl = 0; pl < CFG_NAND_PL_NUM; pl++)

#define for_each_lun(lun) \
        for(lun = 0; lun < CFG_NAND_LUN_NUM; lun++)

#define for_each_pg(pg) \
		for(pg = 0; pg < CFG_NAND_LUN_NUM; pg++)

#define for_each_blk(blk) \
        for(blk= 0; blk < CFG_NAND_BLK_NUM; blk++)



