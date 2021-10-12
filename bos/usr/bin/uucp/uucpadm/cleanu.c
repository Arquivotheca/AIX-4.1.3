static char sccsid[] = "@(#)57	1.3  src/bos/usr/bin/uucp/uucpadm/cleanu.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:55";
/* 
 * COMPONENT_NAME: CMDUUCP cleanu.c
 * 
 * FUNCTIONS: cleanu 
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
#include "defs.h"

int memclr();
int dfault();
int bmenu();

int cleanu(menu,mem,dvalue)
struct menu_shell *menu;
char *mem[MEMSIZE];
char *dvalue;
{
/* clear the menu, memory pointers & initialize defaults. */
(void) bmenu(menu);
(void) memclr(mem);
(void) dfault(menu,dvalue);
return(EX_OK);
}
