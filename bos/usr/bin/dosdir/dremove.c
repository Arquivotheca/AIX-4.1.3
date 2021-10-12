static char sccsid[] = "@(#)04	1.4  src/bos/usr/bin/dosdir/dremove.c, cmdpcdos, bos411, 9428A410j 8/2/91 13:58:33";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dremove 
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
 *      DREMOVE returns 0  else -1 for error
 */

dremove(disk, pathname)
register DCB *disk;
register byte *pathname;
{
register int d;
int startcluster;	/* first cluster in file */

	if (disk->magic != DCBMAGIC)                       /* valid device? */
	{       doserrno = DE_INVAL;
		return(-1);
	}
	_DFsetlock(disk);
	if ((d = _DFlocate(pathname,disk)) == 0)            /* file exists? */
	{	_DFunlock(disk);
		return(-1);
	}
	if (d == PC_ROOTDIR)                           /* can't remove root */
	{       doserrno = DE_ROOT;
		_DFunlock(disk);
		return(-1);
	}
	if (dir->df_attr & FIL_HDD)
	{       doserrno = DE_ISDIR;
		_DFunlock(disk);
		return(-1);
	}
	if (dir->df_attr & FIL_RO)
	{       doserrno = DE_ACCES;
		_DFunlock(disk);
		return(-1);
	}
	startcluster = dir->df_lcl | (dir->df_hcl << 8);
	if (_DFreclaimspace(disk, startcluster)  !=  0)
	{       _DFunlock(disk);
		return(-1);
	}
	dir->df_use = DIR_MT;
	lseek(disk->fd,d,0);
	_devio1(disk, d, disk->protect,disk->fd,dir,1);
	_DFputfat(disk);
	_DFunlock(disk);
	return(0);
}
