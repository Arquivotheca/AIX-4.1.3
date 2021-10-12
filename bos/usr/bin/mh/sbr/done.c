static char sccsid[] = "@(#)50	1.3  src/bos/usr/bin/mh/sbr/done.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:54";
/* 
 * COMPONENT_NAME: CMDMH done.c
 * 
 * FUNCTIONS: done 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* done.c - terminate the program */

#include "mh.h"


void done (status)
register int     status;
{
    exit (status);
}
