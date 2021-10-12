#ifndef lint
static char sccsid[] = "@(#)06 1.24 src/bos/kernext/prnt/ppddb.c, sysxprnt, bos411, 9428A410j 5/3/94 17:42:53";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Printer Device Driver
 *
 *   FUNCTIONS: ppelog_log
 *              ppready
 *		pppulst
 *              ppreadctrl
 *              ppwritectrl
 *              ppwritestat
 *		ppdelay
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/ppdd.h>
#include <sys/lockl.h>
#include <errno.h>
#include <sys/lpio.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/sleep.h>
#include <sys/timer.h>

/* Global variables */

struct pp *ppheader = NULL;    /*  Header of Linked List of pp structures*/
struct pp *pptail = NULL;      /*  Tail of Linked List of pp structures  */
int ppglobal_lock = LOCK_AVAIL;/*  Global Lock Variable                  */


/*
 * NAME: ppelog_log
 *
 * FUNCTION: log an error if one occurs
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * (NOTES:)
 *      copy the loc sturct to the error struct and call errsave to save it
 *
 * RETURNS: none
 *
 */


void ppelog_log(
struct ppiost *loc )                       /* address of io structure    */
{
	bcopy(loc,&loc->pp->ppelog.pioerr,sizeof(struct ppiost)) ;
	errsave((char *)&loc->pp->ppelog,sizeof(struct ppelog)) ;
	return ;
}


/*
 * NAME:  ppready
 *
 * FUNCTION:   test to see if the printer is ready
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: TRUE if ready 0 if not
 */

int ppready(
char statusreg ,
struct pp * pp )
{
	char ctl = 0 ;
	char ignore ;
	char stnum ;

	/* set up for the type of interface  */
	switch (pp->interface)
	{
	case PPIBM_PC :
		ignore = PP_RESERVED | PP_IRQ_STAT | 
		         ( pp->ppmodes & IGNOREPE ? PP_PE : 0 ) ;
		ctl = ( (statusreg & ~ignore) == 0xd8 ) ;
		break;

	case PPCONVERGED :
		stnum = pppulst(statusreg,pp) ;
		if ((statusreg & PP_BUSY_BIT) &&        /* not busy and  */
		   ((stnum == PPNOERROR)      ||        /* no errors or */
		   ((pp->ppmodes & IGNOREPE)  &&        /* ignore paper end */
		    (stnum == PPPERMERROR))))           /* and no errors */
			ctl = 1 ;                       /* return a true */
	}
	return (ctl) ;
}


/*
 * NAME: ppulst
 *
 * FUNCTION:
 *    input hardware status reg and control reg
 *    output converged status number 0 - 16
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: status reg and control reg
 */

int pppulst(
char statusreg,
struct pp * pp)
{
	int ctl ;
	char stnum ;

	ctl = ppreadctrl(pp) ;          /* read the control reg */
	if (ctl == -1)
		return (ctl) ;
	stnum = 7 & (statusreg>>3) ;
	if(ctl & PP_AUTOFD)
		ctl = 0 ;
	else
		ctl = 0x08 ;
	stnum |= ctl ;
	return (stnum) ;
}


/*
 * NAME: ppreadctrl
 *
 * FUNCTION:
 *         This will read the control register and return it to the caller
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * RETURNS: control reg byte
 */

int ppreadctrl(
struct pp *pp )                   /* address of io structure */
{
	struct ppiost loc ;               /* structure for read char   */
	loc.address = (int)pp->ctrlreg ;
	loc.pp = pp ;

	return (ppreadchar(&loc)  ) ;
}


/*
 * NAME: ppwritectrl
 *
 * FUNCTION:
 *         This will write data to the control register supplied by the
 *         caller.
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * RETURNS:
 *      Passed back data from ppwritechar
 *
 */

int ppwritectrl(
struct pp *pp ,                   /* address of io structure */
char   data )                     /* the data to write to control    */
{
	struct ppiost loc ;               /* structure for read char   */
	int rc = 0 ;

	loc.address = (int)pp->ctrlreg ;
	loc.pp = pp ;
	loc.data = data ;                /*  set the data value passed */

	rc = ppwritechar(&loc) ;
	return (rc) ;
}


/*
 * NAME: ppwritestat
 *
 * FUNCTION:
 *         This will write data to the status register supplied by the       *
 *         caller.                                                           *
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * RETURNS:
 *      passed back return code (from ppwritechar)
 *
 */

int ppwritestat(
struct pp *pp ,                   /* address of io structure */
char   data )                     /* data to write to status reg   */
{
	struct ppiost loc ;               /* structure for read char   */
	int rc ;

	loc.address = (int)pp->statreg ;
	loc.pp = pp ;
	loc.data = data ;                /*  set the data value passed */

	rc = ppwritechar(&loc) ;
	return (rc) ;
}



/*
 * NAME: ppdelay
 *
 * FUNCTION: Delay for period specified in nanoseconds.
 *
 * EXECUTION ENVIRONMENT:  Process only
 *
 * RETURN VALUE: 0 if no error, PPABORT if a signal is caught
 *
 */
int
ppdelay(struct pp *pp, ulong nsecs)
{
	void pp_timer_pop();
	int old;
	int rc = 0;
	ulong ns, s;
	struct trb *tmr = pp->tmr;

	s = nsecs / NS_PER_SEC;
	ns = nsecs % NS_PER_SEC;
	tmr->timeout.it_value.tv_nsec = ns;
	tmr->timeout.it_value.tv_sec = s;
	tmr->ipri = INTTIMER;
	tmr->func_data = (ulong) pp;
	tmr->func = pp_timer_pop;

#ifdef _POWER_MP
        tmr->flags = T_MPSAFE;
        old = disable_lock(INTTIMER, &pp->intr_lock);
        tstart(tmr);
        if(THREAD_INTERRUPTED == e_sleep_thread(&pp->pp_event, &pp->intr_lock,
                 LOCK_HANDLER|INTERRUPTIBLE))
                        rc = PPABORT;
        unlock_enable(old, &pp->intr_lock);
        while(tstop(tmr));
#else
	tmr->flags = 0;
	old = i_disable(INTTIMER);
	tstart(tmr);
	if (EVENT_SIG == e_sleep(&pp->pp_event, EVENT_SIGRET))
		rc = PPABORT;
	i_enable(old);
	tstop(tmr);
#endif /* _POWER_MP */

	return(rc);
}

