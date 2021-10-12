/* @(#)46     1.48.1.5  src/bos/kernel/sys/POWER/mpqpdd.h, sysxmpqp, bos411, 9434B411a 8/22/94 16:35:37 */

#ifndef	_H_MPQPDD
#define	_H_MPQPDD

/*---------------------------------------------------------------------------
 *
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqpdd.h - MPQP device driver internal header file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *---------------------------------------------------------------------------
 */                                                                   

#include <sys/listmgr.h>
#include <sys/intr.h>
#include <sys/types.h>
#include <sys/listmgr.h>
#include <sys/lockl.h>


#ifdef _POWER_MP
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>



/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
� Define the locking MACROs for the MPQP Device Driver                   �
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
#define MPQP_LOCK_DISABLE(lvl,lock)     lvl=disable_lock(INTCLASS1, lock)
#define MPQP_UNLOCK_ENABLE(lvl,lock)    unlock_enable(lvl, lock)
#define MPQP_SIMPLE_LOCK(lock)          simple_lock(lock)
#define MPQP_SIMPLE_UNLOCK(lock)        simple_unlock(lock)
#endif /* _POWER_MP */


/**************************************************************************
 *  Multi Protocol Quad	Port Device Driver Defines			  *
 **************************************************************************/

#define	ACMD_ACQ	(unsigned short)0
#define	IO_SEG_REG	0x000c0020
#define	MAX_ADAPTERS	8		/* number of adapters supported	 */ 
#define	NUM_PORTS	4		/* number of ports supported	 */
#define	NUM_SLOTS	8 		/* number of slots supported	 */
#define MAX_BUSES	2   		/* number of buses supported     */
#define	NUM_MPQP_TCWS  64		/* number of TCWs per MPQP adapter */
#define	NUM_RECV_TCWS  16		/* number of receive TCWs per MPQP */
#define	NUM_XMIT_TCWS  48		/* number of transmit TCWs per MPQP */
#define NUM_PORT_TCWS  12		/* number of xmit TCWs per port	 */
#define	N_TXFREE       52		/*				 */
#define TX_SHORT_MAX   48		/* max # bytes in TX SHORT	 */

#define	POS0	       0x100		/* POS Register	0 IOCC offset	 */
#define	P0_F		0x70		/* POS Card ID low, MPQP	 */
#define	POS1	       0x101		/* POS Register	1 IOCC offset	 */
#define	P1_F		0x8F		/* POS1	Card ID	high, MPQP	 */

#define	POS2	       0x102		/* POS Register	2 IOCC offset	 */
#define	P2_ENABLE	0x01		/* -sleep/+ENABLE */
#define P2_INT3		0x00		/* interrupt level 3 mask */
#define P2_INT4		0x02		/* interrupt level 4 mask */
#define P2_INT7		0x04		/* interrupt level 7 mask */
#define P2_INT9		0x06		/* interrupt level 9 mask */
#define P2_INT10	0x08		/* interrupt level 10 mask */
#define P2_INT11	0x0A		/* interrupt level 11 mask */
#define P2_INT12	0x0C		/* interrupt level 12 mask */
#define	P2_SYNC_CHCK	0x80		/* Channel Check Mode = Sync */

#define	POS3	       0x103		/* POS Register	3 IOCC offset	 */

#define	POS4	       0x104		/* POS Register	4 IOCC offset	 */
#define	P4_WSIZ_8K	0x00		/* POS4	Window Size 8K */
#define	P4_WSIZ_16K	0x20		/* POS4	Window Size 16K	*/
#define	P4_WSIZ_32K	0x40		/* POS4	Window Size 32K	*/
#define	P4_WSIZ_64K	0x60		/* POS4	Window Size 64K	*/
#define	P4_WSIZ_128K	0x80		/* POS4	Window Size 128K */
#define	P4_WSIZ_512K	0xA0		/* POS4	Window Size 512K */
#define	P4_WSIZ_1M	0xC0		/* POS4	Window Size 1M */
#define	P4_WSIZ_2M	0xE0		/* POS4	Window Size 2M */

#define	POS5	       0x105		/* POS Register	5 IOCC offset	 */
#define	P5_FAIRNESS	0x01		/* POS5	Fairness Enable	*/
#define	P5_PAREN	0x20		/* POS5	Data Parity Enable */
#define	P5_CHCKS	0x40		/* POS5	I/O Channel Check Status */
#define	P5_CHCKI	0x80		/* POS5	I/O Channel Check Indicator */

#define	POS6	       0x106		/* POS Register	6 IOCC offset	 */
#define	POS7	       0x107		/* POS Register	7 IOCC offset	 */

#define	WINDOW_SIZE    0x10000		/* adapter window size,	64K	 */

/*******************************************************************
  *  I/O Register Offsets from start of I/O Memory or DREG Values  *
  ******************************************************************/

#define	LOCREG0		0x00		/* Location Register 0 (LOCREG0) */
#define	LOCREG1		0x01		/* Location Register 1 (LOCREG1) */
#define	PTRREG		0x02		/* Pointer Register	(PTRREG) */
#define	DREG		0x03		/* Data	Register	  (DREG) */
#define	TASKREG		0x04		/* Mailbox Register    (TASKREG) */
#define	CPUPAGE		0x05		/* CPU Page Register   (CPUPAGE) */
#define	COMREG		0x06		/* Command Register	(COMREG) */
#define	PROCSYNC	0x07		/* Processor Ownership Register  */

#define	INITREG2	0x08		/* Init. Register 2   (INITREG2) */
#define	INTCOM		0x09		/* Adapter Interrupt	(INTREG) */
#define	PCPAR0		0x0A		/* Parity Register 0	(PCPAR0) */
#define	PCPAR1		0x0B		/* Parity Register 1	(PCPAR1) */
#define	GAID		0x0F		/* Gate	Array ID	  (GAID) */
#define	INITREG1	0x10		/* Init. Register 1   (INITREG1) */
#define	PCPAR2		0x11		/* Parity Register 2	(PCPAR2) */
#define	INITREG0	0x12		/* Init. Register 0   (INITREG0) */
#define	INITREG3	0x13		/* Init. Register 3   (INITREG3) */
#define	PCCSTAT		0x14		/* CHCK Status Reg.    (PCCSTAT) */
#define	CAD_EN		0x15		/* Host Reset Enable	(CAD_EN) */

/**************************************************************************
 *   MPQP PCPAR2 VALUES						 	 *
 **************************************************************************/

#define PCP2_SYNC_CHCK	0x40		/* Synchronous IOCHCK   .PCPAR2  */
#define PCP2_EN_CHCK	0x20		/* Enable IOCHCK	.PCPAR2  */
 
/**************************************************************************
 *   MPQP GAID VALUES						 	 *
 **************************************************************************/

#define GA_CNTNDR_3	0x80		/* GAID, Contender 3	  .GAID  */
#define GA_CNTNDR_4	0x81		/* GAID, Contender 4	  .GAID  */
#define GA_CNTNDR_5	0x82		/* GAID, Contender 5	  .GAID  */

/**************************************************************************
 *   MPQP COMREG VALUES						 	 *
 **************************************************************************/

#define COM_RC		0x01		/* Reset Card		.COMREG  */
#define COM_IE		0x10		/* Interrupt Enable	.COMREG  */
#define COM_IP		0x20		/* Interrupt Pending	.COMREG  */

/**************************************************************************
 *   MPQP TASKREG VALUES						  *
 **************************************************************************/

#define	TR_ARQ_I	(unsigned char)0x00	/* ARQ Now non-empty (int) */
#define	TR_TXFL		(unsigned char)0x01	/* Tx Free List	non-empty */
#define TR_DMA0		(unsigned char)0x80	/* DMA TX Ack, Port 0 */
#define TR_DMA1		(unsigned char)0x81	/* DMA TX Ack, Port 1 */
#define TR_DMA2		(unsigned char)0x82	/* DMA TX Ack, Port 2 */
#define TR_DMA3		(unsigned char)0x83	/* DMA TX Ack, Port 3 */
#define	TR_WDE		(unsigned char)0xFE	/* Watchdog timer expired */
#define	TR_NOI		(unsigned char)0xFF	/* No interrupt	pending	*/

/***************************************************************************
 *  Internal Port States...see port_state variable in dds device section   *
 ***************************************************************************/

#define	DORMANT_STATE		0x00	/* initial state */
#define	OPEN_REQUESTED		0x01	/* Open	in progress */
#define	OPEN			0x02	/* Port	opened */
#define	START_REQUESTED		0x03	/* Start in progress */
#define	STARTED			0x04	/* Port	started	*/
#define	DATA_XFER		0x04	/* Data	tranfer	state */
#define	HALT_REQUESTED		0x05	/* Halt	in progress */
#define	HALTED			0x02	/* Port	halted */
#define	CLOSE_REQUESTED		0x07	/* Close requested */
#define	CLOSED			0x00	/* Port	closed */

/**************************************************************************
 *   MPQP TRACE HOOK CONSTANTS						  *
 **************************************************************************/

#define PORT_NOT_OPEN		0xfe		/* Port State != OPEN */
#define PORT_NOT_STARTED	0xff		/* Port State != STARTED */
#define	PIN_CODE_FAIL		0x100		/* pincode attempt failed */
#define ADD_ENTRY_FAIL		0x101		/* 	*/
#define PORT_ALRDY_OPEN		0x102		/* 	*/
#define POS_REG_FAIL		0xfa		/*	*/

/**************************************************************************
 *   MPQP MEMORY PAGE 0	OFFSETS						  *
 **************************************************************************/

#define	PCSEL	       0x440
#define	MAXTASK	       0x444
#define	MAXPRI	       0x445
#define	MAXQUEUE       0x446
#define	MAXTIME	       0x447
#define	BCB	       0x45A		/* Buffer Control Block	offset */
#define	PSB	       0x47C		/* Primary Status Byte offset */

/**************************************************************************
 *   MPQP PSB BIT DEFINITIONS						  *
 **************************************************************************/

#define	LOADED		(unsigned char)0x01	/* PSB "LOADED"	bit */
#define	BUSY		(unsigned char)0x40	/* PSB "BUSY" bit */

/**************************************************************************
 *   MPQP POWER	ON SELF	TEST DEFINITIONS				  *
 **************************************************************************/

#define	IF_BLK		0x400		/* Page	0 address, Interface Block*/
#define	ERRLOG_PTR	0x14		/* Offset, Error Log for POST */
#define	STATOFF		0x7c		/* Offset, primary & secondary stat */
#define	ROSREADY	0x40		/* ROS Ready Bit, INITREG1 */

/* Port	Control	Commands    */

#define	XMIT_SHORT	0x10		/* Transmit Short		*/
#define	XMIT_LONG	0x11		/* Transmit Long		*/
#define	XMIT_GATHER	0x12		/* Transmit Gather		*/
#define	RCV_BUF_INDC	0x13		/* Receive Buffer Indicate	*/
#define	SET_PARAM	0x21		/* Set Parameters		*/
#define	START_PORT	0x22		/* Start Port			*/
#define	STOP_PORT	0x23		/* Stop	Port			*/
#define	TERM_PORT	0x24		/* Terminate Port		*/
#define	FLUSH_PORT	0x25		/* Flush Port			*/
#define	QURY_MDM_INT	0x2a		/* Query Modem Interrupts	*/
#define	STRT_AUTO_RSP	0x2b		/* Start Auto Response		*/
#define	STOP_AUTO_RSP	0x2c		/* Stop	Auto Response		*/
#define	CHG_PARAM	0x2d		/* Change Parameters		*/

/* Port Command Modifiers */

#define ADAP_TX_ACK	0x80		/* Tx ack for Transmit command	*/
#define ADAP_TRANSP	0x40		/* Transparent mode */
#define ADAP_DMA_ACK	0x01		/* DMA ack for Transmit command	*/

/* Adapter Commands    */

# define HALT_RECV	0x30		/* Halt frame reception */
# define START_RECV	0x31		/* Start frame reception */

/* Adapter Constants	*/

#define ADAP_TX_AREA	0x50000		/* Adapter TX buffer area */
#define ADAP_BUF_SIZE	4096		/* Size of TX and RX buffers */

/*------------------------------------------------------------------------*/
/*  Adapter Reset:							  */
/*------------------------------------------------------------------------*/

# define RESET_TIMEOUT	8		/* eight seconds */
# define DL_DELAY_REG   0xE0            /* hardware delay register */


/* Adapter States: */

# define UNKNOWN	0	/* adapter is in an unknown state */
# define RESET		1	/* adapter has been reset */
# define INITIALIZED	2	/* adapter is reset and initialized */
# define RESETTING	3	/* adapter is being reset */
# define SUSPENDED	0x80	/* adapter is waiting for command blocks */

/*------------------------------------------------------------------------*/
/*  MPQP Threshold Defaults						  */
/*------------------------------------------------------------------------*/

# define ERRTHRESH 	1	/* log every error */
# define THRESHPERCENT 	0	/* log every error */
/*------------------------------------------------------------------------*/
/*  Queue definitions:							  */
/*------------------------------------------------------------------------*/

typedef struct {
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last element of queue */
	unsigned char	out;		/* first item to remove */
	unsigned char	in;		/* last item inserted */
	unsigned long	q_elem[ 1 ];	/* queue elements */
} queue_t;

typedef queue_t	 long_queue_t;

typedef struct {
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last element of queue */
	unsigned char	out;		/* first item to remove */
	unsigned char	in;		/* last item inserted */
	unsigned char	q_elem[ 1 ];	/* queue elements */
} byte_queue_t;

/*------------------------------------------------------------------------*/
/*  Transmit/Receive Map definition:				  	  */
/*------------------------------------------------------------------------*/

typedef struct {
    struct mbuf		*p_mbuf;	/* pointer to associated mbuf */
    char		*p_data;	/* data starts here */
    unsigned long	bus_addr;	/* bus address of data */
} recv_map_t;

typedef struct {
    int			map_free;	/* TRUE if map is free */
    unsigned long	write_id;	/* write ID of last write */
    struct mbuf		*m;		/* mbuf to free */
    char		*dma_addr;	/* page to DMA */
    unsigned long	bus_addr;	/* bus address page */
    unsigned long	flags;		/* saved write flags */
} xmit_map_t;

/*------------------------------------------------------------------------*/
/*  Device Definitions Structure:					  */
/*------------------------------------------------------------------------*/

typedef	struct	 MPQDDS
{
    struct DDS_HDW
    {
	unsigned int	slot_num;	/* slot	number of adapter */
	unsigned int	bus_intr_lvl;	/* interrupt level */
	unsigned short	intr_priority;	/* interrupt priority */
	unsigned short	dma_lvl;	/* this is the bus arbitration level */
					/* for this adapter */
	unsigned int	bus_io_addr;	/* base of Bus I/O area for this */
					/* adapter */
	unsigned int	bus_mem_addr;	/* base of Bus Memory "Shared" */
					/* addressability for this adapter */
	unsigned int	tcw_bus_mem_addr; /* base of Bus Memory DMA */
					  /* addressability for this adapter */
        unsigned long   bus_id;         /* bus id */
    } dds_hdw;

    struct DDS_DVC
    {
	unsigned char	port_num;	/* Port	Number for this	port */
	unsigned char	port_state;	/* Port	State */
	unsigned short	rdto;		/* Receive Data Transfer Offset */
	int		net_id;		/* Network ID */
	unsigned short	max_rx_bsiz;	/* maximum receive buffer size */
    } dds_dvc;

    struct DDS_RAS
    {
	t_cio_stats cio_stats;	        /* threshold accumulators	  */
	t_err_threshold err_thresh;	/* threshold values - set by app. */
    } dds_ras;

    struct DDS_VPD
    {
	unsigned short	card_id;	/* Card	ID...POS0 & POS1 */
	unsigned short	ver_num;	/* Version Number */
	char		devname[16];	/* logical device name */
	char		adpt_name[16];	/* logical adpater name */
    } dds_vpd;

    struct DDS_WRK
    {
					/* transmit map table: */
	xmit_map_t	xmit_map_table[ NUM_PORT_TCWS ];
	unsigned short	xmit_enabled;	/* transmit enable flag */

	unsigned short	cmd_seq_num;	/* sequence number of command      */
	unsigned short	cur_chan_num;	/* current channel number          */
	unsigned char	num_starts;	/* number of starts issued on port */
					/* incremented by successful ioctl */
					/* with	CIO_START operator.	   */
					/* decremented by successful ioctl */
					/* with	CIO_HALT operator.	   */

	unsigned char	xmt_ld_flg;	/* flag indicating that the transmit */
					/* chain has been loaded...	     */


	t_chan_info	*p_chan_info[MAX_CHAN];	/* open/select info         */
	unsigned char	modem_intr_mask;
	unsigned char	phys_link;
	unsigned char	field_select;
	unsigned char	dial_proto;
	unsigned char	dial_flags;
	unsigned char	data_proto;
	unsigned char	data_flags;
	unsigned char	modem_flags;
	unsigned char	poll_addr;
	unsigned char	select_addr;
	unsigned char	baud_rate;
	unsigned char	modem_status;
	unsigned short	rcv_timeout;
	unsigned char	cmd_avail_flag;
	struct trb	*ndelay_timer;
	unsigned int	ndelay_timer_pop;
	int             halt_sleep_event;
	int             sleep_on_halt;
	int 		buff_ctr; 	/* rx buffer counter */
	union
	{
		t_x21_data	x21_data;
		t_auto_data	auto_data;
	} t_dial;
    } dds_wrk;
} t_mpqp_dds;

/*------------------------------------------------------------------------*/
/*  Adapter Command Block definition:					  */
/*------------------------------------------------------------------------*/

typedef struct {
    unsigned char		type;		/* Command Type */
    unsigned char		port;		/* Port Number */
    unsigned short		sequence;	/* Sequence Number */
    unsigned short		length;		/* Length of data */
    unsigned short		flags;		/* Control flags */
    unsigned char 		*host_addr;	/* Host address of data */
    unsigned char 		*card_addr;	/* Card address of data */

    union {					/* COMMAND SPECIFIC: */
						/*  TX Short Data */
        unsigned char		data[ TX_SHORT_MAX ];

	struct {				/*  Set Parm */
	    unsigned char	field_sel;	/*    field select */
	    unsigned char	modem_int;	/*    modem interupt */
	    unsigned char	phys_link;	/*    physical link */
	    unsigned char	poll_addr;	/*    poll address */
	    unsigned char	select_addr;	/*    select address */
	    unsigned char	dial_proto;	/*    autodial protocol */
	    unsigned char	dial_flags;	/*    dial flags */
	    unsigned char	data_proto;	/*    data protocol */
	    unsigned short	data_flags;	/*    data flags */
	    unsigned short	recv_timer;	/*    receive timer */
	    unsigned short	baud_rate;	/*    baud rate */
	} set_parm;

	struct autoresp {			/* Auto Response */
	    unsigned char	time_hi;	/*    Time, High */
	    unsigned char	time_lo;	/*    Time, Low */
	    unsigned char	address;	/*    TX/RX Address */
	    unsigned char	xmit_ctrl;	/*    transmit control */
	    unsigned char	recv_ctrl;	/*    receive control */
	    unsigned char	unused;
	} auto_resp;

	struct autodial {			/* Auto Dial */
	    unsigned char	modem_flags;	/* modem flags */
	    unsigned char	unused;
	    unsigned short	rdto;		/* Rcvd Data Transfer Offset */
	    unsigned short	connect_timer;	/* time to wait for DSR */
	    unsigned short	v25b_tx_timer;	/* wait time to xfer data */
	    unsigned short	max_rx_bsiz;	/* Maximum receive buffer size*/
	} auto_dial;

	struct {
	    unsigned char       parm;   	/* Command parameter */
	    unsigned char       filler; 	/* Not used */
	    unsigned short      RDTO;   	/* Receive data offset */
	    unsigned short	filler1;	/* time to wait for DSR */
	    unsigned short	filler2;	/* wait time to xfer data */
	    unsigned short	max_rx_bsiz;	/* Maximum receive buffer size*/
	} start_port;
   } cs;
} adap_cmd_t;

/*------------------------------------------------------------------------*/
/*  Offlevel Interrupt definition:					  */
/*------------------------------------------------------------------------*/

typedef struct OFFL_INTR
{
	struct intr     offl_intr;      /* system wide bit */
	struct t_acb    *p_acb_intr;    /* pointer to acb for i_sched */
} t_offl_intr;

/*-----------------------------------------------------------------------*/
/*  Junk definitions (to be thrown out)					 */
/*-----------------------------------------------------------------------*/

#define	SET_CPUPAGE( p,	s, v ) {\
	if ( p->cpu_page != v )\
	{\
	    p->cpu_page = v;\
	    PIO_PUTC( (unsigned long)(p->io_base) | s + CPUPAGE, v);\
	}}

#define	SET_ACMDREG( p, s, v ) {\
	if ( p->adap_cmd_reg !=	v )\
	{\
	    p->adap_cmd_reg = v;\
	    PIO_PUTS( (unsigned long)(p->p_adap_cmd_reg) | s, v );\
	}}

/*
 *  Old MPQP Byte and Word queue structure types:
 */

typedef	struct
{
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last	element	of queue */
	unsigned char	out;		/* first item to remove	*/
	unsigned char	in;		/* last	item inserted */
	unsigned char	bqueue[1];	/* byte	queue elements */
}t_byte_queue;

typedef	struct
{
	unsigned char	length;		/* length of queue */
	unsigned char	end;		/* last	element	of queue */
	unsigned char	out;		/* first item to remove	*/
	unsigned char	in;		/* last	item inserted */
	unsigned int	wqueue[1];	/* word	queue elements */
}t_word_queue;

/*
 *  Old Adapter Command Block
 */

typedef	struct ADCMDB
{
	unsigned char	cmd_typ;	/* diagnostic command */
	unsigned char	port_nmbr;	/* port	number for command	*/
	unsigned short	seq_num;	/* command sequence number	*/
	unsigned short	rsrvd_1;	/* filler			*/
	unsigned char	lngth;		/* byte	length for data		*/
	unsigned char	cntrl;		/* control information		*/
	char		*p_edrr;	/* pointer to response region	*/
	unsigned int	rsrvd_2;	/* filler			*/
	union
	{
	    struct
	    {
		char		data[48]; /* data area associated with	  */
					  /* this command.		*/
	    }d_ovl;
	   struct
	   {
		unsigned int	tst_addr;
		unsigned short	tst_length;
		unsigned char	cntl;
		unsigned char	fyller[41];
	    }c_ovl;
	}u_data_area;
}t_adap_cmd;

/*------------------------------------------------------------------------*/
/*  Adapter Control Block definition:					  */
/*------------------------------------------------------------------------*/

typedef	struct ACB
{
	struct intr     slih_intr;      /* interrupt handler structure  */
	t_offl_intr     offl;           /* offlevel interrupt structure */
	t_offl_intr     dmaoffl;        /* dma offlevel intr structure  */
	int		rasw_sleep;	/* reload adapter software sleep cell */
	char		*p_dma_tst_buf;	/* pointer to test for bus master dma */
	unsigned int	arb_lvl;	/* MicroChannel	Arbitration Level */
	unsigned int	int_lvl;	/* interrupt level this	adapter	 */
					/* responds to			 */
	unsigned int	int_pri;	/* interrupt priority */
	unsigned int	offl_lvl;	/* offlevel level for interrupts */
					/* from	this adapter		 */
	unsigned int	offl_pri;	/* offlevel priority		 */
	int		txfl_event_lst;	/* transmit free list event list */
	unsigned char	pos0;		/* POS Register	0 Value	*/
	unsigned char	pos1;		/* POS Register	1 Value	*/
	unsigned char	pos2;		/* POS Register	2 Value	*/
	unsigned char	pos3;		/* POS Register	3 Value	*/
	unsigned char	pos4;		/* POS Register	4 Value	*/
	unsigned char	pos5;		/* POS Register	5 Value	*/
	unsigned char	pos6;		/* POS Register	6 Value	*/
	unsigned char	pos7;		/* POS Register	7 Value	*/
	unsigned char	slot_num;	/* slot	number adapter is in	*/
	unsigned char	adapter_state;	/* 0 - uninitialized		*/
					/* 1 - initialization begun	*/
					/* 2 - initialization complete	*/
					/* 3 - reset requested		*/
					/* 0x80 - Suspended: Or Mask	*/
	unsigned char	diag_flag;	/* 0 - no diagnostic mode	*/
					/* 1 - diagnostic open requested*/
					/* 2 - opened for diagnostics	*/
					/* Note: value of 1 set	in	*/
					/* mpqmpx on request for open	*/
					/* and value of	two set	in	*/
					/* mpqopen upon	successful open	*/
					/* for diagnostics.		*/
	unsigned char	asw_load_flag;	/* 0 - Adapter software	is not	*/
					/*     loaded			*/
					/* 1 - Load completed		*/
					/* 0xff	- locked		*/
	unsigned char	cur_intr_val;	/* current interrupt value	 */
	unsigned char	n_cfg_ports;	/* number of ports configured 	 */
					/* on this adapter		 */
	unsigned char	n_open_ports;	/* number of ports opened on	 */
					/* this	adapter			 */
	unsigned char	ds_base_page;	/* CPUPAGE value for data struct-*/
					/* ures	on adapter		 */
	unsigned char	cpu_page;	/* BUSIO Addr -	5		*/
					/* cpu_page is a copy of the	*/
					/* last	value written in shared	*/
					/* memory of the adapter.	*/
	unsigned char	num_starts;	/* aggregate number of starts	*/
					/* on this adapter.  incremented */
					/* on successful start,	decremented */
					/* on successful halt.	used to	 */
					/* determine when to allocate and */
					/* deallocate receive mbuf/tcw(s)*/
	unsigned char	rcv_buf_ind_snt;/* receive buffer indicates sent */
					/* to the adapter...this flag	 */
					/* should be set when the first	 */
					/* successful start port takes	 */
					/* place and reset when	adapter	 */
					/* software is reloaded		 */
	unsigned char	adap_cmd_que_in;/* index to the next place to	*/
					/* receive a command number in  */
					/* the adapter command queue	*/
	lock_t		cmd_queue_lock;	/* lock to access cmd queue */
	unsigned short	adap_cmd_reg;	/* adap_cmd_reg	is a copy of the */
					/* last	value written in shared	 */
					/* memory of the adapter.	 */
	int		dma_channel_id;	/* DMA Channel ID returned from	 */
					/* d_init call			 */
	unsigned long	io_base;	/* base	io address		*/
	unsigned long	mem_base;	/* base	memory address		*/
	unsigned long	dma_base;	/* base	address	of bus memory	*/
					/* for this adapter, set in	*/
					/* mpqconfig			*/
	unsigned long	io_segreg_val;	/* Segment register value for	*/
					/* io space indicator		*/
        unsigned long   iocc_segreg_val;/* Segment register value for   */
                                        /* iocc space indicator         */
	t_mpqp_dds	*p_port_dds[NUM_PORTS];	 /* an array of	pointers to */
						 /* device data	structures  */
						 /* for	all the	ports for   */
						 /* this current adapter    */
	t_byte_queue	*p_txfree_q;	/* pointer to the transmit free	*/
					/* buffer queue	data structure	*/
	t_byte_queue	*p_adap_cmd_que; /* pointer to the adapter com-	 */
					/* mand	queue data structure	*/
	t_word_queue	*p_adap_rsp_que; /* pointer to the adapter re-	 */
					/* sponse queue	data structure	 */
	unsigned short	*p_adap_cmd_reg; /* pointer the	the adapter command  */
					 /* register			    */
	unsigned short	*p_num_cmd;	/* pointer to number of	commands */
	unsigned short	*p_num_rcv_buf;	/* pointer to number of	receive	 */
					/* buffers			 */
	unsigned short	*p_rcv_buf_siz;	/* pointer to receive buffer size */
	unsigned short	*p_rcv_buf_para_num; /*	pointer	to receive buffer */
					     /*	paragraph number (addr)	  */
	unsigned short	*p_num_xmit_buf; /* pointer to number of xmit	 */
					/* buffers			 */
	unsigned short	*p_xmit_buf_siz; /* pointer to xmit buffer size	*/
	unsigned short	*p_xmit_buf_para_num; /* pointer to xmit buffer	*/
					     /*	paragraph number (addr)	  */
					/* Per port pointer to extended	*/
	unsigned char	*p_edrr[NUM_PORTS];
					/* diagnostic response region */
	unsigned char	*p_adap_trc_data; /* pointer to	adapter	trace data */
					/* pointer to port trace data	*/
	unsigned char	*p_port_trc_data[NUM_PORTS];
	t_adap_cmd	*p_cmd_blk;	/* pointer to an array of adapter */
					/* command block data structures */
					/* local TX free buffer	queue */
	t_byte_queue	*p_lcl_txfree_buf_q;
					/* Receive map table */
	recv_map_t	recv_map_table[ NUM_RECV_TCWS ];
	unsigned short	recv_enabled;	/* receive enable flag */
	struct mbreq	mbuf_req;	/* mbuf requirements */
	unsigned int	c_rcv;		/* receive count	     */
	unsigned long	c_intr_rcvd;	/* interrupt counter	     */
	unsigned char	dma_sched;	/* flag for dma sched pending */
	unsigned char	arq_sched;	/* flag for arq sched pending */
	struct trb      *sleep_timer;   /* timer structure for sleep */
	unsigned int	sleep_timer_pop;/* timer pop flag */
	unsigned short	halt_complete;	/* indicator if HALT sequence */
					/* is completed any port      */
} t_acb;

/**************************************************************************
 *   MPQP MACRO	DEFINITIONS						  *
 **************************************************************************/

/* Little Endian <--> Big Endian conversion: */

# define SWAPSHORT(x)   (((((unsigned short)(x)) & 0xFF) << 8) | 	\
			  (((unsigned short)(x)) >> 8))

# define SWAPLONG(x)    (((((unsigned long)(x)) &       0xFF) << 24) | 	\
			 ((((unsigned long)(x)) &     0xFF00) <<  8) | 	\
			 ((((unsigned long)(x)) &   0xFF0000) >>  8) | 	\
			 ((((unsigned long)(x)) & 0xFF000000) >> 24))

/*----------------------------------------------------------------------*/
/*  M_INPAGE  for checking funky mbufs 					*/
/*  This macro determines if the data portion of an mbuf resides within	*/
/*  one page -- if TRUE is returned, the data does not cross a page	*/
/*  boundary.  If FALSE is returned, the data does cross a page 	*/
/*  boundary and cannot be d_mastered.					*/
/*----------------------------------------------------------------------*/

# define M_INPAGE(m)	((((int)MTOD((m), uchar *)			\
				& ~(PAGESIZE - 1)) + PAGESIZE) >	\
				    ((int)MTOD((m), uchar *) + (m)->m_len))

/*------------------------------------------------------------------------*/
/*  DDS Accessors:							  */
/*------------------------------------------------------------------------*/

# define DVC		p_dds->dds_dvc
# define HDW		p_dds->dds_hdw
# define WRK		p_dds->dds_wrk

/*------------------------------------------------------------------------*/
/*  Receive/Transmit Map Definitions:				  	  */	
/*------------------------------------------------------------------------*/

# define XMITMAP	(WRK.xmit_map_table)
# define RECVMAP	(p_acb->recv_map_table)

/*------------------------------------------------------------------------*/
/*  RQE definitions:							  */
/*------------------------------------------------------------------------*/

/* RQE Types: */

# define XMIT_COMPLETE		0x0		/* Transmit complete */
# define RECV_COMPLETE_DMA	0x1		/* Receive complete, DMA */
# define COMMAND_SUCCESS	0x2		/* Command complete, success */
# define SOL_STATUS		0x3		/* Solicited status */
# define FATAL_ERROR		0x6		/* Adapter error, fatal */
# define XMIT_ERROR		0x8		/* Transmit error */
# define RECV_COMPLETE		0x9		/* Recv complete, no DMA */
# define COMMAND_FAILURE	0xA		/* Command complete, failure */
# define UNSOL_STATUS		0xB		/* Unsolicited status */
# define RECOV_ERROR		0xE		/* Adapter error, recoverable */
# define DIAGNOSTIC_ERROR	0xF		/* Diagnostic error */

/* These RQE field accessors are to be used on RQE's that have	*/
/* already been swapped during read from the adapter (as done	*/
/* by the adapter queue routines):				*/

# define RQE_TYPE(rqe)		(((rqe) >> 4 ) & 0x0F)
# define RQE_PORT(rqe)		((rqe) & 0x0F)
# define RQE_COMMAND(rqe)	((unsigned  char)((rqe) >> 8))
# define RQE_SEQUENCE(rqe)	((unsigned short)((rqe) >> 16))
# define RQE_STATUS(rqe)	((unsigned short)((rqe) >> 16))
# define RQE_XESTATUS(rqe)	((unsigned  char)((rqe) >> 8))

/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS	  */
/*  accessor macros.  The essential difference is that retries are 	  */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1	  */
/*  is returned (hence all return an int value).  In the cases of 	  */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

# define C		1	/* Character type of PIO access */
# define S		2	/* Short type of PIO access */
# define SR		3	/* Short-reversed type of PIO access */
# define L		4	/* Long type of PIO access */
# define LR		5	/* Long-reverse type of PIO access */

# define PIO_GETC( a )		((int) PioGet( a, C ))
# define PIO_GETS( a )		((int) PioGet( a, S ))
# define PIO_GETL( a )		((int) PioGet( a, L ))
# define PIO_GETSR( a )		((int) PioGet( a, SR ))
# define PIO_GETLR( a )		((int) PioGet( a, LR ))

# define PIO_PUTC( a, v )	((int) PioPut( a, v, C ))
# define PIO_PUTS( a, v )	((int) PioPut( a, v, S ))
# define PIO_PUTL( a, v )	((int) PioPut( a, v, L ))
# define PIO_PUTSR( a, v )	((int) PioPut( a, v, SR ))
# define PIO_PUTLR( a, v )	((int) PioPut( a, v, LR ))

# define PIO_GETSTR( d, s, l )	((int) PioBusCopy( d, s, l ))
# define PIO_PUTSTR( d, s, l )	((int) PioBusCopy( d, s, l ))
# define PIO_XCHGC( a, v )	((int) PioXchgC( a, v ))

# define PIO_RETRY_COUNT	3

/*------------------------------------------------------------------------*/
/*  Queue Macros:							  */
/*------------------------------------------------------------------------*/

# define NEXT_IN( p_q )							\
	(((p_q)->in  == ((p_q)->end)) ? 0 : ((p_q)->in + 1))

# define NEXT_OUT( p_q )						\
	(((p_q)->out == ((p_q)->end)) ? 0 : ((p_q)->out + 1))

# define Q_FULL( p_q )							\
	((p_q)->out == NEXT_IN( p_q ))

# define Q_EMPTY( p_q )							\
	((p_q)->out == (p_q)->in)


#endif /* _H_MPQPDD */
