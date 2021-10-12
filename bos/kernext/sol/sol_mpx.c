static char sccsid[] = "@(#)44	1.4  src/bos/kernext/sol/sol_mpx.c, sysxsol, bos411, 9428A410j 6/11/91 07:31:25";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_mpx
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/priv.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/soluser.h>
#include "soldd.h"

extern struct sol_ddi	sol_ddi;

/*
 * NAME: sol_mpx
 *
 * FUNCTION: Allocates or deallocates a mpx channel.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called at open or close time from the process
 *	environment, and it can page fault.                   
 *
 * NOTES: Possible operations : 
 *	channame = NULL : de-allocates the channel (after close)
 *	channame = ptr to NULL string : allocate channel for normal open
 *	channame = 'S' : allocate channel for serialized normal open
 *	channame = 'D' : allocate a channel for a diagnostic open
 *	channame = 'F' : allocate a channel for a forced diagnostic open
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	EPERM	- device is in an open mode that does not allow a diag open
 *	EACCES	- no priviledge to open in diagnostic mode
 *	EINVAL	- invalid parameter
 *	EIO	- an error occured
 *	ENOMEM	- not enough memory to allocate open structure
 *	ENODEV	- devno is invalid
 *	EBUSY	- maximum number of opens has been exceeded
 *	EINVAL	- attempt to open /dev/ops0 special file in diag mode
 *	0	- successful completion                        
 */

int
sol_mpx(
dev_t	devno,		/* major/minor device number			*/
int	*chanp,		/* address for returning allocated chan number	*/
char	*channame)	/* pointer to special file name suffix		*/
{
	int	newchan;

	SYS_SOL_TRACE("PmxB", devno, *chanp, *channame);
	(void) lockl(&(sol_ddi.global_lock), LOCK_SHORT);

	if ((uint) minor(devno) > (uint) SOL_OPS_MINOR) {
		unlockl(&(sol_ddi.global_lock));
		SYS_SOL_TRACE("PmxE", ENODEV, *chanp, *channame);
		return ENODEV;
	}
	if (channame == NULL) { /* de-allocate the channel */
		/*
		 *  Check for invalid channel.
		 */
		if ((*chanp >= SOL_TOTAL_OPENS) || (*chanp < 0) ||
		    (sol_ddi.chan_state[*chanp] == SOL_CH_AVAIL)) {
			unlockl(&(sol_ddi.global_lock));
			SYS_SOL_TRACE("PmxE", ENODEV, *chanp, *channame);
			return ENODEV;
		}
		/*
		 *  Free this channel.
		 */
		sol_ddi.chan_state[*chanp] = SOL_CH_AVAIL;
	} else {	/* allocate a new channel */
		/*
		 *  Check for the next available channel by looking
		 *  through the array of channel states.
		 */
		newchan = 0;
		while ((sol_ddi.chan_state[newchan] != SOL_CH_AVAIL) &&
		    (newchan < SOL_TOTAL_OPENS)) {
			newchan++;
		}
		/*
		 *  If there was no channel available, we must have exceeded
		 *  the number of opens allowed.
		 */
		if (newchan >= SOL_TOTAL_OPENS) {
			unlockl(&(sol_ddi.global_lock));
			SYS_SOL_TRACE("PmxE", EBUSY, *chanp, *channame);
			return EBUSY;
		}
		
		if ((*channame == '\0') || (*channame == 'S')) { /* norm open */
			/*
			 *  This is either a normal open or a serialized
			 *  normal open.
			 */
			if (sol_ddi.num_norm_opens == SOL_MAX_OPENS) {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE", EBUSY, *chanp, *channame);
				return EBUSY;
			}
			if (minor(devno) != SOL_OPS_MINOR) {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",ENODEV,*chanp,*channame);
				return ENODEV;
			}
			if (!sol_ddi.ops_config) {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",ENODEV,*chanp,*channame);
				return ENODEV;
			}
			*chanp = newchan;
			if (*channame == 'S') { /* serialized normal open */
				sol_ddi.chan_state[newchan] = SOL_CH_SNORM;
			} else { /* non-serialized open */
				sol_ddi.chan_state[newchan] = SOL_CH_NORM;
			}
		} else { 
			/*
			 *  This is either a normal diagnostic open or a
			 *  forced diagnostic open.  Check that they are
			 *  accessing a port (not ops), have permission,
			 *  the port is not already open in diag mode,
			 *  and that the port is configured.
			 */
			if (minor(devno) == SOL_OPS_MINOR) {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",ENODEV,*chanp,*channame);
				return ENODEV;
			}
			if (privcheck(RAS_CONFIG)) {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",EACCES,*chanp,*channame);
				return EACCES;
			}
			if (sol_ddi.port_state[minor(devno)] == SOL_DIAG_MODE){
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",EPERM,*chanp,*channame);
				return EPERM;
			}
			if (sol_ddi.port_state[minor(devno)] == SOL_NO_PORT) {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",ENODEV,*chanp,*channame);
				return ENODEV;
			}
			if (*channame == 'D') { /* diagnostic open */
				/*
				 *  Normal diagnostic open fails if
				 *  /dev/ops0 is already open.
				 */
				if (sol_ddi.num_norm_opens != 0) {
					unlockl(&(sol_ddi.global_lock));
					SYS_SOL_TRACE("PmxE", EPERM, *chanp,
					    *channame);
					return EPERM;
				}
				sol_ddi.chan_state[newchan] = SOL_CH_DIAG;
				*chanp = newchan;
			} else if (*channame == 'F') { /* forced diag open */
				/*
				 *  Forced open works no matter whether
				 *  /dev/ops0 is open or not.
				 */
				sol_ddi.chan_state[newchan] = SOL_CH_FDIAG;
				*chanp = newchan;
			} else {
				unlockl(&(sol_ddi.global_lock));
				SYS_SOL_TRACE("PmxE",EINVAL,*chanp,*channame);
				return EINVAL;
			}
		}
	}
	unlockl(&(sol_ddi.global_lock));
	SYS_SOL_TRACE("PmxE", 0, *chanp, *channame);
	return 0;
}
