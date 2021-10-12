static char sccsid[] = "@(#)04	1.2  src/bos/kernext/aio/queue.c, sysxaio, bos411, 9428A410j 10/14/93 15:51:59";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: find_queue
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "aio_private.h"

#define MINOR_MASK ((1<<3) - 1)
#define MAJOR_MASK ((1<<4) - 1)
#define QT_INDEX(dev) ((MINOR_MASK & minor(dev))+((MAJOR_MASK & major(dev))<<3))
#define SLOW_MASK (1<<7)
#define QT_MISC (QTABSIZ-1)
#define QT_SOCKET (QTABSIZ-2)
#if QTABSIZ != (SLOW_MASK+(MAJOR_MASK<<3)+MINOR_MASK+3)
#error "bad qtabsiz"
#endif

/* find_queue -- pick a queue that matches the fp passed in
 *
 * can't fail
 */
queue *
find_queue(struct file *fp)
{
	struct stat statbuf;
	int index;
	uint status;

	if (fp_fstat(fp, &statbuf, sizeof statbuf, SYS_ADSPACE)) {
		DPRINTF(("find_queue: stat failed on %x\n", (ulong)fp));
		return &qtab[QT_MISC];
	}
	/*
	 * If it's a device, construct the index from st_rdev.
	 * Set the slow bit if there's a select entry point in
	 * the device driver.
	 *
	 * If it's a file, construct the index from st_dev.
	 * Set the slow bit if it's a fifo or pipe.
	 *
	 * If it's a socket, it gets a queue of its own (and is
	 * considered slow).
	 */
	if (fp->f_type == DTYPE_GNODE) {
		index = QT_INDEX(statbuf.st_rdev);
		if (devswqry (statbuf.st_rdev, &status, NULL) ||
		    (status == DSW_UNDEFINED) ||
		    (status & DSW_SELECT))
			index |= SLOW_MASK;
	} else if (fp->f_type == DTYPE_VNODE) {
		index = QT_INDEX(statbuf.st_dev);
		if (S_ISFIFO(statbuf.st_mode))
			index |= SLOW_MASK;
	} else if (fp->f_type == DTYPE_SOCKET)
		index = QT_SOCKET;

	DBGPRINTF(DBG_FIND_QUEUE, ("find_queue: %d\n", index));
	ASSERT(0 <= index && index < QTABSIZ);
	return &qtab[index];
}
