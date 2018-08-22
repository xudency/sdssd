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
 * FW has lots Task, to facilitate manage these Task, a scheduler is needed
 *
 */


struct list_head g_task_list[CPU_NUM];

// taskfn demo
void gc_read_task_fn(void *para)
{
	return;
}

TCB *create_task(taskfn handler, void *para)
{
	TCB *task;


	return task;
}


void delete_task(TCB *task)
{

}

// set the task.state = SLEEP, thus the task is invisible to scheduler until someone resume it
// if yield self, this task will still run this round
void yield_task(TCB *task)
{
	task->state = TASK_STATE_SLEEP;
}


// set the task.state = ready, thus the task is visible to scheduler.
// the current task will not exit, the resume task can be chosen next round 
void resume_task(TCB *task)
{
	task->state = TASK_STATE_READY
}


// schedule it right now bypass scheduler, the current task will exit right now
// it is for real-time urgent task which need process right now
void schedule_task(TCB *task)
{
	current_task = TASK_STATE_READY;

	//resume_task(task);
	// highest_task(task);

	schedule();

}

// current task sleep right now, yield CPU to other task 
void sleep_task(TCB *task)
{
	yield_task(task);

	schedule();
}

// scheduler as a plugin, easy to switch according CFG
// scheduler pick task from ready list(bit 1 set)
// the pick policy is FIFO Prio-Base Fair .....
void schedule(u8 cpu)
{
	TCB* task;
	void *para;
	taskfn fn;

	context_switch();

	for_each_rdy_task(cpu)
	{
		fn = task->fn;
		para = task->para;

		fn(para);
	}

	// no ready Task now
	
}


void schedule_init(void)
{
	// ops, object-orited design
}

