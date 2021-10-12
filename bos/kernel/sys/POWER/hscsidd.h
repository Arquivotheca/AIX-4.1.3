/* @(#)65	1.27.4.8  src/bos/kernel/sys/POWER/hscsidd.h, sysxscsi, bos411, 9428A410j 2/4/94 16:04:08 */
#ifndef _H_HSCSIDD
#define _H_HSCSIDD
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver Header File
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/* General Device Driver Defines                                        */
/************************************************************************/

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif
#define	ALL_DEVICES	-1
#define MAXADAPTERS     8		/* entries in adap ptr tab      */
#define ADAP_HASH	0x07		/* adapter hash mask value      */

#define	IMDEVICES	64
#define	TMDEVICES	8
#define	DEVPOINTERS	IMDEVICES+TMDEVICES
#define	SC_TM_MAX_TCW_PERCENT 100
#define	SC_IM_MIN_TCWLEN 0x62000	/* 392K: (STA+Pg+MaxRqu) 2+32+64 */
#define	SC_TM_MIN_TCWLEN 0x10000	/* 64K */
#define	SC_TM_BUFSIZE	0x1000		/* 4096 */
#define	SC_TM_MIN_NUMBUFS  16
#define	MAX_TAG		16
#define	ENABLE		0
#define	DISABLE		1
#define	MAPPED_BUFS	1
#define	UNMAPPED_BUFS	0
#define	T_DISABLE	3
#define	MAX_ENA_RETRIES	3
#define	MAX_DIS_RETRIES	3
#define	SC_INITIATOR	0
#define	SC_TARGET	1
#define	PROC_LVL	1
#define	INTR_LVL	0
#define	L2PSIZE		12
#define	WAITING_FOR_RESOURCES	1
#define	WAITING_FOR_MB30	2

#define LPAGESIZE	PAGESIZE	/* internal page size lim       */
#define MAXREQUEST	0x40000		/* largest data xfer size:      */
					/* =  262144 dec. (256KB)       */
#define TCWRANGE	DMA_PSIZE	/* system TCW size              */
#define DMA_INIT	MICRO_CHANNEL_DMA /* DMA init flags             */
#define DMA_TYPE	0		/* DMA master/complete flags    */
#define SYS_ERR		0x80		/* flag value for DMA_SYSTEM err*/
#define ST_SIZE		256		/* size of a small transfer     */
#define STA_ALLOC_SIZE	PAGESIZE*2	/* size to allocate for STA     */
#define NUM_STA_TCWS	(STA_ALLOC_SIZE/TCWRANGE)  /* num STA TCWs used */
#define NUM_STA		(STA_ALLOC_SIZE/ST_SIZE)   /* num STAs available*/
#define BLDCMDSIZE	PAGESIZE	/* size of area to malloc for   */
					/* the ioctl SCSI commands      */


/* the following macro calculates the bus dma address the adapter uses  */
/* when using the reserved TCW range which was passed in the ddi area.  */
#define DMA_ADDR(start_addr, tcw_num) \
	((start_addr) + ((tcw_num) * TCWRANGE))
		

#define INT_TYPE        BUS_MICRO_CHANNEL /* bus type for interrupt level */
#define INT_FLAGS       0               /* define as sharable interrupt */

/* the following macro is used to quickly generate the device index.    */
/* the macro assumes "a" is SCSI ID, and "b" is LUN                     */
#define INDEX(a,b)	((((a) & 0x07)<<3) | ((b) & 0x07))

/* this macro returns the scsi id from a previously generated index.    */
#define SID(x)		((x)>>3)

/* this macro returns the lun id from a previously generated index.     */
#define LUN(x)		((x) & 0x07)

/* this macro returns the byte-swapped version of a 32-bit quantity     */
#define	WORD_REVERSE(a)							\
    (((ulong)(a) & 0xff000000)>>24 | ((ulong)(a) & 0x00ff0000)>>8 |	\
     ((ulong)(a) & 0x0000ff00)<<8  | ((ulong)(a) & 0x000000ff)<<24)

/************************************************************************/
/* Adapter I/O Defines                                                  */
/************************************************************************/
/* Adapter bus memory addressing definitions */
#define BCR		0x00	/* RO,  Basic Control Register (1 byte) */
#define ISR		0x08	/* RO,  Interrupt Status Reg (4 bytes)  */
#define SRR		0x10	/* RO,  Shadow Read Register (4 bytes)  */
#define BCR_SI		0x20	/* W=set, R=reset, Internal SCSI Reset  */
#define BCR_SE		0x24	/* W=set, R=reset, Int/Ext SCSI Reset   */
#define BCR_INT		0x28	/* W=set, R=reset, Interrupt enable     */
#define BCR_DMA		0x2c	/* W=set, R=reset, DMA enable           */
#define MB_START	0x0800	/* R/W, Offset to start of mbox area    */

/* Adapter POS register addressing definitions */
#define POS0		0x400000	/* RO, card id low        0x77  */
#define POS1		0x400001	/* RO, card id high       0x8d  */
#define POS2		0x400002	/* RW, card arb and enable      */
#define POS3		0x400003	/* RW, data when 6&7 non-zero   */
#define POS4		0x400004	/* RW, card int and nibble ena  */
#define POS5		0x400005	/* RW, card status, unused      */
#define POS6		0x400006	/* RW, card addr ext low        */
#define POS7		0x400007	/* RW, card addr ext high       */

#define POS0_VAL	0x77		/* Card ID low value            */
#define POS1_VAL	0x8d		/* Card ID high value           */

/* The following macro is used to generate the mailbox address.         */
/* The macro assumes "x" is the mailbox number (0-31) being addressed.  */
#define MB_ADDR(x)	(MB_START + ((x) * MB_SIZE))

/************************************************************************/
/* Miscellaneous Adapter Defines                                        */
/************************************************************************/
/* Adapter mailbox 30 command option definitions */
#define DIAGNOSTICS	0		/* run diagnostics command      */
#define DOWNLOAD	1		/* download microcode command   */
#define INITIALIZE	2		/* initialize lun or dev cmd    */
#define SCSI_COMMAND	3		/* send scsi command            */
#define SET_SCSI_ID	4		/* change card scsi id cmd      */
#define RESTART		6		/* send a card restart cmd      */
#define	ENA_ID		0x09		/* enable initiator id		*/
#define	ENA_BUF		0x0a		/* enable target mode buffers	*/

/* Send SCSI Command Parameters */
#define HSC_NODISC	0x80		/* do not allow device disc     */
#define HSC_NOSYNC	0x40		/* do not neg. for sync xfer    */
#define HSC_READ	0x20		/* request to do READ DMA       */
#define HSC_TRANS	0x10		/* data transfer requested      */

/* Miscellaneous defines */
#define MAX_POS_RETRIES 3		/* num retries of POS op        */
#define MAX_MB30_RETRIES 3		/* num retries of MB30 cmd      */
#define MAX_RESTART_RETRIES 3		/* num retries of Restart cmd   */
#define IPL_MAX_SECS	10		/* timeout for card diagnostics */
#define SCSI_RESET_T_O	15		/* timeout for scsi reset intrpt*/
#define ADAP_IPL_T_O	1		/* increment for ipl timer      */
#define ADAP_CMD_T_O	5		/* timeout for normal adap cmd  */
#define INIT_CMD_T_O	30		/* timeout for init lun/dev cmd */
#define WAITQ_CMD_T_O   30              /* timeout for cmds on wait que */
#define INIT_CMD_T_O_2	SCSI_RESET_T_O	/* secondary timeout for init   */

#define	ENABUF_CMD_T_O	2		/* set timeout for ena/dis buf  */
#define	ENAID_CMD_T_O	ENABUF_CMD_T_O

#define DELAY_DIVISOR	16		/* used to generate delay (diag)*/
#define NUM_MBOXES	32		/* number of adapter mailboxes  */
#define MB_SIZE		32		/* size of a mailbox (bytes)    */
#define MB_STAT_SIZE	8		/* size of a mb status area     */
#define DMA_BURST	0x00		/* set DMA burst size to 64     */
#define VPD_SIZE	255		/* size of adap VPD area (bytes)*/
#define MC_BLK_SIZE	1024		/* microcode transfer block size*/

/* Adapter Status defines (adapter general return codes) */
#define	MB_PARAMETER_ERROR	0x01
#define	MB_PARITY_ERROR		0x02
#define	MB_SEQUENCE_ERROR	0x03
#define	BAD_FUSE		0x04
#define	ADAP_FATAL		0x05
#define	SCSI_BUS_RESET		0x06
#define	UNKNOWN_SELECT		0x07
#define	TERM_BY_INIT_CMD	0x08
#define	PREVIOUS_ERROR		0x09
#define	OTHER_CMD_RUNNING	0x0b
#define	DIAGNOSE_PAUSED		0x0c
#define	COMMAND_PAUSED		0x10
#define	ADAP_RECOVERD_ERR	0x15
#define	COMPLETE_WITH_ERRORS	0x1f
#define	COMPLETE_NO_ERRORS	0xff

/* Adapter Status defines (adapter extra status) */
#define	NO_STATUS		0x00
#define	NO_RESPONSE		0x01
#define	INCORRECT_NUM_BYTES	0x02
#define	RESIDUAL_COUNT		0x03
#define CHECKSUM_ERROR		0x04
#define	SYSTEM_BUS_DMA_ERROR	0x05
#define	UNEXPECTED_BUS_FREE	0x08
#define	ABORTED_SCSI_CMD	0x0a
#define READY_FOR_NEXT_BLK	0x0e
#define RESEND_LAST_BLK   	0x0f
#define DIAG_CMPLT_PREV_ERR   	0x8f

/************************************************************************/
/* PIO operation defines                                                */
/************************************************************************/
#define	SI_RESET		0x01
#define	SE_RESET		0x02
#define	DMA_DISABLE		0x03
#define	INT_DISABLE		0x04
#define	READ_BCR		0x05
#define	READ_ISR		0x06
#define	READ_SRR		0x07
#define	RD_MB30_STAT		0x08
#define	RD_MBOX_STAT		0x09

#define	RD_ALL_MBOX_STAT	0xa
#define	MAX_PIO_READ_DEFINE	RD_ALL_MBOX_STAT

#define	SI_SET			0x10
#define	SE_SET			0x11
#define	DMA_ENABLE		0x12
#define	INT_ENABLE		0x13
#define	SYNC_DELAY		0x14

#define	WRITE_MB30		0x15
#define MIN_STR_PIO_WR_DEF      WRITE_MB30
#define	WRITE_MBOX		0x16
#define	WRITE_MB31		0x17
#define	WRITE_MC_BLOCK		0x18

/************************************************************************/
/* Miscellaneous Structures                                             */
/************************************************************************/
struct timer	{
	struct watchdog dog;		/* the watchdog struct          */
	ushort		timer_id;	/* my internal timer id val     */
	uchar		save_time;	/* used to manage the active q  */
	uchar		dev_index;	/* index to device struct	*/
					/*  1 = adapter cmd timer       */
					/*  2 = mailbox cmd timer       */
#define SC_ADAP_TMR	1		/* id of adapter timer          */
#define SC_MBOX_TMR	2		/* id of a cmd timer            */
#define SC_ADAP_TMR_2	3		/* id of second adapter timer   */
#define SC_ADAP_TMR_3	4		/* id of wait queue timer       */
	struct adapter_def *adp;	/* pointer to adapter           */
};

struct sta_str	{			/* Small Transfer Area Structure */
	uint	in_use;			/* TRUE if this area in use     */
	char	*stap;			/* pointer to this xfer area    */
};

struct error_log_def {			/* driver error log structure   */
	struct err_rec0	errhead;	/* error log header info        */
	struct rc	data;		/* driver dependent err data    */
};

struct io_parms	{			/* I/O operation parameter struct*/
	struct adapter_def *ap;		/* pointer to adapter           */
	struct mbstruct    *mbp;	/* pointer to mbox struct       */
	volatile caddr_t   mem_addr;	/* return addr from io_att      */
	volatile caddr_t   iocc_addr;   /* return addr from io_att      */
	volatile uint	   opt;		/* i/o function option          */
	caddr_t		   sptr;	/* secondary pointer            */
	volatile ulong	   data;	/* input data for certain opts, */
					/* also used for return data    */
	volatile uchar	   errtype;	/* used to return error info    */
					/*  0 = no error info           */
#define PIO_TEMP_DATA_ERR  0x01		/*    = temp pio data error     */
#define PIO_TEMP_OTHR_ERR  0x02		/*    = temp other pio error    */
#define PIO_TEMP_IOCC_ERR  0x04		/*    = temp iocc pio error     */
#define MAX_PIO_TEMP_ERR   PIO_TEMP_IOCC_ERR	/*  = temp err boundary */
#define PIO_PERM_DATA_ERR  0x10		/*    = perm pio data error     */
#define PIO_PERM_OTHR_ERR  0x20		/*    = perm other pio error    */
#define PIO_PERM_IOCC_ERR  0x40		/*    = perm iocc pio error     */
	volatile uchar	   iocc_err;	/* TRUE if internal iocc err    */
	volatile uchar	   ahs;		/* used to return ahs err info  */
	volatile ulong	   eff_addr;	/* effective address in error   */
};

struct hsc_cdt_tab {			/* component dump table struct  */
	struct	cdt_head   hsc_cdt_head;/* header to the dump table     */
	struct	cdt_entry  hsc_entry[1+MAXADAPTERS];
					/* space for each minor + trace */
};

#define	TM_FATAL	33

/*	target mode read buffer info structure
	Note that this structure is the super-set
        definition of the "tm_buf" struct in scsi.h.
        Any change to this struct should be reflected
        in "tm_buf".
*/
struct	b_link {
	uint	tm_correlator;	/* same as that given to adap driver */
	dev_t	adap_devno;	/* device major/minor of this adapter */
	caddr_t	data_addr;	/* kernel space addr where data begins */
	int	data_len;	/* length of valid data in buffer */
	ushort	user_flag;	/* flags set for the user.  may be one
                                   or more of the following:       */
#ifndef TM_HASDATA
#define	TM_HASDATA	0x04	/* set if this is a valid tm_buf */
#define	TM_MORE_DATA	0x08	/* set if more data coming for this Send */
#define	TM_ERROR	0x8000	/* set if any error occurred on this Send
                                   (mutually exclusive with TM_MORE_DATA) */
#endif
	uchar	user_id;	/* SCSI id which sent the data */
	uchar	owner_id;	/* id which malloced this buffer */
	uchar	status_validity;	/* valid values shown below: */
/* #define	SC_ADAPTER_ERROR	2  general_card_status is valid */
	uchar	general_card_status;	/* defined under struct sc_buf */
	uchar	resvd1;			/* reserved for expansion */
	uchar	resvd2;			/* reserved for expansion */
	uchar	tag;		/* buffer tag used to enable this buffer */
	uchar	option;		/* 0 => enable, 1 => disable */
	ushort	owner_flag;	/* owner state flags */
#define	TM_ALLOCATED	0x01	/* when set, means the buffer is malloced.
                                   (may or may not be in process of being
                                   enabled) */
#define	TM_ENABLED	0x02	/* when set, means buffer has been
                                   completely enabled (mb30 complete) */
	struct	adapter_def *ap;	/* pointer to owning adapter */
	caddr_t	buf_addr;	/* original (saved) buffer address */
	uint	buf_size;	/* original (saved) buffer length */
	uint	dma_addr;	/* I/O bus DMA address used for buffer */
	int	tcws_alloced;	/* number of tcws alloced for buffer */
	int	tcw_start;	/* index of starting tcw for buffer */
	struct	b_link *next;	/* either NULL or link to next b_link
                                   Used when this is on a free list */
	struct	b_link *forw;	/* NULL or forward link when on rdbuf list */
	struct	b_link *back;	/* NULL or back link when on rdbuf list */
	uint	resvd3;		/* reserved for expansion */
	uint	resvd4;		/* reserved for expansion */
	uint	resvd5;		/* reserved for expansion */
	uint	resvd6;		/* reserved for expansion */
};

/*	SEND command info in mb31 status */

struct	tm_send_info {
	uchar	flag;

#define	TM_VALID_DATA	0x01
#define	TM_END_OF_SEND_CMD	0x02
#define	TM_NOMORE_BUFS	0x04
#define	TM_EXCEPTION	0x08

	uchar	id;
	uchar	tag;
	uchar	tm_status;
	uint	data_len;
};

/*	mb31 status structure redefined for target mode */

struct	tm_mb31_info {
	struct	tm_send_info send_info[3];
	int	resvd1;
	uchar	adapter_rc;
	uchar	xtra_stat;
	short	resvd2;
};

/* the gwrite structure is used for management of buffers allocated
   specifically to handle initiator-mode gathered writes */

struct	gwrite {
	caddr_t	buf_addr;	/* original (saved) buffer address */
	int	buf_size;	/* original (saved) buffer length */
	uint	dma_addr;	/* I/O bus DMA address used for buffer */
	struct	gwrite *next;	/* either NULL or link to next gwrite */
};

/************************************************************************/
/* Structures related to device control                                 */
/************************************************************************/
struct dev_info {
	uchar           opened;         /* TRUE if opened (started)     */
	uchar           scsi_id;        /* SCSI ID of this device       */
	uchar           lun_id;         /* LUN ID of this device        */
	uchar 		waiting;	/* device flags:                */
					/*  FALSE = pend queue not wait */
					/*  TRUE  = pend queue waiting  */
	uchar		qstate;		/* device general queue state   */
					/*  0 = normal state            */
					/*  5 = SCIOSTOP in progress    */
					/*      (do not accept cmds)    */
					/*  6 = wait for sc_buf with    */
					/*       resume flag set        */
#define STOP_PENDING	5		/* stopping commands forcibly   */
#define HALTED		6		/* waiting for a resume         */
	uchar		state;		/* device general command state */
					/*  0 = normal state            */
#define WAIT_TO_SEND_INIT_LUN	15	/* waiting to do an Init Lun    */ 
#define WAIT_FOR_INIT_LUN	16	/* waiting for an Init Lun      */
#define WAIT_TO_SEND_RESUME	17	/* waiting to do a resume I L   */ 
#define WAIT_FOR_RESUME         18	/* waiting for a resume I L     */
#define WAIT_FOR_INIT_T_O_2	19	/* wait for either 2nd time-out */
					/*  or the init cmd intrpt      */
	uchar		init_cmd;	/* indicates proc lvl issuing   */
					/* a MB30 init cmd affecting    */
					/* this device.  defines:       */
					/*  0 = no init commands        */
					/*  7 = some init cmd in progr  */
#define INIT_CMD_IN_PROGRESS	7	/* an init cmd is in progress   */
	uchar		num_act_cmds;	/* num cmds sent to adapter     */
	int		stop_event;	/* event word for stop ioctl    */
	struct  timer	*wdog;          /* timer struct per device act q*/
	struct	sc_buf	*head_pend;	/* ptr to pending cmd queue     */
	struct  sc_buf  *tail_pend;	/* end of pending cmd queue     */
	struct	sc_buf	*head_act;	/* ptr to active cmd queue      */
	struct  sc_buf  *tail_act;	/* end of active cmd queue      */
	void		(*async_func)();/* async entry point in head	*/
	uint		async_correlator; /* as passed to async event   */
	uchar		pqstate;	/* pending queue state flag     */
					/*  0 = normal state            */
					/*  15 = previous error prevents*/
					/*       sending pending cmds.  */
#define PENDING_ERROR		15	/* error on previous pend cmd   */
	uchar		trace_enable;	/* enable device internal trace */
	uchar		stopped;	/* False: enabled, True: disabled */
	uchar		in_open;        /* if true, in process of opening */
	uchar		dev_abort;	/* flag to toss first buf abort */
	uchar           pad_char1;      /* padding                      */
	ushort		pad_short1;	/* padding                      */
	uint		num_bufs_recvd;	/* stat: num bufs recvd in session */
	uint		num_bytes_recvd; /* stat: num bytes recvd in session */
	int		num_bufs;	/* no. of bufs(high limit) alloced */
	int		num_to_resume;	/* low water mark for reenable id */
	int		num_bufs_qued;	/* no. of bufs with data qued	*/
	uint		buf_size;	/* size of the read buffers	*/
	int		previous_err;	/* indicate a previous buf error*/
#define	TM_UCODE_ERR	01		/* a previous microcode error   */
#define	TM_DMA_ERR	02		/* a previous DMA error         */
	void		(*recv_func)();	/* recv bufs function in head   */
	uint		tm_correlator;	/* as passed to start target    */
	uchar		queue_depth;	/* used to enabled and disable  */
                                        /* queueing                     */
#define Q_ENABLED       0xFF 
#define Q_DISABLED      0x01 
	uchar		dev_queuing;	/* indicates if queuing to this */
                                        /* device (cmd tag queueing)    */
        uchar           cc_error_state; /* used for check condition     */
                                        /* error recovery/detection     */
                                        /* when cmd tag queueing        */
#define CC_OCCURRED              1
#define WAIT_FOR_RECOVERY_CMD    2
	uchar		queuing_stopped;/* active que forced to quiesce */
	struct  sc_buf  *cmd_save_ptr_cc;/* used for check cond error    */
                                        /* recovery when cmd tag queuing*/
	struct  sc_buf  *cmd_save_ptr_res;/* used for check cond error    */
                                        /* recovery when cmd tag queuing*/
	ushort		pad_short2;	/* padding                      */
	ushort		end_flag;	/* flag end of struct (0xffff)  */
};


/************************************************************************/
/* Structures related to adapter control                                */
/************************************************************************/
 struct mbox	{
	uchar		m_op_code;	/* adapter op code              */
	uchar		m_xfer_id;	/* transfer char. and SCSI ID   */
	uchar		m_cmd_len;	/* burst size/cmd length        */
	uchar		m_sequ_num;	/* number of next MB            */
	uint		m_dma_addr;	/* DMA start address            */
	uint		m_dma_len;	/* DMA length (bytes)           */
	struct sc_cmd	m_scsi_cmd;	/* SCSI cmd block area          */
			/* start of status area                         */
	uint		m_resid;	/* MB residual count            */
	uchar		m_adapter_rc;	/* main adapter ret code        */
	uchar		m_extra_stat;	/* extra status byte            */
	uchar		m_scsi_stat;	/* SCSI status byte             */
	uchar		m_resvd;	/* reserved byte                */
};

struct mbstruct {
	uchar		id0;		/* id char                      */
	uchar		id1;		/* id char                      */
	uchar		id2;		/* id char                      */
	uchar		MB_num;		/* id of this mailbox struct (0-31) */
	struct mbstruct	*next;		/* ptr to next mailbox          */
	struct mbstruct *prev;		/* ptr to previous mailbox      */
	struct sc_buf	*sc_buf_ptr;	/* ptr to orig sc_buf           */
	uint            cmd_state;      /* current state of command     */
					/*  0  = cmd is not active      */
					/*  10 = cmd is active (sent)   */
					/*  15 = intrpt received, or    */
					/*        cmd is complete       */
					/*  17 = wait for intrpt        */
					/*  19 = wait for 2nd t.o. or   */
					/*        cmd interrupt         */
#define INACTIVE        0               /* cmd is not active            */
#define ISACTIVE        10              /* cmd is active (sent to adap) */
#define INTERRUPT_RECVD	15              /* interrupt has been received  */
#define WAIT_FOR_INTRPT	17              /* wait for intrpt (after error)*/
#define WAIT_FOR_T_O_2	19              /* wait for 2nd t.o. or intrpt  */
	int             tcws_allocated; /* num TCWs allocated, 0 if none*/
	int             tcws_start;     /* starting TCW (if alloc not 0)*/
	signed int	sta_index;	/* allocated STA, -1 if none    */
	struct mbox 	mb;		/* 32-byte mailbox structure    */
	uchar		d_cmpl_done;	/* TRUE if d_complete done	*/
	uchar		preempt;	/* cnt # of times cmd preempted */
#define MAX_PREEMPTS	80              /* MAX allowable preempts	*/
	ushort          end_flag;       /* flag end of struct (0xffff)  */
};


struct adapter_def {
	struct intr	intr_struct;		/* int handler struct   */
	struct timer	wdog;          		/* adapter timer struct */
	struct timer	wdog2;         		/* adapter timer struct */
	struct timer	wdog3;         		/* timer strc for waitq */
	lock_t		ap_lock;		/* per adapter lock     */
	struct adapter_def	*next;		/* pointer to next str. */
	dev_t           devno;                  /* adapter major/minor  */
	struct adap_ddi ddi;                    /* passed init data     */
	uint		maxxfer;		/* max allowed transfer */
	uchar           inited;                 /* adapter initialized  */
	uchar           opened;                 /* adapter first opened */
	uchar		devices_in_use;         /* num. opened devices  */
	uchar		adapter_mode;           /* mode opened in:      */
						/*  0 = normal mode     */
						/*  1 = diagnostic mode */
#define NORMAL_MODE     0               	/*  normal operation    */
#define DIAG_MODE       1               	/*  diagnostic mode     */
	int		any_waiting;		/* devices waiting count*/
	int		commands_outstanding;	/* sum of cmds in prog. */
	uchar		adapter_check;		/* adapter check state: */
						/*  0 = normal state    */
						/*  5 = adap dead       */
#define ADAPTER_DEAD	5			/* adap dead (hdw prob) */
	uchar		epow_state;		/* power failure flag   */
						/*  0 = normal state    */
						/*  4 = EPOW pending    */
#define EPOW_PENDING	4			/* adap EPOW pending    */
	uchar		errlog_enable;		/* set if errors are to */
						/*  be logged           */
	signed char	last_dev_index;		/* starting index for   */
						/*  round-robin dev srch*/
	ushort		next_page_req;		/* start for page search*/
	ushort		next_large_req;		/* start for larg search*/
	ushort		page_req_begin;		/* where page reqs start*/
	ushort		page_req_end;		/* where page reqs end  */
	ushort		large_req_begin;	/* where large reqs strt*/
	ushort		large_req_end;		/* where large reqs end */
	ushort		num_tcws;		/* num reserved tcws    */
	ushort		sta_tcw_start;		/* starting tcw for STA */
	char		*TCW_tab;		/* pointer to rsvd tcws */
						/*   management table   */
	uchar		proc_waiting;		/* proc lvl waiting:    */
						/*  0  = not waiting    */
						/*  15 = waiting on MB30*/
						/*  to do INIT LUN      */
						/*  16 = waiting for    */
						/*  intrpt from INIT LUN*/
						/*  17 = wait to send an*/
						/*        INIT DEV      */
						/*  18 = wait for INIT  */
						/*        DEV completion*/
						/*  19 = wait for INIT  */
						/*   cmd t.o. or intrpt */
						/*  20 = wait to send a */
						/*   set scsi id cmd    */
						/*  21 = wait for set   */
						/*   scsi id to complete*/
						/*  22 = wait to send a */
						/*   Restart command    */
						/*  23 = wait for a     */
						/*   Restart to cmplete */
/*	WAIT_TO_SEND_INIT_LUN	15  */		/* wait to do INIT LUN  */
/*	WAIT_FOR_INIT_LUN	16  */		/* wait for an INIT LUN */
#define WAIT_TO_SEND_INIT_DEV	17		/* wait to do INIT DEV  */ 
#define WAIT_FOR_INIT_DEV	18		/* wait for an INIT DEV */
/*	WAIT_FOR_INIT_T_O_2	19  */		/* wait for t.o. or int */
#define WAIT_TO_SEND_SET_ID	20		/* wait to send SCSI ID */
#define WAIT_FOR_SET_ID		21		/* wait for set SCSI ID */
#define WAIT_TO_SEND_RESTART	22		/* wait to do Restart   */
#define WAIT_FOR_RESTART	23		/* wait for a Restart   */
/* 	WAIT_TO_ENA_BUF		31  */		/* handles ena/dis buf  */
/* 	WAIT_FOR_ENA_BUF	32  */
/* 	WAIT_TO_ENA_ID		37  */		/* handles ena/dis id   */
/* 	WAIT_FOR_ENA_ID		38  */
#define	WAIT_FOR_ENA_T_O_2	24		/* wait for t.o. or int */
                                                /* for either buf or id */
#define WAIT_TO_SEND_DNLD_VERSION 25            /* wait to do a download*/
                                                /* version query        */
#define WAIT_FOR_DNLD_VERSION     26            /* waiting for download */
                                                /* version query        */
#define WAIT_TO_SEND_DNLD_CMD     27            /* wait to download mc  */
#define WAIT_FOR_DNLD_CMD         28            /* waiting for donwload */
                                                /* mc completion        */
	uchar		p_scsi_id;		/* SCSI ID for INIT cmd */
	uchar		p_lun_id;		/* LUN for INIT LUN     */
	uchar		proc_results;		/* internal flags for   */
						/* returning results of */
						/* process lvl MB30 cmds*/
						/*  0 = good completion */
						/*  1 = further proc.   */
						/*      needed          */
						/*  2 = adap cmd failed */
						/*  3 = adap cmd timeout*/
#define GOOD_COMPLETION	0			/* adap cmd succ compln */
#define SEE_RC_STAT	1			/* adap cmd non-good,   */
						/* see the rc/stat bytes*/
#define FATAL_ERROR	2			/* adap cmd fatal error */
#define TIMED_OUT	3			/* adap cmd timeout     */
	uint		mb30_resid;		/* saved MB30 status    */
	uchar		mb30_rc;		/* saved MB30 status    */
	uchar		mb30_extra_stat;	/* saved MB30 status    */
	uchar		mb30_byte30;		/* saved MB30 status    */
	uchar		mb30_byte31;		/* saved MB30 status    */
	uint		mb31_resid;		/* saved MB31 status    */
	uchar		mb31_rc;		/* saved MB31 status    */
	uchar		mb31_extra_stat;	/* saved MB31 status    */
	uchar		mb31_byte30;		/* saved MB31 status    */
	uchar		mb31_byte31;		/* saved MB31 status    */
	int		channel_id;             /* dma channel id       */
	int		event;			/* timer event word     */
	int		cl_event;		/* event word for close */
	struct xmem	xmem_buf;		/* local xmem descrip.  */
	struct mbstruct	*head_MB_free;		/* head of MB free list */
	struct mbstruct *tail_MB_free;		/* tail of MB free list */
	struct mbstruct	*head_MB_act;		/* head of MB active list */
	struct mbstruct *tail_MB_act;		/* tail of MB active list */
	struct mbstruct	*head_MB_wait;		/* head of MB wait list */
	struct mbstruct *tail_MB_wait;		/* tail of MB wait list */
	struct gwrite *head_gw_free;		/* gathered write free  */
	struct gwrite *tail_gw_free;		/* gathered write free  */
	uchar		IPL_tmr_cnt;		/* work area used for   */
						/*    adap init         */
	uchar		MB30_retries;		/* MB30 retry count     */
	signed char	MB30_in_use;		/* MB30 current usage:  */
						/*  -1  = not in use    */
						/* 0-63 = init dev using*/
						/* 64-71= reserved      */
						/* 0x50 = tm dev  using */
						/* 0x55 = adapter using */
						/* 0x77 = proc lvl using*/
#define	NOT_IN_USE	-1			/* MB30 is free         */
#define	TM_DEVICE_USING	0x50
#define ADAP_USING	0x55			/* adapter using MB30   */
#define	PROC_USING	0x77			/* proc level using MB30*/
	uchar		waiting_for_mb30;	/* set if any dev wait  */
	uchar		restart_state;		/* error recovery state:*/
						/*  0  = none           */
						/*  22 = wait to send a */
						/*        RESTART       */
						/*  23 = wait for RESTART*/
						/*        completion    */
/*	WAIT_TO_SEND_RESTART	22 */ 		/* waiting to do RESTART*/ 
/*	WAIT_FOR_RESTART	23 */		/* waiting for a RESTART*/
	uchar		restart_again;		/* 1 = need RESTART again*/
	uchar		restart_index;		/* where to start after */
						/* processing a RESTART */
	uchar		restart_index_validity;	/* TRUE if restart_index*/
						/* is valid, else FALSE */
	uchar		restart_retries;	/* num. restart retries */
	uchar		close_state;		/* close routine state  */
						/*  0 = normal          */
						/*  3 = adap close pend */
#define CLOSE_PENDING	3			/* adap close pending   */
	uchar		dump_inited;		/* dump init completed  */
	uchar		dump_started;		/* dump start completed */
	uchar		cdar_scsi_ids;		/* command delay after  */
						/* reset (cdar) scsi ids*/
	uchar		page_count;		/* num. pages this struc*/
        uchar           download_pending;       /* is a microcode       */
#define DOWNLOAD_IN_PROGRESS 1                  /* download pending     */
                                                /* 0 = normal           */
                                                /* 1 = download pending */
        uint            download_mc_len;        /* length of download   */
                                                /* microcode file       */
	int		dump_pri;		/* saved dump int prior.*/
	uint		trace_enable;		/* adapter lvl trace ena*/
	struct timestruc_t time_s;		/* curtime structure    */
	struct mbstruct *MB30p;			/* pointer to MB30      */
	struct mbstruct *MB31p;			/* pointer to MB31      */
	struct sta_str	STA[NUM_STA];		/* STA mgmt table       */
	struct dev_info dev[DEVPOINTERS];	/* array of device info */
						/*  structures          */
				/* access the array via scsi id/lun:    */
				/*       msb  7 6 5 4 3 2 1 0 lsb       */
				/*       ----+-+-+-+-+-+-+-+-+---       */
                                /*            x x i i i l l l           */
				/*                d d d u u u           */
				/*                2 1 0 n n n           */
				/*                      2 1 0           */
				/*                                      */
	struct mbstruct	MB[NUM_MBOXES];		/* array of mailboxes   */
						/*  and assoc. data     */
	char		tm_start[8];		/* start of tm info	  */
	int		num_enabled;		/* no. of bufs enabled	  */
	int		tgt_dma_err;		/* flag for tgt dma error */
                                                /*   recovery state       */
	int		num_tgt_tcws;		/* total num tgt tcws     */
	int		num_tgt_tcws_used;	/* total tgt tcws in use  */
	int		num_tm_devices;		/* num tgt devices opened */
	int		tgt_req_begin;		/* tgt tcw area begin index */
	int		tgt_req_end;		/* tgt tcw area last index */
	int		tgt_next_req;		/* next tgt tcw index      */
	uchar		enabuf_state;		/* intr lvl ena/dis buf stat */
#define	WAIT_TO_ENA_BUF		31
#define	WAIT_FOR_ENA_BUF	32
#define	WAIT_FOR_ENABUF_T_O_2	33
#define WAIT_TO_DIS_BUF		34
#define	WAIT_FOR_DIS_BUF	35
#define	WAIT_FOR_DISBUF_T_O_2	36

	uchar		enaid_state;		/* intr lvl enable id state */
#define	WAIT_TO_ENA_ID		37
#define	WAIT_FOR_ENA_ID		38
#define	WAIT_FOR_ENAID_T_O_2	39

	uchar		disid_state;		/* intr lvl disable id state */
#define	WAIT_TO_DIS_ID		40
#define	WAIT_FOR_DIS_ID		41
#define	WAIT_FOR_DISID_T_O_2	42

	uchar		disable_tag;		/* intr lvl buf tag to disb */
	uchar		disable_id;		/* intr lvl current dis id */
	uchar		waiting_disids;		/* flag byte for ids waiting */
                                                /*   for disable id to run   */
	uchar		t_dis_ids;		/* flag byte for type of dis */
                                                /*   id to run for each id   */
	uchar		ena_id;			/* intr lvl current ena id */
	uchar		waiting_enaids;		/* flag byte for ids waiting */
                                                /*   for enable id to run   */
	uchar		enabuf_retries;		/* intr lvl ena buf retries */
	uchar		disbuf_retries;		/* intr lvl disa buf retries */
	uchar		enaid_retries;		/* intr lvl ena id retries  */
	uchar		disid_retries;		/* intr lvl disa id retries */
	uchar		enable_queuing;	        /* if TRUE driver will queue*/
                                                /* to adapter               */ 
	uchar		epow_reset;		/* indicates if SCSI bus    */
                                                /* reset is due to epow     */
	uchar		reset_pending;		/* indicates if SCSI bus    */
                                                /* reset is pending         */
	uchar		p_id;			/* proc level ena/dis id */
	uchar		p_id_option;		/* proc level id option  */
	struct	b_link	*p_buf;			/* proc lvl ena/dis buf ptr */
	struct	b_link	*ena_buf;		/* intr lvl ena buf ptr */
                                                /* the buffer free lists are */
                                                /* singly-linked lists (next)*/
	struct	b_link	*head_free_mapped;	/* head of mapped free bufs */
	struct	b_link	*tail_free_mapped;	/* tail of mapped free bufs */
	struct	b_link	*head_free_unmapped;	/* head of unmapp free bufs */
	struct	b_link	*tail_free_unmapped;	/* tail of unmapp free bufs */
	struct	b_link	*mapped_rdbufs;		/* head of doubly-linked    */
                                                /*   list (back/forw) of the*/
                                                /*   mapped bufs in head    */
	struct	b_link	*unmapped_rdbufs;	/* same as above but for the*/
                                                /*   unmapped bufs in head  */
	struct	b_link	*tm_garbage;		/* head of garbage list	*/
	struct	b_link	*tm_bufs[MAX_TAG];	/* arry of enabled bufs */
	int             end_flag;		/* end flag (0xffff)    */
};

/* declarations for internal trace table                                */
#define	TRC_CMD		1		/* trace command issuance       */
#define	TRC_DONE	2		/* trace iodone of commands     */
#define	TRC_ISR		3		/* trace ISR contents           */
#define	TRC_MB30	4		/* trace mailbox 30 commands    */
#define	TRC_XXX		5		/* special                      */
#define	TRC_MB31	6		/* trace mailbox 31 commands    */
#define	TRC_ENAB	7
#define	TRC_ENID	8
#define	TRC_STRTBUF	9
#define	TRC_STRTID	10
#define	TRC_STOPID	11
#define	TRC_MB30INTR	12

struct trace_element {
  uint	 id0;	  /*'CMD'       'DONE'      'MB30'  'MB31'  'ISR'  'XXX'  */
  uint	 c00;	  /* ap          ap          ap      ap      ap     ap    */
  uint	 c01;	  /* scp         scp         BLD |   0       isr    mbp   */
		  /*                          DONE                        */
  uchar	 c02;	  /* MB_num      MB_num      0       0       0      MB_n  */
  uchar	 c03;	  /* 0           0           0       0       0      0     */
  uchar	 c04;	  /* 0           0           0       0       0      0     */
  uchar	 c05;	  /* num_act_cmd b_error     0       0       0      0     */
  ushort c06;	  /* tcws_alloc  valid ||    0       0       0      0     */
  		  /*              scsi_st                                 */
  ushort c07;	  /* tcws_start  card_st ||  0       0       0      0     */
  		  /*              0                                       */
  uint	 c08;	  /* sta_index   b_resid     0       0       0      0     */
  struct mbox mb; /* mbox        mbox        mb30    mb31    0      mbox  */
};

#define	TRACE_ENTRIES	132

#ifndef	_NO_PROTO
/*****************************************************************************/
/*     internal functions                                                    */
/*****************************************************************************/

int     hsc_config(dev_t devno, int op, struct uio *uiop);
int     hsc_open(dev_t devno, ulong devflag, int chan, int ext);
int     hsc_close(dev_t devno, int offchan);
int     hsc_strategy(struct sc_buf *bp);
int     hsc_ioctl(dev_t devno, int op, int arg, ulong devflag, int chan,
	int ext);
int     hsc_intr(struct intr *handler);
int     hsc_iodone(struct sc_buf *bp);
struct  sc_buf *hsc_bld_sc_buf();
void    hsc_ioctl_sleep(struct buf *bp);
void    hsc_release(struct sc_buf *scp);
void    hsc_start(struct adapter_def *ap, int dev_index);
int     hsc_start_dev(struct adapter_def *ap, int dev_index);
void    hsc_build_and_send(struct adapter_def *ap, struct sc_buf *scp,
	int dev_index);
void    hsc_issue(struct adapter_def *ap, struct mbstruct *mbp,
	struct sc_buf *scp, int dev_index);
int     hsc_MB_alloc(struct adapter_def *ap, struct sc_buf *scp,
	int dev_index);
void    hsc_MB_dealloc(struct adapter_def *ap, struct mbstruct *mbp);
void    hsc_MB_restore(struct adapter_def *ap, struct mbstruct *mbp);
int     hsc_TCW_alloc(struct adapter_def *ap, struct mbstruct *mbp,
	struct sc_buf *scp, int dev_index);
void    hsc_TCW_dealloc(struct adapter_def *ap, struct mbstruct *mbp);
int     hsc_STA_alloc(struct adapter_def *ap, struct mbstruct *mbp);
void    hsc_STA_dealloc(struct adapter_def *ap, struct mbstruct *mbp);
void    hsc_build_mb30(struct adapter_def *ap, int op, int data1,
	int data2, int data3);
void    hsc_watchdog(struct watchdog *w);
struct  cdt *hsc_cdt_func(int arg);
int     hsc_dump(dev_t devno, struct uio *uiop, int cmd, int arg,
	int chan, int ext);
int     hsc_dumpwrite(struct adapter_def *ap, struct sc_buf *scp);
void    hsc_DMA_err(struct adapter_def *ap, int dev_index);
void    hsc_dev_DMA_err(struct adapter_def *ap, int dev_index,
	struct sc_buf *scp);
int     hsc_dma_cleanup(struct adapter_def *ap, struct sc_buf *scp,
	int dma_err);
void    hsc_MB30_handler(struct adapter_def *ap, int flag);
int     hsc_epow(struct intr *handler);
struct  adapter_def *hsc_alloc_adap(dev_t devno, uint tcw_size);
void    hsc_free_adap(struct adapter_def *ap);
int     hsc_config_adapter(struct adapter_def *ap);
int     hsc_alloc_dev(struct adapter_def *ap, int dev_index, ulong devflag);
int     hsc_dealloc_dev(struct adapter_def *ap, int dev_index, ulong devflag);
int     hsc_halt_lun(struct adapter_def *ap, int dev_index, ulong devflag);
int     hsc_reset_dev(struct adapter_def *ap, int dev_index, ulong devflag);
int     hsc_inquiry(struct adapter_def *ap, dev_t devno, int arg,
	ulong devflag);
int     hsc_readblk(struct adapter_def *ap, dev_t devno, int arg,
	ulong devflag);
int     hsc_startunit(struct adapter_def *ap, dev_t devno, int arg,
	ulong devflag);
int     hsc_testunitready(struct adapter_def *ap, dev_t devno, int arg,
	ulong devflag);
int     hsc_diagnostic(struct adapter_def *ap, int arg, ulong devflag);
int     hsc_diag_test(struct adapter_def *ap, struct sc_card_diag *ptr);
int     hsc_wrap_test(struct adapter_def *ap, struct sc_card_diag *ptr);
int     hsc_reg_test(struct adapter_def *ap, struct sc_card_diag *ptr);
int     hsc_pos_test(struct adapter_def *ap, struct sc_card_diag *ptr);
int     hsc_reset_test(struct adapter_def *ap, struct sc_card_diag *ptr);
int     hsc_download(struct adapter_def *ap, int arg, ulong devflag);
void    hsc_enable_cmd_q(struct adapter_def *ap);
void    hsc_dnld_start_devs(struct adapter_def *ap);
void    hsc_fail_cmd(struct adapter_def *ap, int dev_index);
int     hsc_scsi_reset(struct adapter_def *ap);
void    hsc_logerr(struct adapter_def *ap, int errid, struct mbstruct *mbp,
	int ahs, int errnum, int data1);
#ifdef	HSC_NEVER
void    hsc_internal_trace(struct adapter_def *ap, int hookid, int data1,
	int data2, int data3, int data4, int data5);
#endif HSC_NEVER
#ifdef HSC_TRACE
void    hsc_internal_trace(struct adapter_def *ap, int hookid, int data1,
	int data2, int data3);
#endif  HSC_TRACE
int     hsc_read_POS(struct adapter_def *ap, uint offset);
int     hsc_write_POS(struct adapter_def *ap, uint offset, uchar data);
int     hsc_pio_function(caddr_t parms);
int     hsc_pio_recov(caddr_t parms, int action, struct pio_except *infop);
void    hsc_deq_active(struct adapter_def *ap, struct sc_buf *scp, 
        int dev_index);

int	hsc_alloc_tgt(struct adapter_def *ap,int arg,ulong devflag);
int	hsc_dealloc_tgt(struct adapter_def *ap, int arg, ulong devflag);
int	hsc_enable_buf(struct adapter_def *ap,struct b_link *buf,
			uchar option, uchar level, uchar tag);
int	hsc_enbuf_cmd(struct adapter_def *ap, struct b_link *p,
			uchar option, uchar level);
int	hsc_get_tag(struct adapter_def *ap);
int	hsc_tgt_tcw_alloc(struct adapter_def *ap, struct b_link *p);
int	hsc_free_tmbufs(struct adapter_def *ap, int dev_index);
void	hsc_free_rdbufs(struct adapter_def *ap, uchar id, uchar flag);
int	hsc_tgt_tcw_dealloc(struct adapter_def * ap,struct b_link *buf,
			int forced_dealloc);
void	hsc_free_tm_garbage(struct adapter_def * ap);
void	hsc_free_gwrite(struct adapter_def * ap);
int	hsc_enable_id(struct adapter_def * ap, uchar id,
			uchar option, uchar level);
int	hsc_enaid_cmd(struct adapter_def * ap, uchar id,
			uchar	option, uchar level);
void	hsc_start_bufs(struct adapter_def * ap);
int	hsc_start_ids(struct adapter_def *ap);
int	hsc_stop_ids(struct adapter_def *ap);
void	hsc_buf_free(struct tm_buf *tm_ptr);
void	hsc_free_a_buf(struct b_link *tm_ptr, uchar flag);
void	hsc_tgt_DMA_err(struct adapter_def * ap);
int	hsc_async_notify(struct adapter_def * ap, int dev_index,
	int events);
void	hsc_need_disid(struct adapter_def * ap, struct dev_info * user,
			uchar id);
void	hsc_need_enaid(struct adapter_def * ap, struct dev_info * user,
			uchar id);

#else
/*****************************************************************************/
/*     internal functions                                                    */
/*****************************************************************************/

int     hsc_config();
int     hsc_open();
int     hsc_close();
int     hsc_strategy();
int     hsc_ioctl();
int     hsc_intr();
int     hsc_iodone();
struct  sc_buf *hsc_bld_sc_buf();
void    hsc_ioctl_sleep();
void    hsc_release();
void    hsc_start();
int     hsc_start_dev();
void    hsc_build_and_send();
void    hsc_issue();
int     hsc_MB_alloc();
void    hsc_MB_dealloc();
void    hsc_MB_restore();
int     hsc_TCW_alloc();
void    hsc_TCW_dealloc();
int     hsc_STA_alloc();
void    hsc_STA_dealloc();
void    hsc_build_mb30();
void    hsc_watchdog();
struct  cdt *hsc_cdt_func();
int     hsc_dump();
int     hsc_dumpwrite();
void    hsc_DMA_err();
void    hsc_dev_DMA_err();
int     hsc_dma_cleanup();
void    hsc_MB30_handler();
int     hsc_epow();
struct  adapter_def *hsc_alloc_adap();
void    hsc_free_adap();
int     hsc_config_adapter();
int     hsc_alloc_dev();
int     hsc_dealloc_dev();
int     hsc_halt_lun();
int     hsc_reset_dev();
int     hsc_inquiry();
int     hsc_readblk();
int     hsc_startunit();
int     hsc_testunitready();
int     hsc_diagnostic();
int     hsc_diag_test();
int     hsc_wrap_test();
int     hsc_reg_test();
int     hsc_pos_test();
int     hsc_reset_test();
int     hsc_download();
void    hsc_enable_cmd_q();
void    hsc_dnld_start_devs();
void    hsc_fail_cmd();
int     hsc_scsi_reset();
void    hsc_logerr();
#ifdef HSC_TRACE
void    hsc_internal_trace();
#endif HSC_TRACE
int     hsc_read_POS();
int     hsc_write_POS();
int     hsc_pio_function();
int     hsc_pio_recov();
void    hsc_deq_active();
int	hsc_alloc_tgt();
int	hsc_dealloc_tgt();
int	hsc_enable_buf();
int	hsc_enbuf_cmd();
int	hsc_get_tag();
int	hsc_tgt_tcw_alloc();
void	hsc_free_tmbufs();
void    hsc_free_rdbufs();
int	hsc_tgt_tcw_dealloc();
void	hsc_free_tm_garbage();
void	hsc_free_gwrite();
int	hsc_enable_id();
int	hsc_enaid_cmd();
void	hsc_start_bufs();
int	hsc_start_ids();
int	hsc_stop_ids();
void	hsc_buf_free();
void	hsc_free_a_buf();
void	hsc_tgt_DMA_err();
int	hsc_async_notify();
void	hsc_need_disid();
void	hsc_need_enaid();


#endif /* not _NO_PROTO */

#endif /* _H_HSCSIDD */
