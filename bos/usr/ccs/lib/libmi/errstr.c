static char sccsid[] = "@(#)81	1.1  src/bos/usr/ccs/lib/libmi/errstr.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:12";
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
 ** errstr.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>

static	boolean	last_err_read = true;
static	char	err_str[128];

char *
err_get_str () {
	last_err_read = true;
	return err_str;
}

void
err_set_str (fmt, x1, x2, x3, x4, x5, x6)
char	* fmt;
int	x1, x2, x3, x4, x5, x6;
{
	if (last_err_read) {
		sprintf(err_str, fmt, x1, x2, x3, x4, x5, x6);
		last_err_read = false;
	}
}
