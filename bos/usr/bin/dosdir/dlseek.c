static char sccsid[] = "@(#)67	1.3  src/bos/usr/bin/dosdir/dlseek.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:42";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dlseek 
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
 *      DLSEEK returns file position    or   -1 on error
 */

dlseek(file, where, how)
int where, how;
FCB *file;
{
int offset;

 	if (file->magic != FCBMAGIC)
	{       doserrno = DE_INVAL;
		return (-1);
	}
	switch (how)
	{       case 0: offset = where;               /* new "file pointer" */
			break;
		case 1: offset = file->offset + where;
			break;
		case 2: offset = file->size + where;
			break;
		default: doserrno = DE_INVAL;
			return (-1);
	}
	if (offset < 0)
	{       doserrno = DE_INVAL;
		return(-1);
	}
	file->offset = offset;
	offset = file->clustsize;
	file->clseek = file->offset % file->clustsize;
	file->nowcluster = file->startcluster;
	_DFsetlock(file->disk);
	while (offset <= file->offset)
	{       if (file->size > offset)
		{       offset += file->clustsize;
			file->nowcluster = getnextcluster(file->disk, file->nowcluster);
		}
		else
		{       file->size += file->clustsize;
			offset += file->clustsize;
			if (_DFgetnewcluster(file, file->disk) == 0)
			{       _DFunlock(file->disk);
				return(-1);
			}
			file->size += file->clustsize;
		}
	}
	_DFunlock(file->disk);
	file->seek = file->disk->data + file->clustsize * (file->nowcluster - 2);
	return (file->offset);
}
