static char sccsid[] = "@(#)66  1.4  src/bos/kernext/trace/trcdata.c, systrace, bos411, 9428A410j 10/29/93 05:25:09";
/*
 * COMPONENT_NAME: SYSTRACE   /dev/systrace pseudo-device driver
 *
 * FUNCTIONS: initialization for pinned trchdr structure trchdr[]
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  LEVEL 1, 5 Years Bull Confidential Information  
 */

/*
 * FUNCTION: pinned header for the 8 trace channels
 *
 * The trace routines should always be able to accept trace calls,
 *  even during the early phases of kernel initialization.
 * Therefore, the trchdr structure should be in the initialized data segment.
 * Note: if the trace routines are to be used as 'debug' routines,
 *  replace the pointers to 'spillbuf' with pointers to a debug buffer.
 */

#include <sys/types.h>
#include <sys/trchdr.h>
#include <sys/trchkid.h>
#include <sys/lockl.h>
#include <sys/sleep.h>

/*
 *		struct trchdr {
 *			int    trc_state;					state bits defined in trc.h
 *			int    trc_msr;						msr save area
 *			int    trc_lockword;				serialize
 *			int    trc_sleepword;				e_sleep
 *			short  trc_mode;					modes defined in trc.h
 *			short  trc_fsm;						finite state machine state
 *			short  trc_channel;					channel, used by dump
 *			short  trc_wrapcount;				trace buffer wraparound count
 *			int    trc_ovfcount;				overflow due to wrapping
 *			short  trc_trconcount;				number of times trcon() called
 *			short  trc_fill;					align to 32 bytes
 *			struct trc_q    trc_currq;			used by hook routines
 *			struct trc_q    trc_Aq;
 *			struct trc_q    trc_Bq;
 *			struct trc_log *trc_lp;				trace logfile structure
 *			unsigned char   trc_events[32*16];	cond. event bit map
 *		};
 */
#define T(i) \
{ \
	0, \
	0, \
	LOCK_AVAIL, \
	EVENT_NULL, \
	0, \
	0, \
	0, \
	0, \
	0, \
	0, \
	0, \
	{ 0, }, \
	{ 0, }, \
	{ 0, }, \
	0 \
}

struct trchdr trchdr[8] = {
	T(0),
	T(1),
	T(2),
	T(3),
	T(4),
	T(5),
	T(6),
	T(7)
};

