static char sccsid[] = "@(#)81	1.3  src/bos/usr/bin/uucp/uucpadm/update.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:47";
/* 
 * COMPONENT_NAME: CMDUUCP update.c
 * 
 * FUNCTIONS: update 
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

int bins();
int bupd();
int bstore();
int bload();
char *build();

extern char File[FILES][MAXFILE];
extern char entries[MAXENT][MAXLINE];

int update(target,mptr,buffer,num)
struct files *target;
char *mptr;
char *buffer;
int num;
{
	char *p;
	int err;
	if (mptr == NULL) {
/* This is an add. Follow uucp tradition and insert at end of file. */
	(void) bins(target);
	mptr = File[target->spot] + target->length - 2;
	}
/* Eliminate the CTLU. */
	if (p = strchr(buffer,CTLU))
		*p = '\0';
	if (strlen(buffer) > 0) {
		(void) strcpy(entries[num],buffer);
	}
	if ((p = build(target)) == NULL)
		derror(EX_SOFTWARE,"Unable to build line");	
	(void) bupd(target,mptr,p);
	(void) bstore(target);
	if ((err = bload(target)) != EX_OK)
		derror(err,"Can't reload file after delete!");
	return(EX_OK);
}
