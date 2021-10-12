static char sccsid[] = "@(#)59	1.4  src/bos/usr/bin/uucp/uucpadm/cypherp.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:01";
/* 
 * COMPONENT_NAME: CMDUUCP cypherp.c
 * 
 * FUNCTIONS: cypherp 
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

int cypherp(src,target)
char *src;
struct files *target;
{
	register char *s1;
	register int hit, num;
	s1 = src;
	/* already have entry one, just scoot on by. */
	num = strcspn(s1,target->delimit);
	s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
	num = strspn(s1,target->delimit);
	s1 += (strlen(s1) > num) ? num : strlen(s1);
	while (strlen(s1)) {
 		hit = 1;
		while (hit) {
			hit = 0;
			if (!strlen(entries[1])) {		
				if ((strncmp(s1,"REQUEST=",(size_t) 8)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[1],s1,(size_t) num);
					entries[1][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			if (!strlen(entries[2])) {		
				if ((strncmp(s1,"SENDFILES=",(size_t) 10)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[2],s1,(size_t) num);
					entries[2][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			if (!strlen(entries[3])) {		
				if ((strncmp(s1,"NOREAD=",(size_t) 7)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[3],s1,(size_t) num);
					entries[3][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				if ((strncmp(s1,"READ=",(size_t) 5)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[3],s1,(size_t) num);
					entries[3][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			if (!strlen(entries[4])) {		
				if ((strncmp(s1,"NOWRITE=",(size_t) 8)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[4],s1,(size_t) num);
					entries[4][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				if ((strncmp(s1,"WRITE=",(size_t) 6)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[4],s1,(size_t) num);
					entries[4][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			if (!strlen(entries[5])) {		
				if ((strncmp(s1,"CALLBACK=",(size_t) 9)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[5],s1,(size_t) num);
					entries[5][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			if (!strlen(entries[6])) {		
				if ((strncmp(s1,"COMMANDS=",(size_t) 9)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[6],s1,(size_t) num);
					entries[6][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			if (!strlen(entries[7])) {		
				if ((strncmp(s1,"VALIDATE=",(size_t) 9)) == 0) {
					num = strcspn(s1,target->delimit);
					if (num >= MAXLINE)
						derror(EX_SOFTWARE,"Entry too long for menu. Use editor.");
					(void) strncpy(entries[7],s1,(size_t) num);
					entries[7][num] = '\0';
					s1 += (strlen(s1) > num) ? num + 1 : strlen(s1);
					num = strspn(s1,target->delimit);
					s1 += (strlen(s1) > num) ? num : strlen(s1);
 					hit = 1;
					}
				}
			}
	num = strspn(s1,target->delimit);
	s1 += (num) ? num : 1;
		}
	return (EX_OK);
}
