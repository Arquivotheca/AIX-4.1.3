static char sccsid[] = "@(#)44	1.2  src/bos/usr/ccs/lib/libc/POWER/w31stuff.c, libccnv, bos411, 9428A410j 5/13/91 16:16:24";
/*
 * COMPONENT_NAME: LIBCCNV 
 *
 * FUNCTIONS: wstrtod, watof, watoi, watol, wstrtod
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * these are wide character conversion routines from
 * AIX 3.1.  They must be included for binary compatability
 */

#include <stdlib.h>

/*
 * NAME: wstrtod
 *                                                                    
 * FUNCTION: convert an ascii string to a float/double
 *                                                                    
 * RETURNS: the converted value
 *
 */

double wstrtod( wchar_t *nptr,  wchar_t **endptr)
  {
  return wcstod(nptr, endptr);
  }

/*
 * NAME: watof
 *                                                                    
 * FUNCTION: convert an ascii string to a float/double
 *                                                                    
 * RETURNS: the converted value
 *
 */

double watof(wchar_t *nptr)
  {
  return wcstod(nptr, (wchar_t **) NULL);
  }
     
/*
 * NAME: watoi
 *                                                                    
 * FUNCTION: convert an ascii string to an integer
 *                                                                    
 * RETURNS: the converted value
 *
 */

int watoi(wchar_t *p)
  {
  return (int) wcstol(p, (wchar_t **) NULL, 10);
  }

/*
 * NAME: watol
 *                                                                    
 * FUNCTION: convert an ascii string to a long
 *                                                                    
 * RETURNS: the converted value
 *
 */

long watol(wchar_t *p)
  {
  return wcstol(p, (wchar_t **) NULL, 10);
  }

/*
 * NAME: wstrtol
 *                                                                    
 * FUNCTION: convert an ascii string to a long
 *                                                                    
 * RETURNS: the converted value
 *
 */

long wstrtol(wchar_t *str, wchar_t **ptr, int base)
  {
  return wcstol(str, ptr, base);
  }

