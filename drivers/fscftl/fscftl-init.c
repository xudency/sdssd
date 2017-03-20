/*
 * Copyright (C) 2017 Group XX
 * Initial release: Dengcai Xu <dxu@cnexlabs.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * fscftl initialization.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

#include "hwcfg/cfg/flash_cfg.h"
#include "hwcfg/regrw.h"
#include "../nvme/host/nvme.h"
#include "build_recovery/power.h"
#include "fscftl.h"
#include "../../include/linux/lightnvm.h"

static bool mcp = false;
module_param(mcp, bool, 0644);

static char exdev_name[DISK_NAME_LEN] = "nvme0n1";
module_param_string(bdev, exdev_name, 8, 0);   //basedev name

static int nvm_submit_ppa(struct nvm_exns *exns, struct nvm_rq *rqd);

// inherit interface from Lightnvm Subsystem and pblk
static struct nvme_ppa_ops exdev_ppa_ops = {
	.name			 = "exppassd",
	.module			 = THIS_MODULE,
	.submit_io       = nvm_submit_ppa,
};

// this pool use for prplist ppalist
static int nvm_exdev_setup_pool(struct nvm_exdev *dev, char *name)
{
	struct device *dmadev = &dev->pdev->dev;

	dev->dmapoll = dma_pool_create(name, dmadev, PAGE_SIZE, PAGE_SIZE, 0);
	if(!dev->dmapoll)
		return -ENOMEM;
	else
		return 0;
}

static void nvm_exdev_release_pool(struct nvm_exdev *dev)
{
	dma_pool_destroy(dev->dmapoll);
}

void *nvm_exdev_dma_pool_alloc(struct nvm_exdev *dev, dma_addr_t *dma_handle)
{
	return dma_pool_alloc(dev->dmapoll, GFP_KERNEL, dma_handle);
}

void nvm_exdev_dma_pool_free(struct nvm_exdev *dev, void *vaddr, dma_addr_t dma_handle)
{
	dma_pool_free(dev->dmapoll, vaddr, dma_handle);
}

int set_rqd_nand_ppalist(struct nvm_exdev *dev, struct nvm_rq *rqd, 
						 struct ppa_addr *ppas, int nr_ppas)
{
	int i;

	rqd->nr_ppas = nr_ppas;

	if (nr_ppas == 1) {
		rqd->ppa_addr = ppas[0];
		return 0;
	}
	
	rqd->ppa_list = nvm_exdev_dma_pool_alloc(dev, &rqd->dma_ppa_list);
	if (!rqd->ppa_list) {
		pr_err("nvm: failed to allocate dma memory\n");
		return -ENOMEM;
	} else {
		for (i = 0; i < nr_ppas; i++)
			rqd->ppa_list[i] = ppas[i];
	}

	return 0;
}

void free_rqd_nand_ppalist(struct nvm_exdev * dev, struct nvm_rq *rqd)
{
	if (!rqd->ppa_list)   //nr_ppa
		return;

	nvm_exdev_dma_pool_free(dev, rqd->ppa_list, rqd->dma_ppa_list);
}

//example How to use submit_bio to submit PPA command
// 1. rqd
// 2. bio
// 3. set fn(ctx)
//need form a struct nvm_rq rqd and a bio
static void nvm_end_io_sync(struct nvm_rq *rqd)
{
	struct completion *waiting = rqd->private;

	complete(waiting);
}

int nvm_ersppa_sync(struct nvm_exdev *dev, struct ppa_addr *ppas, int nr_ppas)
{
	int ret = 0;
	struct nvm_rq rqd;
	struct nvm_exns *exns = dev->private_data;

	DECLARE_COMPLETION_ONSTACK(wait);

	memset(&rqd, 0, sizeof(struct nvm_rq));

    // fullfill rqd.xx
    rqd.opcode = NVM_OP_ERASE;
	rqd.flags = NVME_PLANE_SNGL;
	rqd.end_io = nvm_end_io_sync;
	rqd.private = &wait;

	ret = set_rqd_nand_ppalist(dev, &rqd, ppas, nr_ppas);
	if (ret)
		return ret;
	
    ret = dev->ops->submit_io(exns, &rqd);  //nvm_submit_ppa
	if (ret) {
		pr_err("erase I/O submission falied: %d\n", ret);
		goto free_ppa_list;
	}

	wait_for_completion_io(&wait);

free_ppa_list:
	free_rqd_nand_ppalist(dev, &rqd);
    return ret;
}

void erspps_smoke_test(struct nvm_exdev *dev)
{
	struct ppa_addr nandppa[2];

	nandppa[0].ppa = 0x3;	
	nandppa[0].ppa = 0x4;
	
	nvm_ersppa_sync(dev, &nandppa[0], 1);
	nvm_ersppa_sync(dev, &nandppa[1], 1);
}

static inline void nvm_rqd_to_ppacmd(struct nvm_rq *rqd, int instance, 
									 struct nvme_ppa_command *c)
{
	c->opcode = rqd->opcode;
	c->nsid = cpu_to_le32(instance);
	c->ppalist = cpu_to_le64(rqd->ppa_addr.ppa);
	c->metadata = cpu_to_le64(rqd->dma_meta_list);
	c->control = cpu_to_le16(rqd->flags);
	c->nlb = cpu_to_le16(rqd->nr_ppas - 1);

	/* prp1 prp2 is in bio */
}

static void print_cqe_result(struct nvm_rq *rqd)
{
	printk("New CQE complete\n");

	switch (rqd->opcode) {
	case NVM_OP_ESPPA:
		printk("  ersppa");
		break;
	case NVM_OP_WRPPA:
		printk("  wrppa");
		break;
	case NVM_OP_RDPPA:
		printk("  rdppa");
		break;
	case NVM_OP_WRRAW:
		printk("  wrpparaw");
		break;
	case NVM_OP_RDRAW:
		printk("  rdpparaw");
		break;
	}

	printk("  nr_ppas:%d  control:%x\n", rqd->nr_ppas, rqd->flags);
	printk("  status:0x%x result.u64:0x%llx\n", rqd->error, rqd->ppa_status);
	printk("\n");
}

static void nvm_ppa_end_io(struct request *rq, int error)
{
	struct nvm_rq *rqd = rq->end_io_data;

	rqd->ppa_status = nvme_req(rq)->result.u64;
	rqd->error = error;

	print_cqe_result(rqd);

	//callback fn(ctx)
	if (rqd->end_io)
		rqd->end_io(rqd);

	kfree(nvme_req(rq)->cmd);
	blk_mq_free_request(rq);
}

static int nvm_submit_ppa(struct nvm_exns *exns, struct nvm_rq *rqd)
{
	struct nvme_ns *bns = exns->ndev->bns;
	struct request_queue *bq = bns->queue;
	struct request *rq;
	struct bio *bio = rqd->bio;
	struct nvme_ppa_command *cmd;

	cmd = kzalloc(sizeof(struct nvme_ppa_command), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	rq = nvme_alloc_request(bq, (struct nvme_command *)cmd, 0, NVME_QID_ANY);
	if (IS_ERR(rq)) {
		kfree(cmd);
		return -ENOMEM;
	}
	rq->cmd_flags &= ~REQ_FAILFAST_DRIVER;

	if (bio) {
		rq->ioprio = bio_prio(bio);
		rq->__data_len = bio->bi_iter.bi_size;
		rq->bio = rq->biotail = bio;
		if (bio_has_data(bio))
			rq->nr_phys_segments = bio_phys_segments(bq, bio);
	} else {
		// erase
		rq->ioprio = IOPRIO_PRIO_VALUE(IOPRIO_CLASS_BE, IOPRIO_NORM);
		rq->__data_len = 0;
	}

	nvm_rqd_to_ppacmd(rqd, bns->instance, cmd);

	rq->end_io_data = rqd;

	blk_execute_rq_nowait(bq, NULL, rq, 0, nvm_ppa_end_io);

	return 0;
}

////////////////////////////////////////////////////////////////////////////
int fscftl_setup(void)
{
    return 0;
}

void fscftl_cleanup(void)
{
    return;
}

static int __init fscftl_module_init(void)
{
    int ret = 0;
    struct nvm_exdev *exdev;

	printk("NandFlash type:%s\n", FLASH_TYPE);

	exdev = nvm_find_exdev(exdev_name);
    if (exdev == NULL)
        return -ENODEV;

	printk("find exdev:%s  magic_dw:0x%x\n", exdev->bdiskname, exdev->magic_dw);

    exdev->ops = &exdev_ppa_ops;

    ctrl_reg_setup(exdev->ctrl);

	ret = nvm_exdev_setup_pool(exdev, "prp-ppa-list");
	if (ret) 
		return ret;

    ret = fscftl_setup();
    if (ret)
        goto release_pool;

    if (mcp) {
        if (do_manufactory_init(exdev))
            goto err_cleanup;
    } else {
        if (try_recovery_systbl())
            goto err_cleanup;
    }

	ret = nvm_create_exns(exdev);
    if (ret)
        goto err_cleanup;

	/*test*/
	erspps_smoke_test(exdev);

	return 0;

err_cleanup:
    fscftl_cleanup();
release_pool:
	nvm_exdev_release_pool(exdev);
    return -EFAULT;
}

static void __exit fscftl_module_exit(void)
{
	struct nvm_exdev *exdev = nvm_find_exdev(exdev_name);

	nvm_delete_exns(exdev);
    flush_down_systbl();
    fscftl_cleanup();
	nvm_exdev_release_pool(exdev);
    
	return;
}

module_init(fscftl_module_init);
module_exit(fscftl_module_exit);
MODULE_AUTHOR("Dengcai Xu <dxu@cnexlabs.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Host-Based Full Stack Control FTL for NVMe SSDs");

