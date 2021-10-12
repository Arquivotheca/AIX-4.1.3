static char sccsid[] = "@(#)84	1.3  src/bos/usr/bin/uucp/uucpadm/zaps.c, cmduucp, bos411, 9428A410j 8/3/93 16:16:00";
/* 
 * COMPONENT_NAME: CMDUUCP zaps.c
 * 
 * FUNCTIONS: zaps 
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

int derror();
int bdel();
int bstore();
int bload();

int zaps(target,mptr)
struct files *target;
char *mptr;
{
	int err;
	if (mptr == NULL)
		derror(EX_USAGE,"Invalid call for delete!");
	(void) bdel(target,mptr);
	(void) bstore(target);
	if ((err = bload(target)) != EX_OK)
		derror(err,"Can't reload file after delete!");
	return(EX_OK);
}

