static char sccsid[] = "@(#)93	1.16  src/bos/kernel/io/POWER/mem.c, sysio, bos411, 9428A410j 1/31/94 10:05:37";
/*
 * COMPONENT_NAME: (SYSIO) Input/Output
 *
 * FUNCTIONS: mmread, mmwrite, mmrw
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * GLOBAL NOTES:
 *
 * mem , kmem and null special file support
 *	minor device 0 is virtual memory using supplied segvalue (dev/mem)
 *	minor device 1 is process virtual memory as seen by kernel (dev/kmem)  
 *	minor device 2 is dev/null
 *	minor device 3 is dev/zero
 *
 ****************************************************************************/

#include <stdio.h>
#include <macros.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/uio.h>
#include <sys/malloc.h>
     
#define SMALL_ZBUF 128

/*******************************************************************************
*
* NAME: mmread
*
* FUNCTION:   
*		Read from a given memory device.
*		0 dev is /dev/mem
*		1 dev is /dev/kmem
*		2 dev is /dev/null
*		3 dev is /dev/zero
*
*
* EXECUTION ENVIRONMENT:
*
*
* RETURN VALUE DESCRIPTION:
*		Returns a zero if a write to null.  All else is
*		void unless there is an error in which case a -1
*		is returned.
*
* EXTERNAL PROCEDURES CALLED: minor
*
******************************************************************************/

int
mmread(dev, uiop, chan, segval)
dev_t dev;			/* minor dev num */
struct uio *uiop;		/* user i/o pointer */
int chan;			/* Not used */
ulong segval;			/* Segval passed in from readx()
				   or default 0 for read().  */
{
	register struct iovec *iov = uiop->uio_iov;
	int rc;
	static char zbuf[SMALL_ZBUF] = {
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0',
		'\0','\0','\0','\0','\0','\0','\0','\0'
		};

	/*
	 * We only need the minor number from now
	 * on, so get rid of the major.
	 */

	dev = minor(dev);

	/*
	 * If this a read from /dev/null return 
	 * NULL (EOF).
	 */

	if (dev == 2)
		return(NULL);

	/*
	 * If this a read from /dev/zero act as if this
	 * is an infinite source of zeroes.
	 */
	if (dev == 3) { 
		rc = 0;
		while(uiop->uio_resid && rc == 0)
			rc = uiomove(zbuf,
				     min(sizeof zbuf, uiop->uio_resid),
				     UIO_READ, uiop);
		/* offset is meaningless for /dev/zero */
		uiop->uio_offset = 0;
		return(rc);
	}

	/*
	 * From this point we assume we are working
	 * with /dev/kmem or /dev/mem.
	 */

	return(mmrw(dev, 'r', segval, uiop, iov));
}

/*******************************************************************************
*
* NAME: mmwrite
*
* FUNCTION:   
*		Write to a given memory device.
*		0 dev is /dev/mem
*		1 dev is /dev/kmem
*		2 dev is /dev/null
*		3 dev is /dev/zero
*
*
* EXECUTION ENVIRONMENT:
*
*
* RETURN VALUE DESCRIPTION:
*		Returns a zero if a write to null.  All else is
*		void unless there is an error in which case a -1
*		is returned.
*
* EXTERNAL PROCEDURES CALLED: minor
*
******************************************************************************/

int
mmwrite(dev, uiop, chan, segval)
dev_t dev;			/* minor dev num */
struct uio *uiop;		/* user i/o pointer */
int chan;			/* not used */
ulong segval;			/* Segval passed in from writex()
				   or default 0 for write().  */
{
	register unsigned n;
	register struct iovec *iov = uiop->uio_iov;

	/*
	 * We only need the minor number from now
	 * on, so get rid of the major.
	 */

	dev = minor(dev);

	/*
	 * If this a write to /dev/null or /dev/zero fool the
	 * user structure and the world that something
	 * really has been done then return  NULL (EOF).
	 */

	if (dev == 2 || dev == 3) { 
		n = iov->iov_len;
		/* offset is meaningless for /dev/zero and /dev/null */
		uiop->uio_offset = 0;
		uiop->uio_resid = 0;
		iov->iov_base += n;
		iov->iov_len = 0;
		return(NULL);
	}

	/*
	 * From this point we assume we are working
	 * with /dev/kmem or /dev/mem.
	 */

	return(mmrw(dev, 'w', (int)segval, uiop, iov));
}

/*******************************************************************************
*
* NAME: mmrw
*
* FUNCTION:
*		To read/write from/to memory a
*		page at a time.
*
* EXECUTION ENVIRONMENT:
* Process only.
*
* RETURN VALUE DESCRIPTION:
*		0 unless there is an error in which case a ENXIO, EINVAL
*		 or EFAULT is returned.
*
* EXTERNAL PROCEDURES CALLED: vmatt, vmdet, uiomove, min 
*
******************************************************************************/

int
mmrw(dev, dir, segval, uiop, iov)
dev_t dev;		/* Minor dev num */
char dir;		/* Read vs write flag */
ulong segval;		/* Handle desired to attach */
struct uio *uiop;
struct iovec *iov;
{
	register unsigned n;
	register int c, rc=0;
	ulong handle,temp_handle;

	/*
	 * If the device is not /dev/mem or /dev/kmem
	 * we don't know what it is so go no further.
	 */

	if (dev != 0 && dev != 1) {
		return(ENXIO);
	}

	if (dev == 1) {  /* the dev/kmem special file is being used */
		if (segval == TRUE)  {
	/* make offset relative to topmost 2 gigabyte address space */
			handle = (uiop->uio_offset) | 0x80000000;
			segval = 0;
		}
		else {
			if (segval == 0)
				handle = uiop->uio_offset;
			else
				return(EINVAL);
		}
	}
	else 	/* the dev/mem special file is being used */
	/*
	 * If a virtual memory handle was given, load it into a 
	 * suitable segment register. Otherwise
	 * use the segment implied by the offset.
	 */
	{
		if (segval) {
			handle = vm_att(segval, (int)uiop->uio_offset);
			temp_handle=handle;
		}
		else
			handle =  uiop->uio_offset;
	}
	/*
	 * Common code for both kmem and mem support
	 * Read or write, a page at a time.
	 */

	while ( uiop->uio_resid ) {

		c = uiop->uio_offset & (PAGESIZE - 1);
		n = min(uiop->uio_resid, PAGESIZE - c);


		if (dir == 'w') {
 			if (uiomove(handle, n, UIO_WRITE, uiop))
			{
				rc = EFAULT;
				break;
			}
		}
		else {
 			if (uiomove(handle, n, UIO_READ, uiop))
			{
				rc = ENXIO;
				break;
			}
		}
		handle += n;

	} /* end of while */

	/*
	 *  Clean-up and return to caller.Use unchanged offset to detach.
	 */
	if (segval) {
		vm_det( temp_handle );
	}
	return(rc);
}
