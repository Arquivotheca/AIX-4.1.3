static char sccsid[] = "@(#)60	1.5  src/bos/usr/bin/uucp/uucpadm/cyphert.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:05";
/* 
 * COMPONENT_NAME: CMDUUCP cyphert.c
 * 
 * FUNCTIONS: cyphert 
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

int derror();

char *cyphert(src)
char *src;
{
	static char checkit[MAXLINE];
	register int i, num;
	int start, end;
	char *dow="Su+Mo+Tu+We+Th+Fr+Sa+Wk+Any+Never";
	char *s1;
	char *s2;
/* Blow out any string with our delimiter. */
	if (strchr(src,'+') != NULL)
		return(NULL);
	(void) strncpy(checkit,(char *) NULL,(size_t) MAXLINE);
/* Walk through dow. We will reorganize user input to our order. */
	while((num = strcspn(dow,"+")) != 0) {
		for(s1=src; *s1 != '\0'; ++s1) {
			if(*s1 != *dow) 
				continue;
			if (strncmp(s1,dow,(size_t) num) == 0) {
 				if ((strlen(checkit) + num) >= MAXLINE)
					derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
/* bingo. */
				(void) strncat(checkit,dow,(size_t) num);
						}
					}
		dow += num+1;
		}
/* Have the days of the week. Add on any time entries or subfields. */
	if ((s1 = strpbrk(src,"0123456789;")) != NULL && strlen(checkit)) {
/* No junk allowed. */
		if ((i = strspn(s1,"0123456789-;")) == strlen(s1)) {
			s2 = strpbrk(s1,";");
/* scarf the time range. */
			if (s2 == NULL)
				(void) sscanf(s1,"%d-%d",&start,&end);
		else
			if (strlen(s1) > strlen(s2))
				(void) sscanf(s1,"%d-%d",&start,&end);
		if(start < 2401 && end < 2401) {
 				if ((strlen(checkit) + i) >= MAXLINE)
					derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
			(void) strncat(checkit,s1,(size_t) i);	
					}
/* Bad time range. Add subfield if valid. */
		else
			if (strlen(s2) > 1) {
 				if ((strlen(checkit) + strlen(s2)) >= MAXLINE)
					derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
				(void) strcat(checkit,s2);	
				}
		}
	}
	if (strlen(src) == strlen(checkit))
		return(src);
	else
		return(checkit);
}
