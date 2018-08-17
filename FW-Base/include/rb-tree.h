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
} rb_root_t;


#define RB_ROOT (rb_root_t) {RB_NODE_NULL, 0}
#define RB_NODE (rb_node_t) {RB_RED, RB_NODE_NULL, RB_NODE_NULL, RB_NODE_NULL}


/*
#define rb_parent(x)   ((rb_node_t *)((x)->parent))
#define rb_left(x)     ((rb_node_t *)((x)->left))
#define rb_right(x)    ((rb_node_t *)((x)->right))
#define rb_color(x)    ((x)->color)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))*/

//rb_to_bmi()


static inline void RB_NODE_INIT(rb_node_t *node)
{
	node->left = node->right = node->parent = RB_NODE_NULL;
}

static inline void RB_ROOT_INIT(rb_root_t *root)
{
	root->root = RB_NODE_NULL;
	root->count = 0;
}



#endif
