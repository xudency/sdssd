 /*
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

#include "../nvme/host/nvme.h"
#include "hwcfg/cfg/flash_cfg.h"
#include "hwcfg/regrw.h"
#include "build_recovery/power.h"
#include "fscftl.h"
#include "datapath/ppa-ops.h"*/


void *g_meta_buf;
dma_addr_t g_meta_dma;

u64 *g_ppalist_buf;
dma_addr_t g_ppalist_dma;

void **g_data_buf;

struct wcb_lun {
	int a;	
}

int write_cache_alloc(struct nvm_exdev *exdev)
{
	int i;
	void *buf;
	struct device *dmadev = &exdev->pdev->dev;

	g_meta_buf = dma_alloc_coherent(dmadev, PAGE_SIZE*5, &g_meta_dma, GFP_KERNEL);
	if (!g_meta_buf) {
		printk("%s dma alloc meta_buf failed\n", __FUNCTION__);
		return -ENOMEM;
	}
	memset(g_meta_buf, 0x00, PAGE_SIZE*5);

	g_ppalist_buf = dma_alloc_coherent(dmadev, PAGE_SIZE, &g_ppalist_dma, GFP_KERNEL);
	if (!g_ppalist_buf) {
		printk("%s dma alloc ppalist_buf failed\n", __FUNCTION__);
		goto err_free_meta;
	}
	memset(g_ppalist_buf, 0x00, PAGE_SIZE);

	exdev->wcb = kzalloc(sizeof(struct fsc_cache_entry *) * WRITE_CACHE_SEC_NUM, GFP_KERNEL);
	if (!exdev->cwb) {
		print("%s wcb entry alloc failed");
		goto err_free_ppalist;
	}

	for (i = 0; i < WRITE_CACHE_SEC_NUM; i++) {
		exdev->wcb[i] = kzalloc(sizeof(struct fsc_cache_entry), GFP_KERNEL);
	}


	// ch ep pl lun (2+8 page)
	for (i = 0; i < WRITE_CACHE_SEC_NUM; i++) {
		buf = kzalloc(CFG_NAND_EP_SIZE + NAND_META_SIZE, GFP_KERNEL);
		exdev->wcb[i]->data = buf;
		exdev->wcb[i]->meta = (void *)((unsigned long)buf + CFG_NAND_EP_SIZE);	
	}



	//rl = vmalloc(RAID_LUN_SEC_NUM * CFG_NAND_EP_SIZE);



	return 0;

err_free_ppalist:
	dma_free_coherent(dmadev, PAGE_SIZE, g_ppalist_buf, g_ppalist_dma);
err_free_meta:
	dma_free_coherent(dmadev, PAGE_SIZE*5, g_meta_buf, g_meta_dma);
	return -ENOMEM;
}

void write_cache_free(struct nvm_exdev *exdev)
{
	struct device *dmadev = &exdev->pdev->dev;

	dma_free_coherent(dmadev, PAGE_SIZE, g_ppalist_buf, g_ppalist_dma);
	dma_free_coherent(dmadev, PAGE_SIZE*5, g_meta_buf, g_meta_dma);
}

