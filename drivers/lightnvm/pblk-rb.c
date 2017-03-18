/*
 * Copyright (C) 2016 CNEX Labs
 * Initial release: Javier Gonzalez <javier@cnexlabs.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 * pblk-rb.c - pblk's ring buffer
 */

#include <linux/circ_buf.h>

#include "pblk.h"

static DECLARE_RWSEM(pblk_rb_lock);

void pblk_rb_data_free(struct pblk_rb *rb)
{
	struct pblk_rb_pages *p, *t;

	down_write(&pblk_rb_lock);
	list_for_each_entry_safe(p, t, &rb->pages, list) {
		free_pages((unsigned long)page_address(p->pages), p->order);
		list_del(&p->list);
		kfree(p);
	}
	up_write(&pblk_rb_lock);
}

/*
 * Initialize ring buffer. The data and metadata buffers must be previously
 * allocated and their size must be a power of two
 * (Documentation/circular-buffers.txt)
 */
int pblk_rb_init(struct pblk_rb *rb, struct pblk_rb_entry *rb_entry_base,
		 unsigned int power_size, unsigned int power_seg_sz)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);
	unsigned long init_entry = 0;
	unsigned int alloc_order = power_size;
	unsigned int max_order = MAX_ORDER - 1;
	unsigned int order, iter;

	down_write(&pblk_rb_lock);
	rb->entries = rb_entry_base;
	rb->seg_size = (1 << power_seg_sz);
	rb->nr_entries = (1 << power_size);
	rb->mem = rb->subm = rb->sync = rb->l2p_update = 0;
	rb->sync_point = RB_EMPTY_ENTRY;

	spin_lock_init(&rb->w_lock);
	spin_lock_init(&rb->s_lock);

	INIT_LIST_HEAD(&rb->pages);

	if (alloc_order >= max_order) {
		order = max_order;
		iter = (1 << (alloc_order - max_order));
	} else {
		order = alloc_order;
		iter = 1;
	}

	do {
		struct pblk_rb_entry *entry;
		struct pblk_rb_pages *page_set;
		void *kaddr;
		unsigned long set_size;
		int i;

		page_set = kmalloc(sizeof(struct pblk_rb_pages), GFP_KERNEL);
		if (!page_set) {
			up_write(&pblk_rb_lock);
			return -ENOMEM;
		}

		page_set->order = order;
		page_set->pages = alloc_pages(GFP_KERNEL, order);
		if (!page_set->pages) {
			kfree(page_set);
			pblk_rb_data_free(rb);
			up_write(&pblk_rb_lock);
			return -ENOMEM;
		}
		kaddr = page_address(page_set->pages);

		entry = &rb->entries[init_entry];
		entry->data = kaddr;
		entry->cacheline = pblk_cacheline_to_ppa(init_entry++);
		entry->w_ctx.flags = PBLK_WRITABLE_ENTRY;

		set_size = (1 << order);
		for (i = 1; i < set_size; i++) {
			entry = &rb->entries[init_entry];
			entry->cacheline = pblk_cacheline_to_ppa(init_entry++);
			entry->data = kaddr + (i * rb->seg_size);
			entry->w_ctx.flags = PBLK_WRITABLE_ENTRY;
			bio_list_init(&entry->w_ctx.bios);
		}

		list_add_tail(&page_set->list, &rb->pages);
		iter--;
	} while (iter > 0);
	up_write(&pblk_rb_lock);

#ifdef CONFIG_NVM_DEBUG
	atomic_set(&rb->inflight_sync_point, 0);
#endif

	/*
	 * Initialize rate-limiter, which controls access to the write buffer
	 * but user and GC I/O
	 */
	pblk_rl_init(&pblk->rl, rb->nr_entries, &rb->w_lock);

	return 0;
}

/*
 * pblk_rb_calculate_size -- calculate the size of the write buffer
 */
unsigned long pblk_rb_calculate_size(unsigned long nr_entries)
{
	unsigned int power_size;

	power_size = get_count_order(nr_entries);

	/* Have a write buffer that can fit 256KB I/Os */
	power_size = (power_size < 7) ? 7 : power_size;
	return (1 << power_size);
}

void *pblk_rb_entries_ref(struct pblk_rb *rb)
{
	return rb->entries;
}

static void clean_wctx(struct pblk_w_ctx *w_ctx)
{
	smp_store_release(&w_ctx->flags, PBLK_WRITABLE_ENTRY);
	ppa_set_empty(&w_ctx->ppa);
}

#define pblk_rb_ring_count(head, tail, size) CIRC_CNT(head, tail, size)
#define pblk_rb_ring_space(rb, head, tail, size) \
					(CIRC_SPACE(head, tail, size))

/*
 * Buffer space is calculated with respect to the back pointer signaling
 * synchronized entries to the media.
 */
unsigned long pblk_rb_space(struct pblk_rb *rb)
{
	unsigned long mem = READ_ONCE(rb->mem);
	unsigned long sync = READ_ONCE(rb->sync);

	return pblk_rb_ring_space(rb, mem, sync, rb->nr_entries);
}

/*
 * Buffer count is calculated with respect to the submission entry signaling the
 * entries that are available to send to the media
 */
unsigned long pblk_rb_read_count(struct pblk_rb *rb)
{
	unsigned long mem = READ_ONCE(rb->mem);
	unsigned long subm = READ_ONCE(rb->subm);

	return pblk_rb_ring_count(mem, subm, rb->nr_entries);
}

unsigned long pblk_rb_read_commit(struct pblk_rb *rb, unsigned int nr_entries)
{
	unsigned long subm;

	subm = READ_ONCE(rb->subm);
	/* Commit read means updating submission pointer */
	smp_store_release(&rb->subm,
				(subm + nr_entries) & (rb->nr_entries - 1));

	return subm;
}

#if 0
static void pblk_rb_requeue_entry(struct pblk_rb *rb,
				  struct pblk_rb_entry *entry)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);
	struct ppa_addr ppa;
	unsigned long mem, sync;

	/* Serialized in pblk_rb_write_init */
	mem = READ_ONCE(rb->mem);
	sync = READ_ONCE(rb->sync);

	/* Maintain original bio, lba and flags */
	pblk_ppa_set_empty(&entry->w_ctx.ppa);
	entry->w_ctx.paddr = 0;

	/* Move entry to the head of the write buffer and update l2p */
	while (pblk_rb_ring_space(rb, mem, sync, rb->nr_entries) < 1)
		;
	pblk_rb_write_entry(rb, entry->data, entry->w_ctx, mem);

	ppa = pblk_cacheline_to_ppa(mem);
	pblk_update_map(pblk, entry->w_ctx.lba, ppa);

	/* Update memory pointer (head) */
	smp_store_release(&rb->mem, (mem + 1) & (rb->nr_entries - 1));

#ifdef CONFIG_NVM_DEBUG
	atomic_inc(&pblk->inflight_writes);
	atomic_inc(&pblk->requeued_writes);
#endif
}
#endif

// JAVIER: WRITE FAILURE RECOVERY
static int __pblk_rb_update_l2p(struct pblk_rb *rb, unsigned long *l2p_upd,
				unsigned long to_update)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);
	struct pblk_line *line;
	struct pblk_rb_entry *entry;
	struct pblk_w_ctx *w_ctx;
	/* struct pblk_block *rblk; */
	unsigned long i;

	for (i = 0; i < to_update; i++) {
		entry = &rb->entries[*l2p_upd];
		w_ctx = &entry->w_ctx;
		/* rblk = w_ctx->ppa.rblk; */

		/* Grown bad block. For now, we requeue the entry to the write
		 * buffer and make it take the normal path to get a new ppa
		 * mapping. Since the requeue takes a place on the buffer,
		 * unpdate an extra entry.
		 */
		/* if (unlikely(block_is_bad(rblk))) { */
			/* pblk_rb_requeue_entry(rb, entry); */
			/* goto next_unlock; */
		/* } */

		line = &pblk->lines[pblk_ppa_to_line(w_ctx->ppa)];
		pblk_update_map_dev(pblk, w_ctx->lba, w_ctx->ppa,
							entry->cacheline);
		kref_put(&line->ref, pblk_line_put);
/* next_unlock: */
		clean_wctx(w_ctx);
		*l2p_upd = (*l2p_upd + 1) & (rb->nr_entries - 1);
	}

	return 0;
}

/*
 * When we move the l2p_update pointer, we update the l2p table - lookups will
 * point to the physical address instead of to the cacheline in the write buffer
 * from this moment on.
 */
static int pblk_rb_update_l2p(struct pblk_rb *rb, unsigned int nr_entries,
			      unsigned long mem, unsigned long sync)
{
	unsigned long space, count;
	int ret = 0;

#ifdef CONFIG_NVM_DEBUG
	lockdep_assert_held(&rb->w_lock);
#endif

	/* Update l2p only as buffer entries are being overwritten */
	space = pblk_rb_ring_space(rb, mem, rb->l2p_update, rb->nr_entries);
	if (space > nr_entries)
		goto out;

	count = nr_entries - space;
	/* l2p_update used exclusively under rb->w_lock */
	ret = __pblk_rb_update_l2p(rb, &rb->l2p_update, count);

out:
	return ret;
}

/*
 * Update the l2p entry for all sectors stored on the write buffer. This means
 * that all future lookups to the l2p table will point to a device address, not
 * to the cacheline in the write buffer.
 */
void pblk_rb_sync_l2p(struct pblk_rb *rb)
{
	unsigned long sync;
	unsigned int to_update;

	spin_lock(&rb->w_lock);

	/* Protect from reads and writes */
	sync = smp_load_acquire(&rb->sync);

	to_update = pblk_rb_ring_count(sync, rb->l2p_update, rb->nr_entries);
	__pblk_rb_update_l2p(rb, &rb->l2p_update, to_update);

	spin_unlock(&rb->w_lock);
}

/*
 * Write @nr_entries to ring buffer from @data buffer if there is enough space.
 * Typically, 4KB data chunks coming from a bio will be copied to the ring
 * buffer, thus the write will fail if not all incoming data can be copied.
 *
 */
struct ppa_addr pblk_rb_write_entry(struct pblk_rb *rb, void *data,
				    struct pblk_w_ctx w_ctx,
				    unsigned int ring_pos)
{
	struct pblk_rb_entry *entry;
	int flags;

	entry = &rb->entries[ring_pos];
try:
	flags = READ_ONCE(entry->w_ctx.flags);
	if (!(flags & PBLK_WRITABLE_ENTRY))
		goto try;

	memcpy(entry->data, data, rb->seg_size);

	entry->w_ctx.lba = w_ctx.lba;
	entry->w_ctx.ppa = w_ctx.ppa;
	entry->w_ctx.paddr = w_ctx.paddr;
	flags |= w_ctx.flags;

	/* if (bio_list_empty(&w_ctx.bios)) { */
	/* if (!(flags & PBLK_FLUSH_ENTRY)) { */
		/* Release pointer controlling flushes */
		/* smp_store_release(&rb->sync_point, ring_pos); */
	/* } */

	flags &= ~PBLK_WRITABLE_ENTRY;
	flags |= PBLK_WRITTEN_DATA;

	/* Release flags on write context. Protect from writes */
	smp_store_release(&entry->w_ctx.flags, flags);

	return entry->cacheline;
}

int __pblk_rb_may_write(struct pblk_rb *rb, unsigned int nr_entries,
			unsigned long *pos)
{
	unsigned long mem;
	unsigned long sync;

	sync = READ_ONCE(rb->sync);
	mem = READ_ONCE(rb->mem);

	if (pblk_rb_ring_space(rb, mem, sync, rb->nr_entries) < nr_entries)
		return 0;

	if (pblk_rb_update_l2p(rb, nr_entries, mem, sync))
		return 0;

	*pos = mem;

	/* Protect from read count */
	smp_store_release(&rb->mem, (mem + nr_entries) & (rb->nr_entries - 1));
	return 1;
}

/*
 * Atomically check that (i) there is space on the write buffer for the
 * incoming I/O, and (ii) the current I/O type has enough budget in the write
 * buffer (rate-limiter).
 */
int pblk_rb_may_write_user(struct pblk_rb *rb, unsigned int nr_entries,
			   unsigned long *pos)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);

	spin_lock(&rb->w_lock);
	if (!pblk_rl_user_may_in(&pblk->rl, nr_entries)) {
		spin_unlock(&rb->w_lock);
		pblk_write_kick(pblk);
		return 0;
	}

	if (!__pblk_rb_may_write(rb, nr_entries, pos)) {
		spin_unlock(&rb->w_lock);
		return 0;
	}

	pblk_rl_user_in(&pblk->rl, nr_entries);
	spin_unlock(&rb->w_lock);

	return 1;
}

/*
 * Look at pblk_rb_may_write_user comment
 */
int pblk_rb_may_write_gc(struct pblk_rb *rb, unsigned int nr_entries,
			   unsigned long *pos)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);

	spin_lock(&rb->w_lock);
	if (!pblk_rl_gc_may_in(&pblk->rl, nr_entries)) {
		spin_unlock(&rb->w_lock);
		pblk_write_kick(pblk);
		return 0;
	}

	if (!__pblk_rb_may_write(rb, nr_entries, pos)) {
		spin_unlock(&rb->w_lock);
		return 0;
	}

	pblk_rl_gc_in(&pblk->rl, nr_entries);
	spin_unlock(&rb->w_lock);

	return 1;
}

/*
 * The caller of this function must ensure that the backpointer will not
 * overwrite the entries passed on the list.
 */
unsigned int pblk_rb_read_to_bio_list(struct pblk_rb *rb, struct bio *bio,
				      struct list_head *list,
				      unsigned int max)
{
	struct pblk_rb_entry *entry, *tentry;
	struct page *page;
	unsigned int read = 0;
	int ret;

	list_for_each_entry_safe(entry, tentry, list, index) {
		if (read > max) {
			pr_err("pblk: too many entries on list\n");
			goto out;
		}

		page = virt_to_page(entry->data);
		if (!page) {
			pr_err("pblk: could not allocate write bio page\n");
			goto out;
		}

		ret = bio_add_page(bio, page, rb->seg_size, 0);
		if (ret != rb->seg_size) {
			pr_err("pblk: could not add page to write bio\n");
			goto out;
		}

		list_del(&entry->index);
		read++;
	}

out:
	return read;
}

/*
 * Read available entries on rb and add them to the given bio. To avoid a memory
 * copy, a page reference to the write buffer is used to be added to the bio.
 *
 * This function is used by the write thread to form the write bio that will
 * persist data on the write buffer to the media.
 */
unsigned int pblk_rb_read_to_bio(struct pblk_rb *rb, struct bio *bio,
				 struct pblk_compl_ctx *c_ctx,
				 unsigned long pos,
				 unsigned int nr_entries,
				 unsigned int count,
				 unsigned long *sync_point)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);
	struct pblk_rb_entry *entry;
	struct page *page;
	unsigned int pad = 0, read = 0, to_read = nr_entries;
	unsigned int user_io = 0, gc_io = 0;
	unsigned int i;
	int flags;
	int ret;

	if (count < nr_entries) {
		pad = nr_entries - count;
		to_read = count;
	}

	c_ctx->sentry = pos;
	c_ctx->nr_valid = to_read;
	c_ctx->nr_padded = pad;

	for (i = 0; i < to_read; i++) {
		entry = &rb->entries[pos];

		/* A write has been allowed into the buffer, but data is still
		 * being copied to it. It is ok to busy wait.
		 */
try:
		flags = READ_ONCE(entry->w_ctx.flags);
		if (!(flags & PBLK_WRITTEN_DATA))
			goto try;

		if (flags & PBLK_IOTYPE_USER)
			user_io++;
		else if (flags & PBLK_IOTYPE_GC)
			gc_io++;
		else
			WARN(1, "pblk: unknown IO type\n");

		page = virt_to_page(entry->data);
		if (!page) {
			pr_err("pblk: could not allocate write bio page\n");
			flags &= ~PBLK_WRITTEN_DATA;
			flags |= PBLK_WRITABLE_ENTRY;
			/* Release flags on context. Protect from writes */
			smp_store_release(&entry->w_ctx.flags, flags);
			goto out;
		}

		ret = bio_add_page(bio, page, rb->seg_size, 0);
		if (ret != rb->seg_size) {
			pr_err("pblk: could not add page to write bio\n");
			flags &= ~PBLK_WRITTEN_DATA;
			flags |= PBLK_WRITABLE_ENTRY;
			/* Release flags on context. Protect from writes */
			smp_store_release(&entry->w_ctx.flags, flags);
			goto out;
		}

		/* if (!bio_list_empty(&entry->w_ctx.bios)) { */
		if (flags & PBLK_FLUSH_ENTRY) {
			*sync_point = pos;
#ifdef CONFIG_NVM_DEBUG
			atomic_dec(&rb->inflight_sync_point);
#endif
		}

		flags &= ~PBLK_WRITTEN_DATA;
		flags |= PBLK_WRITABLE_ENTRY;

		/* Release flags on context. Protect from writes */
		smp_store_release(&entry->w_ctx.flags, flags);

		pos = (pos + 1) & (rb->nr_entries - 1);
	}

	read = to_read;

	pblk_rl_out(&pblk->rl, user_io, gc_io);

#ifdef CONFIG_NVM_DEBUG
	atomic_add(pad, &((struct pblk *)
			(container_of(rb, struct pblk, rwb)))->padded_writes);
#endif

out:
	return read;
}

/*
 * Copy to bio only if the lba matches the one on the given cache entry.
 * Otherwise, it means that the entry has been overwritten, and the bio should
 * be directed to disk.
 */
int pblk_rb_copy_to_bio(struct pblk_rb *rb, struct bio *bio, sector_t lba,
			u64 pos, int bio_iter)
{
	struct pblk_rb_entry *entry;
	struct pblk_w_ctx *w_ctx;
	void *data;
	int ret = 1;

	spin_lock(&rb->w_lock);

#ifdef CONFIG_NVM_DEBUG
	BUG_ON(pos >= rb->nr_entries);
#endif
	entry = &rb->entries[pos];
	w_ctx = &entry->w_ctx;

	/* Check if the entry has been overwritten */
	if (w_ctx->lba != lba) {
		ret = 0;
		goto out;
	}

	/* Only advance the bio if it hasn't been advanced already. If advanced,
	 * this bio is at least a partial bio (i.e., it has partially been
	 * filled with data from the cache). If part of the data resides on the
	 * media, we will read later on
	 */
	if (unlikely(!bio->bi_iter.bi_idx))
		bio_advance(bio, bio_iter * PBLK_EXPOSED_PAGE_SIZE);

	data = bio_data(bio);
	memcpy(data, entry->data, rb->seg_size);

out:
	spin_unlock(&rb->w_lock);
	return ret;
}

struct pblk_w_ctx *pblk_rb_w_ctx(struct pblk_rb *rb, unsigned long pos)
{
	unsigned long entry = pos & (rb->nr_entries - 1);

	return &rb->entries[entry].w_ctx;
}

unsigned long pblk_rb_sync_init(struct pblk_rb *rb, unsigned long *flags)
{
	if (flags)
		spin_lock_irqsave(&rb->s_lock, *flags);
	else
		spin_lock_irq(&rb->s_lock);

	return rb->sync;
}

unsigned long pblk_rb_sync_advance(struct pblk_rb *rb, unsigned int nr_entries)
{
	struct pblk_rb_entry *entry;
	struct pblk_w_ctx *w_ctx;
	unsigned long sync;
	unsigned long i;

#ifdef CONFIG_NVM_DEBUG
	lockdep_assert_held(&rb->s_lock);
#endif

	sync = READ_ONCE(rb->sync);

	for (i = 0; i < nr_entries; i++) {
		entry = &rb->entries[sync];
		w_ctx = &entry->w_ctx;
		sync = (sync + 1) & (rb->nr_entries - 1);
	}

	/* Protect from counts */
	smp_store_release(&rb->sync, sync);

	return sync;
}

void pblk_rb_sync_end(struct pblk_rb *rb, unsigned long *flags)
{
#ifdef CONFIG_NVM_DEBUG
	lockdep_assert_held(&rb->s_lock);
#endif

	if (flags)
		spin_unlock_irqrestore(&rb->s_lock, *flags);
	else
		spin_unlock_irq(&rb->s_lock);
}

int pblk_rb_sync_point_set(struct pblk_rb *rb, struct bio *bio)
{
	struct pblk_rb_entry *entry;
	unsigned long mem, subm, sync_point;
	int flags;

	/* Protect from reads and writes */
	mem = smp_load_acquire(&rb->mem);
	/* Protect syncs */
	sync_point = smp_load_acquire(&rb->sync_point);
	subm = READ_ONCE(rb->subm);

#ifdef CONFIG_NVM_DEBUG
	atomic_inc(&rb->inflight_sync_point);
#endif

	if (mem == subm)
		return 0;

	spin_lock_irq(&rb->s_lock);

	sync_point = (mem == 0) ? (rb->nr_entries - 1) : (mem - 1);
	entry = &rb->entries[sync_point];

	flags = READ_ONCE(entry->w_ctx.flags);
	flags |= PBLK_FLUSH_ENTRY;

	bio_list_add(&entry->w_ctx.bios, bio);

	/* Protect syncs */
	smp_store_release(&rb->sync_point, sync_point);

	/* Release flags on context. Protect from writes */
	smp_store_release(&entry->w_ctx.flags, flags);

	spin_unlock_irq(&rb->s_lock);
	return 1;
}

void pblk_rb_sync_point_reset(struct pblk_rb *rb, unsigned long sp)
{
	struct pblk_rb_entry *entry;
	unsigned long sync_point;

	/* Protect syncs */
	sync_point = smp_load_acquire(&rb->sync_point);

	/* Entry protected by sync backpointer. No need to take lock */
	if (sync_point == sp) {
		int flags;

		entry = &rb->entries[sp];
		flags = READ_ONCE(entry->w_ctx.flags);

#ifdef CONFIG_NVM_DEBUG
		BUG_ON(!(flags & PBLK_FLUSH_ENTRY));
#endif
		flags &= ~PBLK_FLUSH_ENTRY;

		/* Protect syncs */
		smp_store_release(&rb->sync_point, ADDR_EMPTY);
		/* Release flags on context. Protect from writes */
		smp_store_release(&entry->w_ctx.flags, flags);
	}
}

unsigned int pblk_rb_sync_point_count(struct pblk_rb *rb)
{
	unsigned long subm, sync_point;
	unsigned int count;

	/* Protect syncs */
	sync_point = smp_load_acquire(&rb->sync_point);
	if (sync_point == ADDR_EMPTY)
		return 0;

	subm = READ_ONCE(rb->subm);

	/* The sync point itself counts as a sector to sync */
	count = pblk_rb_ring_count(sync_point, subm, rb->nr_entries) + 1;

	return count;
}

/*
 * Scan from the current position of the sync pointer to find the entry that
 * corresponds to the given ppa. This is necessary since write requests can be
 * completed out of order. The assumption is that the ppa is close to the sync
 * pointer thus the search will not take long.
 *
 * The caller of this function must guarantee that the sync pointer will no
 * reach the entry while it is using the metadata associated with it. With this
 * assumption in mind, there is no need to take the sync lock.
 */
//JAVIER: WRITE RECOVERY LOOK INTO
#if 0
struct pblk_rb_entry *pblk_rb_sync_scan_entry(struct pblk_rb *rb,
					      struct ppa_addr *ppa)
{
	struct pblk *pblk = container_of(rb, struct pblk, rwb);
	/* struct nvm_tgt_dev *dev = pblk->dev; */
	struct pblk_rb_entry *entry;
	struct pblk_w_ctx *w_ctx;
	struct ppa_addr gppa;
	unsigned long sync, subm, count;
	unsigned long i;

	sync = READ_ONCE(rb->sync);
	subm = READ_ONCE(rb->subm);
	count = pblk_rb_ring_count(subm, sync, rb->nr_entries);

	for (i = 0; i < count; i++) {
		entry = &rb->entries[sync];
		w_ctx = &entry->w_ctx;

		/* gppa = pblk_blk_ppa_to_gaddr(dev, w_ctx->ppa.rblk, w_ctx->paddr); */

		/* if (gppa.ppa == ppa->ppa) */
			/* return entry; */

		sync = (sync + 1) & (rb->nr_entries - 1);
	}

	return NULL;
}
#endif

int pblk_rb_tear_down_check(struct pblk_rb *rb)
{
	struct pblk_rb_entry *entry;
	int i;
	int ret = 0;

	spin_lock(&rb->w_lock);
	spin_lock_irq(&rb->s_lock);

	if ((rb->mem == rb->subm) && (rb->subm == rb->sync) &&
				(rb->sync == rb->l2p_update) &&
				(rb->sync_point == RB_EMPTY_ENTRY)) {
		goto out;
	}

	if (rb->entries)
		goto out;

	for (i = 0; i < rb->nr_entries; i++) {
		entry = &rb->entries[i];

		if (entry->data)
			goto out;
	}

	ret = 1;

out:
	spin_unlock(&rb->w_lock);
	spin_unlock_irq(&rb->s_lock);

	return ret;
}

unsigned long pblk_rb_wrap_pos(struct pblk_rb *rb, unsigned long pos)
{
	return (pos & (rb->nr_entries - 1));
}

int pblk_rb_pos_oob(struct pblk_rb *rb, u64 pos)
{
	return (pos >= rb->nr_entries);
}

#ifdef CONFIG_NVM_DEBUG
ssize_t pblk_rb_sysfs(struct pblk_rb *rb, char *buf)
{
	ssize_t offset;

	if (rb->sync_point != ADDR_EMPTY)
		offset = scnprintf(buf, PAGE_SIZE,
			"%lu\t%lu\t%lu\t%lu\t%lu\t%u\t%lu - %lu/%lu/%u\n",
			rb->nr_entries,
			rb->mem,
			rb->subm,
			rb->sync,
			rb->l2p_update,
			atomic_read(&rb->inflight_sync_point),
			rb->sync_point,
			pblk_rb_read_count(rb),
			pblk_rb_space(rb),
			pblk_rb_sync_point_count(rb));
	else
		offset = scnprintf(buf, PAGE_SIZE,
			"%lu\t%lu\t%lu\t%lu\t%lu\t%u\tNULL - %lu/%lu/%u\n",
			rb->nr_entries,
			rb->mem,
			rb->subm,
			rb->sync,
			rb->l2p_update,
			atomic_read(&rb->inflight_sync_point),
			pblk_rb_read_count(rb),
			pblk_rb_space(rb),
			pblk_rb_sync_point_count(rb));

	return offset;
}

ssize_t pblk_rb_sysfs_vb(struct pblk_rb *rb, char *buf)
{
	struct pblk_rb_entry *entry;
	struct pblk_w_ctx *w_ctx;
	int flags;
	int i;

	for (i = 0; i < rb->nr_entries; i++) {
		entry = &rb->entries[i];
		w_ctx = &entry->w_ctx;
		flags = READ_ONCE(w_ctx->flags);

		pr_err("entry:%d - flags:%d\n", i, flags);
	}

	return 0;
}
#endif

