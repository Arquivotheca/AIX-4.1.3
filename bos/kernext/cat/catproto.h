/* @(#)62	1.12  src/bos/kernext/cat/catproto.h, sysxcat, bos411, 9428A410j 9/17/92 10:41:20 */
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef  _H_CATPROTO

#ifdef __STDC__
#define	P(s) s
#else
#define P(s) ()
#endif


struct cdtbl *cat_cdt_func P((void));

void cat_add_cdt P((
	register char *name, 
	register char *ptr, 
	register int len));

void cat_del_cdt P((
	register char *name, 
	register char *ptr, 
	register int len));

int catclose P((
	dev_t dev, 
	chan_t chan));

int catconfig P((
	dev_t dev, 
	int cmd, 
	struct uio *uiop));

void shutdown_adapter P((struct ca *ca));

int cat_init_dev P((struct ca *ca));

int cat_term_dev P((struct ca *ca));

struct ca *catalloc P((void));

int catfree P((struct ca *ca ));

struct ca *catget P((dev_t dev ));

int cat_offlvl P((struct intr *ofl_ptr));

void clean_queue P((struct ca *ca));

int catintr P((struct ca *ca));

void ginny P((struct intr *wds_ptr));

void resource_timeout P((struct trb *trub));

void handle_timer P((struct ca *ca));

void garbage_collector P((struct ca *ca));

int catioctl P((dev_t devno,
	int cmd,
	int arg, 
	ulong devflag, 
	chan_t chan, 
	int ext ));

int reset_adapter P((struct ca *ca));

int do_dnld P((struct ca *ca,
	open_t *openp, 
	int arg,
	ulong devflag));

int cio_start P((
	struct ca *ca, 
	open_t *openp, 
	int arg, 
	ulong devflag, 
	chan_t chan));

int cio_halt P((
	struct ca *ca, 
	open_t *openp, 
	int arg, 
	ulong devflag, 
	chan_t chan));

int cio_query P((
	struct ca *ca, 
	open_t *openp, 
	int arg, 
	ulong devflag, 
	chan_t chan));

int cio_get_stat P((
	struct ca *ca, 
	open_t *openp, 
	int arg, 
	ulong devflag, 
	chan_t chan));

int catinfo P((
	struct ca *ca, 
	caddr_t arg, 
	ulong devflag));

int do_sram_cmd P((
	register struct ca *ca, 
	struct cat_rw_sram *cmd));

int do_pos_cmd P((
	struct ca *ca, 
	struct cat_pos_acc *cmd));

int do_setadap P((struct ca *ca, 
	struct cat_set_adap *args, 
	int devflag));

int reset_sub P((
	struct ca *ca, 
	open_t *openp, 
	struct session_blk *arg, 
	int devflag));

int reset_all P((struct ca *ca));

int rw_sram P((
	struct ca *ca, 
	struct cat_rw_sram *arg));

int do_diag P((
	struct ca *ca, 
	struct cat_run_diag *rd));

int adap_info P((
	struct ca *ca, 
	struct cat_adap_info *ci, 
	dev_t devflag));

int load_cu P((
	struct ca *ca, 
	struct cat_cu_load *arg, 
	dev_t dev_flag));

int do_cutable P((
	struct ca *ca, 
	uchar *uaddr, 
	ulong len, 
	ulong cutype, 
	ulong overwrite));

int do_setsub P((
	struct ca *ca, 
	struct cat_set_sub *setsub_cmd));

int do_stopsub P((
	struct ca *ca, 
	struct session_blk *sess_blk));

int catmpx P((
	dev_t dev, 
	int *chanp, 
	char *channame ));

int catopen P((
	dev_t dev, 
	ulong mode, 
	chan_t chan, 
	struct kopen_ext *ext));

void free_open_struct P((
	struct ca *ca, 
	open_t *openp));

int cat_pos_read P((struct pos_rd *loc));

int cat_pos_write P((struct pos_wr *loc));

int cat_read P((struct mem_acc *loc));

int cat_readrc P((
	struct mem_acc *loc, 
	int action, 
	struct pio_except *infop));

int cat_pos_readrc P((
	struct mem_acc *loc, 
	int action, 
	struct pio_except *infop));

int cat_write P((struct mem_acc *loc));

int cat_writerc P((
	struct mem_acc *loc, 
	int action, 
	struct pio_except *infop));

int cat_pos_writerc P((
	struct mem_acc *loc, 
	int action, 
	struct pio_except *infop));

void letni16 P((twobyte_t *data_in));

void letni32 P((fourbyte_t *data_in));

int cat_wait_sram P((
	struct ca *ca, 
	open_t openp, 
	ulong addr, 
	ulong orig_val, 
	int timeout, 
	ulong *val));

int cat_user_write P((
	register struct ca *ca, 
	uchar *uaddr, 
	ulong saddr, 
	ulong mode, 
	ulong len));

int cat_user_read P((
	register struct ca *ca, 
	uchar *uaddr, 
	ulong saddr, 
	ulong mode, 
	ulong len));

int cat_check_status P((struct ca *ca));

void cat_init_fifos P((struct ca *ca));

int cat_get_stat P((
	struct ca *ca, 
	CTLFIFO *value));

int cat_put_cmd P((
	struct ca *ca, 
	CTLFIFO *value));

int cat_get_cfb P((
	register struct ca *ca, 
	BUFFIFO *value));

int cat_put_rfb P((
	struct ca *ca, 
	ulong *value));

int cat_get_sfb P((
	struct ca *ca, 
	BUFFIFO *value));

int sfb_avail P((struct ca *ca));

int cat_ret_buf P((
	struct ca *ca, 
	ulong buf_addr, 
	int buf_type));

void cat_shutdown P((register struct ca *ca));

int catread P((
	dev_t dev, 
	struct uio *uiop, 
	chan_t chan, 
	cio_read_ext_t *ext_p));

int catselect P((
	dev_t dev, 
	ulong events, 
	ushort *reventp, 
	int chan));

int dma_request P((struct ca *ca));

struct dma_req *dma_alloc P((
	struct ca *ca, 
	caddr_t owner, 
	int nelem));

struct dma_req *free_dma_request P((
	struct ca *ca, 
	dma_req_t *dmap));

void free_recv_element P((
	struct ca *ca, 
	struct recv_elem *recvp));

void clean_sc_queues P((
	struct ca *ca, 
	ulong subchannel));

int async_status P((
	struct ca *ca, 
	open_t *openp, 
	int code, 
	int status, 
	int netid));

int reserve_pca_xmit P((
	struct ca *ca, 
	xmit_elem_t *xmit_elem, 
	int index, 
	int num_requested));

int cat_wait_ack P((
	struct ca *ca, 
	cmd_t *cmdp, 
	int tmout));

void notify_all_opens P((
	struct ca *ca, 
	int status));

int cat_read_buf P((
	struct ca *ca, 
	ulong saddr, 
	uchar *buf, 
	int len));

void push_timeout P((
	struct ca *ca, 
	timeout_t *tp));

void pop_timeout P((
	struct ca *ca, 
	timeout_t *tp));

void cmdack_timeout P((
	struct ca *ca, 
	cmd_t *cmdp));

void cat_logerr P((
	struct ca *ca, 
	ulong errid));

int catwrite P((
	dev_t devno, 
	struct uio *uiop, 
	chan_t chan, 
	cat_write_ext_t *extptr));

struct xmit_elem *xmit_alloc P((struct ca *ca));

void free_xmit_element P((
	struct ca *ca, 
	xmit_elem_t *xmitp));

int send_elem P((
	struct ca *ca, 
	xmit_elem_t *xmitp));

int stop_all_subs P((
	struct ca *ca));

int cat_write_buf P((
	struct ca *ca, 
	ulong saddr, 
	uchar *buf, 
	int len));

void remove_listen P((
	struct ca *ca,
	listen_t *listp));

struct listen *locate_listen P((
	 struct ca *ca,
	 char *WSappl,
         int sc,
	 int linkid,
	 int correl,
	 int flag));

struct listen *start_listen P((
      struct ca *ca,
      open_t *open,
      int sc_id,
      char *Happl,
      char *WSappl,
      int state));

int cat_fastwrt P((
	dev_t devno,
	struct mbuf *p_mbuf));

int num_mbufs P((
	struct mbuf *mbufp));

int do_unst P((
	struct ca *ca,
	cat_unst_t *unst_blk));

void cat_gen_trace(char *,ulong,ulong,ulong);
#undef P
#endif /* _H_CATPROTO */
