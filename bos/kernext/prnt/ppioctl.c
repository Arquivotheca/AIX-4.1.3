#ifndef lint
static char sccsid[] = "@(#)55 1.6 src/bos/kernext/prnt/ppioctl.c, sysxprnt, bos411, 9428A410j 5/3/94 17:42:58";
#endif
/*
 *   COMPONENT_NAME: (SYSXPRNT) Printer Parallel Port Device Driver
 *
 *   FUNCTIONS: ppioctl
 *		ppioctl_read
 *		ppioctl_write
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
 *   NAME: ppioctl
 *
 *   FUNCTIONS:
 *
 *           This code is used to control the environment of the printer and to
 *           supply data on request to the user. To do this a set of sub-commands
 *           have been set up the following is a list of these commands.
 *
 * IOCINFO   Returns a structure defined in sys/devinfo.h which describes
 *           the device.
 *
 * LPQUERY   Returns values which can not be set with ioctl calls.
 *           The field names of the structure are:
 *
 *             status   device status
 *             tadapt   adapter type
 *
 * LPGPRMS   Returns page length, width, and indent, the time-out interval,
 *           and the values of numerous flags. The field names of the structure
 *           are:
 *              ind        contains page indent.
 *              col        contains columns per line.
 *              line       contains lines per page.
 *              v_timeout  contains time-out interval.
 *              modes      contains all the flag values.
 *              flags:
 *
 * LPSPRMS   Sets values of variables and flags listed above under LPGPRMS
 *
 * LPDIAG    Provides direct access to hardware for diagnostic
 *           purposes. Data is read and written using the lpdiag structure
 *           found in sys/pprio.h The structure's fields are:
 *           cmd      - command to perform
 *           value    - data read or to be written, in char format 0 - 255
 *
 *           If the "command" argument is bad ppioctl returns with the
 *           error value of EINVAL. If the address of the structure  of
 *           information  (parameter 3)  is  bad ppioctl returns with the error
 *           value of EFAULT.
 *
 * INPUT
 *         device number
 *         command
 *         address of data
 *         open flags
 *
 * EXCEPTIONS -RETURNS
 *
 *         If no error is found then return with a ZERO return code.
 *         If a signal is received while waiting for a resource then return an
 *              EINTR to the caller.
 *         If the device has not been configured then return an ENXIO to the
 *              caller.
 *         If an attempt is made to set any value or do any diagnostic type
 *              commands an error of EWRPROTECT will be returned.
 *         If the page length is less than one, return EINVAL.
 *         If the column width is less than one, return EINVAL.
 *         If the indent is not garter than or equal to 0 and less than the column
 *              width then return EINVAL.
 *         If the timer value is not grater than zero then return EINVAL.
 *         If data can not be copied to or from user then return a EFAULT to
 *              the caller.
 */


#include <sys/ppdd.h>
#include <errno.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/lpio.h>
#include <sys/devinfo.h>

extern ppglobal_lock ;

#ifdef DEBUG
extern struct ppdbinfo ppdd_db;
extern ulong pbuffer[];
#endif


ppioctl(
dev_t dev,        /* device number                            */
int cmd,          /* command argument ( which ioctl desired ) */
int addr,         /* address of the user space                */
ulong devflag,    /* open flags                               */
int can,          /* channel number (not used)                */
int ext)          /* extinsion (not used)                     */
{
	struct pp *pp;
	int retcd;

	DDHKWD5(HKWD_DD_PPDD, DD_ENTRY_IOCTL, 0, dev, cmd, devflag, 0, 0);
	(void)lockl((lock_t *)&ppglobal_lock, LOCK_SHORT) ;
	pp = ppget(minor(dev)) ;
	unlockl((lock_t *)&ppglobal_lock) ;
	if( pp == NULL ) {
		DDHKWD1(HKWD_DD_PPDD, DD_EXIT_IOCTL, ENXIO, dev);
		return (ENXIO) ;
	}
	switch (cmd) {
		case IOCINFO:          /* get device info                      */
		case LPQUERY:          /* get lpquery struct                   */
		case LPRGET:           /* get current page info                */
		case LPRGTOV:          /* get lptimer structure                */
		case LPRMODG:          /* get lprmod structure                 */
			retcd = ppioctl_read(pp, cmd, addr ) ;
			break;

		case LPRSTOV:          /* set lptimer structure                */
		case LPRSET:           /* set page info                        */
		case LPRMODS:          /* set modes                            */
		case LPDIAG:           /* do diagnostics                       */
			if( !(devflag & FWRITE) ) {
				retcd = EWRPROTECT;
				break;
			}
			(void) lockl((lock_t *)&pp->write_lock, LOCK_SHORT);
			retcd = ppioctl_write(pp, cmd, addr ) ;
			unlockl((lock_t *)&pp->write_lock);
			break ;
#ifdef DEBUG
		case LPRMODS+5:
		        retcd = copyout(&ppdd_db, addr, sizeof(ppdd_db)) ;
			break;
		case LPRMODS+6:
		        retcd = copyout(pbuffer, addr, 4096*4) ;
			break;
#endif
		default:
			retcd = EINVAL;
			break;
	}
	DDHKWD1(HKWD_DD_PPDD, DD_EXIT_IOCTL, retcd, dev);
	return(retcd);
}


/*
 *   NAME: ppioctl_read
 *
 *   FUNCTION:
 *
 * This code is used to control the environment of the printer and to
 * supply data on request to the user. To do this, a set of sub-commands
 * have been set up. The following is a list of these commands.
 *
 * IOCINFO   Returns a structure defined in sys/devinfo.h which describes
 *           the device.
 *
 * LPQUERY   Returns values which can not be set with ioctl calls.
 *           The field names of the structure are:
 *           status   device status
 *           tadapt   adapter type
 *
 * LPGPRMS   Returns page length, width, and indent, the time-out interval,
 *           and the values of numerous flags. The field names of the structure
 *           are:
 *              ind       contains page indent.
 *              col       contains columns per line.
 *              line      contains lines per page.
 *              v_timeout contains time-out interval.
 *              modes     contains all the flag values.
 *                 Note: default for all flags is off.
 *
 * INPUT
 *    printer structure pointer
 *    command
 *    address of data
 *
 * EXCEPTIONS -RETURNS
 */

int ppioctl_read(
   struct pp *pp,
   int cmd,
   int addr)
{
	int retcd = 0 ;                 /* return code for caller  */
	struct devinfo devinfo;
	struct lptimer lptimer;
	struct lprmod lprmod;
	struct lprio lprio;
	struct lpquery lpquery;
	int  statusreg;
	int status;
	switch (cmd) {

	case IOCINFO:                   /* get device info */
		devinfo.devtype = DD_LP ;           /* device type */
		devinfo.devsubtype = DS_PP ;        /* device type */
		devinfo.flags = 0;
		/* copy devinfo from the kernel stack to user space */
		retcd = copyout(&devinfo, addr, sizeof(devinfo)) ;
	     break ;

	case LPQUERY:                   /* get lpquery struct */
		/* read Status register */
		statusreg = ppreadstat(pp) ;
		if ( statusreg == -1 )
			return (ENXIO) ;
		switch (pp->interface) {
			case PPIBM_PC :
				status = ppibmpc((char)statusreg) ;
				break;

			case PPCONVERGED :
				status = ppconverged((char)statusreg, pp) ;
				break ;
		}
		/* check for printer time out       */
		if( pp->flags.pptimeout )
			status |= LPST_TOUT;
		/* check for printer power off      */
		if (pppower(pp, 0) == 0)
			status |= LPST_OFF;
		lpquery.status = status;
		/* bytes in receive buffer, (it does not exist) */
		lpquery.reccnt = 0;
		/* set the adapter type                         */
		lpquery.tadapt = pp->adapter ;
		/* copy lpquery struct from kernel to user space */
		retcd = copyout(&lpquery, addr, sizeof(lpquery)) ;
		break;

	case LPRGET:
		lprio.ind = pp->prt.ind;            /* set indent level */
		lprio.col = pp->prt.col;            /* set # columns */
		lprio.line = pp->prt.line;          /* set # lines/page */
		/* copy lprio struct from kernel to user space */
		retcd = copyout(&lprio, addr, sizeof(lprio)) ;
		break;

	case LPRGTOV:                   /* get lptimer structure */
		lptimer.v_timout = pp->v_timout; /* set timeout value   */
		/* copy lptimer struct from kernel to user space */
		retcd = copyout(&lptimer, addr, sizeof(lptimer)) ;
		break;

	case LPRMODG:                   /* get lprmod structure */
		lprmod.modes = pp->ppmodes;     /* set printer modes*/
		/* copy lprmod struct from kernel to user space */
		retcd = copyout(&lprmod, addr, sizeof(lprmod)) ;
		break;
	}
	if (retcd)
		return (EFAULT) ;
	return(retcd);
}


/*
 *   NAME: ppioctl_write
 *
 *   FUNCTION:
 *
 * This code is used to control the environment of the printer and to supply
 * data on request to the user. To do this a set of sub-commands have
 * been set up. The following is a list of these commands.
 *
 *   LPGPRMS   Sets page length, width, and indent, the time-out interval,
 *             and the values of numerous flags. The field names of the structure
 *             are:
 *
 *                 ind     contains page indent.
 *                 col     contains columns per line.
 *                 line    contains lines per page.
 *                 v_timout
 *                 modes   contains all the flag values.
 *
 *            LPSPRMS Sets values of variables and flags listed above under
 *                    LPGPRMS
 *
 *            LPDIAG   Provides   direct   access   to  hardware  for
 *                     diagnostic purposes. Data is read and written
 *                     using  the  lpdiag structure
 *                     found in sys/pprio.h The structure's fields are:
 *
 *                       cmd      - command to perform
 *                       value    - data read or to be written, in char format
 *                                  0 - 255
 *
 *                    If  the "command" argument is bad ioctl returns with the
 *                    error value of EINVAL. If the address of the structure
 *                    of information (parameter 3) is bad ioctl returns with
 *                    the error value of EFAULT.
 *
 * INPUT
 *     pointer to printer structure
 *     command
 *     address of data
 *
 * EXCEPTIONS -RETURNS
 *     If the input data can not be copied return EFAULT
 *     If the timer value is less than one return EINVAL
 *     If the column width is less than one return EINVAL
 *     If the indent is not less than the column setting then return EINVAL
 *     If the page length is less than one return EINVAL
 */

int ppioctl_write(
struct pp * pp,
int cmd,
int addr) {
	int retcd = 0;                         /* return code to be passed out*/
	struct lptimer lptimer;
	struct lprmod lprmod;
	struct lprio lprio;
	struct lpdiag lpdiag;

	switch (cmd){

	case LPRSTOV:        /* get lptimer structure */
		/* copy lptimer struct from user to kernel space */
		retcd = copyin(addr, &lptimer, sizeof(lptimer)) ;
		if( retcd )
			return (EFAULT) ;
		if(lptimer.v_timout < 1)
				return (EINVAL) ;
		pp->v_timout = lptimer.v_timout;
	      break;

	case LPRSET:
		/* copy lprio struct from user to kernel space */
		retcd = copyin(addr, &lprio, sizeof(lprio)) ;
		if( retcd )
			return (EFAULT); /* bad addr */
		if(lprio.col>0 && lprio.ind >= 0 && lprio.ind < lprio.col) {
			pp->prt.ind = lprio.ind;   /* set indent level*/
			pp->prt.col = lprio.col;   /* set # columns */
			pp->prt.line = lprio.line; /* set # lines/page */
		 } else
			return (EINVAL) ;
	      break;

	 case LPRMODS:                   /* set lprmod structure */
	       /* copy lprmod struct from user to kernel space */
	       retcd = copyin(addr, &lprmod, sizeof(lprmod)) ;
	       if( retcd )
			return (EFAULT) ; /* bad addr */
	       pp->ppmodes = lprmod.modes;
	     break;

	case LPDIAG:
	       /* copy lpdiag struct from user to kernel space */
	       retcd = copyin(addr, &lpdiag, sizeof(lpdiag)) ;
	       if( retcd )
			return (EFAULT); /* bad addr */
	       switch( lpdiag.cmd ) {
			case LP_R_STAT: /* read status port     */
				retcd = ppreadstat(pp) ;
				if (retcd == -1 )
					return (ENXIO) ;
				lpdiag.value = retcd ;
				break;
			case LP_R_CNTL: /* read control port    */
				retcd = ppreadctrl(pp) ;
				if (retcd == -1 )
					return (ENXIO) ;
				lpdiag.value = retcd ;
				break;
			case LP_W_CNTL: /* write control port   */
				retcd = ppwritectrl(pp, (char)lpdiag.value);
				if (retcd == -1 )
					return (ENXIO) ;
				break;
			case LP_R_DATA: /* read data port       */
				retcd = ppreaddata(pp) ;
				if (retcd == -1 )
					return (ENXIO) ;
				lpdiag.value = retcd ;
				break;
			case LP_W_DATA: /* write data port      */
				retcd = ppwritedata(pp, (char)lpdiag.value);
				if (retcd == -1 )
					return (ENXIO) ;
				break;
			case LP_WATCHINT: /* watch for interrupt*/
				pp->flags.ppintocc = 0  ;
				break;
			case LP_DIDINTOCC: /* did interrupt occur */
				if(pp->flags.ppintocc)
					lpdiag.value = LP_INTDIDOCC ;
				else
					lpdiag.value = LP_INTNOTOCC ;
				break;
			default:
				return (EINVAL);
		}
			/* copy lpdiag struct from kernel to user space */
		retcd = copyout(&lpdiag, addr, sizeof(lpdiag)) ;
		if( retcd )
		      return (EFAULT); /* bad addr */
	}
	return retcd ;
}
