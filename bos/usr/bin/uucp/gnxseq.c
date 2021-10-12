static char sccsid[] = "@(#)00	1.6  src/bos/usr/bin/uucp/gnxseq.c, cmduucp, bos411, 9428A410j 4/11/91 10:18:48";
/* 
 * COMPONENT_NAME: UUCP gnxseq.c
 * 
 * FUNCTIONS: cmtseq, gnxseq, ulkseq 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.gnxseq.c
	gnxseq.c	1.1	7/29/85 16:33:06
*/
#include "uucp.h"
/* VERSION( gnxseq.c	5.2 -  -  ); */

extern nl_catd catd;
/*
 * get next conversation sequence number
 *	rmtname	-> name of remote system
 * returns:
 *	0	-> no entery
 *	1	-> 0 sequence number
 */
gnxseq(rmtname)
char *rmtname;
{
	register FILE *fp0, *fp1;
	register struct tm *tp;
/*	extern struct tm *localtime();*/
	int count = 0, ct, ret;
	char buf[BUFSIZ], name[NAMESIZE];
	time_t clock;

	if (access(SQFILE, 0) != 0)
		return(0);

	{
		register int i;
	for (i = 0; i < 5; i++) 
		if ((ret = ulockf(SQLOCK, (time_t)  SQTIME)) == 0)
			break;
		sleep(5);
	}
	if (ret != 0) {
		logent(MSGSTR(MSG_GX1, "CAN'T LOCK"), SQLOCK);
		DEBUG(4, "can't lock %s\n", SQLOCK);
		return(0);
	}
	if ((fp0 = fopen(SQFILE, "r")) == NULL)
		return(0);
	if ((fp1 = fopen(SQTMP, "w")) == NULL) {
		fclose(fp0);
		return(0);
	}
	chmod(SQTMP, 0400);

	while (fgets(buf, BUFSIZ, fp0) != NULL) {
		ret = sscanf(buf, "%s%d", name, &ct);
		if (ret < 2)
			ct = 0;
		name[7] = '\0';
		if (ct > 9998)
			ct = 0;
		if (strncmp(rmtname, name, SYSNSIZE) != SAME) {
			fputs(buf, fp1);
			continue;
		}

		/*
		 * found name
		 */
		count = ++ct;
		time(&clock);
		tp = localtime(&clock);
		fprintf(fp1, "%s %d %d/%d-%d:%2.2d\n", name, ct,
		tp->tm_mon + 1, tp->tm_mday, tp->tm_hour,
		tp->tm_min);

		/*
		 * write should be checked
		 */
		while (fgets(buf, BUFSIZ, fp0) != NULL)
			fputs(buf, fp1);
	}
	fclose(fp0);
	fclose(fp1);
	if (count == 0) {
		rmlock(SQLOCK);
		unlink(SQTMP);
	}
	return(count);
}

/*
 * commit sequence update
 * returns:
 *	0	-> ok
 *	other	-> link failed
 */
cmtseq()
{
	register int ret;

	if ((ret = access(SQTMP, 0)) != 0) {
		rmlock(SQLOCK);
		return(0);
	}
	unlink(SQFILE);
	ret = link(SQTMP, SQFILE);
	unlink(SQTMP);
	rmlock(SQLOCK);
	return(ret);
}

/*
 * unlock sequence file
 */
ulkseq()
{
	unlink(SQTMP);
	rmlock(SQLOCK);
}
