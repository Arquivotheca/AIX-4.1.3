static char sccsid[] = "@(#)53	1.3  src/bos/usr/bin/uucp/uucpadm/bmenu.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:40";
/* 
 * COMPONENT_NAME: CMDUUCP bmenu.c
 * 
 * FUNCTIONS: bmenu 
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
#include "defs.h"

extern int cury[MAXENT+1], curx[MAXENT+1];

int bline();

int bmenu(menu)
struct menu_shell  *menu;
{
	int i;
	for(i=0;i<=menu->counts;i++) {
		bline(cury[i],curx[i]);
		bline(cury[i]+1,curx[i]);
		if (i == menu->counts)
		bline(cury[i]+2,curx[i]);
		}
	return(EX_OK);
}
