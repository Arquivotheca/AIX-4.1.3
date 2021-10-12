static char sccsid[] = "@(#)55	1.37  src/bos/usr/bin/pg/pg.c, cmdscan, bos41B, 412_41B_sync 1/4/95 15:19:01";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * pg.c	1.11  com/cmd/scan,3.1,9013 3/12/90 13:51:59";
 *
 *      pg -- paginator for crt terminals
 *
 *   The pg  command reads files  and writes them  to standard
 *   output one  screen at a time.   If you specify file  as -
 *   (minus) or  run pg  without arguments, pg  reads standard
 *   input.   Each screen  is followed  by a  prompt.  If  you
 *   press the Enter  key, another page is  displayed.  The pg
 *   command lets  you back  up to  review something  that has
 *   already passed.
 *
 *   Includes the ability to display pages that have
 *   already passed by. Also gives the user the ability
 *   to search forward and backwards for regular expressions.
 *   This works for piped input by copying to a temporary file,
 *   and resolving back references from there.
 *
 *	Note:	The reason that there are so many commands to do
 *		the same types of things is to try to accommodate
 *		users of other paginators.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 *
 */
#define _ILS_MACROS

#include <string.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <nl_types.h>
#include <signal.h>
#include <setjmp.h>
#include <wchar.h>
#include <ctype.h>
#include <stdio.h>
#include <curses.h>
#include <errno.h>
#include <term.h>
#include <termios.h>
#include <locale.h>
#include <stdlib.h>
#include "pg_msg.h"

static nl_catd	catd;
#define MSGS(n,s)	catgets(catd,MS_PG,n,s)
#define LINSIZ	1024
#define QUIT	'\034'
#define BOF	(EOF - 1)	/* Beginning of File */
#define STOP    (EOF - 2)
#define PROMPTSIZE	256

struct line {			/* how line addresses are stored */
	long	l_addr;		/* file offset */
	int	l_no;		/* line number in file */
};

typedef	struct line	LINE;
static LINE		*zero = NULL,		/* first line */
		*dot,		/* current line */
		*dol,		/* last line */
		*contig;	/* where contiguous (non-aged) lines start */
static int	        nlall;          /* room for how many LINEs in memory */
 
static FILE		*in_file = NULL;	/* current input stream */
static FILE		*tmp_fin = NULL;	/* pipe temporary file in */
static FILE		*tmp_fou = NULL;	/* pipe temporary file out */
static char		tmp_name[] = "/var/tmp/pgXXXXXX";

static short		sign;		/* sign of command input */

static int             fnum,           /* which file argument we're in */
		pipe_in,	/* set when stdin is a pipe */
		out_is_tty;	/* set if stdout is a tty */

static void	help(),
	newdol(),
	save_pipe(),
	help(),
	copy_file(),
	save_input(),
	lineset();
static char	readch ();
static int		on_brk(int),
		chgwinsz(int),
		end_it(void),
		addmax(int, int);
static short		brk_hit;	/* interrupt handling is pending flag */

static int             window = 0;	/* window size in lines */
static short		win_sz_set = 0; /* window size set by the user */  
static short		eof_pause = 1;	/* pause w/ prompt at end of files */
static short		soflag = 0;	/* output all messages in standout mode */
static short           promptlen;      /* length of the current prompt */
static short           firstf = 1;	/* set before first file has been processed */
static short           inwait,		/* set while waiting for user input */
		errors;         /* set if error message has been printed.
				 * if so, need to erase it and prompt */

static char		**fnames;
static char		*defaultfile="-";
static short		fflag = 0;	/* set if the f option is used */
static short		nflag = 0;	/* set for "no newline" input option */
static short		clropt = 0;	/* set if the clear option is used */
static int		initopt = 0;	/* set if the line option is used */
static int		srchopt = 0;	/* set if the search option is used */
static int		initline;
static char		initbuf[BUFSIZ];
static unsigned char	lastpattern[BUFSIZ];
static char            leave_search = 't';     /* where on the page to leave a found string */
static short           nfiles;
static char		*shell;
static char		*promptstr = ":";
static char		*setprompt();
static short		lenprompt;		/* length of prompt string */
static int		nchars;			/* return from getline in find() */
static jmp_buf		restore;
static char		msgbuf[BUFSIZ];
static unsigned char	Line[LINSIZ+2];
static int		catch_susp = 0;			/* has SIGTSTP been caught? */


struct screen_stat {
	int	first_line;
	int	last_line;
	short	is_eof;
	};
static struct screen_stat old_ss = { 0, 0, 0 };
static struct screen_stat new_ss;

static short		eoflag;		/* set whenever at end of current file */
static short		doliseof;	/* set when last line of file is known */
static int		eofl_no;	/* what the last line of the file is */
static int		exit_val;	/* is 1 if any errors occurred otherwise 0 */

#define USAGE() { fprintf(stderr,MSGS(USGE,"Usage: pg [-number] [-p string] [-cefns] [+line] [+/pattern/] [file...]\n")); exit(1); }	


main(argc, argv)
int argc;
char *argv[];
{
	register char	*s;
	register char	*p;
	register char	ch;
	int		prnames = 0; 
	FILE		*checkf();

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_PG,NL_CAT_LOCALE);

	out_is_tty = isatty(1);  	/* P34397 */ /*A2019*/

	fnum = 0;
	nfiles = argc;
	fnames = argv;
	while (--nfiles > 0) {
		if ((ch = (*++fnames)[0]) == '-') {
			if (fnames[0][1] == '\0' )
				break;
			if ((fnames[0][1] == '-') && (fnames[0][2] == '\0')) {
				nfiles--;
				fnames++;
				break;
			}
			for (s = fnames[0]+1; *s != '\0'; s++)
				if (isdigit((int)*s)) {
					window = 0;
					do {
						window = window*10+*s-'0';
						s++;
					} while (isdigit((int)*s));
					if (*s != '\0')
						USAGE();
					break;
				}
				else if (*s == 'c')
					clropt = 1;
				else if (*s == 'e')
					eof_pause = 0;
				else if (*s == 'f')
					fflag = 1;
				else if (*s == 'n')
					nflag = 1;
				else if (*s == 's')
					soflag = 1;	/* standout mode */
				else if (*s == 'p') {
					if (*++s != '\0')
						promptstr = setprompt(s);
					else if (nfiles > 1) {
						promptstr = setprompt(*++fnames);
						nfiles--;
					}
					else
						USAGE();
					break;
				}
				else
					USAGE();

		}
		else if (ch == '+') {
			s = *fnames;
			if (*++s == '/') {
				srchopt++;
				initopt = 0;
				for (++s, p=initbuf; *s!='\0' && *s!='/';)
					if (p < initbuf + sizeof(initbuf))
						*p++ = *s++;
					else {
						fprintf(stderr,MSGS(PATLONG,"pg: pattern too long\n"));	
						exit(1);
					}
				*p = '\0';
			}
			else {
				initopt++;
				srchopt = 0;
				for (; isdigit((int)*s); s++)
					initline = initline*10 + *s -'0';
				if (*s != '\0')
					USAGE();
			}
		}
		else
			break;
	}
	signal(SIGQUIT,(void (*)(int))end_it);
	signal(SIGINT,(void (*)(int))end_it);
	if (out_is_tty) {
		terminit();
		signal(SIGQUIT,(void (*)(int))on_brk);

		signal(SIGINT,(void (*)(int))on_brk);
	}
	if (window == 0)
		window = lines - 1;
	lenprompt = strlen(promptstr);
	if (window < 1)
		window = 1;
	/* If (window == INT_MAX) then overflow errors will occur. */
	if (window == INT_MAX)
		window = INT_MAX-1;
	if (initline <= 0)
		initline = 1;
	if (nfiles > 1)
		prnames++;

	if (nfiles == 0) {
		fnames = &defaultfile;
		nfiles++;
	}
	while (fnum < nfiles) {
		signal(SIGWINCH,(void (*)(int))chgwinsz);
		if (strcmp(fnames[fnum],"") == 0)
			fnames[fnum] = "-";
		if ((in_file = checkf(fnames[fnum])) == NULL)
			fnum++;
		else {
			if (out_is_tty)
				fnum += screen(fnames[fnum]);
			else {
				if (prnames) {
					pr("::::::::::::::\n");
					pr(fnames[fnum]);
					pr("\n::::::::::::::\n");
				}
				copy_file(in_file,stdout);
				fnum++;
			}
			fflush(stdout);
			if (pipe_in)
				save_pipe();
			else
			if (in_file != tmp_fin)
				fclose(in_file);
		}
	}
	end_it();
}

/*
 * NAME: setprompt
 *                                                                    
 * FUNCTION: 	Set the prompt corresponding to the -p option.  %d is
 *		the current page number.
 *                                                                    
 * RETURN VALUE DESCRIPTION:  A pointer to the string prompt is returned.
 *			    
 */  

static char *
setprompt(s)
register char *s;
{
	register int i = 0;
	register int pct_d = 0;
	static char pstr[PROMPTSIZE];

	while (i < PROMPTSIZE - 2)
		switch(pstr[i++] = *s++) {
		case '\0':
			return(pstr);
		case '%':
			if (*s == 'd' && !pct_d) {
				pct_d++;
			}
			else if (*s != '%')
				pstr[i++] = '%';
			if ((pstr[i++] = *s++) == '\0')
				return(pstr);
			break;
		default:
			break;
		}
	fprintf(stderr,MSGS(PRMPTLNG,"pg: prompt too long\n"));
	exit(1);
}


/*
 * NAME: screen
 *                                                                    
 * FUNCTION: Print out the contents of the file f, one screenful at a time.
 */  

static int
screen(file_name)
char *file_name;
{
	int cmd_ret = 0;
	int start;
	short hadchance = 0;

	old_ss.is_eof = 0;
	old_ss.first_line = 0;
	old_ss.last_line = 0;
	new_ss = old_ss;

	signal(SIGWINCH,(void (*)(int))chgwinsz);
	if (!firstf)
		cmd_ret = command(file_name);
	else {
		firstf = 0;
		if (initopt) {
			initopt = 0;
			new_ss.first_line = initline;
			new_ss.last_line = addmax(initline, window - 1);
		}
		else
		if (srchopt) {
			srchopt = 0;
			if (!search(initbuf,1))
				cmd_ret = command(file_name);
		}
		else {
			new_ss.first_line = 1;
			new_ss.last_line = window;
		}
	}

	for (;;) {
	 	signal(SIGWINCH,(void (*)(int))chgwinsz);
		if (cmd_ret)
			return(cmd_ret);
		if (hadchance && new_ss.first_line >= eofl_no - 1)
			return(1);
		hadchance = 0;

		if (new_ss.last_line < window)
			new_ss.last_line = window;
		if (find(0,new_ss.last_line + 1) != EOF)
			new_ss.is_eof = 0;
		else {
			new_ss.is_eof = 1;
			new_ss.last_line = eofl_no - 1;
			new_ss.first_line = new_ss.last_line - window + 1;
		}

		if (new_ss.first_line < 1)
			new_ss.first_line = 1;
		if (clropt) {
			doclear();
			start = new_ss.first_line;
		}
		else {
			if (new_ss.first_line == old_ss.last_line)
				start = new_ss.first_line + 1;
			else
			if (new_ss.first_line > old_ss.last_line)
				start = new_ss.first_line;
			else
			if (old_ss.first_line < new_ss.first_line)
				start = old_ss.last_line + 1;
			else
				start = new_ss.first_line;

			if (start < old_ss.first_line)
				sopr(MSGS(SKIPBW,"...skipping backward\n"),0);
			else
			if (start > old_ss.last_line + 1)
				sopr(MSGS(SKIPFW,"...skipping forward\n"),0);
		}

		for(; start <= new_ss.last_line; start++) {
			find(0,start);
			pr(Line);
			if (brk_hit) {
				new_ss.last_line = find(1,0);
				new_ss.is_eof = 0;
				break;
			}
		}

		brk_hit = 0;
		fflush(stdout);
		if (new_ss.is_eof) {
			if (!eof_pause || eofl_no == 1)
				return(1);
			hadchance++;
			error(MSGS(SAYEOF, "(EOF)"));
		}
		old_ss = new_ss;
		cmd_ret = command((char *)NULL);
	}
}

static char	cmdbuf[LINSIZ], *cmdptr;
#define BEEP()		if (bell) { putp(bell); fflush(stdout); }
#define	BLANKS(p)	while (*p == ' ' || *p == '\t') p++
#define CHECKEND()	BLANKS(cmdptr); if (*cmdptr) { BEEP(); break; }

/*
 * NAME: command
 *                                                                    
 * FUNCTION: 
 * 	Read a command and do it. A command consists of an optional integer
 * 	argument followed by the command character.
 
 *                                                                    
 * RETURN VALUE: 
 *		Return the number of files to skip, 0 if
 *		we're still talking about the same file.
 */  

static command (filename)
char *filename;
{
	register int nlines;
	register char c;
	FILE *sf;
	char *cmdend;
	int id;
	int skip;

	for (;;) {
		/* Wait for output to drain before going on.     */
		/* This is done so that the user will not hit    */
		/* break and quit before he has seen the prompt. */
		ioctl(1,TCSBRK,1); 
		if (setjmp(restore) != 0)
			end_it();
		inwait = 1;
		brk_hit = 0;
		if (errors)
			errors = 0;
		else {
			kill_line();
			prompt(filename);
		}
	 	signal(SIGWINCH,(void (*)(int))chgwinsz);
		fflush(stdout);
		if (ttyin())
			continue;
		cmdptr = cmdbuf;
	 	signal(SIGWINCH,(void (*)(int))chgwinsz);
		nlines = number();
		BLANKS(cmdptr);
		switch (*cmdptr++) {
		case 'h':
			CHECKEND();
			help();
			break;
		case '\014': /* ^L */
		case '.':       /* redisplay current window */
			CHECKEND();
			new_ss.first_line = old_ss.first_line;
			new_ss.last_line = addmax(new_ss.first_line, window - 1);
			inwait = 0;
			return(0);
		case 'w':       /* set window size */
		case 'z':
			if (sign == -1) {
				BEEP();
				break;
			}
			if (!win_sz_set)
				win_sz_set++;
			CHECKEND();
			if (nlines == 0)
				nlines = window;
			else if (nlines > 0) {
				window = nlines;
				/*
				 * If (window == INT_MAX) then
				 * overflow errors will occur.
				 */
				if (window == INT_MAX)
					window = INT_MAX-1;
			} else {
				BEEP();
				break;
			}
			new_ss.first_line = old_ss.last_line;
			new_ss.last_line = addmax(new_ss.first_line, window - 1);
			inwait = 0;
			return(0);
		case '\004': /* ^D */
		case 'd':
			CHECKEND();
			if (sign == 0)
				sign = 1;
			new_ss.last_line = old_ss.last_line + sign*window/2;
			new_ss.first_line = new_ss.last_line - window + 1;
			inwait = 0;
			return(0);
		case 's':
			/*
			* save input in filename.
			* Check for filename, access, etc.
			*/
			BLANKS(cmdptr);
			if (!*cmdptr) {
				BEEP();
				break;
			}
			if ((sf=fopen(cmdptr,"w")) == NULL) { /* def 76331 */
				error(MSGS(OPSAVERR,"cannot open save file"));
				break;
			}
			if (setjmp(restore) != 0) {
				BEEP();
			}
			else {
				kill_line();
				sprintf(msgbuf, MSGS(SAVFIL,"saving file %s"),
					cmdptr);
				sopr(msgbuf,1);
				fflush(stdout);
				save_input(sf);
				error(MSGS(SAVED,"saved"));
			}
			fclose(sf);
			break;
		case 'q':
		case 'Q':
			CHECKEND();
			inwait = 0;
			end_it();
		case 'f':       /* skip forward screenfuls */
			CHECKEND();
			if (sign == 0)
				sign++;	/* skips are always relative */
			if (nlines == 0)
				nlines++;
			nlines = nlines * (window - 1);
			if (sign == 1)
				new_ss.first_line = old_ss.last_line + nlines;
			else
				new_ss.first_line = old_ss.first_line - nlines;
			new_ss.last_line = addmax(new_ss.first_line, window - 1);
			inwait = 0;
			return(0);
		case 'l':       /* get a line */
			CHECKEND();
			if (nlines == 0) {
				nlines++;
				if (sign == 0)
					sign = 1;
			}
			switch(sign){
			case 1:
				new_ss.last_line = old_ss.last_line + nlines;
				new_ss.first_line = new_ss.last_line - window + 1;
				break;
			case 0:  /* leave addressed line at top */
				new_ss.first_line = nlines;
				new_ss.last_line = addmax(nlines, window - 1);
				break;
			case -1:
				new_ss.first_line = old_ss.first_line - nlines;
				new_ss.last_line = addmax(new_ss.first_line, window - 1);
				break;
			}
			inwait = 0;
			return(0);
		case '\0': /* \n or blank */
			if (nlines == 0){
				nlines++;
				if (sign == 0)
					sign = 1;
			}
			nlines = (nlines - 1) * (window - 1);
			switch(sign) {
			case 1:
				new_ss.first_line = old_ss.last_line + nlines;
				new_ss.last_line = addmax(new_ss.first_line, window - 1);
				break;
			case 0:
				new_ss.first_line = nlines + 1;
				new_ss.last_line = addmax(nlines, window);
				break;
			case -1:
				new_ss.last_line = old_ss.first_line - nlines;
				new_ss.first_line = new_ss.last_line - window + 1;
				break;
			}
			inwait = 0;
			return(0);
		case 'n':       /* switch to next file in arglist */
			CHECKEND();
			if (sign == 0)
				sign = 1;
			if (nlines == 0)
				nlines++;
			if ((skip = skipf(sign *nlines)) == 0) {
				BEEP();
				break;
			}
			inwait = 0;
			return(skip);
		case 'p':       /* switch to previous file in arglist */
			CHECKEND();
			if (sign == 0)
				sign = 1;
			if (nlines == 0)
				nlines++;
			if ((skip = skipf(-sign * nlines)) == 0) {
				BEEP();
				break;
			}
			inwait = 0;
			return(skip);
		case '$':       /* go to end of file */
			CHECKEND();
			sign = 1;
			while(find(1,10000) != EOF)
				/* any large number will do */;
			new_ss.last_line = eofl_no - 1;
			new_ss.first_line = eofl_no - window;
			inwait = 0;
			return(0);
		case '/':       /* search forward for r.e. */
		case '?':       /*   "  backwards */
		case '^':	/* this ones a ? for regent100s */
			if(sign < 0) {
				BEEP();
				break;
			}
			if (nlines == 0)
				nlines++;
			cmdptr--;
			cmdend = cmdptr + (strlen(cmdptr) - 1);
			if ( (cmdend > cmdptr + 1)
				&& (*cmdptr ==  *(cmdend - 1))
				&& ( ((c = *cmdend) == 't')
					|| (c == 'm')
					|| (c == 'b') ) ) {
				leave_search = c;
				cmdend--;
			}
			if ((cmdptr < cmdend) && (*cmdptr == *cmdend))
				*cmdend = '\0';
			if (*cmdptr != '/')  /* signify back search by - */
				nlines = -nlines;
			if (!search(++cmdptr, nlines))
				break;
			else {
				inwait = 0;
				return(0);
			}
		case '!':       /* shell escape */
			if (!hard_copy) { /* redisplay the command */
				pr(cmdbuf);
				pr("\n");
			}
			if ((id = fork ()) < 0) {
				error(MSGS(CANTFORK,"cannot fork, try again later"));
				break;
			}
			if (id == 0) {
				/*
				* if stdin is a pipe, need to close it so
				* that the terminal is really stdin for
				* the command
				*/
				if (catch_susp)
					signal(SIGTSTP, SIG_DFL);
				fclose(stdin);
				dup(fileno(stdout));
				execl(shell, shell, "-c", cmdptr, 0);
				fprintf(stderr,MSGS(EXECFAIL,"exec failed\n"));
				exit(1);
			}
			signal (SIGINT, SIG_IGN);
			signal (SIGQUIT, SIG_IGN);
			wait ((int *) 0);
			pr("!\n");
			fflush(stdout);
			signal(SIGINT,(void (*)(int))on_brk);
			signal(SIGQUIT,(void (*)(int))on_brk);
			break;
		default:
			BEEP();
			break;
		}
	}
}

/*
 * NAME: number
 *                                                                    
 * FUNCTION:	return the number in the command line and the option plus
 *		or minus value.
 *                                                                    
 * RETURN VALUE:  number of lines.
 */  

static number()
{
	register int i;
	register char *p;

	i = 0;
	sign = 0;
	p = cmdptr;
	BLANKS(p);
	if (*p == '+') {
		p++;
		sign = 1;
	}
	else
	if (*p == '-') {
		p++;
		sign = -1;
	}
	while (isdigit((int)*p))
		i = i * 10 + *p++ - '0';
	cmdptr = p;
	return(i);
}

/*
 * NAME: ttyin
 *                                                                    
 * FUNCTION:  read a line of input.
 */  

static ttyin ()
{
	register char *sptr;
	register char ch;
	register int slash = 0;
	int state = 0;

	fixterm();
	set_state(&state,' ');	/* initialize state processing */
	sptr = cmdbuf;
	while(state != 10) {
		ch = readch();
		if (ch == '\n' && !slash)
			break;
		if (ch == erasechar() && !slash) {
			if (sptr > cmdbuf) {
				--promptlen;
				pr("\b \b");
				--sptr;
				if (*sptr < ' ') {
					--promptlen;
					pr("\b \b");
				}
			}
			set_state(&state,ch,sptr);
			fflush(stdout);
			continue;
		}
		else
		if (ch == killchar() && !slash) {
			if (hard_copy)
				putchar(ch);
			resetterm();
			return(1);
		}
		if (slash) {
			slash = 0;
			pr("\b \b");
			sptr--;
			promptlen--;
		}
		else /* is there room to keep this character? */
		if (sptr>=cmdbuf + sizeof(cmdbuf) || promptlen + 2 >= columns) {
			BEEP();
			continue;
		}
		else
		if (ch == '\\')
			slash++;
		if (set_state(&state,ch,sptr) == 0) {
			BEEP();
			continue;
		}
		*sptr++ = ch;
		if (ch < ' ') {
			ch += 0100;
			putchar('^');
			promptlen++;
		}
		putchar(ch);
		promptlen++;
		fflush(stdout);
	}

	*sptr = '\0';
	kill_line();
	fflush(stdout);
	resetterm();
	return(0);
}

/*
 * NAME: set_state
 *                                                                    
 * FUNCTION:  	Set the state of the command line.  Whether incoming command
 *		is a positive number, continuation....
 *                                                                    
 * RETURN VALUE:  return 1 if no errors
 */  

static set_state(pstate,c,pc)
register int *pstate;
register int c;
register char *pc;
{
	static char *psign;
	static char *pnumber;
	static char *pcommand;
	static int slash;

	if (*pstate == 0) {
		psign = NULL;
		pnumber = NULL;
		pcommand = NULL;
		*pstate = 1;
		slash = 0;
		return(1);
	}
	if (c == '\\' && !slash) {
		slash++;
		return(1);
	}
	if (c == erasechar() && !slash)
		switch(*pstate) {
		case 4:
			if (pc > pcommand)
				return(1);
			pcommand = NULL;
		case 3:
			if (pnumber && pc > pnumber) {
				*pstate = 3;
				return(1);
			}
			pnumber = NULL;
		case 2:
			if (psign && pc > psign) {
				*pstate = 2;
				return(1);
			}
			psign = NULL;
		case 1:
			*pstate = 1;
			return(1);
		}

	slash = 0;
	switch(*pstate) {
	case 1: /* before receiving anything interesting */
		if (c == '\t' || (!nflag && c == ' '))
			return(1);
		if (c == '+' || c == '-') {
			psign = pc;
			*pstate = 2;
			return(1);
		}
	case 2: /* received sign, waiting for digit */
		if (isdigit(c)) {
			pnumber = pc;
			*pstate = 3;
			return(1);
		}
	case 3: /* received digit, waiting for the rest of the number */
		if (isdigit(c))
			return(1);
		if (strchr("h\014.wz\004dqQfl np$",c)) {
			pcommand = pc;
			if (nflag)
				*pstate = 10;
			else
				*pstate = 4;
			return(1);
		}
		if (strchr("s/^?!",c)) {
			pcommand = pc;
			*pstate = 4;
			return(1);
		}
		return(0);
	case 4:
		return(1);
	}
}

/*
 * NAME: readch
 *                                                                    
 * FUNCTION:  Input a character.
 *
 * RETURN VALUE:  character
 */  

static char
readch()
{
	char ch;
	static int wasintrd = 0;
	register int rc;

	/*
	 * if we was interrupted before, force a newline...
	 */
	if (wasintrd) {
		wasintrd = 0;
		return ('\n');
		}

	/*
	 * if we're returning from an interrupt (either SIGTSTP or
	 * SIGWINCH), force a redraw...
	 * this becomes tough in the !nflag case, 'cause we also
	 * have to force a newline (see above).
	 *
	 *
	 * yeah, I know it's strange...
	 */
	rc = read (fileno(stdout), &ch, 1);
	if (rc == 0 || rc == -1) {
	    switch(errno) {
              case (EINTR):
	        if (!nflag)
		    wasintrd = 1;
		ch = '\014';
		break;
	      case (ENXIO):
	      case (EIO):
	      case (EBADF):
		end_it(); /* lost the terminal suddenly */
	    }
	}
	return (ch);
}

/*
 * NAME: help
 *                                                                    
 * FUNCTION: Print out a help screen.
 *                                                                    
 * RETURN VALUE: void
 */  

static void
help()
{
	if (clropt)
		doclear();

     pr(MSGS(H01,"-------------------------------------------------------\n"));
     pr(MSGS(H02,"   h               help\n"));
     pr(MSGS(H03,"   q or Q          quit\n"));
     pr(MSGS(H04,"   <blank> or \\n   next page\n"));
     pr(MSGS(H05,"   l               next line\n"));
     pr(MSGS(H06,"   d or ^D         display half a page more\n"));
     pr(MSGS(H07,"   . or ^L         redisplay current page\n"));
     pr(MSGS(H08,"   f               skip the next page forward\n"));
     pr(MSGS(H09,"   n               next file\n"));
     pr(MSGS(H11,"   p               previous file\n"));
     pr(MSGS(H12,"   $               last page\n"));
     pr(MSGS(H13,"   w or z          set window size and display next page\n"));
     pr(MSGS(H14,"   s savefile      save current file in savefile\n"));
     pr(MSGS(H15,"   /pattern/       search forward for pattern\n"));
     pr(MSGS(H16,"   ?pattern? or\n"));
     pr(MSGS(H17,"   ^pattern^       search backward for pattern\n"));
     pr(MSGS(H18,"   !command         execute command\n"));
     pr(MSGS(H19,"\n"));
     pr(MSGS(H20,"Most commands can be preceeded by a number, as in:\n"));
     pr(MSGS(H21,"+1\\n (next page); -1\\n (previous page); 1\\n (page 1).\n"));
     pr(MSGS(H22,"\n"));
     pr(MSGS(H23,"See the manual page for more detail.\n"));
     pr(MSGS(H24,"-------------------------------------------------------\n"));
}

/*
 * NAME: nskip
 *                                                                    
 * FUNCTION: 
 * 		Skip nskip files in the file list (from the command line).
 *		Nskip may be negative.
 *
 * RETURN VALUE:  The number of files skipped.
 */  

static skipf (nskip)
register int nskip;
{
	if (fnum + nskip < 0) {
		nskip = -fnum;
		if (nskip == 0)
			error(MSGS(NOPREV,"No previous file"));	
	}
	else
	if (fnum + nskip > nfiles - 1) {
		nskip = (nfiles - 1) - fnum;
		if (nskip == 0)
			error(MSGS(NONEXT,"No next file"));
	}
	return(nskip);
}

/*
 * NAME: checkf
 *                                                                    
 * FUNCTION: 
 * 	Check whether the file named by fs is a file which the user may
 * 	access.  
 *
 * RETURN VALUE: 	If it is, return the opened file.
 *			Otherwise return NULL. 
 */  


static FILE *
checkf (fs)
register char *fs;
{
	struct stat stbuf;
	register FILE *f;
	int fd;

	pipe_in = 0;
	if (strcmp(fs,"-") == 0) {
		if (tmp_fin == NULL)
			f = stdin;
		else {
			rewind(tmp_fin);
			f = tmp_fin;
		}
	}
	else {
		if ((f=fopen(fs, "r")) == NULL) {
			fflush(stdout);
			perror(fs);
			exit_val = 1;
			return (NULL);
		}
	}
	if (fstat((int)fileno(f), &stbuf) == -1) {
		fflush(stdout);
		perror(fs);
		exit_val = 1;
		return (NULL);
	}
	if (stbuf.st_mode & S_IFDIR) {
		fprintf(stderr,MSGS(DIRECT,"pg: %s is a directory\n"),fs);
		exit_val = 1;
		return (NULL);
	}
	if (stbuf.st_mode & S_IFREG) {
		if (f == stdin)		/* It may have been read from */
			rewind(f);	/* already, and not reopened  */
	}
	else {
		if (f != stdin) {
			fprintf(stderr,MSGS(SPECFIL,"pg: special files only handled as standard input\n"));
			exit_val = 1;
			return(NULL);
		}
		else {
			mktemp(tmp_name);
			if ((fd=creat(tmp_name,0600)) < 0) {
			    fprintf(stderr,MSGS(NOTEMP,"pg: Can't create temp file\n"));
		   	    exit_val = 1;
			    return(NULL);
			}			
			close(fd);
			if ((tmp_fou = fopen(tmp_name, "w")) == NULL) {
				exit_val = 1;
				fprintf(stderr,MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"));
				return(NULL);
			}
			if ((tmp_fin = fopen(tmp_name, "r")) == NULL) {
				exit_val = 1;
				fprintf(stderr,MSGS(TEMPERRR,"pg: Can't get temp file for reading\n"));
				return(NULL);
			}
			pipe_in = 1;
			/* fflag = 1; */ /* no folding */
		}
	}
	lineset(BOF);
	return(f);
}

/*
 * NAME: copy_file
 *                                                                    
 * FUNCTION:  Copy the file to the output file so we can scan back and forth.
 *                                                                    
 * RETURN VALUE:  void
 */  

static void
copy_file(f, out)
register FILE *f, *out;
{
	register int c;

	while ((c = getc(f)) != EOF)
		putc(c,out);
}

#include <regex.h>
static re_error(i)
int i;
{
	int j;
	static struct messages {
		char *message;
		int number;
		} re_errmsg[] = {
		"Pattern not found",				1,
		"Range endpoint too large",			11,
		"Bad number",					16,
		"`\\digit' out of range",			25,
		"Illegal or missing delimeter",			36,
		"No remembered search string",  		41,
		"\\( \\) imbalance",				42,
		"Too many \\(",					43,
		"More than two numbers given in \\{ \\}",	44,
		"} expected after \\",				45,
		"First number exceeds second in \\{ \\}",	46,
		"Invalid endpoint in range",			48,
		"[] imbalance",					49,
		"Regular expression overflow",			50,
		"Bad regular expression",		 	0
		};

	for (j = 0; re_errmsg[j].number != 0; j++ )
		if (re_errmsg[j].number == i )
			break;
	if (re_errmsg[j].number == 0)
		error(MSGS(BADREG,"Bad Regular Expression"));
	else
		error(catgets(catd,MS_PG,i,re_errmsg[j].message));
	longjmp(restore,1);  /* restore to search() */
}

/*
 * NAME: search
 *                                                                    
 * FUNCTION: 
 * Search for nth ocurrence of regular expression contained in buf in the file
 *	negative n implies backward search
 *	n 'guaranteed' non-zero
 *                                                                    
 * RETURN VALUE: 1 if pattern found else 0
 */  

static regex_t re;

static search (ibuf, n)
unsigned char ibuf[];
register int n;
{
	register int direction;
	unsigned char *endbuf;
	unsigned char *buf;
	int END_COND;
	int stat;

	if (*ibuf == NULL) {
		buf = lastpattern;
	} else {
		buf = ibuf;
		strncpy(lastpattern, ibuf, BUFSIZ);
	}
	endbuf = buf + strlen(buf)-2;
	if(*endbuf++ != '\\' && *endbuf == '$') {
		*endbuf++ = '\\';
		*endbuf++ = 'n';
		*endbuf = '\0';
		}
	if (setjmp(restore) == 0) {
		if ( regcomp( &re, buf, 0) != 0) {
			exit_val = 1;
			perror("pg: regcomp");
			}

		if (n < 0) {	/* search back */
			direction = -1;
			find(0,old_ss.first_line);
			END_COND = BOF;
		}
		else {
			direction = 1;
			find(0,old_ss.last_line);
			END_COND = EOF;
		}

		while (find(1,direction) != END_COND){
			if (brk_hit)
				break;
			if ( regexec(&re, Line, (size_t) 0, (regmatch_t *) NULL, 0) == 0)
				if ((n -= direction) == 0) {
					switch(leave_search) {
					case 't':
						new_ss.first_line = find(1,0);
						new_ss.last_line = addmax(new_ss.first_line, window - 1);
						break;
					case 'b':
						new_ss.last_line = find(1,0);
						new_ss.first_line = new_ss.last_line - window + 1;
						break;
					case 'm':
						new_ss.first_line =
find(1,0) - (window - 1)/2;
						new_ss.last_line = addmax(new_ss.first_line, window - 1);
						break;
					}
					return(1);
				}
		}
		re_error(1); /* Pattern not found */
	}
	BEEP();
	return(0);
}

/*
 *	find -- find line in file f, subject to certain constraints.
 *
 *	This is the reason for all the funny stuff with sign and nlines.
 *	We need to be able to differentiate between relative and abosolute
 *	address specifications. 
 *
 *	So...there are basically three cases that this routine
 *	handles. Either line is zero, which  means there is to be
 *	no motion (because line numbers start at one), or
 *	'how' and 'line' specify a number, or line itself is negative,
 *	which is the same as having how == -1 and line == abs(line).
 *
 *	Then, figure where exactly it is that we are going (an absolute
 *	line number). Find out if it is within what we have read,
 *	if so, go there without further ado. Otherwise, do some
 *	magic to get there, saving all the intervening lines,
 *	in case the user wants to see them some time later.
 *
 *	In any case, return the line number that we end up at. 
 *	(This is used by search() and screen()). If we go past EOF,
 *	return EOF.
 *	This EOF will go away eventually, as pg is expanded to
 *	handle multiple files as one huge one. Then EOF will
 *	mean we have run off the file list.
 *	If the requested line number is too far back, return BOF.
 */
static find(how,line)	/* find the line and seek there */
short how;
int line;	/* changed 2-2-88 p28962 */
{
	/* no compacted memory yet */
	register FILE *f = in_file;
 	register int where; /* changed 2-2-88 p28962 */

	if (how == 0)
		where = line;
	else
		if (dot == zero - 1)
			where = how * line;
		else
			where = how * line + dot->l_no;

	/* now, where is either at, before, or after dol */
	/* most likely case is after, so do it first */

	eoflag = 0;
	if (where >= dol->l_no) {
		if (doliseof) {
			dot = dol;
			eoflag++;
			return(EOF);
		}
		if (pipe_in)
			in_file = f = stdin;
		else
			fseek(f, dol->l_addr, 0);
		dot = dol - 1;
		while ((nchars = getline(f)) != EOF) {
			dot++;
			newdol(f);
			if ( where == dot->l_no || brk_hit)
				break;
		}
		if (nchars != EOF)
			return(dot->l_no);
		else { /* EOF */
			dot = dol;
			eoflag++;
			doliseof++;
			eofl_no = dol->l_no;
			return(EOF);
		}
	}
	else { /* where < dol->l_no */
		if (pipe_in) {
			if (fflush(tmp_fou) == EOF) {
				fprintf(stderr,MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"));
				end_it();
			}
			in_file = f = tmp_fin;
		}
		if (where < zero->l_no){
			fseek(f, zero->l_addr, 0);
			dot = zero - 1;
			return(BOF);
		}
		else {
			dot = zero + where - 1;
			fseek(f, dot->l_addr, 0);
			nchars = getline(f);
			return(dot->l_no);
		}
	}
}

/*
 * NAME: getline
 *                                                                    
 * FUNCTION: 
 * 		Get a logical line
 *                                                                    
 * RETURN VALUE:  return the column number in which this line is read.
 */  

static getline(f)
register FILE *f;
{
	register wint_t	c;
	register unsigned char	*p;
	register int	column;
	register int	i;
	register int	cnt;
	register wint_t (*rdchar)(FILE *);
	register int	len;
	
	wint_t fgetputc(FILE *);
        static int      colflg; /* true if received \f or a long line */

	if (pipe_in && f == stdin)
		rdchar = fgetputc;
	else
		rdchar = fgetwc;

	for (i=1, column=0, p=Line; i < LINSIZ-1; i++, p+=cnt) {
		errno = 0;
		c = (*rdchar)(f);
		if ((c==WEOF) && errno) {
			perror(strcat("pg: ",fnames[fnum]));
			exit_val = 1;
			return(EOF);
		}
		cnt = wctomb(p, c);
		switch(c) {
		case WEOF:
			clearerr(f);
			if (p > Line) {	/* last line doesn't have '\n', */
				*p++ = '\n';
				*p = '\0';	/* print it any way */
				return(column);
			}
			return(EOF);
		case L'\n':
			break;
		case L'\t': /* just a guess */
			column = 1 + (column | 7);
			break;
		case L'\b':
			if (column > 0)
				column--;
			break;
		case L'\r':
			column = 0;
			break;
		default:
			if (c >= L' ')
				column += (((len=wcwidth(c))==-1)?1:len);
			break;
		}
		if (c == L'\n') {
			p++;
			break;
		}
		if (column >= columns && !fflag) {
			if (cnt<2)
				p++;
			else
				ungetwc(c,f);
			break;
		}
	}
	if (c != L'\n') { /* We're stopping in the middle of the line */
		if (column != columns || !auto_right_margin)
			*p++ = '\n';	/* for the display */
		/* peek at the next character */
		c = fgetwc(f);
		if (c == L'\n') {
			ungetwc(c,f);
			c = (*rdchar)(f); /* gobble and copy it */
		}
		else
		if (c == WEOF) /* get it next time */
			clearerr(f);
		else
			ungetwc(c,f);
	}
	*p = 0;
	return(column);
}

/*
 * NAME: save_input
 *                                                                    
 * FUNCTION:  	Copy a file, if it is a real file lseek to the begining.
 * 		if output is from a pipe, then start reading from there.
 *
 * RETURN VALUE:  void
 */  

static void
save_input(f)
FILE *f;
{
	if (pipe_in) {
		save_pipe();
		in_file = tmp_fin;
		pipe_in = 0;
	}
	fseek(in_file,0L,0);
	copy_file(in_file,f);
}

/*
 * NAME: save_pipe
 *                                                                    
 * FUNCTION: try to save the output from a pipe.
 *                                                                    
 * RETURN VALUE: void
 */  

static void
save_pipe()
{
	if (!doliseof)
		while (fgetputc(stdin) != WEOF)
			if (brk_hit) {
				brk_hit = 0;
				error(MSGS(PIPSAV,"Piped input only partially saved"));
				break;
			}
	if (fclose(tmp_fou) == EOF) {
		fprintf(stderr,MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"));
		end_it();
	}
}

/*
 * NAME: fgetputc
 *                                                                    
 * FUNCTION:	copy anything read from a pipe to tmp_fou 
 *                                                                    
 * RETURN VALUE: The character read in is returned.
 */  

static wint_t fgetputc(FILE *f)
{
	register wint_t c;
	if ((c = getwc(f)) != WEOF)
		if (putwc(c,tmp_fou) == WEOF) {
			fprintf(stderr,MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"));
			end_it();
		}
	return(c);
}

/*
 * NAME: lineset
 *                                                                    
 * FUNCTION: initialize line memory
 *
 * RETURN VALUE: void
 */  

static void
lineset(how)	
int how;
{
	if (zero == NULL) {
		nlall = 128;
		zero = (LINE *) malloc((size_t)(nlall*sizeof(LINE)));
		if (zero == NULL)
		{
			fputs("malloc failed\n",stderr);
			exit (-1);
		}
			
	}
	dol = contig = zero;
	zero->l_no = 1;
	zero->l_addr = 0l;
	if (how == BOF) {
		dot = zero - 1;
		eoflag = 0;
		doliseof = 0;
		eofl_no = -1;
	}
	else {
		dot = dol;
		eoflag = 1;
		doliseof = 1;
		eofl_no = 1;
	}
}

/*
 * NAME: newdol
 *                                                                    
 * FUNCTION: 	Add address of new 'dol'
 *		assumes that f is currently at beginning of said line
 *		updates dol
 *
 * RETURN VALUE: 
 */  

static void
newdol(f)	
register FILE *f;
{
	register int diff;

	if ((dol - zero) + 1 >= nlall){
		LINE *ozero = zero;

		nlall += 512;
		if ((zero = (LINE *) realloc ((void *) zero,
		     (size_t)(nlall * sizeof(LINE)))) == NULL){
			zero = ozero;
			compact();
		}
		diff = (char *)zero - (char *)ozero;
		dot = (LINE *)((char *)dot + diff);
		dol = (LINE *)((char *)dol + diff);
		contig = (LINE *)((char *)contig + diff);
	}
	dol++;
	if (!pipe_in)
		dol->l_addr = ftell(f);
	else {
		if (fflush(tmp_fou) == EOF) {
			fprintf(stderr,MSGS(TEMPERRW,"pg: Can't get temp file for writing\n"));
			end_it();
		}
		dol->l_addr = ftell(tmp_fou);
	}
	dol->l_no = (dol-1)->l_no + 1;
}

static compact()
{
	fprintf(stderr, MSGS(MEMOUT,"pg: no more memory - line %d\n"),dol->l_no);
	end_it();
}

/*
 * NAME: terminit
 *                                                                    
 * FUNCTION: Set up terminal dependencies from termlib 
 */  
void catchtstp(int);			/* to catch SIGTSTP */


static terminit()	
{
	int err_ret;
        struct termios ntty;
	FILE *fp;

	if ((fp = fopen("/dev/tty","r+")) != NULL) {
		fclose(fp);
		if ((freopen("/dev/tty","r+",stdout)) == NULL) {
			fprintf(stderr,MSGS(NOREOPN,"pg: cannot reopen stdout\n"));
		exit(1);
			}
		}
	setupterm(0,fileno(stdout),&err_ret); 
	if (err_ret != 1)
		setupterm("dumb",fileno(stdout),&err_ret); 
	if (err_ret != 1) {
		fprintf(stderr,MSGS(TERMTYP,"pg: cannot find terminal type\n"));
		exit(1);
	}

	/* there must be a better way using "curses" */
	tcgetattr(fileno(stdout),&ntty);
	ntty.c_iflag |= ICRNL;
	ntty.c_lflag &= ~(ECHONL | ECHO | ICANON);
	ntty.c_cc[VMIN] = 1;
	ntty.c_cc[VTIME] = 1;
	tcsetattr(fileno(stdout),TCSANOW,&ntty);
	/*
	 * catch SIGTSTP
	 */
	if (signal(SIGTSTP, SIG_IGN) == SIG_DFL) {
		signal(SIGTSTP, catchtstp);
		catch_susp++;
	}
	saveterm();
	resetterm();
	if (lines <= 0 || hard_copy) {
		hard_copy = 1;
		lines = 24;
	}
	if (columns <= 0)
		columns = 80;
	if (clropt && !clear_screen)
		clropt = 0;
	if ((shell = getenv("SHELL")) == NULL)
			shell = "/usr/bin/sh";
}

static error(mess)
char *mess;
{
	kill_line();
	sopr(mess,1);
	prompt((char *) NULL);
	errors++;
}

/*
 * NAME: prompt
 *                                                                    
 * FUNCTION: 
 * 		Return a string containing the prompt.
 */  

static prompt(filename)
char *filename;
{
	char outstr[PROMPTSIZE+6];
	int pagenum;

	if (filename != NULL) {
		sprintf(msgbuf, MSGS(NXTFIL, "(Next file: %s)"), filename);
		sopr(msgbuf,1);
	}
	else {
		if ((pagenum=(int)((new_ss.last_line-2)/(window-1)+1))
						> 999999)
			pagenum = 999999;
		sprintf(outstr,promptstr,pagenum);
		sopr(outstr,1);
	}
	fflush(stdout);
}

/*
 * NAME: sopr
 *                                                                    
 * FUNCTION:
 *  sopr puts out the message (please no \n's) surrounded by standout
 *  begins and ends
 *                                                                    
 * RETURN VALUE: none
 */  

static sopr(m,count)
	unsigned char *m;
	short count;
{
	if (count)
		promptlen += strlen(m);
	if (soflag && enter_standout_mode && exit_standout_mode) {
		putp(enter_standout_mode);
		pr(m);
		putp(exit_standout_mode);
	}
	else
		pr(m);
}

static pr(s)
unsigned char	*s;
{
	fputs((char *)s,stdout);
}

static doclear()
{
	if (clear_screen)
		putp(clear_screen);
	putchar('\r');  /* this resets the terminal drivers character */
			/* count in case it is trying to expand tabs  */
}

static kill_line()
{
	erase_line(0);
	if (!clr_eol) putchar ('\r');
}

/*
 * NAME: erase_line
 *                                                                    
 * FUNCTION: 	
 * 		Erase from after col to end of prompt
 */  

static erase_line(col)
register int col;
{

	if (promptlen == 0)
		return;
	if (hard_copy)
		putchar('\n');
	else {
		if (col == 0)
			putchar('\r');
		if (clr_eol) {
			putp(clr_eol);
			putchar('\r');  /* for the terminal driver again */
		}
		else
			for (col = promptlen - col; col > 0; col--)
				putchar (' ');
	}
	promptlen = 0;
}

/*
 * NAME: on_brk
 *                                                                    
 * FUNCTION: 	
 * 		Come here if a quit or interrupt signal is received
 *                                                                    
 */  

static on_brk(int sno)
{
	signal(sno, (void (*)(int))on_brk);
	if (!inwait) {
		BEEP();
		brk_hit = 1;
	}
	else {
		brk_hit = 0;
		longjmp(restore,1);
	}
}

/*
 * NAME: chgwinsz
 *                                                                    
 * FUNCTION:
 * 		Update window size data.
 */  

static chgwinsz (int sno)
{
	struct winsize win;
	
	signal(sno, (void (*)(int))chgwinsz);
	if ((!win_sz_set) && (out_is_tty)) {
    		if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) {
	    		window = win.ws_row-1;
			if (window < 1)
				window = 1;
			/*
			 * If (window == INT_MAX) then overflow errors will
			 * occur.  This is not likely to happen, but it is
			 * good to be safe, just in case.
			 */
			if (window == INT_MAX)
				window = INT_MAX-1;
			columns = win.ws_col;
			if (columns <= 0)
				columns = 80;
    		}
	}
}

/*
 * NAME: end_it
 *                                                                    
 * FUNCTION:
 * 		Clean up terminal state and exit.
 */  

static end_it (void)
{

	if (out_is_tty) {
		kill_line();
		resetterm();
	}
	if (tmp_fin)
		fclose(tmp_fin);
	if (tmp_fou)
		fclose(tmp_fou);
	if (tmp_fou || tmp_fin)
		unlink(tmp_name);
	exit(exit_val);
}

/*
 *	catch SIGTSTP
 */
static void
catchtstp(int sig)
{
	signal(sig, SIG_IGN);		/* temporarily... */

	/* ignore SIGTTOU so we don't get stopped if csh grabs the tty */
	signal(SIGTTOU, SIG_IGN);
	resetterm();
	fflush (stdout);
	signal(SIGTTOU, SIG_DFL);

	/* Send the TSTP signal to suspend our process group */
	signal(sig, SIG_DFL);
	sigsetmask(0);
	kill (0, sig);

	/* Pause for station break */

	/* We're back */
	signal (sig, catchtstp);
	fixterm();
}

/*
 * NAME: addmax
 *                                                                    
 * FUNCTION: Add two INT's and account for MAXINT.
 *
 */  

static int
addmax(int val1, int val2)
{
	/*
	 * Be careful with overflows.  If v1+v2 > INT_MAX then  v1+v2 will
	 * end up being negative.  Thus, use: (v1 + v2 > v3) => (v1 > v3 - v2)
	 */
	if (val1 > ((INT_MAX-1) - val2))
		return (INT_MAX-1);
	else
		return (val1 + val2);
}
