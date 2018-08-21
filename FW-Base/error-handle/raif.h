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
#define RAIF_DIES_NUM		48; // 96 / 192


//rpu: Raif Protection Unit, cross multi Dies according config paramter @RAIF_DIES_NUM
u8 rpu_start_die(ppa_t ppa)
{
	u8 die = PPA_TO_DIE(ppa);

	die = rounddown(die, RAIF_DIES_NUM);

	// this ppa in a raif unit[die, die+RAIF_DIES_NUM-1]

	return die;
}

u8 rpu_end_die(ppa_t ppa)
{
	u8 die = rpu_start_die(ppa);
	
	return die + RAIF_DIES_NUM - 1;
}

#endif
