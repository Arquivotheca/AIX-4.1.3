static char sccsid[] = "@(#)30  1.18  src/bos/kernel/specfs/pdevsubs.c, sysspecfs, bos41J, 9513A_all 3/28/95 16:44:48";

/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: devstrat, devdump
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/user.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/device.h"
#include "sys/uio.h"
#include "sys/errno.h"
#include "sys/dev.h"
#ifdef _POWER_MP
#include <sys/processor.h>
#include <sys/low.h>
#include <sys/ppda.h>
#include <sys/proc.h>
#include "../ios/bio_locks.h"
#endif

extern devcnt;

#ifdef _POWER_MP
struct buf *g_stratlist = NULL;		/* Global 'strategylist' */

/*
 * NAME:	strat_offl
 *
 * FUNCTION:	run on the MP MASTER processor, 
 *		invoke strategy() on all the funneled buffers 
 *              from the global strategy list. 
 *
 * PARAMETERS:	
 *
 * RETURN :	void
 *			
 */
void
strat_offl()
{
	int ipri;
	struct buf *bp;

	ipri = STRATLIST_LOCK();	/* enter critical section	*/
	while (g_stratlist != NULL) {

		bp = g_stratlist ;

		/*
		 * Remove the first buffer from the global strategy list.
		 * The global  strategy list is a simple linked list 
		 * based upon the av_back list pointer to preserve the
		 * av_forw list pointer which is used by strategy().
		 */
		g_stratlist = bp->av_back;
		bp->av_back = NULL;

		/*
		 * Call the strategy() routine
		 */
		STRATLIST_UNLOCK(ipri); /* end of critical section	*/
		(void) (*devsw[major(bp->b_dev)].d_strategy) (bp);
		ipri = STRATLIST_LOCK(); /* enter critical section	*/
	}
	STRATLIST_UNLOCK(ipri);	/* end of critical section	*/

}
#endif /* _POWER_MP  */

/*
 * NAME:	devstrat (bp)
 *
 * FUNCTION:	request block data transfer to/from a block device
 *
 * PARAMETERS:	bp 	- pointer to the buf structure that 
 *			  represents the request
 *
 * RETURN :	ENODEV	- Invalid device number or no d_strategy()
 *			
 */

devstrat (bp)
struct buf *bp;
{
	int rc;
#ifdef _POWER_MP
	struct buf *bp_cur;
	int waslocked;
	int wasfunneled;
	int tmpipri, ipri;
	struct mstsave 	*curcsa;	
	extern mpc_msg_t strat_mpc_msg;
#endif /* _POWER_MP */

#ifdef _POWER_MP
	curcsa = CSA;
	bp_cur = bp;
	while ( bp_cur != NULL) { 
	  /* set B_MPSAFE_INITIAL to register the MPSAFE state of */
	  /* this buffer to the BIO module (see biodone()) */
	  if (bp_cur->b_flags & B_MPSAFE)
		bp_cur->b_flags |= B_MPSAFE_INITIAL;
	  else
		bp_cur->b_flags &= ~B_MPSAFE_INITIAL;
	  /* Clear the MPSAFE state, if the driver is a none MP-SAFE one */
	  if  (!(devsw[major (bp_cur->b_dev)].d_opts & DEV_MPSAFE))
		bp_cur->b_flags &= ~B_MPSAFE;
	  bp_cur = bp_cur->av_forw;
	}
#endif

	if (major (bp->b_dev) < devcnt)
	{
#ifdef _POWER_MP
		if (waslocked = ((curcsa->prev == NULL) &&
				 IS_LOCKED(&kernel_lock)))
			unlockl(&kernel_lock);
		tmpipri = curcsa->intpri;

		if (devsw[major (bp->b_dev)].d_opts & DEV_MPSAFE) {
		  /* MP SAFE driver,	                        */ 
		  /* direct call to the d_stategy entry point.    */
		  (void) (*devsw[major (bp->b_dev)].d_strategy) (bp);
		} else if ((curcsa->prev == NULL) &&
			   (curcsa->intpri == INTBASE)) {
		  /* At INTBASE priority level and in a thread context */
		  /* Funneling is done the same way as in DD_ENT       */
		  /* by calling switch_cpu() to let the schedule       */ 
		  /* do the migration                                  */
		  wasfunneled = switch_cpu(MP_MASTER, SET_PROCESSOR_ID);
		  (void) (*devsw[major (bp->b_dev)].d_strategy) (bp);
		  if (!wasfunneled)
		    switch_cpu(0, RESET_PROCESSOR_ID);

		} else if ( CPUID != MP_MASTER) {
	/* Not at INTBASE priority level or not in a thread context 	*/
	/* Funneling is done by linking the given buffer to global	*/ 
	/* strategy list and kick off strategy() on the MPMASTER	*/
	/* processor by calling	mpc_send().				*/
	/* note: the global strategy list is a simple linked list based */
	/* on the av_back list pointer since av_forw is used to pass a 	*/
	/* list of buffer to stategy(). (see sys/buf.h)			*/
		  ipri = STRATLIST_LOCK();
	/* insert the given buffer in the FIFO global strategy list	*/
		  if (g_stratlist != NULL) {
			bp_cur = g_stratlist;
			while (bp_cur->av_back != NULL) 
				bp_cur = bp_cur->av_back;
			bp_cur->av_back = bp;
		  } else {
			g_stratlist = bp;
		  }
		  bp->av_back = NULL;
		  STRATLIST_UNLOCK(ipri);
		  ASSERT(curcsa->intpri == INTIODONE);
		  mpc_send(MP_MASTER, strat_mpc_msg);
		  } else {
	/* Running on the MP_MASTER cpu,				*/
	/* direct call to the d_stategy entry point.			*/
		  (void) (*devsw[major (bp->b_dev)].d_strategy) (bp);
		}
		
		assert(curcsa->intpri == tmpipri);
		if (waslocked)
			lockl(&kernel_lock, LOCK_SHORT);

#else
		DD_ENT((void) ,(*devsw[major(bp->b_dev)].d_strategy) 
			(bp), IPRI_OTHER, major(bp->b_dev));
#endif /* _POWER_MP */
		rc = 0;
	}
	else
		rc = ENODEV;
	return rc;
	
}


/*
 * NAME:	devdump (devno, uiop, cmd, arg, chan, ext)
 *
 * FUNCTION:	initiate a memory dump to a device
 *
 * PARAMETERS:	devno	- Block device number
 *		uiop	- uiomove args
 *		cmd	- dump command
 *		arg	- address of parameter block
 * 		ext	- extension parameter
 *
 * RETURN :	ENODEV	- Invalid device number or no d_dump()
 *		
 */

devdump (devno, uiop, cmd, arg, chan, ext)
dev_t	devno;
struct uio *uiop;
int	cmd, arg, chan, ext;
{
	int rc;

	if (major (devno) < devcnt)
	{
		DD_ENT(rc = ,(*devsw[major(devno)].d_dump) 
		  (devno, uiop, cmd, arg, chan, ext),IPRI_OTHER,major(devno));
	}
	else
		rc = ENODEV;

	return rc;
}

/*
 * NAME:	devwrite(rdev, uiop, chan, ext)
 *
 * FUNCTION:	write to a device
 *
 * PARAMETERS:	devno	- device number of device to write to
 *		uiop	- write location information
 *		chan	- device channel number
 * 		ext	- extended write parameter
 *
 * RETURN :	ENODEV	- Invalid device number or no d_write()
 *			
 */

devwrite (
	dev_t		devno,		/* dev num of device to write	*/
	struct uio	uiop,		/* write location information	*/
	chan_t		chan,		/* device channel number	*/
	int		ext)		/* extended write parameter	*/
{
	int	rc;

	if (major(devno) < devcnt)
		rc = (*devsw[major(devno)].d_write)(devno, uiop, chan, ext);
	else
		rc = ENODEV;

	return rc;
	
}
