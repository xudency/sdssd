#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list_sort.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "fscftl.h"
#include "writecache/wcb-mngr.h"
#include "datapath/ppa-ops.h"

static const struct block_device_operations nvm_exns_fops = {
	.owner		= THIS_MODULE,
};

PPA_TYPE sys_get_ppa_type(geo_ppa ppa)
{
    // lookup BMI->bb

    // xor last ch

    // ep pl ln pg=0

    // 255 511 pg ep=3

    return USR_DATA;
}

// alloc len valid Normal PPA for this coming io
int alloc_wcb(sector_t slba, u32 nr_ppas, struct wcb_bio_ctx *wcb_resource)
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

void flush_data_to_wcb(struct wcb_bio_ctx *wcb_resource, struct bio *bio)
{
	int i;
	u32 lba;
	u16 pos, bpos, epos;
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
		}

		//bio_memcpy_wcb(wcb_1ctx, bio);
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

// TODO::
bool wcb_available(int nr_ppas)
{
	return true;
}

static blk_qc_t fscftl_make_rq(struct request_queue *q, struct bio *bio)
{
	struct nvm_exns *exns = q->queuedata;
	struct nvm_exdev *exdev = exns->ndev;
    unsigned long flags;
	struct wcb_bio_ctx wcb_resource;
    int nr_ppas = get_bio_nppa(bio);
    sector_t slba = get_bio_slba(bio);
    
    // TODO:: only consider write and read now
    if (bio_data_dir(bio) == READ) {
        /* Read datapath */
        bio_endio(bio);
        return BLK_QC_T_NONE;
    }

    /* Write datapath */
	memset(&wcb_resource, 0x00, sizeof(wcb_resource));

    spin_lock_irqsave(&g_wcb_lun_ctl->wcb_lock, flags);
	if (wcb_available(nr_ppas)) {
		alloc_wcb(slba, nr_ppas, &wcb_resource);
		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
	} else {
   		spin_unlock_irqrestore(&g_wcb_lun_ctl->wcb_lock, flags);
		printk("alloc_wcb fail resubmit this bio\n");
		//add_to_resubmit_list(bio);		
		bio_endio(bio);
		return BLK_QC_T_NONE;
	}

	{
		int i;
		u16 bpos, epos;
		struct wcb_ctx *wcb_1ctx;
		struct wcb_lun_entity *entitys = NULL;

		for (i=0; i < MAX_USED_WCB_ENTITYS; i++) 
		{
			wcb_1ctx = &wcb_resource.bio_wcb[i];
			entitys = wcb_1ctx->entitys;
			if (!entitys)
				break;

			bpos = wcb_1ctx->start_pos;
			epos = wcb_1ctx->end_pos;
			printk("bio nrppa:%d need lun entity%d [%d-%d]\n", 
					nr_ppas, entitys->index, bpos, epos);				
		}

		printk("===========================================\n");
	}

    flush_data_to_wcb(&wcb_resource, bio);

    spin_lock(&g_wcb_lun_ctl->l2ptbl_lock);
	set_l2ptbl_write_path(exdev, &wcb_resource);
    spin_unlock(&g_wcb_lun_ctl->l2ptbl_lock);
    
    bio_endio(bio);
	return BLK_QC_T_NONE;
}

int nvm_create_exns(struct nvm_exdev *exdev)
{
	int instance;
	sector_t capacity;
	struct gendisk *disk;	
	struct request_queue *rqueue;
	struct nvm_exns *exns;
	int node = exdev->node;

	exns = kzalloc_node(sizeof(*exns), GFP_KERNEL, node);
	if (!exns)
		return -ENOMEM;

	instance = idr_alloc(&exdev->nsid_idr, exns, 1, 0, GFP_KERNEL);
	if (instance < 0)
		goto out_free_ns;

	exns->instance = instance;
	exns->ndev = exdev;

	disk = alloc_disk(0);
	if (!disk)
		goto out_remove_idr;
	
	scnprintf(disk->disk_name, DISK_NAME_LEN, "%sexp%d", 
			  exdev->bdiskname, exns->instance);

	exns->disk = disk;
	
	rqueue = blk_alloc_queue_node(GFP_KERNEL, node);
	if (!rqueue)
		goto out_put_disk;
	blk_queue_make_request(rqueue, fscftl_make_rq);
	
	// set disk attribute	
	disk->flags = GENHD_FL_EXT_DEVT;
	disk->major = 0;
	disk->first_minor = 0;
	disk->fops = &nvm_exns_fops;
	disk->queue = rqueue;
	disk->private_data = exns;
	rqueue->queuedata = exns;
	exns->queue = rqueue;
	exdev->private_data = exns;
	
	// set requst_queue attribute
	rqueue->queue_flags = QUEUE_FLAG_DEFAULT;
	queue_flag_set_unlocked(QUEUE_FLAG_NOMERGES, rqueue);
	queue_flag_set_unlocked(QUEUE_FLAG_NONROT, rqueue);
	blk_queue_logical_block_size(rqueue, CFG_NAND_EP_NUM);
	blk_queue_max_hw_sectors(rqueue, 8 * MAX_PPA_PER_CMD); // 512 Unit
	blk_queue_write_cache(rqueue, true, false);

    //capacity = le64_to_cpup(&id->nsze) << (ns->lba_shift - 9);
    capacity = (MAX_USER_LBA + 1) * 8;
	set_capacity(disk, capacity);	// 512 Unit
	printk("create disk:/dev/%s    User Capacity:%ldGB\n", 
			disk->disk_name, (capacity*512 >> 30));
	add_disk(disk);
	
	return 0;

out_put_disk:
	put_disk(disk);
out_remove_idr:
	idr_remove(&exdev->nsid_idr, exns->instance);
out_free_ns:
	kfree(exns);
	return -EFAULT;
}

void nvm_delete_exns(struct nvm_exdev *exdev)
{
	struct nvm_exns *ns = (struct nvm_exns *)exdev->private_data;

	del_gendisk(ns->disk);
	blk_cleanup_queue(ns->queue);
	put_disk(ns->disk);
	idr_remove(&exdev->nsid_idr, ns->instance);
	kfree(ns);
}

struct nvm_exns *find_nvm_exns(struct nvm_exdev *exdev, int instance)
{
	return idr_find(&exdev->nsid_idr, instance);
}

/*
void for_each_exns(struct nvm_exdev *exdev)
{
	u32 i;
	struct nvm_exns *ns;

	idr_for_each_entry(&exdev->nsid_idr, ns, i) {
		printk("exns->instance:%d\n", ns->instance);
	}
}*/
