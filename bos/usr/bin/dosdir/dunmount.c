static char sccsid[] = "@(#)08	1.3  src/bos/usr/bin/dosdir/dunmount.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:12";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dunmount 
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
 *   DUNMOUNT(disk)    returns 0 else -1 if error
 */
dunmount(disk)
register DCB *disk;
{
	int close();
	TRACE(("DUNMOUNT: entry %8.8x\n",disk));
	if (disk->magic != DCBMAGIC)
	{       doserrno = DE_INVAL;
		TRACE(("DUNMOUNT: ERROR EXIT\n"));
		return (-1);
	}
	_DFunlock(disk);               /* to avoid race condition where
					* devio kills itself */
	if (--disk->users <= 0)
	{       TRACE(("DUNMOUNT: prior to PUTFAT; disk-changed = %d\n",
			disk->changed));
		_DFsetlock(disk);
		_DFputfat(disk);
		_devio(close,disk->fd);
		TRACE(("DUNMOUNT: device closed\n"));
		disk->fd = 0;
		disk->magic = 0;
		release_fat(disk->fat_desc);
		_DFunlock(disk);
	}
	TRACE(("DUNMOUNT: exit use-count = %d\n",disk->users));
	disk = NULL;
	return(0);
}
