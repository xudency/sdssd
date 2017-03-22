#ifndef _PPA_OPS_H_
#define _PPA_OPS_H_

#include "../fscftl.h"

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

geo_ppa get_next_entity_baddr(geo_ppa curppa);

int nvm_rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
					  int nr_ppas, u16 ctrl, void *databuf, void *metabuf);

void run_testcase(struct nvm_exdev *exdev);

#endif
