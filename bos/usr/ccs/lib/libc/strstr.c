static char sccsid[] = "@(#)60	1.12  src/bos/usr/ccs/lib/libc/strstr.c, libcstr, bos411, 9428A410j 6/16/90 01:32:37";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>

/*
 * FUNCTION: Locates the first occurrence in the string pointed to by s1
 *	     of the sequence of characters, excluding the terminating null
 *	     character, in the string pointed to by s2.  If s2 points to
 *	     a zero length string, the value of s1 is returned.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS:
 *	     char *s1 - string to search
 *	     char *s2 - string to find
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer; the location of the found
 *	     string, or NULL if the string was not found, or the value of
 *	     s1 if s2 points to a zero length string.
 */
/*LINTLIBRARY*/


char	*
strstr(const char *s1, const char *s2)
{
  char *p, *q, *r;

  if( *s2 == '\0' )
    return((char *)s1);

  for(q=(char *)s1; *q != '\0'; q++) 
  {
    for(r=q, p=(char *)s2; *r != '\0' && *p != '\0'; r++, p++)
    {
      if( *p != *r )
	break;
    }
    if( *p == '\0' )
      break;
  }
  if (*q)
      return(q);
  else
      return(NULL);
}
