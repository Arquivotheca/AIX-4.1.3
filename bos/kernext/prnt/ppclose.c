static char sccsid[] = "@(#)52  1.3  src/bos/kernext/prnt/ppclose.c, sysxprnt, bos411, 9428A410j 8/3/93 08:12:18";
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Printer Device Driver
 *
 *   FUNCTIONS: ppclose
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   NAME: ppclose
 *
 *   FUNCTIONS:
 *
 *       This routine is called to mark the device driver as not open and can
 *       not be used again until open is called. Only the last close of the
 *       device will actually generate a call to the driver close routine.
 *       Sends Form Feed if required.  Disables interrupts. Removes device
 *       driver from AIX interrupt handler list. All i/o is synchronous, so
 *       it is not possible to still have left over data in buffers while a
 *       close is underway. Therefore, neither draining nor flushing of buffers
 *       is necessary. Interrupts are disabled, because the interrupt handler
 *       is freed up here and interrupts should not be allowed to occur for
 *       devices for which there is no interrupt handler.
 *
 *       The pined code will be unpinned.
 *
 *       If the device driver is running in DMA mode then DMA will be cleaned
 *          up.
 *       This function will execute in process state and can page fault.
 *
 * INPUT:
 *     device number
 *
 * EXCEPTIONS - RETURNS:
 *
 *      If no error is found then return with a ZERO return code.
 *      If a signal is received while waiting for a resource then return an
 *          EINTR to the caller
 *      If the device has not been configured then return an ENXIO to the
 *          caller.
 *      If the plot flag is not set then try to do a form feed. If the printer
 *          is busy or the form feed dose not work then set the form feed needed
 *          flag.
 *      If the device has an error mark the device as closed and return the
 *          error back to the caller.
 */

#include <sys/ppdd.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/malloc.h>
#include <errno.h>
#include <sys/lpio.h>

extern int ppglobal_lock ;


ppclose(
dev_t dev,        /* major and minor device number */
int cal,          /* channel number (not used)      */
int ext)          /* extension field (not used)     */
{
	struct  pp      *pp;                    /* pointer to printer struct    */
	int     retcd = 0;                      /* data to be returned to caller*/
	int     c = 0;                          /* used for clearing clist      */
	int     stat;                           /* used for reading printer stat*/
	uint     mode;                          /* set for mode of printer      */

	DDHKWD1(HKWD_DD_PPDD, DD_ENTRY_CLOSE, 0, dev);
	/* Lock Global Lock, waits for lock or returns early on signal */
	(void)lockl((lock_t *)&ppglobal_lock, LOCK_SHORT) ;
	/* Validate device */
	pp = ppget(minor(dev)) ;
	if( pp == NULL) {                       /* no printer to be found       */
		retcd = ENXIO ;
	} else {                      /* printer pointer set to point to printer*/
		if (!(pp->flags.ppopen) ) {
			retcd = ENXIO ;         /* device was not open          */
		} else {
			if( !(pp->ppmodes & PLOT) ) {
				if( pp->prt.outq.c_cc > 0 ) {
					do {
						c = getc(&pp->prt.outq) ;
					} while( c != -1 ) ;
				}
			}
			if( pp->flags.ppiodone ) {
				if (pp->flags.pptimeout) {
					pp->flags.ppff_needed = 1 ;
				} else {     /* ff needed start timer and do */
					mode = pp->ppmodes ;
					pp->ppmodes |= RPTERR ;
					pp->flags.ppdone = FALSE;
					stat = ppsendff(pp) ;
					if( (stat == PPABORT) || (stat == -1)) {
						pp->flags.ppff_needed=1;
					}
					pp->ppmodes = mode ;
				}
				pp->flags.ppiodone = FALSE;
			}
			xmfree(pp->buffer, pinned_heap) ;
			(void)ppclose_dev(pp) ;
			pp->flags.ppopen = 0;
			pp->flags.ppread = 0;
			pp->flags.ppwrite = 0;
			(void)unpincode(ppddpin);
		}
	}
	DDHKWD1(HKWD_DD_PPDD, DD_EXIT_CLOSE, retcd, dev);
	unlockl((lock_t *)&ppglobal_lock);
	return(retcd);
} /* end ppclose */
