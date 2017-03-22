#include "ppa-ops.h"
#include "bio-datapath.h"
#include "../writecache/wcb-mngr.h"


PPA_TYPE sys_get_ppa_type(geo_ppa ppa)
{
    // lookup BMI->bb

    // xor last ch

    // ep pl ln pg=0

    // 255 511 pg ep=3

    return USR_DATA;
}

// TODO::
bool wcb_available(int nr_ppas)
{
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

			// all travesal ppa need inc, regardless of pagetype
			if (atomic_inc_return(&entitys->fill_cnt) == RAID_LUN_SEC_NUM) {
				// TODO:: 
				// push this lun_entity to full fifo and wait write_ts kickin
				//wake_up(&write_ts);

				
				printk("push entity%d to full fifo\n", entitys->index);
				push_lun_entity_to_fifo(&g_wcb_lun_ctl->full_lun, entitys);


				{
					u32 num;
					struct wcb_lun_entity *entry;
					struct fsc_fifo *full_lun =  &g_wcb_lun_ctl->full_lun;

					num = full_lun->head;
					while (num != 0xffff) {
						entry = wcb_lun_entity_idx(num);
						num = entry->prev;
						printk("walk find full fifo entity%d\n", entry->index);
					}						
				
					printk("****************************************");
				}


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

