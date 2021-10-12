static char sccsid[] = "@(#)86	1.11.1.2  src/bos/usr/bin/from/from.c, cmdmailx, bos411, 9428A410j 11/15/93 14:27:03";
/* 
 * COMPONENT_NAME: CMDMAILX from.c
 * 
 * FUNCTIONS: Mfrom, match 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *  char copyright[] =
 *  " Copyright (c) 1980 Regents of the University of California.\n\
 *   All rights reserved.\n";
 *  static char sccsid[] = "from.c	5.2 (Berkeley) 11/4/85";
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <locale.h>
#include <nl_types.h>

#include "from_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(Num, Def)	catgets(scmc_catd, MS_from, Num, Def)

main(argc, argv)
	int argc;
	register char **argv;
{
	char lbuf[BUFSIZ];
	char lbuf2[BUFSIZ];
	register struct passwd *pp;
	int ch, idx;
	register char *name;
	char *sender;
	char *getlogin();
	char *maildir = "/usr/spool/mail";  /* default mailbox dir */
	extern char *optarg;
	extern int optind;

        setlocale(LC_ALL,"");
	scmc_catd = catopen(MF_FROM,NL_CAT_LOCALE);

	sender = NULL;
        while ((ch = getopt(argc, argv, "s:d:")) != EOF)
                switch(ch) {
		    case 'd':  /* set system mailbox directory */
			    maildir = optarg;
			    break;
		    case 's':
			    sender = optarg;
			    for (name = sender; *name; name++)
				    *name = tolower(*name);
			    break;
		    default:
			    fprintf(stderr, MSGSTR(M_MSG_1,
			    "Usage: from [-d directory] [-s sender] [user]\n"));
			    exit (1);
			    break;
                }

	if (chdir(maildir) < 0) {
		perror(maildir);
		exit(1);
	}
	if (argc > optind)  /* user is on cmd line */
		name = argv[optind];
	else {
		name = getlogin ();
		if (name == NULL || strlen(name) == 0) {
			pp = getpwuid(getuid());
			if (pp == NULL) {
				fprintf(stderr, MSGSTR(M_MSG_2,
				    "Cannot get your login name\n"));
				exit(1);
			}
			name = pp->pw_name;
		}
	}
	if (freopen(name, "r", stdin) == NULL) {
		if (errno != ENOENT) {  /* ok if file doesn't exist: no mail */
			fprintf(stderr, MSGSTR(M_MSG_3, 
				"Cannot open %s's mailbox\n"), name);
			exit(1);
		}
	}
	while (fgets(lbuf, sizeof lbuf, stdin) != NULL)
		if (strncmp(lbuf, "From ",5) == 0) {

                        /* These changes are made so that the correct user
                           name and date are displayed. */

                        /* save time and date when mail received */
                        for (idx = 0;lbuf[idx] && lbuf[idx] != ' ';idx++)
                                ;
                        ++idx;
                        for (;lbuf[idx] && lbuf[idx] != ' ';idx++)
                                ;
                        strcpy(lbuf2,&lbuf[idx]);
                        while (fgets(lbuf, sizeof lbuf, stdin) != NULL)
                                if (strncmp(lbuf, "From:", 5) == 0) {
					if (sender == NULL ||
					    match(&lbuf[5], sender)) {
                                        	for(idx=0;lbuf[idx];idx++);
                                        	idx--;
                                        	if(lbuf[idx] == '\n')
                                                	lbuf[idx] = ' ';
                                        	strcat(lbuf,lbuf2);
                                        	printf("%s", lbuf);
                                        	break;
					}
					break;
                                }
                }
	exit(0);
}

match (line, str)
	register char *line, *str;
{
	register char ch;

	while (*line == ' ' || *line == '\t')
		++line;
	if (*line == '\n')
		return (0);
	while (*str && *line != ' ' && *line != '\t' && *line != '\n') {
		ch = isupper(*line) ? tolower(*line) : *line;
		if (ch != *str++)
			return (0);
		line++;
	}
	return (*str == '\0');
}
