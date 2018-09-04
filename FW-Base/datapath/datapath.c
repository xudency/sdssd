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
 * read/write datapath, ingress of commmand handler
 *
 */

#include "nvme_spec.h"
#include "msg_fmt.h"

host_nvme_cmd_entry    gat_host_nvme_cmd_array[HOST_NVME_CMD_ENTRY_CNT]       = {{{0}}};


//struct Queue host_nvme_cmd_free_q;
struct Queue host_nvmd_cmd_pend_q;

// in process
//host_nvme_cmd_entry *current_host_cmd_entry = NULL;

//struct Queue host_nvme_wr_pend_q;
//struct Queue host_nvme_rd_pend_q;


// HW notify CPU method
//    1. HW interrupt ---> FW check Event register what happen ---> FW handle it
//    2. FW Polling check Event register what happen ---> if yes handle it


// when command process completion, this is the hook callback routine
int process_completion_task(void *para)
{
	// TODO: process all to Post CQE to Host

	yield_task(HDC, completion_prio);

	return 0;
}

int send_phif_cmd_req(phif_cmd_req *msg)
{
	if (port_is_available()) {
		memcpy(PHIF_CMD_REQ_SPM, msg, sizeof(phif_cmd_req)); 
		return 0;
	} else {
		/*break, CPU schedule other task*/
		return 1;
	}
}

int send_phif_cmd_cpl(phif_cmd_cpl *msg)
{
	if (port_is_available()) {
		memcpy(PHIF_CMD_CPL_SPM, msg, sizeof(phif_cmd_req)); 
		return 0;
	} else {
		/*break, CPU schedule other task*/
		return 1;
	}
}

// valid: bit0:QW_PI   bit1:QW_ADDR  bit2:QW_DATA
int send_phif_wdma_req(phif_wdma_req_mandatory *m, phif_wdma_req_optional *o, u8 valid)
{
	if (port_is_available()) {
		void *ptr = (void *)PHIF_WDMA_REQ_SPM;
		memcpy(ptr, m, PHIF_WDMA_REQ_M_LEN));
		ptr += PHIF_WDMA_REQ_M_LEN;

		if (valid & WDMA_QW_PI) {
			memcpy(ptr, o, QWORD_BYTES);
			ptr += QWORD_BYTES;
			o += QWORD_BYTES;
		}

		
		if (valid & WDMA_QW_ADDR) {
			memcpy(ptr, o, QWORD_BYTES);
			ptr += QWORD_BYTES;
			o += QWORD_BYTES;
		}

		if (valid & WDMA_QW_DATA) {
			memcpy(ptr, o, QWORD_BYTES*m->control.pld_qwn);
		}

		return 0;
	} else {
		/*break, CPU schedule other task*/
		return 1;
	}
}


// fwdata means FW define specific data, include sysdata and manage data
// host read, data in sram is exclusived
// both cbuff and host address is continuously
int wdma_read_fwdata_to_host(u64 host_addr, u64 cbuff_addr, u16 length)
{
	phif_wdma_req_mandatory m;

	// TODO: tag allocated?  bitmap, find_first_zero_bit, u32 support 32 command is enough
	u8 tag = hdc_alloc_cmdtag();
	
	msg_header_filled(&m.header, 6, MSG_NID_PHIF, MSG_NID_PHIF, MSGID_PHIF_WDMA_REQ, 
					  tag, HDC_EXT_TAG, MSG_NID_HDC, 0);

	m.control.blen = length;	// length
	m.control.pld_qwn = 1;		// only one QW_ADDR, because cbuff must continuous

	m.hdata_addr = host_addr;	//host address

	phif_wdma_req_optional o;
	o.cbuff_addr = cbuff_addr;   // cbuff address

	u8 valid = WDMA_QW_ADDR;

	return send_phif_wdma_req(&m, &o, valid);
}


host_nvme_cmd_entry *__get_host_cmd_entry(u8 tag)
{
	return &gat_host_nvme_cmd_array[tag];
}

host_nvme_cmd_entry *get_host_cmd_entry(u8 tag)
{
	host_nvme_cmd_entry *entry = __get_host_cmd_entry(tag);

	if (entry->state == WRITE_FLOW_STATE_INITIAL) {
		return entry;
	} else {
		print_err("tagid duplicated!!! this tag cmd state:%d", entry->state);
		// if it occur, it is a HE BUG
		//panic();
		return NULL;
	}
}


void saved_to_host_cmd_entry(host_nvme_cmd_entry *entry, hdc_nvme_cmd *cmd)
{
	entry->cmd_tag = cmd->header.tag;
	entry->sqid = cmd->header.sqid;

	entry->vfa = cmd->header.vfa;
	entry->port = cmd->header.port;
	entry->vf = cmd->header.vf;

	entry->sqe = cmd->sqe;
}

void host_cmd_phif_response(void)
{
	phif_cmd_rsp *rsp = (phif_cmd_rsp *)PHIF_CMD_RSP_SPM;

	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(rsp->tag);

	host_cmd_entry->sta_sct = rsp->sta_sct;
	host_cmd_entry->sta_sc = rsp->sta_sc;

	switch (host_cmd_entry->sqe.common.opcode) {
	case nvme_io_write:
		host_cmd_entry.state = WRITE_FLOW_STATE_HAS_PHIF_RSP;
		hdc_host_write_statemachine(host_cmd_entry);
		break;
	case nvme_io_read:
		/////////
		break;
	}

}

void setup_phif_cmd_req(phif_cmd_req *req, host_nvme_cmd_entry *host_cmd_entry)
{
	u8 flbas, lbaf_type;
	u16 lba_size;	// in byte
	ctrl_dw12h_t control;
	u64 start_lba = host_cmd_entry->sqe.rw.slba;
	u32 nsid = host_cmd_entry->sqe.rw.nsid;
	struct nvme_lbaf *lbaf;
	control.ctrl = host_cmd_entry->sqe.rw.control;
	struct nvme_id_ns *ns_info;
	host_nvme_cmd_entry *host_cmd_entry;

	// TODO: rate limiter, add this cmd to a pendimg list
	// rate limiter, throttle host write when GC too slowly

	//QW0
	req->header.cnt = 2; 	// phif_cmd_req, the length is fixed on 3 QW
	req->header.dstfifo = MSG_NID_PHIF;
	req->header.dst = MSG_NID_PHIF;
	req->header.prio = 0;
	req->header.msgid = MSGID_PHIF_CMD_REQ;

	// for host cmd, we support max 256 outstanding commands, tag 8 bit is enough
	// EXTAG no used, must keep it as 0, tag copy from hdc_nvme_cmd which is assign by PHIF
	req->header.tag = host_cmd_entry->cmd_tag;
	req->header.ext_tag = PHIF_EXTAG;
	req->header.src = MSG_NID_HDC;
	req->header.vfa = host_cmd_entry->vfa;
	req->header.port = host_cmd_entry->port;
	req->header.vf = host_cmd_entry->vf;
	req->header.sqid = host_cmd_entry->sqid;

	// TODO: Reservation check, if this is reservation conflict
	
	
	// TODO: TCG support, LBA Range, e.g. LBA[0 99] key1,  LBA[100 199] key2 .....
	req->header.hxts_mode = enabled | key; 
	
	ns_info = get_identify_ns(nsid);
	//QW1	
	req->cpa = start_lba / 1;	 // LBA - > CPA

	flbas = ns_info->flbas;
	lbaf_type = flbas & NVME_NS_FLBAS_LBA_MASK;
	lbaf = &ns_info->lbaf[lbaf_type];

	req->hmeta_size = lbaf->ms / 8;
	req->cph_size = lbaf->cphs;

	lba_size = 1 << lbaf->ds;
	if (lba_size == 512) {
		req->lb_size = 0;
	} else if (lba_size == 4096) {
		req->lb_size = 1;
	} else if (lba_size == 16384) {
		req->lb_size = 2;
	} else {
		print_err("LBA Size:%d not support", lba_size);
		//status.bits.sct = NVME_SCT_GENERIC;
		//status.bits.sc = NVME_SC_INVALID_FORMAT;
		//return status;
	}
	
	req->crc_en = 1;

	// PI in last8/first8 / type 0(disable)  1	2  3  
	req->dps = ns_info->dps;

	// DIX or DIF / format LBA size use which (1 of the 16) 
	req->flbas = flbas;

	req->cache_en = !control.bits.fua;

	// TODO:: according frequency, latency, stream to place host data to  HOSTBAND or WLBAND
	req->band_rdtype = HOSTBAND;  // FQMGR, 9 queue/die

	//QW2
	req->elba = (nsid<<32) | start_lba;

	// ask hw to export these define in .hdl or .rdl
}


void setup_phif_cmd_cpl(phif_cmd_cpl *cpl, host_nvme_cmd_entry *host_cmd_entry)
{
	// QW0
	cpl->header.cnt = 2;
	cpl->header.dstfifo = ;
	cpl->header.dst = MSG_NID_PHIF;
	cpl->header.prio = 0;
	cpl->header.msgid = MSGID_PHIF_CMD_REQ;
	cpl->header.tag = host_cmd_entry->cmd_tag;
	cpl->header.ext_tag = PHIF_EXTAG;
	cpl->header.src = MSG_NID_HDC;
	cpl->header.vfa = host_cmd_entry->vfa;
	cpl->header.port = host_cmd_entry->port;
	cpl->header.vf = host_cmd_entry->vf;
	cpl->header.sqid = host_cmd_entry->sqid;

	//QW 1 2
	//cpl->cqe = host_cmd_entry->cqe;

	cpl->cqe.result = 0; // this is command specified
	//cpl->cqe.sq_id = host_cmd_entry->sqid;
	//cpl->cqe.sq_head = YYYY;
	//cpl->cqe.command_id = host_cmd_entry->sqe.common.command_id;
	cpl->cqe.status = (host_cmd_entry->sta_sc<<1) | (host_cmd_entry->sta_sct<<9);
	// phase is filled by HW
}


// Process Host IO Comamnd
int handle_nvme_io_command(hdc_nvme_cmd *cmd)
{
	int res = 0;
	
	u8 opcode = cmd->sqe.common.opcode;

	switch (opcode) {
	case nvme_io_read:
		res = host_read_ingress(cmd);
		break;
	case nvme_io_write:
		res = host_write_ingress(cmd);
		break;
	case nvme_io_flush:
		break;
	default:
		res = -EINVAL;
		print_err("Opcode:0x%x Invalid", opcode);
		break;
	}

	return res;
}

// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd

// taskfn demo
int hdc_host_cmd_task(void *para)
{
	// HW fetch and fwd the Host CMD to a fix position
	// queue tail head is managed by HW
	//hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)para;
	hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)HDC_NVME_CMD_SPM;

	if (cmd->header.sqid == 0) {
		// admin queue, this is admin cmd
		return handle_nvme_admin_command(cmd);
	} else {
		// io command
		return handle_nvme_io_command(cmd);
	}

}

