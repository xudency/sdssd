#ifndef _PPA_OPS_H_
#define _PPA_OPS_H_

#include "../fscftl.h"

//#define debug_info printk
#define debug_info(format, arg...)  do {} while (0)

#define INCRE_BOUNDED(n, bits, carry) { if (++n == (1 << bits)) { n = 0; carry = 1; } else { carry = 0; } }
#define IF_CARRY_THEN_INCRE_BOUNDED(carry, n, bits) { if (carry) { carry = 0; INCRE_BOUNDED(n, bits, carry); } }

#define CH_INCRS 0
#define EP_INCRS 1

struct nvme_ppa_iod {
	void *ctx;	// lun entity
	struct nvm_exdev *dev;

	void *vaddr_data;
	dma_addr_t dma_data;

	void *vaddr_meta;
	dma_addr_t dma_meta;

	u64 *vaddr_ppalist;
	dma_addr_t dma_ppalist;
	int idx;   // which line or which ch
};

void get_ppa_each_region(geo_ppa *ppa, u8 *ch, u8 *sec, u8 *pl, 
                               u8 *lun, u16 *pg, u16 *blk);

void set_ppa_nand_addr(geo_ppa *ppa, u8 ch, u8 sec, 
                            u8 pl, u8 lun, u16 pg, u16 blk);
#endif
