static char sccsid[] = "@(#)04	1.9  src/bos/usr/ccs/lib/libc/fgetpos.c, libcio, bos411, 9428A410j 1/12/93 11:14:00";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: fgetpos 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>

/*                                                                    
 * FUNCTION: Stores the current value of the file position indicator for the
 *           stream pointed to by stream in the object pointed to by pos.
 *
 * PARAMETERS: FILE *stream - stream to be searched
 *
 *	       fpos_t  *pos      - current value of file position
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      zero if successful
 *	      non-zero if not successful
 */

int	
fgetpos(FILE *stream, fpos_t *pos)
{
	if (stream != NULL && (*pos = ftell(stream)) >= 0) 
		return(0);
	/**********
	   ftell will set the correct errno
	**********/
	return(-1);
}
