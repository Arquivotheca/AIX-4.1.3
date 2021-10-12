static char sccsid[] = "@(#)10	1.3  src/bos/usr/bin/dosdir/getnewcl.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:20";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _DFgetnewcluster getnextcluster 
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
 *              finds first free cluster in FAT
 *              if first cluster in file, stores into directory
 *              else links found cluster into FAT chain
 *              returns cluster number else zero for disk full
 */
int
_DFgetnewcluster(file, disk)
register DCB *disk;
register FCB *file;
{
register int i,clust;
byte cl[2];

	clust = file->nowcluster;
	disk->changed = 1;                         /* need to rewrite FAT */
	for (i=2; i<disk->ccount ; i++)
		if (disk->fat_ptr[i].cluster == 0)
		{       disk->fat_ptr[i].cluster = 0xffff;   /* flag as EOF */
			file->nowcluster = i;
			file->clseek = 0;
			file->seek = disk->data + (file->clustsize * (i-2));
			if (clust)
				disk->fat_ptr[clust].cluster = i;
                        else
			{       file->startcluster = i;
				file->changed |= 1;         /* rewrite dir */
				cl[0] = i & 0xff;
				cl[1] = (i >> 8) & 0xff;
				if (lseek(file->disk->fd,file->d_seek+26,0) <0)
				{       doserrno = errno;
					return(0);
				}
				if (_devio(file->disk->protect,file->disk->fd,cl,2) <0)
				{       doserrno = errno;
					return(0);
				}
			}
			return(i);
		}
	doserrno = DE_NOSPC;
	return(0);
}

int getnextcluster(disk,num)
register DCB *disk;
register int num;
{
	return(disk->fat_ptr[num].cluster);
}
