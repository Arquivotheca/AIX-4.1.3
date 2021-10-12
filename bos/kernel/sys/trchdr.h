/* @(#)98    1.20  src/bos/kernel/sys/trchdr.h, systrace, bos411, 9428A410j 1/28/94 09:09:48 */

/*
 * COMPONENT_NAME:            include/sys/trchdr.h
 *
 * FUNCTIONS: header file for system trace control block and macros
 *
 * ORIGINS: 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * FUNCTION: definition of trchdr trace buffer header.
 *           Also contains state bit definitions and macros.
 *           To be used only inside the kernel.
 */  

#ifndef _H_TRCHDR
#define _H_TRCHDR

/*
 * Words, not bytes, that 'inptr' can grow beyond 'end'.
 * Equal to the maximum number of words per trace.
 */
#define TRC_OVF    68
#define GENBUFSIZE 4096			/* max size of trcgen() buffer */

struct trc_q {
	int *q_start;				/* base of the buffer */
	int *q_end;					/* end of the buffer - TRC_OVF */
	int *q_inptr;				/* trchk() will write here */
	int  q_size;				/* allocated size in bytes of this q */
};

struct trchdr {
	int    trc_state;				/* state bits defined in trc.h */
	int    trc_msr;					/* msr save area */
	int    trc_lockword;			/* serialize */
	int    trc_sleepword;			/* e_sleep */
	short  trc_mode;				/* modes defined in trc.h */
	short  trc_fsm;					/* finite state machine state */
	short  trc_channel;				/* channel, used by dump */
	short  trc_wrapcount;			/* trace buffer wraparound count */
	int    trc_ovfcount;			/* overflow due to wrapping */
	short  trc_trconcount;			/* number of times trcon() called */
	short  trc_fill;				/* align to 32 bytes */
	struct trc_q    trc_currq;		/* used by hook routines */
	struct trc_q    trc_Aq;
	struct trc_q    trc_Bq;
	struct trc_log *trc_lp;			/* trace logfile structure */
	unsigned char   trc_events[32*16];/* cond. event bit map */
};
#define trc_start   trc_currq.q_start
#define trc_end     trc_currq.q_end
#define trc_inptr   trc_currq.q_inptr
#define trc_size    trc_currq.q_size

#define trc_startA  trc_Aq.q_start
#define trc_endA    trc_Aq.q_end
#define trc_inptrA  trc_Aq.q_inptr
#define trc_sizeA   trc_Aq.q_size

#define trc_startB  trc_Bq.q_start
#define trc_endB    trc_Bq.q_end
#define trc_inptrB  trc_Bq.q_inptr
#define trc_sizeB   trc_Bq.q_size

#define FSM_A_B               1
#define FSM_A_BLOCK           2
#define FSM_B_A               3
#define FSM_B_BLOCK           4
#define FSM_SPILL_A           5
#define FSM_SPILL_A_OFF       6
#define FSM_SPILL_A_OFF2      7
#define FSM_SPILL_B           8
#define FSM_SPILL_B_OFF       9
#define FSM_SPILL_B_OFF2      10
#define FSM_SPILL_BLOCK       11
#define FSM_SPILL_EOF         12

extern struct trchdr trchdr[];

#define ST_ISOPEN      0x01		/* exclusivity */
#define ST_TRCON       0x02		/* tracing is active (buffer can SPILL) */
#define ST_WRAP        0x04		/* stop-on-wrap-mode, wrap */
#define ST_TRCSTOP     0x10		/* TRCSTOP ioctl, cause EOF */
#define ST_COND        0x20		/* conditional mode, for trcgen */
#define ST_BUS         0x40		/* bus mode, for trcgen */
#define ST_SYNC        0x80		/* wait for close of /dev/systrace */
#define ST_IACTIVE	0x100		/* interactive trace is being run */

/*
 * trace mode codes. Internal to driver.
 */
#define MD_OFF            0		/* could become MD_DEBUG */
#define MD_CIRCULAR       1
#define MD_CIRCULAR_COND  2
#define MD_ALTERNATE      3
#define MD_ALTERNATE_COND 4
#define MD_SINGLE         5
#define MD_SINGLE_COND    6
#define MD_BUS            7
#define MD_BUS_COND       8

#define HKTOEVENT(hookword) ((unsigned)(hookword) >> 20)

#define ISEVENT(tp,n) \
(tp->trc_events[HKTOEVENT(n)/8] & (1 << HKTOEVENT(n)%8))

#ifdef _IBMRT
#define INT_OFF()  j_disable()
#define INT_ON()   j_enable(0)
#define INT_ONX(s) j_enable(s)
#else
#define INT_OFF()  i_disable(INTMAX)
#define INT_ON()   i_enable(INTBASE)
#define INT_ONX(s) i_enable(s)
#endif

extern char jsnapflg;

#ifdef DEBUG
#define trc_debug  (jsnapflg == 0) ? 0 : jsnap
#define trc_debug2 (jsnapflg <  2) ? 0 : jsnap
#else
#define trc_debug
#define trc_debug2
#endif

#define FULLCHK(tp,s) \
{ \
	int rv; \
\
	rv = 0; \
	if(tp->trc_inptr >= tp->trc_end) \
		rv = trcbuffull(tp); \
	INT_ONX(s); \
	if(rv) \
		trc_wakeup(tp); \
}

#define TIMESTAMP trc_timestamp()

struct trc_log_entry {
	int le_offset;
	int le_size;
};

struct trc_log_hdr {
	int lh_magic;				/* magic number */
	int lh_nentries;			/* number of log_entries */
	int lh_currentry;			/* current entry */
	int lh_mode;				/* wrap mode */
	int lh_wrapcount;			/* wrap mode */
	int lh_fd;					/* logfile file descriptor */
	struct file *lh_fp;			/* logfile file structure */
	struct trc_log_entry lh_ic;	/* initial conditions fro trace command */
};

struct trc_log {
	struct trc_log_hdr   l_h;
	struct trc_log_entry l_data[1];	/* trace buffers */
};
#define l_magic     l_h.lh_magic
#define l_nentries  l_h.lh_nentries
#define l_currentry l_h.lh_currentry
#define l_mode      l_h.lh_mode
#define l_wrapcount l_h.lh_wrapcount
#define l_fd        l_h.lh_fd
#define l_fp        l_h.lh_fp
#define l_ic        l_h.lh_ic

#define MDL_NOWRAP     0x01
#define MDL_STOPONWRAP 0x02

#define TRC_LMAGIC 0xEFDF1111
/* add from bull */
#define TRC_NEW_LMAGIC 0xEFDF1112
/* end add bull */
#define TRC_LSIZE(hp) \
	( sizeof(struct trc_log_hdr) + \
	(hp)->lh_nentries * sizeof(struct trc_log_entry) )

/*
 * used by trcdead to decide which dump buffers to get
 */
struct trctyp {
	int t_type;
};
#define TRCT_A  1
#define TRCT_B  2

#endif /* _H_TRCHDR */

