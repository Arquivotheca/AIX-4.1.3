static char sccsid[] = "@(#)31	1.6  src/bos/usr/bin/csh/parse.c, cmdcsh, bos411, 9428A410j 11/12/92 13:36:18";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: alias asyntax asyn0 asyn3 freenod syntax syn0 syn1 syn1a 
 *            syn1b syn2 syn3 freesyn
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1992
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

#define	PHERE	0x1
#define	PIN	0x2
#define	POUT	0x4
#define	PDIAG	0x8

/*
 * Perform aliasing on the word list lex
 * Do a (very rudimentary) parse to separate into commands.
 * If word 0 of a command has an alias, do it.
 * Repeat a maximum of 20 times.
 */
void
alias(register struct wordent *lex)
{
	volatile int aleft = 21;
	jmp_buf osetexit;

	getexit(osetexit);
	setexit();
	if (haderr) {
		resexit(osetexit);
		reset();
	}
	if (--aleft == 0)
		error(MSGSTR(M_ALIASLOOP, "Alias loop"));
	asyntax(lex->next, lex);
	resexit(osetexit);
}

void
asyntax(register struct wordent *p1, register struct wordent *p2)
{

	while (p1 != p2)
		if (strchr(";&\n",p1->word[0]))
			p1 = p1->next;
		else {
			asyn0(p1, p2);
			return;
		}
}

void
asyn0(struct wordent *p1, register struct wordent *p2)
{
	register struct wordent *p;
	register int l = 0;

	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			if (l < 0)
				error(MSGSTR(M_RPARENS, "Too many )'s"));
			continue;

		case '>':
			if (p->next != p2 && EQ(p->next->word, "&"))
				p = p->next;
			continue;

		case '&':
		case '|':
		case ';':
		case '\n':
			if (l != 0)
				continue;
			asyn3(p1, p);
			asyntax(p->next, p2);
			return;
		}
	if (l == 0)
		asyn3(p1, p2);
}

void
asyn3(struct wordent *p1, register struct wordent *p2)
{
	register struct varent *ap;
	struct wordent alout;
	register bool redid;

	if (p1 == p2)
		return;
	if (p1->word[0] == '(') {
		for (p2 = p2->prev; p2->word[0] != ')'; p2 = p2->prev)
			if (p2 == p1)
				return;
		if (p2 == p1->next)
			return;
		asyn0(p1->next, p2);
		return;
	}
	ap = adrof1((char *)p1->word, &aliases);
	if (ap == 0)
		return;
	alhistp = p1->prev;
	alhistt = p2;
	alvec = ap->vec;
	redid = lex(&alout);
	alhistp = alhistt = 0;
	alvec = 0;
	if (err) {
		freelex(&alout);
		error((char *)err);
	}
	if (p1->word[0] && EQ(p1->word, alout.next->word)) {
		uchar_t *cp = alout.next->word;

		alout.next->word = strspl((uchar_t *)ALIASSTR, cp);
		xfree(cp);
	}
	p1 = freenod(p1, redid ? p2 : p1->next);
	if (alout.next != &alout) {
		p1->next->prev = alout.prev->prev;
		alout.prev->prev->next = p1->next;
		alout.next->prev = p1;
		p1->next = alout.next;
		xfree(alout.prev->word);
		xfree((uchar_t *)(alout.prev));
	}
	reset();		/* throw! */
}

struct wordent *
freenod(register struct wordent *p1, register struct wordent *p2)
{
	register struct wordent *retp = p1->prev;

	while (p1 != p2) {
		xfree(p1->word);
		p1 = p1->next;
		xfree((uchar_t *)(p1->prev));
	}
	retp->next = p2;
	p2->prev = retp;
	return (retp);
}


/*
 * syntax
 *	empty
 *	syn0
 */
struct command *
syntax(register struct wordent *p1, register struct wordent *p2, int flags)
{

	while (p1 != p2)
		if (strchr(";&\n",p1->word[0]))
			p1 = p1->next;
		else
			return (syn0(p1, p2, flags));
	return (0);
}

/*
 * syn0
 *	syn1
 *	syn1 & syntax
 */
struct command *
syn0(struct wordent *p1, struct wordent *p2, int flags)
{
	register struct wordent *p;
	register struct command *t, *t1;
	int l;

	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			if (l < 0)
				seterr(MSGSTR(M_RPARENS, "Too many )'s"));
			continue;

		case '|':
			if (p->word[1] == '|')
				continue;
			/* fall into ... */

		case '>':
			if (p->next != p2 && EQ(p->next->word, "&"))
				p = p->next;
			continue;

		case '&':
			if (l != 0)
				break;
			if (p->word[1] == '&')
				continue;
			t1 = syn1(p1, p, flags);
			if (t1->t_dtyp == TLST) {
				t = (struct command *)calloc(1, sizeof (*t));
				t->t_dtyp = TPAR;
				t->t_dflg = FAND|FINT;
				t->t_dspr = t1;
				t1 = t;
			} else
				t1->t_dflg |= FAND|FINT;
			t = (struct command *)calloc(1, sizeof (*t));
			t->t_dtyp = TLST;
			t->t_dflg = 0;
			t->t_dcar = t1;
			t->t_dcdr = syntax(p, p2, flags);
			return(t);
		}
	if (l == 0)
		return (syn1(p1, p2, flags));
	seterr(MSGSTR(M_LPARENS, "Too many ('s"));
	return (0);
}

/*
 * syn1
 *	syn1a
 *	syn1a ; syntax
 */
struct command *
syn1(struct wordent *p1, struct wordent *p2, int flags)
{
	register struct wordent *p;
	register struct command *t;
	int l;

	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case ';':
		case '\n':
			if (l != 0)
				break;
			t = (struct command *)calloc(1, sizeof (*t));
			t->t_dtyp = TLST;
			t->t_dcar = syn1a(p1, p, flags);
			t->t_dcdr = syntax(p->next, p2, flags);
			if (t->t_dcdr == 0)
				t->t_dcdr = t->t_dcar, t->t_dcar = 0;
			return (t);
		}
	return (syn1a(p1, p2, flags));
}

/*
 * syn1a
 *	syn1b
 *	syn1b || syn1a
 */
struct command *
syn1a(struct wordent *p1, struct wordent *p2, int flags)
{
	register struct wordent *p;
	register struct command *t;
	register int l = 0;

	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case '|':
			if (p->word[1] != '|')
				continue;
			if (l == 0) {
				t = (struct command *)calloc(1, sizeof (*t));
				t->t_dtyp = TOR;
				t->t_dcar = syn1b(p1, p, flags);
				t->t_dcdr = syn1a(p->next, p2, flags);
				t->t_dflg = 0;
				return (t);
			}
			continue;
		}
	return (syn1b(p1, p2, flags));
}

/*
 * syn1b
 *	syn2
 *	syn2 && syn1b
 */
struct command *
syn1b(struct wordent *p1, struct wordent *p2, int flags)
{
	register struct wordent *p;
	register struct command *t;
	register int l = 0;

	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case '&':
			if (p->word[1] == '&' && l == 0) {
				t = (struct command *)calloc(1, sizeof (*t));
				t->t_dtyp = TAND;
				t->t_dcar = syn2(p1, p, flags);
				t->t_dcdr = syn1b(p->next, p2, flags);
				t->t_dflg = 0;
				return (t);
			}
			continue;
		}
	return (syn2(p1, p2, flags));
}

/*
 * syn2
 *	syn3
 *	syn3 | syn2
 *	syn3 |& syn2
 */
struct command *
syn2(struct wordent *p1, struct wordent *p2, int flags)
{
	register struct wordent *p, *pn;
	register struct command *t;
	register int l = 0;
	int f;

	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			l++;
			continue;

		case ')':
			l--;
			continue;

		case '|':
			if (l != 0)
				continue;
			t = (struct command *)calloc(1, sizeof (*t));
			f = flags | POUT;
			pn = p->next;
			if (pn != p2 && pn->word[0] == '&') {
				f |= PDIAG;
				t->t_dflg |= FDIAG;
			}
			t->t_dtyp = TFIL;
			t->t_dcar = syn3(p1, p, f);
			if (pn != p2 && pn->word[0] == '&')
				p = pn;
			t->t_dcdr = syn2(p->next, p2, flags | PIN);
			return (t);
		}
	return (syn3(p1, p2, flags));
}

uchar_t	*RELPAR =	(uchar_t *)"<>()";

/*
 * syn3
 *	( syn0 ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	KEYWORD ( word* ) word* [ < in ] [ > out ]
 *
 *	KEYWORD = (@ exit foreach if set switch test while)
 */
struct command *
syn3(struct wordent *p1, struct wordent *p2, int flags)
{
	register struct wordent *p;
	struct wordent *lp, *rp;
	register struct command *t;
	register int l;
	uchar_t **av;
	int n, c;
	bool specp = 0;

	if (p1 != p2) {
		p = p1;
again:
		switch (srchx(p->word)) {

		case ZELSE:
			p = p->next;
			if (p != p2)
				goto again;
			break;

		case ZEXIT:
		case ZFOREACH:
		case ZIF:
		case ZLET:
		case ZSET:
		case ZSWITCH:
		case ZWHILE:
			specp = 1;
			break;
		}
	}
	n = 0;
	l = 0;
	for (p = p1; p != p2; p = p->next)
		switch (p->word[0]) {

		case '(':
			if (specp)
				n++;
			l++;
			continue;

		case ')':
			if (specp)
				n++;
			l--;
			continue;

		case '>':
		case '<':
			if (l != 0) {
				if (specp)
					n++;
				continue;
			}
			if (p->next == p2)
				continue;
			if (strchr(RELPAR,p->next->word[0]))
				continue;
			n--;
			continue;

		default:
			if (!specp && l != 0)
				continue;
			n++;
			continue;
		}
	if (n < 0)
		n = 0;
	t = (struct command *)calloc(1, sizeof (*t));
	av = (uchar_t **)calloc((unsigned)(n + 1), sizeof (uchar_t **));
	t->t_dcom = av;
	n = 0;
	if (p2->word[0] == ')')
		t->t_dflg = FPAR;
	lp = 0;
	rp = 0;
	l = 0;
	for (p = p1; p != p2; p = p->next) {
		c = p->word[0];
		switch (c) {

		case '(':
			if (l == 0) {
				if (lp != 0 && !specp)
					seterr(MSGSTR(M_BADPAREN, 
						"Badly placed ("));
				lp = p->next;
			}
			l++;
			goto savep;

		case ')':
			l--;
			if (l == 0)
				rp = p;
			goto savep;

		case '>':
			if (l != 0)
				goto savep;
			if (p->word[1] == '>')
				t->t_dflg |= FCAT;
			if (p->next != p2 && EQ(p->next->word, "&")) {
				t->t_dflg |= FDIAG, p = p->next;
				if (flags & (POUT|PDIAG))
					goto badout;
			}
			if (p->next != p2 && EQ(p->next->word, "!"))
				t->t_dflg |= FANY, p = p->next;
			if (p->next == p2) {
missfile:
				seterr(MSGSTR(M_REDIRECT,
					"Missing name for redirect"));
				continue;
			}
			p = p->next;
			if (strchr(RELPAR,p->word[0]))
				goto missfile;
			if ((flags & POUT) && (flags & PDIAG) == 0 || t->t_drit)
badout:
				seterr(MSGSTR(M_BADOUT, 
					"Ambiguous output redirect"));
			else
				t->t_drit = savestr(p->word);
			continue;

		case '<':
			if (l != 0)
				goto savep;
			if (p->word[1] == '<')
				t->t_dflg |= FHERE;
			if (p->next == p2)
				goto missfile;
			p = p->next;
			if (strchr(RELPAR,p->word[0]))
				goto missfile;
			if ((flags & PHERE) && (t->t_dflg & FHERE))
				seterr(MSGSTR(M_HERE, "Can't << within ()'s"));
			else if ((flags & PIN) || t->t_dlef)
				seterr(MSGSTR(M_INPUT,
					"Ambiguous input redirect"));
			else
				t->t_dlef = savestr(p->word);
			continue;

savep:
			if (!specp)
				continue;
		default:
			if (l != 0 && !specp)
				continue;
			if (err == 0)
				av[n] = savestr(p->word);
			n++;
			continue;
		}
	}
	if (lp != 0 && !specp) {
		if (n != 0)
			seterr(MSGSTR(M_BADPARENS, "Badly placed ()'s"));
		t->t_dtyp = TPAR;
		t->t_dspr = syn0(lp, rp, PHERE);
	} else {
		if (n == 0)
			seterr(MSGSTR(M_NULL, "Invalid null command"));
		t->t_dtyp = TCOM;
	}
	return (t);
}

void
freesyn(register struct command *t)
{
	register uchar_t **v;

	if (t == 0)
		return;
	switch (t->t_dtyp) {

	case TCOM:
		for (v = t->t_dcom; *v; v++)
			xfree(*v);
		xfree((uchar_t *)(t->t_dcom));
		goto lr;

	case TPAR:
		freesyn(t->t_dspr);
		/* fall into ... */

lr:
		xfree(t->t_dlef);
		xfree(t->t_drit);
		break;

	case TAND:
	case TOR:
	case TFIL:
	case TLST:
		freesyn(t->t_dcar), freesyn(t->t_dcdr);
		break;
	}
	xfree((uchar_t *)t);
}
