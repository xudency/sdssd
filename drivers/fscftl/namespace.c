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

static const struct block_device_operations nvm_exns_fops = {
	.owner		= THIS_MODULE,
};

static blk_qc_t fscftl_make_rq(struct request_queue *q, struct bio *bio)
{
	//struct nvm_exns *exns = q->queuedata;

	// do nothing
	bio_endio(bio);

	return BLK_QC_T_NONE;
}

//nvme_dev;
//nvme_ctrl;
//nvm_exdev;
//nvm_exns;
int nvm_create_exns(struct nvm_exdev *exdev)
{
	int result, nsid;
	sector_t capacity;
	struct gendisk *disk;	
	struct request_queue *rqueue;
	struct nvm_exns *exns;
	int node = exdev->node;
	struct nvme_ctrl *ctrl = exdev->ctrl;

	exns = kzalloc_node(sizeof(*exns), GFP_KERNEL, node);
	if (!exns)
		return -ENOMEM;

	nsid = idr_alloc(&exdev->nsid_idr, exns, 1, 0, GFP_KERNEL);
	if (nsid < 0)
		goto out_free_ns;

	exns->instance = nsid;
	exns->ndev = exdev;

	disk = alloc_disk(0);
	if (!disk)
		goto out_remove_idr;
	
	scnprintf(disk->disk_name, DISK_NAME_LEN, "nvme%dexns%d", 
			  ctrl->instance, exns->instance);

	exns->disk = disk;
	
	rqueue = blk_alloc_queue_node(GFP_KERNEL, node);
	if (!rqueue)
		goto out_put_disk;
	
	blk_queue_make_request(rqueue, fscftl_make_rq);

	// FTL poweron flow
	//tt->init();

	// set disk attribute	
	disk->flags = GENHD_FL_EXT_DEVT;
	disk->major = 0;
	disk->first_minor = 0;
	disk->fops = &nvm_exns_fops;
	disk->queue = rqueue;
	disk->private_data = exns;
	rqueue->queuedata = exns;
	exns->queue = rqueue;
	
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
	printk("create disk: /dev/%s  capacity:0x%lx(LBA)  %dGB\n", 
			disk->disk_name, capacity/8, (capacity*512 >> 30));

	/*tmpns = idr_find(&exdev->nsid_idr, nsid);
	if (tmpns != exns)
		printk("idr mapp error\n");
	else
		printk("idr mapp OK\n");*/

	add_disk(disk);

	mutex_lock(&exdev->nslist_mutex);
	list_add(&exns->list, &exdev->exns);
	mutex_unlock(&exdev->nslist_mutex);

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
	struct nvm_exns *ns, *tmp;

	mutex_lock(&exdev->nslist_mutex);
	list_for_each_entry_safe(ns, tmp, &exdev->exns, list) {
		list_del(&ns->list);
		del_gendisk(ns->disk);
		blk_cleanup_queue(ns->queue);
		put_disk(ns->disk);
		idr_remove(&exdev->nsid_idr, ns->instance);
		kfree(ns);
	}
	mutex_unlock(&exdev->nslist_mutex);
}

