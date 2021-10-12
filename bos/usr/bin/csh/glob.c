static char sccsid[] = "@(#)24	1.33  src/bos/usr/bin/csh/glob.c, cmdcsh, bos411, 9428A410j 11/9/93 13:58:33";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: glob ginit collect acollect sort expand matchdir execbrc match 
 *            amatch Gmatch Gcat addpath rscan scan tglob trim tback globone 
 *            dobackp backeval psave pword
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include "sh.h"
#include <dirent.h>

#define ADDPATH(cs)				\
{						\
	int fclen;				\
	fclen = mblen((char *)cs,mb_cur_max);	\
	do					\
		addpath(*cs++);			\
	while (--fclen > 0);			\
}

/*
 * Local global variables.
 */
int		globcnt;
int		globbed;
static bool	noglob;
static bool	nonomatch;
const char	*globchars =	"{[*?";
uchar_t		*entp;
uchar_t		**sortbas;
uchar_t		*gpath, *gpathp, *lastgpathp;


uchar_t **
glob(register uchar_t **v)
{
	uchar_t agpath[PATH_MAX+1];
	uchar_t *agargv[GAVSIZ];

	gpath = agpath; gpathp = gpath; *gpathp = 0;
	lastgpathp = &gpath[sizeof agpath - 1];
	ginit(agargv); globcnt = 0;
#ifdef DEBUG
printf("glob entered: ");
blkpr(v);
printf("\n");
#endif
	noglob = (adrof("noglob") != 0);
	nonomatch = adrof("nonomatch") != 0;
	globcnt = noglob | nonomatch;
	while (*v)
		collect(*v++);
#ifdef DEBUG
printf("glob done, globcnt=%d, gflag=%d: ", globcnt, gflag);
blkpr(gargv);
printf("\n");
#endif
	if (globcnt == 0 && (gflag&1)) {
		blkfree(gargv), gargv = 0;
		return (0);
	} else
		return (gargv = copyblk(gargv));
}

void
ginit(uchar_t **agargv)
{

	agargv[0] = 0; gargv = agargv; sortbas = agargv; gargc = 0;
	gnleft = NCARGS - 4;
}

void
collect(register uchar_t *as)
{
	register int i;

	if (any_noquote('`', as)) {
#ifdef DEBUG
printf("doing backp of %s\n", as);
#endif
		dobackp(as, 0);
#ifdef DEBUG
printf("backp done, acollect'ing, pargc = %d\n", pargc);
#endif
		for (i = 0; i < pargc; i++)
			if (noglob)
			{
				pargv[i] = trim_nlquotes(pargv[i]);
				Gcat(pargv[i], (uchar_t *)"");
				sortbas = &gargv[gargc];
			}
			else
				acollect(pargv[i]);
		if (pargv)
			blkfree(pargv), pargv = 0;
#ifdef DEBUG
printf("acollect done\n");
#endif
	} else if (noglob || EQ(as, "{") || EQ(as, "{}")) {
		Gcat(as, (uchar_t *)"");
		sort();
	} else
		acollect(as);
}

void
acollect(register uchar_t *as)
{
	register int ogargc = gargc;

	gpathp = gpath; *gpathp = 0; globbed = 0;
	expand(as);
	if (gargc == ogargc) {
		if (nonomatch) {
			Gcat(as, (uchar_t *)"");
			sort();
		}
	} else
		sort();
}

void
sort(void)
{
	register uchar_t **p1, **p2, *c;
	uchar_t **Gvp = &gargv[gargc];

	p1 = sortbas;
	while (p1 < Gvp-1) {
		p2 = p1;
		while (++p2 < Gvp)
			if (strcmp(*p1, *p2) > 0)
				c = *p1, *p1 = *p2, *p2 = c;
		p1++;
	}
	sortbas = Gvp;
}

trim_nlquotes(as)
	uchar_t *as;
{
	uchar_t *cs;
	uchar_t *newas;

	cs = as;  /* cs traverses the as list */
	newas = as;  /* newas is the new as without NLQUOTES */

	while (*cs) {
	    register int  n;
	    wchar_t nlc;

	    if (*cs == NLQUOTE) 
		cs++;  		/* Skip over NLQUOTE characters */
	    else {
		n = mbtowc(&nlc, (char *)cs, mb_cur_max);
		if (n < 1) {
			n = 1;
			nlc = *cs & 0xff;
		}
		do
			*newas++ = *cs++;
		while (--n > 0);
	    }
	}
	*newas='\0';
	return(as);
}

void
expand(uchar_t *as)
{
	register uchar_t *cs;
	register uchar_t *sgpathp, *oldcs;
	struct stat stb;

	sgpathp = gpathp;
	cs = as;
#ifdef DEBUG
printf ("expand: string= %s(%d)\n", cs, strlen(cs));
#endif
	if (*cs == '~' && gpathp == gpath) {
		addpath('~');
		cs++;
		while (1) {
		    register int  n;
		    wchar_t nlc;

		    if (*cs == NLQUOTE) {
			cs++;
			ADDPATH(cs);
		    } else {
			n = mbtowc(&nlc, (char *)cs, mb_cur_max);
			if (n < 1) {
				n = 1;
				nlc = *cs & 0xff;
			}
			if (letter(nlc) || digit(nlc) || (nlc == '-')) {
			    do
				addpath(*cs++);
			    while (--n > 0);
			} else
			    break;
		    }
		}
		if (!*cs || *cs == '/') {
			if (gpathp != gpath + 1) {
				*gpathp = 0;
				if (gethdir(gpath + 1))  {
					char e[NL_TEXTMAX];

					sprintf(e,MSGSTR(M_UNKNOWN,
						"Unknown user: %s"), gpath + 1);
					error(e);
				}
				strcpy(gpath, gpath + 1);
			} else
				strcpy(gpath, value("home"));
			gpathp = (uchar_t *)strend(gpath);
		}
	}
	while ((*cs == 0) || (!strchr(globchars,*cs))) {
		if (*cs == 0) {
			if (!globbed)
				Gcat(gpath, (uchar_t *)"");
			else if (stat((char *)gpath, &stb) >= 0) {
				Gcat(gpath, (uchar_t *)"");
				globcnt++;
			}
			goto endit;
		}
		if (*cs == NLQUOTE) 
			cs++;
		ADDPATH(cs);
	}
	oldcs = cs;
	while (cs > as && *cs != '/') {
		cs--;
		if (*cs != NLQUOTE)
			gpathp--;
	}
	if (*cs == '/')
		cs++, gpathp++;
	*gpathp = 0;
	if (*oldcs == '{') {
		execbrc(cs, NOSTR);
		return;
	}
#ifdef DEBUG
printf ("expand: pre-matchdir cs = %s, gpath = %s(%d)\n",
		cs,gpath,strlen(gpath));
#endif
	matchdir(cs);
endit:
#ifdef DEBUG
printf ("leaving expand: as = %s, gpath = %s\n", as,gpath);
#endif
	gpathp = sgpathp;
	*gpathp = 0;
}

void
matchdir(uchar_t *pattern)
{
	register DIR   *dirf;
	register struct dirent *dirbuf;
	uchar_t d_name[FILENAME_MAX+1];
	register int cnt;

	dirf = opendir ( *gpath == 0 ? "." : (char *)gpath);
	if (dirf == NULL) {
		if (globbed)
			return;
		goto patherr;
	}
	while ( (dirbuf = readdir(dirf)) != NULL ) {
			strcpy(d_name, dirbuf->d_name);
			if (match(d_name, pattern)) {
				Gcat(gpath, d_name);
				globcnt++;
			}
	}
	closedir(dirf);
	return;

patherr:
	Perror((char *)gpath);
}

int
execbrc(uchar_t *p, uchar_t *s)
{
	uchar_t restbuf[BUFR_SIZ + 2];
	register uchar_t *pe, *pm, *pl;
	int brclev = 0;
	uchar_t *lm, savec, *sgpathp;
	register int n;

	lm = restbuf;
	while(*p != '{'){
		if (*p == NLQUOTE) 
			*lm++ = *p++;
		PUTSTR(lm,p);
	}
	for (pe = ++p; *pe; pe++)
	switch (*pe) {

	case '{':
		brclev++;
		continue;

	case '}':
		if (brclev == 0)
			goto pend;
		brclev--;
		continue;

	case '[':
		for (pe++; *pe;) {
			if (*pe == ']')
				break;
			if (*pe == NLQUOTE)
				pe++;
			n = mblen((char *)pe, mb_cur_max);
			if (n > 0)
				pe += n;
			else
				pe++;
		}
		if (!*pe)
			error(MSGSTR(M_MISSBRK, "Missing ]"));
		continue;
	case NLQUOTE:
		++pe;
		/* fall through */
	default:
		n = mblen((char *)pe, mb_cur_max);
		if (n > 1)
			pe += n - 1;
	}
pend:
	if (brclev || !*pe)
		error(MSGSTR(M_MISSBRC, "Missing }"));
	for (pl = pm = p; pm <= pe; pm++)
	switch (*pm) {

	case '{':
		brclev++;
		continue;

	case '}':
		if (brclev) {
			brclev--;
			continue;
		}
		goto doit;

	case NLQUOTE:
		pm++;
		if (*pm != ',') {
			n = mblen((char *)pm, mb_cur_max);
			if ( n > 1)
				pm += n - 1;
			continue;
		}
		pm--;
		/* fall through */
	case ',':
		if (brclev)
			continue;
doit:
		savec = *pm;
		*pm = 0;
		strcpy(lm, pl);
		strcat(restbuf, pe + 1);
		*pm = savec;
		if (s == 0) {
			sgpathp = gpathp;
			expand(restbuf);
			gpathp = sgpathp;
			*gpathp = 0;
		} else if (amatch(s, restbuf))
			return (1);
		sort();
		if (savec == NLQUOTE) pm++;
		pl = pm + 1;
		continue;

	case '[':
		for (pm++; *pm;) {
			if (*pm == ']')
				break;
			if (*pm == NLQUOTE)
				pm++;
			n = mblen((char *)pm, mb_cur_max);
			if (n > 0)
				pm += n;
			else
				pm++;
		}
		if (!*pm)
			error(MSGSTR(M_MISSBRK, "Missing ]"));
		continue;
	default:
		n = mblen((char *)pm, mb_cur_max);
		if ( n > 1)
			pm += n - 1;
	}
	return (0);
}

int
match(uchar_t *s, uchar_t *p)
{
	register int c;
	register uchar_t *sentp;
	uchar_t sglobbed = globbed;

	if (*s == '.' && *p != '.')
		return (0);
	sentp = entp;
	entp = s;
	c = amatch(s, p);
	entp = sentp;
	globbed = sglobbed;
	return (c);
}

int
amatch(uchar_t *s, uchar_t *p)
{
	register int  scc;
	uchar_t *sgpathp;
	struct stat stb;
	int c;
	register int ns;
	register int np;
	wchar_t nlc;

	globbed = 1;
	for (;;) {
		ns = mbtowc(&nlc, (char *)s, mb_cur_max);
		if (ns < 1)
			scc = *s++ & 0xff;
		else {
			s += ns;
			scc = nlc;
		}
		if (scc == NLQUOTE) {
			ns = mbtowc(&nlc, (char *)s, mb_cur_max);
			if (ns < 1)
				scc = *s++ & 0xff;
			else {
				s += ns;
				scc = nlc;
			}
		}	
		np = mbtowc(&nlc, (char *)p, mb_cur_max);
		if (np < 1)
			c = *p++ & 0xff;
		else {
			p += np;
			c = nlc;
		}
		switch (c) {

		case '{':
			return (execbrc(p - np, s - ns));

		case '[':
			{
			register int cc, ok, lc;
			ok = 0;
                        lc = -1;
			while (cc = *p) {
				np = mbtowc(&nlc, (char *)p, mb_cur_max);
				if (np < 1)
					cc = *p++ & 0xff;
				else {
					p += np;
					cc = nlc;
				}
				if (cc == ']') {
					if (ok)
						break;
					return (0);
				}
				/* Check for range */
				if ((cc == '-')  && (lc > 0)) {
				    	if (*p == NLQUOTE) {
						++p;
						np = mbtowc(&nlc, (char *)p, mb_cur_max);
						if (np < 1)
							cc = *p++ & 0xff;
						else {
							p += np;
							cc = nlc;
						}
					} else {
						np = mbtowc(&nlc, (char *)p, mb_cur_max);
						if (np < 1)
							cc = *p++ & 0xff;
						else {
							p += np;
							cc = nlc;
						}
					}

					if (scc >= lc && scc <= cc)
						ok++;
				/* Check for character class */
				} else if ((cc =='[') && (*p == ':')) {
					uchar_t ifbuf[32], *ib, *pp;
				    	pp = p + 1;
				    	ib = ifbuf;
					do {
						np = mbtowc(&nlc, (char *)pp, mb_cur_max);
						if (np > 0) {
							cc = nlc;
							pp += np;
						} else
							cc = *pp++ & 0xff;
						if (cc == '\0' || cc == '\n' || cc == '-' ||
							cc == '[' || ib + np > ifbuf+29)
							break;
						*ib++ = cc;
					} while (cc != ':');
					*--ib = '\0';
					if ((iswctype(scc, wctype((char *)ifbuf))) && (*pp ==']') && (*++pp == ']')) {
						ok++;
						p = pp;
					}
				/* Check character */
				} else {
				    	if (scc == cc)
						ok++;
					lc = cc;
				}
			}
			if (cc == 0)
				error(MSGSTR(M_MISSBRK, "Missing ]"));
			continue;
			}

		case '*':
			if (!*p)
				return (1);
			if (*p == NLQUOTE)
				p++;
			if (*p == '/') {
				p++;
				goto slash;
			}
#ifdef _AIX
			/* Work around for compiler problem */
			for (s = s - ns; *s;) {
#else
			for (s -= ns; *s;) {
#endif
				if (amatch(s, p))
					return (1);
				ns = mblen((char *)s, mb_cur_max);
				if (ns > 0)
					s += ns;
				else
					s++;
			}
			return (0);

		case 0:
			return (scc == 0);

		case NLQUOTE:
			np = mbtowc(&nlc, (char *)p, mb_cur_max);
			if (np < 1)
				c = *p++ & 0xff;
			else {
				p += np;
				c = nlc;
			}
			/* fall into ... */

		default:
			if (c != scc)
				return (0);
			continue;

		case '?':
			if (scc == 0)
				return (0);
			continue;

		case '/':
			if (scc)
				return (0);
slash:
			s = entp;
			sgpathp = gpathp;
			while (*s)
				addpath(*s++);
			addpath('/');
			if (stat((char *)gpath, &stb) == 0 && 
			    S_ISDIR(stb.st_mode))
				if (*p == 0) {
					Gcat(gpath, (uchar_t *)"");
					globcnt++;
				} else
					expand(p);
			gpathp = sgpathp;
			*gpathp = 0;
			return (0);
		}
	}
}

/*
 * Often called with a piece of an encoded string 
 */
int
Gmatch(uchar_t *s, uchar_t *p)
{
	register int    scc,c;
	register int ns;
	register int np;
	wchar_t nlc;

	ns = mbtowc(&nlc, (char *)s, mb_cur_max);
	if (ns < 1)
		scc = *s++ & 0xff;
	else {
		s += ns;
		scc = nlc;
	}
	np = mbtowc(&nlc, (char *)p, mb_cur_max);
	if (np < 1)
		c = *p++ & 0xff;
	else {
		p += np;
		c = nlc;
	}
	switch (c) {
	case '[':
		{
		bool ok;
		int lc;
		int notflag = 0;

		ok = 0;
		lc = -1;
		if (*p == '!') {
			notflag = 1;
			p++;
		}
		while (*p) {
			np = mbtowc(&nlc, (char *)p, mb_cur_max);
			if (np < 1)
				c = *p++ & 0xff;
			else {
				p += np;
				c = nlc;
			}
			if (c == ']')
				return(ok ? Gmatch(s, p) : 0);

			/* Check for range */
			if ((c == '-') && (lc > 0)) {
				np = mbtowc(&nlc, (char *)p, mb_cur_max);
				if (np < 1)
					c = *p++ & 0xff;
				else {
					p += np;
					c = nlc;
						}
				if (scc >= lc && scc <= c)
					ok += notflag ? 0 : 1;
				else
					ok += notflag ? 1 : 0;
			/* Check for character class */
			} else if ((c =='[') && (*p == ':')) {
				uchar_t	ifbuf[32], *ib, *pp;
				pp = p + 1;
				ib = ifbuf;
				do {
					np = mbtowc(&nlc, (char *)pp, mb_cur_max);
					if (np > 0) {
						c = nlc;
						pp += np;
					} else
						c = *pp++ & 0xff;
					if (c == '\0' || c == '\n' || c == '-' ||
						c == '[' || ib + np > ifbuf+29)
						break;
				} while (c != ']');
				*ib = '\0';
				if (iswctype(scc, wctype((char *)ifbuf))) {
					ok++;
					p = pp;
				}
			/* Check character */
			} else {
				lc = c;
				if (notflag)
					if (scc && scc != lc)
						ok++;
					else
						return(0);
				else if (scc == lc)
					ok++;
				}
			}
			return(0);
		}

	case NLQUOTE:
		np = mbtowc(&nlc, (char *)p, mb_cur_max);
		if (np < 1)
			c = *p++ & 0xff;
		else {
			p += np;
			c = nlc;
		}
	default:
		if (c != scc)
			return(0);
		return(scc ? Gmatch(s, p) : 0);

	case '?':
                return(scc ? Gmatch(s, p) : 0);

	case '*':
		while (*p == '*')
			p++;
		if (!*p)
			return(1);
		s -= ns;
		while (*s)
		{
			if (Gmatch(s, p))
				return(1);
			ns = mblen((char *)s, mb_cur_max);
			if (ns > 0)
				s += ns;
			else
				s++;
		}
		return(0);

	case 0:
		return(scc == 0);
	}
}

void
Gcat(register uchar_t *s1, register uchar_t *s2)
{

	gnleft -= strlen(s1) + strlen(s2) + 1;
	if (gnleft <= 0 || ++gargc >= GAVSIZ)
		error(MSGSTR(M_ARGS, "Arguments too long"));
	gargv[gargc] = 0;
	gargv[gargc - 1] = (uchar_t *)strspl(s1, s2);
}

void
addpath(uchar_t c)
{

	if (gpathp >= lastgpathp)
		error(MSGSTR(M_PATH, "Pathname or word too long"));
	*gpathp++ = c;
	*gpathp = 0;
}

void
rscan(register uchar_t **t, int (*f)(uchar_t))
{
	register uchar_t *p, c;

	while (p = *t++) {
		if (f == tglob)
			if (*p == '~')
				gflag |= 2;
			else if (EQ(p, "{") || EQ(p, "{}"))
				continue;
		/* do not process quoted or non-ASCII characters */
		while (*p) {
			register int n;
			wchar_t nlc;
			n = mbtowc(&nlc, (char *)p, mb_cur_max);
			if (n > 0)
				p += n;
			else
				nlc = *p++;
			if (nlc <= 0xff)
				if (nlc == NLQUOTE && f != Dtestq) {
					n = mbtowc(&nlc, (char *)p, mb_cur_max);
					if (n > 0)
						p += n;
					else
						nlc = *p++;
				} else
					(*f)(nlc);
		}
	}
}

void
scan(register uchar_t **t, void (*f)(void))
{
	register uchar_t *p, c;
	register uchar_t *q;

	while (p = q = *t++) {
		while (c = *p++)
			if (f == trim) {
				/* Amazing hack! do trim() here */
				if (c != NLQUOTE)
					p--;
				PUTSTR (q,p);
			}
			else {
				(*f);
				*q++;
			}
		*q = 0;
       }
}

int
tglob(register uchar_t c)
{

	if (strchr(globchars, c) || c == '`')
		gflag |= (c == '{') ? 2 : 1;
	return (c);
}

void
trim(void)
{
	/* should never be called */
	bferr(MSGSTR(M_INTERNAL, "Internal error - trim called\n"));
}

void
tback(uchar_t c)
{

	if (c == '`')
		gflag = 1;
}

uchar_t *
globone(register uchar_t *str)
{
	uchar_t *gv[2];
	register uchar_t **gvp;
	register uchar_t *cp;

	gv[0] = str;
	gv[1] = 0;
	gflag = 0;
	rscan(gv, tglob);
	if (gflag) {
		gvp = glob(gv);
		if (gvp == 0) {
			setname(str);
			bferr(MSGSTR(M_NOMATCH, "No match"));
		}
		cp = *gvp++;
		if (cp == 0)
			cp = (uchar_t *)"";
		else if (*gvp) {
			setname(str);
			bferr(MSGSTR(M_AMBIG, "Ambiguous"));
		} else
			cp = strip(cp);
		xfree((uchar_t *)gargv);
		gargv = 0;
	} else {
		scan(gv, trim);
		cp = savestr(gv[0]);
	}
	return (cp);
}

/*
 * Command substitute cp.  If literal, then this is
 * a substitution from a << redirection, and so we should
 * not crunch blanks and tabs, separating words only at newlines.
 */


uchar_t **
dobackp(uchar_t *cp, bool literal)
{
	register uchar_t *lp, *rp;
	uchar_t *ep;
	register int n;
	wchar_t nlc;
	static uchar_t word1[BUFR_SIZ];
	static uchar_t *apargv[GAVSIZ + 2];

	pargv = apargv;
	pargv[0] = NOSTR;
	pargcp = pargs = word1;
	pargc = 0;
	pnleft = sizeof(word1) -1; 
	for (;;) {
		for (lp = cp; *lp != '`';) {
			if (*lp == 0) {
				if (pargcp != pargs)
					pword();
#ifdef DEBUG
printf("leaving dobackp\n");
#endif
				return (pargv = copyblk(pargv));
			}
			if (*lp == NLQUOTE)
				psave(*lp++);
			n = mblen((char *)lp, mb_cur_max);
			do
				psave(*lp++);
			while (--n > 0);
		}
		lp++;
		for (rp = lp; *rp && *rp != '`';) {
			if ((*rp == '\\' || *rp == NLQUOTE) && !*++rp)	
				goto oops;
			n = mblen((char *)rp, mb_cur_max);
			if (n > 0)
				rp += n;
			else
				rp++;
		}
		if (!*rp)
oops:
			error(MSGSTR(M_NOBACK, "Unmatched `"));
		ep = savestr(lp);
		ep[rp - lp] = 0;
		backeval(ep, literal);
#ifdef DEBUG
printf("back from backeval\n");
#endif
		cp = rp + 1;
	}
}

void
backeval(uchar_t *cp, bool literal)
{
	int pvec[2];
	int quoted = (literal || (cp[0] == NLQUOTE)) ? QUOTE : 0;
	uchar_t ibuf[BUFR_SIZ];
	register int icnt = 0, c;
	register uchar_t *ip;
	bool hadnl = 0;
	uchar_t *fakecom[2];
	struct command faket;

	faket.t_dtyp = TCOM;
	faket.t_dflg = 0;
	faket.t_dlef = 0;
	faket.t_drit = 0;
	faket.t_dspr = 0;
	faket.t_dcom = fakecom;
	fakecom[0] = (uchar_t *)"` ... `";
	fakecom[1] = 0;
	/*
	 * We do the psave job to temporarily change the current job
	 * so that the following fork is considered a separate job.
	 * This is so that when backquotes are used in a
	 * builtin function that calls glob the "current job" is not corrupted.
	 * We only need one level of pushed jobs as long as we are sure to
	 * fork here.
	 */
	psavejob();
	/*
	 * It would be nicer if we could integrate this redirection more
	 * with the routines in sh.sem.c by doing a fake execute on a builtin
	 * function that was piped out.
	 */
	mypipe(pvec);
	if (pfork(&faket, -1) == 0) {
		struct wordent tmp_paraml;
		struct command *t;
		struct sigvec nsv, osv;

		close(pvec[0]);
		dmove(pvec[1], 1);
		dmove(SHDIAG, 2);
		initdesc();
		arginp = cp;
		strip(cp);
		lex(&tmp_paraml);
		if (err)
			error((char *)err);
		alias(&tmp_paraml);
		t = syntax(tmp_paraml.next, &tmp_paraml, 0);
		if (err)
			error((char *)err);
		if (t)
			t->t_dflg |= FPAR;
                nsv.sv_handler = SIG_IGN;
                nsv.sv_mask = SA_RESTART;
		nsv.sv_onstack = 0;
                (void)sigvec(SIGTSTP, &nsv, &osv);
                (void)sigvec(SIGTTIN, &nsv, &osv);
                (void)sigvec(SIGTTOU, &nsv, &osv);
		execute(t, -1, (int *)0, (int *)0);
		exitstat();
	}
	xfree(cp);
	close(pvec[1]);
	do {
		int cnt = 0;
		register int n;
		wchar_t nlc;

		for (;;) {
			if (icnt == 0) {
				ip = ibuf;
				while ((icnt = read(pvec[0], ip, sizeof(ibuf)))
				< 0 && errno == EINTR);
				if (icnt <= 0) {
					c = -1;
					break;
				}
			}
			if (hadnl)
				break;
			--icnt;
			/*
			 * Pick up character from output of `command`
			 */
			n = mbtowc(&nlc, (char *)ip, mb_cur_max);
			if (n < 1) {
				n = 1;
				c = *ip++ & 0xff;
			} else {
				ip += n;
				icnt -= n - 1;
				c = nlc;
			}
			if (c == 0)
				break;
			if (c == '\n') {
				/*
				 * Continue around the loop one
				 * more time, so that we can eat
				 * the last newline without terminating
				 * this word.
				 */
				hadnl = 1;
				continue;
			}
			if (!quoted && (c == ' ' || c == '\t' || iswblank(c)))
				break;
			cnt++;
			if (quoted)
				psave (NLQUOTE);
			do
				psave (ip[-n]);
			while (--n > 0);
		}
		/*
		 * Unless at end-of-file, we will form a new word
		 * here if there were character in the word, or in
		 * any case when we take text literally.  If
		 * we didn't make empty words here when literal was
		 * set then we would lose blank lines.
		 */
		if (c != -1 && (cnt || literal))
			pword();
		hadnl = 0;
	} while (c >= 0);
#ifdef DEBUG
printf("done in backeval, pvec: %d %d\n", pvec[0], pvec[1]);
printf("also c = %c <%o>\n", c, c);
#endif
	close(pvec[0]);
#ifdef DEBUG
printf("backeval, after close(pvec[0]==%d)\n",pvec[0]);
#endif
	pwait();
#ifdef DEBUG
printf("backeval, after pwait()\n");
#endif
	prestjob();
#ifdef DEBUG
printf("backeval, after prestjob()\n");
#endif
}

void
psave(uchar_t c)
{

	if (--pnleft <= 0)
		error(MSGSTR(M_WORD, "Word too long"));
	*pargcp++ = c;
}

void
pword(void)
{

	psave(0);
	if (pargc == GAVSIZ)
		error(MSGSTR(M_COMSUB, "Too many words from ``"));
	pargv[pargc++] = savestr(pargs);
	pargv[pargc] = NOSTR;
#ifdef DEBUG
printf("got word %s\n", pargv[pargc-1]);
#endif
	pargcp = pargs;
	pnleft = BUFR_SIZ - 4; 
}
