#ifndef lint
static char sccsid[] = "@(#)41 1.7 src/bos/kernext/prnt/POWER/ppddm_bot.c, sysxprnt, bos411, 9428A410j 5/3/94 17:42:44";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Printer Parallel Port Device Driver
 *
 *   FUNCTIONS: ppreadchar
 *              ppreadstat
 *              ppwritedata
 *              ppopen_acc
 *              ppddpin
 *              pp_pio_retry
 *              pp_pio_putc
 *              pp_pio_getc
 *		pp_timer_pop
 *		pprestart_tmr
 *		pp_proc_timeout
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
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/ppdd.h>
#include <sys/lpio.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <errno.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/timer.h>

/* Externally Defined Variables */
#define PPCTRLINIT      PP_INIT
#define NIONORST        0x040   /* turn off para port reset to NIO */

#ifdef DEBUG
struct ppdbinfo ppdd_db ;
ulong *pbufp;
ulong *pbufstart;
ulong pbuffer[4096];
#endif


/*
 * NAME: ppreadchar
 *
 * FUNCTION:
 *         This will read one character from the printer bus and save
 *         it in the structure passed to it(ppiost).
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * (RECOVERY OPERATION:) If an error occurs during this operation then
 *      the ppreadcharrc function is call by PIO recovery to complete the
 *      operation.
 *
 * RETURNS: data read from the requested port. This is passed in data of
 */

int ppreadchar(
	struct ppiost *loc )              /* address of io structure    */
{
	int retcd ;
	struct pp * pp = loc->pp ;

#ifdef _PP_ISA_BUS
	loc->busacc = (int)iomem_att(&pp->iomap);
#else
	loc->busacc = (int)BUSIO_ATT(pp->bus_id, 0) ;
#endif	/* _PP_ISA_BUS */

	retcd = PP_PIO_GETC ;

#ifdef _PP_ISA_BUS
	iomem_det(loc->busacc);
#else
	BUSIO_DET(loc->busacc) ;
#endif	/* _PP_ISA_BUS */

	return(retcd) ;
}


/*
 * NAME: ppreadstat
 *
 * FUNCTION:
 *         This will read the status register and save it for the caller
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * (NOTES:)
 *         fill in the ppiost structure
 *         call read char
 *         return to caller
 *
 * RETURNS: status of pp port
 */

int ppreadstat(
struct pp *pp )                     /* address of io structure */
{
	struct ppiost loc ;               /* structure for read char   */
	loc.address = (int)pp->statreg ;
	loc.pp = pp ;
	return ( ppreadchar(&loc) ) ;
}


/*
 * NAME: ppddpin
 *
 * FUNCTION:
 *         This is the name that will be pined and un pined by the system
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * RETURNS: none never called
 */

int ppddpin()
{
	return(0) ;
}


/*
 * NAME: ppwritedata
 *
 * FUNCTION:
 *         This will write data to the data register supplied by the
 *         caller.
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * RETURNS: none
 */

int ppwritedata(
struct pp *pp ,                    /* address of io structure */
char   data )                      /* data to write out to printer */
{
	struct ppiost loc ;               /* structure for read char   */

	loc.address = (int)pp->datareg ;
	loc.pp = pp ;
	loc.data = data ;                /*  set the data value passed */
	if (ppwritechar(&loc))
		return (-1) ;
	return (0) ;
}


#ifndef _PP_ISA_BUS
/***********************************************************************
 *
 *  NAME pp_pio_retry
 *
 *  FUNCTION: This routine is called when a pio routine returns an
 *      exception.  It will retry the PIO and do error logging.
 *
 *  EXECUTION ENVIRONMENT:
 *      Called by interrupt and processes level routines
 *
 *  RETURNS:
 *      0 - Execution was retried successfully
 *      exception code of last failure in not successful
 *
 *
 ************************************************************************/

 int                                            /* return code                */
 pp_pio_retry(                                  /* entry point                */
struct ppiost *loc,                             /* pointer to io sturcture    */
int excpt_code )                                /* exception code             */
{

	DDHKWD5(HKWD_DD_PPDD, DD_ENTRY_INTR, 0, loc->pp->dev,
						loc->data,
						loc->address,
						loc->pp,
						excpt_code) ;

	if (loc->opflag == PUTC ) {
		excpt_code = BUS_PUTCX( (char *)loc->busacc + loc->address,
					(char) loc->data) ;
	} else {
		excpt_code = BUS_GETCX( (char *)loc->busacc + loc->address,
					(char *)&loc->data);
	}
	if (excpt_code) {
		loc->data2 = 1 ;        /* set data two to hard error */
		ppelog_log(loc) ;
		return(-1);
	} else  {
		loc->data2 = 0 ;        /* set data two to soft error */
		ppelog_log(loc) ;
		return ((int)loc->data) ;
	}
}
#endif


/****************************************************************************
 *
 * NAME: pp_pio_getc
 *
 * FUNCTION:
 *      Get a character from io space, with exception handling.
 *
 * EXECUTION ENVIRONMENT:
 *      Called by interrupt and processes level routines
 *
 *  RETURNS:
 *       0 successful completion
 *      -1 on failure
 *
 ************************************************************************/

int
pp_pio_getc(
struct ppiost *loc)                             /* pointer to io sturcture    */
{
	uint rc ;

#ifdef _PP_ISA_BUS
	__iospace_sync();
	rc = (uint)BUS_GETC(loc->busacc + loc->address);
#else
	rc = BUS_GETCX((char *)loc->busacc + loc->address, (char *)&loc->data);
	if (rc) {
		loc->opflag = GETC ;
		rc = pp_pio_retry( loc, rc) ;
	} else
		rc = (uint) loc->data ;
#endif	/* _PP_ISA_BUS */

	return(rc) ;
}


/****************************************************************************
 *
 * NAME: pp_pio_putc
 *
 * FUNCTION:
 *      Put a character to io space, with exception handling.
 *
 * EXECUTION ENVIRONMENT:
 *      Called by interrupt and processes level routines
 *
 *  RETURNS:
 *       0 successful completion
 *      -1 on failure
 *
 ************************************************************************/

int
pp_pio_putc(
struct ppiost *loc)                             /* pointer to io sturcture    */
{
	int rc ;

#ifdef _PP_ISA_BUS
	BUS_PUTC(loc->busacc + loc->address, loc->data);
	__iospace_eieio();
	rc = 0;
#else
	rc = BUS_PUTCX( (char *)loc->busacc + loc->address, (char)loc->data) ;
	if (rc) {
		loc->opflag = PUTC ;
		rc = pp_pio_retry( loc, rc) ;
	}
#endif	/* _PP_ISA_BUS */
	return(rc) ;
}


/*
 * NAME: pp_timer_pop
 *
 * FUNCTION:
 *	This routine is first called when the timer setup by ppsendlist
 *	expires.  It checks to see if the printer is ready, and outputs
 *	the next character if so.  If not, it restarts the timer.
 *
 *	The total time spent waiting for the printer to go ready is
 *	accumulated and used to keep a running average delay.  This
 *	average is used as a starting point for the next character
 *	delay.  Large delays are weighted to avoid slewing the delay
 *	upward too fast.
 *
 *	The sleeping process is awakened if (1) the output queue is
 *	depleted; (2) an error occurs; or (3) the printer is ready
 *	immediately after a character is output (we can't stay here
 *	long, so we return to process context to loop).
 *
 * EXECUTION ENVIRONMENT:  Interrupt
 *
 * RETURN VALUE: none
 *
 */
void
pp_timer_pop(struct trb *tmr)
{
	struct pp *pp;
	int c_cnt = 0;
	int retcd = PPCONTINUE ;
	int rc;
	void ppstart_tmr();
#ifdef _POWER_MP
	int old_pri;
#endif /* _POWER_MP */

	pp = (struct pp *)tmr->func_data;

#ifdef _POWER_MP
	old_pri = disable_lock(INTTIMER, &pp->intr_lock);
#endif /* _POWER_MP */

	while ((c_cnt < 2) && (retcd == PPCONTINUE) && (!pp->flags.dontsend) &&
		!(pp->busy_delay && c_cnt)) {
	    rc = ppsendchar(pp) ;	/* try to send a character */
	    switch(rc) {
		case PPBUSY:		/* printer was busy */
			/* check for errors once a second */
			if (pp->btimeout >= NS_PER_SEC) {
			    pp->wdtcnt++;  
			    pp->btimeout = 0;
			    if ((pp->wdtcnt > pp->v_timout) &&
			      (pp->ppmodes & RPTERR)) {
				pp->flags.pptimeout = TRUE ;
				retcd = PPBUSY;
				break;
			    }
			}
			if (pp->bflag) {		/* still busy? */
			    if (pp->bflag < 6)	/* short busy */
				pp->nsecs >>= 1;
			    if (pp->bflag > 16) {	/* long busy */
				pp->nsecs += pp->nsecs>>2;
				if (pp->wdtcnt > 5) /* very long */
				    pp->nsecs = PP_WAIT_LONG;
				else
				    if (pp->nsecs > PP_WAIT_SHORT)
					pp->nsecs = PP_WAIT_SHORT;
			    }
			} else {	/* ready-to-busy transition */
			    if (pp->nsecs < PP_WAIT_SHORT) {
				if (pp->totw < pp->avgw) { /* fast delay dec */
				    pp->avgw = pp->totw;
				    pp->countw = 1;
				} else {
				    if (pp->avgw && ((pp->totw>>2) > pp->avgw))
					pp->totw = pp->avgw<<2 ; /* limit inc */
				    pp->avgw = (pp->countw*pp->avgw + pp->totw)
					/ (pp->countw + 1); /* running avg */
				    if (++pp->countw > 8);  /* last 8 only */
					pp->countw = 8;
				}
			    }
			    pp->nsecs = pp->avgw>>1;	/* start at 1/2 */
			    pp->totw = 0;
			}
			if (pp->nsecs < PP_WAIT_MIN) 
			    pp->nsecs = PP_WAIT_MIN;
			pp->btimeout += pp->nsecs; /* how long have we waited */
			pp->totw += pp->nsecs;

#ifdef DEBUG	/* count misses */
if (pp->bflag == 39) ppdd_db.misses++;
if (pp->nsecs > ppdd_db.waitmax) ppdd_db.waitmax = pp->nsecs;
if (pp->avgw > ppdd_db.waitavg) ppdd_db.waitavg = pp->avgw;
if (pp->countw > ppdd_db.countw) ppdd_db.countw = pp->countw;
#endif

			pp->bflag++;
			ppstart_tmr(pp, pp->nsecs, pp_timer_pop);
#ifdef _POWER_MP
			unlock_enable(old_pri, &pp->intr_lock);
#endif /* _POWER_MP */
			return;
		case PPCONTINUE:	/* printer was ready */
			if ((pp->wdtcnt == 0) && 
			    (pp->btimeout <= 2*PP_WAIT_MIN))
				c_cnt = 2;
			pp->wdtcnt = 0;
			pp->btimeout = 0;
#ifdef DEBUG	/* count hits */
if (pp->bflag)
if (ppdd_db.waitmin > pp->nsecs) ppdd_db.waitmin = pp->nsecs;
if (pp->bflag <= 39) ppdd_db.hits++;
if (ppdd_db.bflagmax < pp->bflag) ppdd_db.bflagmax = pp->bflag;
if (pbufp > (pbufstart+4094)) pbufp = pbufstart; /* circular */
*pbufp++ = pp->bflag;
*pbufp++ = pp->avgw;
#endif
			pp->bflag = 0;
			c_cnt++;
			break;
		case PPDONE:		 /* finished */
			retcd = PPDONE;
			break;
		case -1:
		case ENXIO:		/* error */
			retcd = ENXIO;
			break;
	    }
	}
	pp->pop_rc = retcd;
#ifdef _POWER_MP
	unlock_enable(old_pri, &pp->intr_lock);
#endif /* _POWER_MP */
	e_wakeup(&pp->pp_event);	/* wake up ppsendlist process */
}



/*
 * NAME: pp_proc_timeout
 *
 * FUNCTION:
 *	This routine is called when the timer setup by ppsendlist
 *	expires.  The timer is set to notify the process when to 
 *	change from process level polling to interrupt level polling.
 *	The assumption is that once this timer has popped the printer
 *	will be busy for a "long" time and we wish to sleep.
 *
 * EXECUTION ENVIRONMENT:  Interrupt
 *
 * RETURN VALUE: none
 *
 */
void
pp_proc_timeout(struct trb *tmr)
{
	struct pp *pp;

	pp = (struct pp *)tmr->func_data;
	pp->timeoutflag = 1;
}


/*
 * NAME: ppstart_tmr
 *
 * FUNCTION: Start timer for period specified in nanoseconds.
 *
 * EXECUTION ENVIRONMENT:  Interrupt or Process
 *
 * RETURN VALUE: None 
 *
 */
void
ppstart_tmr(struct pp *pp, ulong nsecs, void (*func)())
{
	ulong ns, s;
	struct trb *tmr = pp->tmr;

	s = nsecs / NS_PER_SEC;
	ns = nsecs % NS_PER_SEC;
	tmr->timeout.it_value.tv_nsec = ns;
	tmr->timeout.it_value.tv_sec = s;
	tmr->ipri = INTTIMER;

#ifdef _POWER_MP
	tmr->flags = T_MPSAFE;
#else
	tmr->flags = 0;
#endif /* _POWER_MP */

	tmr->func_data = (ulong)pp;
	tmr->func = func;
	tstart(tmr);
}

