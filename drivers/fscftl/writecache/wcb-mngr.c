#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "../fscftl.h"
#include "wcb-mngr.h"

/* write cache buffer control block */
struct wcb_lun_gctl *g_wcb_lun_ctl;

void fsc_fifo_init(struct fsc_fifo *fifo)
{
	fifo->head = 0xffff;
	fifo->tail = 0xffff;
	fifo->size = 0;
}

static int wcb_lun_ctl_init(void)
{
	int i;
	int ret = 0, debug = 0;

	g_wcb_lun_ctl = kzalloc(sizeof(*g_wcb_lun_ctl), GFP_KERNEL);
	if (!g_wcb_lun_ctl) {
		printk("%s-%d  failed\n", __FUNCTION__, debug++);
		return -ENOMEM;
	}

	spin_lock_init(&g_wcb_lun_ctl->wcb_fifo_lock);

	g_wcb_lun_ctl->lun_entitys = \
		vzalloc(sizeof(struct wcb_lun_entity) * CB_ENTITYS_CNT);
	if (!g_wcb_lun_ctl->lun_entitys) {
		printk("%s-%d  failed\n", __FUNCTION__, debug++);
		ret = -ENOMEM;
		goto out_free_ctl;
	}

	for (i = 0; i < CFG_NAND_LUN_NUM; i++) {
		g_wcb_lun_ctl->ongoing_pg_num[i] = 0xdead;		
		g_wcb_lun_ctl->ongoing_pg_cnt[i] = 0;
	}

	/* fifo init */
	fsc_fifo_init(&g_wcb_lun_ctl->empty_lun);	
	fsc_fifo_init(&g_wcb_lun_ctl->full_lun);
	
	for (i = 0; i < CFG_NAND_LUN_NUM; i++) {
		fsc_fifo_init(&g_wcb_lun_ctl->read_lun[i]);
	}

	return 0;

out_free_ctl:
	kfree(g_wcb_lun_ctl);
	return ret;
}

static void wcb_lun_ctl_exit(void)
{
	vfree(g_wcb_lun_ctl->lun_entitys);
	kfree(g_wcb_lun_ctl);
}

// TODO:: to check vmalloc memory can be dma_map_single ??

static int wcb_single_lun_alloc(struct nvm_exdev *exdev, 
							struct wcb_lun_entity *lun_entitys)
{
	struct device *dmadev = &exdev->pdev->dev;

	lun_entitys->data = vzalloc(RAID_LUN_DATA_SIZE);
	if (!lun_entitys->data)
		return -ENOMEM;

	lun_entitys->meta = dma_alloc_coherent(dmadev, RAID_LUN_META_SIZE, 
								&lun_entitys->meta_dma, GFP_KERNEL);
	if (!lun_entitys->meta) {
		printk("dma_alloc_coherent fail\n");
		vfree(lun_entitys->data);
		return -ENOMEM;
	}

	return 0;
}

static void wcb_single_lun_free(struct nvm_exdev *exdev, 
							struct wcb_lun_entity *lun_entitys)
{
	struct device *dmadev = &exdev->pdev->dev;

	dma_free_coherent(dmadev, RAID_LUN_META_SIZE, 
					  lun_entitys->meta, lun_entitys->meta_dma);
	
	vfree(lun_entitys->data);
}


static int wcb_lun_mem_alloc(struct nvm_exdev *exdev)
{
	int i;
	struct wcb_lun_entity *lun_entitys;

	for (i = 0; i < CB_ENTITYS_CNT ; i++) {
		lun_entitys = g_wcb_lun_ctl->lun_entitys + i;
		lun_entitys->index = i;

		if (wcb_single_lun_alloc(exdev, lun_entitys))
			goto free_lun_mem;
	}

	return 0;

free_lun_mem:
	while (--i) {
		lun_entitys = g_wcb_lun_ctl->lun_entitys + i;
		wcb_single_lun_free(exdev, lun_entitys);
	}

	return -ENOMEM;
}

static void wcb_lun_mem_free(struct nvm_exdev *exdev)
{
	int i;	
	struct wcb_lun_entity *lun_entitys;	

	for (i = 0; i < CB_ENTITYS_CNT ; i++) {
		lun_entitys = g_wcb_lun_ctl->lun_entitys + i;
		wcb_single_lun_free(exdev, lun_entitys);
	}
}

int write_cache_alloc(struct nvm_exdev *exdev)
{
	int ret;

	ret = wcb_lun_ctl_init();
	if (ret)
		return ret;

	ret = wcb_lun_mem_alloc(exdev);
	if (ret) {
		goto out_wcb_lun_ctl;
	}

	return 0;
	
out_wcb_lun_ctl:
	wcb_lun_ctl_exit();
	return ret;
}

void write_cache_free(struct nvm_exdev *exdev)
{
	wcb_lun_mem_free(exdev);
	wcb_lun_ctl_exit();
}

/*
struct device *dmadev = &exdev->pdev->dev;

lun_entitys->meta_dma = dma_map_single(dmadev, lun_entitys->meta, 
						  RAID_LUN_META_SIZE, DMA_TO_DEVICE);


dma_unmap_single(dmadev, lun_entitys->meta_dma, 
				 RAID_LUN_META_SIZE, DMA_TO_DEVICE);


g_ppalist_buf = dma_alloc_coherent(dmadev, PAGE_SIZE, &g_ppalist_dma, GFP_KERNEL);
if (!g_ppalist_buf) {
	printk("%s dma alloc ppalist_buf failed\n", __FUNCTION__);
	goto err_free_meta;
}
memset(g_ppalist_buf, 0x00, PAGE_SIZE);

dma_free_coherent(dmadev, PAGE_SIZE, g_ppalist_buf, g_ppalist_dma);
*/


