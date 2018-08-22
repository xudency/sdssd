#ifndef _RAIF_H_
#define _RAIF_H_

#define RAIF_DISABLE		0
#define RAIF1_ENABLE		1
#define RAIF2_ENABLE		2
// choose RAIF Mode from above 3 mode  
#define RAIF_MODE   		RAIF1_ENABLE		 		   	

//how many Die to generate a RAIF parity it MUST Channel*N
//for emample config this as 48, RAIF1 is enable && RAIF2 Disable
//  Die[000 - 047]:  00-46 is user data,  47 is parity
//  Die[048 - 095]:  48-94 is user data,  95 is parity
//  Die[096 - 143]:  96-142 is user data, 143 is parity
//  Die[144 - 191]:  144-190 is user data, 191 is parity
#define RPU_DIES_NUM_CFG		48; // 96 / 192

#define for_each_rpu(die) \
	for(die=0; die<CFG_NAND_DIE_NUM; die+=RPU_DIES_NUM_CFG)

void raif_die_recalibrate(u16 blk);


#endif
