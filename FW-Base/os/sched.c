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

TCB *create_task(taskfn handler, void *para, u8 cpu)
{
	TCB *task;


	return task;
}


void delete_task(TCB *task)
{

}

// set the task.state = SLEEP, thus the task is invisible to scheduler until someone resume it
// if yield self, this task will still run this round
//unmask the task to scheduler
void yield_task(TCB *task)
{
	set_task_state(task, TASK_STATE_SLEEP);
}


// set the task.state = ready, thus the task is visible to scheduler.
// the current task will not exit, the resume task can be chosen next round
// mask the task o scheduler
void resume_task(TCB *task)
{
	set_task_state(task, TASK_STATE_READY);
}


// schedule the task right now bypass scheduler
// it is for real-time urgent task which need process right now
void run_task(TCB *task)
{
	void *para;
	taskfn fn;

	assert(task->state == TASK_STATE_READY);

	task->state = TASK_STATE_RUNNING;

	fn = task->fn;
	para = task->para;

	// run it
	fn(para);
}

// 1.stop current task(the caller)
// 2.current not mask
void pend_task(TCB *task)
{
	// break current task

	//schedule();
}

// scheduler as a plugin, easy to switch according CFG
// scheduler pick task from ready list(bit 1 set)
// the pick policy is FIFO Prio-Base Fair .....
void bit_scan_scheduler(u8 cpu)
{
	TCB* task;

	//context_switch();

	for_each_rdy_task(cpu)
	{
		run_task(task);
	}

	// no ready Task now
}


static sched_obj_t XOS_Scheduler;

void scheduler_init(void)
{
	// select a scheduler
	XOS_Scheduler->name = "bit scan";
	XOS_Scheduler->schedule = bit_scan_scheduler;
}

///////////////////////////////////////////////////////////////////////////////////////////////
/*
 * os_start is used to start the multitasking process which lets XOS manages the tasks that 
 * you have created. Before you can call os_start(), you MUST have called os_init() to init 
 * and select a scheduler, beside you MUST have call create_task to created at least one task.
 *
 *
 * BEWARE: os_start() is not supposed to return.  If it does, that would be considered a fatal error.
 */

int os_start(void)
{
	XOS_Scheduler.schedule(HDC);

	return 0;
}

void os_init(void)
{
	scheduler_init();

	return;
}


