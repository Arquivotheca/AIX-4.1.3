static char sccsid[] = "@(#)64	1.3  src/bos/usr/bin/dosdir/dfirst.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:31";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dfirst 
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

dfirst(disk, dirpath, mode, block)
int mode;
DCB *disk;
byte *dirpath;
SRCHBLK *block;
{
byte *localdir;
byte *buffer, *malloc();
register int d, tnxtcl, count, seek;
int read();

	if (disk->magic != DCBMAGIC)
	{       doserrno = DE_INVAL;
		return(-1);
	}
	_DFsetlock(disk);
	if ((d = _DFlocate(dirpath,disk)) == 0)         /* can't find name! */
	{	_DFunlock(disk);
		return(-1);
	}
	if (d == PC_ROOTDIR)                             /* is it the root? */
	{       count = (disk->data - disk->root) / 32;
		localdir = malloc(disk->data - disk->root);
		lseek(disk->fd,disk->root,0);
		_devio(read,disk->fd,localdir,disk->data - disk->root);
	}
	else
	{       if ((dir->df_attr & 0x10) == 0)         /* not a directory! */
		{       doserrno = DE_NOTDIR;
			_DFunlock(disk);
			return(-1);
		}
		tnxtcl = dir->df_lcl|(dir->df_hcl<<8);
		if (tnxtcl)
		{       seek = disk->data + (disk->clsize * (tnxtcl-2));
			count = 0;
			while(tnxtcl <= disk->ccount )
			{       tnxtcl = getnextcluster(disk,tnxtcl);
				count += disk->clsize/32;
			}
			localdir = malloc(count*32);
			tnxtcl = dir->df_lcl|(dir->df_hcl<<8);
			buffer = localdir;
			while(tnxtcl <= disk->ccount)
			{       lseek(disk->fd,seek,0);
				_devio(read,disk->fd,buffer,disk->clsize);
				buffer += disk->clsize;
				tnxtcl = getnextcluster(disk,tnxtcl);
				seek = disk->data + (disk->clsize * (tnxtcl-2));
			}
		}
		else
		{       seek = disk->root;
			count = (disk->data - disk->root) / 32;
			localdir = malloc(disk->data-disk->root);
			lseek(disk->fd,disk->root,0);
			_devio(read,disk->fd,localdir,disk->data-disk->root);
		}
	}
	block->mode = (long)localdir;                /* address of storage  */
	block->seek = (long)localdir;                /* address of next dir */
	block->count = count;                /* number of dirs left to read */
	block->tnxtcl = DCBMAGIC;
	_DFunlock(disk);
	return(0);
}
