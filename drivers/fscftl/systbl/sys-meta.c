/*
 * Manage FTL sys metadata, Various kind of table
 *	l2ptbl
 *	bmitbl
 *	firstpage
 *	ftllog
 *	vpctbl
 *	freelist/closelist/openblk
 */ 
#include <linux/vmalloc.h>
#include "sys-meta.h"

struct sys_status_tbl *statetbl;
struct bmi_item *bmitbl;
u32 *vpctbl;

// TODO::
// blk-pool lunentity-fifo unify use one interface
u32 pull_blk_from_pool(struct fsc_fifo *fifo)
{
	struct bmi_item *bmi, *nhbmi;
	u32 old_head = fifo->head;

	if (old_head == 0xffff)
		return 0xffff;

	bmi = bmitbl + old_head;

	if (fifo->head == fifo->tail) {
		fifo->head = fifo->tail = 0xffff;
	} else {
		nhbmi = bmitbl + bmi->prev;
		nhbmi->next = 0xffff;
		fifo->head = bmi->prev;	
	}

	fifo->size--;
	bmi->next = bmi->prev = 0xffff;
	
	return old_head;
}

void push_blk_to_pool(struct fsc_fifo *fifo, u32 blk)
{
	u32 old_tail;
	struct bmi_item *bmi, *ntbmi;
	
	old_tail = fifo->tail;
	bmi = bmitbl + blk;

	if (old_tail == 0xffff) {
		fifo->head = fifo->tail = blk;
		bmi->next = 0xffff;
		bmi->prev = 0xffff;
	} else {
		fifo->tail = blk;
		bmi->next = old_tail;
		bmi->prev = 0xffff;
		ntbmi = bmitbl + old_tail;
		ntbmi->prev = blk;
	}

	fifo->size++;
}

//form freelist when do power on bmitbl
u32 get_blk_from_free_list(void)
{

	u32 blk = pull_blk_from_pool(&statetbl->free_blk_pool);
        struct bmi_item *bmi;

	if (blk == 0xffff) {
		printk("ERROR no free blk !!! \n");
		return 0;
	}
        
        bmi = bmitbl + blk;

	printk("open blk:%d  left cnt:%d\n", blk, statetbl->free_blk_pool.size);
        bmi->bmstate = RAID_BLK_OPEN;

	return blk;
}

void insert_blk_to_free_list(u32 blk)
{
        struct bmi_item *bmi = bmitbl + blk;
        
	push_blk_to_pool(&statetbl->free_blk_pool, blk);
        bmi->bmstate = RAID_BLK_FREE;
}

int statetbl_init(void)
{
        // move this to primary-page
	statetbl = kzalloc(sizeof(struct sys_status_tbl), GFP_KERNEL);
	if (!statetbl)
		return -ENOMEM;

	fsc_fifo_init(&statetbl->closed_blk_pool);	
	fsc_fifo_init(&statetbl->free_blk_pool);
	fsc_fifo_init(&statetbl->gc_blk_pool);

	return 0;
}

void statetbl_exit(void)
{
	kfree(statetbl);
}

int bmitbl_init(void)
{
	int blk;
	struct bmi_item *bmi;

	bmitbl = vzalloc(BMITBL_SIZE);
	if (!bmitbl)
		return -ENOMEM;

	for (blk = 0; blk < CFG_NAND_BLOCK_NUM; blk++) {
		bmi = bmitbl + blk;
		bmi->blknum = blk;
		bmi->next = 0xffff;		
		bmi->prev = 0xffff;
	}

	return 0;
}

void bmitbl_exit(void)
{
	vfree(bmitbl);
}

int vpctbl_init(void)
{
        vpctbl = kzalloc(VPCTBL_SIZE);
        if (!vpctbl)
                return -ENOMEM;
}

void vpctbl_exit(void)
{
        kfree(vpctbl);
}

int l2ptbl_init(struct nvm_exdev *exdev)
{
	exdev->l2ptbl = vmalloc(USR_FTLTBL_SIZE);
	if (!exdev->l2ptbl) {
		printk("l2ptbl malloc failed\n");
		return -ENOMEM;
	}

        memset(exdev->l2ptbl, INVALID_PAGE, USR_FTLTBL_SIZE);

        exdev->l2pl1tbl = kzalloc(L1TBL_SIZE);
        if (!exdev->l2pl1tbl)
                goto free_l2ptbl;

	return 0;
        
free_l2ptbl:
	vfree(exdev->l2ptbl);
        return -ENOMEM;
}

void l2ptbl_exit(struct nvm_exdev *exdev)
{
	vfree(exdev->l2ptbl);
        kfree(exdev->l2pl1tbl);
}


