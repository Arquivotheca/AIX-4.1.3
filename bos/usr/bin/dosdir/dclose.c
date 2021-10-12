static char sccsid[] = "@(#)61	1.4  src/bos/usr/bin/dosdir/dclose.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:19";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dclose 
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

#include "doserrno.h"
#include "pcdos.h"

/*
 *      DCLOSE returns zero if OK else -1
 */

dclose(file)
register FCB *file;
{
	int read();

	if (file->magic != FCBMAGIC)
	{       doserrno = DE_INVAL;
		return(-1);
	}
	if (file->changed)
	{       _DFsetlock(file->disk);
		file->disk->changed++;
		if (lseek(file->disk->fd, file->d_seek,0) < 0)
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
		if (_devio(read,file->disk->fd,dir,32) < 0)
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
		if (file->changed < 0x4a41)
			file->timestamp = time((time_t *)0);
		_DFmakedate(dir,file->timestamp);
		dir->df_lcl    =  file->startcluster       & 0xff;
		dir->df_hcl    = (file->startcluster >> 8) & 0xff;
		dir->df_siz0   =  file->size        & 0xff;
		dir->df_siz1   = (file->size >>  8) & 0xff;
		dir->df_siz2   = (file->size >> 16) & 0xff;
		dir->df_siz3   = (file->size >> 24) & 0xff;
		if (lseek(file->disk->fd, file->d_seek,0) < 0)
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
		if (_devio(file->disk->protect,file->disk->fd,dir,32) < 0)
		{       doserrno = errno;
			_DFunlock(file->disk);
			return(-1);
		}
		if (_DFputfat(file->disk) < 0)
		{       _DFunlock(file->disk);
			return(-1);
		}
		_DFunlock(file->disk);
	}

	/* Decrement use count for this file.
	 * When use count falls to zero, file
	 * can be deleted.
	 */
	release_cluster(file->disk, file->startcluster);

	return (file->magic = 0);
}
