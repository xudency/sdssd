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
 * each CPU core support up to 64 task, 32 HW task + 32 FW task
 * the HW task priority is [0:31],  FW task priority is [32:63] 
 *
 */


task_sched_ctl_t gat_tasks_ctl_ctx;

// seperate defined, because this will placed in Core-CCM
static TCB *gat_hdc_task_array[MAX_TASKS_PER_CPU] = {NULL};
static TCB *gat_atc_task_array[MAX_TASKS_PER_CPU] = {NULL};
static TCB *gat_stc_task_array[MAX_TASKS_PER_CPU] = {NULL};
static TCB *gat_fdc_task_array[MAX_TASKS_PER_CPU] = {NULL};


TCB *this_task(u8 cpu, u8 prio)
{
	return gat_tasks_ctl_ctx.task_array[cpu] + prio;
}


/*bool task_presented(u8 cpu, u8 prio)
{
	u32 present = gat_tasks_ctl_ctx.task_presented[cpu];
	return bit_set(present, prio);
}
*/


/*void set_task_null(u8 cpu, u8 prio)
{
	*(gat_tasks_ctl_ctx.task_array[cpu] + prio) = NULL;
}*/

// taskfn demo


TCB *create_task(taskfn handler, void *para, u8 cpu, u8 prio, bool ready)
{
	TCB *tsk_array;
	TCB *task;

	if (prio >= MAX_TASKS_PER_CPU) {
		print_err("prio :%d is Invalid", prio);
		return NULL;
	}

	if (cpu >= CPU_NUM) {
		print_err("cpu :%d is Invalid", cpu);
		return NULL;
	}
    
	task = this_task(cpu, prio);
	if (task) {
		print_err("already exist");
        return NULL;
	}

	// task not exist
	task = kmalloc(sizeof(TCB), GFP_KERNEL)

	task->prio = prio;

	if (ready)
		task->state = TASK_STATE_READY;  //default state
	else
		task->state = TASK_STATE_SLEEP;
	
	task->fn = handler;
	task->prio = prio;
	
	return task;
}


void __delete_task(TCB *task)
{
	assert(task != NULL);

	kfree(task);
	task = NULL;
}

void delete_task(u8 cpu, u8 prio)
{
	TCB *task = this_task(cpu, prio);
	__delete_task(task);
}


// set the task.state = SLEEP, thus the task is invisible to scheduler until someone resume it
// if yield self, this task will still run this round
//unmask the task to scheduler
void __yield_task(TCB *task)
{
	assert(task != NULL);
	set_task_state(task, TASK_STATE_SLEEP);
}

void yield_task(u8 cpu, u8 prio)
{
	TCB *task = this_task(cpu, prio);
	__yield_task(task);
}

// set the task.state = ready, thus the task is visible to scheduler.
// the current task will not exit, the resume task can be chosen next round
// mask the task o scheduler
void __resume_task(TCB *task)
{
	assert(task != NULL);
	set_task_state(task, TASK_STATE_READY);
}

void resume_task(u8 cpu, u8 prio)
{
	TCB *task = this_task(cpu, prio);

	__resume_task(task);
}

// schedule the task right now bypass scheduler
// it is for real-time urgent task which need process right now
void __run_task(TCB *task)
{
	void *para;
	taskfn fn;

	assert(task != NULL);
	assert(task->state == TASK_STATE_READY);

	task->state = TASK_STATE_RUNNING;

	fn = task->fn;
	para = task->para;

	fn(para);
}

void run_task(u8 cpu, u8 prio)
{
	TCB *task = this_task(cpu, prio);
	__run_task(task);
}


// alway schedule the LSB task to run if the related bit is set
// when bit is clear, select the next bit to schedule
void lsb_prefer_scheduler(u8 cpu)
{
	int bit;
	TCB *task;
    TCB **tasks;
	register loop_cnt = 0;

loooop:
	// event register, a bit represent a event from HW
	u32 hw_event = readl(HW_EVENTF);
	// event sw indicator, a bit represent a event from FW internal
	u32 fw_event = readl(FW_EVENTF);

    tasks = gat_tasks_ctl_ctx.task_array[cpu];

	// handle hw event
	for_each_set_bit(bit, &hw_event, REG32_BITS) {
		task = tasks[bit];
		if (task && task->state == TASK_STATE_READY) run_task(task);
	}

	// handle fw event
	for_each_set_bit(bit, &fw_event, REG32_BITS) {
		bit_clear(fw_event, bit);
		task = tasks[bit];
		if (task && task->state == TASK_STATE_READY) run_task(task);
	}

	// XXX: CQE is process in ISR, here add a oppotinuty to process it
	if ((loop_cnt&0x3f) == 0) {
		resume_task(HDC, completion_prio);
	}

	goto loooop;
	//process_completion();
}


/*void fair_scheduler(u8 cpu)
{
	TCB* task;

loop:
	//context_switch();
	for_each_rdy_task(cpu) {
		run_task(task);
	}

	goto loop;
}*/


static sched_obj_t XOS_Scheduler;

// select a scheduler
void scheduler_init(char *name, sched_strategy sched_fn)
{
	//u8 cpu;
	
	//for_each_cpu(cpu) {
		//gat_tasks_ctl_ctx.task_presented[cpu] = 0;
	//}

	gat_tasks_ctl_ctx.task_array[HDC] = gat_hdc_task_array;
	gat_tasks_ctl_ctx.task_array[ATC] = gat_atc_task_array;
	gat_tasks_ctl_ctx.task_array[STC] = gat_stc_task_array;
	gat_tasks_ctl_ctx.task_array[FDC] = gat_fdc_task_array;

	XOS_Scheduler->name = name;
	XOS_Scheduler->schedule = sched_fn;
}

/*
 * os_start is used to start the multitasking process which lets XOS manages the tasks that 
 * you have created. Before you can call os_start(), you MUST have called os_init() to init 
 * and select a scheduler, beside you MUST have call create_task to created at least one task.
 *
 */
int os_start(void)
{
	// run independent in 4 core
	// TODO: how distrubute to 4 core
	XOS_Scheduler.schedule(HDC);

	XOS_Scheduler.schedule(ATC);
	
	XOS_Scheduler.schedule(STC);

	XOS_Scheduler.schedule(FDC);

	//BEWARE: the schedule will schedule a rdy task to run, if no ready 
	//or if no fw_event && sw_event, schedule will run a idle task
	//so its a loop forever, not supposed to return.  
	//If it does, that would be considered a fatal error.
	//panic();

	return 0;
}

void os_init(void)
{
	scheduler_init("lsb-prefre", lsb_prefer_scheduler);

	return;
}


