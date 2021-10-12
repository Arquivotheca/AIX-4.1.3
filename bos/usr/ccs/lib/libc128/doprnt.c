static char sccsid[] = "@(#)82	1.3  src/bos/usr/ccs/lib/libc128/doprnt.c, libc128, bos411, 9428A410j 10/3/93 18:03:48";
/*
 *   COMPONENT_NAME: LIBC128
 *
 *   FUNCTIONS: _doprnt
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <varargs.h>		/* va_list */
#include <stdio.h>		/* FILE typedef */

/*
 * Function Name: _doprnt
 * 
 * Description:  Print function for libc128.a.  It
 * simply passes it's agruments to _dorpnt128()
 * [in libc.a] which actually does the work.
 */

extern int _doprnt128();	/* located libc.a */

int
_doprnt(char *oformat, va_list oargs, FILE iop)
  {
  return _doprnt128(oformat, oargs, iop);
  }
