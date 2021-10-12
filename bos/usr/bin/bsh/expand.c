static char sccsid[] = "@(#)97  1.38  src/bos/usr/bin/bsh/expand.c, cmdbsh, bos411, 9428A410j 3/24/94 18:47:03";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: expand gmatch addg makearg
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
 * 1.30  com/cmd/sh/sh/expand.c, cmdsh, bos320, 9140320 9/26/91 11:03:21	
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

#include	"defs.h"
#include	<sys/limits.h>
#include	<sys/types.h>
#include	<string.h>
#include	<sys/param.h>
#include	<dirent.h>
#include	<ctype.h>
#include	<stdlib.h>

static uchar_t             entry[3*NAME_MAX+2];
			/* room for possible 3 bytes/char. when encoded,
			   1 for header, 1 for null at end */

/*
 * globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches chartacter class
 * "[...a-z...]" in params matches a through z.
 *
 */
static int	addg();


expand(as, rcnt)
	uchar_t	*as;
{
	int	count;
	DIR	*dirf;
	BOOL	dir = 0;
	uchar_t	*rescan = 0;
	register uchar_t	*s, *cs, *p;
	struct argnod	*schain = gchain;
	BOOL	slash;

#ifdef NLSDEBUG
	debug("expand",as);
#endif

	if (trapnote & SIGSET)
		return(0);
	s = cs = as;

	/*
	 * check for meta chars
	 */
	{
		register BOOL open;

		slash = 0;
		open = 0;
		do
		{
			switch (*cs++)
			{
			case 0:
				if (rcnt && slash)
					break;
				else
					return(0);

			case '/':
				slash++;
				open = 0;
				continue;

			case '[':
				open++;
				continue;

			case ']':
				if (open == 0)
					continue;
			case '?':
			case '*':
				if (rcnt > slash)
					continue;
				else
					cs--;
				break;


			default:
				continue;
			}
			break;
		} while (TRUE);
	}

	for (;;)
	{
		if (cs == s)
		{
			s = nullstr;
			break;
		}
		else if (*--cs == '/')
		{
			*cs = 0;
			if (s+1 == cs) {
				static uchar_t fnls_slash[] = { FNLS, '/', '\0' };
				s = fnls_slash;
			}
			break;
		}
	}

#ifdef NLSDEBUG
	debug("expand open",s);
#endif


	p = *s ? NLSndecode (s) : (uchar_t *)".";
	if ((dirf = opendir((char *)p)) != NULL)
		dir++;

	count = 0;
	if (*cs == 0)
		*cs++ = 0200;
	if (dir)		/* check for rescan */
	{
		register uchar_t *rs;
		struct dirent *e;

		rs = cs;
		do
		{
			if (*rs == '/')
			{
				rescan = rs;
				*rs = 0;
				gchain = 0;
			}
		} while (*rs++);

		while ((e = readdir(dirf)) && (trapnote & SIGSET) == 0)
		{
			NLSencode(e->d_name, entry, sizeof(entry));
			NLSskiphdr(cs);
			if (entry[1] == '.' && *cs != '.')
				continue;

			if (gmatch(entry, cs))
			{
				addg(s, entry, rescan);
				count++;
			}
		}
		closedir(dirf);

		if (rescan)
		{
			register struct argnod	*rchain;

			rchain = gchain;
			gchain = schain;
			if (count)
			{
				count = 0;
				while (rchain)
				{
					count += expand(rchain->argval, slash + 1);
					rchain = rchain->argnxt;
				}
			}
			*rescan = '/';
		}
	}

	{
		register uchar_t	c;

		s = as;
		while (c = *s) {
			*s++ = (c & STRIP ? c : '/');
			if (NLSfontshift(c)) 
			{
				if ((c == FSH20) || (c == FSH21))
				{
					register int mbcnt;
					mbcnt = *s & STRIP;
					*s++;
					while(mbcnt--) {
						*s++;	
						*s++;	
					}
				} else
					if (*s)
						*s++;
			}
		}
	}
	return(count);
}

gmatch(s, p)
char	*s, *p;
{

	extern char *NLSndechr();
#define GET_COLLATE(s,t)				\
{							\
	if (NLSfontshift(s)) {				\
		char *ch = NLSndechr(t);		\
		t += NLSenclen(t);			\
		mbtowc((wchar_t *)&s, ch, MB_CUR_MAX);	\
    	} else t++;					\
}

	int		ccc;
	char		ifbuf[32], *ib, *eb, *pp;
	wchar_t		scc,c;

	/* often called with a piece of an encoded string */
	NLSskiphdr(s);
	NLSskiphdr(p);

	if (scc = *s++){
		if ((scc &= STRIP) == 0)
			scc=0200;
	}

	switch (c = *p++)
	{
	case '[':
		s--;
		GET_COLLATE(scc,s);
		{
			BOOL ok;
			int lc;
			int notflag = 0;

			ok = 0;
			lc = -1;
			if (*p == '!')
			{
				notflag = 1;
				p++;
			}
			while (c = *p++)
			{
				if (c == ']')
					return(ok ? gmatch(s, p) : 0);
				else if ((c == MINUS) && (lc > 0))
				{
					wchar_t	nc;

					nc = *p++ & STRIP;
					p--;
					GET_COLLATE(nc,p);

					if (notflag)
					{
						if( scc < lc || scc > nc )
							ok++;
						else
							return(0);
					}
					else
					{
						ok += (lc <= scc && scc <= nc);	
					}
				}
				/* Check for character class. */
				else if ((c =='[') && (*p == ':'))
				{
				    pp = p;
				    ib = ifbuf;
				    eb = &ifbuf[31];
					/* the following saves string */
					/* "[:...." in ifbuf, up to   */
					/* ibuflen or "]" or "-"      */
				    do {			
					if(c == '\0' || c == '\n') 
					    break;
					*ib++ = c;
				    } while(((c = *pp++) != ']') && (c != '-') && (c != '[') && (ib < eb));
				    if (c == ']' && ib[-1] == ':') { 
					*--ib = '\0';	
					/* check for character class */
					if(iswctype((int)scc,wctype(ifbuf+2))){
						ok++;
						p = pp;
				    	}
				    }
				}
				else
				{
					wchar_t tmp_lc;

					tmp_lc = c & STRIP;
					p--;
					GET_COLLATE(tmp_lc,p);
					lc = tmp_lc;
					if (notflag)
					{
						if (scc && scc != tmp_lc)
							ok++;
						else
							return(0);
					}
					else
					{
						if (scc == tmp_lc)
							ok++;
					}
				}
			}
			return(0);
		}

	default:
		if ((c &= STRIP) == 0)
			c=0200;
		if (c != scc)
			return(0);
		if (NLSfontshift(c)) {
			ccc = NLSenclen(s);
			while (--ccc)
				if ((c = *p++) && (scc = *s++) && (c != scc))
					return (0);
		}
		return(scc ? gmatch(s, p) : 0);

	case '?':
		/* Don't match ? against just a fshift */
		if (NLSfontshift(scc&STRIP))
			s += NLSenclen(s-1) - 1;
		return(scc ? gmatch(s, p) : 0);

	case '*':
		while (*p == '*')
			p++;

		if (*p == 0)
			return(1);
		--s;
		while (*s)
		{
			if (gmatch(s++, p))
				return(1);
			/* don't match * against just a font-shift */
			if (NLSfontshift(s[-1])) 
				s += NLSenclen(s-1) - 1;
			if (NLSfontshift(s[-1]))
				 s++;
		}
		return(0);

	case 0:
		return(scc == 0);
	}
}


static int
addg(as1, as2, as3)
uchar_t	*as1, *as2, *as3;
{
	register uchar_t	*s1, *s2;
	register int	c;
	register int	j;

	s2 = locstak() + BYTESPERWORD;
	s1 = as1;
	while (c = *s1++)
	{
		if ((c &= STRIP) == 0)
		{
			needmem(s2);
			*s2++ = '/';
			break;
		}
		needmem(s2);
		*s2++ = c;
		if (NLSfontshift (c)) {
			j = NLSenclen (s1-1);
			while (--j) {
				needmem (s2);
				*s2++ = *s1++;
			} /* End while */
		} /* End if */
	} /* End while */
	s1 = as2;
	while (1) {
		needmem(s2);
		if(!(*s2 = *s1++))
			break;
		s2++;
	}
	if (s1 = as3)
	{
		needmem(s2);
		*s2++ = '/';
		do
			needmem(s2);
		while (*s2++ = *++s1);
	}
	makearg(endstak(s2));
}

makearg(args)
register struct argnod *args;
{
	args->argnxt = gchain;
	gchain = args;
}
