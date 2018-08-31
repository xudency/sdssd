/*
 * Copyright (C) 2018-2020 NET-Swift.
 * Initial release: Dengcai Xu <dengcaixu@net-swift.com>
 *
 * ALL RIGHTS RESERVED. These coded instructions and program statements are
 * copyrighted works and confidential proprietary information of NET-Swift Corp.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part.
 *
 * function: 
 * libqueue implement,add node in rear, get node from front default
 * also you can add new node in front by enqueue_front
 *
 */

#ifndef __LIB_QUEUE_H__
#define __LIB_QUEUE_H__

struct qnode {
	struct qnode *next;
};

struct Queue {
	u32 size;
	struct qnode *front;
	struct qnode *rear;
};

//#define QUEUE_TERMINATOR(q) 		((void*)(q))

static inline void queue_init(struct Queue *queue)
{
	queue->size = 0;
	queue->front = queue->rear = NULL;
}

static inline int queue_empty(struct Queue *queue)
{
	return queue->size==0;
}

static inline u32 queue_size(struct Queue *queue)
{
	return queue->size;
}

static inline struct qnode *peek_queue(struct Queue *queue)
{
	return queue->front;
}

// queue add new in rear default
static inline void enqueue(struct Queue *queue, struct qnode *node)
{
	node->next=NULL;
	
	if(queue->front)
		queue->rear->next=node;
	else
		queue->front=node;  // Queue is empty
	
	queue->rear=node;
	queue->size++;
}

// get from queue front
static inline struct qnode *dequeue(struct Queue *queue)
{
	if(queue_empty(queue))
		return NULL;
	
	struct qnode *node = queue->front;
	node->next = NULL;
	
	if(queue->front == queue->rear)
		queue->rear = NULL;
	
	queue->front = queue->front->next;
	queue->size--;
	return node;
}

// it may used when a node dequeue, but we want to re-enqueue it
static inline void enqueue_front(struct Queue *queue, struct qnode *node)
{
	node->next = queue->front;

	if (queue->rear == NULL)
		queue->rear = node;
	
	queue->front = node;
	queue->size++;
}

// merge queue1 to queue2 and init queue1
static inline void merge_queue(struct Queue *queue2, struct Queue * queue1)
{
	if (queue1->front == NULL) {
		return;  //queue1 is empty, nothing to merge
	}

    //src_queue->rear->next = NULL;
    queue2->rear->next = queue1->front;
    queue2->rear = queue1->rear;
    queue2->size += queue1->size;

    queue_init(queue1);
}


#endif

