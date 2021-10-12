/* @(#)10  1.12  src/bos/kernext/psla/gswextd.h, sysxpsla, bos41J, bai15 4/12/95 12:42:35 */
/* @(#)10	1.10  10/12/93 11:07:55 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Device driver for MSLA on R2 extern data items that     */
/*              are included when the other routines are compiled.      */
/*              Their allocation is in gsw13.c                          */
/*									*/
/*;MJ 021690    ddsp pointer to gsw_dds structure was added.            */
/*;BB 022890    added intr_priority for DISABLE calls.                  */
/*;bb 030690    Removed 'frst_pass' variable. not needed since INIT     */
/*;             now called once per adapter.                            */
/*              Found 'pa' was not 'extern' in this file.               */
/*;MJ 032090    Added 'diag_mode' flag.					*/
/*;bb 050990    Add 'parityrc' and 'parbufadr'.                         */
/*;bb 051290    Remove 'parityrc' and 'parbufadr'.                      */
/*;fp 030895    Added simple_lock, 3.2 to 4.1 port                      */
/************************************************************************/

extern int diag_mode;			/* diagnostics running flag.	*/
extern int ipl_flag;			/* indicates the need for IPL   */
extern int  send_done;                  /* toggle for SEND done.        */
extern int save_bus;
extern int dma_chanid;                  /* dma channel id from d_init   */
extern int dflt_size[ALL_MAX_INP];
extern int tbl_size[ALL_MAX_INP];

extern ulong start_busmem;              /* start of adapter busmem      */
extern ulong start_busio;               /* start of adapter busio       */
extern ulong bus_id;                    /* bus id passed in dds         */
extern int intr_priority;               /* intr_priority passed in dds  */

extern short wsf_len[14];
extern short wsf_typ[14];

extern ushort comm_lock;                /* communication area lock      */

extern char gintio_in_prog;             /* set to TRUE when call to gint*/
extern char adapter_slot;               /* adapter slot in wkstation    */
extern char rmidata[3];
extern char *ucp;                       /* ucode data area ptr          */
extern char *intr_name;
extern char *setupsio_name;
extern char *dodma_name;
extern char *undodma_name;
extern char *sendsol_name;
extern char *senduns_name;
extern char *stopstrt_name;
extern char *stataccp_name;
extern char *fpsetupsio_name;
extern char *fpsendsol_name;
extern char *timer_name;
extern char *fndqel_name;
extern char *cfg_name;
extern char *open_name;
extern char *ioctl_name;
extern char *gswio_name;
extern char *names[NumDevSupp];
extern struct msla_flags mf;

extern GSWCB *mgcb_ptr;
extern struct fpgi_send *fp_sendptr;
extern struct xmem *xmemdp;
					/* msla diag intr. counters ptr */
extern struct msla_intr_count * intr_cntptr;
extern struct intr *intrstrucp;         /* structure intr pointer       */

extern char     ae_exp_lda;             /* adapter end expected lda     */
extern char     ae_exp_cmd;             /* adapter end expected cmd     */

extern ushort   rmiq[NumDevSupp+1];     /* rmi queue -                  */
extern ushort   rmiqhead;               /* rmi queue head               */
extern ushort   rmiqtail;               /* rmi queue tail               */

extern ushort   pndq[NumDevSupp+1];     /* cmd pend q -                 */
extern ushort   pndqhead;               /* cmd pend q head              */
extern ushort   pndqtail;               /* cmd pend q tail              */

extern ushort   bufstat[NumDevSupp+1];  /* device buffer status         */

extern struct ccw        *ccw_offset;   /* set in setup_io              */
extern struct com_elm    *ce_offset;    /* set in setup_io              */
extern struct ccb        *hdr_offset;   /* set in setup_io              */

extern struct ccw        *fpccw_offset; /* set in fpsetup_io            */
extern struct com_elm    *fpce_offset;  /* set in fpsetup_io            */
extern struct ccb        *fphdr_offset; /* set in fpsetup_io            */
extern struct in_buf_hdr *free_buffer;  /* set in fpsetup_io            */

extern struct pinned_areas pa[NumDevSupp];      /* record malloced areas*/
extern struct gsw_dds *ddsp;

extern struct mr_comm_area volatile *cap;/* ptr to comm area-msla_to_r2 */
extern struct rm_comm_area volatile *rap;/* ptr to comm area-r2_to_msla */
extern struct msla_intr_count *intr_cntptr;/* msla diag intr. cnters ptr*/
extern char *rst_rgp;                   /* reset reg adr on adapter     */
extern char *disi_rgp;                  /* disable intr reg adr on adptr*/
extern char *enai_rgp;                  /* enable intr reg adr on adptr */
extern char *int_rgp;                   /* intrpt reg adr on adapter    */
extern char *stop_rgp;                  /* halt  reg adr on adapter     */
extern char *strt_rgp;                  /* start reg adr on adapter     */
extern char *rsti_rgp;                  /* reset intr reg adr on adpt   */
extern char *stat_rgp;                  /* status     reg adr on adpt   */
extern char ras_unique[];               /* used to number 'err_log's in */
					/*     a driver source file.    */

/* Added - JCC */
extern Simple_lock psladd_lock;
