static char sccsid[] = "@(#)23	1.39  src/bos/usr/bin/csh/func.c, cmdcsh, bos411, 9428A410j 5/13/94 15:47:30";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: isbfunc func dolabel doonintr donohup dozip prvars doalias 
 *            unalias dologout dologin donewgrp islogin doif reexecute doelse
 *            dogoto doswitch dobreak doexit doforeach dowhile preread 
 *            doend docontin doagain dorepeat doswbrk srchx search getword 
 *            toend wfree doecho doglob echo dosetenv dounsetenv setcenv 
 *            unsetcenv doumask findlim dolimit getval limtail plim dounlimit
 *            setlim dosuspend doeval
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

#include "sh.h"
#include "pathnames.h"
#include <sys/ioctl.h>
#include <locale.h>
#include <unistd.h>
#include <ctype.h>

extern char	**environ;

void check_nls_and_locale(uchar_t *);

struct biltins *
isbfunc(register struct command *t)
{
	register uchar_t *cp = t->t_dcom[0];
	register uchar_t *dp;
	register struct biltins *bp;
	void dolabel(), dofg1(), dobg1();
	static struct biltins label = { (uchar_t *)"", dolabel, 0, 0 };
	static struct biltins foregnd = { (uchar_t *)"%job", dofg1, 0, 0 };
	static struct biltins backgnd = { (uchar_t *)"%job &", dobg1, 0, 0 };

	if (lastchr(cp) == ':') {
		label.bname = cp;
		return (&label);
	}
	if (*cp == '%') {
		if (t->t_dflg & FAND) {
			t->t_dflg &= ~FAND;
			backgnd.bname = cp;
			return (&backgnd);
		} 
		foregnd.bname = cp;
		return (&foregnd);
	}
	for (bp = bfunc; dp = bp->bname; bp++) {
		if (dp[0] == cp[0] && EQ(dp, cp))
			return (bp);
		if (dp[0] > cp[0])
			break;
	}
	return (0);
}

void
func(register struct command *t, register struct biltins *bp)
{
	int i;

	xechoit(t->t_dcom);
	setname(bp->bname);
	i = blklen(t->t_dcom) - 1;
	if (i < bp->minargs)
		bferr(MSGSTR(M_TOOFEW, "Too few arguments"));
	if (i > bp->maxargs)
		bferr(MSGSTR(M_TOOMANY, "Too many arguments"));
	(*bp->bfunct)(t->t_dcom, t);
	return;
}

void
dolabel(void)
{
return;
}

void
doonintr( uchar_t **v)
{
	register uchar_t *cp;
	register uchar_t *vv = v[1];
	struct sigvec nsv;

	if (parintr == SIG_IGN)
		return;
	if (setintr && intty)
		bferr(MSGSTR(M_CANT, "Can't from terminal"));
	cp = gointr;
	gointr = 0;
	xfree(cp);
	if (vv == 0) {
		if (setintr) {
			(void)sigblock(sigmask(SIGINT));
		}
		else  {
			nsv.sv_handler = SIG_DFL;
			nsv.sv_mask = 0;
			nsv.sv_onstack = 0;
			(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
		}
		gointr = 0;
	} else if (EQ((vv = strip(vv)), "-")) {
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
		gointr = (uchar_t *)"-";
	} else {
		gointr = savestr(vv);
		nsv.sv_handler = (void (*)(int))pintr;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGINT, &nsv, (struct sigvec *)NULL);
	}
}

void
donohup(void)
{
	struct sigvec nsv;	

	if (intty) {
		bferr(MSGSTR(M_CANT, "Can't from terminal"));
		return;
	}
	if (!loginsh && !intact) {
		nsv.sv_handler = SIG_IGN;
		nsv.sv_mask = 0;
		nsv.sv_onstack = 0;
		(void)sigvec(SIGHUP, &nsv, (struct sigvec *)NULL);
	}
}

void
dozip(void)
{
return;
}

void
prvars(void)
{
plist(&shvhed);
}

void
doalias(register uchar_t **v)
{
	register struct varent *vp;
	register uchar_t *p;

	v++;
	p = *v++;
	if (p == 0)
		plist(&aliases);
	else if (*v == 0) {
		vp = adrof1((char *)strip(p), &aliases);
		if (vp)
			blkpr(vp->vec), printf("\n");
		else
               		bferr(MSGSTR(M_NOMATCH, "No match"));
	} else {
		if (EQ(p, "alias") || EQ(p, "unalias")) {
			setname(p);
			bferr(MSGSTR(M_ALIAS, "Too dangerous to alias that"));
		}
		set1((char *)strip(p), saveblk(v), &aliases);
	}
}

void
unalias(uchar_t **v)
{
unset1(v, &aliases);
}

void
dologout(void)
{

	islogin();
	goodbye();
}

void
dologin(uchar_t **v)
{
	struct sigvec nsv;	

	islogin();
	rechist();
	nsv.sv_handler = parterm;
	nsv.sv_mask = 0;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTERM, &nsv, (struct sigvec *)NULL);
	execv(_PATH_LOGIN, (char**)v);
	untty();
	exitcsh(1);
}

void
donewgrp(uchar_t **v)
{
	struct sigvec nsv;	

	if (chkstop == 0 && setintr)
		panystop(0);
	nsv.sv_handler = parterm;
	nsv.sv_mask = 0;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTERM, &nsv, (struct sigvec *)NULL);
	execv(_PATH_NEWGRP, &v[0] );
	untty();
	exitcsh(1);
}

void
doinlib(uchar_t **v)
{
#ifdef _OSF
	if((v = glob(v)) == 0)
		error(MSGSTR(M_NOMATCH, "No match"));
	else {
		if(!ldr_install(v[1]))
			return;
		error(MSGSTR(M_BADINLIB, "Inlib install failed."));
	}
#endif
	return;
}

void
dormlib(uchar_t **v)
{
#ifdef _OSF
	if((v = glob(v)) == 0)
		error(MSGSTR(M_NOMATCH, "No match"));
	else {
		if(!ldr_remove(v[1]))
			return;
		error(MSGSTR(M_BADRMLIB, "rmlib removal failed."));
	}
#endif
	return;
}

void
islogin(void)
{

	if (chkstop == 0 && setintr)
		panystop(0);
	if (loginsh)
		return;
	error(MSGSTR(M_NOTLOGIN, "Not login shell"));
}

void
doif(uchar_t **v, struct command *kp)
{
	register int i;
	register uchar_t **vv;

	v++;
	i = exp(&v);
	vv = v;
	if (*vv == NOSTR)
		bferr(MSGSTR(M_EMPTYIF, "Empty if"));
	if (EQ(*vv, "then")) {
		if (*++vv)
			bferr(MSGSTR(M_THEN, "Improper then"));
		setname((uchar_t *)"then");
		/*
		 * If expression was zero, then scan to else,
		 * otherwise just fall into following code.
		 */
		if (!i)
			search(ZIF, 0, (uchar_t)NULL);
		return;
	}
	/*
	 * Simple command attached to this if.
	 * Left shift the node in this tree, munging it
	 * so we can reexecute it.
	 */
	if (i) {
		lshift(kp->t_dcom, vv - kp->t_dcom);
		reexecute(kp);
		if (didfds)
			donefds();
	}
}

/*
 * Reexecute a command, being careful not
 * to redo i/o redirection, which is already set up.
 */
void
reexecute(register struct command *kp)
{

	kp->t_dflg &= FSAVE;
	kp->t_dflg |= FREDO;
	/*
	 * If tty is still ours to arbitrate, arbitrate it;
	 * otherwise dont even set pgrp's as the jobs would
	 * then have no way to get the tty (we can't give it
	 * to them, and our parent wouldn't know their pgrp, etc.
	 */
	execute(kp, tpgrp > 0 ? tpgrp : -1, (int *)0, (int *)0);
}

void
doelse(void)
{

	search(ZELSE, 0, (uchar_t)NULL);
}

void
dogoto(uchar_t **v)
{
	register struct whyle *wp;
	uchar_t *lp;

	/*
	 * While we still can, locate any unknown ends of existing loops.
	 * This obscure code is the WORST result of the fact that we
	 * don't really parse.
	 */
	for (wp = whyles; wp; wp = wp->w_next)
		if (wp->w_end == 0) {
			search(ZBREAK, 0, (uchar_t)NULL);
			wp->w_end = btell();
		} else
			bseek(wp->w_end);
	search(ZGOTO, 0, lp = globone(v[1]));
	xfree(lp);
	/*
	 * Eliminate loops which were exited.
	 */
	wfree();
}

void
doswitch(register uchar_t **v)
{
	register uchar_t *cp, *lp;

	v++;
	if (!*v || *(*v++) != '(')
		goto syntax;
	cp = (**v == ')') ? (uchar_t *)"" : *v++;
	if (*(*v++) != ')')
		v--;
	if (*v)
syntax:
		error(MSGSTR(M_SYNERR, "Syntax error"));
	search(ZSWITCH, 0, lp = globone(cp));
	xfree(lp);
}

void
dobreak(void)
{

	if (whyles)
		toend();
	else
		bferr(MSGSTR(M_WHILE, "Not in while/foreach"));
}

void
doexit(uchar_t **v)
{

	if (chkstop == 0)
		panystop(0);
	/*
	 * Don't DEMAND parentheses here either.
	 */
	v++;
	if (*v) {
		set("status", putn(exp(&v)));
		if (*v)
			bferr(MSGSTR(M_EXPR, "Expression syntax"));
	}

	btoeof();
	if (intty)
		close(SHIN);
}

void
doforeach(register uchar_t **v)
{
	register uchar_t *cp;
	register struct whyle *nwp;
	int n;
	wchar_t nlc;

	v++;
	cp = strip(*v);
        if (*cp) {
            n = mbtowc(&nlc, (char *)cp, mb_cur_max);
	    if (n < 1) {
		n = 1;
		nlc = *cp & 0xff;
	    }
            if (letter(nlc))
                for (cp += n; *cp; cp += n) {
                    n = mbtowc(&nlc, (char *)cp, mb_cur_max);
		    if (n < 1) {
			n = 1;
			nlc = *cp & 0xff;
		    }
                    if (!alnum(nlc))
                        break;
                }
        }
        if (*cp || strlen(*v) >= 20*MB_LEN_MAX)
		bferr(MSGSTR(M_INVVAR, "Invalid variable"));
	cp = *v++;
	if (v[0][0] != '(' || v[blklen(v) - 1][0] != ')')
		bferr(MSGSTR(M_PAREN, "Words not ()'ed"));
	v++;
	gflag = 0;
	rscan(v, tglob);
        v = glob(v);
        if (v == 0)
                bferr(MSGSTR(M_NOMATCH, "No match"));
	nwp = (struct whyle *)calloc(1, sizeof *nwp);
	nwp->w_fe = nwp->w_fe0 = v; gargv = 0;
	nwp->w_start = btell();
	nwp->w_fename = savestr(cp);
	nwp->w_next = whyles;
	whyles = nwp;
	/*
	 * Pre-read the loop so as to be more
	 * comprehensible to a terminal user.
	 */
	if (intty)
		preread();
	doagain();
}

void
dowhile(uchar_t **v)
{
	register int status;
	register bool again = whyles != 0 && whyles->w_start == lineloc &&
	    whyles->w_fename == 0;

	v++;
	/*
	 * Implement prereading here also, taking care not to
	 * evaluate the expression before the loop has been read up
	 * from a terminal.
	 */
	if (intty && !again)
		status = !exp0(&v, 1);
	else
		status = !exp(&v);
	if (*v)
		bferr(MSGSTR(M_EXPR, "Expression syntax"));
	if (!again) {
		register struct whyle *nwp;

		nwp = (struct whyle *)calloc(1, sizeof (*nwp));
		nwp->w_start = lineloc;
		nwp->w_end = 0;
		nwp->w_next = whyles;
		whyles = nwp;
		if (intty) {
			/*
			 * The tty preread
			 */
			preread();
			doagain();
			return;
		}
	}
	if (status)
		/* We ain't gonna loop no more, no more! */
		toend();
}

void
preread(void)
{

	whyles->w_end = -1;
	if (setintr)
		sigrelse(SIGINT);
	search(ZBREAK, 0, (uchar_t)NULL);
	if (setintr)
		sighold(SIGINT);
	whyles->w_end = btell();
}

void
doend(void)
{

	if (!whyles)
		bferr(MSGSTR(M_WHILE, "Not in while/foreach"));
	whyles->w_end = btell();
	doagain();
}

void
docontin(void)
{

	if (!whyles)
		bferr(MSGSTR(M_WHILE, "Not in while/foreach"));
	doagain();
}

void
doagain(void)
{

	/* Repeating a while is simple */
	if (whyles->w_fename == 0) {
		bseek(whyles->w_start);
		return;
	}
	/*
	 * The foreach variable list actually has a spurious word
	 * ")" at the end of the w_fe list.  Thus we are at the
	 * of the list if one word beyond this is 0.
	 */
	if (!whyles->w_fe[1]) {
		dobreak();
		return;
	}
	set((char *)whyles->w_fename, savestr(*whyles->w_fe++));
	bseek(whyles->w_start);
}

void
dorepeat(uchar_t **v, struct command *kp)
{
	register int i;

	i = getn(v[1]);
	if (setintr)
		sighold(SIGINT);
	lshift(v, 2);
	while (i > 0) {
		if (setintr)
			sigrelse(SIGINT);
		reexecute(kp);
		--i;
	}
	if (didfds)
		donefds();
	if (setintr)
		sigrelse(SIGINT);
}

void
doswbrk(void)
{

	search(ZBRKSW, 0, (uchar_t)NULL);
}

int
srchx(register uchar_t *cp)
{
	register struct srch *sp;

	for (sp = srchn; sp->s_name; sp++)
		if (EQ(cp, sp->s_name))
			return (sp->s_value);
	return (-1);
}

uchar_t	Stype;
uchar_t	*Sgoal;

void
search(int type, register int level, uchar_t *goal)
{
	uchar_t wordbuf[BUFR_SIZ];
	register uchar_t *aword = wordbuf;
	register uchar_t *cp;
	extern uchar_t *linp, linbuf[];

	Stype = type; Sgoal = goal;
	if (type == ZGOTO)
		bseek(0L);
	do {
		if (intty && fseekp == feobp) {
			printf("? ");
			flush();
		}
		aword[0] = 0, getword(aword);
		switch (srchx(aword)) {

		case ZELSE:
			if (level == 0 && type == ZIF)
				return;
			break;

		case ZIF:
			while (getword(aword))
				continue;
			if ((type == ZIF || type == ZELSE) && EQ(aword, "then"))
				level++;
			break;

		case ZENDIF:
			if (type == ZIF || type == ZELSE)
				level--;
			break;

		case ZFOREACH:
		case ZWHILE:
			if (type == ZBREAK)
				level++;
			break;

		case ZEND:
			if (type == ZBREAK)
				level--;
			break;

		case ZSWITCH:
			if (type == ZSWITCH || type == ZBRKSW)
				level++;
			break;

		case ZENDSW:
			if (type == ZSWITCH || type == ZBRKSW)
				level--;
			break;

		case ZLABEL:
			if (type == ZGOTO && getword(aword) && EQ(aword, goal))
				level = -1;
			break;

		default:
			if (type != ZGOTO && (type != ZSWITCH || level != 0))
				break;
			if (lastchr(aword) != ':')
				break;
			aword[strlen(aword) - 1] = 0;
			if (type == ZGOTO && EQ(aword, goal) || type == ZSWITCH && EQ(aword, "default"))
				level = -1;
			break;

		case ZCASE:
			if (type != ZSWITCH || level != 0)
				break;
			getword(aword);
			if (lastchr(aword) == ':')
				aword[strlen(aword) - 1] = 0;
			cp = strip(Dfix1(aword));
			if (Gmatch(goal, cp))
				level = -1;
			xfree(cp);
			break;

		case ZDEFAULT:
			if (type == ZSWITCH && level == 0)
				level = -1;
			break;
		}
		getword(NOSTR);
	} while (level >= 0);
}

int
getword(register uchar_t *wp)
{
	register int found = 0;
	register int d;
	char         spacesInBetween;
	wint_t c;

	c = readc(1);
	d = 0;
	do {
		spacesInBetween = 1;
		while (c == ' ' || c == '\t' || iswblank(c))
			c = readc(1);
		if (c == '#')
			do
				c = readc(1);
			while (c >= 0 && c != '\n');
		if (c < 0)
			goto past;
		if (c == '\n') {
			if (wp)
				break;
			return (0);
		}
		unreadc(c);
		found = 1;
		do {
			c = readc(1);
			if ( d == 0 && (c == '(' || c == ')') )
                               if ( spacesInBetween == 1 )
                                       unreadc(' ');
                               else
                               {
                                       unreadc(c);
                                       c = ' ';
                                       spacesInBetween = 0;
                               }
			else
                               spacesInBetween = 2;

			if (c == '\\' && (c = readc(1)) == '\n')
				c = ' ';
			if (any(c, "'\""))
				if (d == 0)
					d = c;
				else if (d == c)
					d = 0;
			if (c < 0)
				goto past;
			if (wp)
				PUTCH (wp,c);
		}while ((d || c != ' ' && c != '\t' && !iswblank(c)) && c != '\n');
	} while (wp == 0);
	if ( spacesInBetween )
		unreadc(c);
	if (found)
		*--wp = 0;
	return (found);

past:
	switch (Stype) {

	case ZIF:
		bferr(MSGSTR(M_ZIF, "then/endif not found"));

	case ZELSE:
		bferr(MSGSTR(M_ZELSE, "endif not found"));

	case ZBRKSW:
	case ZSWITCH:
		bferr(MSGSTR(M_ZSWITCH, "endsw not found"));

	case ZBREAK:
		bferr(MSGSTR(M_ZBREAK, "end not found"));

	case ZGOTO:
		setname(Sgoal);
		bferr(MSGSTR(M_ZGOTO, "label not found"));
	}
	/*NOTREACHED*/
}

void
toend(void)
{

	if (whyles->w_end == 0) {
		search(ZBREAK, 0, (uchar_t)NULL);
		whyles->w_end = btell() - 1;
	} else
		bseek(whyles->w_end);
	wfree();
}

void
wfree(void)
{
	long seek_ptr = btell();

	while (whyles) {
		register struct whyle *wp = whyles;
		register struct whyle *nwp = wp->w_next;

		if (seek_ptr >= wp->w_start && 
		(wp->w_end == 0 || seek_ptr < wp->w_end))
			break;
		if (wp->w_fe0)
			blkfree(wp->w_fe0);
		if (wp->w_fename)
			xfree(wp->w_fename);
		xfree((uchar_t *)wp);
		whyles = nwp;
	}
}

void
doecho(uchar_t **v)
{
	echo(' ', v);
}

void
doglob(uchar_t **v)
{

	echo(0, v);
	flush();
}

void
echo(uchar_t sep, register uchar_t **v)
{
	register uchar_t *cp;
	int nonl = 0;

	if (setintr)
		sigrelse(SIGINT);
	v++;
	if (*v == 0)
		return;
	gflag = 0;
	rscan(v, tglob);
	if (gflag) {
		v = glob(v);
		if (v == 0)
			bferr(MSGSTR(M_NOMATCH, "No match"));
		if (*v == 0)
			return;
	} 
	scan(v, trim);
	if (sep && !strcmp((char *)*v, "-n")) {
		nonl++;
		v++;
	}
	while (cp = *v++) {
		register int c;
		/* quoting here is to print control uchar_ts as is */
		while (c = *cp++) {
			if (c < ' ' || c == 0177)
				display_char(NLQUOTE);
			display_char(c);
		}
		if (*v) {
			if (!sep)
				display_char(NLQUOTE);
			display_char(sep);
		}
	}
	if (sep && !nonl)
		display_char('\n');
	else
		flush();
	if (setintr)
		sighold(SIGINT);
	if (gargv)
		blkfree(gargv), gargv = 0;
}


void
dosetenv(register uchar_t **v)
{
	uchar_t *vp, *lp;

	v++;
	if ((vp = *v++) == (uchar_t *)0) {
		register uchar_t **ep;

		if (setintr)
			(void)sigsetmask(sigblock(0) & ~ sigmask(SIGINT));
		for (ep = (uchar_t **)environ; *ep; ep++)
			printf("%s\n", *ep);
		return;
	}
	if ((lp = *v++) == (uchar_t *)0) {
                int c = '=';
                if (any(c, vp)){
                        setname((uchar_t *) "setenv");
                        bferr((MSGSTR(M_SYNERR, "Syntax error")));
                }
                setcenv((char *)vp, (uchar_t *)"");
	}
	setcenv((char *)vp, lp = globone(lp));
	if (EQ(vp, "PATH")) {
		importpath(lp);
		dohash();
	}
        else 
		check_nls_and_locale(vp);
	xfree(lp);
}

void
dounsetenv(register uchar_t **v)
{

	/* increment v past the setenv command string to get to the */
        /* environment variable name being unset.                   */
	v++;

	do {
		/* Do while there are still names in the list to unset */
		/* If the name specified to unset has a * in it for */
                /* matching a pattern or for unsetting everything   */
                if (any('*', *v)) {
                        /* While variables still remain to be unset */
                        /* that match the pattern in *v             */
                        while (unsetcenv(*v) == 0)
                                ;
                        *v++;
                } else
                        /* Otherwise, merely unset the one variable */
			unsetcenv(*v++);
	}while (*v);
}

void
setcenv(char *name, uchar_t *value)
{
	register uchar_t **ep = (uchar_t **)environ;
	register uchar_t *cp, *dp;
	uchar_t *blk[2], **oep = ep;

#ifdef DEBUG
printf("debug: environ=0x%x\n", ep);
printf("debug: setcenv(%s, %s)\n", name, value);
#endif
	for (; *ep; ep++) {
#ifdef DEBUG
printf("debug:ep=0x%x, *ep=0x%x\n", ep, *ep);
printf("debug: *ep=%s\n", *ep);
printf("debug:**ep=%c\n", **ep);
#endif
		for (cp=(uchar_t *)name, dp =*ep; *cp && *cp == *dp; cp++, dp++)
{
#ifdef DEBUG
printf("debug: *dp=%c\n", *dp);
#endif
			continue;
}
#ifdef DEBUG
printf("debug name=%s\n",name);
#endif
		if (*cp != 0 || *dp != '=')
			continue;
		cp = (uchar_t *)strspl((uchar_t *)"=", value);
#ifdef DEBUG
printf("debug: before first free, *ep=%s\n", *ep);
#endif
		xfree(*ep);
		*ep = strspl((uchar_t *)name, cp);
#ifdef DEBUG
printf("debug: before second free, cp=%s\n", cp);
#endif
		xfree(cp);
		scan(ep, trim);
		return;
	}
#ifdef DEBUG
printf("here\n");
#endif
	blk[0] = strspl((uchar_t *)name, (uchar_t *)"=");
	blk[1] = 0;
#ifdef DEBUG
printf("here\n");
#endif
	environ = (char **)blkspl((uchar_t **)environ, (uchar_t **)blk);
#ifdef DEBUG
printf("here\n");
#endif
	xfree((uchar_t *)oep);
	setcenv(name, value);
}

int
unsetcenv(uchar_t *name)
{
	register uchar_t **ep = (uchar_t **)environ;
	register uchar_t *cp, *dp;
	uchar_t **oep = ep;
	char *ptr;

	for (; *ep; ep++) {
		for (cp = name, dp = *ep; *cp && *cp == *dp; cp++, dp++)
			continue;
		if ( (*cp != 0 || *dp != '=' ) && (*cp != '*') )
			continue;
		cp = *ep;
		*ep = (uchar_t *)NULL;
		environ = (char **)blkspl((uchar_t **)environ, ep+1);
		*ep = cp;
		/* ep points to a form of Variable=value */
		if ((ptr = strchr(*ep, '=')) != NULL) {
			*ptr++ = '\0';
			/* when unsetting a locale related variable, setlocale()
			   should be called to pick up the current locale info*/
			check_nls_and_locale(*ep);
		}
		xfree(cp);
		xfree((uchar_t *)oep);
		return(0);
	}
	return(1);
}

void
doumask(register uchar_t **v)
{
	register uchar_t *cp = v[1];
	register int i;

	if (cp == 0) {
		i = umask(0);
		umask(i);
		printf("%o\n", i);
		return;
	}
	i = 0;
	while (*cp >= '0' && *cp <= '7')
		i = i * 8 + *cp++ - '0';
	if (*cp || i > 0777 || i < 0)
		bferr(MSGSTR(M_MASK,"Improper mask"));
	umask(i);
}

struct limits {
        int     limconst;
        char    *limname;
        int     limdiv;
        char    *limscale;
} limits[] = {
        RLIMIT_CPU,     "cputime",      1,      "seconds",
        RLIMIT_FSIZE,   "filesize",     1024,   "kbytes",
        RLIMIT_DATA,    "datasize",     1024,   "kbytes",
        RLIMIT_STACK,   "stacksize",    1024,   "kbytes",
        RLIMIT_CORE,    "coredumpsize", 1024,   "kbytes",
        RLIMIT_RSS,     "memoryuse",    1024,   "kbytes",
#ifdef _OSF
	RLIMIT_NOFILE,  "descriptors",  1,      "files",
	RLIMIT_AS,      "addressspace", 1024,   "kbytes",
#endif
        -1,             0,
};


struct limits *
findlim(uchar_t *cp)
{
	register struct limits *lp, *res;

	res = 0;
	for (lp = limits; lp->limconst >= 0; lp++)
		if (prefix(cp, (uchar_t *)lp->limname)) {
			if (res)
				bferr(MSGSTR(M_AMBIG, "Ambiguous"));
			res = lp;
		}
	if (res)
		return (res);
	bferr(MSGSTR(M_NOLIMIT, "No such limit"));
}

void
dolimit(register uchar_t **v)
{
	register struct limits *lp;
	register int limit;
	uchar_t hard = 0;

	v++;
        if (*v && EQ(*v, "-h")) {
                hard = 1;
                v++;
        }
	if (*v == 0) {
		for (lp = limits; lp->limconst >= 0; lp++)
			plim(lp, hard);
		return;
	}
	lp = findlim(v[0]);
	if (v[1] == 0) {
		plim(lp, hard);
		return;
	}
	limit = getval(lp, v+1);
        if (setlim(lp, hard, limit) < 0)
                error((char *)NOSTR);
}

int
getval(register struct limits *lp, uchar_t **v)
{
	register float f;
	char **ptr;
	uchar_t *cp = *v++;

	f = strtod((char *)cp,(char **)NULL);
	while (digit(*cp) || *cp == '.' || *cp == 'e' || *cp == 'E')
		cp++;
	if (*cp == 0) {
		if (*v == 0)
			return ((int)(f+0.5) * lp->limdiv);
		cp = *v;
	}
	switch (*cp) {

	case ':':
		if (lp->limconst != RLIMIT_CPU)
			goto badscal;
		return ((int)(f * 60.0 + atof((char *)cp+1)));

	case 'h':
		if (lp->limconst != RLIMIT_CPU)
			goto badscal;
		limtail(cp, "hours");
		f *= 3600.;
		break;

	case 'b':
		if (lp->limconst == RLIMIT_CPU)
			goto badscal;
		limtail(cp, "blocks");
		f *= 512.;
		break;
	case 'm':
		if (lp->limconst == RLIMIT_CPU) {
			limtail(cp, "minutes");
			f *= 60.;
			break;
		}
	case 'M':
		if (lp->limconst == RLIMIT_CPU)
			goto badscal;
		*cp = 'm';
		limtail(cp, "megabytes");
		f *= 1024.*1024.;
		break;

	case 's':
		if (lp->limconst != RLIMIT_CPU)
			goto badscal;
		limtail(cp, "seconds");
		break;

	case 'k':
		if (lp->limconst == RLIMIT_CPU)
			goto badscal;
		limtail(cp, "kbytes");
		f *= 1024;
		break;

	case 'u':
		limtail(cp, "unlimited");
		return (RLIM_INFINITY);

	default:
badscal:
		bferr(MSGSTR(M_SCALE, "Improper or unknown scale factor"));
	}
	return ((int)(f+0.5));
}

void
limtail(uchar_t *cp, char *str0)
{
	register uchar_t *str = (uchar_t *)str0;

	while (*cp && *cp == *str)
		cp++, str++;
	if (*cp)
		error(MSGSTR(M_BAD, "Bad scaling"));
}

void
plim(register struct limits *lp, uchar_t hard)
{
	struct rlimit rlim;
	int lim;

	printf("%s \t", lp->limname);
        (void) getrlimit(lp->limconst, &rlim);
        lim = hard ? rlim.rlim_max : rlim.rlim_cur;
	if (lim == RLIM_INFINITY)
		printf(MSGSTR(M_UNLIMITED, "unlimited"));
	else if (lp->limconst == RLIMIT_CPU)
		psecs((long)lim);
	else
		printf("%d %s", lim / lp->limdiv, lp->limscale);
	printf("\n");
}

void
dounlimit(register uchar_t **v)
{
        register struct limits *lp;
        int err = 0;
        uchar_t hard = 0;

        v++;
        if (*v && EQ(*v, "-h")) {
                hard = 1;
                v++;
        }
        if (*v == 0) {
                for (lp = limits; lp->limconst >= 0; lp++)
                        if (setlim(lp, hard, (int)RLIM_INFINITY) < 0)
                                err++;
                if (err)
                        error((char *)NOSTR);
                return;
        }
        while (*v) {
                lp = findlim(*v++);
                if (setlim(lp, hard, (int)RLIM_INFINITY) < 0)
                        error((char *)NOSTR);
        }
}

int
setlim(register struct limits *lp, uchar_t hard, int limit)
{
        struct rlimit rlim;

        (void) getrlimit(lp->limconst, &rlim);
        if (hard)
                rlim.rlim_max = limit;
        else if (limit == RLIM_INFINITY && geteuid() != 0)
                rlim.rlim_cur = rlim.rlim_max;
        else
                rlim.rlim_cur = limit;

       	if (setrlimit(lp->limconst, &rlim) < 0) {
		perror("");
               	printf("%s: %s: Can't %s%s limit\n", bname, lp->limname,
               		(limit == RLIM_INFINITY) ? "remove" : "set", 
			hard ? " hard" : "");
               	return (-1);
       	}
        return (0);
}

void
dosuspend(void)
{
	int old, ldisc;
	pid_t ctpgrp;
	struct sigvec nsv, osv;	

	if (loginsh)
		error(MSGSTR(M_SUSPEND, "Can't suspend a login shell (yet)"));
	untty();
	nsv.sv_handler = SIG_DFL;
	nsv.sv_mask = SA_RESTART;
	nsv.sv_onstack = 0;
	(void)sigvec(SIGTSTP, &nsv, &osv);
	kill(0, SIGTSTP);
	/* the shell stops here */
	(void)sigvec(SIGTSTP, &osv, (struct sigvec *)NULL);
	if (tpgrp != -1) {
retry:
		IOCTL(FSHTTY, TIOCGPGRP, &ctpgrp, "15");
		if (ctpgrp != opgrp) {
			(void)sigvec(SIGTTIN, &nsv, &osv);
			kill(0, SIGTTIN);
			(void)sigvec(SIGTTIN, &osv, (struct sigvec *)NULL);
			goto retry;
		}
		nsv.sv_handler = SIG_IGN;
		(void)sigvec(SIGTTOU, &nsv, &osv);
		IOCTL(FSHTTY, TIOCSPGRP, &shpgrp, "16");
		(void)sigvec(SIGTTOU, &osv, (struct sigvec *)NULL);
		setpgid(0, shpgrp);
	}
}

void
doeval(uchar_t **v)
{
	uchar_t **oevalvec = evalvec;
	uchar_t *oevalp = evalp;
	jmp_buf osetexit;
	volatile int reenter;
	uchar_t **gv = 0;

	v++;
	if (*v == 0)
		return;
	gflag = 0; 
        rscan(v, tglob);
	if (gflag) 
	{
		gv = glob(v);
		v = gv;
		gargv = 0;
		if (v == 0)
			error(MSGSTR(M_NOMATCH, "No match"));
		v = copyblk(v);
	} else
		scan(v, trim);
	getexit(osetexit);
	reenter = 0;
	setexit();
	reenter++;
	if (reenter == 1) {
		evalvec = v;
		evalp = 0;
		process(0);
	}
	evalvec = oevalvec;
	evalp = oevalp;
	doneinp = 0;
	if (gv)
		blkfree(gv);
	resexit(osetexit);
	if (reenter >= 2)
		error((char *)NOSTR);
}

void check_nls_and_locale(uchar_t *n)
{
	if (EQ(n, "NLSPATH") || EQ(n, "LANG") || EQ(n, "LC_ALL") ||
	    EQ(n, "LOCPATH") || EQ(n, "LC_MESSAGES")) {
		catclose(catd);
		setlocale(LC_ALL, "");
        	mb_cur_max = MB_CUR_MAX;
        	iswblank_handle = wctype("blank");
		catd = catopen(MF_CSH, NL_CAT_LOCALE);
		(void)MSGSTR(M_BYEBYE,""); /* dummy message call to set ptr */
		/* ensure that catd and catd->_fd are valid before passing it
		   to fileno() */
		if ((catd != CATD_ERR) && (catd->_fd != -1)) {
			FSHMSG = fileno(catd->_fd);
			track_open(FSHMSG);
		}
		return;
	}
	/* for the following LC_* variables, calling setlocale(LC_ALL, "")
	   is necessary because not only the particular category should
	   be set, also the current LANG value should be reflected. */
	if (EQ(n, "LC_COLLATE") || EQ(n, "LC_CTYPE") ||
	    EQ(n, "LC_MONETARY") || EQ(n, "LC_NUMERIC") ||
 	    EQ(n, "LC_TIME")) {
                setlocale(LC_ALL, "");
        	mb_cur_max = MB_CUR_MAX;
        	iswblank_handle = wctype("blank");
	}
}
