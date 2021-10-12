#ifndef lint
static char sccsid[] = "@(#)54 1.9 src/bos/kernext/prnt/ppwrite.c, sysxprnt, bos41B, 9505A 1/24/95 13:36:25";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Printer Parallel Port Device Driver
 *
 *   FUNCTIONS: ppwrite
 *              ppwrited
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

/*
 *   NAME: ppwrite
 *
 *   FUNCTIONS:
 *
 *      Writes data to the printer. Optionally performs Form Feed prior to
 *      write if required. Will not return  until  write is complete, cancelled
 *      or an error occurs.  Optionally translates the data if PLOT is not
 *      enabled. The printer structure  is locked during the write so that
 *      two or more processes can not attempt to write at once.
 *
 *      The write routine contains a loop to  feed data to the clists or DMA
 *      in manageable chunks.  This  is  necessary because of the limited
 *      amount of clist/pined memory.
 *
 *      Note that the so-called "i/o done" flag, when set, does not simply
 *      mean that i/o has been done, but rather that i/o has been done AND
 *      that the last i/o done was not done in PLOT mode.
 *
 * INPUT
 *      device number
 *      uio structure pointer
 *
 * EXCEPTIONS -RETURNS
 *
 *      If no error is found then return with a ZERO return code.
 *      If a signal is received while waiting for a resource then return an
 *          EINTR to the caller
 *      If the device has not been configured then return an ENXIO to the
 *          caller.
 *      If running in wrap mode and the space from the indent to the last
 *          column is less than 5 and the data being printed is larger than four
 *          then the error EINVAL  will be returned.
 *      If the printer is not powered on at the start of the write then try
 *          for ten seconds and if it is not up by that time return to the caller
 *          with a ENOTREADY return code and the time-out flag set.
 *      If Power is lost after the data has started going to the device then
 *          the error return code ENOTREADY will be returned to the caller.
 *      If at any time the device stops taking data for more that the preset
 *          time out value then a zero will be returned to the caller and the
 *          count of the amount of data sent to the device will be returned. The
 *          time out flag will be set.  If no data was sent to the printer,
 *          an error code of ENOTREADY will be returned.
 */


#include <sys/ppdd.h>
#include <errno.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/lpio.h>

extern ppglobal_lock ;


ppwrite(
dev_t dev,                      /* major and minor device number  */
struct uio *uio,                /* uio structure pointer          */
int chl,                        /* channel number (not used       */
int ext)                        /* extension (not used)           */
{
	struct pp *pp;               /*  pointer to printer struct           */
	int retcd;                   /*  return codes                        */

	DDHKWD5(HKWD_DD_PPDD, DD_ENTRY_WRITE, 0, dev, uio->uio_resid,
	    (int)uio->uio_iovcnt, (off_t)uio->uio_offset, (int)uio->uio_fmode);
	(void) lockl((lock_t *)&ppglobal_lock, LOCK_SHORT) ;
	pp = ppget(minor(dev)) ;                /* check to see if this is here */
	unlockl((lock_t *)&ppglobal_lock) ;
	if( pp == NULL ) {
		DDHKWD1(HKWD_DD_PPDD, DD_EXIT_WRITE, ENXIO, dev);
		return (ENXIO);
	}      
	(void) lockl((lock_t *) &pp->write_lock, LOCK_SHORT ) ;

	if( (pp->ppmodes & WRAP) && ((
		 pp->prt.col - pp->prt.ind < 5) && (uio->uio_resid > 5)) )
		retcd = EINVAL ;
	else {
		retcd = 0 ;
   		if ((( pp->ppmodes & IGNOREPE ) == 0) &&
		     ( pppower(pp, 1) == 0 )) {
			pp->flags.pptimeout = TRUE ;
			retcd = ENOTREADY ;
		} else {
			pp->uio = uio ;
			pp->flags.writeflag = TRUE ;
			retcd = ppwrited(pp) ;
			if(pp->flags.pptimeout)
   			if ((( pp->ppmodes & IGNOREPE ) == 0) &&
			     ( pppower(pp, 0) == 0 ))
				retcd = ENOTREADY ;
		}
	}
	unlockl((lock_t *) &pp->write_lock ) ;
	DDHKWD1(HKWD_DD_PPDD, DD_EXIT_WRITE, retcd, dev);
	return(retcd);
}  /* ppwrite() */


/*
 * NAME: ppwrited :
 *
 * FUNCTION:
 *      This section of code is to write data to the printer in any mode.
 *      When enterred all test for device available and validity of command
 *      has been done. This code will start sending data to the printer.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * INPUT:
 *
 *      pointer to printer structure
 *
 * EXCEPTIONS -RETURNS:
 *
 */

int ppwrited(struct pp *pp)
{
	int retcd = 0, c, rc;
	int org_resid;

	pp->flags.pptimeout = FALSE ;
	pp->error = 0 ;
	/* If a form feed is needed put it in the queue */
	if( pp->flags.ppff_needed ) {
		pp->flags.ppdone = FALSE;
		retcd = ppsendff(pp) ;
		if( retcd != PPCONTINUE ) {
			DDHKWD1(HKWD_DD_PPDD, DD_EXIT_WRITE, pp->error, pp->dev);
			if( retcd == PPABORT )
				return (0) ;
			else
				return (ENXIO) ;
		}
	}

	retcd = 0 ;
	/* save original value of user byte count */
	org_resid = pp->uio->uio_resid;

	while (pp->uio->uio_resid > 0) {    /* while data avail to send */
		pp->wdtcnt = 0;    /* set watchdog timer to zero */
		if (pp->flags.errrecov) {
			if ((char)*pp->uio->uio_iov->iov_base ==
			    pp->lastchar) {
				c = uwritec(pp->uio) ;
				pp->flags.errrecov = 0 ;
			}
		} else {
			pp->flags.ppdone = FALSE ;
			pp->buf_index = 0 ;
			if(pp->ppmodes & PLOT ) {
				if ( pp->uio->uio_resid > pp->buf_size ) {
					pp->buf_count = pp->buf_size ;
				} else {
					pp->buf_count=pp->uio->uio_resid;
				}
				/* The following move is done because
				 * during error recovery control is passed
				 * back to the caller and the data being sent
				 * is stell set up for DMA. With this in mine
				 * it can not be under user control
				 */
				rc = uiomove(pp->buffer, pp->buf_count,
						     UIO_WRITE, pp->uio) ;
				if (rc) {
					c = -1 ;
					pp->uio->uio_resid++ ;
					pp->prt.outq.c_cc = 0 ;
				}
				pp->lastchar = pp->buffer[pp->buf_count-1] ;
			} else { /* not plot */
				do {
					c = uwritec(pp->uio) ;
					if ( c == -1 ) {
						break ;
					}
					pp->lastchar = c ;
					prnformat(&pp->prt, pp->ppmodes, c);

				} while ( pp->uio->uio_resid > 0 &&
					pp->prt.outq.c_cc<= PPCLISTMAX) ;
			}
			/* still valid test ????? */
			if( (c == -1) && (pp->uio->uio_resid != 0) &&
			    (pp->prt.outq.c_cc ==0)){
				/* illegal memory access */
				DDHKWD1(HKWD_DD_PPDD, DD_EXIT_WRITE, pp->error,
					pp->dev);
				return(EFAULT);
			}
		}
		rc = ppsendlist(pp);
		if( (rc != PPCONTINUE) && (rc != PPDONE) ) {
			retcd = TRUE ;
			break;
		} else {
			pp->flags.errrecov = 0 ;
		}
	} /* end of data to send loop */

	if (retcd) {
		if (!pp->flags.errrecov) {
			pp->flags.errrecov = TRUE ;
			pp->uio->uio_resid++ ;
		}
		if (org_resid == pp->uio->uio_resid)
			pp->error = ENOTREADY;
	} else {
		pp->flags.errrecov = 0 ;
	}
	/* test for non-plot mode io */
	if( pp->uio->uio_resid == 0 ) {
		/* i/o has been done */
		if( pp->ppmodes & PLOT ) {
			/* i/o done in PLOT mode so it doesn't count toward **
			** causing a form feed */
			pp->flags.ppiodone = FALSE ;
		} else {
			/* i/o not done in PLOT mode. PPIODONE is set **
			** so that a form feed will be done when we **
			** close */
			pp->flags.ppiodone = TRUE ;
		}
	}

	DDHKWD1(HKWD_DD_PPDD, DD_EXIT_WRITE, pp->error, pp->dev) ;
	return(pp->error) ;
}  /* ppwrited() */
