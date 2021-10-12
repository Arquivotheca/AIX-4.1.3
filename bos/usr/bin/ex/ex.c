#if !defined(lint)
static char sccsid [] = "@(#)67  1.11.1.18  src/bos/usr/bin/ex/ex.c, cmdedit, bos41J, 9512A_all 3/15/95 18:06:34";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex.c
 *
 * FUNCTIONS: main, init, tailpath
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 
 * Copyright (c) 1981 Regents of the University of California 
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include <langinfo.h>
#include <locale.h>

nl_catd catd;

#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"

/*
 * The code for ex is divided as follows:
 *
 * ex.c 		Entry point and routines handling interrupt, hangup
 *			signals; initialization code.
 *
 * ex_addr.c		Address parsing routines for command mode decoding.
 *			Routines to set and check address ranges on commands.
 *
 * ex_cmds.c		Command mode command decoding.
 *
 * ex_cmds2.c		Subroutines for command decoding and processing of
 *			file names in the argument list.  Routines to print
 *			messages and reset state when errors occur.
 *
 * ex_cmdsub.c		Subroutines which implement command mode functions
 *			such as append, delete, join.
 *
 * ex_data.c		Initialization of options.
 *
 * ex_get.c		Command mode input routines.
 *
 * ex_io.c		General input/output processing: file i/o, unix
 *			escapes, filtering, source commands, preserving
 *			and recovering.
 *
 * ex_put.c		Terminal driving and optimizing routines for low-level
 *			output (cursor-positioning); output line formatting
 *			routines.
 *
 * ex_re.c		Global commands, substitute, regular expression
 *			compilation and execution.
 *
 * ex_set.c		The set command.
 *
 * ex_subr.c		Loads of miscellaneous subroutines.
 *
 * ex_temp.c		Editor buffer routines for main buffer and also
 *			for named buffers (Q registers if you will.)
 *
 * ex_tty.c		Terminal dependent initializations from termcap
 *			data base, grabbing of tty modes (at beginning
 *			and after escapes).
 *
 * ex_unix.c		Routines for the ! command and its variations.
 *
 * ex_v*.c		Visual/open mode routines... see ex_v.c for a
 *			guide to the overall organization.
 */

/* AIX security enhancement */
#if defined(TVI)
short ivis;
int	auditproc(int, int, int, int);
int	kleenup(int, int, int);
#endif
/* TCSEC Division C Class C2 */

/*
 * Main procedure.  Process arguments and then
 * transfer control to the main command processing loop
 * in the routine commands.  We are entered as either "ex", "edit", "vi"
 * or "view" and the distinction is made here.	Actually, we are "vi" if
 * there is a 'v' in our name, "view" if there is a 'w', and "edit" if
 * there is a 'd' in our name.	For edit we just diddle options;
 * for vi we actually force an early visual command.
 */

extern unsigned nerrors;
extern char closepunct[ONMSZ];

int
main(register int ac, register char *av[])
{
	register int c;
	char *cp;
	char *ptr;
	char *test;

/* AIX security enhancement */
#if !defined (TVI)
	static char buft[PATH_MAX];
	short ivis;
#endif
/* TCSEC Division C Class C2 */

	int maxlines = MAXLINES;
	int maxlines_used = 0;
	short recov = 0;
 	short itag = 0;
	short fast = 0;
	short srcflag = 0;
	char *charstr;
	extern int verbose;

/* AIX security enhancement */
#if defined (TVI)
	/* c2 start-kleenup and suspend auditing */
	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	kleenup (3, 0, 0);
	/* c2 end-kleenup and suspend auditing */
#endif
/* TCSEC Division C Class C2 */

	/*
	 * Immediately grab the tty modes so that we wont
	 * get messed up if an interrupt comes in quickly.
	 */
	gTTY(2);
#if !defined(USG) && !defined(_POSIX_SOURCE)
	normf = tty.sg_flags;
#else
	normf = tty;
#endif
	setlocale(LC_ALL,"");		
	catd = catopen(MF_EX, NL_CAT_LOCALE);
	ppid = getpid();
	exit_status = 0;
	outfd = 1;

	/*
	 * Defend against d's, v's, w's, and a's in directories of
	 * path leading to our true name.
	 */
	av[0] = tailpath(av[0]);

	/*
	 * Figure out how we were invoked: ex, edit, vi, view.
	 */
	ivis = any('v', av[0]); /* "vi" */
	if (any('w', av[0]))	/* "view" */
		value(READONLY) = 1;
	if (any('d', av[0])) {	/* "edit" or "vedit" */
		value(NOVICE) = 1;
		value(REPORT) = 1;
		value(MAGIC) = 0;
		value(SHOWMODE) = 1;
	}

	/*
	 * Open the error message file.
	 */
	draino();
	pstop();

	/*
	 * Initialize interrupt handling.
	 */
	signal(SIGUSR1, SIG_IGN);	
	oldhup = signal(SIGHUP, SIG_IGN);
	if (oldhup == SIG_DFL)
		signal(SIGHUP, onhup);
	oldquit = signal(SIGQUIT, SIG_IGN);
	ruptible = (signal(SIGINT, SIG_IGN) == SIG_DFL);
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
	signal(SIGILL, oncore);
	signal(SIGTRAP, oncore);
	signal(SIGIOT, oncore);
	signal(SIGFPE, oncore);
	signal(SIGBUS, oncore);
	signal(SIGSEGV, oncore);
	signal(SIGPIPE, oncore);

	/*
	 * Process flag arguments.
	 */
	c=0;
	while(c != -1) {
		if (strcmp(av[optind], "-") == 0) {
			hush = 1;
			value(AUTOPRINT) = 0;
			fast++;
			optind++;
		} else if (av[optind][0] == '+') {
			add_pattern(av[optind]+1);
			optind++;
		} else {
#ifdef TRACE
			c = getopt(ac, av, ":Rslrvt:vw:y:c:T:");
#else
			c = getopt(ac, av, ":Rslrvt:vw:y:c:");
#endif
			switch (c) {
				case -1:
					break; /* Done parsing */
				case 'R':
					value(READONLY) = 1;
					break;
				case 's':
					hush = 1;
					value(AUTOPRINT) = 0;
					fast++;
					break;
#ifdef TRACE
				case 'T':
					trace = fopen(optarg, "w");
					if (trace == NULL)
						ex_printf("Trace create error\n");
					else
						setbuf(trace, NULL);
					break;
#endif
				case 'l':
					value(LISP) = 1;
					value(SHOWMATCH) = 1;
					break;
				case 'r':
					recov++;
					break;
				case 'V':
					verbose = 1;
					break;
				case 't':
					/* truncates too long tag. */
					if (mbstowcs(lasttag, optarg, WCSIZE(lasttag)) == -1) {
						fprintf(stderr, MSGSTR(M_650, "Invalid multibyte string, conversion failed.\n"));
						exit(1);
					}
					itag = 1;
					break;
				case 'v':
					ivis = 1;
					break;
				case 'w':
					defwind = (int) strtoul(optarg, &cp, 10);
					if (*cp) {
						fprintf(stderr, MSGSTR(M_305, 
							"The -%c option requires an integer argument.\n"), c);
						exit(1);
					}
					break;
				case 'y':
					maxlines = (int) strtoul(optarg, &cp, 10);
					if (*cp) {
						fprintf(stderr, MSGSTR(M_305, 
							"The -%c option requires an integer argument.\n"), c);
						exit(1);
					}        
					maxlines_used = 1;
					break;
				case 'c':
					add_pattern(optarg);
					break;	
				case ':':
					fprintf(stderr, MSGSTR(M_304,
						"The -%c option requires an argument.\n"), optopt);
					exit(1);
				default:
					fprintf(stderr, MSGSTR(M_000, "Unknown option %s\n"), optopt);
					exit(1);
					break;
			}
		}
	}
	ac-=optind;
	av+=optind;

#ifdef SIGTSTP
# ifdef		TIOCLGET	/* Berkeley 4BSD */
	if (!hush && olttyc.t_suspc != '\377'
	    && signal(SIGTSTP, SIG_IGN) == SIG_DFL){
 		dosusp++;
		signal(SIGTSTP, onsusp);
	}
#endif
# ifdef		_POSIX_JOB_CONTROL
	if (!hush && tty.c_cc[VSUSP] != _POSIX_VDISABLE
	    && signal(SIGTSTP, SIG_IGN) == SIG_DFL){
 		dosusp++;
		signal(SIGTSTP, onsusp);
	}
#endif
#endif
	/*
	 * If we are doing a recover and no filename
	 * was given, then execute an exrecover command with
	 * the -r option to type out the list of saved file names.
	 * Otherwise set the remembered file name to the first argument
	 * file name so the "recover" initial command will find it.
	 */
	if (recov) {
/* AIX security enhancement */
#if defined(TVI)
		smerror(MSGSTR(M_503, "Recover command not allowed"), DUMMY_CHARP);
#else
		if (ac == 0) {
			ppid = 0;
			setrupt();
			execl(EXRECOVER, "exrecover", "-r", 0);
			filioerr(EXRECOVER);
			catclose(catd);
			exit(1);
		}
		strcpy(savedfile, *av++), ac--;
#endif
/* TCSEC Division C Class C2 */
	}

	/*
	 * Initialize the argument list.
	 */
	argv0 = av;
	argc0 = ac;
	args0 = av[0];

	/* Test for Japanese language to provide compatable closing
	punctuation with previous release. This is only valid for ibm932
	codeset  */

	ptr = nl_langinfo(CODESET);
    test = "IBM-932";			/* SHIFT JIS codeset */
    if (strcmp(test, ptr) == 0 )
    {
        int count;

        for(count = 0; closepunct[count] != 0; count++);
        sprintf(&closepunct[count], "%c%c%c%c%c%c%c%c",
            0xa1, 0xa4, 0x81, 0x41, 0x81, 0x42, 0x81, 0x76);
    }

	/*
	 * Initialize a temporary file (buffer) and
	 * set up terminal environment.  Read user startup commands.
	 */
	if (setexit() == 0) {
		setrupt();
		intty = isatty(0);
		value(PROMPT) = intty;
		if (cp = getenv("SHELL"))
			strcpy(shell, cp);
		if (fast)
			setterm("dumb");
		else {
			gettmode();
			cp = getenv("TERM");
			if (cp == NULL || *cp == '\0')
				cp = "unknown";
			setterm(cp);
		}
	}

	erewind();

#ifdef  UNIX_SBRK
	/*
	 * Initialize end of core pointers.
	 * Normally we avoid breaking back to fendcore after each
	 * file since this can be expensive (much core-core copying).
	 * If your system can scatter load processes you could do
	 * this as ed does, saving a little core, but it will probably
	 * not often make much difference.
	 */
	fendcore = (line *) sbrk(0);
	endcore = fendcore - 2;
#else
	/*
	 * Allocate all the memory we will ever use in one chunk.
	 * This is for system such as VMS where sbrk() does not
	 * guarantee that the memory allocated beyond the end is
	 * consecutive.  VMS's RMS does all sorts of memory allocation
	 * and screwed up ex royally because ex assumes that all
	 * memory up to "endcore" belongs to it and RMS has different
	 * ideas.
	 */
	if (maxlines_used) {
		/*
	 	* Initialize end of core pointers.
	 	*/
		if (maxlines < 1024) maxlines = 1024;
		while ((fendcore = (line *)malloc(maxlines * sizeof(line))) == NULL)
			maxlines -= 1024;
		value(LINELIMIT) = maxlines;
	} else {
		fendcore = (line *) malloc((unsigned)
			value(LINELIMIT) * sizeof (line *));
		if (fendcore == NULL) {
			lprintf("ex: cannot handle %d lines", (char *)value(LINELIMIT));
			lprintf("ex: set \"linelimit\" lower", DUMMY_CHARP);
			flush();
			catclose(catd);
			exit(1);
		}
	}
	endcore = fendcore + (value(LINELIMIT) - 1);
#endif
	initaddr();
	if (setexit() == 0 && !fast) {
/* AIX security enhancement */
#if !defined(TVI)
		char *p = getenv ("EXINIT");

		if (p && *p) {
			/* need to count the number of code points */
			char *tmpp;
			int len = 0;
			int charlen;

			tmpp = p;
			while ((charlen = mblen(tmpp, MB_CUR_MAX)) > 0){
				tmpp += charlen;
				len++;
			}
			len += 1;
			if ((globp = (wchar_t *) malloc(sizeof(wchar_t) * len)) == NULL)
			{
				lprintf("ex: Not enough memory space available.", DUMMY_CHARP);
				flush();
				catclose(catd);
				exit(1);
			}
			if (mbstowcs(globp, p, (size_t)len) == -1)
				error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);
			commands((short)1,(short)1);
		}
		else {
			globp = 0;
			if ((cp = getenv("HOME")) != NULL && *cp) {
				strcat(strcpy(buft, cp), "/.exrc");
				/*
				 * If "$HOME/.exrc" and "./.exrc" are the same
				 * file then that file should only be sourced
				 * once.  This is a requirement of POSIX.2
				 */
				if (samefile(buft, ".exrc")) 
					srcflag = 1;
				if (iownit(buft))
					source(buft,(short)1);
			}
		}
		/* If exrc option is set, allow local .exrc too. */
		if (value(EXRC) && iownit(".exrc") && !srcflag) {
			source(".exrc", (short)1);
		}
#else
		trusted_input = !trusted_input;  /* set for use by source() */
		source("/etc/.exrc", (short)1);
		trusted_input = !trusted_input;  /* clear after use */
#endif
/* TCSEC Division C Class C2 */
	}
	/*
	 * This comes after the above chunk of code so that the
	 * DIRECTORY option works.
	 */
	fileinit();

	/*
	 * Initial processing.	Handle tag, recover, and file argument
	 * implied next commands.  If going in as 'vi', then don't do
	 * anything, just set initev so we will do it later (from within
	 * visual).
	 */
	if (setexit() == 0) {
		static wchar_t globbuf[8];	/* fit strlen("recover")+1 */

		if (recov)
		{
		    charstr = "recover";
		    globp = globbuf;
		    if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);
		}
		else if (itag)
		    {
	            charstr = ivis ? "tag" : "tag|p";
		    globp = globbuf;		
		    if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);
		    }
		else if (argc)
		    {
		    charstr = "next";
		    globp = globbuf;
		    if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);
		    }
		/*
		 * If a tag is specified, it must be processed before
		 * firstpat is processed.
		 */
		if (ivis)
			initev = globp;
		else if (globp) {
			inglobal = 1;
			commands((short)1, (short)1);
			inglobal = 0;
		}

		if (ivis && !initev) {
			initev = firstpat;
			firstpat = 0;
		}

		if (firstpat && !ivis) {
			globp = firstpat;
			inglobal = 1;
			commands((short)1, (short)1);
			free(firstpat);
			firstpat = globp = NULL;
			inglobal = 0;
		}
	}

	/*
	 * Vi command... go into visual.
	 * Strange... everything in vi usually happens
	 * before we ever "start".
	 */
	if (ivis) {
		static wchar_t globbuf[7];	/* fit strlen("visual")+1 */

		/* Give the user a chance to see what errors might have
		   occured during startup, such as errors in his or her
		   .exrc file or EXINIT environment variable. */

		if (nerrors) {
			merror(MSGSTR(M_035, "[Hit return to continue] "), DUMMY_INT);
			flush();
#ifndef CBREAK
			vraw();
#endif
			getkey();
			vclrech((short)1);
		}
		/*
		 * Don't have to be upward compatible with stupidity
		 * of starting editing at line $.
		 */
		if (dol > zero)
			dot = one;

		charstr = "visual";
		globp = globbuf;
		if (mbstowcs(globp, charstr, WCSIZE(globbuf)) == -1)
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

		if (setexit() == 0)
			commands((short)1, (short)1);
	}

	/*
	 * Clear out trash in state accumulated by startup,
	 * and then do the main command loop for a normal edit.
	 * If you quit out of a 'vi' command by doing Q or ^\,
	 * you also fall through to here.
	 */
	seenprompt = 1;
	ungetchar(0);
	globp = 0;
	initev = 0;
	setlastchar('\n');
	setexit();
	commands((short)0, (short)0);
	cleanup((short)1);
	catclose(catd);
	exit(exit_status);
	return(0);
/* NOTREACHED */
}

/*
 * Initialization, before editing a new file.
 * Main thing here is to get a new buffer (in fileinit),
 * rest is peripheral state resetting.
 */
void
init(void)
{
	initaddr();
	fileinit();
}

static initaddr(void)
{
	register int i;

	dot = zero = truedol = unddol = dol = fendcore;
	one = zero+1;
	undkind = UNDNONE;
	chng = 0;
	edited = 0;
	for (i = 0; i <= 'z'-'a'+1; i++)
		names[i] = 1;
	anymarks = 0;
}

/*
 * Return last component of unix path name p.
 */
static char *
tailpath(register char *p)
{
	register char *r;

	for (r=p; *p; p++)
		if (*p == '/')
			r = p+1;
	return(r);
}

/*
 * Check ownership of file.
 * Return nonzero if it exists and is owned by the user.
 */
static int
iownit(char *file)
{
	struct stat sb;

	if ((stat(file, &sb) == 0) && (sb.st_uid == geteuid()))
		return(1);
	else
		return(0);
}

/*
 * Check if two files are the same file.
 * Return nonzero if they are the same.
 */
static int
samefile(char *file1, char *file2)
{
	struct stat sb1, sb2;

	if ((stat(file1, &sb1) < 0) || (stat(file2, &sb2) < 0))
		return(0);
	if (sb1.st_dev != sb2.st_dev)
		return(0);
	return (sb1.st_ino == sb2.st_ino);
}

static add_pattern(char *pat)
{
	int len = strlen(pat);

	if (firstpat) {
		firstpat = (wchar_t *) realloc(firstpat,
			(wcslen(firstpat)+2+len) * sizeof(wchar_t));
		if ((mbstowcs((firstpat+wcslen(firstpat)), "\n", 2) == -1) ||
		    (mbstowcs((firstpat+wcslen(firstpat)), pat, len+1) == -1))
			error(MSGSTR(M_650,
			    "Invalid multibyte string, conversion failed."),
			    DUMMY_INT);
	} else {
		firstpat = (wchar_t *) malloc((len+1) * sizeof(wchar_t));
		if (mbstowcs(firstpat, pat, len+1) == -1)
			error(MSGSTR(M_650,
			    "Invalid multibyte string, conversion failed."),
			    DUMMY_INT);
	}
}

