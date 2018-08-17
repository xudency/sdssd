/*
 * Copyright (C) 2018-2020 - NET-Swift Corp.
 * Initial release: Dengcai Xu <dengcaixu@net-swift.com>
 *
 * Red-black trees are a type of self-balancing binary search tree
 * it is similar to AVL trees, but provide faster real-time bounded 
 * worst case performance for insertion and deletion 
 * at most two rotations and three rotations, respectively, to balance the tree
 * with slightly slower (but still O(log n)) lookup time.
 * used for storing sortable key/value data pairs.  
 *
 * function:
 * the code is ported from Linux Kernel lib/rbtree.c. 
 * As here rbtree is dedicated design for R-Block management, 
 * so instead of (void *), the rb_node use blknum to identify is enough. 
 *
 */


#ifndef _RB_TREE_H_
#define _RB_TREE_H_


#define RB_NODE_NULL 0xffff
#define	RB_RED		0
#define	RB_BLACK	1


struct rb_node {
	u16  rb_color;
	struct rb_node *rb_parent;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));

typedef struct rb_node rb_node_t;

typedef struct rb_root {
	struct rb_node *rb_node;
	u32 count;
} rb_root_t;


/*
// RB tree node is a BMI
typedef struct rb_tree_node {
	u16 color;
	u16 parent;
	u16 left;
	u16 right;
} rb_node_t;


typedef struct rb_tree_root {
	u16 root;
	u16 count; //rb_node in this tree
	// root is black
	//compare();
} rb_root_t;*/



#define rb_parent(r)       ((rb_node_t *)((r)->rb_parent))
#define rb_left_child(r)   ((rb_node_t *)((r)->rb_left))
#define rb_right_child(r)  ((rb_node_t *)((r)->rb_right))
#define rb_color(r)        ((r)->rb_color)
#define rb_is_red(r)       ((r)->rb_color & RB_RED)
#define rb_is_black(r)     ((r)->rb_color & RB_BLACK)
#define rb_set_red(r)      ((r)->rb_color = RB_RED)
#define rb_set_black(r)    ((r)->rb_color = RB_BLACK)

#define RB_ROOT (rb_root_t) {RB_NODE_NULL, 0}
#define RB_NODE (rb_node_t) {RB_RED, RB_NODE_NULL, RB_NODE_NULL, RB_NODE_NULL}


static inline void RB_NODE_INIT(rb_node_t *node)
{
	node->rb_left = node->rb_right = node->rb_parent = RB_NODE_NULL;
}

static inline void RB_ROOT_INIT(rb_root_t *root)
{
	root->root = RB_NODE_NULL;
	root->count = 0;
}

static inline void rb_set_parent(rb_node_t* node, rb_node_t* p) 
{
	node->rb_parent = p;
}

static inline void rb_set_left(rb_node_t* node, rb_node_t* child) 
{
	node->rb_left = child;
	rb_set_parent(child, node);
}

static inline void rb_set_right(rb_node_t* node, rb_node_t* child) 
{
	node->rb_right = child;
	rb_set_parent(child, node);
}

static inline bool rb_is_left(rb_node_t* node, rb_node_t* p) 
{
	return p->rb_left == node;
}

static inline bool rb_is_right(rb_node_t* node, rb_node_t* p) 
{
	return p->rb_right == node;
}


#endif
