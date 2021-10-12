static char sccsid[] = "@(#)17	1.3  src/bos/usr/bin/dosdir/putfat.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:48";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _DFputfat 
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
#include "tables.h"

_DFputfat(disk)
DCB *disk;
{
register int i, j, fatsize;
register byte *fatptr, *p;
byte *calloc();
int write();

	if (disk->magic != DCBMAGIC)
	{       doserrno = DE_INVAL;
		return(-1);
	}
	if (disk->changed)
	{       fatsize = disk->bpb.pb_fatsiz * disk->bpb.pb_secsiz;
		fatptr = calloc(1,fatsize);  /* zero space to be partially written */
		TRACE(("PUTFAT: disk %s updated\n", disk->dev_name));
		if (disk->fatentsiz == 16)
			for (i=0,j=0; j<disk->ccount; j++)
			{
				fatptr[i++] =  disk->fat_ptr[j].cluster & 0x00ff;
				fatptr[i++] = (disk->fat_ptr[j].cluster & 0xff00) >> 8 ;
			}
		else
			for (i=0,j=0; j<disk->ccount; j+=2)
			{
				fatptr[i++] =  disk->fat_ptr[j].cluster   & 0x00ff;
				fatptr[i++] = (disk->fat_ptr[j].cluster   & 0x0f00) >> 8 |
					      (disk->fat_ptr[j+1].cluster & 0x000f) << 4 ;
				fatptr[i++] = (disk->fat_ptr[j+1].cluster & 0x0ff0) >> 4 ;
			}
		if (lseek(disk->fd,disk->zero+512,0) < 0)
		{       doserrno = errno;
			return(-1);
		}
		for (i=0; i< disk->bpb.pb_fatcnt; i++)
		{       if (_devio(disk->protect,disk->fd,fatptr,fatsize) < 0)
			{       doserrno = errno;
				return(-1);
			}
		}
		free(fatptr);
	}
	disk->changed = 0;
	return( 0 );
}
