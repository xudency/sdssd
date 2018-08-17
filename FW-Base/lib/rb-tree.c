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


// closed_blk_rbtree sort by sequence
// free_blk_rbtree sort by PE Cycle
// obsoleted_blk_tree sort by PE Cycle


rbtree_rotate_left(rb_root_t *root, rb_node_t *x)
{
	rb_node_t *y = x->right;
}



