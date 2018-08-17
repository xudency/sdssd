#include <linux/delay.h>
#include "power.h"
#include "../bootblk/bootblk-mngr.h"
#include "../datapath/bio-datapath.h"

void flush_sysmeta_to_wcb(struct nvm_exdev *exdev, 
                          struct wcb_bio_ctx *wcb_resource, void *src_base)
{
	int i, idx = 0;
	u32 lba;
	u16 pos, bpos, epos;
	unsigned long flags;
	struct wcb_ctx *wcb_1ctx;
	struct wcb_lun_entity *entity = NULL;

	for (i=0; i < MAX_USED_WCB_ENTITYS; i++) {
		void *src, *dst;
		wcb_1ctx = &wcb_resource->bio_wcb[i];
		entity = wcb_1ctx->entitys;
		if (!entity)
			break;
		
		bpos = wcb_1ctx->start_pos;
		epos = wcb_1ctx->end_pos;
		for (pos = bpos; pos <= epos; pos++) {
			lba = entity->lba[pos];
			dst = wcb_entity_offt_data(entity->index, pos);
			src = src_base + EXP_PPA_SIZE * idx;
			idx++;

			if (lba >= EXTEND_LBA_BASE && lba < EXTEND_LBA_RSVD0) {
				memcpy(dst, src, EXP_PPA_SIZE); 		
			} else {
				TRACE_TAG("LBA out of range");
			}

			// all travesal ppa need inc, regardless of pagetype
			if (atomic_inc_return(&entity->fill_cnt) == RAID_LUN_SEC_NUM) {				
				
				atomic_set(&entity->fill_cnt, 0);
				spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
				push_lun_entity_to_fifo(&g_wcb_lun_ctl->full_lun, entity);
				spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
				
				wake_up_process(exdev->writer_thread);
			}
		}
	}
}

void flush_dummy_lun(struct nvm_exdev *exdev)
{
	int i;
	unsigned long flags;
	struct wcb_lun_entity *partial = partial_wcb_lun_entity();
	u16 pos = partial->pos;

	if (pos == 0)
		return;

	for (i = pos; i < RAID_LUN_SEC_NUM; i++) {		
		void *wcbuf = wcb_entity_offt_data(partial->index, pos);
		memset(wcbuf, 0x00, EXP_PPA_SIZE);
	}

	spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
	push_lun_entity_to_fifo(&g_wcb_lun_ctl->full_lun, partial);
	get_next_lun_entity(partial->baddr);
	spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
	
	atomic_set(&partial->fill_cnt, 0);

	wake_up_process(exdev->writer_thread);

	return;
}

void flush_ftl_internal_systbl(struct nvm_exdev *exdev, void * const buff, 
				u32 nr_ppas, u32 slba)
{
	u32 budget = 1;
	u32 left_ppas = nr_ppas;
	void *databuf = buff;

	while (1) {
		unsigned long flags;
		struct wcb_bio_ctx wcb_resource;

		memset(&wcb_resource, 0x00, sizeof(wcb_resource));

		if (left_ppas == 0)
			break;

	retry_wcb:
		spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
		if (wcb_available(budget)) {
			alloc_wcb_core(slba, budget, &wcb_resource);
			spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
		} else {
			spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

			// TODO:: use completion Notify replace msleep
			msleep(1);
			goto retry_wcb;
		}

		flush_sysmeta_to_wcb(exdev, &wcb_resource, databuf);
		slba += budget;
		left_ppas -= budget;
		databuf = (void *)((uintptr_t)databuf + EXP_PPA_SIZE*budget);
		//spin_lock(&g_wcb_lun_ctl->l2ptbl_lock);
		//set_l2ptbl_write_path(exdev, &wcb_resource);
		//spin_unlock(&g_wcb_lun_ctl->l2ptbl_lock);
	}
}

void flush_user_l2ptbl(struct nvm_exdev *exdev)
{
	printk("flush l2p table start");

	flush_ftl_internal_systbl(exdev, exdev->l2ptbl, 
				  USR_FTLTBL_SEC_NUM, EXTEND_LBA_UFTL);
	
	flush_dummy_lun(exdev);

	printk("flush l2p table complete");

	//wait_done();
}

int rebuild_systbl(struct nvm_exdev *exdev)
{
	printk("start %s\n", __FUNCTION__);

	// L2->L1

	// bmitbl

	// vpctbl

	// open blk ftllog

	// l2ptbl
	printk("complete %s\n", __FUNCTION__);

	return 0;
}

void flush_down_systbl(struct nvm_exdev *exdev)
{
	printk("start %s\n", __FUNCTION__);

	/* l2ptbl */
	//flush_user_l2ptbl(exdev);

	// bmitbl

	// vpc

	// L1

	// openblk ftllog

	// 8 padding page of dummy data to stable the User data

	bootblk_flush_bbt_page(exdev);
	bootblk_flush_meta_page(exdev, POWER_DOWN_SAFE);

	printk("complete %s\n", __FUNCTION__);

	return;
}

// return 0: rebuild systbl success, or rebuild fail
int try_recovery_systbl(struct nvm_exdev *exdev)
{
	int result;
	enum power_down_flag power_flag = POWER_DOWN_SAFE;

	printk("start %s\n", __FUNCTION__);

	result = bootblk_recovery_meta_page(exdev);
	if (unlikely(result))
		return 1;

	result = bootblk_recovery_bbt_page(exdev);
	if (unlikely(result))
		return 1;

	print_pdf(power_flag);

	if (power_flag == POWER_DOWN_SAFE) {
		result = rebuild_systbl(exdev);
	} else if (power_flag == POWER_DOWN_UNSAFE) {
		result = crash_recovery(exdev);
	} else {
		printk("primary page power_flag:%d invalid\n", power_flag);
		return 1;
	}

	bootblk_flush_bbt_page(exdev);
	bootblk_flush_meta_page(exdev, POWER_DOWN_UNSAFE);

	printk("complete %s\n", __FUNCTION__);

	return result;
}

