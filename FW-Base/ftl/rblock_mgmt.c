/*
 * Copyright (C) 2018-2020 NET-Swift.
 * Initial release: Dengcai Xu <dengcaixu@net-swift.com>
 *
 * ALL RIGHTS RESERVED. These coded instructions and program statements are
 * copyrighted works and confidential proprietary information of NET-Swift Corp.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part.
 *
 * function: 
 * R-Block management, R-Block dlist/rbtree, insert/delete
 *
 */


#include "list.h"
 
 
 // push at tail
void push_to_rblk_dlist(dlist *dlist, u16 blk)
{
    u16 old_tail;
	bmi *item, *nitem;
	 
	old_tail = dlist->tail;
	item = GET_BMI(blk);
 
	if (old_tail == 0xffff) {
		// the dlist is empty
		dlist->head = dlist->tail = blk;
		item->next = 0xffff;
		item->prev = 0xffff;
	} else {
		dlist->tail = blk;
		item->next = old_tail;
		item->prev = 0xffff;
		nitem = GET_BMI(old_tail);
		nitem->prev = blk;
	}

	dlist->size++;
}
 
 // pull from head
u16 pull_from_rblk_dlist(dlist *dlist)
{
	bmi *item, *pitem;
	u16 old_head = dlist->head;

	if (old_head == 0xffff)
		return DLIST_EMPTY;   // dlist is empty

	item = GET_BMI(old_head);

	if (dlist->head == dlist->tail) {
		// dlist only 1 node
		dlist->head = dlist->tail = 0xffff;
	} else {
		// dlist conatin at least 2 node
		pitem = get_prev_bmi(old_head);
		pitem->next = 0xffff;
		dlist->head = item->prev;  
	}

	dlist->size--;
	item->next = item->prev = 0xffff;

	return old_head;
}

u16 get_blk_from_free_list(void)
{
	u16 blk;
	bmi *item;

	boot_blk_primary_page *primary_page = get_primary_page();

	blk = pull_from_rblk_dlist(&primary_page->page.free_blk_list);
	if (blk == DLIST_EMPTY) {
		//NEVER
		//panic();
	}

	item = GET_BMI(blk);
	item->state = RBLK_OPEN;
	item->timestamp = current_timestamp();
	item->sequence = sequence_id++;
	
	return blk;
}

void add_blk_to_free_list(u16 blk)
{
	bmi *item = GET_BMI(blk);
	boot_blk_primary_page *primary_page = get_primary_page();

	push_to_rblk_dlist(&primary_page->page.free_blk_list, blk);

	item = GET_BMI(blk);
	item->state = RBLK_FREE;
	item->timestamp = 0;
	item->sequence = 0;
}

