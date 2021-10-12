static char sccsid[] = "@(#)99	1.7  src/bos/kernext/disk/sd/sdrdwr.c, sysxdisk, bos411, 9428A410j 3/16/94 10:14:32";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_read(), sd_write(), sd_mincnt()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sd.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/errno.h>

/*
 * NAME: sd_read
 *
 * FUNCTION: Read Entry Point for Serial DASD Subsystem Device Driver
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                           
 *
 * (RECOVERY OPERATION:)  If a failure occurs , the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	uio - structure used for data control block
 *
 * RETURNS: 	0         - successful completion
 *		Results of uphysio call, if error
 */

int	sd_read(
dev_t	devno, 
struct	uio	*uiop, 
int	chan, 
int	ext)
{
	int	rc;

	DDHKWD1(HKWD_DD_SERDASDD, DD_ENTRY_READ, 0, devno);
	rc = uphysio(uiop, B_READ, 9, devno, sd_strategy, sd_mincnt, ext);
	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_READ, rc, devno);
	return(rc);
}

/*
 * NAME: sd_write
 *
 * FUNCTION: Write Entry Point for Serial DASD Subsystem Device Driver
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                           
 *
 * (RECOVERY OPERATION:)  If a failure occurs , the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	uio - structure used for data control block
 *
 * RETURNS: 	0         - successful completion
 *		Results of uphysio call, if error
 */

sd_write(
dev_t	devno, 
struct	uio	*uiop, 
int	chan, 
int	ext)
{
	int	rc;

	DDHKWD1(HKWD_DD_SERDASDD, DD_ENTRY_WRITE, 0, devno);
	rc = uphysio(uiop, B_WRITE, 9, devno, sd_strategy, sd_mincnt, ext);
	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_WRITE, rc, devno);
	return(rc);
}

/*
 * NAME: sd_mincnt
 *
 * FUNCTION: Transfers options from RAW I/O request into buf structure, and
 * 	verifies the transfer size is within limits
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                              
 *
 * (DATA STRUCTURES:) 	buf - buf structure data control block
 *			sd_dasd_info - DASD information structure
 *
 * RETURNS: 	0         - successful completion
 */

int	sd_mincnt(
struct buf *bp, 
void *minparms)
{
	struct sd_dasd_info *dp;

	dp = sd_hash(bp->b_dev);

        if (dp == NULL) {
                return(ENXIO);
        }

	if (bp->b_bcount > dp->max_transfer)
		/*
		 * If this buf bigger than we can handle,
		 * change count to our max transfer
		 */
		bp->b_bcount = dp->max_transfer;

	bp->b_options = (int) minparms;
	return(0);
}

