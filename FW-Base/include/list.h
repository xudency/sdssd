#ifndef _LIST_H_
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


