static char sccsid[] = "@(#)78	1.15  src/bos/usr/bin/test/test.c, cmdsh, bos41J, 9515B_all 4/12/95 11:07:11";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: main, exp, e1, e2, e3, getarg, tio 
 *
 * ORIGINS: 3, 26, 27, 71
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <sys/priv.h>
#include <locale.h>
#include <sys/errno.h>

# include	"test_msg.h"
static nl_catd catd;
# define	MSGSTR(errno, str)	catgets(catd, MS_TEST, errno, str)

/*
 * string comparison
 */
#define	eq(s1, s2)		(strcmp(s1, s2) == 0)

/*
 * check file mode
 */
#define	filmod(file, mode) \
	(stat(file, &statb) == 0 && (statb.st_mode & mode) == mode)

/*
 * check file type
 */
#define	filtyp(file, type) \
	(stat(file, &statb) == 0 && (statb.st_mode & S_IFMT) == type)

/*
 * check file size > 0
 */
#define	fsizep(file)	(stat(file, &statb) == 0 && statb.st_size > 0)

/*
 * macro to 'unget' an arg (push it back so getarg will return
 * the same arg next time...)
 */
#define	ungetarg()	(ap--)

/*
 * is 'arg' a valid expression separator? (see exp())
 */
#define	separator(arg)	(eq(arg, "-a") || eq(arg, "-o") || eq(arg, ")"))

/*
 * failed - print error message and exit
 */
#define failed(msg)	{fprintf(stderr, msg); exit(255);}

/*
 * flags to getarg() to indicate whether next argument required or not
 */
#define	REQUIRED	(1)
#define	NOT_REQUIRED	(0)

/*
 * local functions
 */
char *getarg();
int exp(), e1(), e2(), e3();

/*
 * globals
 */
static int	ap = 1;			/* index to current argument	*/
static int	ac;			/* global for 'argc'		*/
static char	**av;			/* global for 'argv'		*/
static struct stat statb;		/* for filmod, filtype & fsizep	*/

/*
 * NAME:	test
 *
 * FUNCTION:	test
 *
 * SYNOPSIS:	test [expression]
 *		[ [expression] ]
 *
 * NOTES:	Test evaluates 'expression' and returns a true value (0)
 *		if 'expression' evaluates true.  In the second form of
 *		the command: '[', the ']' is required and the brackets
 *		must be surrounded by blanks.
 *
 * RETURN VALUE DESCRIPTION:	255 if a syntax error is encountered within
 *		'expression', 0 if 'expression' evaluates true, else 1
 */

int
main(argc, argv)
int argc;
char *argv[];
{
	int status;

	(void) setlocale (LC_ALL, "");

	catd = catopen(MF_TEST,NL_CAT_LOCALE);

	/*
	 * initialize globals...
	 */
	ac = argc;
	av = argv;

	/*
	 * check for matching brackets if called as '[' ...
	 */
	if (eq(av[0], "[") && !eq(av[--ac], "]"))
		failed(MSGSTR(MISSING_BRACKET, "test: ] missing\n"))

	if (ac <= 1)			/* fail on no expression	*/
		return(1);

	status = exp() ? 0 : 1;		/* evaluate expression		*/

	if (getarg(NOT_REQUIRED) != 0) 	/* check for extra args		*/
		failed(MSGSTR(TOO_MANY_ARGS, "test: too many arguments\n"))

	return (status);
}

/*
 * NAME:	exp
 *
 * FUNCTION:	exp - top level of expression evaluation
 *
 * NOTES:	Exp is the top level of evaluation.  It calls e1 to
 *		do the actual evaluation, then handles the -o (binary
 *		or) operator, which has the lowest precedence of the
 *		combining operators.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

static int
exp()
{
	register int value;
	register char *arg2;

	/*
	 * evaluate current function
	 */
	value = e1();

	/*
	 * see if there are any args left
	 */
	if ((arg2 = getarg(NOT_REQUIRED)) != 0)
	{
		/*
		 * if -o, get the next expression and or it's value in
		 * with this one...
		 */
		if (eq(arg2, "-o"))
			return(value | exp());

		/*
		 * if not -o, then it should be a ')' (since e3 could
		 * have called exp() because of a '(')
		 */
		if (!eq(arg2, ")"))
			failed(MSGSTR(SYNTAX, "test: syntax error\n"))

		/*
		 * push the ) back on
		 */
		ungetarg();
	}

	return(value);
}

/*
 * NAME:	e1
 *
 * FUNCTION:	e1 - second level of expression evaluation
 *
 * NOTES:	E1 is the second level of evaluation.  It calls
 *		e2() and handles the -a (binary and) operator, which
 *		has higher priority than -o but lower than !.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

static int
e1()
{
	register int value;
	register char *arg2;

	/*
	 * evaluate current function
	 */
	value = e2();

	/*
	 * check for -a
	 */
	if ((arg2 = getarg(NOT_REQUIRED)) != 0)
	{
		/*
		 * process -a
		 */
		if (eq(arg2, "-a"))
			return(value & e1());

		/*
		 * not -a.  stick the arg back
		 */
		ungetarg();
	}

	return(value);
}

/*
 * NAME:	e2
 *
 * FUNCTION:	e2 - third level of expression evaluation
 *
 * NOTES:	E2 is the third level of evaluation.  It processes
 *		the ! (negation) operator.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

static int
e2()
{
	/*
	 * negate value if ! found
	 */
	if (eq(getarg(REQUIRED), "!"))
		return(!e3());

	ungetarg();

	return(e3());
}

/*
 * NAME:	e3
 *
 * FUNCTION:	e3 - last level of evaluation
 *
 * NOTES:	E3 does the actual evaluation of a function.  It also
 *		handles expressions within parentheses.
 *
 * RETURN VALUE DESCRIPTION:	1 if the expression evaluated is true,
 *		else 0
 */

static int
e3()
{
	int value;
	long int1, int2;
	register char *arg1, *arg2;

	arg1 = getarg(REQUIRED);		/* get first token */

	if (eq(arg1, "("))
	{
		/*
		 * found paren.  call exp() to evaluate the
		 * nested expression.
		 */
		value = exp();

		/*
		 * eat right paren
		 */
		if ((arg1 = getarg(NOT_REQUIRED)) == 0 || !eq(arg1, ")"))
			failed(MSGSTR(PAREN_EXPECTED, "test: ) expected\n"))

		return(value);
	}

	arg2 = getarg(NOT_REQUIRED);	/* get next token ... */
	ungetarg();		/* put it back in case we need it again */

	/*
	 * if no more tokens exist, or if the next isn't "=" or "!=",
	 * then try to evaluate the operator (note that if no more
	 * tokens exist, the getarg(REQUIRED)'s will error and exit)
	 */
	if ((arg2 == 0) || (!eq(arg2, "=") && !eq(arg2, "!=")))
	{
		if (eq(arg1, "-r"))	/* check file for read perm */
			return(tio(getarg(REQUIRED), R_OK));
		if (eq(arg1, "-w"))	/* check file for write perm */
			return(tio(getarg(REQUIRED), W_OK));
		if (eq(arg1, "-x"))	/* check file for execute perm */
			return(tio(getarg(REQUIRED), X_OK));
		if (eq(arg1, "-d"))	/* directory? */
			return(filtyp(getarg(REQUIRED), S_IFDIR));
		if (eq(arg1, "-c"))	/* char special device? */
			return(filtyp(getarg(REQUIRED), S_IFCHR));
		if (eq(arg1, "-b"))	/* block special device? */
			return(filtyp(getarg(REQUIRED), S_IFBLK));
		if (eq(arg1, "-f"))	/* regular file? */
			return(filtyp(getarg(REQUIRED), S_IFREG));
		if (eq(arg1, "-e"))	/* file exists? */
			return(lstat(getarg(REQUIRED), &statb) == 0 ? 1 : 0);
		if (eq(arg1, "-u"))	/* setuid on? */
			return(filmod(getarg(REQUIRED), S_ISUID));
		if (eq(arg1, "-g"))	/* setgid on? */
			return(filmod(getarg(REQUIRED), S_ISGID));
		if (eq(arg1, "-k"))	/* sticky bit on? */
			return(filmod(getarg(REQUIRED), S_ISVTX));
		if (eq(arg1, "-p"))	/* named pipe? (FIFO) */
			return(filtyp(getarg(REQUIRED),S_IFIFO));
   		if (eq(arg1, "-s"))	/* size > 0? */
			return(fsizep(getarg(REQUIRED)));
		if (eq(arg1, "-t"))	/* isatty? */
		{
			/*
			 * get file descriptor to check
			 */
			if ((arg1 = getarg(REQUIRED)) == 0 ||
			    separator(arg1))
			{
				/*
				 * no file descriptor, check 1...
				 * stick the arg back if it's a separator
				 */
				if (arg1 != 0)
					ungetarg();
				return(isatty(1));
			}
			
		 	{
				int ret;	
				char *ptr;
			
				errno = 0;
				ret = strtol(arg1, &ptr, 10);
				/* check for numeric number or partially
                                   converted */
				if (errno || strcmp(arg1, ptr) == 0 ||
          			    *ptr != (char *)NULL)
					return(0);
				else
					return(isatty(ret));
			}
		}
		if (eq(arg1, "-L") || eq(arg1, "-h"))
		{
			struct stat statb;
			if (lstat((char *)getarg(0), &statb) < 0)
				return(0);
			return((statb.st_mode&S_IFMT)==S_IFLNK);
		}
		if (eq(arg1, "-n"))	/* check string length != 0 */
			return(!eq(getarg(REQUIRED), ""));
		if (eq(arg1, "-z"))	/* check string length == 0 */
			return(eq(getarg(REQUIRED), ""));
	}

	/*
	 * if no more tokens in this expression, check the string length != 0
	 * (like -n)
	 */
	if ((arg2 = getarg(NOT_REQUIRED)) == 0 || separator(arg2))
	{
		/*
		 * stick the arg back if it's a separator
		 */
		if (arg2 != 0)
			ungetarg();
		return(!eq(arg1, ""));
	}

	if (eq(arg2, "="))	/* string comparison */
		return(eq(getarg(REQUIRED), arg1));

	if (eq(arg2, "!="))	/* ! string comparison */
		return(!eq(getarg(REQUIRED), arg1));

	int1 = atol(arg1);
	int2 = atol(getarg(REQUIRED));

	/*
	 * do algebraic comparisons ...
	 */
	if (eq(arg2, "-eq"))
		return(int1 == int2);
	if (eq(arg2, "-ne"))
		return(int1 != int2);
	if (eq(arg2, "-gt"))
		return(int1 > int2);
	if (eq(arg2, "-lt"))
		return(int1 < int2);
	if (eq(arg2, "-ge"))
		return(int1 >= int2);
	if (eq(arg2, "-le"))
		return(int1 <= int2);

	/*
	 * there's a token in place where there should be an operator,
	 * but we don't recognize it ...
	 */
	failed(MSGSTR(UNKNOWN_OP, "test: unknown operator\n"))
	/* NOTREACHED */
}

/*
 * NAME:	getarg
 *
 * FUNCTION:	getarg - return next argument
 *
 * NOTES:	Getarg returns the next argument to look at.  If 'required'
 *		is non-zero, getarg will print an error message and exit
 *		if no more arguments exist.
 *
 * DATA STRUCTURES:	ap is incremented
 *
 * RETURN VALUE DESCRIPTION:	0 if there are no more args to look
 *		at, else a pointer to the next arg
 */

static char *
getarg(required)
int required;
{
	/*
	 * return the next argument if one exists
	 */
	if (ap < ac)
		return(av[ap++]);

	/*
	 * print an error & exit if this argument was required
	 */
	if (required)
		failed(MSGSTR(ARG_EXPECTED, "test: argument expected\n"))

	/*
	 * increment ap anyway (in case of an ungetarg()) and
	 * return 0 since there are no arguments left
	 */
	ap++;

	return(0);
}

/*
 * check file access
 * RETURN VALUE DESCRIPTION:	1 if true; else 0. 
 */
static tio(name, mode)
char	*name;
int	mode;
{
	int	statmode;
	struct stat statb;
	static  uid_t  euid = -1,
	               egid = -1;

	if ( euid == -1 )
	{
		euid = geteuid() ;
		egid = getegid() ;
	}

	if(*name==0)
		/* null statement */ ;

	/*
	 *  POSIX 1003.2 - 1992
	 *     -r file   True if file exists and is readable.
	 *     -w file   True if file exists and is writable.
	 *               True shall indicate only that the write flag is on.
	 *               The file shall not be writable on a read-only file
	 *               system even if this test indicates true.
	 *     -x file   True if file exists and is executable.
	 *               True shall indicate only that the execute flag is on.
	 *               If file is a directory, true indicates that the
	 *               file can be searched.
	 */

	/* to check for readability */
	else if( mode != W_OK && mode != X_OK )
	{
		if ( access(name,mode) == 0 )
			return(1);
	}

	/* to check the -w flag or -x flag */
	else if(stat(name, &statb) == 0)
	{
		/* for root, true if any one of the w/x flag is on */
		if (euid == 0)
		{
			if(mode == W_OK)
				statmode = (S_IWUSR|S_IWGRP|S_IWOTH);
			else	/* mode == X_OK */
				statmode = (S_IXUSR|S_IXGRP|S_IXOTH);
			if(statb.st_mode & statmode)
				return(1);
			else
				return(0);
		}

		/* true if w/x flag is on for others */ 
		if (mode == W_OK && statb.st_mode & S_IWOTH)
			return(1);
		if (mode == X_OK && statb.st_mode & S_IXOTH)
			return(1);

		/* for owner of the file, true if u+w or u+x */
		if (euid == statb.st_uid) {
			if(mode == W_OK)
				statmode = S_IWUSR;
			else	/* mode == X_OK */
				statmode = S_IXUSR;
			if(statb.st_mode & statmode)
				return(1);
		}

		/* for user who belongs to the same group as the file,
		   true if g+w or g+x */
		if (egid == statb.st_gid)
			if(mode == W_OK)
				statmode = S_IWGRP;
			else	/* mode == X_OK */
				statmode = S_IXGRP;
		else
		{
			/* you can be in several groups */
			int n = NGROUPS_MAX;
			gid_t groups[NGROUPS_MAX];

			statmode = 0;

			n = getgroups(n,groups);
			while(--n >= 0)
				if(groups[n] == statb.st_gid)
				{
					if(mode == W_OK)
						statmode = S_IWGRP;
					else	/* mode == X_OK */
						statmode = S_IXGRP;
					break;
				}
		}

		if(statb.st_mode & statmode)
			return(1);
	}
	return(0);
}
