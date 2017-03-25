#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "../fscftl.h"
#include "wcb-mngr.h"
#include "../systbl/sys-meta.h"
#include "../datapath/ppa-ops.h"

/* write cache buffer control block */
struct wcb_lun_gctl *g_wcb_lun_ctl;

// the fifo api is not locked, the caller should guarantee race outside
void fsc_fifo_init(struct fsc_fifo *fifo)
{
	fifo->head = 0xffff;
	fifo->tail = 0xffff;
	fifo->size = 0;
}

void print_lun_entitys_fifo(void)
{
	printk("Lun entitys FIFO size(empty:%d full:%d ongoning:%d)", 
			g_wcb_lun_ctl->empty_lun.size, 
			g_wcb_lun_ctl->full_lun.size,
			g_wcb_lun_ctl->ongoing_lun.size);
}

// push to tail 
void push_lun_entity_to_fifo(struct fsc_fifo *fifo, struct wcb_lun_entity *entry)
{
	u32 old_tail;
	struct wcb_lun_entity *otentry;
	
	old_tail = fifo->tail;

	if (old_tail == 0xffff) {
		fifo->head = fifo->tail = entry->index;
		entry->next = 0xffff;
		entry->prev = 0xffff;
	} else {
		fifo->tail = entry->index;
		entry->next = old_tail;
		entry->prev = 0xffff;
		otentry = wcb_lun_entity_idx(old_tail);
		otentry->prev = entry->index;
	}

	fifo->size++;
}

// and pull from head
struct wcb_lun_entity *pull_lun_entity_from_fifo(struct fsc_fifo *fifo)
{
	u32 old_head = fifo->head;
	struct wcb_lun_entity *entry, *nhentry;

	if (old_head == 0xffff) {
		//printk("FiFO is empty\n");
		return NULL;
	}

	entry = wcb_lun_entity_idx(old_head);

	if (fifo->head == fifo->tail) {
		// only 1 entry in this fifo
		fifo->head = fifo->tail = 0xffff;
	} else {
		nhentry = wcb_lun_entity_idx(entry->prev);
		fifo->head = entry->prev; //nhentry->index;
		nhentry->next = 0xffff;
	}

	entry->next = entry->prev = 0xffff;
	fifo->size--;	
	return entry;
}

geo_ppa get_next_entity_baddr(geo_ppa curppa)
{
	bool carry;
	u8 ch, sec, pl, lun;
	u16 blk, pg;
	geo_ppa bppa;

	bppa.ppa = curppa.ppa;

	get_ppa_each_region(&bppa, &ch, &sec, &pl, &lun, &pg, &blk);

	//printk("curppa blk:%d pg:%d lun:%d\n", 
	//curppa.nand.blk, curppa.nand.pg, curppa.nand.lun);

	INCRE_BOUNDED(lun, LN_BITS, carry);
	IF_CARRY_THEN_INCRE_BOUNDED(carry, pg, PG_BITS);

	if (carry) { 
		carry = 0;
		blk = get_blk_from_free_list();
	}

	//IF_CARRY_THEN_INCRE_BOUNDED(carry, blk, BL_BITS);

	bppa.nand.ch = 0;
	bppa.nand.sec = 0;
	bppa.nand.pl = 0;
	bppa.nand.lun = lun;
	bppa.nand.pg = pg;
	bppa.nand.blk = blk;

	//printk("newppa blk:%d pg:%d lun:%d\n", 
	//bppa.nand.blk, bppa.nand.pg, bppa.nand.lun);

	return bppa;
}

// get next lun entity and set baddr according last ppa
struct wcb_lun_entity *get_next_lun_entity(geo_ppa curppa)
{
	struct wcb_lun_entity *lun_entity;

	/* Round Robin
	lun_entity = g_wcb_lun_ctl->lun_entitys + \
			((g_wcb_lun_ctl->entitynum++) % CB_ENTITYS_CNT);*/

	lun_entity = pull_lun_entity_from_fifo(&g_wcb_lun_ctl->empty_lun);
	if (lun_entity == NULL) {
		// Never Run here, we should keep a eye beforehand
		// to prevent run here
		// FIXME
		printk("Can't get_next_lun_entity\n");
		print_lun_entitys_fifo();
		return NULL;
	}

	lun_entity->baddr = get_next_entity_baddr(curppa);
	lun_entity->pos = 0;
	lun_entity->ch_status = 0;

	g_wcb_lun_ctl->partial_entity = lun_entity;

	debug_info("get LUNs(blk:%d pg:%d lun:%d) from empty fifo\n", 
		    lun_entity->baddr.nand.blk, 
		    lun_entity->baddr.nand.pg, 
		    lun_entity->baddr.nand.lun);

	return lun_entity;
}

struct wcb_lun_entity *get_lun_entity(geo_ppa startppa)
{
	u16 ch, ep ,pl, pos;
	struct wcb_lun_entity *lun_entity;

	lun_entity = pull_lun_entity_from_fifo(&g_wcb_lun_ctl->empty_lun);
	if (lun_entity == NULL) {
		// Never Run here, we should keep a eye beforehand
		// to prevent run here
		printk("Can't get_lun_entity\n");
		return NULL;
	}

	ch = startppa.nand.ch;
	ep = startppa.nand.sec;
	pl = startppa.nand.pl;
	pos = ch | (ep << CH_BITS) | (pl <<(CH_BITS + EP_BITS));

	startppa.nand.ch = 0;
	startppa.nand.sec = 0;
	startppa.nand.pl = 0;
	lun_entity->baddr = startppa;
	lun_entity->pos = pos;
	lun_entity->ch_status = 0;

	g_wcb_lun_ctl->partial_entity = lun_entity;

	debug_info("get LUNs(blk:%d pg:%d lun:%d) from empty fifo\n", 
			    lun_entity->baddr.nand.blk, 
			    lun_entity->baddr.nand.pg, 
			    lun_entity->baddr.nand.lun);

	return lun_entity;
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

	spin_lock_init(&g_wcb_lun_ctl->wcb_lock);
	spin_lock_init(&g_wcb_lun_ctl->fifo_lock);
	spin_lock_init(&g_wcb_lun_ctl->l2ptbl_lock);
	spin_lock_init(&g_wcb_lun_ctl->biolist_lock);

	bio_list_init(&g_wcb_lun_ctl->requeue_wr_bios);

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
	fsc_fifo_init(&g_wcb_lun_ctl->ongoing_lun);
	
	for (i = 0; i < CFG_NAND_LUN_NUM; i++) {
		fsc_fifo_init(&g_wcb_lun_ctl->read_lun[i]);
	}
    
	for (i = 0; i < CB_ENTITYS_CNT; i++) {
		/* all pushed to emptyfifo */
		push_lun_entity_to_fifo(&g_wcb_lun_ctl->empty_lun, wcb_lun_entity_idx(i));
	}

	printk("empty lun entitys fifo size:%d\n", g_wcb_lun_ctl->empty_lun.size);

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

static int wcb_single_lun_alloc(struct nvm_exdev *exdev, 
				struct wcb_lun_entity *lun_entitys)
{
	struct device *dmadev = &exdev->pdev->dev;

	lun_entitys->data = vzalloc(RAID_LUN_DATA_SIZE);
	if (!lun_entitys->data)
		return -ENOMEM;

	lun_entitys->meta = dma_alloc_coherent(dmadev, RAID_LUN_META_SIZE, 
					&lun_entitys->meta_dma, GFP_KERNEL);
	if (!lun_entitys->meta) 
        	goto out_free_data;
    
	lun_entitys->ppa = dma_alloc_coherent(dmadev, RAID_LUN_SEC_NUM * sizeof(u64), 
					&lun_entitys->ppa_dma, GFP_KERNEL);
	if (!lun_entitys->ppa)
        	goto out_free_meta;
    
	return 0;

out_free_meta:
	dma_free_coherent(dmadev, RAID_LUN_META_SIZE, 
			  lun_entitys->meta, lun_entitys->meta_dma);    
out_free_data:
	printk("dma_alloc_coherent fail\n");
	vfree(lun_entitys->data);
	return -ENOMEM;
}

static void wcb_single_lun_free(struct nvm_exdev *exdev, 
				struct wcb_lun_entity *lun_entitys)
{
	struct device *dmadev = &exdev->pdev->dev;

	dma_free_coherent(dmadev, RAID_LUN_META_SIZE, 
			  lun_entitys->ppa, lun_entitys->ppa_dma);

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

		atomic_set(&lun_entitys->fill_cnt, 0);

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


