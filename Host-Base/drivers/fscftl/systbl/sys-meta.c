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
u32 *vpctbl;   // prevent by l2plock

void mark_bbt_tbl(u32 blk, u32 lun, u32 ch, bool status)
{
	struct bmi_item *bmi = get_bmi_item(blk);

	if (status == BADB)
		bmi->bbt[lun] |= (1<<ch);
	else
		bmi->bbt[lun] &= ~(1<<ch);
}

bool is_ppa_badblock(geo_ppa ppa)
{
	struct bmi_item *bmi = get_bmi_item(ppa.nand.blk);
	u16 bbt = bmi->bbt[ppa.nand.lun];

	return BIT_TEST(bbt, ppa.nand.ch);
}

bool is_ppa_firstpage(geo_ppa ppa)
{
	if ((ppa.nand.sec == 0) &&
	    (ppa.nand.pl == 0)  &&
	    (ppa.nand.lun == 0) &&
	    (ppa.nand.pg == 0))
	    return true;

	return false;
}

PPA_TYPE sys_get_ppa_type(geo_ppa ppa)
{
	// badblock
	//if (is_ppa_badblock(ppa))
		//return BAD_BLK;
	
	// firstpage

	// ftllog

	// xor parity

	return USR_DATA;
}

// TODO::
// blk-pool lunentity-fifo unify use one interface
u32 pull_blk_from_pool(struct fsc_fifo *fifo)
{
	struct bmi_item *bmi, *nhbmi;
	u32 old_head = fifo->head;

	if (old_head == 0xffff)
		return 0xffff;

	bmi = get_bmi_item(old_head);

	if (fifo->head == fifo->tail) {
		fifo->head = fifo->tail = 0xffff;
	} else {		
		nhbmi = get_bmi_item(bmi->prev);
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
	bmi = get_bmi_item(blk);

	if (old_tail == 0xffff) {
		fifo->head = fifo->tail = blk;
		bmi->next = 0xffff;
		bmi->prev = 0xffff;
	} else {
		fifo->tail = blk;
		bmi->next = old_tail;
		bmi->prev = 0xffff;		
		ntbmi = get_bmi_item(old_tail);
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
        	
	bmi = get_bmi_item(blk);

	printk("open blk:%4d  left cnt:%d\n", blk, statetbl->free_blk_pool.size);
        bmi->bmstate = RAID_BLK_OPEN;

	return blk;
}

void insert_blk_to_free_list(u32 blk)
{
        struct bmi_item *bmi = get_bmi_item(blk);
        
	push_blk_to_pool(&statetbl->free_blk_pool, blk);
        bmi->bmstate = RAID_BLK_FREE;
	
	printk("insert blk:%4d to free_list\n", blk);
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
        vpctbl = kzalloc(VPCTBL_SIZE, GFP_KERNEL);
        if (!vpctbl)
                return -ENOMEM;

	return 0;
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

	exdev->l2pl1tbl = kzalloc(L1TBL_SIZE, GFP_KERNEL);
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


