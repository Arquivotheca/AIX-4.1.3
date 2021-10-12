static char sccsid[] = "@(#)56  1.5  src/bos/usr/lpp/jls/dictutil/kusize.c, cmdKJI, bos411, 9428A410j 8/27/91 12:26:42";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 */


#include <stdio.h>
#include <sys/ioctl.h>

int kusize(row, col)
	int  *row, *col;
{
	struct	winsize win;

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
