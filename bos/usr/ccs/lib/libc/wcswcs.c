static char sccsid[] = "@(#)29	1.2  src/bos/usr/ccs/lib/libc/wcswcs.c, libcstr, bos411, 9428A410j 1/12/93 11:20:46";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcswcs
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
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

#include <sys/types.h>

/*
 * NAME: wcswcs
 *
 * FUNCTION: Locate the first occurrence in the string pointed to by
 *  string1 of the sequence of wchar_t characters in the string pointed
 *  to by string2.
 *
 * PARAMETERS:
 *	wchar_t *string1	-	the wide character string
 *	wchar_t *string2	-	the wide character string
 *
 * RETURN VALUE DESCRIPTION: the pointer to the located string or NULL if
 *  the string is not found.  If string2 points to a string with zero length
 *  the function returns string1.
 */
wchar_t *
wcswcs(const wchar_t *string1,const wchar_t *string2)
{
	register wchar_t *p;
	register wchar_t *q;
	register wchar_t *r;

  if( *string2 == '\0' )
    return(string1);

  for(q=string1; *q != '\0'; q++) 
  {
    for(r=q, p=string2; *r != '\0' && *p != '\0'; r++, p++)
    {
      if( *p != *r )
	break;
    }
    if( *p == '\0' )
      break;
  }
  if (*q)
      return(q);
  return(0);
}
