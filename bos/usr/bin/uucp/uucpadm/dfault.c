static char sccsid[] = "@(#)64	1.4  src/bos/usr/bin/uucp/uucpadm/dfault.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:30";
/* 
 * COMPONENT_NAME: CMDUUCP dfault.c
 * 
 * FUNCTIONS: dfault 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include <string.h>
#include "defs.h"

extern char entries[MAXENT][MAXLINE];

int dfault(menu,dvalue)
struct menu_shell  *menu;
char *dvalue;
{
	register int i;
	entries[0][0] = '\0';
	for (i=1;i < menu->counts ;i++) {
		(void) strcpy(entries[i],dvalue);
		}
	return (EX_OK);
}
