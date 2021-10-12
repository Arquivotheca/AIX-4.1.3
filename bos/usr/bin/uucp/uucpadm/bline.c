static char sccsid[] = "@(#)51	1.3  src/bos/usr/bin/uucp/uucpadm/bline.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:29";
/* 
 * COMPONENT_NAME: CMDUUCP bline.c
 * 
 * FUNCTIONS: bline 
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
#include <curses.h>
#include "defs.h"

extern WINDOW *active;

int bline(numy,numx)
int numy;
int numx;
{
	int i, istop;
	if (i = COLS - REQ / 2 < 0)
		i = 0;
	istop = i + REQ - 1;
	for(i = numx;i <= istop;i++)  {
	if (mvwaddch(active,numy,i,' ') == ERR)
		derror(EX_SC,"Error on write to screen!");
			}
	if (wmove(active,numy,numx) == ERR)
		derror(EX_SC,"Error on move to screen!");
	return(EX_OK);
}
