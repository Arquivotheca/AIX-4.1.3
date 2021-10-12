static char sccsid[] = "@(#)06	1.3  src/bos/usr/bin/dosdir/drmdir.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:08";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: drmdir 
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
 *      DRMDIR returns 0  else -1 for error
 */

drmdir(disk, pathname)
register DCB *disk;
register byte *pathname;
{
register int d, dseek, count, tnxtcl, dclust;
int read();

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
	if ((dir->df_attr & FIL_HDD) == 0)
	{       doserrno = DE_NOTDIR;
		_DFunlock(disk);
		return(-1);
	}
	if (dir->df_attr & FIL_RO)
	{       doserrno = DE_ACCES;
		_DFunlock(disk);
		return(-1);
	}
	dclust = dir->df_lcl | (dir->df_hcl << 8);
	dseek = disk->data + disk->clsize * (dclust-2) + 64;
						  /* step over dot & dotdot */
	count = (disk->clsize / 32) - 2;
	tnxtcl = getnextcluster(disk,dclust);
	while (tnxtcl)                   /* search for other existing files */
	{       while (count--)
		{       if (lseek(disk->fd,dseek,0) < 0)
			{       doserrno = errno;
				_DFunlock(disk);
				return(-1);
			}
			if (_devio(read,disk->fd,dir,1) < 0)
			{       doserrno = errno;
				_DFunlock(disk);
				return(-1);
			}
			dseek += 32;
			if (dir->df_use == DIR_NIL)
			{       tnxtcl = PC_EOF;
				break;
			}
			if (dir->df_use != DIR_MT)
			{       doserrno = DE_NEMPTY;
				_DFunlock(disk);
				return(-1);
			}
		}
		if (tnxtcl >= PC_EOF)
			tnxtcl = 0;
		else
		{       count = disk->clsize / 32;
			dseek = disk->data + disk->clsize * (tnxtcl-2);
			tnxtcl = getnextcluster(disk,tnxtcl);
		}
	}
	if (_DFreclaimspace(disk, dclust)  !=  0)
	{       _DFunlock(disk);
		return(-1);
	}
	dir->df_use = DIR_MT;
	if (lseek(disk->fd,d,0) < 0)
	{       doserrno = errno;
		_DFunlock(disk);
		return(-1);
	}
	if (_devio(disk->protect,disk->fd,dir,1) < 0)
	{       doserrno = errno;
		_DFunlock(disk);
		return(-1);
	}
	disk->changed = 1;
	_DFputfat(disk);
	_DFunlock(disk);
	return(0);
}
