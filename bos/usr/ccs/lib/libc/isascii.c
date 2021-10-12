static char sccsid[] = "@(#)35	1.2  src/bos/usr/ccs/lib/libc/isascii.c, libcchr, bos411, 9428A410j 5/16/91 13:16:34";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: isascii
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */
/*
 *
 * FUNCTION: isascii
 *	    
 *
 * PARAMETERS: int c:  character to be converted to ascii
 *
 *
 * RETURN VALUE: converted ascii character
 *
 *
 */

/*
 *
 * This routine is not included in the ANSI C standard, but is included
 * for compatibility with prior releases
 */

int isascii(int c)
{
	c &= ~0177;
	return(!c);
}
