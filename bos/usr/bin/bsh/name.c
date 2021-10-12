static char sccsid[] = "@(#)09	1.32  src/bos/usr/bin/bsh/name.c, cmdbsh, bos411, 9428A410j 5/2/94 11:19:24";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: syslook setlist setname replace dfault assign readvar assnum
 *	      make lookup scanset chkid namscan namwalk printnam staknam
 *	      exname printro printexp setup_env addNode pushnam sh_setenv
 *	      findnam unset_name check_nls_and_locale call_putenv
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
 * 1.21  com/cmd/sh/sh/name.c, cmdsh, bos320, 9125320 6/6/91 23:11:00
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	<locale.h>
#include	"defs.h"
#include	"sym.h"

extern	BOOL	chkid();
extern	int	mailchk;
extern	int	timeout;

static	int	namwalk ();
extern	uchar_t	*simple ();
static	uchar_t	*staknam ();

      void    check_nls_and_locale (struct namnod *);
static        int     call_putenv (struct namnod *);


struct namnod cdpnod =          /* CDPATH */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	cdpname
};
struct namnod homenod =         /* HOME */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	homename
};
struct namnod ifsnod =          /* IFS */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	ifsname
};
struct namnod langnod =         /* LANG */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	lang
};
struct namnod lcallnod =         /* LC_ALL */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	lcall
};
struct namnod collatenod =         /* LC_COLLATE */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	collate
};
struct namnod ctypenod =         /* LC_CTYPE */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	ctype
};
struct namnod monetarynod =         /* LC_MONETARY */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	monetary
};
struct namnod lctimenod =         /* LC_TIME */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	lctime
};
struct namnod messagenod =         /* LC_MESSAGES */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	messages
};
struct namnod numericnod =         /* LC_NUMERIC */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	numeric
};
struct namnod locpathnod =         /* LOCPATH */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	locpath	
};
struct namnod mailnod =         /* MAIL */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	mailname
};
struct namnod mchknod =         /* MAILCHECK */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	mchkname
};
struct namnod mailmnod =        /* MAILMSG */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	mailmname
};
struct namnod mailpnod =        /* MAILPATH */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	mailpname
};
struct namnod nlspathnod =         /* NLSPATH */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	nlspath
};
struct namnod pathnod =         /* PATH */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	pathname
};
struct namnod ps1nod =          /* PS1 */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	ps1name
};
struct namnod ps2nod =          /* PS2 */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	ps2name
};
struct namnod acctnod =         /* SHACCT */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	acctname
};
struct namnod shellnod =          /* SHELL */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	shellname
};
struct namnod timenod =         /* TIMEOUT */
{
	(struct namnod *)NIL,
	(struct namnod *)NIL,
	timename
};

static	struct namnod	*namep =0;

/* ========	variable and string handling	======== */

syslook(w, syswds, n)
	register uchar_t *w;
	register struct sysnod syswds[];
	int n;
{
	int	low;
	int	high;
	int	mid;
	register int cond;

	if (w == 0 || *w == 0)
		return(0);

	w = NLSndecode(w);

	low = 0;
	high = n - 1;

	while (low <= high)
	{
		mid = (low + high) / 2;

		if ((cond = cf(w, syswds[mid].sysnam)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(syswds[mid].sysval);
	}
	return(0);
}

setlist(arg, xp)
register struct argnod *arg;
int	xp;
{
	if (flags & exportflg)
		xp |= N_EXPORT;

	while (arg)
	{
		register uchar_t *s = mactrim(arg->argval);
		s = NLSndecode(s);
		setname(s, xp);
		arg = arg->argnxt;
		if (flags & execpr)
		{
			prs(s);
			if (arg)
				blank();
			else
				newline();
		}
	}
}


setname(argi, xp)	/* does parameter assignments */
uchar_t	*argi;
int	xp;
{
	register uchar_t *argscan = argi;
	register struct namnod *n;

	extern uchar_t *setpos;

	chkid(argscan);
	if (argscan = setpos) {
			*argscan = 0;   /* make name a cohesive string */
			n = lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			if (xp & N_ENVNAM) {
				n->namenv = n->namval = argscan;
				check_nls_and_locale (n);
			}
#ifdef _AIX
			else
				assign(n, argscan);
#else
			else {
				assign(n, argscan);
				if ( n == &langnod ) { /* export LANG */
					attrib(n, N_EXPORT);
					check_nls_and_locale (n);
				}
			}
#endif
			return;
	}
	failed(argi, MSGSTR(M_NOTID,(char *)notid));
}

replace(a, v)
register uchar_t	**a;
uchar_t	*v;
{
	free(*a);
	*a = make(v);
}

dfault(n, v)
struct namnod *n;
uchar_t	*v;
{
	if (n->namval == 0)
		assign(n, v);
}

assign(n, v)
struct namnod *n;
uchar_t	*v;
{
	if (n->namflg & N_RDONLY)
		failed(n->namid, MSGSTR(M_WTFAILED,(char *)wtfailed));
	else if (flags & rshflg)
	{
		if (n == &pathnod || eq(n->namid, "SHELL"))
			failed(n->namid, MSGSTR(M_RESTRICTED,(char *)restricted));
	}
	else if (n->namflg & N_FUNCTN)
	{
		func_unhash(n->namid);
		freefunc(n);

		n->namenv = 0;
		n->namflg = N_DEFAULT;
	}

	if (n == &mchknod)
	{
		mailchk = stoi(v);
	}
	else if (n == &timenod)
	{
		if(*v)
			timeout = stoi(v);
		else
			timeout = 0;
	}
		
#ifdef NLSDEBUG
	debug("assign fr", n->namid);
	debug("assign to", v);
#endif
	replace(&n->namval, v);
	attrib(n, N_ENVCHG);

	if (n == &pathnod)
	{
		zaphash();
		set_dotpath();
		return;
	}
#ifdef _AIX
	else if ((n == &langnod) && (n->namflg & N_EXPORT))
	{
		check_nls_and_locale(n);
	}
#endif

	if (flags & prompt)
	{
		if ((n == &mailpnod) || (n == &mailnod && mailpnod.namflg == N_DEFAULT))
			setmail(n->namval);
	}
}

readvar(names)
uchar_t	**names;
{
	struct fileblk	fb;
	register struct fileblk *f = &fb;
	register uchar_t  *c;
	extern uchar_t    *nextwc();
	uchar_t		ifsch[100], *ifs = ifsch;
	register int	rc = 0;
	struct namnod *n = lookup(*names++);	/* done now to avoid storage mess */
	uchar_t	*rel = (uchar_t *)relstak();
	extern	uchar_t	readvarQuoted ;		/**	5A APAR 4138	**/

	push(f);
	initf(dup(0));

	if (lseek(0, 0L, 1) == -1)
		f->fsiz = 1;

	/*
	 * strip leading IFS characters
	 */
	readvarQuoted = 0 ;
	if (!NLSisencoded(ifsnod.namval)) {
		NLSencode (ifsnod.namval, ifs, 100);
		NLSskiphdr (ifs);
	} else
		ifs = ifsnod.namval;
	while ((NLany((c = nextwc()), ifs)) && !(eolchar(*c)))
	{
#ifdef NLSDEBUG
		debug ("readvar - leading IFS stripped", ifs);
#endif
	}
	/* If c is an EOF or newline character, do not push FNLS. */  
	if (!eolchar(*c))
		pushstak (FNLS);

	for (;;)
	{
		if ((*names && NLany(c, ifs)) || eolchar(*c))
		{
			zerostak();
			assign(n, absstak(rel));
			setstak(rel);
			if (*names)
				n = lookup(*names++);
			else
				n = 0;
			if (eolchar(*c))
			{
				break;
			}
			else		/* strip imbedded IFS characters */
			{
				while ((NLany((c = nextwc()), ifs)) &&
					!(eolchar(*c)))
# ifdef NLSDEBUG
					debug("readvar - embedded IFS stripped",
						ifs);
# endif
					;
				/* If c is an EOF or newline character, do not push FNLS. */  
				if (!eolchar(*c))
					pushstak (FNLS);
			}
		}
		else
		{
		int j;
		pushstak (*c);
		if (*c == (ESCAPE | QUOTE))	/* to handle quoted backslash */
		    pushstak(ESCAPE);
		j = NLSenclen (c);
		while (--j) {
			c++;
			pushstak (*c);
		}
		c = nextwc();
		if (eolchar (*c)) 
		{
			uchar_t *ch, d, *sav;
			do {
			    ch = stakbot;
			    while (sav = ch, d = *ch & STRIP)
				if ((ch += NLSenclen (ch)) >= staktop)
				    break;
			    /* d will be ASCII character or magic font shift */
			    if (!NLany (sav, ifs))
				    break;
#ifdef NLSDEBUG
			    debug("readvar - trailing IFS stripped", ifs);
#endif
			} while ((staktop -= NLSenclen (sav)) > stakbot);
		}
		}
	}
	while (n)
	{
		assign(n, nullstr);
		if (*names)
			n = lookup(*names++);
		else
			n = 0;
	}

	if (eof)
		rc = 1;
	lseek(0, (long)(f->fnxt - f->fend), 1);
	pop();
	return(rc);
}

assnum(p, i)
uchar_t	**p;
int	i;
{
	itos(i);
	replace(p, numbuf);
}

uchar_t *
make(v)
uchar_t	*v;
{
	register uchar_t	*p;

	if (v)
	{
		movstr(v, p = (uchar_t *) malloc(length(v)));
		return(p);
	}
	else
		return(0);
}


struct namnod *
lookup(nam)
	register uchar_t	*nam;
{
	register struct namnod *nscan = namep;
	register struct namnod **prev;
	int		LR;
#ifdef NLSDEBUG
	debug("lookup",nam);
#endif
	if (!chkid(nam))
		failed(nam, MSGSTR(M_NOTID,(char *)notid));
	while (nscan)
	{
		if ((LR = cf(nam, nscan->namid)) == 0)
			return(nscan);

		else if (LR < 0)
			prev = &(nscan->namlft);
		else
			prev = &(nscan->namrgt);
		nscan = *prev;
	}
	/*
	 * add name node
	 */
	nscan = (struct namnod *)malloc(sizeof *nscan);
	nscan->namlft = nscan->namrgt = (struct namnod *)NIL;
	nscan->namid = make(nam);
	nscan->namval = 0;
	nscan->namflg = N_DEFAULT;
	nscan->namenv = 0;
	return(*prev = nscan);
}

/* cheap side-effect trick for the moment */
uchar_t *setpos;

uchar_t *
scanset(s)
uchar_t	*s;
{
	s = NLSndecode(s);
	chkid(s);
	return setpos;
}

BOOL
chkid(nam)
uchar_t    *nam;
{
	register uchar_t *cp = nam;
	wchar_t nlc;
	uchar_t c;
	setpos = 0;

		/* as a side-effect of chkid(), setpos is non-zero if nam */
		/* appears to be a valid envt. assignment; setpos points  */
		/* to the = in nam.                                       */

#ifdef NLSDEBUG
  /*  Arguments to chkid() should always be decoded. */
        debug ("chkid", cp);

#endif

	cp += mbtowc (&nlc, cp, MBMAX); 
	if (!NLSletter(nlc))
		return(FALSE);
	while (cp += mbtowc (&nlc, cp, MBMAX), nlc)
	{
		if (nlc == '=') 
		{
			setpos = cp -1;
			return (FALSE);
		}
		if (!NLSalphanum (nlc))
			return (FALSE);
	}
	return(TRUE);
}

static int (*namfn)();
namscan(fn)
	int	(*fn)();
{
	namfn = fn;
	namwalk(namep);
}

static int
namwalk(np)
register struct namnod *np;
{
	if (np)
	{
		namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	}
}

printnam(n)
struct namnod *n;
{
	register uchar_t	*s;

	sigchk();

	if (n->namflg & N_FUNCTN)
	{
		prs_buff(n->namid);
		prs_buff("(){\n");
		prf(n->namenv);
		prs_buff("\n}\n");
	}
	else if (s = n->namval)
	{
		prs_buff(n->namid);
		prc_buff('=');
		prs_buff(s);
		prc_buff(NL);
	}
}

static uchar_t *
staknam(n)
register struct namnod *n;
{
	register uchar_t	*p;

	needmem (staktop + length (n->namid) + length (n->namval));
	p = movstr(n->namid, staktop);
	p = movstr("=", p);
	p = movstr(n->namval, p);
	return(getstak(p + 1 - (uchar_t *)(stakbot)));
}

static int namec;

exname(n)
	register struct namnod *n;
{
	register int 	flg = n->namflg;

	if (flg & N_ENVCHG)
	{

		if (flg & N_EXPORT)
		{
			free(n->namenv);
			n->namenv = make(n->namval);
		}
		else
		{
			free(n->namval);
			n->namval = make(n->namenv);
		}
	}

	
	if (!(flg & N_FUNCTN))
		n->namflg = N_DEFAULT;

	if (n->namval)
		namec++;

}

printro(n)
register struct namnod *n;
{
	if (n->namflg & N_RDONLY)
	{
		prs_buff(readonly);
		prc_buff(SP);
		prs_buff(n->namid);
		prc_buff(NL);
	}
}

printexp(n)
register struct namnod *n;
{
	if (n->namflg & N_EXPORT)
	{
		prs_buff(export);
		prc_buff(SP);
		prs_buff(n->namid);
		prc_buff(NL);
	}
}

setup_env()
{
	register uchar_t **e = (uchar_t **)environ;

	/* sort builtin envt. names according to strcmp order */
	addNode(&mailnod);         /* MAIL */
	addNode(&ifsnod);          /* IFS */
	addNode(&homenod);         /* HOME */
	addNode(&cdpnod);          /* CDPATH */
	addNode(&collatenod);      /* LC_COLLATE */
	addNode(&langnod);         /* LANG */
	addNode(&monetarynod);     /* LC_MONETARY */
	addNode(&lcallnod);        /* LC_ALL */
	addNode(&ctypenod);        /* LC_CTYPE */
	addNode(&lctimenod);       /* LC_TIME */
	addNode(&messagenod);      /* LC_MESSAGES */
	addNode(&numericnod);      /* LC_NUMERIC */
	addNode(&locpathnod);      /* LOCPATH */
	addNode(&acctnod);         /* SHACCT */
	addNode(&pathnod);         /* PATH */
	addNode(&mailmnod);        /* MAILMSG */
	addNode(&mchknod);         /* MAILCHECK */
	addNode(&mailpnod);        /* MAILPATH */
	addNode(&nlspathnod);      /* NLSPATH */
	addNode(&ps1nod);          /* PS1 */
	addNode(&ps2nod);          /* PS2 */
	addNode(&shellnod);        /* SHELL */
	addNode(&timenod);         /* TIMEOUT */
	while (*e)
		setname(*e++, N_ENVNAM);

}

addNode(n)
register struct namnod *n;
{
	register struct namnod **np = &namep;
	n->namlft = n->namrgt = 0;
	while(*np)
	{
		if (strcmp(n->namid, (*np)->namid) < 0)
		       np = &(*np)->namlft;
		else
		       np = &(*np)->namrgt;
	}
	*np = n;
}


static uchar_t **argnam;

pushnam(n)
struct namnod *n;
{
	if (n->namval)
		*argnam++ = staknam(n);
}

uchar_t **
sh_setenv()
{
	register uchar_t	**er;

	namec = 0;
	namscan(exname);

	argnam = er = (uchar_t **)getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return(er);
}

struct namnod *
findnam(nam)
	register uchar_t	*nam;
{
	register struct namnod *nscan = namep;
	int             LR;

	if (strcmp(nam,"SHELL")!=0)        /* special case main() */
	       nam = NLSndecode(nam);

	if (!chkid(nam))
		return(0);
	while (nscan)
	{
		if ((LR = cf(nam, nscan->namid)) == 0)
			return(nscan);
		else if (LR < 0)
			nscan = nscan->namlft;
		else
			nscan = nscan->namrgt;
	}
	return(0); 
}


unset_name(name)
	register uchar_t 	*name;
{
	register struct namnod	*n;
	uchar_t			empty_string = 0;

	if (n = findnam(name))
	{
		if (n->namflg & N_RDONLY)
			NLSfailed(name, MSGSTR(M_WTFAILED,(char *)wtfailed));

		if (n == &pathnod ||
		    n == &ifsnod ||
		    n == &ps1nod ||
		    n == &ps2nod ||
		    n == &mchknod)
		{
			failed(name, MSGSTR(M_BADUNSET,(char *)badunset));
		}
		if ((flags & rshflg) && eq(name, "SHELL"))
			failed(name, MSGSTR(M_RESTRICTED,(char *)restricted));
		if (n->namflg & N_FUNCTN)
		{
			func_unhash(name);
			freefunc(n);
		}
		else
		{
			free(n->namval);
			free(n->namenv);
		}

		n->namval = n->namenv = &empty_string;
		n->namflg = N_DEFAULT;

		check_nls_and_locale (n);
		n->namval = n->namenv = 0;

		if (flags & prompt)
		{
			if (n == &mailpnod)
				setmail(mailnod.namval);
			else if (n == &mailnod && mailpnod.namflg == N_DEFAULT)
				setmail(0);
			else if (n == &timenod)
				timeout = 0;
		}
	}
}


void check_nls_and_locale (struct namnod *n)
{
	if ( n == &nlspathnod || n == &langnod || n == &lcallnod ||
	     n == &locpathnod || n == &messagenod) {
		if ( call_putenv (n))
			return ;
                setlocale (LC_ALL, "");
                catclose (catd);
                catd = catopen (MF_BSH, NL_CAT_LOCALE);
		return ;
	}
	/* for the following LC_* variables, calling setlocale(LC_ALL, "")
	   is necessary because not only the particular category should
           be set, also the current LANG value should be reflected. */ 
	if ( n == &collatenod || n == &ctypenod || n == &monetarynod ||
	     n == &numericnod || n == &lctimenod ) {
		if ( call_putenv (n))
			return ;
		setlocale(LC_ALL, "");
	}
}


static int call_putenv(struct namnod *n)
{

	uchar_t	*env, *oldenv;
	int	env_off;

                        /*      compare old name to new name    */
        oldenv = (uchar_t *) getenv ( (char *)n->namid );
        if ((!(n->namval) && (oldenv == NULL)) ||
            (oldenv && n->namval && !strcmp (oldenv, n->namval)) )
                return (TRUE);
	env_off = (int) strlen (n->namid);
	if(n->namval)
		env = (uchar_t *)malloc (env_off + (int) strlen(n->namval) + 2);
	else
		env = (uchar_t *)malloc (env_off + 2);
	strcpy(env, n->namid);
	env[env_off] = '=';
	env[env_off+1] = (char) 0;
	if(n->namval)
		strcat (env, n->namval);
	putenv (env);
        return (FALSE);
}
