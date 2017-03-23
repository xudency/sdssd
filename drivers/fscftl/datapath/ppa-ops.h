#ifndef _PPA_OPS_H_
#define _PPA_OPS_H_

#include "../fscftl.h"

//#define debug_info printk
#define debug_info(format, arg...)  do {} while (0)

#define INCRE_BOUNDED(n, bits, carry) { if (++n == (1 << bits)) { n = 0; carry = 1; } else { carry = 0; } }
#define IF_CARRY_THEN_INCRE_BOUNDED(carry, n, bits) { if (carry) { carry = 0; INCRE_BOUNDED(n, bits, carry); } }


struct nvme_ppa_iod {
	void *vaddr_meta;
	dma_addr_t *dma_meta;

	void *vaddr_ppalist;
	dma_addr_t *dma_ppalist;
};

typedef union phy_ppa {
	u64 nandppa;
	dma_addr_t dma_ppa_list;
} dmappa;

void get_ppa_each_region(geo_ppa *ppa, u8 *ch, u8 *sec, 
						 u8 *pl, u8 *lun, u16 *pg, u16 *blk); 

int nvm_rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
					  int nr_ppas, u16 ctrl, void *databuf, void *metabuf);

void run_testcase(struct nvm_exdev *exdev);

#endif
