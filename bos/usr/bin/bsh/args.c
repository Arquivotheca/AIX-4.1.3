static char sccsid[] = "@(#)84	1.16  src/bos/usr/bin/bsh/args.c, cmdbsh, bos411, 9428A410j 4/22/94 18:20:30";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: options freedolh setargs freeargs copyargs clean_args clearup
 * 	      useargs savedol setdol
 *
 * ORIGINS: 3, 26, 27, 71
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.9  com/cmd/sh/sh/args.c, cmdsh, bos324
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 *
 * OSF/1 1.1
 */

#include	"defs.h"

static struct dolnod	*copyargs();
	struct dolnod	*freeargs();
	void		freedolh();
	void		savedol ();
	void		setdol ();
static struct dolnod	*dolh;

uchar_t	flagadr[14];

uchar_t	flagchar[] =
{
	'x',
	'n', 
	'v', 
	't', 
	STDFLG, 
	'i', 
	'e', 
	'r', 
	'k', 
	'u', 
	'h',
	'f',
	'a',
#ifdef NLSDEBUG
	'D',
#endif
	 0
};

long	flagval[]  =
{
	execpr,	
	noexec,	
	readpr,	
	oneflg,	
	stdflg,	
	intflg,	
	errflg,	
	rshflg,	
	keyflg,	
	setflg,	
	hashflg,
	nofngflg,
	exportflg,
#ifdef NLSDEBUG
	debugflg,
#endif
	  0
};

/* ========	option handling	======== */


options(argc,argv)
	uchar_t	**argv;
	int	argc;
{
	register uchar_t *cp;
	register uchar_t **argp = argv;
	register uchar_t *flagc;
	uchar_t	*flagp;

	if (argc > 1 && *argp[1] == '-')
	{
		/*
		 * if first argument is "--" then options are not
		 * to be changed. Fix for problems getting 
		 * $1 starting with a "-"
		 */

		cp = argp[1];
		if (cp[1] == '-')
		{
			argp[1] = argp[0];
			argc--;
			return(argc);
		}
		if (cp[1] == '\0')
			flags &= ~(execpr|readpr);

		/*
		 * Step along 'flagchar[]' looking for matches.
		 * 'sicr' are not legal with 'set' command.
		 */

		while (*++cp)
		{
			flagc = flagchar;
			while (*flagc && *flagc != *cp)
				flagc++;
			if (*cp == *flagc)
			{
				if (eq(argv[0], "set") && any(*cp, "sicr"))
					failed(argv[1],MSGSTR(M_BADOPT,(char *)badopt));
				else
				{
					flags |= flagval[flagc-flagchar];
					if (flags & errflg)
						eflag = errflg;
				}
			}
			else if (*cp == 'c' && argc > 2 && comdiv == 0)
			{
				comdiv = argp[2];
				argp[1] = argp[0];
				argp++;
				argc--;
			}
			else
				failed(argv[1],MSGSTR(M_BADOPT,(char *)badopt));
		}
		argp[1] = argp[0];
		argc--;
	}
	else if (argc > 1 && *argp[1] == '+')	/* unset flags x, k, t, n, v, e, u */
	{
		cp = argp[1];
		while (*++cp)
		{
			flagc = flagchar;
			while (*flagc && *flagc != *cp)
				flagc++;
			
			if (*cp != *flagc) {
				failed(argv[1],MSGSTR(M_BADOPT,(char *)badopt));
			}  else {
				
				/*
				 * step through flags
				 */
				if (!any(*cp, "sicr") && *cp == *flagc)	{
					/*
					 * only turn off if already on
					 */
					if ((flags & flagval[flagc-flagchar])) {
						flags &= ~(flagval[flagc-flagchar]);
						if (*cp == 'e')
							eflag = 0;
					}
				}
			}
		}
		argp[1] = argp[0];
		argc--;
	}
	/*
	 * set up $-
	 */
	flagp = flagadr;
	if (flags)
	{
		flagc = flagchar;
		while (*flagc)
		{
			if (flags & flagval[flagc-flagchar])
				*flagp++ = *flagc;
			flagc++;
		}
	}
	*flagp = 0;
	return(argc);
}

/*
 * sets up positional parameters
 */
setargs(argi)
	uchar_t	*argi[];
{
	register uchar_t **argp = argi;	/* count args */
	register int argn = 0;

	while (Rcheat(*argp++) != ENDARGS)
		argn++;
	/*
	 * free old ones unless on for loop chain
	 */
	freedolh();
	dolh = copyargs(argi, argn);
	dolc = argn - 1;
}


void
freedolh()
{
	register uchar_t **argp;
	register struct dolnod *argblk;

	if (argblk = dolh)
	{
		if ((--argblk->doluse) == 0)
		{
			for (argp = argblk->dolarg; Rcheat(*argp) != ENDARGS; argp++)
				free(*argp);
			free(argblk);
		}
	}
}

struct dolnod *
freeargs(blk)
	struct dolnod *blk;
{
	register uchar_t **argp;
	register struct dolnod *argr = 0;
	register struct dolnod *argblk;
	int cnt;

	if (argblk = blk)
	{
		argr = argblk->dolnxt;
		cnt  = --argblk->doluse;

		if (argblk == dolh)
		{
			if (cnt == 1)
				return(argr);
			else
				return(argblk);
		}
		else
		{			
			if (cnt == 0)
			{
				for (argp = argblk->dolarg; Rcheat(*argp) != ENDARGS; argp++)
					free(*argp);
				free(argblk);
			}
		}
	}
	return(argr);
}

static struct dolnod *
copyargs(from, n)
	uchar_t	*from[];
{
	register struct dolnod *np = (struct dolnod *)malloc(sizeof(char**) * n + 3 * BYTESPERWORD);
	register uchar_t **fp = from;
	register uchar_t **pp;

	np->doluse = 1;	/* use count */
	pp = np->dolarg;
	dolv = pp;
	
	while (n--)
		*pp++ = make(*fp++);
	*pp++ = ENDARGS;
	return(np);
}


struct dolnod *
clean_args(blk)
	struct dolnod *blk;
{
	register uchar_t **argp;
	register struct dolnod *argr = 0;
	register struct dolnod *argblk;

	if (argblk = blk)
	{
		argr = argblk->dolnxt;

		if (argblk == dolh)
			argblk->doluse = 1;
		else
		{
			for (argp = argblk->dolarg; Rcheat(*argp) != ENDARGS; argp++)
				free(*argp);
			free(argblk);
		}
	}
	return(argr);
}

clearup()
{
	/*
	 * force `for' $* lists to go away
	 */
	while (argfor = clean_args(argfor))
		;
	/*
	 * clean up io files
	 */
	while (pop())
		;

	/*
	 * clean up tmp files
	*/
	while (poptemp())
		;
}

struct dolnod *
useargs()
{
	if (dolh)
	{
		if (dolh->doluse++ == 1)
		{
			dolh->dolnxt = argfor;
			argfor = dolh;
		}
	}
	return(dolh);
}


		/*	this function will return the current value
		 *	of positional parameters
		*/
void
savedol ( save_dol )
	struct dolsave	*save_dol;
{
	save_dol->s_dolh = dolh ;
	save_dol->s_dolv = dolv ;
	save_dol->s_dolc = dolc ;
	dolh = 0;
	dolv = 0;
	dolc = 0;
}


		/*	this function will reset the current value of
		 *	dolh, the structure pointing to the positional
		 *	parameters.
		*/
void
setdol ( last_dolh )
	struct dolsave	*last_dolh;
{
	dolh = last_dolh->s_dolh;
	dolv = last_dolh->s_dolv;
	dolc = last_dolh->s_dolc;
}
