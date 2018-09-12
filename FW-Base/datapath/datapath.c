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
 * HW notify CPU method
 *    1. HW interrupt ---> FW check Event register what happen ---> FW handle it
 *    2. FW Polling check Event register what happen ---> if yes handle it
 *
 */

#include "nvme_spec.h"
#include "msg_fmt.h"
#include "datapath.h"

// host coming nvme command
host_nvme_cmd_entry    gat_host_nvme_cmd_array[HOST_NVME_CMD_ENTRY_CNT]       = {{{0}}};
struct Queue host_nvme_cmd_pend_q;

// phif_cmd_cpl NOT define in stack, to prevenrt re-constructed in statemachine
phif_cmd_cpl	gat_host_cmd_cpl_array[HOST_NVME_CMD_ENTRY_CNT];
struct Queue host_nvme_cpl_pend_q;



// fw internal command
fw_cmd_ctl_ctx gat_fw_itnl_cmd_ctl;

phif_wdma_req gat_fw_wdma_req_array[FW_WDMA_REQ_CNT];
struct Queue fw_wdma_req_pend_q;

phif_wdma_req gat_fw_rdma_req_array[FW_RDMA_REQ_CNT];
struct Queue fw_rdma_req_pend_q;


u16 fw_alloc_itnl_tag(u8 cmd_types)
{
	u16 start_bit, last_bit;

	switch (cmd_types) {
	case FW_WDMA_REQ:
		start_bit = 0; 
		last_bit = FW_WDMA_REQ_CNT-1;
		break;
	case FW_RDMA_REQ:
		start_bit = FW_WDMA_REQ_CNT; 
		last_bit = FW_WDMA_REQ_CNT + FW_RDMA_REQ_CNT-1;
		break;
	}
	
	// TODO: tag allocated?  bitmap, find_first_zero_bit_range 
	return find_first_zero_bit_range(gat_fw_itnl_cmd_ctl.itnl_tag, start_bit, last_bit);
}

void fw_free_itnl_tag(u16 itnl_tag)
{
	bit_clear(gat_fw_itnl_cmd_ctl.itnl_tag, itnl_tag);
}

u16 itnl_tag_to_host_tag(u16 itnl_tag)
{
	fw_internal_cmd_entry *fw_cmd_entry = __get_fw_cmd_entry(itnl_tag);

	return fw_cmd_entry->host_tag;
}

void *fw_cmd_get_ctx_fn(u16 itnl_tag, fw_cmd_callback *fn)
{
	void *ctx;
	fw_internal_cmd_entry *fw_cmd = __get_fw_cmd_entry(itnl_tag);

	*fn = fw_cmd->fn;
	ctx = fw_cmd->ctx;

	return ctx;
}

/*
the_bind_host_cmd_entry(void *itnl)
{
	phif_wdma_rsp *rsp = (phif_wdma_rsp *)para;
	u8 host_tag = itnl_tag_to_host_tag(rsp->tag);
	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(host_tag);
}*/

// when command process completion, this is the hook callback routine
int process_completion_task(void *para)
{
	// TODO: process all to Post CQE to Host

	yield_task(HDC, completion_prio);

	return 0;
}

void msg_header_filled(struct msg_qw0 *header, u8 cnt, u8 dstfifo, u8 dst, 
							u8 msgid, u8 tag, u8 ext_tag, u8 src, u8 sriov)
{
	header->cnt = cnt; 	// phif_cmd_req, the length is fixed on 3 QW
	header->dstfifo = dstfifo;
	header->dst = dst;
	header->prio = 0;
	header->msgid = msgid;
	header->tag = tag;
	header->ext_tag = ext_tag;
	header->src = src;
	header->vfa = sriov & 0x1;
	header->port = (sriov >>1) & 0x1;
	header->vf = (sriov >>2) & 0xf;
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
		memcpy(PHIF_CMD_CPL_SPM, msg, sizeof(phif_cmd_cpl)); 
		return 0;
	} else {
		/*break, CPU schedule other task*/
		return 1;
	}
}

// phif_rwdma_req/rsp is variable-length
int send_phif_wdma_req(phif_wdma_req *req, u16 valid)
{
	if (port_is_available()) {
		void *port = (void *)PHIF_WDMA_REQ_SPM;
		void  *msg = (void *)req;
		memcpy(port, msg, PHIF_RWDMA_REQ_M_LEN));
		port += PHIF_RWDMA_REQ_M_LEN;
		msg += PHIF_RWDMA_REQ_M_LEN;

		if (valid & RWDMA_QW_PI) {
			memcpy(port, msg, QWORD_BYTES);
			port += QWORD_BYTES;
			msg += QWORD_BYTES;
		}
		
		if (valid & RWDMA_QW_ADDR) {
			memcpy(port, msg, QWORD_BYTES);
			port += QWORD_BYTES;
			msg += QWORD_BYTES;
		}

		if (valid & RWDMA_QW_DATA0) {
			memcpy(port, msg, QWORD_BYTES);
			port += QWORD_BYTES;
			msg += QWORD_BYTES;
		}

		if (valid & RWDMA_QW_DATA1) {
			memcpy(port, msg, QWORD_BYTES);
		}

		return 0;
	} else {
		/*break, CPU schedule other task*/
		return 1;
	}
}

// a piece of  data, mode 1/2, 0 is used by HW accelerated
void fw_send_rdma_req(u64 host_addr, u64 cbuff_addr, u16 length, u16 host_tag, 
						   fw_cmd_callback handler, void *ctx)
{
	u16 itnl_tag = fw_alloc_itnl_tag();
	phif_rdma_req *req = __get_fw_rdma_req_entry(itnl_tag);
	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(host_tag);
	fw_internal_cmd_entry *itnl_cmd_entry = __get_fw_cmd_entry(itnl_tag);
	struct msg_qw0 *header = (struct msg_qw0 *)req;
	
	itnl_cmd_entry->msgptr = req;
	itnl_cmd_entry->host_tag = host_tag;
	itnl_cmd_entry->fn = handler;
	itnl_cmd_entry->ctx = ctx;

	msg_header_filled(header, 6, MSG_NID_PHIF, MSG_NID_PHIF, MSGID_PHIF_RDMA_REQ, 
					  itnl_tag, HDC_EXT_TAG, MSG_NID_HDC, 0);
	req->band = FW_PRIVATE_DATA;
	req->cache_en = 0;

	req->control.blen = length;	// length
	req->control.addr_qwen = 1;		// mode 0/1, there is QW_ADDR
	req->hdata_addr = host_addr;	// host address
	req->cbuff_addr = cbuff_addr;  // cbuff address

	u16 valid = RWDMA_QW_ADDR;
	
	if(send_phif_rdma_req(req, valid)) {
		enqueue(&fw_rdma_req_pend_q, &itnl_cmd_entry->next);
	}

	host_cmd_entry->ckc += 1;

	return;
}

void rdma_host_dptr_to_cbuff(union nvme_data_ptr dptr, u64 cbuff, u16 length, 
									u16 host_tag, fw_cmd_callback handler)
{
	// TODO:: parse prp1 prp2(list)   SGL
}


// move data from Cbuff to Host memory via PHIF WDMA
// beware: one phif_wdma_req only can move data that
// both src:cbuff and dest:host address is continuously
void fw_send_wdma_req(u64 host_addr, u64 cbuff_addr, u16 length, u16 host_tag, 
						  fw_cmd_callback handler, void *ctx)
{

	u16 itnl_tag = fw_alloc_itnl_tag();
	phif_wdma_req *req = __get_fw_wdma_req_entry(itnl_tag);	
	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(host_tag);
	fw_internal_cmd_entry *itnl_cmd_entry = __get_fw_cmd_entry(itnl_tag);
	struct msg_qw0 *header = (struct msg_qw0 *)req;
	
	itnl_cmd_entry->msgptr = req;
	itnl_cmd_entry->host_tag = host_tag;
	itnl_cmd_entry->fn = handler;
	itnl_cmd_entry->ctx = ctx;

	msg_header_filled(header, 6, MSG_NID_PHIF, MSG_NID_PHIF, MSGID_PHIF_WDMA_REQ, 
					  itnl_tag, HDC_EXT_TAG, MSG_NID_HDC, 0);

	req->control.blen = length;	   // length
	req->control.pld_qwn = 1;	   // only one QW_ADDR, because cbuff must continuous
	req->hdata_addr = host_addr;   // host address
	req->cbuff_addr = cbuff_addr;  // cbuff address

	u16 valid = RWDMA_QW_ADDR;

	if(send_phif_wdma_req(req, valid)) {
		enqueue(&fw_wdma_req_pend_q, &itnl_cmd_entry->next);
	}

	host_cmd_entry->ckc += 1;

	return;
}

// cbuff address must continuously
void wdma_cbuff_to_host_dptr(union nvme_data_ptr dptr, u64 cbuff, u16 length, 
									u16 host_tag, fw_cmd_callback handler, void *ctx)
{
	u64 prp1 = dptr.prp1;
	u64 prp2 = dptr.prp2;
	u64 prp1_offset = page_offset(prp1);
	//u16 length = SZ_4K - prp1_offset;
	
	// Concer case handle 1.cmdtag alloc fail  2.wdma_req_spm busy
	if (!prp1_offset) {
		// PRP1 is 4K align
		//host_cmd_entry->ckc = 1;
		fw_send_wdma_req(prp1, cbuff, SZ_4K, host_tag, handler, ctx);
	} else {
		// PRP1 not 4K align, PRP2 is used
		//host_cmd_entry->ckc = 2;
		fw_send_wdma_req(prp1, cbuff, length, host_tag, handler, ctx);
		fw_send_wdma_req(prp2, cbuff+length, prp1_offset, host_tag, handler, ctx);
	}

	// TODO: prp2 list analysis

	// TODO:: SGL analysis
}


void save_in_host_cmd_entry(host_nvme_cmd_entry *entry, hdc_nvme_cmd *cmd)
{
	entry->cmd_tag = cmd->header.tag;
	entry->sqid = cmd->header.sqid;

	entry->vfa = cmd->header.vfa;
	entry->port = cmd->header.port;
	entry->vf = cmd->header.vf;

	entry->sqe = cmd->sqe;
}

// write/read LBA(HW accelerate) command complete
void phif_cmd_response_to_hdc(void)
{
	phif_cmd_rsp *rsp = (phif_cmd_rsp *)PHIF_CMD_RSP_SPM;

	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(rsp->tag);

	host_cmd_entry->sta_sct = rsp->sta_sct;
	host_cmd_entry->sta_sc = rsp->sta_sc;


	switch (host_cmd_entry->sqe.common.opcode) {
	case nvme_io_write:
		host_cmd_entry.state = WRITE_FLOW_STATE_HAS_PHIF_RSP;
		write_datapath_hdc(host_cmd_entry);
		break;
	case nvme_io_read:
		host_cmd_entry.state = READ_FLOW_STATE_HAS_PHIF_RSP;
		read_datapath_hdc(host_cmd_entry);
		break;
	}

}


int host_cmd_rwdma_completion(void *para)
{
	// when data has move to host buffer, what else you want do....
	return 0;
}


// 0-RDMA   1-WDMA
void phif_rwdma_response_to_hdc(bool rw)
{
	u8 itnl_tag, status;
	//void *ptr;

	if (rw) {
		phif_wdma_rsp *rsp = (phif_wdma_rsp *)PHIF_WDMA_RSP_SPM;
		itnl_tag = rsp->tag;
		status = rsp->status;
		//ptr = (void *)rsp;
	} else {
		phif_rdma_rsp *rsp = (phif_rdma_rsp *)PHIF_RDMA_RSP_SPM;		
		itnl_tag = rsp->tag;
		status = rsp->status;
		//ptr = (void *)rsp;
	}

	fw_internal_cmd_entry *fw_cmd_entry = __get_fw_cmd_entry(itnl_tag);
	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(fw_cmd_entry->host_tag);

	// any chunk error, this host command is error
	if (status) {
		host_cmd_entry->sta_sc = NVME_SC_INTERNAL;
		// reset HW
	}
	
	if (--host_cmd_entry->ckc) {
		fw_cmd_callback fn;
		void *ctx;
		ctx = fw_cmd_get_ctx_fn(itnl_tag, &fn);
		fn(ctx);
	
		// prp1 part and prp2 part all response, structured a phif_cmd_cpl to PHIF
		phif_cmd_cpl *cpl = __get_host_cmd_cpl_entry(fw_cmd_entry->host_tag);
		setup_phif_cmd_cpl(cpl, host_cmd_entry);
	
		if (send_phif_cmd_cpl(cpl)) {
			enqueue_front(&host_nvme_cpl_pend_q, &host_cmd_entry->next);
		}
	}

	fw_free_itnl_tag(itnl_tag);

	//dequeue(fw_wdma_req_pend_q);

	return;

}

// host datapath prepare phif_cmd_req
void setup_phif_cmd_req(phif_cmd_req *req, host_nvme_cmd_entry *host_cmd_entry)
{
	u8 flbas, lbaf_type;
	u16 lba_size;	// in byte
	rw_ctrl_t control;
	u8 opcode = host_cmd_entry->sqe.rw.opcode;
	u64 start_lba = host_cmd_entry->sqe.rw.slba;
	u32 nsid = host_cmd_entry->sqe.rw.nsid;
	struct nvme_lbaf *lbaf;
	control.ctrl = host_cmd_entry->sqe.rw.control;
	struct nvme_id_ns *ns_info;
	host_nvme_cmd_entry *host_cmd_entry;

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
	req->cpa = start_lba / 1;
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
	}
	
	req->crc_en = 1;
	req->dps = ns_info->dps;  // PI type and PI in meta pos, first or last 8B
	req->flbas = flbas;       // DIX or DIF / format LBA size use which (1 of the 16) 
	req->cache_en = !control.bits.fua;

	// TODO:: write directive, 
	if (opcode == nvme_io_read) {
		//Latency Control via Queue schedule in FQMGR(9 queue per die)
		
		dsm_dw13_t dsmgmt;
		dsmgmt.dw13 = host_cmd_entry->sqe.rw.dsmgmt;
		u8 access_lat = dsmgmt.bits.dsm_latency;
		switch (access_lat) {
		case NVME_RW_DSM_LATENCY_NONE:
			// XXX: if no latency info provide, add to which queue ?
			req.band_rdtype = FQMGR_HOST_READ1;
			break;	
		case NVME_RW_DSM_LATENCY_IDLE:
			req.band_rdtype = FQMGR_HOST_READ2;
			break;
		case NVME_RW_DSM_LATENCY_NORM:
			req.band_rdtype = FQMGR_HOST_READ1;
			break;
		case NVME_RW_DSM_LATENCY_LOW:
			req.band_rdtype = FQMGR_HOST_READ0;
			break;
		default:
			print_err("access latency Invalid");
			return - EINVAL;
		}
	} else if (opcode == nvme_io_write) {		
		req->band_rdtype = HOSTBAND;
	} else {
		//...
	}
	
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

	cpl->cqe.result = host_cmd_entry->result; // this is command specified
	cpl->cqe.status = (host_cmd_entry->sta_sc<<1) | (host_cmd_entry->sta_sct<<9);
	// phase is filled by HW
}


host_nvme_cmd_entry *get_next_host_cmd_entry(void)
{
	host_nvme_cmd_entry *host_cmd_entry = NULL;

	if (queue_empty(&host_nvme_cpl_pend_q)) {
		host_cmd_entry = dequeue(&host_nvme_cmd_pend_q);
	} else {
		host_cmd_entry = dequeue(&host_nvme_cpl_pend_q);
	}

	return host_cmd_entry;
}


// Process Host IO Comamnd
int handle_nvme_io_command(host_nvme_cmd_entry *host_cmd_entry)
{
	u8 opcode = host_cmd_entry->sqe.common.opcode;
	u64 start_lba = host_cmd_entry->sqe.rw.slba;
	u16 nlb = host_cmd_entry->sqe.rw.length;
	u32 nsid = host_cmd_entry->sqe.rw.nsid;
	u8 tag = host_cmd_entry->cmd_tag;	
	
	// para check
	if ((start_lba + nlb) > MAX_LBA) {
		print_err("the write LBA Range[%lld--%lld] exceed max_lba:%d", start_lba, start_lba+nlb, MAX_LBA);
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_LBA_RANGE);
		return NVME_REQUEST_INVALID;	
	}

	if (nvme_nsid_valid(nsid)) {
		print_err("NSID:%d is Invalid", nsid);		
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_INVALID_NS);
		return NVME_REQUEST_INVALID;	
	}

	// maximum data transfer size MDTS

	enqueue(&host_nvme_cmd_pend_q, host_cmd_entry->next);

	if (opcode == nvme_io_read) {
		host_cmd_entry->state = READ_FLOW_STATE_QUEUED;
	} else if (opcode == nvme_io_write) {
		host_cmd_entry->state = WRITE_FLOW_STATE_QUEUED;
	}

	host_cmd_entry = get_next_host_cmd_entry();
	
	assert(host_cmd_entry);
	
	switch (opcode) {
	case nvme_io_read:
		read_datapath_hdc(host_cmd_entry);
		break;
	case nvme_io_write:
		write_datapath_hdc(host_cmd_entry);
		break;
	case nvme_io_flush:
		break;
	case nvme_io_compare:
		break;
	default:
		print_err("Opcode:0x%x Invalid", opcode);
		set_host_cmd_staus(host_cmd_entry, NVME_SCT_GENERIC, NVME_SC_INVALID_OPCODE);
		return NVME_REQUEST_INVALID;
	}

	return NVME_REQUEST_PROCESSING;    // host command in-pocess 1.wait phif_cmd_rsp,  2.wait SPM available
}

// when host prepare a SQE and submit it to the SQ, then write SQTail DB
// Phif fetch it and save in CMD_TABLE, then notify HDC by message hdc_nvme_cmd
void hdc_host_cmd_task(void *para)
{
	int res = 0;
	hdc_nvme_cmd *cmd = (hdc_nvme_cmd *)HDC_NVME_CMD_SPM; //(hdc_nvme_cmd *)para
	u8 tag = cmd->header.tag;
	
	// tag is assigned by PHIF, it can guarantee this tag is free,
	host_nvme_cmd_entry *host_cmd_entry = __get_host_cmd_entry(tag);

	save_in_host_cmd_entry(host_cmd_entry, cmd);

	if (cmd->header.sqid == 0) {
		res = handle_nvme_admin_command(host_cmd_entry);
	} else {
		res = handle_nvme_io_command(host_cmd_entry);
	}

	// this nvme request has fwd to HW for further process
	// 1.HW in-processing, FW wait response  
	// 2.As SPM not available, FW has push it in a queue
	if (res == NVME_REQUEST_PROCESSING)
		return;

	
	// complete this nvme request directly
	// 1.this NVMe Command is Invalid
	// 2.this NVMe Command has handle finish, don't need wait HW response
	phif_cmd_cpl *cpl = __get_host_cmd_cpl_entry(tag)
	setup_phif_cmd_cpl(cpl, host_cmd_entry);
	if (send_phif_cmd_cpl(cpl)) {
		enqueue_front(host_nvme_cpl_pend_q, &host_cmd_entry->next);
	}

	return;

}

