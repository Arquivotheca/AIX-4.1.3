static char sccsid[] = "@(#)09	1.5  src/bos/usr/bin/dosdir/dwrite.c, cmdpcdos, bos411, 9428A410j 1/25/91 01:42:27";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dwrite 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include "pcdos.h"
#include "doserrno.h"

static int getnextclusterX(register FCB *file);

/*
 *      DWRITE returns # bytes written or -1 on error
 */

dwrite(file, buffer, length)
    int             length;
    register FCB   *file;
    byte           *buffer;
{
    register int lwrite, rspace;
    int             i, ret;
    extern int      dos_pid;
    char *bufsave = buffer;
    int curcluster;

    TRACE(("dwrite(file = %x, buffer = %x, length = %d)\n", file, buffer,
	   length));

    if (file->magic != FCBMAGIC) {
	doserrno = DE_INVAL;
	return (-1);
    }
    if ((file->oflag & 3) == DO_RDONLY) {
	doserrno = DE_ACCES;
	return (-1);
    }
    _DFsetlock(file->disk);
    if (file->oflag & DO_APPEND) {
	if (dlseek(file, 0, 2) < 0) {
	    _DFunlock(file->disk);
	    return (-1);
	}
    }
    if (!file->nowcluster && !_DFgetnewcluster(file, file->disk)) {
	_DFunlock(file->disk);
	return (-1);
    }

    for (i = 0; i < LOCK_RETRY_COUNT; i++) {
	if ((ret = dlock(dos_pid, file,
			 file->offset, length, L_TEST)) == -1) {
	    /* hit a lock, try again */
	    sleep(LOCK_RETRY_TIME);
	} else
	    break;
    }
    if (ret == -1) {
	doserrno = DE_DEADLK;
	return (-1);
    }

    while (length) {
	int extra = length;

	/* seek to current clust */
	if (lseek(file->disk->fd, file->seek + file->clseek, 0) < 0) {
	    doserrno = errno;
	    _DFunlock(file->disk);
	    return (-1);
	}

	/*
	 * if we need more space, we get the next cluster and if it is
	 * the one past the current cluster, we just increase the
	 * amount we are going to write.  The idea is to make big
	 * writes if we are luck and get consecutive clusters.
	 */
	curcluster = file->nowcluster;
	if ((rspace = file->clustsize - file->clseek) > extra)
	    rspace = extra;

	if (!(extra -= rspace))		/* fits in this cluster */
	    file->clseek += rspace;
	else
	    while (extra &&
		   (file->nowcluster = getnextclusterX(file)) &&
		   file->nowcluster == curcluster + 1) {
		curcluster = file->nowcluster;
		if (extra > file->clustsize)
		    file->clseek = file->clustsize;
		else
		    file->clseek = extra;
		extra -= file->clseek;
		rspace += file->clseek;
	    }

	while (rspace) {
	    if ((lwrite = _devio(file->disk->protect, file->disk->fd, buffer,
				 rspace)) < 0) {
		if (errno == EINTR)
		    continue;
		doserrno = errno;
		_DFunlock(file->disk);
		return (-1);
	    }

	    length -= lwrite;
	    file->offset += lwrite;
	    file->changed |= 1;
	    if (file->offset > file->size)
		file->size = file->offset;
	    buffer += lwrite;
	    rspace -= lwrite;
	}

	if (file->nowcluster == 0) {
	    _DFunlock(file->disk);
	    return (buffer - bufsave);
	}
	file->seek = file->disk->data +
	    (file->clustsize * (file->nowcluster - 2));
    }
    _DFunlock(file->disk);
    return (buffer - bufsave);
}

static int getnextclusterX(register FCB *file)
{
    int rc;

    if ((rc = getnextcluster(file->disk, file->nowcluster)) >= PC_EOF)
	rc = _DFgetnewcluster(file, file->disk);
    return rc;
}
