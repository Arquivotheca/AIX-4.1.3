static char sccsid[] = "@(#)70	1.4  src/bos/usr/bin/dosdir/dnext.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:56:55";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dnext 
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
 *      DNEXT returns pointer to next active dir element.
 *               NULL if error or at end of directory.
 */
byte *dnext(block)
register SRCHBLK *block;
{
register pc_dirent *ndir;

	if (block->tnxtcl != DCBMAGIC)
	{       doserrno = DE_INVAL;
		return(NULL);
	}
	while (block->count--)
	{       ndir = (pc_dirent *)block->seek;
		block->seek += 32;
		if (ndir->df_use != DIR_MT)
			if (ndir->df_use != DIR_NIL)
				return((byte *)ndir);
	}
	free((char *)block->mode);
	return(NULL);
}
