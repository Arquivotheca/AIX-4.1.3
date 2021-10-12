static char sccsid[] = "@(#)41	1.4  src/bos/kernext/sol/sol_config.c, sysxsol, bos411, 9428A410j 8/21/91 20:41:58";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_config
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
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/adspace.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/soluser.h>
#include "soldd.h"

extern struct sol_ddi	sol_ddi;

/*
 * NAME: sol_config
 *
 * FUNCTION: Config entry point for Serial Optical Link Device Handler
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called by the configuration process from the
 *	process environment, and it can page fault.
 *
 * NOTES: Possible operations :                             
 *	CFG_INIT : Adds driver to devsw table, allocates memory for
 *                 driver info, initializes sol_ddi structure, saves
 *                 dds structure.
 *      CFG_TERM : Terminates driver, deletes driver from devsw table,
 *                 and frees resources.
 *      CFG_QVPD : Returns Vital Product Data via *uiop.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  Result of devswadd() if error results
 *	     ENOMEM	- couldn't malloc memory
 *	     EBUSY	- already initialized or unable to terminate
 *	     EFAULT	- invalid address specified
 *	     ENODEV	- no device to terminate
 *           0		- successful completion                        
 */
int
sol_config(
dev_t		devno,		/* major/minor device number		*/
int		cmd,		/* operation (INIT, TERM, QVPD)		*/
struct uio	*uiop)		/* pointer to dds			*/

{
	int 		errnoval, minor_num;
	ulong		vpd_data;
	char		dds_type;
	struct sol_dds	dds;
	struct devsw	devsw_struct;
	volatile struct slaregs	*sla_ptr;
	caddr_t		cfg_addr;

	extern		nodev();

	minor_num = minor(devno);
	if ((uint) minor_num > (uint) SOL_OPS_MINOR) {
		return ENODEV;
	}
	errnoval = 0;
	(void) lockl(&(sol_ddi.global_lock), LOCK_SHORT);

	switch(cmd) {
	case CFG_INIT:
		/*
		 *  Copy in the dds
		 */
		if (errnoval = uiomove((caddr_t)&dds, sizeof(struct sol_dds),
		    UIO_WRITE, uiop)) {
			errnoval = EFAULT;
			break;
		}

		if (dds.dds_type == SOL_PORT_DDS) {
			if (sol_ddi.num_ports == 0) {
				/*
				 * Zero out the ddi structure, except for the
				 * global lock in the first field of the
				 * structure.
				 */
				bzero((caddr_t) ((uint)&sol_ddi+sizeof(lock_t)),
				    sizeof(struct sol_ddi) - sizeof(lock_t));

				/*
				 *  Initialize devsw struct and call devswadd
				 */
				devsw_struct.d_open = (int(*)())sol_open;
				devsw_struct.d_close = (int(*)())sol_close;
				devsw_struct.d_read = (int(*)())sol_read;
				devsw_struct.d_write = (int(*)())sol_write;
				devsw_struct.d_ioctl = (int(*)())sol_ioctl;
				devsw_struct.d_strategy=(int(*)())nodev;
				devsw_struct.d_ttys = (struct tty *)NULL;
				devsw_struct.d_select = (int(*)())sol_select;
				devsw_struct.d_config = (int(*)())sol_config;
				devsw_struct.d_print = (int(*)())nodev;
				devsw_struct.d_dump = (int(*)())nodev;
				devsw_struct.d_mpx = (int(*)())sol_mpx;
				devsw_struct.d_revoke = (int(*)())nodev;
				devsw_struct.d_dsdptr = NULL;
				devsw_struct.d_selptr = NULL;

				if (errnoval = devswadd(devno,&devsw_struct)) {
					break;
				}
			}
			/*
			 *  Check if this port is already configured.
			 */
			if (sol_ddi.port_state[minor_num] != SOL_NO_PORT) {
				errnoval = EBUSY;
				break;
			}
			sol_ddi.port_state[minor_num] = SOL_NORM_MODE;
			sol_ddi.sla_buid[minor_num] = dds.un.sol_port.buid;
			sol_ddi.num_ports++;
		} else if (dds.dds_type == SOL_OPS_DDS) {
			/*
			 *  Check to make sure at least one port has
			 *  been configured, and if the ops is already
			 *  configured.
			 */
			if (sol_ddi.num_ports == 0) {
				errnoval = ENODEV;
				break;
			}
			if (sol_ddi.ops_config) {
				errnoval = EBUSY;
				break;
			}
			sol_ddi.ops_info = dds.un.sol_ops;
			sol_ddi.devno = devno;
			sol_ddi.ops_config = TRUE;
#ifdef DEBUG 
/*
 * initialize   soltrace.next = 0
 *              soltrace.res1 = "SOL_"
 *              soltrace.res2 = "TRAC"
 *              soltrace.res3 = "ETBL"
 *              soltrace.table[0] = "!!!!"
 *
 * In the low-level debugger, one would do a find for
 *
 *              SOL_TRACETBL
 *
 * to find the starting location of the internal trace table.
 *
 */
			sol_ddi.soltrace.next = 0;
			sol_ddi.soltrace.res1 = 0x534F4C5F; 
			sol_ddi.soltrace.res2 = 0x54524143;
			sol_ddi.soltrace.res3 = 0x4554424c;
			sol_ddi.soltrace.table[0] = 0x21212121;
#endif

			break;
		}

		break;

	case CFG_TERM:
		if (minor_num == SOL_OPS_MINOR) { /* unconfigure ops */
			/*
			 *  Make sure the subsystem is really configured.
			 */
			if (!sol_ddi.ops_config) {
				errnoval = ENODEV;
				break;
			}
		} else {	/* Unconfigure a port */
			/*
			 *  Make sure the port is configured.
			 */
			if (sol_ddi.port_state[minor_num] == SOL_NO_PORT) {
				errnoval = ENODEV;
				break;
			}
			/*
			 *  If this is the last port, don't allow it to be
			 *  terminated if ops is still configured.
			 */
			if ((sol_ddi.num_ports == 1) && (sol_ddi.ops_config)) {
				errnoval = EBUSY;
				break;
			}
		}
		/*
		 *  If still open, fail CFG_TERM.
		 */
		if ((sol_ddi.num_norm_opens + sol_ddi.num_diag_opens) != 0) {
			errnoval = EBUSY;
			break;
		}
		if (minor_num == SOL_OPS_MINOR) {	/* terminate ops */
			sol_ddi.ops_config = FALSE;
		} else {				/* terminate a port */
			sol_ddi.num_ports--;
			sol_ddi.port_state[minor_num] = SOL_NO_PORT;
		}
		if ((sol_ddi.num_ports == 0) && !sol_ddi.ops_config) {
			devswdel(devno);
		}
		break;

	case CFG_QVPD:
		if (minor_num == SOL_OPS_MINOR) {
			/*
			 *  Can only query VPD from a port (adapter).
			 */
			errnoval = ENODEV;
			break;
		}
		if (sol_ddi.port_state[minor_num] == SOL_NO_PORT) {
			errnoval = ENODEV;
			break;
		}
		/*
		 *  Set up seg reg and read the config register from SLA
		 */
		ADDR_SLA(minor_num, sla_ptr);
		vpd_data = sla_ptr->config;
		UNADDR_SLA(sla_ptr);
		if (errnoval = uiomove((caddr_t)&vpd_data, 4, UIO_READ, uiop)){
			errnoval = EFAULT;
			break;
		}
		break;

	default:
		errnoval = EINVAL;
		break;
	}

	unlockl(&(sol_ddi.global_lock));
	return errnoval;
}
