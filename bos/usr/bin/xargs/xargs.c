static char sccsid[] = "@(#)81	1.23  src/bos/usr/bin/xargs/xargs.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:12:49";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: xargs
 *
 * ORIGINS: 3, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

#include <wchar.h>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <nl_types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "xargs_msg.h"

#define	MSGSTR(Num, Str)	catgets(catd, MS_XARGS, Num, Str)

#define MAXINSERTS	5		/* max replacements per line */
#define MAXSBUF		255		/* single replacement buffer size */
                                        /* total xargs insert cmd buffer size:
                                         * inserts * (buffersize + space + null)
                                         * + maximum pathname size of command */
#define MAXIBUF		((MAXINSERTS * (MAXSBUF + 2)) + PATH_MAX)
 					/* size for generic character arrays */
#define BUFSIZE		(ARG_MAX - 2048) /* maximum BUFFER for xargs commands */
#define MAXBUFLIM	(BUFSIZE - 2048) /* less some user/system environment */
#define MAXARGS		1024		/* max # of command line args */
#define	ECHO		"/usr/bin/echo"	/* default command to run */
#define MAXRESP		64		/* maximum length of prompt response */

/*
 * functions called, both external and local ...
 */

void    ermsg(char *);
void    addibuf(struct inserts *);
int     xindex(char *, char *);
int     echoargs();
void    lcall(char *, char **);
char *  addarg(char *);
char *  getarg();
char *  checklen(char *);
char *  insert(char *, char *);
wchar_t getchr();

/*
 * globals ...
 */
static nl_catd	catd;			/* message catalog descriptor */
static char Errstr[LINE_MAX];		/* generic error message buffer	*/
static char *arglist[MAXARGS+1];	/* ptrs to args for the command to execute */
static char argbuf[BUFSIZE+1];		/* destination for copied in arguments */
static char *next = argbuf;		/* pointer to next empty spot in 'argbuf' */
static char *lastarg = "";		/* last arg we parsed, but didn't use yet */
static char **ARGV = arglist;		/* next empty slot in 'arglist' */
static char *LEOF = "_"; 		/* logical end-of-file string */
static char *INSPAT = "{}";		/* replace string pattern */
static struct inserts {
	char **p_ARGV;		/* where to put newarg ptr in arg list */
	char *p_skel;		/* ptr to arg template */
	} saveargv[MAXINSERTS];
static char ins_buf[MAXIBUF];		/* insert buffer */
static char *p_ibuf;			/* pointer within ins_buf */
static int PROMPT = -1;		/* prompt /dev/tty file descriptor */
static int BUFLIM = MAXBUFLIM;		/* max command line size */
static int N_ARGS = MAXARGS-1;		/* # of standard input args to use per cmd */
static int N_args = 0;			/* # of arguments we've read so far */
static int N_lines = 0;		/* # of input lines used so far for this cmd */
static int DASHX = FALSE;		/* -x arg given? */
static int MORE = TRUE;		/* process more input? */
static int PER_LINE = FALSE;		/* # of input lines to use per cmd */
static int ERR = FALSE;		/* should we stop from errors yet? */
static int OK = TRUE;			/* had any errors yet? */
static int LEGAL = FALSE;		/* stop if argument list size > BUFLIM */
static int TRACE = FALSE;		/* echo cmd and args each time? */
static int INSERT = FALSE;		/* do command replacements? */
static int linesize = 0;		/* current size of cmd & args */
static int ibufsize = 0;		/* current length of ins_buf */
static int mb_cur_max;			/* Max bytes in a multibyte character */
static int lastexitval = 0;		/* Last non-zero exit value (if any) */
static wctype_t blank_tab;		/* used in one time call to wctype for blank */
/*
 * The getopt() routine in the standard library cannot handle the optional
 * option arguments. Therefore, a special version of the getopt() called
 * xargs_getopt() is written here to parse the optional option arguments
 * for -e, -i and -l options.
 *   
 * When the -e, -i and -l options become obsolescent, the getopt() routine
 * in the standard library shall be used, and the following 3 lines shall
 * be removed.
 */
static int	optind = 1;		/* index of next argv			*/
static int	optopt;			/* current option we're looking at	*/
static char	*optarg;		/* argument for current option		*/

/*
 * NAME:	xargs
 *
 * SYNTAX:	xargs [flags] command
 *
 * FUNCTION:	Xargs constructs argument lists and runs commands
 *
 * RETURN VALUE DESCRIPTION:
 *		0   if all utilities exited with 0.
 *	      126   if utility was found but could not be invoked. 
 *	      127   if utility could not be found.
 *	        1   if any other error occurs.
 */

int
main(int argc, char **argv)
{
	char *cmdname, *initbuf, **initlist, *flagval;
	int  initsize, c;
	register int j, n_inserts;
	register struct inserts *psave;
	/*
	 * The next 2 lines shall be used when the getopt() routine in the
	 * standard library is used.  
	 * extern char *optarg;
	 * extern int optind, optopt;
	 */

	(void) setlocale (LC_ALL, "");
	mb_cur_max=MB_CUR_MAX;

	catd = catopen(MF_XARGS, NL_CAT_LOCALE);

	n_inserts = 0;
	psave = saveargv;

/*
 * If we are building the bldenv xargs then it may be the case
 * that wctype and iswctype are not available. This occurs if
 * xargs is built on a 3.2* machine. To resolve this problem
 * the following macro is defined. This macro can only be
 * removed when all possible build machines are running 4.1.
 */

#ifdef _BLD
	blank_tab = get_wctype((char *)"blank"); /* set for getarg loop use */
#else
	blank_tab = wctype((char *)"blank"); /* set for getarg loop use */
#endif

	/* look for flag arguments */

	while ((c=xargs_getopt(argc, argv, ":eE:I:iL:ln:ps:tx")) != EOF)
        {
                switch (c) {
			case 'e':	/* obsolescent: equivalent to -E */
			case 'E':	/* set logical end-of-file string */
				LEOF = optarg;
				break;

			case 'i':	/* obsolescent: equivalent to -I */
			case 'I':   	/* process replace strings in arglist */
				INSERT = PER_LINE = LEGAL = TRUE;
				N_ARGS = 0;
				if (optarg != NULL)
					INSPAT = optarg;
				break;

			case 'l': /* obsolescent: similar to -L, turns on -x */
				PER_LINE = LEGAL = TRUE;
				N_ARGS = 0;
				INSERT = FALSE;
				if((optarg != NULL) && (PER_LINE=atoi(optarg)) <= 0) {
					sprintf(Errstr, MSGSTR(CNT,
						"#lines must be positive int for option: -%c\n"),
						(char)optopt);
					ermsg(Errstr);
				}
				break;

			case 'L': /* specify number of input arg lines per cmd */
				PER_LINE = TRUE;
				LEGAL = FALSE;
				N_ARGS = 0;
				INSERT = FALSE;
				if((optarg != NULL) && (PER_LINE=atoi(optarg)) <= 0) {
					sprintf(Errstr, MSGSTR(CNT,
						"#lines must be positive int for option: -%c\n"),
						(char)optopt);
					ermsg(Errstr);
				}
				break;

			case 'n':  /* number of arguments to use per cmd */
				if( (N_ARGS = atoi(optarg)) <= 0 ) {
					sprintf(Errstr, MSGSTR(CNT,
						"#args must be positive int for option: -%c\n"),
						(char)optopt);
					ermsg(Errstr);
				}
			  	else {
					LEGAL = DASHX || N_ARGS==1;
					INSERT = PER_LINE = FALSE;
				}
				break;

			case 'p':   /* prompt for each cmd before running */
				if( (PROMPT = open("/dev/tty",0)) == -1)
					ermsg(MSGSTR(TTYREAD,
						"can't read from tty for -p\n"));
				else
					TRACE = TRUE;
				break;

			case 's': /* set max size of arg list (max: MAXBUFLIM) */
				BUFLIM = atoi(optarg);
				if( BUFLIM > MAXBUFLIM  ||  BUFLIM <= 0 ) {
					sprintf(Errstr, MSGSTR(LINESIZ,
						"-s value not within system limits; %s is not valid.\n"),
						optarg);
					ermsg(Errstr);
				}
				break;

			case 't':   /* echo each argument list to stderr */
				TRACE = TRUE;
				break;

			case 'x':   /* quit if any arg list size > BUFLIM */
				DASHX = LEGAL = TRUE;
				break;

			case ':':
				sprintf(Errstr,
					MSGSTR(MISSARG, "option -%c requires an argument\n"), (char)optopt); 
				ermsg(Errstr);
				break;

			default:
				sprintf(Errstr,
					MSGSTR(UNKWNOPT, "unknown option: %c\n"),
					(char)optopt);
				ermsg(Errstr);
				break;
                }
        }


	argv += optind;
	argc -= optind;

	if( ! OK )
		ERR = TRUE;

	/* pick up command name */

	if ( argc == 0 ) {
		cmdname = ECHO;
		*ARGV++ = addarg(cmdname);	/* add echo into our argv */
		}
	else
		cmdname = *argv;	/* will be added to argv below */

	/* pick up args on command line */

	while ( OK == TRUE && argc-- > 0) {
		if ( INSERT == TRUE && ! ERR ) {
			/* does this argument have an insert pattern? */
			if ( xindex(*argv, INSPAT) != -1 ) {
				/* yes, keep track of which arg has it */
				if ( ++n_inserts > MAXINSERTS ) {
					sprintf(Errstr, MSGSTR(ARGSIZ,
						"too many args with %s\n"),
						INSPAT);
					ermsg(Errstr);
					ERR = TRUE;
				}	
				psave->p_ARGV = ARGV;
				(psave++)->p_skel = *argv;
				}
			}
		/* add arg to our new argv */
		*ARGV++ = addarg( *argv++ );
		}

	/* pick up args from standard input */

	initbuf = next;			/* save first spot past cmd & argv */
	initlist = ARGV;		/* save first argv past cmd & argv */
	initsize = linesize;		/* save current total cmd size */

	/* loop once for each time we need to call cmd... */
	while ( OK == TRUE && MORE ) {
		/*
		 * reset our pointers and line size
		 */
		next = initbuf;
		ARGV = initlist;
		linesize = initsize;

		/*
		 * get any previous arguments we didn't process yet
		 */
		if ( *lastarg )
			*ARGV++ = addarg( lastarg );

		/*
		 * get the new args
		 */
		while ( (*ARGV++ = getarg()) && OK == TRUE )
			;

		/* insert arg if requested */

		if ( !ERR && INSERT == TRUE ) {
			p_ibuf = ins_buf;
			ARGV--;
			j = ibufsize = 0;
			for ( psave=saveargv;  ++j<=n_inserts;  ++psave ) {
				addibuf(psave);
				if ( ERR )
					break;
				}
			}
		*ARGV = 0;

		/* exec command */

		if ( ! ERR ) {
			if ( ! MORE &&
			    (PER_LINE && N_lines==0 || N_ARGS && N_args==0) )
				exit (0);
			OK = TRUE;
			j = TRACE ? echoargs() : TRUE;
			if( j ) {
				lcall(cmdname, arglist);
				continue;
			}
		}
	}
	if (lastexitval)
		exit(lastexitval);
	else if (!OK)
		exit(1);
	else
		exit(0);
	/* NOTREACHED */
}

/*
 * NAME:	checklen
 *
 * FUNCTION:	checklen - check length of current arguments plus a new one
 *
 * NOTES:	Checklen checks the length of the current arguments plus
 *		the new one.  If we've gone last BUFLIM, we possibly print
 *		an error and set some error flags.
 *
 * RETURN VALUE DESCRIPTION:	NULL if we've gone past BUFLIM, else
 *		we return the new argument
 */

static char *
checklen(char *arg)
{
	register int oklen;

	oklen = TRUE;
	if ( (linesize += strlen(arg)+1) > BUFLIM ) {
		lastarg = arg;
		oklen = OK = FALSE;
		if ( LEGAL ) {
			ERR = TRUE;
			ermsg(MSGSTR(ARGLIST, "arg list too long\n"));
			}
		else if( N_args > 1 )
			N_args = 1;
		else {
			ermsg(MSGSTR(SNGLARG,
		"a single arg was greater than the max arglist size\n"));
			ERR = TRUE;
			}
		}

	return ( oklen == TRUE  ? arg : 0 );
}

/*
 * NAME:	addarg
 *
 * FUNCTION:	addarg - copy in our new arg
 *
 * NOTES:	Addarg copies in our new arg, then calls checklen().
 *
 * RETURN VALUE DESCRIPTION:	return value from checklen()
 */

static char *
addarg(char *arg)
{
	(void) strcpy(next, arg);
	arg = next;
	next += strlen(arg) + 1;

	return ( checklen(arg) );
}

/*
 * NAME:	getarg
 *
 * FUNCTION:	getarg - read our next argument from stdin
 *
 * NOTES:	Getarg reads/parses our next argument from stdin.
 *
 * RETURN VALUE DESCRIPTION:	0 if there are no more args or if
 *		the arg doesn't fit, else a pointer to the arg
 */

static char *
getarg()
{
	register wchar_t c, c1;
	register char *arg;
	char *retarg;

/*
 * If we are building the bldenv xargs then it may be the case
 * that wctype and iswctype are not available. This occurs if
 * xargs is built on a 3.2* machine. To resolve this problem
 * the following macro is defined. This macro can only be
 * removed when all possible build machines are running 4.1.
 */

#ifdef _BLD
	/* skip white space (blank, tab and newline) */
	while ( (c=getchr()) == '\n' || is_wctype((wint_t)c, blank_tab) )
#else
	/* skip white space (blank, tab and newline) */
	while ( (c=getchr()) == '\n' || iswctype((wint_t)c, blank_tab) )
#endif
		;

	/*
	 * all done?
	 */
	if ( c == '\0' ) {
		MORE = FALSE;
		return ((char *)NULL);
		}

	arg = next;
	/*
	 * read chars and process them ...
	 */
	for ( ; ; c = getchr() )
		switch ( c ) {

		case '\t':		/* blank space */
		case ' ' :
		space:
			/*
			 * only save blank characters if we have to do replace
			 * patterns...
			 */
			if ( INSERT == TRUE ) {
				if ( mb_cur_max == 1 )
					*next++ = c;
				else
					next += wctomb(next, c);
				break;
			}

		case '\0':		/* end of line/file */
		case '\n':
			*next++ = '\0';	/* null terminate current arg */
			/*
			 * Process logical EOF. Stop trying to read
			 * if a newline or null terminator typed at. 
			 */
			if( strcmp(arg, LEOF) == 0 || c == '\0' ) {
				MORE = FALSE;
				while ( c != '\n' && c != '\0')
					c = getchr();
				return ((char *)NULL);
				}
			else {
				/*
				 * add arg to our arguments
				 */
				++N_args;

				if ( INSERT == TRUE )
					/* checklen will be done */
					/* at substitution time. */
					retarg = arg;
				else
					retarg = checklen(arg);

				if( retarg ) {
					if( (PER_LINE && c=='\n' &&
					     ++N_lines>=PER_LINE)
					||   (N_ARGS && N_args>=N_ARGS) ) {
						N_lines = N_args = 0;
						lastarg = "";
						OK = FALSE;
						}
					}
				return retarg;
			}

		case '\\':		/* backslash */
			c = getchr();
			if ( mb_cur_max == 1 )
				*next++ = c;
			else
				next += wctomb(next, c);
			break;

		case '"':		/* quotes and double quotes */
		case '\'':
			while( (c1=getchr()) != c) {
				/* copy chars in till hit the next one */
				if( c1 == '\0' || c1 == '\n' ) {
					/* missing a quote... */
					*next++ = '\0';
					sprintf(Errstr, MSGSTR(MSNGQUOT,
						"missing quote?: %s\n"), arg);
					ermsg(Errstr);
					ERR = TRUE;
					return ((char *)NULL);
					}
				if ( mb_cur_max == 1 )
					*next++ = c1;
				else
					next += wctomb(next, c1);
				}
			break;

		default:
			if (iswspace(c))
				goto space;
			if ( mb_cur_max == 1 )
				*next++ = c;
			else
				next += wctomb(next, c);
			break;
		}
}

/*
 * NAME:	ermsg
 *
 * FUNCTION:	ermsg - print an error message to standard error
 *
 * NOTES:	Errmsg prints the error messages 'messages' to
 *		standard error and sets the OK flag to false.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

static void
ermsg(char *messages)
{
	write(2, "xargs: ", 7);
	write(2, messages, strlen(messages));

	OK = FALSE;
}

/*
 * NAME:	echoargs
 *
 * FUNCTION:	echoargs - print out arguments and prompt for yes or
 *		no
 *
 * NOTES:	Echoargs prints out the arguments and prompts the user
 *		for a yes or no answer.
 *
 * RETURN VALUE DESCRIPTION:	TRUE if the user responds with yes expression,
 *		else FALSE
 */

static int
echoargs()
{
	register char **anarg;
	char yesorno[MAXRESP+1], *presp;
	char *prompt;
	int j;

	anarg = arglist-1;
	while ( *++anarg ) {
		write(2, *anarg, strlen(*anarg) );
		write(2," ",1);
	}

	if( PROMPT == -1 ) {
		write(2,"\n",1);
		return (TRUE);
	}

	prompt = MSGSTR(QUESTION, "?...");
	write(2, prompt, strlen(prompt));

	presp = yesorno;
	while ( ((j = read(PROMPT,presp,1))==1) && (*presp!='\n') )
		if ( presp < &yesorno[MAXRESP] )
			presp++;
	if ( j == 0 )
		exit(0);
	if ( yesorno[0] == '\n')
		return (FALSE);
	*presp = '\0';
	return (rpmatch(yesorno) == 1 ? TRUE : FALSE);
}

/*
 * NAME:	insert
 *
 * FUNCTION:	insert - handle a replacement
 *
 * NOTES:	Insert takes a pattern and a replacement string and
 *		does any replacements necessary in the pattern.  The new
 *		string is returned.
 *
 * RETURN VALUE DESCRIPTION:	The new string after replacements.
 */

static char *
insert(char *pattern, char *subst)
{
	static char buffer[MAXSBUF+1];
	int len, ipatlen;
	register char *pat;
	register char *bufend;
	register char *pbuf;

	len = strlen(subst);
	ipatlen = strlen(INSPAT)-1;
	pat = pattern-1;
	pbuf = buffer;
	bufend = &buffer[MAXSBUF];

	while ( *++pat ) {
		if( xindex(pat,INSPAT) == 0 ) {
			if ( pbuf+len >= bufend )
				break;
			else {
				strcpy(pbuf, subst);
				pat += ipatlen;
				pbuf += len;
				}
			}
		else {
			*pbuf++ = *pat;
			if (pbuf >= bufend )
				break;
			}
		}

	if ( ! *pat ) {
		*pbuf = '\0';
		return (buffer);
		}

	sprintf(Errstr, MSGSTR(MAXSARGSIZ,
		"max arg size with insertion via %s's exceeded\n"), INSPAT);
	ermsg(Errstr);
	ERR = TRUE;

	return ((char *)NULL);
}

/*
 * NAME:	addibuf
 *
 * FUNCTION:	addibuf - perform a command replacement
 *
 * NOTES:	Addibuf looks at a struct insert structure, which
 *		contains the information for one replacement in
 *		the command string, and does the replacement.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

static void
addibuf(struct inserts *p)
{
	register char *newarg, *skel, *sub;
	int l;

	skel = p->p_skel;	/* place in argv the replacement was */
	sub = *ARGV;		/* destination ARGV for the replacement */

	if (sub) {
		linesize -= strlen(skel)+1;
		newarg = insert(skel, sub);	/* do the replacement */

		/*
	 	 * make sure the replacement fits
	 	 */
		if ( checklen(newarg) ) {
			if( (ibufsize += (l=strlen(newarg)+1)) > MAXIBUF) {
				ermsg(MSGSTR(OVRFLOW, "insert-buffer overflow\n"));
				ERR = TRUE;
			}

			strcpy(p_ibuf, newarg);	/* save replacement string */
			*(p->p_ARGV) = p_ibuf;	/* point to it */
			p_ibuf += l;		/* increment replacement pointer */
		}
	}
	else
		return;
}

/*
 * NAME:	getchr
 *
 * FUNCTION:	getchr - read the next character from standard input
 *
 * NOTES:	Getchr reads the next character from standard input
 *		and returns it.  0 is returned on end of file or error.
 *
 * RETURN VALUE DESCRIPTION:	0 if end of file is encountered or
 *		we get an I/O error,  else the character process code
 *		is returned
 */

static wchar_t
getchr()
{
	int c;

	if ( mb_cur_max == 1)
		{
		c = getchar();
		if ( c == EOF )
			c = 0;
		}
	else
		{
		c = getwchar();
		if ( c == WEOF )
			c = 0;
		}

	return (c);
}

/*
 * NAME:	lcall
 *
 * FUNCTION:	lcall - exec program with arguments
 *
 * NOTES:	Lcall forks a new process and executes the program 'sub'
 *		using 'subargs' as it's arguments.  It also waits for the
 *		program to finish and if there is no error occurred, it
 *		will return implicitly. This routine always exits on
 *	 	failure.
 *
 * EIXT VALUE DESCRIPTION:
 *	      126   if utility was found but could not be invoked. 
 *	      127   if utility could not be found.
 *	        1   if any other error occurs.
 *
 * RETURN VALUE DESCRIPTION: none
 */

static void
lcall(char *sub, char **subargs)
{

	int retcode, exitcode;
	register pid_t iwait; 
	pid_t child;

	switch( child=fork() ) {
default:
		while( (iwait = wait(&retcode)) != child  &&  iwait!= -1 )
			;

		/* exit code macros are in sys/wait.h ... */
		if( iwait == -1  ||  !WIFEXITED(retcode) || (exitcode=WEXITSTATUS(retcode)) == 255 ) {
			sprintf(Errstr, MSGSTR(NOEXEC,
				"%s not executed or returned -1\n"), sub);
			ermsg(Errstr);
			exit(1);
		}

		if ( exitcode == 126 )
			exit(126);
		if ( exitcode == 127 )
			exit(127);
		if (exitcode)
			lastexitval = exitcode;

		break;

	case 0:    /* child */
		(void) close(0);
		(void) open("/dev/null", O_RDONLY);
		(void) execvp(sub, subargs);

		if (errno == ENOENT) {
			sprintf(Errstr, "%s: %s\n", sub, strerror(errno));
			ermsg(Errstr);
			exit(127);
		}
		else {
			sprintf(Errstr, "%s: %s\n", sub, strerror(errno));
			ermsg(Errstr);
			exit(126);
		}

	case -1:
		sprintf(Errstr, "%s: %s\n", sub, strerror(errno));
		ermsg(Errstr);
		exit(1);
	}

	/* NOTREACHED */
}
/************************************************************************/
/*	WARNING: RETURNED STATUS OF OFFSET WITHIN STRING IS		*/
/*	IGNORED - WHICH MAKES THE USE OF THIS FUNCTION SUSPECT		*/
/*	WHEN ARGUMENT REPLACEMENT IS PERFORMED				*/
/************************************************************************/

/*
 * NAME:	xindex
 *
 * FUNCTION:	xindex - search for substring
 *
 * NOTES:	Xindex searches for a substring 'as2' in the string 'as1'.
 *		If the substring exists, it returns the offset of the
 *		first occurrence.  If it doesn't exist, it returns -1.
 *
 * RETURN VALUE DESCRIPTION:	-1 if the substring doesn't exist, else
 *		the index of the first occurrence of the substring
 */

static int
xindex(char *as1, char *as2)
{
	register char *s1,*s2,c;
	int offset;

	s1 = as1;
	s2 = as2;
	c = *s2;

	while (*s1)
		if (*s1++ == c) {
			offset = s1 - as1 - 1;
			s2++;
			while ((c = *s2++) == *s1++ && c)
				;
			if (c == 0)
				return(offset);
			s1 = offset + as1 + 1;
			s2 = as2;
			c = *s2;
		}

	 return (-1);
}

/*
 * xargs_getopt() is the same as the getopt() in the standard library,
 * except:
 *      1) It is particularly designed to parse the optional option arguments
 *         for -e, -i and -l options. 
 *      2) The code related to the message catalog is removed. 
 * This code shall be deleted when the -e, -i and -l options become
 * obsolescent, and the standard getopt() shall be used.
 */  

static int
xargs_getopt(int argc, char **argv, char *optstring)
{
	static int sp = 1;
	int c;
	char *cp;

	if(sp == 1)
		if(optind >= argc || argv[optind] == NULL ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(-1);
		else if(strcmp(argv[optind], "--") == 0) {
			optind++;
			return(-1);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(optstring, c)) == NULL) {  
						/* check for illegal options */
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {			 /* parameter is needed */
		if(argv[optind][sp+1] != '\0')   /* no blanks to separate
						    option and parameter */
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {      /* see if parameter missing */
			sp = 1;
			return(optstring[0] == ':' ? ':' : '?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else { 				 /* parameter not needed */
		if(argv[optind][++sp] == '\0') { /* if c is the last option
						    update optind        */
			sp = 1;
			optind++;
			optarg = NULL;
		}
		else 
			if (optopt == 'e' || optopt == 'i' || optopt == 'l') {
				optarg = &argv[optind++][sp];
				sp = 1;
			} else
				optarg = NULL;
	}
	return(c);
}
