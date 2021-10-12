static char sccsid[] = "@(#)79	1.1  src/bos/usr/ccs/lib/libmi/emalloc.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:08";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** emalloc.c 2.1, last change 11/14/90
 **/


#include <pse/clib.h>

char	*
emalloc (size)
	unsigned	size;
{
	char	* ptr;

	if (!(ptr = malloc(size)))
		err_set_str("out of memory");
	return ptr;
}
