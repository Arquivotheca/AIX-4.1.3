static char sccsid[] = "@(#)53	1.1  src/bos/usr/ccs/lib/libc/doprnt64.c, libcprnt, bos411, 9428A410j 10/3/93 13:18:29";
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: _doprnt64
 *
 * ORIGINS: 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <varargs.h>		/* va_list */
#include <stdio.h>		/* FILE */

extern int _doprnt();

/*
 * NAME: _doprnt64
 *
 * DESCRIPTION: All a user to explictly invoke _doprnt() to
 *              interpret long double as 64-bits.  This allows
 *              mixed-mode applications to be created.
 */

int
_doprnt64(const char *format, va_list oargs, FILE *iop)
  {
  return _doprnt(format, oargs, iop);
  }

