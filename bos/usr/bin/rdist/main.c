static char sccsid[] = "@(#)02	1.10  src/bos/usr/bin/rdist/main.c, cmdarch, bos411, 9428A410j 11/9/93 12:06:40";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
char copyright[] =
"(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "(#)main.c	5.1 (Berkeley) 6/6/85";
#endif not lint
*/

#include <nl_types.h>
#include "rdist_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_RDIST,N,S)
nl_catd catd;

#include "defs.h"
#include <locale.h>

#define NHOSTS 100

/*
 * Remote distribution program.
 */

char	*distfile = NULL;
char	tmpf[] = "/tmp/rdistXXXXXX";
char	*tmpname = &tmpf[5];

int	debug;		/* debugging flag */
int	nflag;		/* NOP flag, just print commands without executing */
int	qflag;		/* Quiet. Don't print messages */
int	options;	/* global options */
int	iamremote;	/* act as remote server for transfering files */

FILE	*fin = NULL;	/* input file pointer */
int	rem = -1;	/* file descriptor to remote source/sink process */
char	host[32];	/* host name */
int	nerrs;		/* number of errors while sending/receiving */
char	user[10];	/* user's name */
char	homedir[128];	/* user's home directory */
int	groupid;	/* user's group ID */

struct	passwd *pw;	/* pointer to static area used by getpwent */
struct	group *gr;	/* pointer to static area used by getgrent */

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *arg;
	int cmdargs = 0;
	char *dhosts[NHOSTS], **hp = dhosts;

	(void) setlocale(LC_ALL,"");
	catd = catopen("rdist.cat", NL_CAT_LOCALE);
	pw = getpwuid((uid_t)getuid());
	if (pw == NULL) {
		fprintf(stderr, MSGSTR(WHOARE, "%s: Who are you?\n"), argv[0]);
		exit(1);
	}
	strcpy(user, pw->pw_name);
	strcpy(homedir, pw->pw_dir);
	groupid = pw->pw_gid;
	gethostname(host, sizeof(host));

	while (--argc > 0) {
		if ((arg = *++argv)[0] != '-')
			break;
		if (!strcmp(arg, "-Server"))
			iamremote++;
		else while (*++arg)
			switch (*arg) {
			case 'f':
				if (--argc <= 0)
					usage();
				distfile = *++argv;
				if (distfile[0] == '-' && distfile[1] == '\0')
					fin = stdin;
				break;

			case 'm':
				if (--argc <= 0)
					usage();
				if (hp >= &dhosts[NHOSTS-2]) {
					fprintf(stderr, MSGSTR(DESTTOO, "rdist: too many destination hosts\n"));
					exit(1);
				}
				*hp++ = *++argv;
				break;

			case 'd':
				if (--argc <= 0)
					usage();
				define(*++argv);
				break;

			case 'D':
				debug++;
				break;

			case 'c':
				cmdargs++;
				break;

			case 'n':
				if (options & VERIFY) {
					fprintf(stderr, MSGSTR(OVERR, "rdist: -n overrides -v\n"));
					options &= ~VERIFY;
				}
				nflag++;
				break;

			case 'q':
				qflag++;
				break;

			case 'b':
				options |= COMPARE;
				break;

			case 'R':
				options |= REMOVE;
				break;

			case 'v':
				if (nflag) {
					fprintf(stderr, MSGSTR(OVERR, "rdist: -n overrides -v\n"));
					break;
				}
				options |= VERIFY;
				break;

			case 'w':
				options |= WHOLE;
				break;

			case 'y':
				options |= YOUNGER;
				break;

			case 'h':
				options |= FOLLOW;
				break;

			case 'i':
				options |= IGNLNKS;
				break;

			default:
				usage();
			}
	}
	*hp = NULL;

	seteuid(getuid());
	mktemp(tmpf);

	if (iamremote) {
		server();
		exit(nerrs != 0);
	}

	if (cmdargs)
		docmdargs(argc, argv);
	else {
		if (fin == NULL) {
			if(distfile == NULL) {
				if((fin = fopen("distfile","r")) == NULL)
					fin = fopen("Distfile", "r");
			} else
				fin = fopen(distfile, "r");
			if(fin == NULL) {
				perror(distfile ? distfile : "distfile");
				exit(1);
			}
		}
		yyparse();
		if (nerrs == 0)
			docmds(dhosts, argc, argv);
	}

	catclose(catd);
	exit(nerrs != 0);
}

usage()
{
	fprintf(stderr, MSGSTR(USAGE,
		"Usage: rdist [-nqbRhivwyD] [-f distfile] [-d var=value] [-m host] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE2,
		"or: rdist [-nqbRhivwyD] -c source [...] [login@]machine[:dest]\n"));
	exit(1);
}

/*
 * rcp like interface for distributing files.
 */
docmdargs(nargs, args)
	int nargs;
	char *args[];
{
	register struct namelist *nl, *prev;
	register char *cp;
	struct namelist *files, *hosts;
	struct subcmd *cmds;
	char *dest;
	static struct namelist tnl = { NULL, NULL };
	int i;

	if (nargs < 2)
		usage();

	prev = NULL;
	for (i = 0; i < nargs - 1; i++) {
		nl = makenl(args[i]);
		if (prev == NULL)
			files = prev = nl;
		else {
			prev->n_next = nl;
			prev = nl;
		}
	}

	cp = args[i];
	if ((dest = index(cp, ':')) != NULL)
		*dest++ = '\0';
	tnl.n_name = cp;
	hosts = expand(&tnl, E_ALL);
	if (nerrs)
		exit(1);

	if (dest == NULL || *dest == '\0')
		cmds = NULL;
	else {
		cmds = makesubcmd(INSTALL);
		cmds->sc_options = options;
		cmds->sc_name = dest;
	}

	if (debug) {
		printf("docmdargs()\nfiles = ");
		prnames(files);
		printf("hosts = ");
		prnames(hosts);
	}
	insert(NULL, files, hosts, cmds);
	docmds(NULL, 0, NULL);
}

/*
 * Print a list of NAME blocks (mostly for debugging).
 */
prnames(nl)
	register struct namelist *nl;
{
	printf("( ");
	while (nl != NULL) {
		printf("%s ", nl->n_name);
		nl = nl->n_next;
	}
	printf(")\n");
}

/*VARARGS*/
warn(fmt, a1, a2,a3)
	char *fmt;
{
	extern int yylineno;

	fprintf(stderr, MSGSTR(LINEW, "rdist: line %d: Warning: "), yylineno);
	fprintf(stderr, fmt, a1, a2, a3);
	fputc('\n', stderr);
}
