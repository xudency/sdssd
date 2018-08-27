#ifndef _OS_SCHED_H_
#define _OS_SCHED_H_

#define CPU_NUM 		    4   // we use ARC HS36 X4 core

#define HDC 				0
#define ATC 				1
#define STC 				2
#define FDC 				3

#define MAX_TASKS_PER_CPU   64    // 32hw + 32fw

#define TASK_STATE_SLEEP    0
#define TASK_STATE_READY    1
#define TASK_STATE_RUNNING  2

#define for_each_cpu(cpu) \
        for(cpu = 0; cpu < CPU_NUM; cpu++)


// this task is trigger by HW and Host
enum hw_task {
	phif_nvme_cmd_task		= 0x00,
	phif_power_on_task		= 0x01,
	phif_power_down_task	= 0x02,
	nte_power_fail_task		= 0x03,
	fif_program_fail_task	= 0x04,
	fif_read_uecc_task		= 0x05,
	fif_erase_fail_task		= 0x06,
};

// this task is trigger by FW-self
enum fw_task {
	crash_recovery_task		= 0x20,   // when fw detect unsafe power down
	atc_assign_ppa_task		= 0x21,	  // when allocate PPA to fill WPB
	stc_recycle_task		= 0x22,   // when free blk < 10
	stc_ckpt_task			= 0x23,   // when Log Page has flushed 200
	stc_scrub_task			= 0x24,   // timer, periodicity set this task ready
	fdc_flushed_task		= 0x25,   // when cache full, flush it CBUFF->NAND
};

// some other task will run in ISR, OS don't need crate task for it



// scheduler type dependon the schedule algorithm
// we only implement a very simple bit scan scheduler
// but it is easy to change to use other, just as a plugin
typedef void (*sched_strategy) (u8 cpu);

typedef struct sched_obj {
	char name[16];    //so please don't take a too long name	
	void (*schedule) (u8 cpu);
	sched_strategy schedfn;
} sched_obj_t;

typedef int (*taskfn)(void *);

// Task Control Block
typedef struct task_ctl_block {
	//u8 tid;
	u8 state;     //runing ready sleep
	u8 prio;	  // the priority is unique, bitmap index, 0-highest  31-lowest 
	u16 rsvd;     // keep 4B align

	taskfn fn;
	void *para;

	// XXX: use list or heap or array is OK ?
	//struct list_node node;
	//struct task_ctl_block *parent;
} TCB;


typedef struct task_sched_ctl {
	//u32 task_presented[CPU_NUM];
	TCB **task_array[CPU_NUM];
} task_sched_ctl_t;

static inline set_task_state(TCB *task, u8 state)
{
	task->state = state;
}

extern task_sched_ctl_t gat_tasks_ctl_ctx;

extern int os_start(void);
extern void os_init(void);


#endif

