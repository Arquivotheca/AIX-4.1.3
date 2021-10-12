static char sccsid[] = "@(#)13	1.8  src/bos/usr/ccs/lib/libc/fsetpos.c, libcio, bos411, 9428A410j 1/12/93 11:14:45";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: fsetpos 
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
 * FUNCTION: Sets the file position indicator for the stream pointed to by
 *           stream to the value of the object pointed to by pos.
 *
 * PARAMETERS: FILE *stream      - stream to be searched
 *	       fpos_t  *pos      - current value of file position
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      zero if successful
 *	      non-zero if not successful
 *	      errno is set on error to indicate the error
 */

int	
fsetpos(FILE *stream, const fpos_t *pos)
{
	if (fseek(stream, *pos, 0) == 0)
		return(0);
	/**********
	  fseek will set the correct errno
	**********/
	return(-1);
	
}
