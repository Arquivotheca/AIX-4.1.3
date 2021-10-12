#ifndef lint
static char sccsid[] = "@(#)07	1.37  src/bos/kernext/prnt/ppddt.c, sysxprnt, bos411, 9434A411a 8/11/94 09:46:27";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Port Device Driver
 *
 *   FUNCTIONS: ppalloc
 *              ppconverged
 *              ppfree
 *              ppget
 *              ppibmpc
 *              pppower
 *              ppreaddata
 *              ppsendff
 *		ppsendlist
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */


#include <sys/ppdd.h>
#include <sys/malloc.h>
#include <sys/lpio.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <errno.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>

#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _POWER_MP */

extern  struct pp *ppheader;
extern  struct pp *pptail;

#ifdef DEBUG
extern struct ppdbinfo ppdd_db ;
extern ulong *pbufp;
extern ulong *pbufstart;
#endif

struct ppa pp_adapter;		/* Adapter specific config data */



/*
 * NAME: ppalloc:
 *
 * FUNCTION:
 * Allocate memory for printer structure from the pended heap and put
 * into a linked list  and set up all dependent structures.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * INPUT
 *    header
 *
 * EXCEPTIONS -RETURNS
 *    If memory is available and every thing else is in good shape then
 *    return a pointer to storage.
 *    NULL if none was available or timer could not be set up.
 *
 */

struct pp *ppalloc(
int devno,              /* device number    */
struct uio *uiop,       /* uio structure containing device dependent data */
struct pp *pp_exist)    /* NULL if pp struct needs to be allocated */
{
	struct  pp   *pp;
	struct ppdds * ppdds = (struct ppdds *)uiop->uio_iov->iov_base;
	struct ppa *ppa = &pp_adapter;
	int reuse = 0;

	if (pp_exist)		/* Need to allocate a new one? */
		pp = pp_exist;	/* No - just reinitialize it */
	else {

	/* First look for the adapter device.  If it is in the list, */
	/* re-use its structure for this printer.  */
		for( pp = ppheader;
		   (pp != NULL) && (!(minor(pp->dev) & 0x8000)); 
		     pp = pp->next ) ;
		if (pp == NULL) {
			pp = (struct pp *) xmalloc(sizeof(struct pp), 
				2, pinned_heap) ;
			if( pp == NULL )
				return (pp) ;
			pp->next = NULL;
			if( ppheader == NULL )
				ppheader = pptail = pp;
			else {
				pptail->next = pp;
				pptail = pp;
			}
#ifdef _POWER_MP
			lock_alloc(&pp->intr_lock, LOCK_ALLOC_PIN, 
				PRNTDD_LOCK_CLASS, -1);
			simple_lock_init(&pp->intr_lock);
#endif /* _POWER_MP */

		} 
	}
	bcopy(ppdds->name,pp->ppelog.header.resource_name,16) ;

	/* Adapter specific data - init only when ppa configures */
	if (minor(devno) & 0x8000) {
		ppa->bus_id    = ppdds->bus_id |0x800c0060;
		ppa->rg        = ppdds->rg ;
		ppa->cidreg1   = ppdds->posadd ;
	}
	pp->statreg          = ppa->rg+PPSTATREG ;
	pp->ctrlreg          = ppa->rg+PPCTRLREG ;
	pp->datareg          = ppa->rg+PPDATAREG ;
	pp->cidreg1          = ppa->cidreg1 | PPCIDREG ;
	pp->cidreg2          = pp->cidreg1 + 1 ;
	pp->bus_id	     = ppa->bus_id;
#ifdef _PP_ISA_BUS
	pp->iomap.key = IO_MEM_MAP;
	pp->iomap.flags = 0;
	pp->iomap.size = PAGESIZE;
	pp->iomap.bid = ppa->bus_id;
	pp->iomap.busaddr = 0;
#endif	/* _PP_ISA_BUS */

	/* Printer specific config data */
	pp->prt.col          = ppdds->pp_col_default ;
	pp->prt.line         = ppdds->pp_lin_default ;
	pp->prt.ind          = ppdds->pp_ind_default ;
	pp->ppmodes          = ppdds->modes ;
	pp->v_timout         = ppdds->v_timeout;
	pp->interface        = ppdds->interface;
	pp->busy_delay       = ppdds->busy_delay;
	pp->dev              = devno;

	pp->flags.ppread = 0;  /* clear all flags at init time */
	pp->flags.ppwrite= 0;
	pp->flags.ppopen = 0;
	pp->flags.inited = 0;
	pp->flags.ppiodone = 0;
	pp->flags.ppdone = 0;
	pp->flags.ppinitfail = 0;
	pp->flags.ppcancel = 0;
	pp->flags.ppoverstrike = 0;
	pp->flags.ppdiagmod = 0;
	pp->flags.pptimerset = 0 ;
	pp->flags.pptimeout = 0 ;
	pp->flags.errrecov = 0 ;
	pp->flags.ppcharsent = 0 ;
	pp->flags.ppff_needed = 0 ;
	pp->flags.dontsend = 0 ;
	pp->write_lock = LOCK_AVAIL;
	pp->wdtcnt = 0;
	pp->pp_event = EVENT_NULL;
	pp->prt.chars_sent = 0;
	pp->prt.outq.c_cc = 0;
	pp->prt.outq.c_cf = NULL;
	pp->prt.outq.c_cl = NULL;
	pp->ppelog.header.error_id = ERRID_PPRINTER_ERR1 ;
	return(pp);
} /* end ppalloc */


/*
 * NAME: ppconverged :
 *
 * FUNCTION: interpret and return status to caller for converged interface
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: device status
 */

int ppconverged(
char statusreg,
struct pp * pp)
{
	int status = 0 ;             /* return status set to zero to start */
	char stnum ;                 /* original status reg                */

	stnum = pppulst(statusreg,pp) ;
	switch (stnum)
	{
		/*  paper out condition                */
	case PPPE :
		status |= LPST_POUT;
		break ;
	case PPDESELECT :
		/* check to see if printer selected             */
		status |= LPST_NOSLCT;
		break ;
	case PPECCERROR    :           /* ECC error                           */
	case PPASFJAM      :           /* ASF JAM                             */
	case PPRESERVED2   :           /* Reserved                            */
	case PPRESERVED6   :           /* Reserved                            */
	case PPHEADALARM   :           /* High temperature head alarm         */
	case PPCANREQ      :           /* Cancel switch is depressed          */
	case PPRESERVED9   :           /* Reserved                            */
	case PPRESERVED5   :           /* Reserved                            */
	case PPRESERVED11  :           /* Reserved                            */
	case PPRESERVED13  :           /* Reserved                            */
	case PPPERMERROR   :           /* Printer dead                        */
	case PPNOTRMR      :           /* Not receive machine ready           */
		/* check for unspecified error                  */
		status |= LPST_ERROR;
		break ;
	}
	/* check to see if printer is busy              */
	if( !(statusreg & PP_BUSY_BIT)) /* for select bit, on = 1 */
		status |= LPST_BUSY;
	/* check for software error                     */
	/* NOT IMPLEMENTED */
	return(status) ;
}


/*
 * NAME: ppfree:
 *
 * FUNCTION: frees pp structure
 *
 *    This code will remove a printer structure from the link list and free
 *    any other linked  services.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * INPUT
 *     pointer to structure to free
 *
 * EXCEPTIONS -RETURNS
 *     NONE
 *
 */

void ppfree(
struct pp *pp)
{
	struct pp *old;

	if( pp == ppheader ) {
		ppheader = ppheader->next ;
		if ( ppheader == NULL )
			pptail = NULL ;
	} else {
		/* Search list for pp and remove it if found */
		for(old = ppheader;
		    old->next != pp ;
		    old = old->next )
			;
		old->next = old->next->next ;
		if( old->next == NULL )
			pptail = old;
	}
#ifdef _POWER_MP
	lock_free(&pp->intr_lock);
#endif /* _POWER_MP */
	xmfree((void *)pp,pinned_heap);
	return;
}	/* ppfree() */


/*
 * NAME: ppget
 *
 * FUNCTION: Get pp structure associated with minor device number
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: Struct pp address is returned or a NULL.
 */

struct pp *ppget(
int dev)        /* minor device number */
{
	struct pp *pp;

	for( pp = ppheader;
	     pp != NULL && ( minor(pp->dev) != dev );
	     pp = pp->next );
	return(pp);
}


/*
 * NAME: ppibmpc:
 *
 * FUNCTION: interpret device status and return for IBM interface
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: device status
 */

int ppibmpc(char statusreg)
{
	int status = 0 ;             /* return status set to zero to start */

	/* check for paper out condition                */
	if( statusreg & PP_PE )         /* for paper end, on = 1 */
		status |= LPST_POUT;

	/* check for unspecified error                  */
	if( !(statusreg & PP_ERROR_B) ) /* for ERROR bit, on = 0 */
		status |= LPST_ERROR;

	/* check to see if printer selected             */
	if( !(statusreg & PP_SELECT) )  /* for select bit, on = 1 */
		status |= LPST_NOSLCT;

	/* check to see if printer is busy              */
	if( !(statusreg & PP_BUSY_BIT)) /* for busy bit, on = 0   */
		status |= LPST_BUSY;

	/* check for software error                     */
	/* NOT IMPLEMENTED */
	return(status) ;
}


/*
 * NAME: pppower:
 *
 * FUNCTION: Test to see if the power is off on the printer
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * RETURNS: device status
 *          0 - power is off
 *          1 - power is on
 */


int pppower(
    struct pp * pp,                     /* printer structure pointer  */
    int tcount )                        /* count in ten second steps  */
{
	int retcd = 0 ;                 /* set the power on flag      */
	int  stat ;                     /* status                     */
	int  count = 0 ;                /* count for while loop       */

	do {
		retcd = 1 ;
		stat = ppreadstat (pp) ;
		if (stat == -1)
			return (0) ;
		switch (pp->interface) {
		case PPCONVERGED :
			stat = ppconverged ( (char)stat, pp ) ;
			if ( stat & LPST_ERROR )
				retcd = 0 ;
			break ;
		case PPIBM_PC :
			if (!(stat & PP_ACK))/* if the ack bit is off (ack on)*/
				retcd = 0 ;
			else
				/*TEST for select bit on (=1)*/
				if((stat & PP_SELECT) && (stat & PP_PE))
					retcd = 0;
			break ;
		}
		if ((retcd == 0) && tcount)
			delay(HZ) ;
		count++ ;
	 } while ( retcd == 0 && count < 10*tcount);
	 return (retcd) ;
}


/*
 * NAME: ppreaddata
 *
 * FUNCTION:
 *         This will read the data register and return it to the caller
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * RETURNS: the data read
 */

int ppreaddata(
struct pp *pp )                   /* address of io structure */
{
	struct ppiost loc ;                   /* structure for read char    */

	loc.address = (int)pp->datareg ;
	loc.pp = pp ;
	return ( ppreadchar( &loc) );
}


/*
 * NAME: ppsendff
 *
 * FUNCTION:
 *         Builds a list of characters consisting of a single form
 *         feed or enough new lines to achieve the equivalent of a,
 *         form feed,  the  means of causing a form feed depending
 *         on whether the device is in PLOT mode or not.  The list
 *         is then sent to the printer with a call to ppsendlist.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 *
 * RETURNS:
 *         - PPCONTINUE                                                      *
 *         - PPABORT                                                         *
 */

ppsendff(
struct pp *pp)
{
	int plotflag = 0 ;
	int retcd = PPCONTINUE ;
	int rc ;

	DDHKWD1(HKWD_DD_PPDD, PP_SNDFF, pp->error, pp->dev);
	if (!pp->prt.line)	/* No FF if line=0 */
		return(retcd);
	if( pp->ppmodes & PLOT ) {
		plotflag = TRUE ;
		pp->ppmodes &= ~PLOT ;
	}
	prnformat(&pp->prt, pp->ppmodes, FF);  /*put form feed on out queue*/
	rc = ppsendlist(pp);
	if( (rc == PPCONTINUE) || (rc == PPDONE) )
		pp->flags.ppff_needed = 0 ; /* reset form feed needed flag */
	else
		if (rc == -1 )
			retcd = -1 ;
		else
			retcd = PPABORT ;
	if (plotflag)
		pp->ppmodes |= PLOT ;
	return (retcd) ;
}



/*
 * NAME: ppsendlist
 *
 * FUNCTION:
 *
 * 	Outputs characters from the output queue in the pp structure.
 *	If the printer can keep up, the characters are output here
 *	with no delay.  This is the "fast path", which results in
 *	the best possible data throughput rate.
 *
 *	If the printer is busy, a timer is started and
 *	the process sleeps.  The timer routine outputs the next
 *	character if the printer is ready.  Control returns to this
 *	routine (via a wakeup from the timer routine) when (1) the
 *	output queue is depleted; (2) a signal is caught; (3) a
 *	printer error occurs; or (4) the printer is ready for 2
 *	characters without delay.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * (NOTES:)
 *
 * RETURNS:
 *         - PPCONTINUE: continue processing
 *         - PPABORT: error - abort processing
 *	   - PPDONE: buffer empty
 *         - ENXIO if returned data for sendchar or sendnio is -1
 */

ppsendlist(
struct pp *pp)
{
	int rc;
	int busy_loop = 0;
	int retcd = PPCONTINUE ;
	void ppstart_tmr();
	void pp_proc_timeout();
	void io_delay();

	DDHKWD1(HKWD_DD_PPDD, PP_SNDLST_AB, 0, pp->dev);
	pp->nsecs = PP_WAIT_MIN;
	pp->btimeout = 0;
	pp->bflag = 0;
	pp->avgw = 0;
	pp->countw = 0;
	pp->totw = 0;
	pp->timeoutflag = 0;
	while (retcd == PPCONTINUE) {
		/* delay for slow printers */
		if ((pp->busy_delay) && (busy_loop == 0)) {
			if (pp->busy_delay < 100) {
			    io_delay(pp->busy_delay);
			} else {
			    pp->flags.dontsend = 1;
			    if (PPABORT == ppdelay(pp, 1000 * pp->busy_delay)) {
			        pp->flags.dontsend = 0;
			        return(PPABORT);
			    }
			    pp->flags.dontsend = 0;
			}
		}
		rc = ppsendchar(pp) ;	/* try to send a character */
		switch(rc) {
			case PPBUSY:		/* printer was busy */
		/* loop a few times to avoid flopping between timer and proc */
				if (busy_loop++ == 0)
					ppstart_tmr(pp, 2*PP_WAIT_MIN, 
						    pp_proc_timeout);
				if (pp->timeoutflag == 1) 
					pp->timeoutflag++;
				else if (pp->timeoutflag == 2) {
					pp->timeoutflag = 0;
					busy_loop = 0;
					if (PPABORT == ppdelay(pp, pp->nsecs))
					    return(PPABORT);
					retcd = pp->pop_rc;
				}
				break;
			case PPCONTINUE:	/* printer was ready */
#ifdef DEBUG	/* count hits */
if (pp->bflag)
  if (ppdd_db.waitmin > pp->nsecs) ppdd_db.waitmin = pp->nsecs;
if (pp->bflag <= 39) ppdd_db.hits++;
if (ppdd_db.bflagmax < pp->bflag) ppdd_db.bflagmax = pp->bflag;
if (pbufp > (pbufstart+4094)) pbufp = pbufstart; /* circular */
*pbufp++ = pp->bflag;
*pbufp++ = pp->avgw;
#endif
				pp->nsecs = PP_WAIT_MIN;
				break;
			case PPDONE:		 /* finished */
				retcd = PPDONE;
				break;
			case -1:
			case ENXIO:		/* error */
				retcd = ENXIO;
				break;
		}
		/* Turn off the timer if necessary */
		if ((busy_loop) && (rc != PPBUSY)) {
#ifdef _POWER_MP
			while(tstop(pp->tmr));
#else
			tstop(pp->tmr);
#endif /* _POWER_MP */
			busy_loop = 0;
			pp->timeoutflag = 0;
		}
	}
	return(retcd);
} /* end ppsendlist */

