static char sccsid[] = "@(#)05	1.27  src/bos/usr/bin/bsh/macro.c, cmdbsh, bos411, 9428A410j 9/1/93 17:33:10";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: copyto skipto skipch macro comsubst subst flush
 *
 * ORIGINS: 3, 26, 27, 71
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.21  com/cmd/sh/sh/macro.c, cmdsh, bos320, 9141320 10/3/91 10:13:11
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	"defs.h"
#include	"sym.h"
#include        <termios.h>
#include	<sys/errno.h>
#include	"shctype.h"

static uchar_t	quote;	/* used locally */
static uchar_t	quoted;	/* used locally */

static	int	comsubst ();
static	uchar_t	*copyto ();
static	void	flush ();
static	uchar_t	getch ();

static uchar_t *
copyto(endch)
register uchar_t   endch;
{
	register uchar_t	c;
	uchar_t         	previousChar = 0;

	while ((c = getch(endch)) != endch && c)
		if ( NLSfontshift(previousChar) )
		{
			pushstak(c);
			previousChar = 0 ;
		}
		else
		{
			pushstak(c | quote);
			previousChar = c ;
		}
	zerostak();
	if (c != endch)
		error(MSGSTR(M_BADSUB,(char *)badsub));
}

static
skipto(endch)
register uchar_t	endch;
{
	/*
	 * skip uchar_ts up to }
	 */
	register uchar_t	c;

	while ((c = readc()) && c != endch)
	{
		switch (c)
		{
		case SQUOTE:
			skipto(SQUOTE);
			break;

		case DQUOTE:
			skipto(DQUOTE);
			break;

		case DOLLAR:
			if (readc() == BRACE)
				skipto('}');
		}
	}
	if (c != endch)
		error(MSGSTR(M_BADSUB,(char *)badsub));
}

static uchar_t
getch(endch)
uchar_t	endch;
{
	register uchar_t  d;
	uchar_t           idb[2] = { 0,0 };
	int               atflag = 0; /* set if $@ or ${array[@]} within "" */

retry:
	d = readc();
	if (!subchar(d))
		return(d);
	if (d == DOLLAR)
	{
		register unsigned int	c;
		register unsigned int	nlc;
		c = readc();
		nlc = readwc(c);
		if (dolchar(c)|| (NLSencchar(c) && NLSletter(nlc)))
		/* if this works, new dolchar() should be derived */
		{
			struct namnod *n = (struct namnod *)NIL;
			int		dolg = 0;
			BOOL		bra;
			BOOL		nulflg;
			BOOL		qflag = 0;
			register uchar_t	*argp, *v;
			uchar_t		*id = idb;

			if (bra = (c == BRACE))
			{
				c = readc();
				nlc = readwc(c);
			}
			if (NLSencchar(c) && NLSletter(nlc))
			{
				register uchar_t *s;
				argp = (uchar_t *)relstak();
				pushstak(FNLS);

				while (NLSencchar(c) && NLSalphanum (nlc)) {
					pushstak (c);
					if (NLSfontshift(c))
					    NLpushstak (nlc);
					c = readc ();
					nlc = readwc(c);
				}	
				if (NLSfontshift (c))
					unreadwc (nlc);
				zerostak();
				n =  lookup(NLSndecode(absstak(argp)));
				setstak(argp);
				if (n->namflg & N_FUNCTN)
					error(MSGSTR(M_BADSUB,(char *)badsub));
				v = n->namval;
				id = n->namid;
				peekc = c | MARK;
			}
			else if (digchar(c))
			{
				*id = c;
				idb[1] = 0;
				if (astchar(c))
				{
					dolg = 1;
					c = '1';
				}
				c -= '0';
				v = ((c == 0) ? cmdadr : (c <= dolc) ? dolv[c] : (uchar_t *)(dolg = 0));
			}
			else if (c == '$')
				v = pidadr;
			else if (c == '!')
				v = pcsadr;
			else if (c == '#')
			{
				itos(dolc);
				v = numbuf;
			}
			else if (c == '?')
			{
				itos(retval);
				v = numbuf;
			}
			else if (c == '-')
				v = flagadr;
			else if (bra)
				error(MSGSTR(M_BADSUB,(char *)badsub));
			else
				goto retry;
			c = readc();
			if (idb[0]=='@' && quote && !atflag)
			{
				quoted--;
				atflag = 1;
			}
			if (c == ':' && bra)	/* null and unset fix */
			{
				nulflg = 1;
				c = readc();
			}
			else 
			{
				nulflg = 0;
			}
			if ( (c == '?') && bra && (!quote) )
				qflag = 1;
			if (!defchar(c) && bra)
				error(MSGSTR(M_BADSUB,(char *)badsub));
			argp = 0;
			if (bra)
			{
				if (c != '}')
				{
					argp = (uchar_t *)relstak();
					if ((v == 0 || (nulflg && *v == 0))
					    ^ (setchar(c)))
					{
					/* if possibility of conditional   */
					/* substitution, add magic header  */
					/* to mark string encoded         */
					    if (qflag == 1)
						pushstak(FNLS);
					    copyto('}');
					}
					else
					    skipto('}');
					argp = absstak(argp);
				}
			}
			else
			{
				peekc = c | MARK;
				c = 0;
			}
			if (v && (!nulflg || *v))
			{
				uchar_t tmp = (*id == '*' ? SP | quote : SP);

				if (c != '+')
				{
					for (;;)
					{
						if (*v == 0 && quote) {
							pushstak(QUOTE);
						} else
						{

	/* a bizarre inversion is happening here.
	Ordinarily decoded characters are put into the
	input stream and encoded by readc().  macro() sets
	a flag to allow it to put encoded characters into
	the stream.  But within $ substitutions it gets
	decoded strings from lookup.  So here it encodes
	them & pushes them into the stream.  The quote
	flag is merged into characters as required. No
	FNLS character is pushed on the stack.
	*/

							if (NLSisencoded(v))
							    while (c = *(++v))
									pushstak(c | quote);
							else
			    				NLSencode_stk(v,quote);
						}

						if (dolg == 0 || (++dolg > dolc))
							break;
						else
						{
							v = dolv[dolg];
							pushstak(tmp);
						}
					}
				}
			}
			else if (argp)
			{
				if (c == '?') {
					/*
					 * if there is an error message behind
					 * the '?', print it.  else print the
					 * standard "parameter null or not
					 * set" error
					 */

					/*
					 * have to do something special here
					 * for this to work with the FNLS
					 * code...
					 */
					uchar_t *arg = argp;

					/*
					 * if the arg is encoded, but that's
					 * the only thing here, point at the
					 * NULL so we get the proper mesg
					 */
					if (NLSisencoded(arg)) {
						(void) NLSskiphdr(arg);
						if (*arg != '\0')
							arg = argp;
						}

					failed(id, *arg ? arg : 
					       (uchar_t *)(MSGSTR(M_BADPARAM,(char *)badparam)));
					}
				else if (c == '=')
				{
					if (n)
					{
						trim(argp);
						assign(n, NLSndecode (argp));
					}
					else
						error(MSGSTR(M_BADSUB,(char *)badsub));
				}
			}
			else if (flags & setflg)
				failed(id, MSGSTR(M_UNSET,(char *)unset));
			goto retry;
		}
		else
		{
			peekc = c | MARK;
			if (NLSfontshift(c))
				unreadwc (nlc);
		}
	}
	else if (d == endch)
		return(d);
	else if (d == SQUOTE)
	{
		comsubst();
		goto retry;
	}
	else if (d == DQUOTE)
	{
		if ( quote == 0 )
		{
			atflag = 0;
			quoted++;
		}
		quote ^= QUOTE;
		goto retry;
	}
	return(d);
}

uchar_t *
macro(as)
uchar_t	*as;
{
	/*
	 * Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	register BOOL	savqu = quoted;
	register uchar_t	savq = quote;
	struct fileheader fb;

	push(&fb);
	estabf(as);
# ifdef NLSDEBUG
	debug("macro in",as);
# endif
	standin->fraw = TRUE;

	/* estabf puts the given string into a file descriptor, then
	   processes it (using copyto()) as if it were input.  In general,
	   such processing encodes NLS strings to the internal format.
	   Since in this case the string 'as' is already encoded,
	   the flag fraw tells readc() to disable NLS encoding.
	   Some characters may be quoted by the shell at this point;
	   all extended NLS characters are preceded by some Font Shift.
	*/
	usestak();
	quote = 0;
	quoted = 0;
	copyto(0);
	pop();
	if (quoted && (stakbot == staktop-1))
		/* quoted null string still has FNLS header */
		pushstak(QUOTE);
/*
 * above is the fix for *'.c' bug
 */
	quote = savq;
	quoted = savqu;
#if defined (NLSDEBUG)
	{ register uchar_t *s = fixstak();
	  fprintf ( stderr , "macro out = <%s>\n",s);
	  return(s);
	}
#else
	return(fixstak());
#endif
}


static int
comsubst()
{
	/*
	 * command substn
	 */
	struct fileblk	cb;
	register uchar_t	d;
	register uchar_t *savptr = fixstak();
        register uchar_t *savptr2;
        register long savflags = flags;
	extern int errno;

	usestak();
	while ((d = readc()) != SQUOTE && d)
		pushstak(d);
	{
		register uchar_t	*argc;

		trim(argc = fixstak());
		push(&cb);
		estabf(argc);
		standin->fraw = TRUE;
	}
	{
		register struct trenod *t = makefork(FPOUT, cmd(EOFSYM, MTFLG | NLFLG));
		int		pv[2];

		/*
		 * this is done like this so that the pipe
		 * is open only when needed
		 */
		chkpipe(pv);
		initf(pv[INPIPE]);
		standin->fraw = TRUE;
		execute(t, 0, (int)(flags & errflg), 0, pv);
		close(pv[OUTPIPE]);
	}
	tdystak(savptr);
	staktop = movstr(savptr, stakbot);
	standin->fraw = FALSE;

	errno = 0;
	{
	uchar_t previousChar = 0;
	while (d = readc()) {
		if (errno == EINTR)
			break;
		if ( NLSfontshift(previousChar) )
		{
			pushstak(d);
			previousChar = 0 ;
		}
		else
		{
			pushstak(d | quote);
			previousChar = d ;
		}
	}
	}
	await(0, 0);
	flags = savflags;

	/*  For SJIS, cannot strip trailing newlines from top of
	 *  stack down by examining top byte only.  Need to find
	 *  start of character at top of stack. ((0x0a | 0x80) is 
	 *  not a legal 1-byte character but is a legal second byte
	 *  of a 2-byte character -- if so, previous byte could be 
	 *  a SJIS font shift or the second byte of a 2-byte character.)
	 */
	{
	do {
		savptr = stakbot;
		while (savptr2 = savptr, d = *savptr & STRIP)
		    if ((savptr += NLSenclen (savptr)) >= staktop)
			break;
		/* d will be ASCII character or magic font shift */
		if (d != NL)
			break;
#ifdef NLSDEBUG
		debug ("comsubst - NL stripped", d);
#endif
	} while ((staktop -= NLSenclen (savptr2)) > stakbot);
	}
	pop();
}

#define CPYSIZ	512

subst(in, ot)
int	in, ot;
{
	register uchar_t	c;
	struct fileblk	fb;
	register int	count = CPYSIZ;
	register int	mblength;

	push(&fb);
	initf(in);
	standin->fraw = TRUE;   /* avoid re-NLS-encoding input */
	/*
	 * DQUOTE used to stop it from quoting
	 */
	while (c = (getch(DQUOTE) & STRIP))
	{
		pushstak(c);

		if (c == FSH0)
			pushstak(getch(DQUOTE)); 
		else if ((c == FSH20) || (c == FSH21)) {
			pushstak(c = getch(DQUOTE)); /* set c to the char that
			--count;		        holds the MB char */
			mblength = c & STRIP;
			while (mblength--) {
				pushstak(getch(DQUOTE));
				pushstak(getch(DQUOTE));
				count -= 2;
			}
		}
		if (--count <= 0)  /* using stack, CPYSIZ can be off 1 */
		{
			flush(ot);
			count = CPYSIZ;
		}
	}
	flush(ot);
	pop();
}

static void
flush(ot)
{
	*staktop = 0;
# if NLSDEBUG
	debug("flush in",stakbot);
# endif
	NLSdecode(stakbot);
	staktop = stakbot + strlen(stakbot);
# if NLSDEBUG
	debug("flush out",stakbot);
# endif
	write(ot, stakbot, staktop - stakbot);

		/*	The execpr is a define in
		*	defs.h that will mask for the "-x" flag. Why it
		*	does this is a mystery, because it will place a
		*	"\n" in the stderr file.
		*	I have left the code here because I do not know
		*	if anything else is affected. I ran some tests
		*	and they appeared to run fine.
	if (flags & execpr)
		write(output, stakbot, staktop - stakbot);
		*/

	staktop = stakbot;
}
