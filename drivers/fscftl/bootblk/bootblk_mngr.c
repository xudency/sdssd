#include "bootblk_mngr.h"

struct bootblk_bbt_page *boot_bbt_page_info;
struct bootblk_meta_page *boot_meta_page_info;


int bootblk_page_init(void)
{
	BUILD_BUG_ON(sizeof(struct bootblk_bbt_page) > BOOTBLK_PORTION_SIZE);
	BUILD_BUG_ON(sizeof(struct bootblk_meta_page) > BOOTBLK_PORTION_SIZE);

	// bbtpage
	boot_bbt_page_info = kzalloc(BOOTBLK_PORTION_SIZE, GFP_KERNEL);
	if (!boot_bbt_page_info) 
		return -ENOMEM;

	boot_bbt_page_info->magic_dw[0] = BOOTBLK_BBT_MDW0;	
	boot_bbt_page_info->magic_dw[1] = BOOTBLK_BBT_MDW1;
	boot_bbt_page_info->magic_dw[2] = BOOTBLK_BBT_MDW2;
	boot_bbt_page_info->magic_dw[3] = BOOTBLK_BBT_MDW3;

	// metapage
	boot_meta_page_info = kzalloc(BOOTBLK_PORTION_SIZE, GFP_KERNEL);
	if (!boot_meta_page_info)
		goto free_bbt_page;

	boot_meta_page_info->magic_dw[0] = BOOTBLK_META_MDW0;	
	boot_meta_page_info->magic_dw[1] = BOOTBLK_META_MDW1;
	boot_meta_page_info->magic_dw[2] = BOOTBLK_META_MDW2;
	boot_meta_page_info->magic_dw[3] = BOOTBLK_META_MDW3;

	return 0;
	
free_bbt_page:
	kfree(boot_bbt_page_info);
	return -ENOMEM;
}

void bootblk_page_exit(void)
{
	kfree(boot_meta_page_info);
	kfree(boot_bbt_page_info);
}

int bootblk_recovery_meta_page(void)
{
	return 0;
}

void bootblk_flush_meta_page(enum power_down_flag flag)
{
	return;
}

void bootblk_flush_bbt(void)
{
	return;
}

void bootblk_recovery_bbt(void)
{
	return;
}

