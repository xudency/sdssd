/*
 * Manage FTL sys metadata, Various kind of table
 *	l2ptbl
 *	bmitbl
 *	firstpage
 *	ftllog
 *	vpctbl
 *	freelist/closelist/openblk
 */ 
#include <linux/vmalloc.h>
#include "sys-meta.h"


int l2ptbl_init(struct nvm_exdev *exdev)
{
	exdev->l2ptbl = vmalloc(sizeof(u32) * MAX_USER_LBA);
	if (!exdev->l2ptbl) {
		printk("l2ptbl malloc failed\n");
		return -ENOMEM;
	}

	return 0;
}

void l2ptbl_exit(struct nvm_exdev *exdev)
{
	vfree(exdev->l2ptbl);
}


