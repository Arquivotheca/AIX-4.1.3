static char sccsid[] = "@(#)77	1.5  src/bos/usr/bin/uucp/uucpadm/srch.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:30";
/* 
 * COMPONENT_NAME: CMDUUCP srch.c
 * 
 * FUNCTIONS: srch 
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
/*
 *  Uucpadm
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */

/*
 *  srch - locate a string in a multistring buffer.
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"

char  *srch (s1, len1, strt, last, spot, target)
char  *s1, *strt, *last;
struct files *target;
int  len1, spot;
{
    int i, imove;
    char  *cur;
    char  *buf;
    char  *chartmp;

    cur = strt;
    while (1)
    {
	if (*cur == 0xff || (last != NULL && cur >= last))
	    return (NULL);
	buf = cur;
/* ignore continuation lines. */
	if (strchr(target->comment,(int) *buf) != NULL) {
		cur += strlen (cur) + 1;
		continue;
		}
/* Locate the spot. */
	for(i=1;i < spot;i++) {
		if ((buf = strpbrk(buf,target->delimit)) == NULL)
			break;
		imove = strspn(buf,target->delimit);
			buf += imove;
		}
	if (buf == NULL) {
		cur += strlen (cur) + 1;
		continue;
		}

	if ( (s1 != NULL && !strncmp (s1, buf,(size_t) len1))) {
/* Require exact match. */
/*
 *              The following line was causing entries to be 
 *              truncated.  All we are doing is checking the length, 
 *              and we can compute that by subtraction, so instead of
 *              putting in a '\0', just check the length. 
 */
/* 		 *strpbrk(buf,target->delimit) = '\0'; */
/*		 if (strlen(s1) == strlen(buf))        */
/*		    return (cur);                      */
/*
 *              If no delimiter found, set chartmp to point to the end
 *              of the string.
 */
              chartmp = strpbrk(buf,target->delimit);
              if (chartmp == NULL) chartmp = (char *) (buf + strlen(buf));
              if (strlen(s1) == (int) chartmp - (int) buf) return(cur);
	}
	cur += strlen (cur) + 1;
    }
/*NOTREACHED*/
}
