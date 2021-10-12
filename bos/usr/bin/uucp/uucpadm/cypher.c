static char sccsid[] = "@(#)58	1.5  src/bos/usr/bin/uucp/uucpadm/cypher.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:58";
/* 
 * COMPONENT_NAME: CMDUUCP cypher.c
 * 
 * FUNCTIONS: cypher 
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
#include <string.h>
#include <stdio.h>
#include "defs.h"

extern char entries[MAXENT][MAXLINE];

int derror();

int cypher(src,target,counts)
char *src;
struct files *target;
int counts;
{
	register char *s1;
	register int i, num;
	s1 = src;
	for (i=0;i < counts;i++) {
	num = strcspn(s1,target->delimit);
	if (num >= MAXLINE)
		derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
	(void) strncpy(entries[i],s1,(size_t) num);
	entries[i][num] = '\0';
	s1 += num;
	num = strspn(s1,target->delimit);
	s1 += num;
		}
/* Munch the rest of the line into entries[counts-1]. who knows what we
*  will get.
*/
	if (strlen(s1) >= MAXLINE)
		derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
/*	(void) strcpy(entries[counts - 1],s1);  */
	return (EX_OK);
}
