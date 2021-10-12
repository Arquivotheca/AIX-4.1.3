static char sccsid[] = "@(#)82	1.4  src/bos/usr/ccs/lib/libc/rindex.c, libcstr, bos411, 9428A410j 8/2/91 16:03:10";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: rindex
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>

#ifdef rindex
#   undef rindex
#endif /* rindex */

/*
 *
 * FUNCTION: Returns a pointer to the last occurrence of c, converted
 *	     to a char, in the string pointed to by s.  A NULL pointer
 *	     is returned if the character does not occur in the string. 
 *	     The terminating null character is considered to be part of
 *	     the string. 
 *
 * NOTES:    rindex is included for compatibility with BSD and is not part
 *	     of the ANSI C Library.
 *
 * PARAMETERS: 
 *	     char *s - string to be searched
 *	     int  c  - character to be found
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the last occurrence of
 *	     character c in string s; NULL if c is not found in s.
 */
/*LINTLIBRARY*/


char *
rindex(const char *s, int c)
{
	return( strrchr( s, c ) );
}
