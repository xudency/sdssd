#include <linux/kthread.h>
#include <linux/delay.h>
#include "ppa-ops.h"
#include "bio-datapath.h"
#include "../fscftl.h"
#include "../writecache/wcb-mngr.h"

/*  loop fifo
{
	u32 num;
	struct wcb_lun_entity *entry;
	struct fsc_fifo *full_lun =  &g_wcb_lun_ctl->full_lun;

	num = full_lun->head;
	while (num != 0xffff) {
		entry = wcb_lun_entity_idx(num);
		num = entry->prev;
		printk("walk find full fifo entity%d\n", entry->index);
	}						

	printk("****************************************");
}
*/

/* Write datapath backend, pull a LUN entity from fullfifo, 
 * and check if It can Submit Now(LUN BUSY?) NO NEED
 * flush data from wcb to NandFlash
 * return 1: continue to pull FUll LUN,  0: no full fifo thread schedule out
 */
static struct workqueue_struct *requeue_bios_wq;


int submit_write_backend(struct nvm_exdev *exdev)
{
	unsigned long flags;
	struct fsc_fifo *full_lun =  &g_wcb_lun_ctl->full_lun;
	struct fsc_fifo *ongoing_lun =  &g_wcb_lun_ctl->ongoing_lun;	
	struct wcb_lun_entity *entity;

	spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);

	entity = pull_lun_entity_from_fifo(full_lun);
	if(entity == NULL) {
		// full fifo is empty
		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
		return 0;
	}

	debug_info("Fscftl-Writer find LUN(blk:%d pg:%d lun:%d) in full fifo\n", 
		    entity->baddr.nand.blk, 
		    entity->baddr.nand.pg, 
		    entity->baddr.nand.lun);
	debug_info("submit LUNs(blk:%d pg:%d lun:%d) ongoing and wait for cqe back\n", 
		    entity->baddr.nand.blk, 
		    entity->baddr.nand.pg, 
		    entity->baddr.nand.lun);

	push_lun_entity_to_fifo(ongoing_lun, entity);

	spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

	// Simulation IRQ back
	queue_work(requeue_bios_wq, &exdev->yirq);

	return 1;
}

// Simulation HW IRQ callback fn Timer
void simulate_cqe_back_fn(unsigned long data)
{
	unsigned long flags;
	struct nvm_exdev *exdev = (struct nvm_exdev *)data;
	struct fsc_fifo *empty_lun =  &g_wcb_lun_ctl->empty_lun;
	struct fsc_fifo *ongoing_lun =  &g_wcb_lun_ctl->ongoing_lun;	
	struct wcb_lun_entity *entity;

	while (1) {
		spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);

		entity = pull_lun_entity_from_fifo(ongoing_lun);
		if (entity == NULL) {
			spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
			break;
		}
	    
		push_lun_entity_to_fifo(empty_lun, entity);

		debug_info("LUNs(blk:%d pg:%d lun:%d) write complete push to empty fifo\n", 
			    entity->baddr.nand.blk, 
			    entity->baddr.nand.pg, 
			    entity->baddr.nand.lun);

		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

		// Now wcb is available, we can process the pending bios in bio_list
		// IRQ should keep as short as it can, Moreover it hold lock
		//wake_up(wait_queue_head_t);
		//completion notify
		//workqueue
		//mod_timer(&exdev->cqe_timer, jiffies + msecs_to_jiffies(100));
		queue_work(requeue_bios_wq, &exdev->requeue_ws);
	}
}

int fscftl_write_kthread(void *data)
{
	struct nvm_exdev *exdev = data;	

	while (!kthread_should_stop()) {
		if (submit_write_backend(exdev))
			continue;
				
		set_current_state(TASK_INTERRUPTIBLE);
		io_schedule();
	}

	return 0;
}

static void fscftl_requeue_workfn(struct work_struct *work)
{
	struct nvm_exdev *exdev = container_of(work, struct nvm_exdev, requeue_ws);
	struct nvm_exns *ns = (struct nvm_exns *)exdev->private_data;
	struct bio_list bios;
	struct bio *bio;

	bio_list_init(&bios);

	spin_lock(&g_wcb_lun_ctl->biolist_lock);
	bio_list_merge(&bios, &g_wcb_lun_ctl->requeue_wr_bios);
	bio_list_init(&g_wcb_lun_ctl->requeue_wr_bios);
	spin_unlock(&g_wcb_lun_ctl->biolist_lock);

	while ((bio = bio_list_pop(&bios)))
		fscftl_make_rq(ns->queue, bio);
}

// Simulation HW IRQ callback fn Workqueue
static void fscftl_yirq_workfn(struct work_struct *work)
{
	unsigned long flags;
	struct nvm_exdev *exdev = container_of(work, struct nvm_exdev, yirq);
	struct fsc_fifo *empty_lun =  &g_wcb_lun_ctl->empty_lun;
	struct fsc_fifo *ongoing_lun =	&g_wcb_lun_ctl->ongoing_lun;	
	struct wcb_lun_entity *entity;

	// loop ?
	while (1) {
		spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);

		entity = pull_lun_entity_from_fifo(ongoing_lun);
		if (entity == NULL) {
			spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
			break;
		}
		
		push_lun_entity_to_fifo(empty_lun, entity);
		
		debug_info("LUNs(blk:%d pg:%d lun:%d) write complete push to emptyfifo\n", 
					entity->baddr.nand.blk, 
					entity->baddr.nand.pg, 
					entity->baddr.nand.lun);

		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

		queue_work(requeue_bios_wq, &exdev->requeue_ws);
	}
}

int fscftl_writer_init(struct nvm_exdev *exdev)
{
	requeue_bios_wq = alloc_workqueue("bios-dequeue", WQ_UNBOUND, 0);

	exdev->writer_thread = kthread_create(fscftl_write_kthread, exdev, "fscftl-writer");

	setup_timer(&exdev->cqe_timer, simulate_cqe_back_fn, (unsigned long)exdev);

	wake_up_process(exdev->writer_thread);

	INIT_WORK(&exdev->requeue_ws, fscftl_requeue_workfn);	
	INIT_WORK(&exdev->yirq, fscftl_yirq_workfn);

	return 0;
}

void fscftl_writer_exit(struct nvm_exdev *exdev)
{
	if (exdev->writer_thread)
		kthread_stop(exdev->writer_thread);
	del_timer(&exdev->cqe_timer);

	destroy_workqueue(requeue_bios_wq);
}

PPA_TYPE sys_get_ppa_type(geo_ppa ppa)
{
	// lookup BMI->bb

	// xor last ch

	// ep pl ln pg=0

	// 255 511 pg ep=3

	return USR_DATA;
}

bool wcb_available(int nr_ppas)
{
	u16 left;
	struct wcb_lun_entity *entity;
	u32 emptysize = g_wcb_lun_ctl->empty_lun.size;
	u16 extra = (nr_ppas >> 2) + 2;    // rsvd for sysmeta and bb and xor, etc 

	entity = partial_wcb_lun_entity();
	left = RAID_LUN_SEC_NUM - (entity->pos);

	if ((nr_ppas+extra) >= left && emptysize == 0)
		return false;

	return true;
}

// alloc len valid Normal PPA for this coming io
int alloc_wcb_core(sector_t slba, u32 nr_ppas, struct wcb_bio_ctx *wcb_resource)
{
	int loop = 1;
	int num = 0;
	geo_ppa curppa;
	u32 left_len = nr_ppas;
	struct wcb_lun_entity *cur_lun_entity = partial_wcb_lun_entity();

	wcb_resource->bio_wcb[num].entitys = cur_lun_entity;	
	wcb_resource->bio_wcb[num].start_pos = cur_lun_entity->pos;	
	wcb_resource->bio_wcb[num].end_pos = cur_lun_entity->pos;

	//curppa = current_ppa();
	curppa.ppa = cur_lun_entity->baddr.ppa + cur_lun_entity->pos;
    
	while (loop) 
	{
		switch (sys_get_ppa_type(curppa)) {
		case USR_DATA:
			left_len--;
			cur_lun_entity->lba[cur_lun_entity->pos] = slba++;
			cur_lun_entity->ppa[cur_lun_entity->pos] = curppa.ppa;   //EP+
			wcb_resource->bio_wcb[num].end_pos = cur_lun_entity->pos;
			cur_lun_entity->pos++;

			if (cur_lun_entity->pos == RAID_LUN_SEC_NUM) {	
				cur_lun_entity = get_next_lun_entity(curppa);
				if (!cur_lun_entity)
					return -1;

				if (left_len == 0) {
					loop = 0;
					break;
				}
						
				num++;
				wcb_resource->bio_wcb[num].entitys = cur_lun_entity;
				wcb_resource->bio_wcb[num].start_pos = cur_lun_entity->pos;
				wcb_resource->bio_wcb[num].end_pos = cur_lun_entity->pos;

				curppa = cur_lun_entity->baddr;
			} else {
			// TODO:: use EP+ Mode
				curppa.ppa++;
			}

			if (left_len == 0) {
				/* wcb is prepare ready */
				loop = 0;
			}

			break;
	    
		// TODO::
		default:
	   		break;
		}
	}

    return 0;
}

void bio_memcpy_wcb(struct wcb_ctx *wcb_1ctx, struct bio *bio)
{
	u32 lba;
	u16 pos, bpos, epos;
	void *src, *dst;
	struct wcb_lun_entity *entitys = wcb_1ctx->entitys;

	bpos = wcb_1ctx->start_pos;
	epos = wcb_1ctx->end_pos;
	for (pos = bpos; pos <= epos; pos++) {
		lba = entitys->lba[pos];
		dst = wcb_entity_offt_data(entitys->index, pos);
		src = bio_data(bio);

		if (lba < MAX_USER_LBA) {
			memcpy(dst, src, EXP_PPA_SIZE);			
			bio_advance(bio, EXP_PPA_SIZE);
		} else {
			// TODO::
			printk("sys data or bb\n");
		}
	}

	return;
}

void flush_data_to_wcb(struct nvm_exdev *exdev, 
						struct wcb_bio_ctx *wcb_resource, struct bio *bio)
{
	int i;
	u32 lba;
	u16 pos, bpos, epos;
	unsigned long flags;
	struct wcb_ctx *wcb_1ctx;
	struct wcb_lun_entity *entitys = NULL;

	for (i=0; i < MAX_USED_WCB_ENTITYS; i++) {
		void *src, *dst;
		wcb_1ctx = &wcb_resource->bio_wcb[i];
		entitys = wcb_1ctx->entitys;
		if (!entitys)
			break;
		
		bpos = wcb_1ctx->start_pos;
		epos = wcb_1ctx->end_pos;
		for (pos = bpos; pos <= epos; pos++) {
			lba = entitys->lba[pos];
			dst = wcb_entity_offt_data(entitys->index, pos);
			src = bio_data(bio);
		
			if (lba < MAX_USER_LBA) {
				memcpy(dst, src, EXP_PPA_SIZE); 		
				bio_advance(bio, EXP_PPA_SIZE);
			} else {
				//:: TODO update firstpage ftllog
				printk("sys data or bb\n");
			}

			// all travesal ppa need inc, regardless of pagetype
			if (atomic_inc_return(&entitys->fill_cnt) == RAID_LUN_SEC_NUM) {				
				debug_info("LUNs(blk:%d pg:%d lun:%d) push to full fifo\n", 
						   entitys->baddr.nand.blk, 
						   entitys->baddr.nand.pg, 
						   entitys->baddr.nand.lun);
				atomic_set(&entitys->fill_cnt, 0);
				spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
				push_lun_entity_to_fifo(&g_wcb_lun_ctl->full_lun, entitys);
				spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
				
				wake_up_process(exdev->writer_thread);
			}
		}
	}
}

void set_l2ptbl_incache(struct nvm_exdev *dev, u32 lba, u32 ppa)
{
	geo_ppa mapping;
	mapping.ppa = ppa;
	mapping.cache.in_cache = 1;
	
	if (lba < MAX_USER_LBA) {
		// TODO:: vpc
		dev->l2ptbl[lba] = mapping.ppa;
	} else {

	}

	return;
}

void set_l2ptbl_write_path(struct nvm_exdev *exdev, 
						   struct wcb_bio_ctx *wcb_resource)
{
	int i;
	u16 pos, bpos, epos;
	u32 lba, ppa;
	struct wcb_ctx *wcb_1ctx;
	struct wcb_lun_entity *entitys = NULL;

	for (i=0; i < MAX_USED_WCB_ENTITYS; i++) {
		wcb_1ctx = &wcb_resource->bio_wcb[i];
		entitys = wcb_1ctx->entitys;
		if (!entitys)
			break;

		bpos = wcb_1ctx->start_pos;
		epos = wcb_1ctx->end_pos;
		for (pos = bpos; pos <= epos; pos++) {
			lba = entitys->lba[pos];
			ppa = (u32)entitys->ppa[pos];
			
			if (lba < MAX_USER_LBA) {
				set_l2ptbl_incache(exdev, lba, ppa);
			} else {
				//:: TODO
				printk("lba out of bound sys data or bb\n");
			}
		}
	}
}

