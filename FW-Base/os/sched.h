#ifndef _OS_SCHED_H_
#define _OS_SCHED_H_

#define CPU_NUM 		    4   // we use ARC HS36 X4 core

#define HDC 				0
#define ATC 				1
#define STC 				2
#define FDC 				3

#define TASK_STATE_SLEEP    0
#define TASK_STATE_READY    1
#define TASK_STATE_RUNNING  2

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
typedef struct sched_obj {
	char name[16];    //so please don't take a too long name	
	void (*schedule) (u8 cpu);
} sched_obj_t;

typedef void (*taskfn)(void *);

// Task Control Block
typedef struct task_ctl_block {
	u8 tid;
	u8 state;     //runing ready sleep
	u8 native_prio;
	u8 tmp_prio; // temporary changed  

	taskfn fn;
	void *para;

	// XXX: use list or heap ?
	struct list_node node;
	struct task_ctl_block *parent;
} TCB;



static inline set_task_state(TCB *task, u8 val)
{
	task->state = val;
}


#endif

