#include <linux/kthread.h>
#include <linux/delay.h>
#include "ppa-ops.h"
#include "bio-datapath.h"
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
 * and check if It can Submit Now(LUN BUSY?)
 * flush data from wcb to NandFlash
 * return 1: continue to pull FUll LUN,  0: no full fifo thread schedule out
 */
int submit_write_backend(struct nvm_exdev *exdev)
{
	unsigned long flags;
	struct fsc_fifo *full_lun =  &g_wcb_lun_ctl->full_lun;
    struct fsc_fifo *ongoing_lun =  &g_wcb_lun_ctl->ongoing_lun;	
	struct wcb_lun_entity *entity;

	spin_lock_irqsave(&g_wcb_lun_ctl->fifo_lock, flags);
	
	entity = pull_lun_entity_from_fifo(full_lun);
	if(entity == NULL) {
		// full fifo is empty
		spin_unlock_irqrestore(&g_wcb_lun_ctl->fifo_lock, flags);
		return 0;
	}

	printk("Fscftl-Writer find a entity%d in full fifo\n", entity->index);

	printk("write this LUNs(blk:%d pg:%d lun:%d) to Nand\n", 
            entity->baddr.nand.blk, 
            entity->baddr.nand.pg, 
            entity->baddr.nand.lun);
    
    push_lun_entity_to_fifo(ongoing_lun, entity);

    /* Simulation HW CQE back, 10ms */
    mod_timer(&exdev->cqe_timer, jiffies + msecs_to_jiffies(10));
		
	spin_unlock_irqrestore(&g_wcb_lun_ctl->fifo_lock, flags);

	return 1;
}

void simulate_cqe_back_fn(unsigned long data)
{
    unsigned long flags;
	struct nvm_exdev *exdev = (struct nvm_exdev *)data;
    struct fsc_fifo *empty_lun =  &g_wcb_lun_ctl->empty_lun;
    struct fsc_fifo *ongoing_lun =  &g_wcb_lun_ctl->ongoing_lun;	
	struct wcb_lun_entity *entity;

	spin_lock_irqsave(&g_wcb_lun_ctl->fifo_lock, flags);

	entity = pull_lun_entity_from_fifo(ongoing_lun);
    if (entity == NULL) {
	    spin_unlock_irqrestore(&g_wcb_lun_ctl->fifo_lock, flags);
        return;
    }
    
    push_lun_entity_to_fifo(empty_lun, entity);
	spin_unlock_irqrestore(&g_wcb_lun_ctl->fifo_lock, flags);

    mod_timer(&exdev->cqe_timer, jiffies + msecs_to_jiffies(100));
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

int fscftl_writer_init(struct nvm_exdev *exdev)
{
	exdev->writer_thread = kthread_create(fscftl_write_kthread, exdev, "fscftl-writer");

    setup_timer(&exdev->cqe_timer, simulate_cqe_back_fn, (unsigned long)exdev)

	wake_up_process(exdev->writer_thread);

	return 0;
}

void fscftl_writer_exit(struct nvm_exdev *exdev)
{
	if (exdev->writer_thread)
		kthread_stop(exdev->writer_thread);
    del_timer(&exdev->cqe_timer);
}

PPA_TYPE sys_get_ppa_type(geo_ppa ppa)
{
    // lookup BMI->bb

    // xor last ch

    // ep pl ln pg=0

    // 255 511 pg ep=3

    return USR_DATA;
}

// TODO::
bool wcb_available(int nr_ppas)
{
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
                cur_lun_entity = get_new_lun_entity(curppa);
                if (!cur_lun_entity) {
                    // TODO:: restore context
                    printk("alloc write cache failed\n");
                    return -1;
                }

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

    //set_current_ppa(curppa);

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
			//:: TODO
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
				printk("push entity%d to full fifo\n", entitys->index);
				atomic_set(&entitys->fill_cnt, 0);
				spin_lock_irqsave(&g_wcb_lun_ctl->fifo_lock, flags);
				push_lun_entity_to_fifo(&g_wcb_lun_ctl->full_lun, entitys);
				spin_unlock_irqrestore(&g_wcb_lun_ctl->fifo_lock, flags);
				
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

