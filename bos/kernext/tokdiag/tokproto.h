/* @(#)76	1.16  src/bos/kernext/tokdiag/tokproto.h, diagddtok, bos411, 9428A410j 11/30/93 10:37:41 */
#ifndef _H_TOKPROTO
#define _H_TOKPROTO

/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokproto.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* old-style declaration due to variable number of arguments */
extern void save_trace ();

/* unlink first element of list and return pointer to that element */
extern sll_elem_ptr_t sll_unlink_first (s_link_list_t *ll_ptr);

/* report connection complete to all waiting starters */
extern void conn_done (dds_t *p_dds);

/* report status to user */
extern void report_status (dds_t          *p_dds,
                           open_elem_t    *open_ptr,
                           cio_stat_blk_t *stat_blk_ptr);

/* final processing for a valid received packet */
extern void proc_recv (dds_t       *p_dds,
                       open_elem_t *open_ptr,
                       rec_elem_t  *rec_ptr);

/* read in the DMA receive list */
extern void read_recv_list (dds_t	*p_dds,
					recv_list_t  *rlptr,
					int          i);

/* final processing after a transmit complete */
extern void tok_xmit_done (dds_t      *p_dds,
                       xmt_elem_t *xmt_ptr,
                       ulong       status,
                       ulong       stat2);

/* que an offlevel element for later processing */
extern int que_oflv (offlevel_que_t *offl_ptr,
                     offl_elem_t    *elem_ptr);

/* generate a 16-bit crc value */
extern ushort cio_gen_crc (uchar *buf,
                           int    len);

/* add an entry to the component dump table */
extern void cio_add_cdt (char *name,
                         char *ptr,
                         int   len);

/* delete an entry from the component dump table */
extern void cio_del_cdt (char *name,
                         char *ptr,
                         int   len);

/* component dump table function */
extern cdt_t *cio_cdt_func ( );

/* initialize  a singly-link list structure */
extern void sll_init_list( s_link_list_t  *p_ll,
                           sll_elem_ptr_t p_space,
                           int            max_elem,
                           int            elem_size,
                           unsigned long  *p_hwm);

/* allocate a singly-linked list element */
extern sll_elem_ptr_t sll_alloc_elem( s_link_list_t *p_ll);

/* free a singly-linked list element */
extern void sll_free_elem( s_link_list_t  *p_ll,
                           sll_elem_ptr_t p_elem);

/* link a singly-linked list element to the end of list */
extern void sll_link_last( s_link_list_t  *p_ll,
                           sll_elem_ptr_t p_elem);

/* unlink the first singly-linked list element of active list */
extern sll_elem_ptr_t sll_unlink_first( s_link_list_t *p_ll);

/*
*  queue a status block (for a user process only)
*/
extern void que_stat_block( dds_t             *p_dds,
                            open_elem_t       *p_open,
                            cio_stat_blk_t    *p_stat_blk);

/*
*  mpx entry point from kernel
*/
extern int tokmpx( dev_t  devno,
                   int    *p_chan,
                   char   *p_channame);

/*
*  open entry point from kernel
*/
extern int tokopen(dev_t              devno,
                   unsigned long      devflag,
                   chan_t             chan,
                   cio_kopen_ext_t    *p_ext);

/*
*  close entry point from kernel
*/
extern int tokclose(dev_t  devno,
                    chan_t chan);

/*
*  write entry point from kernel
*/
extern int tokwrite(dev_t             devno,
                    struct uio        *p_uio,
                    chan_t            chan,
                    cio_write_ext_t   *p_ext);

/*
*  read entry point from kernel
*/
extern int tokread( dev_t             devno,
                    struct uio        *p_uio,
                    chan_t            chan,
                    cio_read_ext_t   *p_ext);

/*
*  ioctl entry point from kernel
*/
extern int tokioctl(dev_t          devno,
                    int            cmd,
                    int            arg,
                    unsigned long  devflag,
                    chan_t         chan,
                    int            ext);

/*
*  select entry point from kernel (user process only)
*/
extern int tokselect(dev_t             devno,
                     unsigned short    events,
                     unsigned short    *p_revent,
                     int               chan);

/*
*  device driver OFLV routine
*/
extern int tokoflv( offlevel_que_t *p_oflv);

/*
*  Timeout handler
*/
extern void cio_queofflms_func( struct trb *p_systimer);

/*
*  Watchdog Timer
*/
extern void cio_wdt_func( struct watchdog *p_wdog);

/*
 * Bringup Timer
 */
extern void bringup_timer(struct trb *p_time);

/* device-specific initialization of dds, set POS regiseters, get VPD */
extern int initdds (dds_t *p_dds);

/* activate ("open" and/or connect) the adapter */
extern int ds_act (dds_t *p_dds);

/* inactivate ("close" and/or disconnect) the adapter */
extern int ds_deact (dds_t *p_dds);

/* CIO_START ioctl */
extern int  cio_start(dds_t    *p_dds,
                      open_elem_t *open_ptr,
                      int cmd, int arg, ulong devflag,
                      chan_t chan, int ext);

/* CIO_HALT ioctl */
extern int  cio_halt(dds_t    *p_dds,
                      open_elem_t *open_ptr,
                      int cmd, int arg, ulong devflag,
                      chan_t chan, int ext);

/* CIO_GET_STAT ioctl */
extern int  cio_get_stat(dds_t    *p_dds,
                      open_elem_t *open_ptr,
                      int cmd, int arg, ulong devflag,
                      chan_t chan, int ext);

/* CIO_QUERY ioctl */
extern int  cio_query(dds_t    *p_dds,
                      open_elem_t *open_ptr,
                      int cmd, int arg, ulong devflag,
                      chan_t chan, int ext);

/* fill in the START_DONE status block */
extern void ds_startblk (dds_t          *p_dds,
                         netid_t   netid,
                         cio_stat_blk_t *stat_blk_ptr);

/* process a halt (usually just build HALT_DONE status block and report it) */
extern void ds_halt (dds_t          *p_dds,
                     open_elem_t    *open_ptr,
                     cio_sess_blk_t *sess_blk_ptr);

/* allow device specific code to clean up if abnormal close */
extern void ds_close (dds_t          *p_dds,
                      open_elem_t    *open_ptr);


extern int oflv_bringup(dds_t *p_dds,
                        offl_elem_t *p_owe);

extern int open_adap_pend(dds_t *p_dds,
                          offl_elem_t *p_owe);

extern int move_ring_info(dds_t *p_dds);

extern int get_adap_point(dds_t *p_dds);

extern int get_ring_info(dds_t *p_dds);


extern int kill_adap(dds_t *p_dds);

extern int tokdnld(dds_t *p_dds,
                     caddr_t arg,
                     unsigned long devflag);

extern void tokdsgetvpd(dds_t *p_dds);


extern int aca_alloc(dds_t *p_dds, int *first_open);

extern int aca_dealloc(dds_t *p_dds);

extern int get_mem(dds_t   *p_dds);

extern int get_mem_undo(dds_t   *p_dds);

extern int sb_setup(dds_t   *p_dds);

extern int sb_undo(dds_t   *p_dds);

extern int reset_adap(dds_t        *p_dds,
                      offl_elem_t  *p_owe);


extern int reset_phase2(dds_t        *p_dds,
                        offl_elem_t  *p_owe);


extern int init_adap(dds_t        *p_dds,
                     offl_elem_t  *p_owe);


extern int init_adap_phase0(dds_t        *p_dds,
                            offl_elem_t  *p_owe);


extern int init_adap_phase1(dds_t        *p_dds,
                            offl_elem_t  *p_owe);

extern int open_adap(dds_t        *p_dds,
                     offl_elem_t  *p_owe);

extern int open_adap_phase0(dds_t        *p_dds,
                            offl_elem_t  *p_owe);


extern int open_adap_pend(dds_t        *p_dds,
                          offl_elem_t  *p_owe);


extern int open_timeout(dds_t        *p_dds,
                        offl_elem_t  *p_owe);


extern void async_status(dds_t                *p_dds,
                         struct status_block  *p_sb);

extern void logerr(dds_t           *p_dds,
                   unsigned long   errid);

extern int clean_rcv( dds_t *p_dds);
extern int clean_tx( dds_t *p_dds);
extern int rcv_limbo_startup( dds_t *p_dds);
extern int tx_limbo_startup( dds_t *p_dds);

extern int enter_limbo( dds_t          *p_dds,
                        unsigned int   reason,
                        unsigned short ac );

extern int kill_limbo( dds_t   *p_dds );

extern int cycle_limbo( dds_t  *p_dds );

extern int egress_limbo( dds_t  *p_dds );

extern int kill_act( dds_t *p_dds);

extern int cfg_adap_parms( dds_t *p_dds);

extern int cfg_pos_regs(dds_t *p_dds);

extern int pio_read( dds_t         *p_dds,
                     unsigned int  reg);

extern int pio_write( dds_t        *p_dds,
                     unsigned int  reg,
                     unsigned short  value);

extern int attach_bus( dds_t   *p_dds);

extern int detach_bus( dds_t        *p_dds,
                       unsigned int prev_att);

extern int attach_iocc( dds_t   *p_dds);

extern int detach_iocc( dds_t        *p_dds,
                       unsigned int prev_att);

extern int initiate_scb_command( dds_t         *p_dds,
                                 unsigned int  command,
                                 unsigned int  address);

extern int issue_scb_command( dds_t         *p_dds,
                              unsigned int  command,
                              unsigned int  address);

extern void check_scb_command( dds_t *p_dds);

extern int bug_out( dds_t          *p_dds,
                    unsigned int   reason,
                    unsigned short ac);
#endif /* ! _H_TOKPROTO */
