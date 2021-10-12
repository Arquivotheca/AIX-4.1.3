static char sccsid[] = "@(#)59	1.3  src/bos/usr/bin/dosdir/dchdir.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:11";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dchdir 
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
 *      DCHDIR returns 0 else -1 for fail
 */

dchdir(disk, dirpath)
register DCB *disk;
byte *dirpath;
{
register int d, tnxtcl;

	if (disk->magic != DCBMAGIC)
	{       doserrno = DE_INVAL;
		return(-1);
	}
	_DFsetlock(disk);
	if ((d=_DFlocate(dirpath,disk)) == 0)  /* cant find name! */
	{       doserrno = DE_NOENT;
		_DFunlock(disk);
		return(-1);
	}
	if (d != PC_ROOTDIR)
	{	if ((dir->df_attr & 0x10) == 0)           /* not a directory! */
		{       doserrno = DE_NOTDIR;
			_DFunlock(disk);
			return(-1);
		}
		tnxtcl = dir->df_lcl|(dir->df_hcl<<8);
		if (tnxtcl)
		{       current.dir[disk->home].pathname = d;
			current.dir[disk->home].start = disk->data + disk->clsize * (tnxtcl-2);
			current.dir[disk->home].nxtcluster = getnextcluster(disk,tnxtcl);
			_DFunlock(disk);
			return(0);
		}
	}
	_DFunlock(disk);
	current.dir[disk->home].pathname = PC_ROOTDIR;
	current.dir[disk->home].start = disk->root;
	current.dir[disk->home].nxtcluster = PC_EOF;
	return(0);
}
