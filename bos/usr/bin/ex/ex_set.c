#if !defined(lint)
static char sccsid[] = "@(#)09	1.12  src/bos/usr/bin/ex/ex_set.c, cmdedit, bos41B, 9504A 12/19/94 11:46:43";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_set.c
 *
 * FUNCTION: prall, propt, propts, set, setend, seteq
 *
 * ORIGINS: 3, 10, 13, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ex_set.c  1.4  com/cmd/edit/vi,3.1,9013 12/20/89 10:57:19
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */
/* ex_set.c Revision: 2.5.2.2 (OSF) Date: 90/10/12 15:46:37 */

#include "ex.h"
#include "ex_temp.h"
#include "ex_tty.h"

/*
 * seteq compares a (wchar_t *) to a (char *)
 */
#define nseq(a,b)		((a) && (b) && seteq(a,b) == 0)
#define sneq(a,b)		((a) && (b) && seteq(b,a) == 0)
static void propts(void);
static void prall(void);
static void propt(register struct option *op);
extern int vSCROLL;

static int seteq(register wchar_t * n,register char *s)
{
    while (*n == *s++)
	if (*n++ == 0)
	    return 0;
    return (*n - *--s);
}

/*
 * Set command.
 */
static wchar_t	optname[ONMSZ];

void set(void)
{
	register wchar_t *cp;
	register struct option *op;
	register int c;
	short no;
	char *temp_charptr;
	extern short ospeed;

	setnoaddr();
	if (skipend()) {
		if (peekchar() != EOF)
			ignchar();
	                propts();	
		return;
	}
	do {
		cp = optname;
		do {
			if (cp < &optname[ONMSZ - 2])
				*cp++ = ex_getchar();
		} while (iswalnum(peekchar()));
		*cp = 0;
		cp = optname;
		if (sneq("all", cp)) {
			if (inopen)
				pofix();
			prall();
			goto next;
		}
		no = 0;
		if (cp[0] == 'n' && cp[1] == 'o') {
			cp += 2;
			no++;
		}
		/* Implement w300, w1200, and w9600 specially */
		if (nseq(cp, "w300")) {
			if (ospeed >= B1200) {
dontset:
				ignore(ex_getchar());	/* = */
				ignore(getnum());	/* value */
				continue;
			}

			temp_charptr = "window";
			if (mbstowcs(cp, temp_charptr, ONMSZ) == -1)
				error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

		} else if (nseq(cp, "w1200")) {
			if (ospeed < B1200 || ospeed >= B2400)
				goto dontset;

			temp_charptr = "window";
			if (mbstowcs(cp, temp_charptr, ONMSZ) == -1)
				error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);


		} else if (nseq(cp, "w9600")) {
			if (ospeed < B2400)
				goto dontset;

			temp_charptr = "window";
			if (mbstowcs(cp, temp_charptr, ONMSZ) == -1)
				error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);
		}
		for (op = options; op < &options[NOPTS]; op++)
			if (sneq(op->oname, cp) || op->oabbrev && sneq(op->oabbrev, cp))
				break;
		if (op->oname == 0) {
			if (nseq(cp, "vice")) 
				 serror(MSGSTR(M_302, "Cannot set novice interactively - invoke as vedit"), DUMMY_CHARP);
			else 
				serror(MSGSTR(M_142, "%S: No such option@- 'set all' gives all option values"), (char *)cp);
		}
		c = skipwh();
		if (peekchar() == '?') {
			ignchar();
printone:
			propt(op);
			noonl();
			goto next;
		}
		if (op->otype == ONOFF) {
			op->ovalue = 1 - no;
			if (op == &options[PROMPT])
				oprompt = 1 - no;
			goto next;
		}
		if (no)
			serror(MSGSTR(M_143, "Option %s is not a toggle"), op->oname);
		if (c != 0 || setend())
			goto printone;
		if (ex_getchar() != '=')
			serror(MSGSTR(M_144, "Missing =@in assignment to option %s"), op->oname);
		switch (op->otype) {

		case NUMERIC:
			if (!iswdigit(peekchar()))
				error(MSGSTR(M_145, "Digits required@after ="), DUMMY_INT);
			op->ovalue = getnum();
			vSCROLL = value(SCROLL);
			if (value(TABSTOP) <= 0){
			    value(TABSTOP) = TABS;
			    serror(MSGSTR(M_514, "The %s option must be greater than 0. Default value used.")
				,"tabstop");
			}
			if (value(HARDTABS) <= 0){
			    value(HARDTABS) = TABS;
			    serror(MSGSTR(M_514, "The %s option must be greater than 0. Default value used.")
				,"hardtabs");
			}
			if (value(SHIFTWIDTH) <= 0) {
			    value(SHIFTWIDTH) = TABS;
			    serror(MSGSTR(M_514, "The %s option must be greater than 0. Default value used.")
				,"shiftwidth");
			}
			if (op == &options[WINDOW]) {
				if (value(WINDOW) >= lines)
					value(WINDOW) = lines-1;
				vsetsiz(value(WINDOW));
			}
			break;

		case STRING:
		case OTERM:
		case CLOSEP:	/* closing punctuation */
		case ONECOL:	/* partial character indicator */
		case WRAP:	/* wraptype */
		  {
			char s_type[ONMSZ];	/* terminal type */

			cp = optname;
			while (!setend()) {
				if (cp >= &optname[ONMSZ])
					error(MSGSTR(M_146, "String too long@in option assignment"), DUMMY_INT);
				/* adb change:  allow whitepace in strings */
				if( (*cp = ex_getchar()) == '\\')
					if( peekchar() != EOF)
						*cp = ex_getchar();
				cp++;
			}
			*cp = 0;

			if (wcstombs(s_type, optname, ONMSZ) ==-1)
				error(MSGSTR(M_651, "Invalid wide character string, conversion failed."), DUMMY_INT);

			if (op->otype == OTERM) {
/*
 * If the terminal type
 * is changed inside visual mode, as long as we assume the screen is
 * a mess and redraw it. However, it's a much harder problem than that.
 * If you happen to change from 1 crt to another that both have the same
 * size screen, it's OK. But if the screen size if different, the stuff
 * that gets initialized in vop() will be wrong. This could be overcome
 * by redoing the initialization, e.g. making the first 90% of vop into
 * a subroutine. However, the most useful case is where you forgot to do
 * a setenv before you went into the editor and it thinks you're on a dumb
 * terminal. Ex treats this like hardcopy and goes into HARDOPEN mode.
 * This loses because the first part of vop calls oop in this case.
 * The problem is so hard I gave up. I'm not saying it can't be done,
 * but I am saying it probably isn't worth the effort.
 */
				if (inopen) {
					error(MSGSTR(M_147, "Can't change type of terminal from within open/visual"), DUMMY_INT);
				}
				setterm(s_type);
			} else {
				if (op->otype == WRAP)
					switch(s_type[0]) {
					case 'f':
					case 'g':
					case 'r':
					case 'w':
						break;
					default:
						error(MSGSTR(M_290, "f(lexible), g(eneral), r(igid), or w(ord) required@after ="),	DUMMY_INT);
						break;
					}
				else if (op->otype == ONECOL) {
					wchar_t pwcptr;
					wchar_t nc;
			
					if (mbtowc(&pwcptr, s_type, MB_CUR_MAX) == -1)
						error(MSGSTR(M_652, "Incomplete or invalid multibyte character, conversion failed."), DUMMY_INT);
					nc = pwcptr;



					/* PARTIALCHAR must be a printable single col char */
					if 	((wcwidth(nc) > 1) || !iswprint(nc))
						error(MSGSTR(M_289, "partial character indicator should display in one column"), DUMMY_INT);
				}
				if (op->otype != CLOSEP) {
					strcpy(op->osvalue, s_type);
				} else {
					char *p = s_type ;
					char *q = op->osvalue ;
					int i;

					/* multi-character punctuation is from 1 to 9 chars */
					while (*p) {
						if (*p >= '1' && *p <= '9') {
							i = *p - '0' + 1;
							/* copy and verify multi-char punctuation count */
							while (i--)
							{
							int char_len;
							wchar_t pwcptr;

							if ((char_len = mbtowc(&pwcptr, p, MB_CUR_MAX)) > 0)
							{
							c = pwcptr;
							p += char_len;
							q += wctomb(q, pwcptr);
							}
							else
								error(MSGSTR(M_293, "fewer characters than specified"), DUMMY_INT);

							}
						} else {
							*q++ = *p++;
						}
					}
					*q = '\0';
				}
				op->odefault = 1;
			}
			break;
		  }
		}
next:
		flush();
	} while (!skipend());
	eol();
}

static int setend(void)
{

	return (iswhite(peekchar()) || endcmd(peekchar()));
}

static void prall(void)
{
	register int incr = (NOPTS + 2) / 3;
	register int rows = incr;
	register struct option *op = options;

	for (; rows; rows--, op++) {
		propt(op);
		gotab(24);
		propt(&op[incr]);
		if (&op[2*incr] < &options[NOPTS]) {
			gotab(56);
			propt(&op[2 * incr]);
		}
		putNFL();
	}
}

static void propts(void)
{
	register struct option *op;

	for (op = options; op < &options[NOPTS]; op++) {
		if (op == &options[TTYTYPE])
			continue;
		switch (op->otype) {

		case ONOFF:
		case NUMERIC:
			if (op->ovalue == op->odefault)
				continue;
			break;

		case CLOSEP:
		case ONECOL:
		case WRAP:
		case STRING:
			if (op->odefault == 0)
				continue;
			break;
		}
		propt(op);
		ex_putchar(' ');
	}
	noonl();
	flush();
}

static void propt(register struct option *op)
{
	register char *name;
	
	name = op->oname;

	switch (op->otype) {

	case ONOFF:
		ex_printf("%s%s", op->ovalue ? "" : "no", name);
		break;

	case NUMERIC:
		ex_printf("%s=%d", name, op->ovalue);
		break;

	case CLOSEP:
	case ONECOL:
	case WRAP:
	case STRING:
	case OTERM:
		ex_printf("%s=%s", name, op->osvalue);
		break;
	}
}
