#ifndef lint
static char sccsid[] = "@(#)51 1.3 src/bos/kernext/prnt/ppopen.c, sysxprnt, bos411, 9428A410j 12/15/93 07:11:47";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Printer Device Driver
 *
 *   FUNCTIONS: ppopen
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 *
 * NAME: ppopen
 *
 * FUNCTIONS:
 *
 *      Called to make the printer available for use. In this routine the
 *      status of the printer is tested and if the request can be done then
 *      the device is marked open. 
 *      The device can be opened for write by only one process if
 *      a second open is tried an error is returned. If this is the first
 *      device to be opened,  the interrupt handler is initialized and the non
 *      paging portion of the code is pined and enables the hardware and
 *      hardware interrupts.
 *
 *      This function will execute in process state and can page fault
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function will execute in process state and can page fault
 *
 * INPUT:
 *      device number
 *      mode of open
 *      channel number
 *      extension
 *
 * EXCEPTIONS -RETURNS:
 *
 *      If no error is found then return with a ZERO return code.
 *      If a signal is received while waiting for a resource then return an
 *          EINTR to the caller
 *      If the device has not been configured then return an ENXIO to the
 *          caller.
 *      If opening with a write or read/write request and the device is open
 *          for writing then return an EBUSY to the caller.
 *      If opening for LPDIAGMOD (ext code) or  open for DIAGMOD then return
 *          an EINVAL to caller.
 *      If the pined code could not be pined then return the code that was
 *          passed back from pincode system call.
 *      If the hardware could not be set up, pass the return code back.
 *      If DMA is available for this device but the DMA function could not
 *      be initialized then return ENXIO.
 *      If the power is off at the device and opening with delay (NDELAY flag
 *      clear) test the printer for ten seconds and if the power did not come
 *      up then return to caller with ENXIO.
 *
 */

#include <sys/ppdd.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <errno.h>
#include <sys/malloc.h>
#include <sys/lpio.h>

extern ppglobal_lock ;


ppopen(
dev_t dev,        /* major and minor device number */
ulong mode,       /* defined in file.h */
int chn,          /* channel number     */
int ext)          /* extension val     */
{
    struct  pp      *pp;
    int     retcd = 0 ;                  /* return code to be passed back  */

    DDHKWD5(HKWD_DD_PPDD, DD_ENTRY_OPEN, 0, dev, mode,0,0,0);
    /* Lock Global Lock, waits for lock or returns early on signal */
    (void) lockl((lock_t *)&ppglobal_lock, LOCK_SHORT ) ;
    /* Validate device */
    pp = ppget(minor(dev)) ;             /* request the print sturct pointer */
    if(pp == NULL)
	retcd = ENXIO ;                  /* did not find one                  */
    else {
	/* is printer powered on, ok if not delaying                          */
	if ( !(mode & FNDELAY) && (pppower(pp,1) == 0) ) {
	    retcd = ENXIO ;             /* power is off on the printer        */
	} else {
	    if( pp->flags.ppopen ) {              /* is the printer open      */
		if( ext == LPDIAGMOD )            /* is this open a lpdiag    */
		    retcd = EINVAL ;              /* they are bad boys        */
		else {
		    if( mode & FWRITE ) {         /* is this a write request  */
			if( pp->flags.ppwrite )   /* are we open for write    */
			    retcd = EBUSY;
			else
			    pp->flags.ppwrite  = TRUE ; /* ok set writing     */
		    }
		}
	    } else {                              /* printer not open         */
		retcd = pincode(ppddpin);         /* pin the code             */
		if (retcd == 0) {                 /* did it work              */
		    /* do any machine specfic open code                       */
		    retcd = ppopen_dev(pp) ;
		    if( retcd != 0 )  {
			unpincode(ppddpin) ;
		    } else {
			/* request buffer for writing  */
			pp->buffer = (char *) xmalloc(PIOBUFFSZ, 2,
							      pinned_heap);
			pp->buf_size = PIOBUFFSZ ;
			if (pp->buffer == NULL) { /* did we get it        */
			  ppclose_dev(pp) ;     /* shut the device down */
			  unpincode(ppddpin) ;  /* no so unpin code     */
			  retcd = ENOMEM ;
			} else {
			    pp->flags.errrecov = 0 ;
			    pp->flags.ppopen = 1;
			    if (mode & FWRITE) {
				pp->flags.ppwrite= 1;
			    } else {
				pp->flags.ppread = 1;
			    }
			    if(ext == LPDIAGMOD ) {
				pp->flags.ppwrite= 1;
				pp->flags.ppdiagmod = TRUE;
			    }
			    pp->prt.ccc = pp->prt.ind;
			    pp->prt.mcc = 0;
			    pp->prt.mlc = 0;
			}
		    }
		}
	    } /* end processing device not yet open case */
	}
    }
    unlockl((lock_t *)&ppglobal_lock);
    DDHKWD1(HKWD_DD_PPDD, DD_EXIT_OPEN, retcd, dev);
    return(retcd);
} /* end ppopen */
