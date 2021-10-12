static char sccsid[] = "@(#)15	1.15  src/bos/usr/ccs/lib/libc/ctermid.c, libcio, bos411, 9428A410j 10/27/93 15:59:59";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: ctermid
 *
 *   ORIGINS: 3,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*LINTLIBRARY*/

#include <stdio.h>

extern char *strcpy();

static char *res = "/dev/tty";

char *
ctermid(char *s)
{
	if (s == NULL) 
		return(res);
	return(strcpy(s, res));
}
