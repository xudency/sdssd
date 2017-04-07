#include <linux/miscdevice.h>
#include <linux/genhd.h>
#include <linux/nvme_ioctl.h>
#include "backend.h"
#include "../fscftl.h"
#include "../../nvme/host/nvme.h"
#include "../utils/utils.h"

// nvme-cli extend
// nvme-cli fscftl passthru
static void user_passthru_cmd_completion(struct request *rq, int error)
{
	struct completion *waiting = rq->end_io_data;

	complete(waiting);
}

static int handle_user_ppa_cmd(struct nvm_exdev *dev,
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

	ret = handle_user_ppa_cmd(dev, &c,
                     (void __user *)(uintptr_t)vio.addr, datalen,
                     (void __user *)(uintptr_t)vio.metadata, metalen,
		     (void __user *)(uintptr_t)vio.slba, nlb, 0);
	
	return ret;
}

static int nvme_be_ioctl_test(struct nvm_exdev *dev, struct nvme_user_io *uvio)
{
	int i;
	u64 ppa_list[MAX_PPA_PER_CMD];
	struct nvme_user_io vio;
	
	if (copy_from_user(&vio, uvio, sizeof(vio)))
		return -EFAULT;

	if (copy_from_user(ppa_list, (void __user *)vio.slba,
			sizeof(u64) * (vio.nblocks + 1)))

	printk("opcode     :0x%x\n", vio.opcode);
	printk("nlb        :%d\n", vio.nblocks);
	printk("control    :0x%x\n", vio.control);
	printk("dsmgmt     :0x%x\n", vio.dsmgmt);

	for (i = 0; i <= vio.nblocks; i++)
		printk("Kernel ppa[%d]=0x%llx\n", i, ppa_list[i]);

	return 0;
}

static int backend_dev_open(struct inode *inode, struct file *f)
{
	struct nvm_exdev *dev = container_of(f->private_data, \
					struct nvm_exdev, miscdev);
	//kref_get(&dev->kref);
	f->private_data = dev;

	return 0;
}

static int backend_dev_release(struct inode *inode, struct file *f)
{
	//struct nvm_exdev *dev = f->private_data;
	
	//kref_put(&dev->kref, NULL);
	
	return 0;
}

static long backend_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	struct nvm_exdev *dev = f->private_data;
	switch (cmd) {
	case FSCFTL_IOCTL_SMOKE_TEST:
		return nvme_be_ioctl_test(dev, (void __user *)arg);
		
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

/*
static struct miscdevice backend_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "nvme0n1be",
	.fops = &backdev_dev_fops;
}

module_misc_device(backend_misc_device);*/

int backend_miscdev_create(struct nvm_exdev *dev)
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

