static char sccsid[] = "@(#)07        1.6  src/bos/kernel/io/errdd_pg.c, syserrlg, bos411, 9428A410j 10/11/93 11:24:23"; 

/*
 * COMPONENT_NAME: SYSERRLG   /dev/error pseudo-device driver
 *
 * FUNCTIONS: erropen, errclose, errioctl, errwrite
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This file contains error logging device driver routines that are
 * not required to be pinned in memory.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/low.h>
#include <sys/malloc.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/trchkid.h>
#include <sys/sleep.h>
#include <sys/erec.h>
#include "errdd.h"

int errclose(dev_t mmdev);
int errwrite(dev_t mmdev,struct uio *uiop);
int errioctl(dev_t mmdev,int cmd,int arg);
int erropen(dev_t mmdev,int oflag);

extern void reset_state();
extern void errput();
extern void err_wakeup_dd();

extern struct errc errc;  
extern struct errstat errstat;

#ifdef _POWER_MP
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
extern Simple_lock errdd_lock;
#endif /* _POWER_MP */

/*
 * NAME:      erropen
 *
 * FUNCTION:  devsw[] open routine for /dev/error
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : Yes
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : Yes
 *    Serialization:       : none needed
 *
 * DATA STRUCTURES:
 *    errc.e_state     (static errdd control structure)
 *
 * RETURNS:
 *   EBUSY    /dev/error already open
 *   ENODEV   mmdev is not /dev/error or /dev/errorctl
 *   ENXIO    error log buffer not allocated
 *   EACESS   permission to read /dev/error denied
 *
 * /dev/error is an exclusive use device.
 * Only the opening process (and its children) can have this file open.
 */

int
erropen(dev_t mmdev,int oflag)
{
	int mdev;
	int rv_error;

	rv_error = 0;
	mdev = minor(mmdev);

	switch(mdev) {
	case MDEV_ERROR:
		if(errc.e_start == NULL) {
			rv_error = ENXIO;
			break;
		}
		if(oflag & FREAD) {
			if(ras_privchk() < 0) {
				rv_error = EACCES;
				break;
			}
			if(errc.e_state & ST_RDOPEN) {
				rv_error = EBUSY;
				break;
			}
			/* When the demon is started, clear the discard
			   count.  It was decided not to clear this
			   immediately when the errdemon is stopped
			   so that it can be viewed from a dump or
			   from crash until the demon is started again.
			*/
			if(errc.e_discard_count > 0) {
				errc.e_discard_count = 0;
				errc.e_errid_1st_discarded = 0;
				errc.e_errid_last_discarded = 0;
			}
			rv_error = 0;
			set_state(ST_RDOPEN);
		}
		errstat.es_opencount++;
		break;
	case MDEV_ERRORCTL:
		break;
	default:
		rv_error = ENODEV;
		
	}
	if(rv_error)
		ERR_TRCHK(ERROPEN,rv_error);
	return(rv_error);
}



/*
 * NAME:      errclose
 *
 * FUNCTION:  devsw[] close routine for /dev/error
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : Yes
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : Yes
 *    Serialization:       : none needed
 *
 * DATA STRUCTURES:
 *    errc.e_state     (static errdd control structure)
 *
 * RETURNS:
 *   ENODEV   mmdev is not /dev/error or /dev/errorctl
 */

int
errclose(dev_t mmdev)
{
	int mdev;
	int rv_error;
	int intpri;

	rv_error = 0;

 	mdev = minor(mmdev);

	switch(mdev) {
	case MDEV_ERROR:
		reset_state(-1);
		break;
	case MDEV_ERRORCTL:
		break;
	default:
		rv_error = ENODEV;
	}
	if(rv_error)
		ERR_TRCHK(ERRCLOSE,rv_error);
	return(rv_error);
}

/*
 * NAME:      errioctl
 *
 * FUNCTION:  devsw[] ioctl routine for /dev/errorctl
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : Yes
 *    May Page Fault       : Yes
 *    Serialization:       : none needed
 *
 * DATA STRUCTURES:
 *    errc.e_state
 *
 * RETURNS:
 *   ENODEV   mmdev is not /dev/error or /dev/errorctl
 *   EINVAL   mmdev is /dev/error
 *   EINVAL   mmdev is /dev/error and 'cmd' is invalid
 *   ENXIO    ERRIOC_STOP when /dev/error is not open for read
 *   EFAULT   invalid user address specified in copyout() call
 *
 * /dev/error does not have any ioctl commands.
 * A second node, /dev/errorctl, with separate filesystem privileges,
 *   allows control over the errdemon through /dev/error.
 *
 * The ioctl commands for /dev/errorctl are:
 *   ERRIOC_STOP     Send EOF to process reading from /dev/error.
 *                   This is used by errstop.
 *   ERRIOC_STAT     Get the error logging device driver status 	
 *					 information contained in errstat.
 *   ERRIOC_BUFSET   Set the error log memory buffer size. 
 *		     This reallocates the error log buffer, copies
 *		     over the existing data to the new buffer, and
 *		     resets the buffer pointers.	
 *   ERRIOC_BUFSTAT  Get the error log memory buffer size.	
 */

int
errioctl(dev_t mmdev,int cmd,int arg)
/* mmdev: contains MDEV_ERRCTL
 * cmd: ioctl command
 * arg: sometimes-used additional argument
 */
{
	int mdev;
	int rv_error;
	int intpri;
	int bufsize;
	int inptr_offset, outptr_offset, staleptr_offset;
	char *start, *old_start;

	rv_error = 0;

 	mdev = minor(mmdev);

	if (mdev != MDEV_ERRORCTL)
		rv_error = EINVAL;
	
	else {

		switch(cmd) {
		case ERRIOC_STOP:
			if(!(errc.e_state & ST_RDOPEN)) {
				rv_error = ENXIO;
				break;
			}
			err_wakeup_dd(ST_STOP);
			/* When the errdemon is stopped, clear the over-
			   write count.  It was decided not to clear this
			   immediately when the demon is started so that
			   this value could be viewed from a dump or through
			   crash until the deactivated demon is started 
			   again. */
			errc.e_over_write_count = 0;
			break;
		case ERRIOC_STAT:
			if(copyout(&errstat,arg,sizeof(errstat)))
				rv_error = EFAULT;
			break;
		case ERRIOC_BUFSET:
		        if ((start = (char *)xmalloc(arg,2,pinned_heap)) == 0)
		         	rv_error = ENOMEM;
			else
				{
#ifdef _POWER_MP
				intpri = disable_lock(INTMAX, &errdd_lock);
#else
				intpri = i_disable(INTMAX);
#endif /* _POWER_MP */
				old_start = errc.e_start;
				bcopy(errc.e_start,start,errc.e_size);

				/* get the pointer offsets */
				inptr_offset = errc.e_inptr - errc.e_start;
				outptr_offset = errc.e_outptr - errc.e_start;
				staleptr_offset = errc.e_stale_data_ptr - errc.e_start;

				/* reset the buffer pointers */
			  	errc.e_start = start;
				errc.e_end = start + arg;
				errc.e_size = arg;
				errc.e_inptr = start + inptr_offset;
				errc.e_outptr = start + outptr_offset;
				errc.e_stale_data_ptr = start + staleptr_offset;
#ifdef _POWER_MP
				unlock_enable(intpri, &errdd_lock);
#else
				i_enable(intpri);
#endif /* _POWER_MP */

				/* free the old buffer space */
				xmfree(old_start, pinned_heap);
				}
			break;
	        case ERRIOC_BUFSTAT:
			bufsize = errc.e_size;
			if (copyout(&bufsize,arg,sizeof(bufsize)))
				rv_error = EFAULT;
			break;
		default:
			rv_error = EINVAL;
			break;
		}
	}
	if(rv_error)
		ERR_TRCHKL(ERRIOCTL,rv_error,mmdev << 16 | cmd);
	return(rv_error);
}


/*
 * NAME:      errwrite
 *
 * FUNCTION:  devsw[] write routine for /dev/error
 *
 * EXECUTION ENVIRONMENT:
 *    Preemptable          : Yes
 *    Runs on Fixed Stack  : No
 *    May Page Fault       : Yes
 *    Serialization:       : none needed
 *
 * DATA STRUCTURES:
 *    errc.e_state, errc.e_inptr
 *
 * RETURNS:
 *   EFAULT   (from uiomove) is target address is invalid
 *        0   Success
 */

int
errwrite(dev_t mmdev,struct uio *uiop)
{
	int mdev;
	int ucount, data_len;
	struct erec *e;
	struct errvec *evp;		/* Address vector for errput() */
	int rv_error;

	if (dump_started)
		return;
	rv_error = 0;
	ucount = uiop->uio_resid;
	if(ucount > ERR_REC_MAX_SIZE)
		ucount = ERR_REC_MAX_SIZE;
	
	/* Allocate pinned buffer space.
	 * This must include the address vector.
	 */
	data_len = ucount+sizeof(struct erec0);
	if (!(e = (struct erec *)xmalloc(data_len+sizeof(struct errvec),BPWSHIFT,pinned_heap))) {
		rv_error = EAGAIN;
	}
	else {
		/* Point to address vector. */
		evp = (struct errvec *)((int)e+data_len);

		/* Get the data */
		if((rv_error = uiomove(&e->erec_rec,ucount,UIO_WRITE,uiop))==0) {
			/* Setup the error record header and address vector */
			e->erec_len = data_len;
			e->erec_rec_len = ucount;
			e->erec_symp_len = 0;
			evp->error_id = e->erec_rec.error_id;
			evp->nel = 1;
			evp->v[0].p = e;
			evp->v[0].len = data_len;

			/* Put the data in the buffer. */
			errput(evp,data_len);
		}
		xmfree(e,pinned_heap);
	}
	if(rv_error)
		ERR_TRCHK(ERRWRITE,rv_error);
	return(rv_error);
}


