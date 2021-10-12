#ifndef lint
static char sccsid[] = "@(#)53 1.4 src/bos/kernext/prnt/ppread.c, sysxprnt, bos411, 9428A410j 5/3/94 17:43:00";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Port Printer Device Driver
 *
 *   FUNCTIONS: ppread
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
 *   NAME: ppread
 *
 *   FUNCTIONS:
 *
 *      This routine is to read data on the National chip only. If
 *      this device is not one of them then reading is not allowed. If reading
 *      is done then a way to terminate the transaction must be possible from
 *      both ends and the count of the data left to read should be returned
 *      to the caller.
 *
 * INPUT
 *      device number
 *      uio structure
 *
 * EXCEPTIONS -RETURNS
 *      At this time only EINVAL will be returned.
 *      If no error is found then return with a ZERO return code.
 *      If a signal is received while waiting for a resource then return an
 *          EINTR to the caller
 *      If the device has not been configured then return an ENXIO to the
 *          caller.
 *      If the read is for PIO port then EINVAL will be returned to the caller.
 *      If the printer is not powered on at the start of the write then try
 *          for ten seconds and if it will not come up by that time return to
 *          the caller with a zero return code and the timeout flag set.
 *      If Power is lost after the data has started going to the device then
 *          the error return code ENXIO will be returned to the caller.
 *      If the device stops sending data before the count is zero then a zero
 *          return code will be sent to the caller and the count will be set to
 *          the amount of data received. The time out flag will be set so if the
 *          caller request a lpquery using IOCTL then they will know that the
 *          device timed out. The time out will be determined by the device. For
 *          the National chip it is 5us.
 */


#include <sys/ppdd.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <errno.h>
extern ppglobal_lock ;

ppread(
dev_t dev,                      /* major and minor device number */
struct uio *uio,                /* uio structure pointer          */
int chl,                        /* channel number (not used)      */
int ext)                        /* extinsion (not used)          */
{
	struct pp *pp ;         /* printer structure pointer                    */

	DDHKWD5(HKWD_DD_PPDD, DD_ENTRY_READ, 0, dev, uio->uio_resid,
	    (int)uio->uio_iovcnt, (off_t)uio->uio_offset, (int)uio->uio_fmode);
	(void) lockl((lock_t *)&ppglobal_lock, LOCK_SHORT) ;
	pp = ppget(minor(dev)) ;        /* request pointer to printer struct    */
	unlockl((lock_t *)&ppglobal_lock) ;
	if( pp == NULL )
		return (ENXIO) ;
	return (EINVAL) ;
}
