static char sccsid[] = "@(#)03	1.3  src/bos/usr/bin/dosdir/dread.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:58:56";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dread 
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

#include "pcdos.h"
#include "doserrno.h"

/*
 *      DREAD returns # bytes read or -1 on error
 */

dread(file, buffer, length)
int length;
register FCB *file;
byte *buffer;
{
register int absseek, lread, rspace, readtot;
int read();

TRACE(("dread(file = %x, buffer = %x, length = %d)\n", file, buffer, length));
        readtot = 0;
 	if (file->magic != FCBMAGIC)
	{       doserrno = DE_INVAL;
		return (-1);
	}
	if ((file->oflag & 3) == DO_WRONLY)
	{       doserrno = DE_ACCES;
		return(-1);
	}

	{
		int i, ret;
		extern int dos_pid;

		for(i=0;i<LOCK_RETRY_COUNT;i++){
			if((ret = dlock(dos_pid, file,
			    file->offset, length, L_TEST)) == -1){
				/* hit a lock, try again */
				sleep(LOCK_RETRY_TIME);
			} else break;
		}
		if(ret == -1){
			doserrno = DE_DEADLK;
			return(-1);
		}
	}
	
			   /* determine if read will cross cluster boundary */
	rspace = file->clustsize - file->clseek;
	if ((file->size - file->offset) < length)
		length = file->size - file->offset;
	_DFsetlock(file->disk);
	while (length >= rspace)
	{       absseek = file->seek + file->clseek;
		if (lseek(file->disk->fd,absseek,0) < 0) /* find current clust*/
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
		lread = _devio(read,file->disk->fd,buffer+readtot,rspace);
		if (lread < 0)
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
                readtot += lread;
		length -= lread;
		file->offset += lread;
		rspace = file->clustsize;
		file->nowcluster = getnextcluster(file->disk,file->nowcluster);
		file->seek = file->disk->data + file->clustsize
				 * ((file->nowcluster)-2);
                file->clseek = 0;
	}
	if (length>0)
	{       absseek = file->seek+file->clseek;
		if (lseek(file->disk->fd,absseek,0) < 0) /* find current real disk posn*/
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
		lread = _devio(read,file->disk->fd,buffer+readtot,length);
		if (lread < 0)
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
                readtot += lread;
		file->offset += lread;
		file->clseek += lread;
	}
	_DFunlock(file->disk);
	return(readtot);
}
