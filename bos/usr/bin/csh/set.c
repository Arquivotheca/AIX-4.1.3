static char sccsid[] = "@(#)38	1.19  src/bos/usr/bin/csh/set.c, cmdcsh, bos411, 9437B411a 9/15/94 10:40:26";
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: doset getinx asx getvx dolet xset operate xfree savestr
 *            putn putn1 getn value value1 adrof madrof adrof1 set 
 *            set1 setq unset unset1 unsetv unsetv1 shift exportpath
 *	      onlyread
 *
 * ORIGINS:  10,26,27,18,71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
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
#include <locale.h>

/*
 * Static functions.
 */
static int		onltread(uchar_t *);


void
doset(register uchar_t **v)
{
	register uchar_t *p;
	uchar_t *vp, op;
	uchar_t **vecp;
	bool hadsub;
	int subscr;
	register int n;
	wchar_t nlc;

	v++;
	p = *v++;
	if (p == 0) {
		prvars();
		return;
	}
	do {
		hadsub = 0;
		vp = p;
		n = mbtowc(&nlc, (char *)p, mb_cur_max);
		if (n < 1) {
			n = 1;
			nlc = *p & 0xff;
		}
		if (!letter(nlc))
			goto setsyn;	
		for (p += n; *p; p += n) {
			n = mbtowc(&nlc, (char *)p, mb_cur_max);
			if (n < 1) {
				n = 1;
				nlc = *p & 0xff;
			}
			if (!alnum(nlc)) 
				break;
		}
		if (*p == '[') {
			hadsub++;
			p = getinx(p, &subscr);
		}
		if (op = *p) {
			*p++ = 0;
			if (*p == 0 && *v && **v == '(')
				p = *v++;
		} else if (*v && EQ(*v, "=")) {
			op = '=', v++;
			if (*v)
				p = *v++;
		}
		if (op && op != '=')
			goto setsyn;
		if (EQ(p, "(")) {
			register uchar_t **e = v;

			if (hadsub)
				goto setsyn;
			for (;;) {
				if (!*e)
					bferr(MSGSTR(M_MISSP, "Missing )"));
				if (**e == ')')
					break;
				e++;
			}
			p = *e;
			*e = 0;
			vecp = saveblk(v);
			set1((char *)vp, vecp, &shvhed);
			*e = p;
			v = e + 1;
		} else if (hadsub)
			asx(vp, subscr, savestr(p));
		else
			set((char *)vp, savestr(p));
		if (EQ(vp, "path")) {
			exportpath(adrof("path")->vec);
			dohash();
		} else if (EQ(vp, "histchars")) {
			register uchar_t *p = value("histchars");

			wchar_t nlc1;
			n = mbtowc(&nlc1, (char *)p, mb_cur_max);
			if (n < 0)
				goto setsyn;
			else if (n == 0) {
				HIST = 0;
				HISTSUB = 0;
			} else {
				n = mbtowc(&nlc,(char *)p+n, mb_cur_max);
				if (n < 1)
					goto setsyn;
				HISTSUB = nlc;
				HIST = nlc1;
			}
		} else if (EQ(vp, "user"))
			setcenv("USER", value((char *)vp));
		else if (EQ(vp, "term")) {
			setcenv("TERM", value((char *)vp));
#ifdef CMDEDIT
                        dosetupterm();
#endif
		}
		else if (EQ(vp, "home"))
			setcenv("HOME", value((char *)vp));
		else if (EQ(vp, "filec"))
			filec = 1;
#ifdef CMDEDIT
                else if (EQ(vp, "editmode")) {
                        setcenv("EDITMODE", value((char *)vp));
                        if (EQ(value((char *)vp), "none")) {
                                cmdedit = 0;
                                continue;
                        }
                        cmdedit = 1;
                        dobindings((char *)value((char *)vp));
                        if (!EQ(value((char *)vp),"dumb") && !adrof("edithist"))
                                set("edithist",(uchar_t *)"");
                }
#endif
	} while (p = *v++);
	return;
setsyn:
	bferr(MSGSTR(M_SYNERR, "Syntax error"));
	/* NOTREACHED */
}

uchar_t *
getinx(register uchar_t *cp, register int *ip)
{

	*ip = 0;
	*cp++ = 0;
	while (*cp && digit(*cp))
		*ip = *ip * 10 + *cp++ - '0';
	if (*cp++ != ']')
		bferr(MSGSTR(M_SUBERR, "Subscript error"));
	return (cp);
}

void
asx(uchar_t *vp, int subscr, uchar_t *p)
{
	register struct varent *v = getvx((int)vp, subscr);

	xfree(v->vec[subscr - 1]);
	v->vec[subscr - 1] = globone(p);
}

struct varent *
getvx(int vp, int subscr)
{
	register struct varent *v = adrof((char *)vp);

	if (v == 0)
		udvar((uchar_t *)vp);
	if (subscr < 1 || subscr > blklen(v->vec))
		bferr(MSGSTR(M_SUBOUT, "Subscript out of range"));
	return (v);
}

static uchar_t	plusplus[2] = { '1', 0 };

void
dolet(uchar_t **v)
{
	register uchar_t *p;
	uchar_t *vp, c, op;
	bool hadsub;
	int subscr;
	register int n;
	wchar_t nlc;

	v++;
	p = *v++;
	if (p == 0) {
		prvars();
		return;
	}
	do {
		hadsub = 0;
		vp = p;
		n = mbtowc(&nlc, (char *)p, mb_cur_max);
		if (n < 1) {
			n = 1;
			nlc = *p & 0xff;
		}
		if (!letter(nlc))
			goto letsyn;	
		for (p += n; *p; p += n) {
			n = mbtowc(&nlc, (char *)p, mb_cur_max);
			if (n < 1) {
				n = 1;
				nlc = *p & 0xff;
			}
			if (!alnum(nlc)) 
				break;
		}
		if (*p == '[') {
			hadsub++;
			p = getinx(p, &subscr);
		}
		if (*p == 0 && *v)
			p = *v++;
		if (op = *p)
			*p++ = 0;
		else
			goto letsyn;
		vp = savestr(vp);
		if (op == '=') {
			c = '=';
			p = xset(p, &v);
		} else {
			c = *p++;
			if (strchr("+-", c)) {
				if (c != op || *p)
					goto letsyn;
				p = plusplus;
			} else {
				if (strchr("<>", op)) {
					if (c != op)
						goto letsyn;
					c = *p++;
					goto letsyn;
				}
				if (c != '=')
					goto letsyn;
				p = xset(p, &v);
			}
		}
		if (op == '=')
			if (hadsub)
				asx(vp, subscr, p);
			else
				set((char *)vp, p);
		else
			if (hadsub)
				asx(vp, subscr, operate(op, 
					getvx((int)vp, subscr)->vec[subscr - 1], 
					p));
			else
				set((char *)vp, operate(op, value((char *)vp), p));
		if (strcmp(vp, "path") == 0)
			dohash();
		xfree(vp);
		if (c != '=')
			xfree(p);
	} while (p = *v++);
	return;
letsyn:
	bferr(MSGSTR(M_SYNERR, "Syntax error"));
}

uchar_t *
xset(uchar_t *cp, uchar_t ***vp)
{
	register uchar_t *dp;

	if (*cp) {
		dp = savestr(cp);
		--(*vp);
		xfree(**vp);
		**vp = dp;
	}
	return (putn(exp(vp)));
}

uchar_t *
operate(uchar_t op, uchar_t *vp, uchar_t *p)
{
	uchar_t opr[2];
	uchar_t *vec[5];
	register uchar_t **v = vec;
	uchar_t **vecp = v;
	register int i;

	if (op != '=') {
		if (*vp)
			*v++ = vp;
		opr[0] = op;
		opr[1] = 0;
		*v++ = opr;
		if (op == '<' || op == '>')
			*v++ = opr;
	}
	*v++ = p;
	*v++ = 0;
	i = exp(&vecp);
	if (*vecp)
		bferr(MSGSTR(M_EXPR, "Expression syntax"));
	return (putn(i));
}

void
xfree(uchar_t *cp)
{
	extern uchar_t end[];

	if (cp >= end && cp < (uchar_t *) &cp)
		free(cp);
}

uchar_t *
savestr(register uchar_t *s)
{
	register uchar_t *n;

	if (s == (uchar_t)NULL )
		s = (uchar_t *)"";
	n = (uchar_t *) strdup(s);
	return (n);
}

static	uchar_t *putp;
 
uchar_t *
putn(register int n)
{
	static uchar_t number[15];

	putp = number;
	if (n < 0) {
		n = -n;
		*putp++ = '-';
	}
	if (sizeof (int) == 2 && n == -32768) {
		*putp++ = '3';
		n = 2768;
	} else if (sizeof (int) == 4 && n == (8<<28)) {
		*putp++ = '2';
		n = 147483648;
	}
	putn1(n);
	*putp = 0;
	return (savestr(number));
}

void
putn1(register int n)
{
	if (n > 9)
		putn1(n / 10);
	*putp++ = n % 10 + '0';
}

int
getn(cp)
	register uchar_t *cp;
{
	register int n;
	int sign, base=10;

	sign = 0;
	if (cp[0] == '+' && cp[1])
		cp++;
	if (*cp == '-') {
		sign++;
		cp++;
		if (!digit(*cp))
			goto badnum;
	}
	n = 0;
        if (cp[0] == '0') {
                base = 8;
		/* D14887:
		 *	A leading zero means interpret as octal.
		 *	Non-octal digits are now recognized and rejected.
		 */
        	while (digit(*cp) && *cp - '0' < base)
        		n = n * base + *cp++ - '0';
	} else {
        	while (digit(*cp))
        		n = n * base + *cp++ - '0';
	}
	if (*cp)
		goto badnum;
	return (sign ? -n : n);
badnum:
	bferr(MSGSTR(M_BADNUM, "Badly formed number"));
	return (0);
}

uchar_t *
value(char *var)
{
	return(value1(var, &shvhed));
}

uchar_t *
value1(char *var, struct varent *head)
{
	register struct varent *vp;

	vp = adrof1(var, head);
	return (vp == 0 || vp->vec[0] == 0 ? (uchar_t *)"" : vp->vec[0]);
}

static	struct varent *shprev;

struct varent *
adrof(char *var)
{
	return (adrof1(var, &shvhed));
}

struct varent *
madrof(uchar_t *pat, struct varent *head)
{
	register struct varent *vp;

	shprev = head;
	for (vp = shprev->link; vp != 0; vp = vp->link) {
		if (Gmatch(vp->name, pat))
			return (vp);
		shprev = vp;
	}
	return (0);
}

/*
 * The list worked on by adrof1() is
 * a sorted list.
 */
struct varent *
adrof1(char *var, struct varent *head)
{
	register struct varent *vp;
	int cmp;

	shprev = head;
	for (vp = shprev->link; vp != 0; vp = vp->link) {
		cmp = strcmp(vp->name, var);
		if (cmp == 0)
			return (vp);
		else if (cmp > 0)
			return (0);
		shprev = vp;
	}
	return (0);
}

/*
 * The caller is responsible for putting value in a safe place
 */
void
set(char *var, uchar_t *value)
{
	register uchar_t **vec = (uchar_t **)calloc(2, sizeof(char **));

	vec[0] = onlyread(value) ? savestr(value) : value;
	set1(var, vec, &shvhed);
}

/********************************************
 * set_noglob
 *    identical to set() except that it calls
 *    setq() directly in order to avoid the
 *    globbing done by set1()
 ********************************************/
set_noglob(var, value)
        uchar_t *var, *value;
{
        register uchar_t **vec = (uchar_t **) calloc(2, sizeof (char **));

        vec[0] = onlyread(value) ? savestr(value) : value;
        setq(var, vec, &shvhed);
}

void
set1(char *var, uchar_t **vec, struct varent *head)
{

	register uchar_t **oldv = vec;

	gflag = 0;
	rscan(oldv, tglob);
	if (gflag) {
		vec = glob(oldv);
		if (vec == 0) {
			bferr(MSGSTR( M_NOMATCH, "No match"));
			blkfree(oldv);
			return;
		}
		blkfree(oldv);
		gargv = 0;
	}
	setq(var, vec, head);
}

void
setq(char *var, uchar_t **vec, struct varent *head)
{
	register struct varent *vp;

	vp = adrof1(var, head);
	if (vp == 0) {
		vp = (struct varent *)calloc(1, sizeof(*vp));
		vp->name = savestr((uchar_t *)var);
		vp->link = shprev->link;
		shprev->link = vp;
	}
	if (vp->vec)
		blkfree(vp->vec);
	scan(vec, trim);
	vp->vec = vec;
}

void
unset(uchar_t *v[])
{

	unset1(v, &shvhed);
	if (adrof("histchars") == 0) {
		HIST = '!';
		HISTSUB = '^';
	}
#ifdef CMDEDIT
        if (adrof("editmode") == 0) {
                struct varent *vp;

                if ((vp = adrof("edithist")) != 0)
                        unsetv1((char *)v, (struct varent *)vp);
                unsetcenv((uchar_t *)"EDITMODE");
                cmdedit = 0;
        }
#endif
	if (adrof("filec") == 0)
		filec = 0;
}

void
unset1(register uchar_t *v[], struct varent *head)
{
	register uchar_t *var;
	register struct varent *vp;
	register int cnt;

	v++;
	while (var = *v++) {
		cnt = 0;
		while (vp = madrof(var, head))
			unsetv1((char *)vp->name, head), cnt++;
		if (cnt == 0)
			setname(var);
	}
}

void
unsetv(char *var)
{
	unsetv1(var, &shvhed);
}

void
unsetv1(char *var, struct varent *head)
{
	register struct varent *vp;

	vp = adrof1(var, head);
	if (vp == 0)
		udvar((uchar_t *)var);
	vp = shprev->link;
	shprev->link = vp->link;
	blkfree(vp->vec);
	xfree(vp->name);
	xfree((uchar_t *)vp);
}

void
shift( register uchar_t **v)
{
	register struct varent *argv;
	register uchar_t *name;

	v++;
	name = *v;
	if (name == 0)
		name = (uchar_t *)"argv";
	else
		strip(name);
	argv = adrof((char *)name);
	if (argv == 0)
		udvar(name);
	if (argv->vec[0] == 0)
		bferr(MSGSTR(M_NOMORE, "No more words"));
	lshift(argv->vec, 1);
}

void
exportpath(uchar_t **val)
{
	uchar_t exppath[BUFR_SIZ];
	register uchar_t *dir;

	exppath[0] = 0;
	if (val)
		while (*val) {
			if (strlen(*val) + strlen(exppath) + 2 > BUFR_SIZ) {
				printf(MSGSTR(M_LONGPATH,
				"Warning: ridiculously long PATH truncated\n"));
				break;
			}
			strcat(exppath, *val++);
			if (*val == 0 || EQ(*val, ")"))
				break;
			strcat(exppath, ":");
		}
	setcenv("PATH", exppath);
}

int
onlyread(uchar_t *cp)
{
        extern uchar_t end[];

        return (cp < end);
}
