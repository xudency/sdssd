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
#include "backend/backend.h"
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

	ret = backend_miscdev_create(exdev);
	if (ret)
		goto release_pool;

	ret = fscftl_setup(exdev);
	if (ret)
		goto del_misc_dev;

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
del_misc_dev:
	backend_miscdev_delete(exdev);
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
	backend_miscdev_delete(exdev);
	nvm_exdev_release_pool(exdev);
    
	return;
}

#if MDULE_TEST
static int __init fscftl_test_init(void)
{
	printk("MAX_USER_LBA:0x%llx  EXTEND_LBA_BASE:0x%llx\n", 
		MAX_USER_LBA, EXTEND_LBA_BASE);
	printk("L2Ptbl need PPAs:%lld [0x%llx-0x%llx]\n", 
		USR_FTLTBL_SEC_NUM, EXTEND_LBA_UFTL, EXTEND_LBA_BMITBL-1);
	printk("BMItbl need PPAs:%ld [0x%llx-0x%llx]\n", 
		BMITBL_SEC_NUM, EXTEND_LBA_BMITBL, EXTEND_LBA_VPCTBL-1);
	printk("VPCtbl need PPAs:%d [0x%llx-0x%llx]\n", 
		VPCTBL_SEC_NUM, EXTEND_LBA_VPCTBL, EXTEND_LBA_L1TBL-1);
	printk("L1tbl need PPAs:%lld [0x%llx-0x%llx]\n", 
		L1TBL_SEC_NUM, EXTEND_LBA_L1TBL, EXTEND_LBA_RSVD0-1);

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

