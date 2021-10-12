static char sccsid[] = "@(#)94  1.47  src/bos/usr/bin/man/man.c, cmdman, bos41J, 9519A_all 5/5/95 16:31:41";
/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: runpath, manual, pathstat, nroff, troff, local_remove,
 *	      blklen, apropos, match, amatch, lmatch, whatis,
 *	      wmatch, trim, usage, usage2, usage3
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#define _ILS_MACROS
#include "man_msg.h"
static nl_catd  man_catd;   /* Cat descriptor for scmc conversion */
#define MSGSTR(num,str) catgets(man_catd, MS_man, num, str)

#if defined(KRS_API) /* Symbol Clashes with KRS      */
#define trim    mantrim
#define match   manmatch
#endif

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <errno.h>
#include <sgtty.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <signal.h>
#include <strings.h>
#include <ctype.h>

/*
 * man
 * link also to apropos and whatis
 * This version uses more for underlining and paging.
 */
#define NROFFX "/usr/bin/nroff"         /*  need to check if nroff installed */
#define TROFFX "/usr/bin/troff"         /*  need to check if troff installed */
#define NTBL "/usr/bin/tbl -TX"         /* tbl command when used with nroff */
#define TTBL "/usr/bin/tbl"             /* tbl command when used with troff */
#define NROFFCAT "/usr/bin/nroff -h -man" /* for nroffing to cat file */
#define NROFF   "/usr/bin/nroff -man"   /* for nroffing to tty */
#define MORE_S  "/usr/bin/more -s"      /* option for more */
#define CAT_    "/usr/bin/cat"          /* for when output is not a tty */
#define CAT_S   "/usr/bin/cat -s"       /* for '-' opt (no more) */
#define PCAT    "/usr/bin/pcat"         /* for when output is compressed */
#define TROFFCMD "/usr/bin/troff -man"
#define ALLSECT "CLFnlpo12345678" /* Order to search for sections */
#define SUBSEC1 "cgxm"		/* Subsections to try in section 1*/
#define SUBSEC3 "sxmncf"	/* Subsections to try in section 3*/
#define SUBSEC4 "pfn"		/* Subsections to try in section 4*/
#define SUBSEC8 "cv"		/* Subsections to try in section 8*/
#define WHATIS  "whatis"
#define KRSFAILURE 2

static int     nomore;
static char    *CAT    = CAT_;
static char    manpath[PATH_MAX] = "/usr/share/man";
static char    pager_string[PATH_MAX] = MORE_S;
char    tempfile[20];
FILE    *fp;
static struct	stat tempstat;
static int	fromknowset = KRSFAILURE;
char    *getenv();
int     local_remove(void);
int     apropos();
int     whatis();
static int     section;
static int     sub_sec;
static int	troffit = 0;
int     mypid;
static int	errflag = 0;
static int	fflag = 0;
static int	kflag = 0;
static int     zflag;
static char	*progname;

/*****************************************************************************
 ****************************************************************************/
main(int argc, char **argv)
{
	int c;
	char *mp, *pgrs;	/* man path and pager string */

	progname = argv[0];
	(void) setlocale(LC_ALL, "");
	man_catd = catopen(MF_MAN, NL_CAT_LOCALE);

	if ((mp = getenv("MANPATH")) != NULL && strlen(mp) > 0)
		strcpy(manpath,mp);
	if ((pgrs = getenv("PAGER")) != NULL && strlen(pgrs) > 0)
		strcpy(pager_string,pgrs); /* assign pager program */
        umask((mode_t)0);
	mypid = getpid(); 
/*  test if command is apropos or whatis and call
        (only in this case) runpath()
*/
        if (strcmp(basename(argv[0]), "apropos") == 0)
                runpath(&argc, &argv, apropos);
        if (strcmp(basename(argv[0]), "whatis") == 0)
                runpath(&argc, &argv, whatis);
	while ((c = getopt(argc, argv, "cfkM:P:t")) != EOF) {
                switch(c) {
		case 'c':
			nomore = 1;
			CAT = CAT_S;
			if (pgrs == NULL || strlen(pgrs) == 0)
				strcpy(pager_string,CAT);
			break;
                case 'f':
			fflag = (kflag | troffit) ? usage() : 1;
                        break;
                case 'k':
			kflag = (fflag | troffit) ? usage() : 1;
                        break;
                case 'M':
                case 'P':               /* backwards compatibility */
                        (void) strcpy(manpath, optarg);
                        break;
                case 't':
			troffit = (fflag | kflag) ? usage() : 1;
			break;
                default:
                        usage();
                }
        }
	argc -= optind;
	argv += optind;
	if (fflag)
		whatis(argc, argv);
	if (kflag)
		apropos(argc, argv);
	if (!argc)
		usage();
	if (troffit == 0 && nomore == 0 && !isatty(1))
		nomore++;
	/* find section on command line, if there is one */
	if ((strchr(ALLSECT, (int)argv[0][0]) && (argv[0][1] == 0)) ||
	(isdigit((int)argv[0][0]) && (argv[0][1] == 0 || argv[0][2] == 0))) {
		section = argv[0][0];
		sub_sec = argv[0][1];
		argc--, argv++;
		if (argc == 0) {
			fprintf(stderr, MSGSTR(M_MSG_1, 
				"The man page name is missing.\n"));
			usage();
		}
	}
	signal(SIGINT, local_remove);
	signal(SIGQUIT, local_remove);
	signal(SIGTERM, local_remove);
	do {
		manual(section, sub_sec, argv[0]);
		argc--, argv++;
	} while (argc > 0);
	exit(errflag ? 1 : 0);
}

/*****************************************************************************
 ****************************************************************************/
static runpath(int *ac, char **av[], int (*f)())
{
	int c;

	while ((c = getopt(*ac, *av, "M:P:")) != EOF) {
		switch(c) {
		case 'M':
		case 'P':
			*manpath = optarg;
			break;
		default:
			if (strcmp(basename(progname), "apropos"))
				usage3();
			else
				usage2();
		}
	}
	*ac -= optind;
	*av += optind;
	(*f)(*ac, *av);
}

/*****************************************************************************
 ****************************************************************************/
static manual(int sec, int subsec, char *name)
{
	char work[PATH_MAX], work2[PATH_MAX];
	char cmdbuf[PATH_MAX];
	char path[PATH_MAX+1], realname[PATH_MAX+1];
	struct stat stbuf, stbuf2, stbuf3;
	int last;
	char *sp = ALLSECT;
	FILE *it;
	char abuf[BUFSIZ];
	char *cp;

	strcpy(work, "manX/");
	strcat(work, name);
	last = strlen(work);
	if (sec) {
		abuf[0] = sec;
		abuf[1] = '\0';
		sp = abuf;
	}
	for (; *sp; sp++) { 
		zflag=0;
		work[3] = *sp;
		work[last] = 0;
		if (pathstat(work, path, &stbuf))
			break;
		work[last] = '.';
		work[last+1] = *sp;
		work[last+2] = subsec;
		work[last+3] = '\0';
		if (pathstat(work, path, &stbuf))
			break;
                /*  The filename may have the <.section> extension and
                    be in compressed format <.z> */
		work[last+2] = '.';
		work[last+3] = 'z';
		work[last+4] = '\0';
		zflag=1; 
		if (pathstat(work, path, &stbuf))
			break;
		zflag=0; 
                /*  If subsection not defined try default subsections */
		if (!subsec) {
			switch (*sp) {
			case '1': cp = SUBSEC1; break;
			case '3': cp = SUBSEC3; break;
			case '4': cp = SUBSEC4; break;
			case '8': cp = SUBSEC8; break;
			default: continue;
			}
			work[last+3] = '\0';
			while (*cp) {
				work[last+2] = *cp++;
				if (pathstat(work, path, &stbuf))
					goto found;
			}
		}
	}
found:
	if (!*sp) {  /* Search failed */
#if defined(KRS_API)
		if ((fromknowset = KnowSet(name, sec)) != KRSFAILURE) {
			if (isatty(1)) {
				sprintf(cmdbuf, "%s %s; trap '' 1 15", pager_string, tempfile);
				if (system(cmdbuf)) {
					unlink(tempfile);
					exit(1);
				}
				unlink(tempfile);
			}
			return;
		}
#endif
		if (!sec)
			fprintf(stderr, MSGSTR(M_MSG_3,
			"No manual entry found for %s.\n"), name);
		else
			fprintf(stderr, MSGSTR(M_MSG_4, 
			"No entry found for %s in section %c of the manual.\n"), name, sec);
		errflag++;
		return;
	}
	sprintf(realname, "%s/%s", path, work);         /* Search succeeded */
	if (troffit) {
		if (stat(TROFFX, &stbuf3)) {
			fprintf(stderr, MSGSTR(M_MSG_18, "Nroff/troff are not currently installed, they must be \n \tinstalled in order to use formatted man pages.\n"));
			exit(1);
		}
		troff(path, work);
		return;
	}
	if (!nomore) {
		if ((it = fopen(realname, "r")) == NULL) {
			goto catit;
		}
		if (fgets(abuf,BUFSIZ-1,it) != NULL && strncmp(abuf,".so ",4) == 0) {
			register char *cp = abuf+4;
			char *dp;

			while (*cp && *cp != '\n')
				cp++;
			*cp = 0;
			while (cp > abuf && *--cp != '/')
				;
			dp = ".so man";
			if (cp != abuf+strlen(dp)+1) {
tohard:
				fclose(it);
				nomore = 1;
				strcpy(work, abuf+4);
				goto hardway;
			}
			for (cp = abuf; *cp == *dp && *cp; cp++, dp++)
				;
			if (*dp)
				goto tohard;
			strcpy(work, cp-3);
		}
		fclose(it);
	}
catit:
	strcpy(work2, "cat");
	work2[3] = work[3];
	work2[4] = 0;
	sprintf(realname, "%s/%s", path, work2);

	if (stat(realname, &stbuf2) < 0)
		goto hardway;
	strcpy(work2+4, work+4);
	sprintf(realname, "%s/%s", path, work2);

	if (stat(realname, &stbuf2) < 0 || stbuf2.st_mtime < stbuf.st_mtime) {
		if (stat(NROFFX,&stbuf3)) {
			fprintf(stderr, MSGSTR(M_MSG_18, "Nroff/troff are not currently installed, they must be \n \tinstalled in order to use formatted man pages.\n"));
			exit(1);
		}
		if (nomore)
			goto hardway;
		fprintf(stderr, MSGSTR(M_MSG_5, "Reformatting page.  Wait..."));
		fflush(stderr);
		sprintf(cmdbuf, "%s %s/%s | %s > /tmp/man%d; trap '' 1 15",
			NTBL, path, work, NROFFCAT, mypid);
		if (system(cmdbuf)) {
			fprintf(stderr, MSGSTR(M_MSG_6, " aborted (sorry)\n"));
			local_remove();
                        /*NOTREACHED*/
		}
		sprintf(cmdbuf,"test -s /tmp/man%d",mypid);
		if (system(cmdbuf)) {
			fprintf(stderr, MSGSTR(M_MSG_19, "\nError writing to temporary file.\n"));
			local_remove();
		}
		sprintf(cmdbuf, "/usr/bin/mv -f /tmp/man%d %s/%s 2>/dev/null",
			mypid, path, work2);

		if (system(cmdbuf)) {
			sprintf(path,  "/");
			sprintf(work2, "tmp/man%d", mypid);
		}
		fprintf(stderr, MSGSTR(M_MSG_7, " finished\n") );
	}
	strcpy(work, work2);
hardway:
	nroff(path, work);
}

/*****************************************************************************
 ****************************************************************************/
/*
 * Use the manpath to look for
 * the file name.  The result of
 * stat is returned in stbuf, the
 * successful path in path.
 */
static pathstat(name, path, stbuf)
        char *name, path[];
        struct stat *stbuf;
{
        char *cp, *tp, *ep;
        char **cpp;
        static char *manpaths[] = {"man", "cat", 0};
        static char *nopaths[]  = {"", 0};

        if (strncmp(name, "man", (size_t)3) == 0)
                cpp = manpaths;
        else
                cpp = nopaths;
        for ( ; *cpp ; cpp++) {
                for (cp = manpath; cp && *cp; cp = tp) {
                        tp = strchr(cp, ':');
                        if (tp) {
                                if (tp == cp) {
                                        sprintf(path, "%s%s", *cpp,
                                                name+strlen(*cpp));
                                }
                                else {
                                        sprintf(path, "%.*s/%s%s", tp-cp, cp,
                                                *cpp, name+strlen(*cpp));
                                }
                                ep = path + (tp-cp);
                                tp++;
                        } else {
                                sprintf(path, "%s/%s%s", cp, *cpp,
                                        name+strlen(*cpp));
                                ep = path + strlen(cp);
                        }
                        if (stat(path, stbuf) >= 0) {
                                *ep = '\0';

                                return (1);
                        }
                }
        }
        return (0);
}

/*****************************************************************************
 ****************************************************************************/
static nroff(char *pp, char *wp)
{
	char cmdbuff[BUFSIZ];

	chdir(pp);

	if (wp[0] == 'c' || wp[0] == 't') {
		if(zflag) {  /* file is compressed with .z */
			wp[strlen(wp)-2]='\0';
			sprintf(cmdbuff, "%s %s|%s", PCAT, wp, pager_string);
		} else
			sprintf(cmdbuff, "%s %s", nomore? CAT : pager_string, wp);
	} else
		sprintf(cmdbuff, nomore? "%s %s | %s" : "%s %s | %s | %s", NTBL, wp, NROFF, pager_string);
	(void) system(cmdbuff);
	if (wp[0] == 't')
		unlink(wp);
	return;
}

/*****************************************************************************
 ****************************************************************************/
static troff(char *pp, char *wp)
{
	char cmdbuf[BUFSIZ];

	chdir(pp);
	sprintf(cmdbuf, "%s %s | %s", TTBL, wp, TROFFCMD);
	(void) system(cmdbuf);
	if (wp[0] == 't')
		unlink(wp);
}

/*****************************************************************************
 ****************************************************************************/
static any(register int c, register char *sp)
{
	register int d;

	while (d = *sp++)
		if (c == d)
			return (1);
	return (0);
}

/*****************************************************************************
 ****************************************************************************/
static local_remove(void)
{
	char name[15];

	sprintf(name, "/tmp/man%d", mypid);
	unlink(name);
	exit(1);
}

/*****************************************************************************
 ****************************************************************************/
static unsigned int
blklen(register char **ip)
{
	register unsigned int i = 0;

	while (*ip++)
		i++;
	return (i);
}

/*****************************************************************************
 ****************************************************************************/
static apropos(int argc, char **argv)
{
	char buf[BUFSIZ], file[BUFSIZ+1];
	char *gotit, *cp, *tp;
	register char **vp;
	FILE *fp;
	int i;
	int isempty;

	if (argc == 0) {
		fprintf(stderr, MSGSTR(M_MSG_8, "Missing argument\n"));
		if (kflag)
			usage();
		else
			usage2();
	}
	gotit = calloc(1, blklen(argv));
	/*      First Check the "whatis" file           */
	for (cp = manpath; cp; cp = tp) {
		tp = strchr(cp, ':');
		if (tp) {
			if (tp == cp)
				strcpy(file, WHATIS);
			else
				sprintf(file, "%.*s/%s", tp-cp, cp, WHATIS);
			tp++;
		} else
		sprintf(file, "%s/%s", cp, WHATIS);
		if ((fp = fopen(file, "r")) != NULL) {
			isempty = 0;
			while (fgets(buf, (int)(sizeof buf), fp) != NULL) {
				isempty = 1;
				for (vp = argv; *vp; vp++)
					if (match(buf, *vp)) {
						printf("%s", buf);
						gotit[vp - argv] = '1';
						for (vp++; *vp; vp++)
							if (match(buf, *vp))
								gotit[vp - argv] = '1';
						break;
					}
			}
			if (isempty == 0) {
				fprintf(stderr,MSGSTR(ISEMPTY,"%s: database %s is empty\n"),progname,file);
				if (strlen(tp) <= 1)
					exit(4);
			}
			fclose(fp);
		} else {
			fprintf(stderr,MSGSTR(NOTFOUND,"%s: file %s not found\n"),progname,file);
			exit(3);
		}
	}
	for(i=0; i < argc; i++)
		if (gotit[i] != '1') {
			fprintf(stderr, MSGSTR(M_MSG_9, "%s: nothing appropriate\n") , argv[i]);
			errflag++;
		}
	exit(errflag ? 1 : 0);
}

/*****************************************************************************
 ****************************************************************************/
match(register char *bp, char *str)
{
	for (;;) {
		if (*bp == 0)
			return (0);
		if (amatch(bp, str))
			return (1);
		bp++;
	}
}

/*****************************************************************************
 ****************************************************************************/
static amatch(register char *cp, register char *dp)
{
	while (*cp && *dp && lmatch(*cp, *dp))
		dp++, cp++;
	if (*dp == 0)
		return (1);
	return (0);
}

/*****************************************************************************
 ****************************************************************************/
static lmatch(int c, int d)
{
        if (c == d)
                return (1);
        if (!isalpha(c) || !isalpha(d))
                return (0);
        if (islower(c))
                c = toupper(c);
        if (islower(d))
                d = toupper(d);
        return (c == d);
}

/*****************************************************************************
 ****************************************************************************/
static whatis(int argc, char **argv)
{
	register char *gotit, **vp;
	char buf[BUFSIZ], file[BUFSIZ+1], *cp, *tp;
	FILE *fp;
	int i;
        int isempty;

	if (argc == 0) {
		fprintf(stderr, MSGSTR(M_MSG_10, "Missing argument\n"));
		if (fflag)
			usage();
		else
			usage3();
	}
	for (vp = argv; *vp; vp++)
		*vp = trim(*vp);
	gotit = malloc(argc);
	for (cp = manpath; cp; cp = tp) {
		tp = strchr(cp, ':');
		if (tp) {
			if (tp == cp)
				strcpy(file, WHATIS);
			else
				sprintf(file, "%.*s/%s", tp-cp, cp, WHATIS);
			tp++;
		} else
			sprintf(file, "%s/%s", cp, WHATIS);
		if ((fp = fopen(file, "r" )) != NULL) {
			isempty = 0;
			while (fgets(buf, (int)sizeof buf, fp) != NULL) {
				isempty = 1;
				for (vp = argv; *vp; vp++)
					if (wmatch(buf, *vp)) {
						printf("%s", buf);
						gotit[vp - argv] = '1';
						for (vp++; *vp; vp++)
							if (wmatch(buf, *vp))
								gotit[vp - argv] = '1';
                               		        break;
					}
			}
			if (isempty == 0) {
				fprintf(stderr,MSGSTR(ISEMPTY,"%s: database %s is empty\n"),progname,file);
				if (strlen(tp) <= 1)
					exit(4);
			}
			fclose(fp);
		} else {
			fprintf(stderr,MSGSTR(NOTFOUND,"%s: file %s not found\n"),progname,file);
			exit(3);
		}
	}
	for(i=0; i < argc; i++)
		if (gotit[i] != '1') {
			fprintf(stderr, MSGSTR(M_MSG_11, "%s: not found\n") , argv[i]);
			errflag++;
		}
	exit(errflag ? 1 : 0);
}

/*****************************************************************************
 ****************************************************************************/
static wmatch(char *buf, char *str)
{
        register char *bp, *cp;

        bp = buf;
again:
        cp = str;
        while (*bp && *cp && *bp == *cp)
                bp++, cp++;
        if (*cp == 0 && (*bp == '(' || *bp == ',' || *bp == '\t' || *bp == ' '))
                return (1);
        while (isalpha(*bp) || isdigit(*bp))
                bp++;
        if (*bp != ',')
                return (0);
        bp++;
        while (isspace(*bp))
                bp++;
        goto again;
}

/*****************************************************************************
 ****************************************************************************/
trim(register char *cp)
{
	register char *dp;

	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (cp[0] != '.') {
		if (cp + 3 <= dp && dp[-2] == '.' &&
			any(dp[-1], "cosa12345678npP"))
			dp[-2] = 0;
		if (cp + 4 <= dp && dp[-3] == '.' &&
			any(dp[-2], "13") && isalpha((int)dp[-1]))
			dp[-3] = 0;
	}
	return (cp);
}

/*****************************************************************************
 ****************************************************************************/
static usage()
{
	fprintf(stderr,MSGSTR(M_MSG_12,"Usage: man [[[-ct] [section]] | [-f | -k]] [-M path] name...\n"));
	exit(1);
}

static usage2()
{
	fprintf(stderr,MSGSTR(M_MSG_14,"Usage: apropos [-M path] keyword...\n"));
	exit(1);
}

static usage3()
{
	fprintf(stderr,MSGSTR(M_MSG_15,"Usage: whatis [-M path] command...\n"));
	exit(1);
}
