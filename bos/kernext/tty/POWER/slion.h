/* @(#)81 1.8 src/bos/kernext/tty/POWER/slion.h, sysxs64, bos411, 9435C411a 8/25/94 12:56:35 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_SLION
#define _H_SLION


/* All the bloody includes that we need */
/* For stream structures */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/strconf.h>

#include <sys/str_tty.h>
#include <termios.h>
#include <sys/termiox.h>

#ifdef _KERNEL

#include <sys/syspest.h>		/* For assert and ASSERT */
#include <sys/intr.h>
#include <sys/pin.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/eu.h>
#include <sys/errids.h>

#include "ttydrv.h"

					/* Compile time options */
#define DO_PIO				/* Do pio error logging */
#define VPD_BOX_RESET			/* Fix for box reset problem */
#define FEATURES			/* Do multi port stuff */

/* Define all the PIO stuff we need here *****************************/
#ifdef DO_PIO

/* The second argument to the func routine in the PIO stuff is 
 ** used to tell the recovery routine what type of error to log 
 ** if any. Values are:	
 * 0 - No Log Error
 * 1 - Log Permanent Error
 * 2 - Log Temporary Error
 */

/** The first arg is user defineable.*/

#include <sys/except.h>

typedef struct pud {
    struct pio_except p_except;
    int p_check;
    int p_mask;
} pud_t;

#define DoingPio    void (*PioF)(); volatile int Rtry; int Setrc;\
		    label_t JmpBuf; void *Arg; pud_t Pud
#define StartPio(func, arg, thunk)	\
{					\
    PioF = func;			\
    Arg = arg;				\
    Rtry=PIO_RETRY_COUNT;		\
    while (Setrc = setjmpx(&JmpBuf)) {	\
	if (Setrc == EXCEPT_IO ||	\
	    Setrc == EXCEPT_IO_IOCC) {	\
	    if (--Rtry <= 0) {		\
		if (PioF)		\
		    PioF(Arg, &Pud, 1);	\
		thunk;			\
	    } else if (PioF)		\
		PioF(Arg, &Pud, 0);	\
	} else				\
	    longjmpx(Setrc);		\
    }					\
    if (Rtry) {

#define EndPio()			\
	if (Rtry != PIO_RETRY_COUNT &&	\
		PioF)			\
	    PioF(Arg, &Pud, 2);		\
	clrjmpx(&JmpBuf);		\
    }					\
}

#else /* ~DO_PIO */
#define DoingPio 	int NotUsed
#define StartPio(a,b,c)
#define EndPio()
#endif /* DO_PIO */
/* END:Define all the PIO stuff *************************************/

#endif /* _KERNEL */

#define STACKSIZE		40
#define INTSIZE 		64
#define SISIZE			64

/*
** ======================== 
**    KEY_SEQ structure
** ======================== 
*/
struct key_seq {
    uchar 	pkeys[64];
    uchar 	goto1[4];
    uchar 	goto2[4];
    uchar 	go_bott[4];
    uchar 	out_bott[4];
    uchar 	in_xpar[11];
    uchar 	lv_xpar[11];
    uchar 	in_bott[11];
    uchar 	lv_bott[11];
    uchar 	in_main[11];
    uchar 	in_alt[11];
};

/*
** ======================== 
**      RING structure
** ======================== 
*/
struct ring {				/* Ring buffer definition */
    ushort 	buf_beg;		/* Ptr of the beginning of the buf.*/
    ushort 	buf_end;		/* Pointer of the end of the buffer*/
    ushort 	buf_head;		/* The head pointer into the buffer*/
    ushort 	buf_tail;		/* The tail pointer into the buffer*/
    ushort 	buf_almost;		/*  */
    ushort 	buf_max;		/* Ring buffer size -1 */
    ushort 	buf_fill;
};

/*
** ======================== 
**      IPU structure
** ======================== 
**
** This structure is defined for each port and they are used
** for communicating commands and data with the BOX on a port by
** port basis.
*/
struct ipu {
    struct ring b_tr_main;		/* TX main */
    struct ring b_tr_alt;		/* TX alternate */
    struct ring b_tr_bot;
    struct ring b_rc_main;		/* RX main */
    struct ring b_rc_alt;		/* RX alternate */
    struct ring b_rc_bot;
    struct ring b_tr_xpar;		/* TX transparent */
    struct ring b_tr_raw;
    struct ring b_rc_raw;
    ushort 	pkey_add;
    ushort 	prog_end;
    ushort 	glob_add;		/* Offset of global buffer */
    ushort 	tr_stack[STACKSIZE];
    ushort 	rc_stack[STACKSIZE];
    ushort 	save_sps[2];
    ushort 	tr_width;		/* Width of Tx chunks */
    ushort 	rc_width;		/* Width of Rx chunks */
    ushort 	scc_comm;
    ushort 	scc_data;
    ushort 	x_full;
    ushort 	x_empty;
    ushort 	tr_point;
    ushort 	tr_count;
    ushort 	strip_point;
    ushort 	strip_count;
    ushort 	rc_current;
    ushort 	nbreak;
    ushort 	held;
    ushort 	t1;
    ushort 	t2;
    uchar 	cur_screen;
    uchar 	cur_xparent;
    uchar 	cur_bottom;
    uchar 	cur_dcd;		/* DCD status */
    uchar 	cur_cts;		/* CTS status */
    uchar 	cur_sync;
    uchar 	got_break;
    uchar 	rc_word;
    uchar 	cmask;
    uchar 	wr3_image;
    uchar 	wr4_image;
    uchar 	wr5_image;
    uchar 	wr14_image;
    uchar 	rr0_image;
    uchar 	rep_break;
    uchar 	util_break;
    uchar 	rep_dss;
    uchar 	util_cts;
    uchar 	util_dcd;
    uchar 	tr_int;
    uchar 	rc_int;
    uchar 	progr;
    uchar 	multi_s;
    uchar 	botline;
    uchar 	transp;
    uchar 	crlf;
    uchar 	xp_crlf;
    uchar 	features;
    uchar 	xp_prior;
    uchar 	last_screen;
    uchar 	shaking;
    uchar 	x_rc;
    uchar 	x_tr;
    uchar 	x_inbusy;
    uchar 	x_outbusy;
    uchar 	x_issue;
    uchar 	x_inhibit;
    uchar 	char_size;
    uchar 	stop_bits;
    uchar 	parity;
    uchar 	dtr;
    uchar 	rts;
    uchar 	baud;
    uchar 	timer_low;
    uchar 	timer_high;
    uchar 	build[4];
    uchar 	comm_byte;
    uchar 	comm_param;
    uchar 	dummyx[24];
    uchar 	err_int;
};

/*
** ======================== 
**     IPMEM structure
** ======================== 
*/
struct ipmem {
    ulong	vectors[40];
    uchar 	copyrt[64];		/* Copyright message */
    uchar 	firm_vers[8];		/* Firmware Version */
    ushort 	mem_size;
    ushort 	des_pres;

    uchar 	diag_comm[64];		/* For diagnostics */
    uchar	diag_stat[32];		/* For diagnostics */
    ushort 	bd_type;		/* Board type */
    ushort 	live_box;		/* Communication active */
    ushort 	diag_stt2[14];
    uchar 	int_vectors[64];	/* Interrupt type per port */
    uchar 	int_mask;		/* Box interrupt mask */
    uchar 	des_block[63];

    ushort 	req_change;		/* Used to reset box */
    ushort 	req_arg;
    ushort 	req_channel;
    ushort 	req_buffer;
    ushort 	ch_adds[17];		/* Point to ipu structure */
    ushort 	baud_rtc[19];
    ushort 	maxchan;
    ushort 	first_free;
    ushort 	new_timer;
    ushort 	ints[INTSIZE];
    uchar 	si_table[SISIZE+1];
    uchar 	tr_rc[SISIZE];
    uchar 	time_const[SISIZE];
    uchar 	gen_ints;
    uchar 	ack_ints;
    uchar 	dippy;
    uchar 	bar_graph;
    uchar 	num_chann;
    uchar 	uart_type;
    uchar 	resvd[20];
};

typedef struct ipmem * ipmem;
static int lion_copyrt[] = {
    0x28632920, 0x436f7079, 0x72696768, 0x74203139,	/* (c) Copyright 19 */
    0x38382043, 0x6f6d7075, 0x746f6e65, 0x20537973,	/* 88 Computone Sys */
};

#ifdef _KERNEL
/*
** ========================================== 
** CONTROL WORD & IN_LINE COMMANDs DEFINITION
** ========================================== 
**
**	set by:	slion_comm(ld, cmnd)
**
**	Baud Rates - add 1 to the value passed by AIX to get the value 
**			for docomm
**	
**	bit(15) = 0	TX data
**	   			bit(0 - 14): the size of TX data(bytes) 
**		= 1	In_line commands to IP board
**
**	bit(14) = 1	All output pending in any of the buffers must be
**			flushed before performing the specific command
**			(command is " synchronous").
**		= 0	The command will be performed immediately.
**
**	bit(8 - 13)	The commands as below:
**	bit(0 - 7)	Parameter's value 
**
**			   +--- In_line command code(HEX)
**			   |
**			   v
*/
#define BAUD	0x8000	/* 00 Set baud rate */
#define SBRK	0x8100	/* 01 Send break (length determined by nbreak */
#define ADTR	0x8200	/* 02 Enable DTR */
#define DDTR	0x8300	/* 03 Disable DTR */
#define ARTS	0x8400	/* 04 Enable RTS */
#define DRTS	0x8500	/* 05 Disable RTS */
#define SCLN	0x8800	/* 08 Set bits/character (bit(0-1:0=5 bits 1=6 bits */
			/*    				  2=7 bits 3=8 bits */
#define SNSB	0x8900	/* 09 Set # of stop bits (bit(0-1:0=1 stp  1=1.5 stp*/
			/*				  2=2 stp bits      */
#define SPAR	0x8a00	/* 0a Set parity (bit(0-1):0=non 1=odd 2=even	    */
#define FTXM	0x8b00	/* 0b Flush transmit main */
#define FTXA	0x8c00	/* 0c Flush transmit alt */
#define FTXT	0x8d00	/* 0d Flush transmit transparent */

/* #define GPUP	0x8e00	   SYNC STUFF */
/* #define GPDN	0x8f00	   SYNC STUFF */
/* #define FALL	0x9000	   Flush All */
/* #define STRP	0x9100	   Istrip */

#define XANY	0x9202	/* 12 IXANY set:RX XOFF stops o/p,any chr restarts op*/
#define XON1	0x9201	/* 12 IXON set:RX XOFF stops o/p, RX XON restarts o/p*/
#define XON0	0x9200	/* 12 IXON clear: no i/p XON/XOFF processing         */

#define XOF1	0x9301	/* 13 IXOF set : XOFF sent when i/p buf almost full, */
			/*		 XON sent when i/p buf almost empty  */
#define XOF0	0x9300	/* 13 IXOF clear : no o/p handshake */

#define RXAC	0x9400	/* 14 Set incoming(Remote) start Char (bit(0-7))*/
#define RXOC	0x9500	/* 15 Set incoming(Remote) stOp Char (bit(0-7)) */
#define LXAC	0x9600	/* 16 Set outgoing(Local) start Char (bit(0-7)) */
#define LXOC	0x9700	/* 17 Set outgoing(Local) stOp Char (bit(0-7))  */

#define ALT1	0x9a00	/* 1a Alt screen on  */
#define ALT0	0x9a01	/* 1a Alt screen off */

#define RTS1	0x9d01	/* 1d RTS flow enable(bit(0)=1) */
#define RTS0	0x9d00	/* 1d RTS flow disable bit(0)=0 */
#define CTS1	0x9e01	/* 1e CTS flow enable bit(0)=1  */
#define CTS0	0x9e00	/* 1e CTS flow disable bit(0)=0 */

#define PAUS	0x9f00	/* 1f Pause(delay) for (parm*5/255) seconds */
			/* bit(0-7) 0 - 5 sec			    */

#define DTR1	0xa001	/* 20 DTR flow enable(bit(0)=1)  */
#define DTR0	0xa000	/* 20 DTR flow disable (bit(0)=0)*/
#define DCD1	0xa101	/* 21 DCD flow enable (bit(0)=1) */
#define DCD0	0xa100	/* 21 DCD flow disable (bit(0)=0)*/

#define S_FL	0xa200	/* 22 Start flush mode:all data are discarded */
#define E_FL	0xa300	/* 23 Stop flush mode			      */


/*
** ========================== 
** GLOBAL COMMANDS DEFINITION
** ========================== 
**
** Global commands:
**	They are transferred to the box asynchronously with respect to
**	the  "in_line" commands. The struct ipu field "glob_add" is
**	the offset to the global command buffer from the beginning of
**	a 64K block
**	The data is stored in the buffer starting with the second byte,
**	and then the global commandd is stored in the first byte of the 
**	buffer.After transmitted this command to the BOX, the first
**	byte will be reset to 0.
*/
#define G_DTR_UP	2
#define G_RTS_UP	4
#define G_PKEYS		6
#define G_GOTO1		8	/* Define goto1 sequence */
#define G_GOTO2		10
#define G_IN_XPAR	12	/* Set in xpar sequence: Bytes 1-11 contain 
				** the chars which make up the sequence     */

#define G_LV_XPAR	14	/* Set lv_xpar sequence: Bytes 1-11 char seq*/
#define G_SCREEN1	16
#define G_SCREEN2	18
#define G_CLRPKEYS	20
#define G_SETPKEYS	22
#define G_PCRLF_S	24
#define G_PCRLF_C	26
#define G_FLSH		28	/* 0 main, 1 vger, 2 xpar 		*/
#define G_GOMAIN	30	/* Forces channel to main screen 	*/
#define G_SET_PARM	32	/* Set port parameters: Btes 1-8 are as:
				**   byte 0 - command(0x20)
				**   	  1 - reg_crlf
				** 	  2 - xpar_crlf
				** 	  3 - baud
				** 	  4 - stops
				** 	  5 - parity
				** 	  6 - bpc
				** 	  7 - priority
				** 	  8 - shaking 
				**	   (0 none, 1 xon/xoff, 2 busy/ready)
				** Setting any of these fields to 0xff causes
				** the value remain unchanged.
				*/

#define G_ALT_DEFS	34	/* Def alternate screen key seq. Bytes 1-30:
				**   byte 1 -  4 = goto1 sequence
				**        5 -  8 = goto2 sequence
				**        9 - 19 = screen1 sequence
				**       20 - 30 = screen2 sequence
				*/

#define G_RPT_ERRS	36	/* Parity/framing error report: byte 1= 0x2a*/
#define G_RPT_ERRS_PARM	0x2a	/* byte 1				    */

				/* Error data sequence:
				**	MM	MM= 0x00 - 0xfe data is MM
				**	FF FF   		data is 0xff
				**	FF XX YY		data is YY
				**		xx is:
				*/
#define CC_BUFFERO	0x08
#define CC_OVERRUN	0x10
#define CC_PARITY	0x20
#define CC_FRAMING	0x40
#define CC_BREAK	0x80

#define G_DDS_FLSH	38	/* Turn on flush mode(TX data only) byte 1 is 
				** 	0=main, 1=vger, 2=xpar screen
				*/

#define G_XINHIBIT	40	/* Set Output Inhibit command: Byte 1 is: 
				**   0= "or" in a host defined inhibit cond.
				**   1= clear the host defined inhibit cond.
				**   2= clear all inhibit conditio.
				*/
#define G_EXTENDED	42
#define G_EX_SET232	5
#define G_EX_SET422	6

				/* int_vectors: interrupt statusa	*/
#define CC_INT_RCMAIN	(0x01)	/* RX data available in b_rc_main	*/
#define CC_INT_RCALT 	(0x02)	/* RX data available in b_rc_alt	*/
#define CC_INT_TXMAIN	(0x04)	/* b_tr_main ring buffer empty		*/
#define CC_INT_TXALT 	(0x08)	/* b_tr_alt ring buffer empty		*/
#define CC_INT_TXPAR 	(0x10)	/* b_tr_xpar ring buffer empty		*/
#define CC_INT_DCD   	(0x20)	/* Change in DCD occured		*/
#define CC_INT_HILINK	(0x40)	/* Adapter lost communication with BOX	*/
#define CC_INT_CTS	(0x80)	/* Change in CTS occured		*/

#endif /* _KERNEL */

/* 
** These are bit maps for the global commands.
** We save up the ones we want to do in a bit map in the slionprms struct.
*/
#define GC_FLSH_MAIN	0x00000001
#define GC_FLSH_VGER	0x00000002
#define GC_FLSH_XPAR	0x00000004

#define GC_SET_INHIB	0x00000008
#define GC_ZAP_INHIB	0x00000010

#define GC_E_FL_MAIN	0x00000020
#define GC_E_FL_VGER	0x00000040
#define GC_E_FL_XPAR	0x00000080

#define GC_SET_PARMS	0x00000100
#define GC_VGER_DEFS	0x00000200
#define GC_GOTO_XPAR	0x00000400
#define GC_LEAV_XPAR	0x00000800
#define GC_DEF_ERROR	0x00001000
#define GC_CLR_INHIB	0x00002000

#define GC_FRCE_MAIN	0x00004000

#define GC_BOX_RST_1	0x00008000	/* BOX_RST_1&2 must be lowest */
#define GC_BOX_RST_2	0x00010000	/* Priority -- see lionl.c */
#define GC_MAX		0x00020000

/* pos register stuff */
static uchar ccIntTbl[8] = {15, 11, 10, 4, 12, 7, 5, 3, };

#define CC_CRD_ENAB	0x01
#define CC_INT_MASK	0x0e
#define CC_INT_BASE	0x40

#ifdef _KERNEL

/*
** For the TANK, we need to attach/detach the adapter.  The attach returns
** a pointer to the base of bus memory.  Local to each function that
** accesses the card, we delare 'uchar *BusMem'.  The board is attached
** using fields from either the sliondata structure or the adapter structure
** and BusMem is returned.  The access macros RD/WR_CHAR/SHORT index off of
** BusMem to read the data.
*/
#define ATT_BD(adap) BusMem=(uchar *)(((int)io_att((adap)->nseg,0)))
#define DET_BD();		io_det(BusMem);
	
#ifdef ASSEMBLY
#define RD_SHORT(addr)		(rd_short(BusMem+(int)(addr)))
#define WR_SHORT(addr, val)	(wr_short((BusMem+(int)(addr)), val))
#else  /* ~ASSEMBLY */
#define RD_SHORT(addr)		(BUS_GETSR(BusMem+(int)(addr)))
#define WR_SHORT(addr, val)	BUS_PUTSR((BusMem+(int)(addr)), (val))
#endif /* ASSEMBLY */

#define RD_CHAR(addr)		BUS_GETC(BusMem+(int)(addr))
#define WR_CHAR(addr, val)	BUS_PUTC((BusMem+(int)(addr)), (val))
#define MK_ADDR(value)		(BusMem+((int)&(value)))

#define CHAN_MAIN	0
#define CHAN_VGER	1
#define CHAN_XPAR	2

#define BOX_VPD_BYTE0	0xfe80	/* Offset to byte 0 of box VPD */
#define BOX_VPD_BYTE1	0xfe81	/* Offset to byte 1 of box VPD */
#define BOX_VPD_BYTEL	0xfeff	/* Offset to last byte of box VPD */
#define CRD_VPD_BYTE1	0xfe01	/* Offset to byte 1 of card VPD */

#endif /* _KERNEL */

/*
** this structure is passed to the init routine
** this may very well change considerably depending upon the
** way config is handled in the tank
*/
enum adap_type {
    Unknown_Adap, SixtyFourPort,
};


/* There are two kinds of DDS structures: One for adapters configuration 
** and one for lines configuration.
*/
/*
** ======================== 
** LION_ADAP_DDS structure
** ======================== 
** This structure (or something like it) is fed down at config time.  The
** adapter type must be one that is known.  The other fields are fed down
** and checked with those in the known interrupt structure.  Any field marked
** as CONFIG_TIME is allowed to be set.  Any other field is compared and
** must match or the config will fail.
*/
struct lion_adap_dds {
    enum   dds_type	which_dds;    		/* DDS id.(LION_ADAP_DDS)*/
    char   		lc_name[DEV_NAME_LN+1];	/* Adapter name */
    enum   adap_type	lc_type;		/* Adapter type */
    uchar  		lc_anum;		/* Adapter number */
    uchar  		lc_slot;		/* Slot adapter is in */
    dev_t  		lc_parent;		/* Expansion unit devno */
    uchar  		lc_xbox;		/* Adapter is in xbox */
    int    		lc_level;		/* Interrupt level */
    int    		lc_priority;		/* Interrupt class */
    ushort 		lc_bus;			/* probably never settable*/
    ushort 		lc_flags;		/* probably always 0 */
    ulong  		lc_nseg;		/* Seg reg for normal access */
    ulong  		lc_base;		/* Base address of adapter */
    ulong  		lc_iseg;		/* Seg reg for id access */
    ulong  		lc_ibase;		/* Base of id registers */
};

/*
** ======================== 
** LION_LINE_DDS structure
** ======================== 
*/
struct lion_line_dds {
    enum   dds_type	which_dds;		/* DDS id(LION_LINE_DDS) */
    char   		line_name[DEV_NAME_LN+1]; /* Line name */
    char   		adap_name[DEV_NAME_LN+1]; /* Adpater(parent)line name*/
    struct termios 	ctl_modes;	 	/* Control modes */
    struct termiox 	disc_ctl;		/* HW flw ctl/open flw disc*/
    ushort 		tbc;			/* Transfer buffer count */
    uchar  		in_xpar[11];		/* Turn on xprint mode */
    uchar  		lv_xpar[11];		/* Turn off xprint mode */
    uchar  		priority;		/* Xprint priority */
};

#define COMM_SIZE 	16		/* Must be power of 2 */

/*
** ======================== 
**    STR_LION structure		== tp ==
** ======================== 
*/
typedef struct str_lion *str_lionp_t;

struct str_lion {
    char 		t_name[DEV_NAME_LN+1];	/* Line name from line_dds */
    dev_t 		t_dev;		/* Major/minor number */
    chan_t 		t_channel;	/* Channel number */
					/* For Streams porting */
    mblk_t 		*t_outmsg;	/* To save mesage being transmitted*/
    mblk_t 		*t_inmsg;	/* To anticipate input mesg alloc */
    mblk_t		*t_ioctlmsg;	/* M_IOCACK replyed after a timeout*/

					/* For Streams porting */
    queue_t 		*t_ctl;		/* Read queue in the streams stack */
    void 		*t_hptr;	/* Hardware storage */

					/* Other things */
    int 		t_event;	/* Event list for e_sleep */
    int 		t_lock;		/* Lock per tty */
    int 		t_gcommtmr;	/* Call timeout from slion_set_gcomm */
    int 		t_slpxtmr;	/* Call timeout from slioc_dslpx */
    int 		t_dslptmr;	/* Call timeout from slioc_dslp */
    int			t_timeout;	/* Call timeout from M_DELAY */ 
    int			t_brktimeout;	/* Call timeout from BREAK */ 
    int			t_alloctimer;	/* Call timeout from slion_allocmsg*/ 
    uchar	        t_draining;     /* Data is being drained at close */
    uchar		t_sched;	/* Off-level should be rescheduled */
    int  		t_bufcall;	/* Bufcal utility was called */
    uchar		t_hupcl;        /* current HUPCL termios cfalg */
    uchar		t_localstop;	/* stop if M_STOP is on */

    struct termios 	t_termios; 	/* Initial line settings */
    struct termiox 	t_termiox; 	/* For HW pacing & open discipline */

					/* Hardware state flags */
    uint t_carrier:	1;		/* 0x80000000 Carrier flag */
    uint t_slpclose:	1;		/* 0x40000000 Slp in close flg:NU */
    uint t_isopen:	1;		/* 0x20000000 TTY is open */
    uint t_busy:	1;		/* 0x10000000 Output in progress */
    uint t_stop:	1;		/* 0x08000000 Output is stopped,
					 * 	      remote pacing*/
    uint t_block:	1;		/* 0x04000000 Input is stopped, 
					 *	      local pacing */
    uint t_ctlx:	1;		/* 0x02000000 Got a control X */
    uint t_sak:		1;		/* 0x01000000 Sak is enabled */
    uint t_cread:	1;		/* 0x00800000 Termios cread flag */
    uint t_config:	1;		/* 0x00400000 TTy configured flag */
    uint t_excl:	1;		/* 0x00200000 EXCL:1/NXCL:0 flag */
    uint t_clocal:	1;		/* 0x00100000 Termios clocal flag */
    uint t_wclose:	1;		/* 0x00080000 Waiting close flag */

	 	              		/* Open disciplines */
    int 		t_disctype;	/* Current open discipline 
					**   0 == dtropen 
					**   1 == Other Open Discipline
					*/

    caddr_t 		t_OpenRetrieve;	/* To retrieve current open dis */
					
    mblk_t 		*t_ctloutmsg;	/* To save ctl message*/
    uchar		t_dslpcnt;	/* Use for LI_DSLP handling */
    uchar		t_liflg;	/* Use for LI_DSLP/SETXP/SETVT
					 * handling */
    int			t_rxcnt;	/* Current rx data count on msg blk*/
    int			t_wopen;	/* Numb of openes in progress */
    uchar		*t_busmem;	/* Save BusMem:*/
};

					/* Def. of t_dslpflg */
#define	DSLPDLY1	1		/* Process SYLPDLY1 */
#define	DSLPDLY2	2		/* Process SYLPDLY2 */
#define	DSLPDLY3	3		/* Process SYLPDLY3 */
#define	FLGLISET	0x80		/* Called by LI_SET..*/

/*
** ======================== 
**   SLIONDATA structure		== ld ==
** ======================== 
** This is the virtual port data. There is one struct for each virtual port,
** making 192 sliondata structs per adapter.  This structure contains a 
** pointer back to tp for cross reference.  There is also a pointer to 
** the slionprms structure which contains the 'physical' port data, such 
** as baud rate, control sequences, pacing flags, . . .  
** The buf_beg and buf_almost values for the ring buffers are kept in 
** this struct to eliminate the need for bus accesses for these values 
** since they don't change.  The logical or (or boolean value) of the 
** flags field indicate a t_busy condition.  That is, tp->t_busy depends 
** upon all of the bits in this field.
*/
struct sliondata {
    str_lionp_t		tp;		 /* Pointer to str_lion structure */
    struct slionprms 	*prms;		 /* phys parameters */
    uchar 		*board;		 /* Base address of board */
    struct ipu 		*ip;		 /* User structure on board */
    struct ring 	*tr_buf,*rc_buf; /* Ring structure pointers */
    ushort tr_bb, 	tr_ba;		 /* Buf begin and buf almost */
    ushort rc_bb, 	rc_ba;		 /* Buf begin and buf almost */

    uchar 		*txbuf;		 /* Pointer to transmit data */
    uint 		txcnt;		 /* Count of transmit data */
    uint 		nseg;		 /* For io_att */

    unsigned block:	1;	 	 /* Are we blocked for input */

    uchar 		flags;		 /* See list below */
    uchar 		rc_state;	 /* Keeps track of error chk stuff */
    ulong		i_count, o_count;

    ushort 		comm_head;	 /* Head of ring buffer */
    ushort 		comm_tail;	 /* Head of ring buffer */
    ushort 		comm_list[COMM_SIZE]; /* Ring buf. of pending cmd.*/

#ifdef COMM_DEBUG
    uchar  		pad_1;
    uchar  		pad_curr;
    ushort 		pad_entry[256];
#endif

    /* The last field buffers the structure to a power of 2 size */
};

/*
 * definition of flags in sliondata structure 
 */
#define COMM_FLAG	0x01		/* Waiting to send a command */
#define FLSH_FLAG	0x02		/* Waiting for flush */
#define XMIT_FLAG	0x04		/* Waiting to send data */
#define DIAG_FLAG	0x08		/* Waiting for a diagnostic test */
#define BUSY_MASK	(COMM_FLAG|FLSH_FLAG|XMIT_FLAG|DIAG_FLAG) /*0x0f*/
#define XBOX_FLAG       0x10		/* In expansion box */


/*
** ======================== 
**    SLIONPRMS structure		== prms ==
** ======================== 
**
** Want to put as much stuff in this structure as possible and take it
** out of the sliondata structure.  The more info here to eliminate 
** repetition of data the better.
*/
struct slionprms {			/* Current parms for the phys port */
    uchar 		goto1[4];	/* Goto screen 1 cmd seq from user */
    uchar 		goto2[4];	/* Goto screen 2 cmd seq from user */
    uchar 		screen1[11];	/* Goto screen 1 seq to terminal */
    uchar 		screen2[11];	/* Goto screen 2 seq to terminal */
    uchar 		in_xpar[11];	/* Turn on xprint mode */
    uchar 		lv_xpar[11];	/* Turn off xprint mode */
    uchar 		priority;	/* Xprint priority */

    ushort 		tr_width;	/* Width of transmit chunks */
    ushort 		tr_tbc;		/* Chars to write to each chunk */
    ushort 		rc_width;	/* Width of receive chunks */

    unsigned int 	baud;

    unsigned char_size:	2;		/* Bits per character */
    unsigned parity:	3;
    unsigned stop_bits:	2;

    unsigned dcd:	1;		/* Last values */
    unsigned cts:	1;

    unsigned dtr:	1;		/* Current state of outputs */
    unsigned rts:	1;

    /* 
    ** State of the pacing modes kept in these flags.  Each of the 4 
    ** modem status line pacings is individually controllable.  Note: 
    ** xon and xany are mutually exclusive.
    */
    unsigned dtrp:	1;
    unsigned dcdp:	1;
    unsigned rtsp:	1;
    unsigned ctsp:	1;
    unsigned xonp:	1;
    unsigned xoff:	1;
    unsigned xany:	1;

    unsigned vger_defd:	1;
    unsigned xpar_defd:	1;

    uchar 		lxac;		/* Local xon/xoff start character  */
    uchar 		lxoc;		/* Local xon/xoff stop  character  */
    uchar 		rxac;		/* Remote xon/xoff start character */
    uchar 		rxoc;		/* Remote xon/xoff stop  character */

    unsigned has_died:	1;
    unsigned has_died2:	1;

    int 		m_open_evt;	/* e_sleep event for open */

    int 		gcomm_evt;	/* Timer for global commands. */
    int 		gcomm_mask;
    str_lionp_t 	tp;

#ifdef FEATURES
    str_lionp_t 	vtp;		/* Get from on virtual port's */
    str_lionp_t 	xtp;		/* Struct to another's */
#endif /* FEATURES */

};

#ifdef _KERNEL

/*
** ======================== 
**    SLI_ADAP structure		== adap ==
** ======================== 
**
** We will need two intr structures.  One for the slih, and one for
** off level.  Since the slih intr is (apparently) distinguished by
** nseg and int level ONLY, we may have multiple adapters for each
** slih intr.  We will have one off level intr struct for each adapter.
** These will be chained in the slih intr struct so the slih can scan
** each of the adapters attached.
** There will be one of these structures for each of the adapters.
** They will be in two (types of) linked lists.  The first is a
** list of all of the adapters of that type.  The second is a list
** for slih chaining.  NOTE the bus id is in the intr struct.
*/
static struct sli_adap {
    struct intr 	intr;		/* To pass to off level */
    struct sli_adap 	*list_next;	/* Linked list of adap structures */
    struct sli_adap 	*slih_next;	/* Linked list for slih */
    ulong 		nseg;		/* Seg for normal access */
    ipmem 		board;		/* Addrs of the virt board */
    ulong 		iseg;		/* Seg for pos access */
    ulong 		ibase;		/* Slot stuff ?? */
    uchar 		slot;		/* Slot the adapter resides in */
    dev_t 		parent;		/* Expansion unit devno */
    unsigned posted:	1;		/* This adapter posted to off level */
    unsigned inxbox:	1;		/* This adapter is in expansion box */
    unsigned inconfig:	1;		/* In the config process */

    /* 
    ** The sliondata structs take up A LOT, but this reduces the number
    ** of mallocs we need to do in slionadd (and frees in sliondel).
    ** And it insures that the data is as packed as can be.
    */
    struct sliondata 	main[64];	/* 64 Main screen virtual ports */

#ifdef FEATURES
    struct sliondata 	vger[64];	/* 64 Virgin gerbil ports */
    struct sliondata 	xpar[64];	/* 64 Transparent printer ports */
#endif  /* FEATURES */

    struct slionprms 	prms[64];	/* Phys parms for all 3 virt ports */
    struct str_lion 	m_tty[64];	/* For the main screen */
    str_lionp_t 	m_ttyp[64];	/* Pointers to the str_lion structs */

#ifdef FEATURES
    struct str_lion 	v_tty[64];	/* For the virgin gerbil */
    str_lionp_t 	v_ttyp[64];	/* Pointers to the str_lion structs */
    struct str_lion 	x_tty[64];	/* And for the xparent printer */
    str_lionp_t 	x_ttyp[64];	/* Pointers to the str_lion structs */
#endif  /* FEATURES */

    char   		adap_name[DEV_NAME_LN+1];	/* Adapter name */
};

/*
** ======================== 
**    SLI_SLIH structure
** ======================== 
*/
static struct sli_slih {
    struct intr 	intr;		/* To pass to slih */
    struct sli_slih 	*next;		/* Linked list of slih's */
    struct sli_adap 	*adap_chain;	/* List of adaps on this slih chain */
};

enum li_trctypes {
    LION_ADD=TTY_LAST, LION_DEL, LION_LAST
 };

extern	int 		li_debug[2];

#ifdef DO_PIO
void slion_pio(struct sliondata *ld, pud_t *pud, int logit);
#endif /* DO_PIO */ 


/* Declare common variables here (defines in lionl.c) */
extern struct 		sli_adap **adap_list;
extern char 		resource[16];

#define SLRDBUFSZ       64              /* Read data  message  size */

#ifdef	SLI_TRACE
#define	SL_printf	printf
#else
#define	SL_printf
#endif	/* SLI_TRACE */

#define TRC_LION(w)	((HKWD_STTY_LION)|w)

#endif /* _KERNEL */

#endif /* _H_SLION */
