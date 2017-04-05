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
#include "../nvme/host/nvme.h"
#include "hwcfg/cfg/flash_cfg.h"
#include "hwcfg/regrw.h"
#include "build_recovery/power.h"
#include "fscftl.h"
#include "datapath/ppa-ops.h"
#include "writecache/wcb-mngr.h"
#include "systbl/sys-meta.h"
#include "datapath/bio-datapath.h"
#include "utils/utils.h"
#include <linux/nvme_ioctl.h>

static bool mcp = true;
module_param(mcp, bool, 0644);

static char exdev_name[DISK_NAME_LEN] = "nvme0n1";
module_param_string(bdev, exdev_name, 8, 0);   //basedev name

extern struct nvme_ppa_ops exdev_ppa_ops;

#define MDULE_TEST 0

int fscftl_setup(struct nvm_exdev *exdev)
{
	int ret = 0;

	ret = statetbl_init();
	if (ret)
		return ret;

	ret = bootblk_page_init();
	if (ret)
		goto out_free_statetbl;

	ret = bmitbl_init();
	if (ret)
		goto out_free_bootpage;

        ret = vpctbl_init();
        if (ret)
                goto out_free_bmitbl;

	ret = l2ptbl_init(exdev);
	if (ret)
		goto out_free_vpctbl;

	ret = write_cache_alloc(exdev);
	if (ret)
		goto out_free_l2p;

	ret = fscftl_writer_init(exdev);
	if (ret)
		goto out_free_wcb;

    return ret;
	
out_free_wcb:
	write_cache_free(exdev);
out_free_l2p:
	l2ptbl_exit(exdev);
out_free_vpctbl:
        vpctbl_exit();
out_free_bmitbl:
	bmitbl_exit();
out_free_bootpage:
	bootblk_page_exit();
out_free_statetbl:
	statetbl_exit();
	return ret;
}

void fscftl_cleanup(struct nvm_exdev *exdev)
{
	fscftl_writer_exit(exdev);
	write_cache_free(exdev);
	l2ptbl_exit(exdev);
        vpctbl_exit();
	bmitbl_exit();	
	bootblk_page_exit();
	statetbl_exit();

	return;
}

// prepare lun_entity && pos && free_list, etc.
void prepare_write_context(void)
{
	int blk;
	geo_ppa startppa;

	startppa.ppa = 0;
	blk = get_blk_from_free_list();
	startppa.nand.blk = blk;

	g_wcb_lun_ctl->partial_entity = get_lun_entity(startppa);

	print_lun_entitys_fifo();
}

///////////////////BackEnd Ioctl sync////////////////////////////////////////
#define FSCFTL_IOCTL_USER_PPA_CMD	_IOW('M', 0x90, struct nvme_user_io)

// nvme-cli extend
// nvme-cli fscftl passthru
static void user_passthru_cmd_completion(struct request *rq, int error)
{
	struct completion *waiting = rq->end_io_data;

	complete(waiting);
}

static int submit_user_passthru_cmd(struct nvm_exdev *dev,
				struct nvme_ppa_command *cmd,
				void __user *ubuf, unsigned int bufflen,
				void __user *meta_buf, unsigned int meta_len,
				void __user *ppa_buf, unsigned int nlb,
				unsigned int timeout)
{
	bool write = nvme_is_write((struct nvme_command *)cmd);
        struct request_queue *q = dev->bns->queue;
	struct gendisk *disk = dev->bns->disk;
	struct request *rq;
	struct bio *bio = NULL;
	__le64 *ppa_list = NULL;
	dma_addr_t ppa_dma;
	__le64 *metadata = NULL;
	dma_addr_t metadata_dma;
	DECLARE_COMPLETION_ONSTACK(wait);
	int ret;

	rq = nvme_alloc_request(q, (struct nvme_command *)cmd, 0, NVME_QID_ANY);
	if (IS_ERR(rq)) {
		ret = -ENOMEM;
		goto err_cmd;
	}

	rq->timeout = timeout ? timeout : ADMIN_TIMEOUT;

	rq->cmd_flags &= ~REQ_FAILFAST_DRIVER;
	rq->end_io_data = &wait;

	if (ppa_buf && nlb) {
                ppa_list = dma_pool_page_zalloc(dev, &ppa_dma);
		if (!ppa_list) {
			ret = -ENOMEM;
			goto err_rq;
		}
		if (copy_from_user(ppa_list, (void __user *)ppa_buf,
						sizeof(u64) * (nlb + 1))) {
			ret = -EFAULT;
			goto err_ppa;
		}
		cmd->ppalist = cpu_to_le64(ppa_dma);
	} else {
		cmd->ppalist = cpu_to_le64((uintptr_t)ppa_buf);
	}

	if (ubuf && bufflen) {
		ret = blk_rq_map_user(q, rq, NULL, ubuf, bufflen, GFP_KERNEL);
		if (ret)
			goto err_ppa;
		bio = rq->bio;

		if (meta_buf && meta_len) {
			metadata = dma_pool_page_zalloc(dev, &metadata_dma);
			if (!metadata) {
				ret = -ENOMEM;
				goto err_map;
			}

			if (write) {
				if (copy_from_user(metadata,
						(void __user *)meta_buf,
						meta_len)) {
					ret = -EFAULT;
					goto err_meta;
				}
			}
			cmd->metadata = cpu_to_le64(metadata_dma);
		}

		if (!disk)
			goto submit;

		bio->bi_bdev = bdget_disk(disk, 0);
		if (!bio->bi_bdev) {
			ret = -ENODEV;
			goto err_meta;
		}
	}

submit:
	blk_execute_rq_nowait(q, NULL, rq, 0, user_passthru_cmd_completion);

	wait_for_completion_io(&wait);

	printk("status:0x%x  result.u64:0x%llx\n", 
		rq->errors, nvme_req(rq)->result.u64);

	if (metadata && !ret && !write) {
		if (copy_to_user(meta_buf, (void *)metadata, meta_len))
			ret = -EFAULT;
	}
err_meta:
	if (meta_buf && meta_len)
		dma_pool_page_free(dev, metadata, metadata_dma);
err_map:
	if (bio) {
		if (disk && bio->bi_bdev)
			bdput(bio->bi_bdev);
		blk_rq_unmap_user(bio);
	}
err_ppa:
	if (ppa_buf && nlb)
		dma_pool_page_free(dev, ppa_list, ppa_dma);
err_rq:
	blk_mq_free_request(rq);
err_cmd:
	return ret;
}

static int nvme_submit_user_ppa_cmd(struct nvm_exdev *dev, struct nvme_user_io *uvio)
{
	int ret;
	struct nvme_user_io vio;
        struct nvme_ppa_command c;
	u16 nlb = 0;
	int metalen = 0;
	int datalen = 0;

	if (copy_from_user(&vio, uvio, sizeof(vio)))
		return -EFAULT;
	if (vio.flags)
		return -EINVAL;
	
	nlb = vio.nblocks;     // this is 0-base, aligh with spec
	metalen = (nlb + 1) * NAND_META_SIZE;
	datalen = (nlb + 1) * EXP_PPA_SIZE;

	memset(&c, 0, sizeof(c));
	c.opcode = vio.opcode;
	c.nsid = cpu_to_le32(dev->bns->ns_id);
	c.control = cpu_to_le16(vio.control);
	c.nlb = cpu_to_le16(vio.nblocks);
        c.dsmgmt = cpu_to_le32(vio.dsmgmt);

	ret = submit_user_passthru_cmd(dev, &c,
                     (void __user *)(uintptr_t)vio.addr, datalen,
                     (void __user *)(uintptr_t)vio.metadata, metalen,
		     (void __user *)(uintptr_t)vio.slba, nlb, 0);
	
	return ret;
}

static int backend_dev_open(struct inode *inode, struct file *f)
{
	struct nvm_exdev *dev = container_of(f->private_data, \
					struct nvm_exdev, miscdev);
	kref_get(&dev->kref);
	f->private_data = dev;

	return 0;
}

static int backend_dev_release(struct inode *inode, struct file *f)
{
	struct nvm_exdev *dev = f->private_data;
	
	kref_put(&dev->kref, NULL);
	
	return 0;
}

static long backend_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	struct nvm_exdev *dev = f->private_data;
	switch (cmd) {
	case FSCFTL_IOCTL_USER_PPA_CMD:
		return nvme_submit_user_ppa_cmd(dev, (void __user *)arg);
	default:
		pr_err("ioctl cmdcode invalid\n");
		return -ENOTTY;
	}
}

static const struct file_operations backdev_dev_fops = {
	.owner             = THIS_MODULE,
	.open              = backend_dev_open,
	.release           = backend_dev_release,
	.unlocked_ioctl    = backend_dev_ioctl,
	.compat_ioctl      = backend_dev_ioctl,
};

int backed_miscdev_create(struct nvm_exdev *dev)
{
	int ret = 0;

	scnprintf(dev->miscname, DISK_NAME_LEN, "%sbe", dev->bdiskname);

	dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	dev->miscdev.name = dev->miscname;
	dev->miscdev.fops = &backdev_dev_fops;
	ret = misc_register(&dev->miscdev);
	if (ret)
		return -EFAULT;

	TRACE_TAG("Backedn miscdevice %s create success", dev->miscname);
	
	return 0;
}

void backend_miscdev_delete(struct nvm_exdev *dev)
{
	misc_deregister(&dev->miscdev);
	
	TRACE_TAG("Backend miscdevice %s del success", dev->miscname);

	return ;
}
////////////////////////////////

static int __init fscftl_module_init(void)
{
	int ret = 0;
	struct nvm_exdev *exdev;

	printk("NandFlash type:%s\n", FLASH_TYPE);

	exdev = nvm_find_exdev(exdev_name);
	if (exdev == NULL)
		return -ENODEV;

	printk("find exdev:%s idndw:0x%x\n", exdev->bdiskname, exdev->magic_dw);

	exdev->ops = &exdev_ppa_ops;

	ctrl_register_config(exdev->ctrl);

	ret = nvm_exdev_setup_pool(exdev, "prp-ppa-list");
	if (ret) 
		return ret;

	ret = fscftl_setup(exdev);
	if (ret)
		goto release_pool;

	if (mcp) {
		if (do_manufactory_init(exdev))
	    		goto err_cleanup;
	} else {
		if (try_recovery_systbl(exdev))
	    		goto err_cleanup;
	}
	
	prepare_write_context();

	ret = nvm_create_exns(exdev);
	if (ret)
		goto err_cleanup;

	return 0;

err_cleanup:
	fscftl_cleanup(exdev);
release_pool:
	nvm_exdev_release_pool(exdev);
	return -EFAULT;
}

static void __exit fscftl_module_exit(void)
{
	struct nvm_exdev *exdev = nvm_find_exdev(exdev_name);

	nvm_delete_exns(exdev);
	flush_down_systbl(exdev);
	fscftl_cleanup(exdev);
	nvm_exdev_release_pool(exdev);
    
	return;
}

#if MDULE_TEST
static int __init fscftl_test_init(void)
{
	return 0;
}
static void __exit fscftl_test_exit(void)
{
	return;
}

module_init(fscftl_test_init);
module_exit(fscftl_test_exit);

#else
module_init(fscftl_module_init);
module_exit(fscftl_module_exit);
#endif

MODULE_AUTHOR("Dengcai Xu <dxu@cnexlabs.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Host-Based Full Stack Control FTL for NVMe SSDs");

