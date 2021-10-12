#ifndef lint
static char sccsid[] = "@(#)50 1.3 src/bos/kernext/prnt/ppconfig.c, sysxprnt, bos411, 9428A410j 5/3/94 17:42:50";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Printer Device Driver
 *
 *   FUNCTIONS: config
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
 * NAME: ppconfig (AIX entry point)
 *
 * FUNCTION:
 *      This entry point is used to configure or delete a device. A structure
 *      is allocated and initialized for each device. If the request is to
 *      delete a device then the structure is removed from the system.
 *
 *      As each device is added to the system a linklist is used to keep track
 *      of them. The storage needed for each structure is requested from the
 *      pined heap. Using this method a device can be added or taken away
 *      at any time with out affecting the system operation.
 *
 *      If this is the first printer to be configured the device switch table
 *      is to be setup.
 *
 *      When the device is configured a test will be made of the device to
 *      see what type it is. This information will be saved in the printer
 *      structure. The types of devices that can be handled by this device
 *      driver will be PIO, the current system (LPAD_R1), NS16C553 National
 *      chip (LPAD_NIO).
 *
 *      <input>
 *
 *      device Major-minor number
 *      command
 *        CFG_INIT to set up the printer for use
 *        CFG_TERM to remove the device from operating status
 *      uio structure pointer
 *         the structure will contain device dependent information
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure will execute in the process environment
 *
 *  EXCEPTIONS - RETURNS:
 *
 *       If there no error is detected then ZERO is returned.
 *       If a signal is received while waiting for exclusive use of the overall
 *          system then return an EINTR to the caller.
 *       If the device has been added before, an EBUSY will be returned to the
 *          caller.
 *       If there is a hardware error detected, return an ENXIO to the caller.
 *       If memory can not be reserved from the pined memory space, return
 *          with an ENOMEM error.
 *       If the device switch table can not be set up then return whatever
 *          is returned from dev switch add.
 *       If CFG_TERM is requested, delete device, and the device is open then
 *          return an EBUSY.
 */

#include <sys/ppdd.h>
#include <errno.h>

/* Extern Global Variables */

extern  int ppglobal_lock;
extern  int nodev(void) ;
extern  struct pp *ppheader;

int ppconfig(
dev_t   devno,          /* major-minor device number    */
int     cmd,            /* command                      */
struct uio *uiop)       /* uio structure containing device dependent data */
{
	struct pp  *pp ;
	int retcd = 0;
	static struct   devsw   ppdsw;         /* the static devswadd/del    */

	/* Lock Global Lock, waits for lock or returns early on signal       */
	(void)lockl((lock_t *)&ppglobal_lock, (int)LOCK_SHORT) ;
	pp = ppget(minor(devno));              /* get pp struct if it exists */
	switch(cmd) {                          /* switch on command          */
		case CFG_INIT:                 /* device is being added      */
			if( pp != (struct pp *)NULL ) {
				if( pp->flags.ppopen )
					retcd  =  EBUSY; /* Device still open */
				else  /* reinit pp from dds */
					ppalloc(devno, uiop, pp);
			} else {
				/* No pp structure allocated for device      */
				if (ppheader == NULL) {
					/* first dev to be init'd            */
					ppdsw.d_open = ppopen ;
					ppdsw.d_close = ppclose;
					ppdsw.d_read = ppread;
					ppdsw.d_write = ppwrite;
					ppdsw.d_ioctl = ppioctl;
					ppdsw.d_strategy = (int (*)(
							    struct buf *))nodev;
					ppdsw.d_ttys = NULL;
					ppdsw.d_select = (int (*)(dev_t, ulong, ulong, int))nodev;
					ppdsw.d_config = ppconfig;
					ppdsw.d_dump = (int (*)(dev_t, struct uio *, int, int, int, int))nodev;
					ppdsw.d_mpx = (int (*)(dev_t, int, char *))nodev;
					ppdsw.d_revoke = (int (*)(dev_t, int, int))nodev;
					ppdsw.d_dsdptr = NULL;
					ppdsw.d_selptr = NULL;
#ifdef _POWER_MP
					ppdsw.d_opts = DEV_MPSAFE;
#else
					ppdsw.d_opts = 0;
#endif /* _POWER_MP */
					retcd = devswadd(devno,&ppdsw) ;
				}  /* if no devs inited yet                  */
				if (retcd == 0) {
					pp = ppalloc(devno, uiop, (struct pp *)NULL) ;
					if (pp == (struct pp *)NULL) {
						retcd = ENOMEM ;
					} else {
						retcd = ppinit_dev(pp) ;
						if (retcd) {
							ppfree (pp) ;
						}
					}
				}
			}
			break;
		case CFG_TERM:     /* device is being deleted */
			if( pp == NULL )
				retcd = ENXIO ;
			else {
				if( pp->flags.ppopen )
					/* Device still open. Fail init. */
					retcd  =  EBUSY;
				else {
					ppfree(pp);   /* free pp structure */
					if (ppheader == NULL)
						retcd = devswdel(devno);
				}
			 }
			 break;  /* end device deletion */
		    case (CFG_QVPD) :
		    default:
			retcd = EINVAL ;
			break;
		}
	/* Unlock Global Lock Variable */
	unlockl((lock_t *)&ppglobal_lock);
	return(retcd);
} /* end ppconfig */
