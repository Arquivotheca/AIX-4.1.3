/* @(#)61	1.4  src/bos/kernext/sol/sol_proto.h, sysxsol, bos411, 9428A410j 7/16/91 11:56:50 */
#ifndef _H_SOL_PROTO
#define _H_SOL_PROTO
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: sol_proto.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
void	 imcs_diag_timer(int sla_id);
void	 imcs_sla_on(int sla_id, void * slaamp);
void	 init_auditor();
void	 imcs_sla_addr_off(int sla_id, slalinka_t sla_address);
void	 imcs_sla_off(int sla_id, int mode);
void	 cdd_enqueue(struct imcs_header *header);
void	 cdd_next(int sla_id);
void	 cdd_send_done(int sla_id);
void	 cdd_rcv_start(int sla_id);
ushort	 cdd_schn_int(int sla_id, uint subchannel, uint B_reg, uint op_code);
void	 requeue_to_schn(struct imcs_header *header);
void	 cdd_clean_sla(int sla_id);
void	 cdd_close();
void	 restart_sla(struct trb *t);
void	 restart_send(struct trb *t);
void	 start_ala_STUB(struct trb *t);
void	 restart_ala(struct trb *t);
void	 restart_sync(struct trb *t);
void	 restart_cont(struct trb *t);
void	 restart_recovery(struct trb *t);
void	 restart_recpoll(struct trb *t);
void	 restart_contscr(struct trb *t);
void	 sla_addr_unh_STUB(struct trb *t);
void	 restart_int(struct trb *t);
int	 slaih();
int	 sla_init();
void	 sla_close();
int	 sla_send(int sla_id);
void	 sla_rcv_rtsi(int sla_id);
struct	 imcs_header * imcs_send_done(struct imcs_header *header);
struct	 irq_block * rqctl_get();
void	 rqctl_hash(struct irq_block *irq);
void	 rqctl_put(struct irq_block *irq);
struct	 irq_block * rqctl_find(uint queue_id);
struct	 irq_block * rqctl_unhash(uint queue_id);
void	 rqctl_close();
void	 imcs_rcv_start();
uint	 imcs_rcv_done(int sla_id);
void	 imcs_rcv_fail(struct imcs_header * header);
int	 ipool_put(int pageno);
int	 ipool_get();
int	 ipool_count();
void	 ipool_init(int num_fr);
void	 sla_ala_completed(int sla_id, uchar sla_address, void *slaamp);
void	 sla_rcv_done(int sla_id, uchar source_address);
void	 sla_rtr_snd_done(int sla_id, uchar source_address);
void	 sla_rcv_error(int sla_id);
boolean_t sla_get_next_addr(int sla_id);
void	 sla_error(int sla_id, int mode);
void	 sla_address_error(int sla_id);
int	 req_imcs_hdr(short num_hdr);
caddr_t	 get_imcs_hdr();
void	 put_imcs_hdr(caddr_t vaddr);
short	 count_imcs_hdr();
void	 init_imcs_hdr();
void	 close_imcs_hdr();
int	 imcs_ctl(uint queue_id, int status);
int	 imcs_rethdr(uint queue_id, caddr_t header);
int	 imcs_addbuf(uint queue_id, int pageno);
int	 imcs_declare(ushort *queue_idp, void (*shli_ext)(), 
			int type, int num_hdrs);
int	 imcs_undeclare(int queue_id);
int	 imcs_sendmsg(struct imcs_header *header);
int 	 tagwords(caddr_t waddr, long length, caddr_t hdr_vaddr, int *starttag);

uint do_notsync(SLAIOP(slap) , int sla_id, uint s1_val, uint s2_val);
uint do_notident(SLAIOP(slap), int sla_id, uint s1_val, uint s2_val);
void sla_sdma_unh(SLAIOP(slap), int sla_id);
void sla_rdma_unh(SLAIOP(slap), int sla_id);
void sla_channel_check(SLAIOP(slap), int sla_id, uint s2_val);
int	cdd_return_msgs(int processor_id, short outcome,int qa);

void sla_terminate();
#endif _H_SOL_PROTO
