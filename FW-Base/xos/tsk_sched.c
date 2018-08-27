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
	register loop_cnt = 0;

loooop:
	// Any HW Task Priority is higher than all FW Task.
	handle_hw_event(cpu);
	
	handle_fw_event(cpu);

	if (loop_cnt & 0xf)
		resume_task(cpu, completion_prio);

	goto loooop;
}

static sched_obj_t XOS_Scheduler;

// select a scheduler
void scheduler_init(char *name, sched_strategy sched_fn)
{
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


// HDC is responsibile for
//   1. handle NVMe Command(admin/io) from Host
//   2. for read, in completion, move data from CBUFF to Host
int hdc_tasks_create(void)
{
	create_task(hdc_host_cmd_task, NULL, HDC, 0, true);
	
	create_task(process_completion_task, NULL, HDC, 32, false);

	return 0;
}

// ATC is responsibile for
//   1. MAP, Address Translate, slot-->PPA
//   2. CheckPoint sace/restore
//   3. 
int atc_tasks_create()
{

	return 0;
}

// STC ss responsibile for 
//   1. recycle
//   2. recovery
//   3. Root FS(Primary info)
int stc_tasks_create()
{


	return 0;
}

// FDC is responsibile for
//   1. CBUFF Flush Management
//   2. Media Error-Handle
int fdc_tasks_create()
{


	return 0;
}


void os_task_create()
{
	hdc_tasks_create();
	
	atc_tasks_create();
	
	stc_tasks_create();
	
	fdc_tasks_create();
}

