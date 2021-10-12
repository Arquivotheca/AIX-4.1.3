static char sccsid[] = "@(#)27	1.2  src/bos/usr/lpp/kls/dictutil/husize.c, cmdkr, bos411, 9428A410j 11/30/93 16:34:12";
/*
 * COMPONENT_NAME:	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS:		get display size
 *
 * ORIGINS:		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <sys/ioctl.h>

int husize(row, col)
	int *row, *col;
{
        struct  winsize win;

	if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) {
		*col = win.ws_col;
		*row = win.ws_row;
	}
	else {
		printf("%s: Sorry, can't get the screen size.");
		exit(1);
	}
	return(0);
}

