#ifndef _LIST_H_#define _LIST_H_#define DLIST_EMPTY 0xffff
/* Directions for iterators */
#define DLIST_START_HEAD 0
#define DLIST_START_TAIL 1


// List pointer to next prev by u16 rather than *(address) 
typedef struct dlink_list {
	u16 head;	 // pull
	u16 tail;    // push
	u16 size;
	u16 rsvd;
} dlist;

// dlink_list iterator
typedef struct dlist_iter {
	u16 next;
	u16 direction;  // start from HEAD or Tail
} dlist_iter;
static inline bool dlist_size(dlist *dlist){	return dlist->size;}
static inline bool dlist_empty(dlist *dlist){	return dlist->size == 0;}static inline void dlink_list_init(dlist *dlist){	dlist->head = 0xffff;	dlist->tail = 0xffff;	dlist->size = 0;	dlist->rsvd = 0;}static inline void dlist_iter_init(dlist *list, dlist_iter *iter, bool dir){	iter->direction = dir;	if (dir == DLIST_START_HEAD)		iter->next = list->head;	else		iter->next = list->tail;}static inline u16 rblk_iter(dlist_iter *iter){	u16 blk = iter->next;	if (blk != 0xffff) {		if (iter->direction == DLIST_START_HEAD)			iter->next = BMI_PREV_BLK(blk);		else			iter->next = BMI_NEXT_BLK(blk);	}	return blk;}/* pos: 0-head  1-tail*/static inline int peek_dlink_list(dlist *dlist, bool pos){	if (dlist_empty(dlist))		return -1;	return pos ? dlist->tail: dlist->head;}// iter should define and dlist_iter_init/*#define dlist_for_each_entry(dlist, blk, dir){	dlist_iter iter;	dlist_iter_init(dlist, &iter, dir)	while((blk = rblk_iter(&iter) != 0xffff) {	}}*/// travesal dlist from head to tail, sequence little ---> large#define dlist_for_each_head(blk, dlist) \	for (blk = (dlist)->head; blk != 0xffff; blk = BMI_PREV_BLK(blk))// travesal dlist from head, sequence large ---> little#define dlist_for_each_tail(blk, dlist) \		for (blk = (dlist)->tail; blk != 0xffff; blk = BMI_NEXT_BLK(blk))/////////////////Single List///////////////////typedef struct _s_node_t_{	struct _s_node_t_	* next;} s_node_t, * p_s_node_t;typedef struct _s_queue_t_{	s_node_t	* head;	s_node_t	* tail;	u32 	count;} s_queue_t, * p_s_queue_t;#endif
