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

// expose PPA namespace
struct nvm_exns {
	struct list_head list;	// linked in nvm_exdev->exns
	struct request_queue *queue;
	struct gendisk *disk;
	struct nvm_exdev *ndev;
	int instance;
};

void nvm_create_exns(struct nvm_exdev *exdev)
{
	struct nvm_exns *exns, *tmpns;
	int node = exdev->node;
	int nsid;
	//struct nvme_ctrl *ctrl = exdev->ctrl;

	exns = kzalloc_node(sizeof(*exns), GFP_KERNEL, node);
	if (!exns)
		return;

	//exns->instance  ctrl->instance
	nsid = idr_alloc(&exdev->nsid_idr, exns, 1, 0, GFP_KERNEL);
	if (nsid < 0)
		goto out_free_ns;

	printk("alloc nsid:%d\n", nsid);
	exns->instance = nsid;

	//sprintf(disk_name, "nvme%dexns%d", ctrl->instance, exns->instance);
	//memcpy(disk->disk_name, disk_name, DISK_NAME_LEN);

	tmpns = idr_find(&exdev->nsid_idr, nsid);
	if (tmpns != exns)
		printk("idr mapp error\n");
	else
		printk("idr mapp OK\n");

	list_add(&exns->list, &exdev->exns);

	return;

out_free_ns:
	kfree(exns);
	return;
}

void nvm_delete_exns(struct nvm_exdev *exdev)
{
	struct nvm_exns *ns, *tmp;

	mutex_lock(&exdev->nslist_mutex);
	list_for_each_entry_safe(ns, tmp, &exdev->exns, list) {
		list_del(&ns->list);
		idr_remove(&exdev->nsid_idr, ns->instance);
		kfree(ns);
	}
	mutex_unlock(&exdev->nslist_mutex);

}

