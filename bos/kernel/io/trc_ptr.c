static char sccsid[] = "@(#)02  1.9  src/bos/kernel/io/trc_ptr.c, systrace, bos411, 9428A410j 6/23/94 11:43:23";
/*
 *   COMPONENT_NAME: SYSTRACE 
 *
 *   FUNCTIONS: T
 *		trc_ptrdmy
 *		trc_ptrdmy0
 *		trc_timeout
 *		trc_timeoutinit
 *
 *   ORIGINS: 27 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *   
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */



#include <sys/types.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/trchdr.h>
#include <sys/trchkid.h>
#include <sys/lockl.h>
#include <sys/sleep.h>

uint trc_bussreg;
uint trc_busaddr;

/*
 * Lock used to synchronize trcgen functions with
 * the unloading of the driver
 * 
 * initfp_lock = |x|x|x|x|                       count|
 *		 31   29                              0
 *
 * count is less than the number of CPU 
 *
 * if initfp_lock == 0 then the lock is not take ( ie mode TN)
 * if initfp_lock >= (1<<29) then lock is taken in write ( ie mode TW)
 * if count <> 0 ( ie initfp_lock <> 0 and  <> (1<<29) ) then the lock is taken in read ( ie mode TR)
 * ie:
 *     - mode TN : initfp_lock == 0
 *     - mode TW : (initfp_lock & (1<<29) ) <> 0
 *     - mode TR : (initfp_lock & ~(1<<29) ) <> 0
 *
 * if the mode is TN or TR, the trcgen routines are called.
 * if the mode is TW, the trcgen routines aren't called.
 * if the mode is TN, the driver is unloaded 
 * if the mode is TR, the driver isn't unloaded
 * before final code of the driver, trcconfig put the mode to TW
 * after the final code of the driver: initfp_lock = 0; 
 * before to call trcgen: initfp_lock++
 * after the calling of trcgen: initfp_lock--
 */

int initfp_lock;

/* 
 * Dummy functions
 */
void trc_ptrdmy()
{
}

int trc_ptrdmy0()
{
	return(0);
}

/*
 * These function pointers are used in trcdd.c
 */
int (*_trcbuffullfp)()=(int)trc_ptrdmy;
void (*_trcgenfp)()=trc_ptrdmy;
void (*_trcgentfp)()=trc_ptrdmy;
void (*_trcgenkfp)()=trc_ptrdmy;
void (*_trcgenktfp)()=trc_ptrdmy;
char *trc_savefp;

/*
 * These function pointers are used in dumpdd.c
 */
int (*_trc_trcofffp)() = trc_ptrdmy0;
int (*_trc_trconfp)() = trc_ptrdmy0;

/*
 * These function pointers are used in errdd.c
 */
int (*Jprintf_fp)() = trc_ptrdmy0;
int (*jsnap_fp)() = trc_ptrdmy0;

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

struct trchdr fake_trchdr[8] = {
	T(0),
	T(1),
	T(2),
	T(3),
	T(4),
	T(5),
	T(6),
	T(7)
};

/*
 * *trchdr_ptr is initialized to fake_trchdr[] when the trace driver is 
 * not loaded in the kernel
 */
struct trchdr	*trchdr_ptr=&fake_trchdr[0];
extern unsigned char  Trconflag[];
/*
 * These bunch of code here below belonged to trace driver when the trace
 * driver was part of the kernel; now, the trace driver is a subsystem
 * therefore, they must be removed from trchkutil.c code and stay in the 
 * kernel
 */

struct trb *trc_timerp;         /* talloc-ed once at trcconfig() time */ 

/*
 * Function : initialize trc_timerp structure
 * Intputs : none
 * Outputs : none
 */
trc_timeoutinit()
{

	trc_timerp = talloc();
}

/*
 * Function : call to trace hook and generic trace hook routines for 
 *	      every channel and schedule a timeout if not already scheduled
 * Intputs : intflg	used to set up the timeout flag
 * Outputs : none
 */
void trc_timeout(intflg)
{
	register i, j;
	register chancount;
	static timeoutflg;

	if(intflg)
		timeoutflg = 0;
	chancount = 0;
	if(TRC_ISON(0)) {
		TRCHKT(HKWD_TRACE_UTIL);
		chancount++;
	}
	for(i = 1; i < TRC_NCHANNELS; i++) {
		if(TRC_ISON(i)) {
			TRCGENT(i,HKWD_TRACE_UTIL,i,0,"");
			chancount++;
		}
	}
	/*
	 * schedule a timeout if not already scheduled
	 */
	if(chancount > 0 && !timeoutflg) {
		trc_timerp->flags     = 0;
		trc_timerp->func_data = 1;
		trc_timerp->func      = trc_timeout;
		trc_timerp->ipri      = INTBASE;
		trc_timerp->timeout.it_value.tv_sec  = 0;
		trc_timerp->timeout.it_value.tv_nsec = 900000000;	/* .9 secs */
		tstart(trc_timerp);
		timeoutflg = 1;
	}
}
