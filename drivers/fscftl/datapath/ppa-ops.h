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
	void *vaddr_meta;
	dma_addr_t *dma_meta;

	void *vaddr_ppalist;
	dma_addr_t *dma_ppalist;
};

typedef union phy_ppa {
	u64 nandppa;
	dma_addr_t dma_ppa_list;
} dmappa;

typedef struct pos_ep_ppa
{
  union
  {
    // ppa format in nand
    struct
    {
      u16 sec       : EP_BITS;  // 4 sectors per page
      u16 pl        : PL_BITS;  // 2 planes per LUN
      u16 ch        : CH_BITS;  // 16 channels
      u16 resved    : (16-EP_BITS-PL_BITS-CH_BITS);
    } bits;

    u16 all;
  };
} geo_pos;

void get_ppa_each_region(geo_ppa *ppa, u8 *ch, u8 *sec, u8 *pl, 
                               u8 *lun, u16 *pg, u16 *blk);

void set_ppa_nand_addr(geo_ppa *ppa, u8 ch, u8 sec, 
                            u8 pl, u8 lun, u16 pg, u16 blk);

void ppa_step_ep(geo_ppa *ppa);

int nvm_rdpparaw_sync(struct nvm_exdev *exdev, struct physical_address *ppa, 
                             int nr_ppas, u16 ctrl, void *databuf, void *metabuf);

void run_testcase(struct nvm_exdev *exdev);

#endif
