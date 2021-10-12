static char sccsid[] = "@(#)33	1.5  src/bos/usr/ccs/lib/libc/tell.c, libcio, bos411, 9428A410j 6/16/90 01:20:06";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: tell 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/
/*
 *
 * return offset in file.
 */

extern long lseek();

long
tell(f)
int	f;
{
	return(lseek(f, 0L, 1));
}
