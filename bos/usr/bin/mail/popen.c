static char sccsid[] = "@(#)48	1.3  src/bos/usr/bin/mail/popen.c, cmdmailx, bos411, 9428A410j 11/30/93 13:34:01";
/*
 *   COMPONENT_NAME: cmdmailx
 *
 *   FUNCTIONS: Fclose
 *		Fdopen
 *		Fopen
 *		MSGSTR
 *		Pclose
 *		Popen
 *		close_all_files
 *		register_file
 *		unregister_file
 *
 *   ORIGINS: 26,27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 *   ALL RIGHTS RESERVED
 *
 *   Copyright (c) 1980 Regents of the University of California.
 *   All rights reserved.  The Berkeley software License Agreement
 *   specifies the terms and conditions for redistribution.
 */
/*
 * #ifndef lint
 * static char *sccsid = "popen.c"	5.15 (Berkeley) 6/25/90";
 * #endif not lint
 */


#include "rcv.h"

#include "mail_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s)

struct fp {
	FILE *fp;
	int pipe;
	struct fp *link;
};
static struct fp *fp_head;

static void register_file();
static void unregister_file();


FILE *
Fopen(file, mode)
	char *file, *mode;
{
	FILE *fp;

	if ((fp = fopen(file, mode)) != NULL)
		register_file(fp, 0);
	return fp;
}

FILE *
Fdopen(fd, mode)
	char *mode;
{
	FILE *fp;
	
	if ((fp = fdopen(fd, mode)) != NULL)
		register_file(fp, 0);
	return fp;	
}

int
Fclose(fp)
	FILE *fp;
{
	unregister_file(fp);
	return fclose(fp);
}

FILE *
Popen(cmd, mode)
	char *cmd, *mode;
{
	FILE *fp;

	if ((fp = popen(cmd, mode)) != NULL)
		register_file(fp, 1);
	return fp;
}

int
Pclose(fp)
	FILE *fp;
{
	unregister_file(fp);
	return pclose(fp);
}

void
close_all_files()
{

	while (fp_head)
		if (fp_head->pipe)
			(void) Pclose(fp_head->fp);
		else
			(void) Fclose(fp_head->fp);
}

static void
register_file(fp, pipe)
	FILE *fp;
{
	struct fp *fpp;

	if ((fpp = (struct fp *) malloc(sizeof *fpp)) == NULL)
		panic(MSGSTR(MEMORY, "Out of memory"));
	fpp->fp = fp;
	fpp->pipe = pipe;
	fpp->link = fp_head;
	fp_head = fpp;
}

static void
unregister_file(fp)
	FILE *fp;
{
	struct fp **pp, *p;

	for (pp = &fp_head; p = *pp; pp = &p->link)
		if (p->fp == fp) {
			*pp = p->link;
			free((char *) p);
			return;
		}
}
