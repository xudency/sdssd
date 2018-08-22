#ifndef _OS_SCHED_H_
#define _OS_SCHED_H_

#define CPU_NUM 		    4   // we use ARC HS36 X4 core

#define TASK_STATE_SLEEP    0
#define TASK_STATE_READY    1
#define TASK_STATE_RUNNING  2


typedef void (*taskfn)(void *);

// Task Control Block
typedef struct task_ctl_block {
	u8 tid;
	u8 state;     //runing ready sleep
	u8 native_prio;
	u8 tmp_prio; // temporary changed  

	taskfn fn;
	void *para;

	struct list_node node;   //next prev
	struct task_ctl_block *parent;
} TCB;


void schedule_init(void);

#endif

