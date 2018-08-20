#ifndef _BADBLOCK_MGMT_H_
#define _BADBLOCK_MGMT_H_


// bbt unit, 
/*typedef struct {
    u8 pl   : PL_BITS;
    u8 ch   :
    u8 lun;
    u16 blk;
} bbt_uint;
*/ 

extern u16 *get_blk_bbt_base(u16 blk);



#endif
