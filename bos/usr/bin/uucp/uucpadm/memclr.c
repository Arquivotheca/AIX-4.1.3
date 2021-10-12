static char sccsid[] = "@(#)75	1.3  src/bos/usr/bin/uucp/uucpadm/memclr.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:22";
/* 
 * COMPONENT_NAME: CMDUUCP memclr.c
 * 
 * FUNCTIONS: memclr 
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

int memclr(mem)
char *mem[MEMSIZE];
{
	int i;
	for(i=0; i < MEMSIZE;i++)
		mem[i] = NULL;
	return(EX_OK);
}
