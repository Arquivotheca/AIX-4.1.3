static char sccsid[] = "@(#)97	1.4  src/bos/usr/bin/mh/uip/whatnow.c, cmdmh, bos411, 9428A410j 6/26/90 17:13:20";
/* 
 * COMPONENT_NAME: CMDMH whatnow.c
 * 
 * FUNCTIONS: Mwhatnow 
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

/* whatnow.c - the MH WhatNow? shell */

#include "mh.h"

main (argc, argv)
int	argc;
char  **argv;
{
    setlocale(LC_ALL,"");
    WhatNow (argc, argv);
}
