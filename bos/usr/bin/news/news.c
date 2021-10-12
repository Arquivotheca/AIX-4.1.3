static char sccsid[] = "@(#)97 1.20  src/bos/usr/bin/news/news.c, cmdcomm, bos41B, 9504A 12/23/94 12:51:29";
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: news
 *
 * ORIGINS: 3, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/*
 * news.c	1.14  com/cmd/comm/news.c, bos320 4/12/91 15:02:38";
 */

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <pwd.h>
#include <utime.h>
#include <langinfo.h>

#include "news_msg.h"

static nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_NEWS, Num, Str)

/* The number of leading spaces on each line of output.  This value should
 * be less than the number of spaces in a tab.
 */
#define INDENT 3

/*
 *	The following items should not be printed.
 */
static char *ignore[] = {
	"core",
	NULL
};

static struct n_file {
	long n_time;
	char *n_name;
} *n_list;

static char NEWS[] = "/var/news"; /* d70416 */

static int n_count;
static int errflag = 0;
static char stdbuf[BUFSIZ];
static void	usage(void);

static jmp_buf	save_addr;

/*
 * NAME: news [ -a | -n | -s | items ]
 *
 * FUNCTION:  displays the current news for the system
 *            anyone with w permission for /var/news directory can
 *            post the news and with r permission can read the news.
 *	news foo	prints /var/news/foo
 *	news -a		prints all news items, latest first
 *	news -n		lists names of new items
 *	news -s		tells count of new items only
 *	news		prints items changed since last news
 */
main (argc, argv)
	int argc;
	char **argv;
{
	int print_item(), notify(), count();
	int c;
	int aflag=0, nflag=0, sflag=0;
	extern int optind;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_NEWS, NL_CAT_LOCALE);
	setbuf (stdout, stdbuf);
	initialize();
	read_dir();
	if (argc <= 1) {
		late_news (print_item, 1);
		exit (0);
	}
	while ((c = getopt(argc, argv, "ans")) != -1) {
		switch (c) {
		case 'a':
			aflag = 1;
			break;

		case 'n':
			nflag = 1;
			break;

		case 's':
			sflag = 1;
			break;

		default:
			usage();
		}
	}
	if ((aflag + nflag + sflag + (optind < argc)) > 1)
		usage();

	if (aflag)
		all_news();
	else if (nflag)
		late_news (notify, 0);
	else if (sflag)
		late_news (count, 0);
	else {
		for (; optind < argc; optind++)
			print_item (argv[optind]);
	}
	exit(!!errflag);
}

/*
 * NAME: read_dir
 *
 * FUNCTION:  get the file names and modification dates for the
 *	files in /var/news into n_list; sort them in reverse by
 *	modification date. We assume /var/news is the working directory.
 */
static read_dir()
{
	struct	dirent *nf;
	struct	stat sbuf;
	DIR	*fd;
	int	i, j;
	char	*malloc(), *realloc();
	long	name_max;	/* max # bytes in file name in current dir */

	/* Open the current directory */
	if ((fd = opendir (".")) == NULL) {
		fprintf (stderr, MSGSTR(CANTOPEN, "Cannot open %s\n"), NEWS);
		exit (1);
	}

	name_max = pathconf(".", _PC_NAME_MAX);

	/* Read the file names into n_list */
	n_count = 0;
	while ((nf = readdir(fd)) != NULL) {
		if (stat(nf->d_name, &sbuf) >= 0
				&& (sbuf.st_mode & S_IFMT) == S_IFREG) {
			register char **p;
			p = ignore; /* list of file names to ignore */
			while (*p && strncmp (*p, nf->d_name, nf->d_namlen))
				++p;
			if (!*p) { /* if file name not in ignore list... */
				if (n_count++ > 0)
					n_list = (struct n_file *)
						realloc ((char *) n_list,
						(unsigned)
						(sizeof (struct n_file)
						    * n_count));
				else
					n_list = (struct n_file *) malloc
						((unsigned)
						(sizeof (struct n_file) *
						n_count));
				if (n_list == NULL) {
					fprintf (stderr, MSGSTR(NOSTRG,
						"No storage\n"));
					exit (1);
				}
				n_list[n_count-1].n_name = malloc(name_max);
				if (n_list[n_count-1].n_name == NULL) {
					fprintf (stderr, MSGSTR(NOSTRG,
						"No storage\n"));
					exit (1);
				}
				n_list[n_count-1].n_time = sbuf.st_mtime;
				strncpy (n_list[n_count-1].n_name,
					nf->d_name, nf->d_namlen);
				n_list[n_count-1].n_name[nf->d_namlen] = '\0';
			}
		}
	}

	/* Sort the elements of n_list in decreasing time order */
	for (i=1; i<n_count; i++)
		for (j=0; j<i; j++)
			if (n_list[j].n_time < n_list[i].n_time) {
				struct n_file temp;
				temp = n_list[i];
				n_list[i] = n_list[j];
				n_list[j] = temp;
			}

	/* Clean up */
	closedir (fd);
}

/*
 * NAME: initialize
 *                                                                    
 * FUNCTION: initialize by catching SIGQUIT and change current directory
 *           to news directory
 */  
static initialize()
{
	if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
		signal (SIGQUIT, (void (*)(int))_exit);
	umask ((mode_t)022);
	if (chdir (NEWS) < 0) {
		fprintf (stderr, MSGSTR(CANTCHDIR,
			"Cannot chdir to %s\n"), NEWS);
		exit (1);
	}
}

/*
 * NAME: all_news
 *                                                                    
 * FUNCTION: display all of the news items
 */  
static all_news()
{
	int i;

	for (i=0; i<n_count; i++)
		print_item (n_list[i].n_name);
}

/*
 * NAME: print_item
 *                                                                    
 * FUNCTION: print news item
 */  
static print_item (fname)
	char *fname;
{
	char tstr[BUFSIZ];

	register int c;
	register int i, iflg;
	struct stat sbuf;
	struct passwd *pw;
	FILE *fd;
	static int firstitem = 1;
	int onintr(void);

	if (fname == NULL) {
		return;
	}
	if ((fd = fopen (fname, "r")) == NULL) {
		fprintf (stderr, MSGSTR(OPENERR, "Cannot open %s/%s\n"), NEWS, fname);
		errflag++;
	} else {
		fstat ((int)fileno(fd), &sbuf);
		if (firstitem) {
			firstitem = 0;
			putchar ('\n');
		}
		if (setjmp(save_addr))
			goto finish;
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, (void (*)(int))onintr);
		printf ("%s ", fname);
		pw = getpwuid (sbuf.st_uid);
		if (pw)
			printf ("(%s)", pw->pw_name);
		else
			printf (".....");

		strftime(tstr, BUFSIZ, nl_langinfo(D_T_FMT),
					localtime(&sbuf.st_mtime));
		printf (" %s\n", tstr);

		/* If we're at the beginning of a line, then remember that
		 * we have to indent.  If we are outputting a character, then
		 * check to see if we are at the beginning of a line, and if
		 * we are, indent first.
		 */
		iflg = 1;       /* at beginning of line ==> we need to indent */
		while ((c = getc (fd)) != EOF) {
			switch (c) {
			case '\r':
			case '\n':
				iflg = 1;
				putchar(c);
				break;
			default:
				if (iflg) {
					for (i=0; i<INDENT; i++)
					putchar(' ');
					iflg = 0;
				}
				putchar (c);
				break;
			}
		}
		fflush (stdout);
finish:
		putchar ('\n');
		fclose (fd);
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, SIG_DFL);
	}
}

/*
 * NAME: late_news
 *                                                                    
 * FUNCTION: display the news not read yet
 */  
static late_news (emit, update)
	int (*emit)();	/* address of routine to display file.  Routine
			 * must have one argument which is the name of
			 * the file being displayed.
			 */
	int update;	/* if update != 0, recreate $HOME/.news_time
			 * in order to refresh update time
			 */
{
	long cutoff;
	int i;
	char fname[PATH_MAX], *cp;
	struct stat newstime;
	int fd;
	struct utimbuf utb;

	/* Determine the time when last called */
	cp = getenv ("HOME");
	if (cp == NULL) {
		fprintf (stderr, MSGSTR(NOHOME, "Cannot find HOME variable\n"));
		exit (1);
	}
	strcpy (fname, cp);
	strcat (fname, "/.news_time");
	cutoff = stat (fname, &newstime) < 0? 0: newstime.st_mtime;

	/* Print the recent items */
	for (i=0; i<n_count && n_list[i].n_time > cutoff; i++)
		(*emit) (n_list[i].n_name);
	(*emit) ((char *) NULL);
	fflush (stdout);

	if (update) {
		/* Re-create $HOME/.news_time to refresh the update time */
		if (n_count > 0 && (fd = creat (fname, 0666)) >= 0) {
			/* n_list was sorted to have most recent at index 0 */
			utb.actime = utb.modtime = n_list[0].n_time;
			close (fd);
			utime (fname, &utb);
		}
	}
}

/*
 * NAME: notify
 *                                                                    
 * FUNCTION: notify user of s news item
 */  
static notify (s)
	char *s;
{
	static int first = 1;

	if (s) {
		if (first) {
			first = 0;
			printf (MSGSTR(NEWSMSG, "news:\n")); /* d77279 */
		}
		printf ("%s\n", s); /* d77279 */
	} else if (!first)
		putchar ('\n');
}

/*
 * NAME: count
 *                                                                    
 * FUNCTION: display the number of news items unread
 */  
/*ARGSUSED*/
static count (s)
	char *s;
{
	static int nitems = 0;

	if (s)
		nitems++;
	else if (nitems) {
		if (nitems == 1)
			printf (MSGSTR(NEWSITM, "%d news item.\n"), nitems);
		else
			printf (MSGSTR(NEWSITMS, "%d news items.\n"), nitems);
	}

}

/*
 * NAME: onintr
 *                                                                    
 * FUNCTION: on interrupt, pause for 2 seconds, then continue
 */  
static onintr(void)
{
	sleep((unsigned)2);
	longjmp(save_addr, 1);
}

/*
 * NAME: usage
 *
 * FUNCTION: print a usage message
 */
void
usage(void)
{
	fprintf(stderr,
		MSGSTR(USAGE, "Usage: news [ -a | -n | -s | items ]\n"));
	exit(1);
}
