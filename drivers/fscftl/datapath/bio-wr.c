#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include "ppa-ops.h"
#include "bio-datapath.h"
#include "../fscftl.h"
#include "../utils/utils.h"
#include "../writecache/wcb-mngr.h"

static struct workqueue_struct *requeue_bios_wq;
struct task_struct *bios_rewr_thread;
struct completion lun_completion;


void print_lun_entity_baddr(char *bs, struct wcb_lun_entity *entity, char *as)
{
	debug_info("%s LUN(blk:%d pg:%d lun:%d) %s\n", bs, 
		    entity->baddr.nand.blk, 
		    entity->baddr.nand.pg, 
		    entity->baddr.nand.lun, as);
}

// Simulation HW IRQ callback fn Timer
// dead code
/*void simulate_cqe_back_fn(unsigned long data)
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

		queue_work(requeue_bios_wq, &exdev->requeue_ws);
	}
}*/

// Simulation HW IRQ callback fn Workqueue
static void fscftl_yirq_workfn(struct work_struct *work)
{
	unsigned long flags;
	struct nvm_exdev *exdev = container_of(work, struct nvm_exdev, yirq);
	struct fsc_fifo *empty_lun =  &g_wcb_lun_ctl->empty_lun;
	struct fsc_fifo *ongoing_lun =	&g_wcb_lun_ctl->ongoing_lun;	
	struct wcb_lun_entity *entity;

	while (1) {
		spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);

		entity = pull_lun_entity_from_fifo(ongoing_lun);
		if (entity == NULL) {
			spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
			break;
		}
		
		push_lun_entity_to_fifo(empty_lun, entity);

		print_lun_entity_baddr("push", entity, "to emptyfifo");
		printk("\n");

		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

		queue_work(requeue_bios_wq, &exdev->requeue_ws);
	}
}

// NOTICE: this routine us in IRQ context, 
// wcb lun entity one line(ch) complete
static void wrppa_lun_completion(struct request *req, int error)
{
	unsigned long flags;
	struct nvme_ppa_iod *ppa_iod = req->end_io_data;
	struct wcb_lun_entity *entity = ppa_iod->ctx;
	int idx = ppa_iod->idx;
	int status = error;			/* No phase tag */
	u64 result = nvme_req(req)->result.u64; /* 64bit completion btmap */
	struct nvme_command *cmd = nvme_req(req)->cmd;	/* original sqe */

	if (status != 0) {
		// TODO:: program fail Handle
		debug_info("result:0x%llx  status:0x%x\n", result, status);
	}

	if (BIT_TEST(entity->cqe_flag, idx)) {
		printk("ERR: lun entity line%d complete twice\n", idx);
	} else {		
		debug_info("lun entity line%d complete\n", idx);
		BIT_SET(entity->cqe_flag, idx);
	}
	
	if (entity->cqe_flag == (u16)BIT2MASK(CFG_DRIVE_LINE_NUM)) {
		spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
		push_lun_entity_to_fifo(&g_wcb_lun_ctl->empty_lun, entity);
		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

		atomic_dec(&g_wcb_lun_ctl->outstanding_lun);

		print_lun_entity_baddr("this entity cqe back, push", 
			   		entity, "to emptyfifo");
		
		queue_work(requeue_bios_wq, &ppa_iod->dev->requeue_ws);

		if (!bio_list_empty(&g_wcb_lun_ctl->requeue_wr_bios)) {
			complete(&lun_completion);
		}
	}
	
	kfree(cmd);
	blk_mq_free_request(req);
}

// full LUNs submit nvme command to HW, by CH+
void nvme_wrppa_lun(struct nvm_exdev *exdev, struct wcb_lun_entity *entity)
{
	int line;
	void *databuf;
	dma_addr_t meta_dma, ppa_dma;
	int nr_ppas = CFG_NAND_CHANNEL_NUM;
	u16 ctrl = NVM_IO_DUAL_ACCESS | NVM_IO_SCRAMBLE_ENABLE;
	struct nvme_ppa_iod *ppa_iod;
	struct nvme_ppa_command *ppa_cmd;

	print_lun_entity_baddr("submit wrppa of", entity, "to Nand");

	atomic_inc(&g_wcb_lun_ctl->outstanding_lun);

	printk("LUN[blk:%d pg:%d lun:%d] wrppa to Nand\n", 
		entity->baddr.nand.blk, 
		entity->baddr.nand.pg, 
		entity->baddr.nand.lun);

	printk("\n");
	/*for (int i = 0; i < RAID_LUN_SEC_NUM; i++) {
		debug_info("ppa[%d]=0x%llx\n", i, entity->ppa[i]);
	}*/
	
	entity->cqe_flag = 0;
	for (line = 0; line < CFG_DRIVE_LINE_NUM; line++) {
		ppa_cmd = kzalloc(sizeof(struct nvme_ppa_command) + \
				  sizeof(struct nvme_ppa_iod), GFP_KERNEL);

		ppa_iod = (struct nvme_ppa_iod *)((uintptr_t)ppa_cmd + \
					sizeof(struct nvme_ppa_command));

		ppa_iod->dev = exdev;
		ppa_iod->ctx = entity;
		ppa_iod->idx = line;
		ppa_iod->vaddr_ppalist = entity->ppa + line*CFG_NAND_CHANNEL_NUM;
		
		ppa_cmd->opcode = NVM_OP_WRPPA;
		ppa_cmd->nsid = exdev->bns->ns_id;
		ppa_cmd->nlb = cpu_to_le16(nr_ppas - 1);
		ppa_cmd->control = cpu_to_le16(ctrl);

		meta_dma = entity->meta_dma + line*CFG_NAND_CHANNEL_NUM*NAND_META_SIZE;
		ppa_cmd->metadata = cpu_to_le64(meta_dma);

		ppa_dma = entity->ppa_dma + line*CFG_NAND_CHANNEL_NUM*sizeof(u64);
		ppa_cmd->ppalist = ppa_dma;

		databuf = entity->data+ line*CFG_NAND_CHANNEL_NUM*EXP_PPA_SIZE;
		//databuf = g_vdata + line*CFG_NAND_CHANNEL_NUM*EXP_PPA_SIZE;

		nvme_submit_ppa_cmd(exdev, ppa_cmd, databuf, 
				    nr_ppas * EXP_PPA_SIZE, 
				    wrppa_lun_completion, ppa_iod);
		
		debug_info("submit line%d\n", line);
	}	
}

/* Write datapath backend, pull a LUN entity from fullfifo, 
 * and check if It can Submit Now(LUN BUSY?) NO NEED
 * flush data from wcb to NandFlash
 * return 1: continue to pull FUll LUN,  0: no full fifo thread schedule out
 */
int submit_write_backend(struct nvm_exdev *exdev)
{
	unsigned long flags;
	struct wcb_lun_entity *entity;

	spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);

	entity = pull_lun_entity_from_fifo(&g_wcb_lun_ctl->full_lun);
	if(entity == NULL) {
		// full fifo is empty
		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
		return 0;
	}
	
	print_lun_entity_baddr("Fscftl-writer pull", entity, "from fullfifo");
	
#if 1	// Really NVMe Command
	spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
	nvme_wrppa_lun(exdev, entity);

#else	// Simulation
	push_lun_entity_to_fifo(&g_wcb_lun_ctl->ongoing_lun, entity);

	spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

	queue_work(requeue_bios_wq, &exdev->yirq);
#endif	
	return 1;
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

int process_write_bio(struct request_queue *q, struct bio *bio)
{
	unsigned long flags;	
	struct nvm_exns *exns = q->queuedata;
	struct nvm_exdev *exdev = exns->ndev;
	struct wcb_bio_ctx wcb_resource;
	int nr_ppas = get_bio_nppa(bio);
	sector_t slba = get_bio_slba(bio);

	memset(&wcb_resource, 0x00, sizeof(wcb_resource));

	spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
	
	if (wcb_available(nr_ppas)) {
		alloc_wcb_core(slba, nr_ppas, &wcb_resource);
		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
	} else {
		print_lun_entitys_fifo();
	
		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);

		printk("wcb_unavailable outstanding Lun:%d\n", 
			atomic_read(&g_wcb_lun_ctl->outstanding_lun));

		return FSCFTL_BIO_RESUBMIT;
	}

	flush_data_to_wcb(exdev, &wcb_resource, bio);

	spin_lock(&g_wcb_lun_ctl->l2ptbl_lock);
	set_l2ptbl_write_path(exdev, &wcb_resource);
	spin_unlock(&g_wcb_lun_ctl->l2ptbl_lock);

	return FSCFTL_BIO_COMPLETE;
}

void resubmit_wr_bios_list(struct nvm_exdev *exdev)
{
	struct nvm_exns *ns = (struct nvm_exns *)exdev->private_data;
	struct bio *bio;
	int ret;

	do {
		spin_lock(&g_wcb_lun_ctl->biolist_lock);
		if (!bio_list_empty(&g_wcb_lun_ctl->requeue_wr_bios)) {
			bio = bio_list_pop(&g_wcb_lun_ctl->requeue_wr_bios);
			spin_unlock(&g_wcb_lun_ctl->biolist_lock);
		
			if (!bio)
				return;
			
			ret = process_write_bio(ns->queue, bio);
			if (ret == FSCFTL_BIO_COMPLETE) {
				bio_endio(bio);
			} else {
				spin_lock(&g_wcb_lun_ctl->biolist_lock);
				bio_list_add_head(&g_wcb_lun_ctl->requeue_wr_bios, bio);
				spin_unlock(&g_wcb_lun_ctl->biolist_lock);
				return;
			}
	
		} else {
			spin_unlock(&g_wcb_lun_ctl->biolist_lock);
			return;
		}
		
	} while(1);
}

static void fscftl_requeue_workfn(struct work_struct *work)
{
	struct nvm_exdev *exdev = container_of(work, struct nvm_exdev, requeue_ws);

	resubmit_wr_bios_list(exdev);
}

// process the pending write bios
int fscftl_rewr_kthread(void *data)
{
	struct nvm_exdev *exdev = data;

	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		//reinit_completion(&lun_completion);
		wait_for_completion(&lun_completion);

		resubmit_wr_bios_list(exdev);
	}

	return 0;
}

int fscftl_writer_init(struct nvm_exdev *exdev)
{
	requeue_bios_wq = alloc_workqueue("bios-dequeue", WQ_UNBOUND, 0);

	init_completion(&lun_completion);

	bios_rewr_thread = kthread_create(fscftl_rewr_kthread, exdev, "fscftl-rewr");
	wake_up_process(bios_rewr_thread);

	exdev->writer_thread = kthread_create(fscftl_write_kthread, exdev, "fscftl-writer");
	wake_up_process(exdev->writer_thread);

	//setup_timer(&exdev->cqe_timer, simulate_cqe_back_fn, (unsigned long)exdev);

	INIT_WORK(&exdev->requeue_ws, fscftl_requeue_workfn);	
	INIT_WORK(&exdev->yirq, fscftl_yirq_workfn);

	return 0;
}

void fscftl_writer_exit(struct nvm_exdev *exdev)
{
	//del_timer(&exdev->cqe_timer);
	if (exdev->writer_thread)
		kthread_stop(exdev->writer_thread);
	if (bios_rewr_thread) {
		complete(&lun_completion);
		kthread_stop(bios_rewr_thread);
	}

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

	if ((nr_ppas+extra) >= left && (emptysize == 0))
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
	
	curppa.ppa = cur_lun_entity->baddr.ppa + cur_lun_entity->pos;

	while (loop) 
	{
		switch (sys_get_ppa_type(curppa)) {
		case USR_DATA:
			left_len--;
			cur_lun_entity->lba[cur_lun_entity->pos] = slba++;
			cur_lun_entity->ppa[cur_lun_entity->pos] = curppa.ppa;
			wcb_resource->bio_wcb[num].end_pos = cur_lun_entity->pos;
			cur_lun_entity->pos++;

			if (cur_lun_entity->pos == RAID_LUN_SEC_NUM) {	
				cur_lun_entity = get_next_lun_entity(curppa);
				if (!cur_lun_entity) // Never
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
                                curppa.ppa++;
			}

			if (left_len == 0) {
				/* wcb is prepare ready */
				loop = 0;
			}
			break;

		case DUMMY_DATA:
			break;

		case BAD_BLK:
			break;

		default:
			break;
		}
	}

	/*{
	int i;
	u16 bpos, epos;
	struct wcb_ctx *wcb_1ctx;
	struct wcb_lun_entity *entitys = NULL;

	for (i=0; i < MAX_USED_WCB_ENTITYS; i++) 
	{
        	wcb_1ctx = &wcb_resource->bio_wcb[i];
        	entitys = wcb_1ctx->entitys;
        	if (!entitys)
        	        break;

        	bpos = wcb_1ctx->start_pos;
        	epos = wcb_1ctx->end_pos;
        	printk("bio nrppa:%d need entity [blk%d pg%d lun%d] pos[%d-%d]\n", 
        	        nr_ppas, entitys->baddr.nand.blk, entitys->baddr.nand.pg, 
        	        entitys->baddr.nand.lun, bpos, epos);				
	}

	printk("===========================================\n");
	}*/

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
		
			if (lba <= MAX_USER_LBA) {
				memcpy(dst, src, EXP_PPA_SIZE); 		
				bio_advance(bio, EXP_PPA_SIZE);
			} else {
				//:: TODO update firstpage ftllog
				printk("sys data or bb\n");
			}

			// all travesal ppa need inc, regardless of pagetype
			if (atomic_inc_return(&entitys->fill_cnt) == RAID_LUN_SEC_NUM) {				
				print_lun_entity_baddr("this lun is full push", 
							entitys, 
							"to fullfifo");
				
				atomic_set(&entitys->fill_cnt, 0);
				spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
				push_lun_entity_to_fifo(&g_wcb_lun_ctl->full_lun, entitys);
				spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
				
				wake_up_process(exdev->writer_thread);
			}
		}
	}
}

inline geo_ppa get_l2ptbl_mapping(struct nvm_exdev *dev, u32 lba)
{
	geo_ppa ppa;
	ppa.ppa = dev->l2ptbl[lba];
	//ppa.cache.in_cache = 0;

	return ppa;
}

void set_l2ptbl_incache(struct nvm_exdev *dev, u32 lba, u32 ppa)
{
	geo_ppa prev, mapping;
	mapping.ppa = ppa;
	mapping.cache.in_cache = 1;
	
	if (lba <= MAX_USER_LBA) {
		prev = get_l2ptbl_mapping(dev, lba);
		
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
			set_l2ptbl_incache(exdev, lba, ppa);
		}
	}
}

