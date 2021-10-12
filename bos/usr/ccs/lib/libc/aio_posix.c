static char sccsid[] = "@(#)77	1.1  src/bos/usr/ccs/lib/libc/aio_posix.c, libcaio, bos411, 9428A410j 5/27/91 17:05:54";
/*
 * COMPONENT_NAME: (LIBCAIO) Standard C Library Asynchronous I/O Functions
 *
 * FUNCTIONS: aio_read, aio_write, aio_suspend, aio_cancel, lio_listio,
 *	      aio_dump
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <errno.h>
#include "aio_interface.h"

static int aio_rdwr(int fildes, struct aiocb *aiocbp, int cmd);

int
aio_read(int fildes, struct aiocb *aiocbp)
{
	return aio_rdwr(fildes, aiocbp, QREADREQ);
}

int
aio_write(int fildes, struct aiocb *aiocbp)
{
	return aio_rdwr(fildes, aiocbp, QWRITEREQ);
}

/* aio_rdwr -- code common to aio_read and aio_write
 *
 * confirm that there is an aiocb
 * call the underlying kernel service
 */
static
int
aio_rdwr(int fildes, struct aiocb *aiocbp, int cmd)
{
	if (!aiocbp) {
		errno = EFAULT;
		return -1;
	}
	if (errno = kaio_rdwr(cmd, fildes, aiocbp, POSIX_AIO))
		return -1;

	return 0;
}

/*
 * iosuspend -- block waiting on completion of one of a group of aios
 */

/* XXX we should test for already completed ios here before calling
   the kernel routine */
int
aio_suspend(int cnt, struct aiocb *aiocbpa[])
{
	return iosuspend(cnt, aiocbpa);
}

int
aio_cancel(int fildes, struct aiocb *aiocbp)
{
	return acancel(fildes, aiocbp);
}

/* XXX we should be doing the EIO testing here rather than in the kernel */
int
lio_listio(int cmd, struct liocb *list[], int nent, struct sigevent *eventp)
{
	return listio(cmd, list, nent, eventp);
}

#ifdef DEBUG
int
aio_dump(struct aio_dump *adp)
{
	return kaio_dump(adp);
}
#endif /* DEBUG */
