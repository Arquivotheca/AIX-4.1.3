static char sccsid[] = "@(#)28	1.25  src/bos/usr/bin/csh/lex.c, cmdcsh, bos411, 9428A410j 5/4/94 16:44:45";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: lex prlex copylex freelex word getC getdol addla getexcl 
 *            getsub dosub subword domod matchs getsel gethent findev noev 
 *            matchev setexclp unreadc readc bgetc bfree bseek btell 
 *            btoeof
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
 *
 *
 * These lexical routines read input and form lists of words.
 * There is some involved processing here, because of the complications
 * of input buffering, and especially because of history substitution.
 * 
 *
 * Peekc is a peek character for getC, peekread for readc.
 * There is a subtlety here in many places... history routines
 * will read ahead and then insert stuff into the input stream.
 * If they push back a character then they must push it behind
 * the text substituted by the history substitution.  On the other
 * hand in several places we need 2 peek character.  To make this
 * all work, the history routines read with getC, and make use both
 * of ungetC and unreadc.  The key observation is that the state
 * of getC at the call of a history reference is such that calls
 * to getC from the history routines will always yield calls of
 * readc, unless this peeking is involved.  That is to say that during
 * getexcl the variables lap, exclp, and exclnxt are all zero.
 *
 * Getdol invokes history substitution, hence the extra peek, peekd,
 * which it can ungetD to be before history substitutions.
 * 
 * The quoting mechanism for ILS characters is to precede each quoted
 * character by an NLQUOTE.
 */

#include "sh.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>

#define ungetC(c)       peekc = c
#define	ungetD(c)	peekd = c

/* savehist() will purge it if (eventno - np->Href) >= histlen */
#define HIST_PURGE    -50000000

#define	ungetE(c)	peeke = c
uchar_t		peekread[MB_LEN_MAX];
int		peekreadmb;
int		peeke;

/*
 * Local static and global variables.
 */
int		peekc, peekd;
int		exclc;		/* Count of remainig words in ! subst */
uchar_t		*exclp;		/* (Tail of) current word from ! subst */
uchar_t		*alvecp;	/* "Globp" for alias resubstitution */
struct wordent	*exclnxt;	/* The rest of the ! subst words */
bool		hadhist;

static uchar_t	*WORDMETA = (uchar_t *) "# '`\"\t;&<>()|\n";

/*
 * Lex returns to its caller not only a wordlist (as a "var" parameter)
 * but also whether a history substitution occurred.  This is used in
 * the main (process) routine to determine whether to echo, and also
 * when called by the alias routine to determine whether to keep the
 * argument list.
 */

int
lex(register struct wordent *hp)
{
	register struct wordent *wdp;
	wchar_t c;

	lineloc = btell();
	hp->next = hp->prev = hp;
	hp->word = (uchar_t *)"";
	alvecp = 0;
	hadhist = 0;
	do
		c = readc(0);
	while (iswblank(c));
	if (c == HISTSUB && intty)
		/* ^lef^rit	from tty is short !:s^lef^rit */
		getexcl(c);
	else
		unreadc(c);
	wdp = hp;
	/*
	 * The following loop is written so that the links needed
	 * by freelex will be ready and rarin to go even if it is
	 * interrupted.
	 */
	do {
		register struct wordent *new;

		new = (struct wordent *)calloc(1, sizeof *wdp);
		new->prev = wdp;
		new->next = hp;
		wdp->next = new;
		wdp = new;
		wdp->word = word();
	} while (wdp->word[0] != '\n');
	hp->prev = wdp;
	return (hadhist);
}

void
prlex(struct wordent *sp0)
{
	register struct wordent *sp = sp0->next;

	for (;;) {
		printf("%s", sp->word);
		sp = sp->next;
		if (sp == sp0)
			break;
		display_char(' ');
	}
}

#ifdef CMDEDIT
void
prpushlex(struct wordent *sp0)
{
        static uchar_t *buffer, *p, *q;
        static int buflen = 0;
        int len = 0;
        register struct wordent *sp = sp0->next;

        for (;;) {
                len += strlen(sp->word) + 1;
                sp = sp->next;
                if (sp->next == sp0)
                        break;
        }
        if (buflen < len) {
                if (buffer)
                        free(buffer);
                buffer = malloc(buflen = len);
        }
        p = buffer;
        sp = sp0->next;
        for (;;) {
                q = sp->word;
                while (*p = *q++)
                        p++;
                sp = sp->next;
                if (sp->next == sp0)
                        break;
                if (sp->word[0] != '\n')
                        *p++ = ' ';
        }
        *p = '\0';
        ShellTypeAheadToTenex = buffer;
}
#endif  /* CMDEDIT */

void
copylex(register struct wordent *hp, struct wordent *fp)
{
	register struct wordent *wdp;

	wdp = hp;
	fp = fp->next;
	do {
		register struct wordent *new = 
			(struct wordent *) calloc(1, sizeof(*wdp));

		new->prev = wdp;
		new->next = hp;
		wdp->next = new;
		wdp = new;
		wdp->word = savestr(fp->word);
		fp = fp->next;
	} while (wdp->word[0] != '\n');
	hp->prev = wdp;
}

void
freelex(register struct wordent *vp)
{
	register struct wordent *fp;

	while (vp->next != vp) {
		fp = vp->next;
		vp->next = fp->next;
		xfree(fp->word);
		xfree((uchar_t *)fp);
	}
	vp->prev = vp;
}


uchar_t *
word(void)
{
	register int c1;
	wchar_t c;
	register uchar_t *wp;
	uchar_t wbuf[BUFR_SIZ];
	register bool dolflg;

	wp = wbuf;
loop:
	c = getC(DOALL);
	if (iswblank(c))
		goto loop;
	switch (c) {
	case '`':
	case '\'':
	case '"':
		c1 = c;
		*wp++ = c;
		dolflg = c == '"' ? DOALL : DOEXCL;
		for (;;) {
			if (wp > wbuf + BUFR_SIZ - MB_LEN_MAX*2)
				goto toochars;
			c = getC(dolflg);
			if (c == c1)
				break;
			if (c == '\n') {
				seterrc(MSGSTR(M_UNMATCH, "Unmatched "), c1);
				ungetC(c);
				goto ret;
			}

			if (c == '\\') {
				c = getC(0);
				if (c == '\n') {
					*wp++ = '\\';
					*wp++ = NLQUOTE;
				} else if (c == HIST) {
					*wp++ = NLQUOTE;
				} else {
					ungetC(c);
					c = '\\';
				}
			}
			PUTCH(wp, c);
		}
		*wp++ = c;
		goto pack;

	case '&':
	case '|':
	case '<':
	case '>':
		*wp++ = c;
		c1 = getC(DOALL);
		if (c1 == c)
			*wp++ = c1;
		else
			ungetC(c1);
		goto ret;

	case '#':
		if (intty)
			break;
		if (wp != wbuf) {
			ungetC(c);
			goto ret;
		}
		c = 0;
		do {
			c1 = c;
			c = getC(0);
		} while (c != '\n');
		if (c1 == '\\')
			goto loop;
		/* fall into ... */

	case ';':
	case '(':
	case ')':
	case '\n':
		*wp++ = c;
		goto ret;

casebksl:
	case '\\':
		c = getC(0);
		if (c == '\n') {
			if (onelflg == 1)
				onelflg = 2;
			goto loop;
		}
		*wp++ = '\\';
		PUTCH(wp,c);
		goto pack;
	}
	if (c != NLQUOTE)
		ungetC(c);
	else {
		*wp++ = NLQUOTE;	/* quote any quote chars as input */
		*wp++ = c;
	}
pack:
	for (;;) {
		if (wp > wbuf + BUFR_SIZ - MB_LEN_MAX*2)
			goto toochars;
		c = getC(DOALL);
		if (c == '\\' || c == NLQUOTE) {
			c1 = c;
			c = getC(0);
			if (c == '\n') {
				if (onelflg == 1)
					onelflg = 2;
				goto ret;
			}
			*wp++ = c1;
		} else if (any(c, WORDMETA + intty) || iswblank(c)) {
			ungetC(c);
			if (any(c, (uchar_t *)"\"'`"))
				goto loop;
			goto ret;
		}
		PUTCH (wp, c);
	}
toochars:
	seterr(MSGSTR(M_WORD, "Word too long"));
	wp = &wbuf[1];
ret:
	*wp = 0;

	return (savestr(wbuf));
}

/*
 * Get next character process code
 */
int
getC(register int flag)
{
	register int n;		/* # bytes for character */
	wchar_t nlc;		/* character process code */
	register wchar_t c;
/*
 * Return character from ungetC() buffer
 * Return next character from $ expansion lap buffer
 */

top:
	if (c = peekc) {
		peekc = 0;
		return (c);
	}
	if (lap) {
		n = mbtowc(&nlc, (char *)lap, mb_cur_max);
		if (n != 0) {
			if (n < 0)
				c = *lap++ & 0xff;
			else {
				lap += n;
				c = nlc;
			}
			if (any(c, WORDMETA + intty) || iswblank(c)) {
				peekc = c;
				c = NLQUOTE;
			}
			return (c);
		} else
			lap = NULL;
	}
	if (c = peekd) {
		peekd = 0;
		return (c);
	}
	if (c = peeke) {
		peeke = 0;
		return (c);
	}
	if (exclp) {
		n = mbtowc(&nlc, (char *)exclp, mb_cur_max);
		if (n == 0)
			c = 0;
		else if (n < 0)
			c = *exclp++ & 0xff;
		else {
			exclp += n;
			c = nlc;
		}
		if (c != '\0')
			return (c);
		if (exclnxt && --exclc >= 0) {
			exclnxt = exclnxt->next;
			setexclp(exclnxt->word);
			return (' ');
		}
		exclp = 0;
		exclnxt = 0;
	}
	if (exclnxt) {
		exclnxt = exclnxt->next;
		if (--exclc < 0)
			exclnxt = 0;
		else
			setexclp(exclnxt->word);
		goto top;
	}
	c = readc(0);
	if (c == '$' && (flag & DODOL)) {
		getdol();
		goto top;
	}
	if (c == HIST && (flag & DOEXCL)) {
		getexcl(0);
		goto top;
	}
	return (c);
}

void
getdol(void)
{
	register uchar_t *np;
	uchar_t name[MAX_ENV_NAMLEN];
	register int c;
	int sc;
	bool special = 0;

	np = name, *np++ = '$';
	c = sc = getC(DOEXCL);
	if (any(c, (uchar_t *)"\t \n") || iswblank(c)) {
		ungetC(NLQUOTE);
		ungetD('$');
		ungetE(c);
		return;
	}
	if (c == '{')
		*np++ = c, c = getC(DOEXCL);
	if (c == '#' || c == '?')
		special++, *np++ = c, c = getC(DOEXCL);
	PUTCH (np,c);
	switch (c) {
	
	case '<':
	case '$':
		if (special)
			goto vsyn;
		goto ret;

	case '\n':
		ungetD(c);
		np--;
		goto vsyn;

	case '*':
		if (special)
			goto vsyn;
		goto ret;

	default:
		if (digit(c)) {
/*
 * let $?0 pass for now
			if (special)
				goto vsyn;
*/
			while ((c = getC(DOEXCL)) && digit(c)) {
				if (np < &name[sizeof name - 1])
					*np++ = c;
			}
		} else if (letter(c)) {
			c = getC(DOEXCL);
			while (alnum(c)) {
				if (np < &name[sizeof name - MB_LEN_MAX - 1])
					PUTCH (np,c);
				c = getC(DOEXCL);
			}
		} else
			goto vsyn;
	}
	if (c == '[') {
		*np++ = c;
		do {
			c = getC(DOEXCL);
			if (c == '\n') {
				ungetD(c);
				np--;
				goto vsyn;
			}
			if (np >= &name[sizeof name - 8])
				goto vsyn;
			PUTCH (np,c);
		} while (c != ']');
		c = getC(DOEXCL);
	}
	if (c == ':') {
		*np++ = c, c = getC(DOEXCL);
		if (c == 'g')
			*np++ = c, c = getC(DOEXCL);
		PUTCH (np,c);
		if (!any(c,(uchar_t *)"htrqxe"))
			goto vsyn;
	} else
		ungetD(c);
	if (sc == '{') {
		c = getC(DOEXCL);
		if (c != '}') {
			ungetC(c);
			goto vsyn;
		}
		*np++ = c;
	}
ret:
	*np = 0;
	addla(name);
	return;

vsyn:
	seterr(MSGSTR(M_VARSYN, "Variable syntax"));
	goto ret;
}

void
addla(uchar_t *cp)
{
	uchar_t buf[BUFR_SIZ];

	if (lap != 0 && strlen(cp) + strlen(lap) >= sizeof (labuf) - 4) {
		seterr(MSGSTR(M_LABUF, "Expansion buf ovflo"));
		return;
	}
	if (lap)
		strcpy(buf, lap);
	strcpy(labuf, cp);
	if (lap)
		strcat(labuf, buf);
	lap = labuf;
}

/*
 * These hold left hand side from
 * last substitution.
 */
uchar_t	lhsb[32*MB_LEN_MAX];
uchar_t	slhs[32*MB_LEN_MAX];
uchar_t	rhsb[64*MB_LEN_MAX];

int	quesarg;

void
getexcl(int sc)
{
	register struct wordent *hp, *ip;
	int left, right, dol;
	register wchar_t c;

	if (sc == 0) {
		sc = getC(0);
		if (sc != '{') {
			ungetC(sc);
			sc = 0;
		}
	}
	quesarg = -1;
	lastev = eventno;
	hp = gethent(sc);
	if (hp == 0)
		return;
	hadhist = 1;
	dol = 0;
	if (hp == alhistp)
		for (ip = hp->next->next; ip != alhistt; ip = ip->next)
			dol++;
	else
		for (ip = hp->next->next; ip != hp->prev; ip = ip->next)
			dol++;
	left = 0, right = dol;
	if (sc == HISTSUB) {
		ungetC('s'), unreadc(HISTSUB), c = ':';
		goto subst;
	}
	c = getC(0);
	if (!any(c, (uchar_t *)":^$*-%"))
		goto subst;
	left = right = -1;
	if (c == ':') {
		c = getC(0);
		unreadc(c);
		if (letter(c) || c == '&') {
			c = ':';
			left = 0, right = dol;
			goto subst;
		}
	} else
		ungetC(c);
	if (!getsel(&left, &right, dol))
		return;
	c = getC(0);
	if (c == '*')
		ungetC(c), c = '-';
	if (c == '-') {
		if (!getsel(&left, &right, dol))
			return;
		c = getC(0);
	}
subst:
	exclc = right - left + 1;
	while (--left >= 0)
		hp = hp->next;
	if (sc == HISTSUB || c == ':') {
		do {
			hp = getsub(hp);
			c = getC(0);
		} while (c == ':');
	}
	unreadc(c);
	if (sc == '{') {
		c = getC(0);
		if (c != '}')
			seterr(MSGSTR(M_BADEXCL, "Bad ! form"));
	}
	exclnxt = hp;
}

struct wordent *
getsub(struct wordent *en)
{
	register uchar_t *cp;
	int delim;
	register wchar_t c;
	int sc;
	bool global = 0;
	uchar_t orhsb[sizeof rhsb];

	exclnxt = 0;
	sc = c = getC(0);
	if (c == 'g')
		global++, c = getC(0);
	switch (c) {

	case 'p':
		justpr++;
		goto ret;

	case 'x':
	case 'q':
		global++;
		/* fall into ... */

	case 'h':
	case 'r':
	case 't':
	case 'e':
		break;

	case '&':
		if (slhs[0] == 0) {
			seterr(MSGSTR(M_SUB, "No prev sub"));
			goto ret;
		}
		strcpy(lhsb, slhs);
		break;


	case 's':
		delim = getC(0);
		if (letter(delim) || digit(delim) || any(delim,(uchar_t *)" \t\n")
		    || iswblank(delim)) {
			unreadc(delim);
bads:
			lhsb[0] = 0;
			seterr(MSGSTR(M_BADSUB, "Bad substitute"));
			goto ret;
		}
		cp = lhsb;
		for (;;) {
			c = getC(0);
			if (c == '\n') {
				unreadc(c);
				break;
			}
			if (c == delim)
				break;
			if (cp > &lhsb[sizeof lhsb - MB_LEN_MAX - 2])
				goto bads;
			if (c == '\\') {
				c = getC(0);
				if (c != delim && c != '\\')
					*cp++ = '\\';
			}
			PUTCH (cp,c);
		}
		if (cp != lhsb)
			*cp++ = 0;
		else if (lhsb[0] == 0) {
			seterr(MSGSTR(M_BADLHS, "No prev lhs"));
			goto ret;
		}
		cp = rhsb;
		strcpy(orhsb, cp);
		for (;;) {
			c = getC(0);
			if (c == '\n') {
				unreadc(c);
				break;
			}
			if (c == delim)
				break;
			if (cp > &rhsb[sizeof rhsb - MB_LEN_MAX - 2]) {
				seterr(MSGSTR(M_BADRHS, "Rhs too long"));
				goto ret;
			}
			if (c == '\\') {
				c = getC(0);
				if (c != delim /* && c != '~' */)
					*cp++ = '\\';
			}
			PUTCH (cp,c);
		}
		*cp++ = 0;
		break;

	default:
		if (c == '\n')
			unreadc(c);
		seterrc(MSGSTR(M_BADEXCLM, "Bad ! modifier: "), c);
		goto ret;
	}
	strcpy(slhs, lhsb);
	if (exclc)
		en = dosub(sc, en, global);
ret:
	return (en);
}

struct wordent *
dosub(int sc, struct wordent *en, bool global)
{
	struct wordent lex;
	bool didsub = 0;
	struct wordent *hp = &lex;
	register struct wordent *wdp;
	register int i = exclc;

	wdp = hp;
	while (--i >= 0) {
		register struct wordent *new = 
			(struct wordent *) calloc(1, sizeof(*wdp));

		new->prev = wdp;
		new->next = hp;
		wdp->next = new;
		wdp = new;
		en = en->next;
		wdp->word = global || didsub == 0 ?
		    subword(en->word, sc, &didsub) : savestr(en->word);
	}
	if (didsub == 0)
		seterr(MSGSTR(M_MODFAIL, "Modifier failed"));
	hp->prev = wdp;
	/* Defect 67506 */
	return (&enthist(HIST_PURGE, &lex, 0)->Hlex);
}

uchar_t *
subword(uchar_t *cp, int type, bool *adid)
{
	uchar_t wbuf[BUFR_SIZ];
	register uchar_t *wp, *mp, *np;
	register int n;
	wchar_t nlc;

	switch (type) {
	case 'r':
	case 'e':
	case 'h':
	case 't':
	case 'q':
	case 'x':
		wp = domod(cp, type);
		if (wp == 0)
			return (savestr(cp));
		*adid = 1;
		return (wp);
	}

	for (mp = cp; *mp;) {
		if (matchs(mp, lhsb)) {
			n = mp - cp;
			strncpy(wbuf, cp, n);
			wp = wbuf + n;
			for (np = rhsb; *np;)
				switch (*np) {

				case '\\':
					if (np[1] == '&')
						np++;
					/* fall into ... */

				default:
					n = mblen((char *)np, mb_cur_max);
					if (n < 0)
						n = 1;
					do
						*wp++ = *np++;
					while (--n > 0);
					if (wp > wbuf + BUFR_SIZ - MB_LEN_MAX - 1)
						goto ovflo;
					break;

				case '&':
					n = strlen(lhsb);
					if (wp > wbuf + BUFR_SIZ - 2 - n)
						goto ovflo;
					strncpy(wp, lhsb, n);
					wp += n;
					np++;
					break;
				}
			mp += strlen(lhsb);
			n = strlen(mp);
			if (wp > wbuf + BUFR_SIZ - 2 - n) {
ovflo:
				seterr(MSGSTR(M_OVFLO, "Subst buf ovflo"));
				return ((uchar_t *)"");
			}

			/* Defect 41205 */
			strncpy(wp, mp, n);
                        wp += n;
                        *wp = 0;
			*adid = 1;
			return (savestr(wbuf));
		}
		n = mblen((char *)mp, mb_cur_max);
		if (n < 0)
			n = 1;
		mp += n;
	}
	return (savestr(cp));
}

uchar_t *
domod(uchar_t *cp, int type)
{
	register uchar_t *wp, *xp;

	switch (type) {

	case 'x':
	case 'q':
		/* string must be big enough to NLQUOTE every character */
		wp = calloc(1, strlen(cp)*2 + 1);
		{
		register int n;
		wchar_t nlc;

		for (xp = wp; *cp;) {
			n = mbtowc(&nlc, (char *)cp, mb_cur_max);
			if (n < 0)
				*xp++ = NLQUOTE;
			else if (nlc == NLQUOTE)
				*xp++ = *cp++;
			else if (type == 'q' || !iswblank((wint_t)nlc))
				*xp++ = NLQUOTE;
			do
				*xp++ = *cp++;
			while (--n > 0);
		}
		return (wp);
		}

	case 'h':
	case 't':
		if (!any('/', cp))	/* what if :h :t are both the same? */
			return((int)NULL);
		wp = strend(cp);
		while (*--wp != '/')
			continue;
		if (type == 'h')
take:
			xp = savestr(cp), xp[wp - cp] = 0;
		else
			xp = savestr(wp + 1);
		return (xp);

	case 'e':
	case 'r':
		wp = strend(cp);
		for (wp--; wp >= cp && *wp != '/'; wp--)
			if (*wp == '.') {
				if (type == 'e')
					xp = savestr(wp + 1);
				else
					xp = savestr(cp), xp[wp - cp] = 0;
				return (xp);
			}
		return (savestr((type == 'e') ? (uchar_t *)"" : cp));
	}
	return((int)NULL);
}

int
matchs(register uchar_t *str, register uchar_t *pat)
{

	while (*str && *pat && *str == *pat)
		str++, pat++;
	return (*pat == 0);
}

int
getsel(register int *al, register int *ar, int dol)
{
	register wchar_t c = getC(0);
	register int i;
	bool first = *al < 0;

	switch (c) {

	case '%':
		if (quesarg == -1)
			goto bad;
		if (*al < 0)
			*al = quesarg;
		*ar = quesarg;
		break;

	case '-':
		if (*al < 0) {
			*al = 0;
			*ar = dol - 1;
			unreadc(c);
		}
		return (1);

	case '^':
		if (*al < 0)
			*al = 1;
		*ar = 1;
		break;

	case '$':
		if (*al < 0)
			*al = dol;
		*ar = dol;
		break;

	case '*':
		if (*al < 0)
			*al = 1;
		*ar = dol;
		if (*ar < *al) {
			*ar = 0;
			*al = 1;
			return (1);
		}
		break;

	default:
		if (digit(c)) {
			i = 0;
			while (digit(c)) {
				i = i * 10 + c - '0';
				c = getC(0);
			}
			if (*al < 0)
				*al = i;
			*ar = i;
		} else
			if (*al < 0)
				*al = 0, *ar = dol;
			else
				*ar = dol - 1;
		unreadc(c);
		break;
	}
	if (first) {
		c = getC(0);
		unreadc(c);
		if (any(c, (uchar_t *)"-$*"))
			return (1);
	}
	if (*al > *ar || *ar > dol) {
bad:
		seterr(MSGSTR(M_BADARG, "Bad ! arg selector"));
		return (0);
	}
	return (1);

}

struct wordent *
gethent(int sc)
{
	register struct Hist *hp;
	register uchar_t *np;
	register wchar_t c;
	int event;
	bool back = 0;

	c = (sc == HISTSUB) ? HIST : getC(0);
	if (c == HIST) {
		if (alhistp)
			return (alhistp);
		event = eventno;
		goto skip;
	}
	switch (c) {

	case ':':
	case '^':
	case '$':
	case '*':
	case '%':
		ungetC(c);
		if (lastev == eventno && alhistp)
			return (alhistp);
		event = lastev;
		break;

	case '-':
		back = 1;
		c = getC(0);
		goto number;

	case '#':		/* !# is command being typed in (mrh) */
		return(&paraml);
	default:
		if (any(c, (uchar_t *)"(=~")) {
			unreadc(c);
			ungetC(HIST);
			return((struct wordent *)NULL);
		}
		if (digit(c))
			goto number;
		np = lhsb;
		while (!any(c,(uchar_t *)": \t\\\n}") && !iswblank(c)) {
			if (np < &lhsb[sizeof lhsb - MB_LEN_MAX - 1])
				PUTCH (np,c);
			c = getC(0);
		}
		unreadc(c);
		if (np == lhsb) {
			ungetC(HIST);
			return((struct wordent *)NULL);
		}
		*np++ = 0;
		hp = findev(lhsb, 0);
		if (hp)
			lastev = hp->Hnum;
		else
			return((struct wordent *)NULL);
		return (&hp->Hlex);

	case '?':
		np = lhsb;
		for (;;) {
			c = getC(0);
			if (c == '\n') {
				unreadc(c);
				break;
			}
			if (c == '?')
				break;
			if (np < &lhsb[sizeof lhsb - MB_LEN_MAX - 1])
				PUTCH (np,c);
		}
		if (np == lhsb) {
			if (lhsb[0] == 0) {
				seterr(MSGSTR(NO_SEARCH, "No prev search"));
				return((struct wordent *)NULL);
			}
		} else
			*np++ = 0;
		hp = findev(lhsb, 1);
		if (hp)
			lastev = hp->Hnum;
		return (&hp->Hlex);

	number:
		event = 0;
		while (digit(c)) {
			event = event * 10 + c - '0';
			c = getC(0);
		}
		if (back)
			event = eventno + (alhistp == 0) - (event ? event : 0);
		unreadc(c);
		break;
	}
skip:
	for (hp = Histlist.Hnext; hp; hp = hp->Hnext)
		if (hp->Hnum == event) {
			hp->Href = eventno;
			lastev = hp->Hnum;
			return (&hp->Hlex);
		}
	np = putn(event);
	noev(np);
	return((struct wordent *)NULL);
}

struct Hist *
findev(uchar_t *cp, bool anyarg)
{
	register struct Hist *hp;

	for (hp = Histlist.Hnext; hp; hp = hp->Hnext)
		if (matchev(hp, cp, anyarg))
			return (hp);
	noev(cp);
	return((struct Hist *)NULL);
}

void
noev(uchar_t *cp)
{

	seterr2((char *)cp, (uchar_t *)MSGSTR(M_NOEVENT,": Event not found"));
}

int
matchev(register struct Hist *hp, uchar_t *cp, bool anyarg)
{
	register uchar_t *dp;
	struct wordent *lp = &hp->Hlex;
	int argno = 0;
	register int n;

	for (;;) {
		lp = lp->next;
		if (lp->word[0] == '\n')
			return (0);
		for (dp = lp->word; *dp;) {
			if (matchs(dp, cp)) {
				if (anyarg)
					quesarg = argno;
				return (1);
			}
			if (!anyarg)
				return (0);
		n = mblen((char *)dp, mb_cur_max);
		if (n < 0)
			n = 1;
		dp += n;
		}
		argno++;
	}
}

void
setexclp(register uchar_t *cp)
{

	if (cp && cp[0] == '\n')
		exclp = (uchar_t *) "";
	else
		exclp = cp;
	return;
}

void
unreadc(wchar_t c)
{
	char *mb_ptr = (char *)peekread;
	wctomb(mb_ptr, c);
}

/*
 * Read single byte character and return process code, or
 * Read multiple bytes and return process code, or
 * Return first byte of invalid character and preserve
 *   remaining bytes in peekreadmb (This works as long as
 *   MB_LEN_MAX is 5 or less because peekreadmb is an int.)
*/

static uchar_t ilsbuf[MB_LEN_MAX+1];
static uchar_t *pils = ilsbuf;

int
readc(bool wanteof)
{
	register int i, c;
	wchar_t nlc;

	memset(ilsbuf, 0, sizeof(ilsbuf));

	/* return single byte character or EOF */
	c = read_one_c(wanteof);
	if (c < 128 || mb_cur_max == 1)
		return (c);
	*pils++ = c;
	if (mbtowc(&nlc, (char *)ilsbuf, mb_cur_max) > 0) {
		pils = ilsbuf;
		return (nlc);
	}

	/* return multiple byte character */
	while ((pils - ilsbuf) < mb_cur_max) {
		c = read_one_c(wanteof);
		if (c < 0)
			break;
		*pils++ = c;
		if (mbtowc(&nlc, (char *)ilsbuf, mb_cur_max) > 0) {
			pils = ilsbuf;
			return (nlc);
		}
	}

	/* return first invalid byte and save remaining bytes */
	while (--pils > ilsbuf)
		peekreadmb = (peekreadmb << 8) + *pils;
	return (*pils);
}

int
read_one_c(bool wanteof)
{
	int c;
	static sincereal;

	if (c = peekread[0]) {
		strcpy(ilsbuf, peekread);
		memset(peekread, 0, sizeof(peekread));
		return (c);
	}
	if (c = peekreadmb) {
		peekreadmb = ((unsigned) peekreadmb >> 8) & 0x00ffffff;
		return (c & 0xff);
	}
top:
	if (alvecp) {
		if (c = *alvecp++)
			return (c);
		if (*alvec) {
			alvecp = *alvec++;
			return (' ');
		}
	}
	if (alvec) {
		if (alvecp = *alvec) {
			alvec++;
			goto top;
		}
		/* Infinite source! */
		return ('\n');
	}
	if (evalp) {
		if (c = *evalp++)
			return (c);
		if (*evalvec) {
			evalp = *evalvec++;
			return (' ');
		}
		evalp = 0;
	}
	if (evalvec) {
		if (evalvec == (uchar_t **)1) {
			doneinp = 1;
			reset();
		}
		if (evalp = *evalvec) {
			evalvec++;
			goto top;
		}
		evalvec = (uchar_t **)1;
		return ('\n');
	}
	do {
		if (arginp == (uchar_t *) 1 || onelflg == 1) {
			if (wanteof)
				return (-1);
			exitstat();
		}
		if (arginp) {
			if ((c = *arginp++) == 0) {
				arginp = (uchar_t *) 1;
				return ('\n');
			}
			return (c);
		}
reread:
		c = bgetc();
		if (c < 0) {

#ifdef BSD_LINE_DISC
                        struct sgttyb tty;
#else
                        struct termios tty;
#endif

			if (wanteof)
				return (-1);

#ifdef BSD_LINE_DISC
                        /* was isatty but raw with ignoreeof yields problems */
                        if (ioctl(SHIN, TIOCGETP, &tty)==0 &&
                            (tty.sg_flags & RAW) == 0) {
#else
                        if (ioctl(SHIN, TCGETS, &tty)==0 &&
                            (tty.c_lflag & (ISIG | ICANON)) != 0) {
#endif

				pid_t ctpgrp;

				if (++sincereal > 25)
					goto oops;
				if (tpgrp != -1 &&
				    ioctl(FSHTTY, TIOCGPGRP, &ctpgrp) == 0 &&
				    tpgrp != ctpgrp) {
						struct sigaction nsa, osa;
			
						nsa.sa_handler = SIG_IGN;
#ifdef _AIX
						nsa.sa_mask.losigs = SA_RESTART;
#else
						nsa.sa_mask = SA_RESTART;
#endif
						nsa.sa_flags = SA_RESTART;
						(void) sigaction(SIGTTOU, &nsa, &osa);
						IOCTL(FSHTTY, TIOCSPGRP, &tpgrp, "35");
						(void) sigaction(SIGTTOU, &osa, (struct sigaction *)NULL);
						goto reread;
				}
				if (adrof("ignoreeof")) {
					if (loginsh)
						printf(MSGSTR(M_LOGOUT,"\nUse \"logout\" to logout.\n"));
					else
						printf(MSGSTR(M_EXIT,"\nUse \"exit\" to leave csh.\n"));
					reset();
				}
				if (chkstop == 0)
					panystop(1);
			}
oops:
			doneinp = 1;
			reset();
		}
		sincereal = 0;
		if (c == '\n' && onelflg)
			onelflg--;
	} while (c == 0);
	return (c);
}

int
bgetc(void)
{
	register int buf, off, c;
	int flags;
	uchar_t ttyline[LINE_MAX];
	register int numleft = 0, roomleft;

again:
	buf = (int) fseekp / BUFSIZ;
	if (buf >= fblocks) {
		register uchar_t **nfbuf;

		nfbuf = (uchar_t **)calloc(fblocks+2, sizeof (uchar_t **));
		if (fbuf) {
			blkcpy(nfbuf, fbuf);
			xfree((uchar_t *)fbuf);
		}
		fbuf = (uchar_t **)nfbuf;
		fbuf[fblocks] = (uchar_t *)calloc(BUFSIZ, sizeof (uchar_t));
		fblocks++;
		goto again;
	}
	if (fseekp >= feobp) {
		buf = (int) feobp / BUFSIZ;
		off = (int) feobp % BUFSIZ;
		roomleft = BUFSIZ - off;
		for (;;) {
#ifdef CMDEDIT
			if ((cmdedit || filec) && intty) {
				c = numleft ? numleft :
					cmdedit ? 
					     ed_tenex((char *)ttyline, BUFSIZ) :
					     tenex(ttyline, BUFSIZ);
#else
			if (filec && intty) {
				c = numleft ? numleft : tenex(ttyline, BUFSIZ);
#endif
				if (c > roomleft) {
					/* start with fresh buffer */
					feobp = fseekp = fblocks * BUFSIZ;
					numleft = c;
					goto again;
				}
				if (c > 0)
					copy(fbuf[buf] + off, ttyline, c);
				numleft = 0;
			} else
				c = read(SHIN, (char *)fbuf[buf] + off, roomleft);
			if (c >= 0)
				break;
			if (errno == EWOULDBLOCK) {
				int	flags;
				/* set no blocking */
				flags = fcntl(SHIN, F_GETFL, 0) | O_NDELAY;
				fcntl(SHIN, F_SETFL, 0);
			} else if (errno != EINTR)
				break;
		}
		if (c <= 0)
			return (-1);
		feobp += c;
#ifdef CMDEDIT
		if ((cmdedit || filec) && !intty)
#else
		if (filec && !intty)
#endif
			goto again;
	}
	c = fbuf[buf][(int) fseekp % BUFSIZ];
	fseekp++;
	return (c);
}

void
bfree(void)
{
	register int sb, i;

	if (whyles)
		return;
	sb = (int) (fseekp - 1) / BUFSIZ;
	if (sb > 0) {
		for (i = 0; i < sb; i++)
			xfree(fbuf[i]);
		blkcpy(fbuf, &fbuf[sb]);
		fseekp -= BUFSIZ * sb;
		feobp -= BUFSIZ * sb;
		fblocks -= sb;
	}
}

void
bseek(long l)
{
	register struct whyle *wp;

	fseekp = l;
	if (!whyles)
		return;
	else {
		for (wp = whyles; wp->w_next; wp = wp->w_next)
			continue;
		if (wp->w_start > l)
			l = wp->w_start;
		return;
	}
}

long
btell(void)
{
	return (fseekp);
}

void
btoeof(void)
{

	lseek(SHIN, 0L, SEEK_END);
	fseekp = feobp;
	wfree();
	bfree();
}
