#ifndef _PPA_OPS_H_
#define _PPA_OPS_H_

#include "../fscftl.h"

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


int nvm_rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
					  int nr_ppas, u16 ctrl, void *databuf, void *metabuf);

void run_testcase(struct nvm_exdev *exdev);

#endif
