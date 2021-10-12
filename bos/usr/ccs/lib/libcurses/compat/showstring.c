static char sccsid[] = "@(#)95  1.10.1.5  src/bos/usr/ccs/lib/libcurses/compat/showstring.c, libcurses, bos411, 9428A410j 11/19/93 11:02:38";

/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _showstring
 *
 * ORIGINS: 3, 10, 27, 28
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"
#ifdef NLS
static char conv[2];
static short convret;
static NLchar rawc;
#endif

/*include file for message text*/
#ifdef MSG
#include "showstring_msg.h"
#include <nl_types.h>
#ifndef DONOTUSELOCALE
#include <locale.h>
#endif
nl_catd showstring_catd = CATD_ERR;
#endif
/*
 * NAME:        _showstring
 *
 * FUNCTION:
 *
 *      Dump the string running from first to last out to the terminal.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Take into account attributes, and attempt to take advantage of
 *      large pieces of white space and text that's already there.
 *      oldline is the old text of the line.
 *
 *      Variable naming convention: *x means "extension", e.g. a rubber band
 *      that briefly looks ahead; *c means a character version of an
 *      otherwise chtype pointer; old means what was on the screen before
 *      this call; left means the char 1 space to the left.
 */

_showstring(sline, scol, first, last, oldlp)
int sline, scol;
chtype *first, *last; 
struct line *oldlp;
{
	register int hl = 0;    /* nontrivial line, highlighted/with holes*/
	int prevhl=SP->virt_gr, thishl;	/* highlight state tty is in	 */
	register chtype *p, *px;	/* current char being considered */
	register chtype *oldp, *oldpx;	/* stuff under p and px		 */
	register char *pc, *pcx;	/* like p, px but in char buffer */
	chtype *tailoldp;		/* last valid oldp		 */
	int oldlen;			/* length of old line		 */
	int lcol, lrow;			/* position on screen		 */
	int oldc;			/* char at oldp			 */
	int leftoldc, leftnewc;		/* old & new chars to left of p  */
	int diff_cookies;		/* magic cookies changed	 */
	chtype *oldline;
#ifdef NONSTANDARD
	static
#endif NONSTANDARD

#ifdef NLS
	   char firstc[512], *lastc;	/* char copy of input first, last */
#else
	   char firstc[256], *lastc;	/* char copy of input first, last */
#endif

	/* set up prevhl value to the val at location */
      if (oldlp)
		prevhl = *(oldlp->body+scol) & A_ATTRIBUTES;
/*	if (outf) {
		fprintf(outf,"_showstring: setting up prevhl\n");
		fprintf(outf,"        SP->curbody[sline+1]->body 0x%x\n",
			*SP->curbody[sline+1]->body);
		fprintf(outf,"       *SP->curbody[sline+1]->body+scol 0x%x\n",
			*(SP->curbody[sline+1]->body+scol) 0x%x);
		fprintf(outf,"       prevhl 0x%x\n",prevhl);
	} */

#ifdef MSG /*open the message catalogue file descriptor*/
#ifndef DONOTUSELOCALE
	setlocale(LC_ALL,"");
#endif
	if(showstring_catd == CATD_ERR)
		showstring_catd = NLcatopen(MF_SHOWSTRING,NL_CAT_LOCALE);
#endif
#ifdef DEBUG
	if(outf) fprintf(outf,"_showstring: last 0x%x first 0x%x\n",
			last,first);
	if(outf) fprintf(outf,
		"_showstring((%d,%d) %d:'", sline, scol, last-first+1);
	if(outf)
		for (p=first; p<=last; p++) {
			thishl = *p & A_ATTRIBUTES;
			if (thishl)
				putc('\'', outf);
			putc(*p & A_CHARTEXT, outf);
		}
	if(outf) fprintf(outf, "').\n");
#endif
	if (last-first > columns) {
		_pos(lines-1, 0);
#ifndef		NONSTANDARD
    #ifdef MSG
		fprintf(stderr,NLcatgets(showstring_catd,MS_showstring,DISPCOLS,
		"_showstring: String must be less than %1$d characters wide.\
		first %2$d, last %3$d, difference %4$d\n"),
		columns, first, last, last-first);
    #else
		fprintf(stderr,
		"Bad call to _showstring, first %x, last %x, diff %dpcx\n",
		first, last, last-first);
    #endif
#endif
		abort();
	}
	if (oldlp) {
		oldline = oldlp->body;
		oldp = oldline+scol;
	}
	else
		oldp = 0;
	for (p=first,lastc=firstc; p<=last; ) {
		if (*p & A_ATTRIBUTES)
			hl++;	/* attributes on the line */
		if (oldp && (*oldp++ & A_ATTRIBUTES))
			hl++;	/* attributes on old line */
		if (*p==' ' && (px=p+1,*px++==' ') && *px++==' ' && *px==' ')
			hl++;	/* a run of at least 4 blanks */
#ifdef NLS
		rawc = *p & A_CHARTEXT;
		convret = NCencode(&rawc, conv);
		*lastc++ = conv[0];
		if (convret == 2)
			*lastc++ = conv[1];
#else
		*lastc++ = *p & A_CHARTEXT;
#endif
		p++;
#ifdef DEBUG
	if(outf) fprintf(outf,
		"p %x '%c' %o, lastc %x %o, oldp %x %o, hl %d\n",
		p, p[-1], p[-1], lastc, lastc[-1], oldp,
		oldp ? oldp[-1] : 0, hl);
#endif
	}
	lastc--;

	lcol = scol; lrow = sline;
	if (oldlp) {
		oldline = oldlp->body;
		oldlen = oldlp->length;
		/* Check for runs of stuff that's already there. */
		for (p=first,oldp=oldline+lcol; p<=last; p++,oldp++) {
			if (*p==*oldp &&
				(px=p+1,oldpx=oldp+1,*px++==*oldpx++)
					  && *px++==*oldpx++ && *px==*oldpx)
				hl++;	/* a run of at least 4 matches */
#ifdef DEBUG
	if(outf) fprintf(outf,
	"p %x '%c%c%c%c', oldp %x '%c%c%c%c', hl %d\n",
	p, p[0], p[1], p[2], p[3],
	oldp, oldp[0], oldp[1], oldp[2], oldp[3],
	hl);
#endif
		}
	} else {
		oldline = NULL;
		oldlen = 0;
	}

	if (!hl) {
		/* Simple, common case.  Do it fast. */
		_pos(lrow, lcol);
		_hlmode(0);
		_writechars(firstc, lastc);
		return;
	}

#ifdef DEBUG
	if(outf) fprintf(outf,
		"oldlp %x, oldline %x, oldlen %d 0x%x\n",
		oldlp, oldline, oldlen, oldlen);
	if(outf) fprintf(outf, "old body('");
	if (oldlp)
		for (p=oldline; p<=oldline+oldlen; p++)
			if(outf) fprintf(outf, "%c", *p);
	if(outf) fprintf(outf, "').\n");
#endif
	oldc = first[-1];
	tailoldp = oldline + oldlen;
	for (p=first, oldp=oldline+lcol, pc=firstc; pc<=lastc;
						p++,oldp++,pc++) {
		thishl = *p & A_ATTRIBUTES;
#ifdef DEBUG
		if(outf) fprintf(outf,
			"prevhl %o, thishl %o\n", prevhl, thishl);
#endif
		leftoldc = oldc & A_ATTRIBUTES;
		leftnewc = p[-1] & A_ATTRIBUTES;
		diff_cookies = (magic_cookie_glitch>=0) &&
					(leftoldc != leftnewc);

		if (oldp >= tailoldp)
			oldc = ' ';
		else
			oldc = *oldp;
#ifdef DEBUG
		if(outf) fprintf(outf,
"p %x *p %o, pc %x *pc %o, oldp %x, *oldp %o, lcol %d, lrow %d, oldc %o\n",
p, *p, pc, *pc, oldp, *oldp, lcol, lrow, oldc);
#endif
		if (*p != oldc || SP->virt_irm == 1 || diff_cookies ||
			ceol_standout_glitch || insert_null_glitch &&
			(oldp >= oldline+oldlen)) {
			register int n;

			_pos(lrow, lcol);

			/*
			 * HP 2645/2626: make sure attributes are set when 
			 * necessary.
			 */
			if (ceol_standout_glitch && 
				(thishl != (oldc&A_ATTRIBUTES))) {
#ifdef DEBUG
				if(outf) fprintf(outf,
					"ceol %d, thishl %d, prevhl %d\n",
					ceol_standout_glitch, thishl, prevhl);
#endif
				_forcehl();
			}

			/* Force highlighting to be right */
			_hlmode(thishl);
			if (thishl != prevhl) {
				if (magic_cookie_glitch >= 0 &&
				    oldlen != 0) {
					_sethl();
					p += magic_cookie_glitch;
					oldp += magic_cookie_glitch;
					pc += magic_cookie_glitch;
					lcol += magic_cookie_glitch;
				}
			}

			/*
			 * Gather chunks of chars together, to be more
			 * efficient, and to allow repeats to be detected.
			 * Not done for blanks on cookie terminals because
			 * the last one might be a cookie.
			 */
			if (magic_cookie_glitch<0 || *pc != ' ') {
				for (px=p+1,oldpx=oldp+1;
					px<=last && *p==*px;
					px++,oldpx++) {
					if(!(repeat_char && oldpx<tailoldp &&
							*p==*oldpx))
						break;
				}
				px--; oldpx--;
				n = px - p;
				pcx = pc + n;
			} else {
				n = 0;
				pcx = pc;
			}
			if (((magic_cookie_glitch>=0) &&
			    (prevhl == thishl) &&
			    (SP->phys_gr != SP->virt_gr)) ||
			    (SP->virt_irm==1)) {
				SP->phys_gr = thishl;
#ifdef DEBUG
			if(outf) fprintf(outf,
	"cookie %d, thishl %d, prevhl %d SP->phys_gr 0x%x SP->virt_gr 0x%x\n",
		magic_cookie_glitch, thishl, prevhl, SP->phys_gr, SP->virt_gr);
#endif
			}
#ifdef DEBUG
			if (outf) fprintf(outf,"I'm here\n");
			if(outf) fprintf(outf,
	"cookie %d, thishl %d, prevhl %d SP->phys_gr 0x%x SP->virt_gr 0x%x\n",
		magic_cookie_glitch, thishl, prevhl, SP->phys_gr, SP->virt_gr);
#endif
			_writechars(pc, pcx);
			lcol += n; pc += n; p += n; oldp += n;
			prevhl = thishl;
		}
		lcol++;
	}
	if (magic_cookie_glitch >= 0 && prevhl) {
		/* Have to turn off highlighting at end of line */
#ifdef DEBUG
			if(outf)
				fprintf(outf,"cookie %d, prevhl %d\n",
					magic_cookie_glitch, prevhl);
#endif
		_hlmode(0);
		_sethl();
	}
}

