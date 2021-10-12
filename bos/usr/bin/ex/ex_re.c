static char sccsid[] = "@(#)98	1.18  src/bos/usr/bin/ex/ex_re.c, cmdedit, bos41J, 9523A_all 6/2/95 16:44:28";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_re.c
 *
 * FUNCTIONS: advance, cclass, cerror, compile, comprhs, compsub, confirmed,
 * dosub, dosubcon, execute, fixcase, getsub, global, inrange, place,
 * re_classcode, same, snote, substitute, ugo, expand_chandle,
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ex_re.c  1.6  com/cmd/edit/vi,3.1,9013 2/20/90 17:03:11
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_re.h"

static void cerror(char *);

#ifdef DEBUG
static debug_out_i(char * file, char * string, int data)
{
FILE *fp;
fp=fopen(file,"a");
fprintf(fp,"%s: %d\n",string,data);
fclose(fp);
}
static debug_out_c(char * file, char * string, wchar_t data)
{
FILE *fp;
fp=fopen(file,"a");
fprintf(fp,"%s: %C\n",string,data);
fclose(fp);
}
#endif

#ifndef _NO_PROTO
wchar_t colval(wchar_t c);
#endif
/*
 * Global, substitute and regular expressions.
 * Very similar to ed, with some re extensions and
 * confirmed substitute.
 */
void global(short k)
{
	register wchar_t *gp;
	register int c;
	register line *a1;
	wchar_t globuf[GBSIZE];
	char *Cwas;
	int nlines = lineDOL();
	int oinglobal = inglobal;
	wchar_t *oglobp = globp;
	Cwas = Command;
	/*
	 * States of inglobal:
	 *  0: ordinary - not in a global command.
	 *  1: text coming from some buffer, not tty.
	 *  2: like 1, but the source of the buffer is a global command.
	 * Hence you're only in a global command if inglobal==2. This
	 * strange sounding convention is historically derived from
	 * everybody simulating a global command.
	 */
	if (inglobal==2)
		error(MSGSTR(M_111, "Global within global@not allowed"), DUMMY_INT);
	markDOT();
	setall();
	nonzero();
	if (skipend())
		error(MSGSTR(M_112, "Global needs re|Missing regular expression for global"), DUMMY_INT);
	c = ex_getchar();
	ignore(compile(c, 1));
	savere(scanre);
	gp = globuf;
	while ((c = ex_getchar()) != '\n') {
		switch (c) {

		case EOF:
			c = '\n';
			goto brkwh;

		case '\\':
			c = ex_getchar();
			switch (c) {

			case '\\':
				ungetchar(c);
				break;

			case '\n':
				break;

			default:
				*gp++ = '\\';
				break;
			}
			break;
		}
		*gp++ = c;
		if (gp > &globuf[GBSIZE])
			error(MSGSTR(M_113, "Global command too long"), DUMMY_INT);
	}
brkwh:
	ungetchar(c);
	donewline();
	*gp++ = c;
	*gp++ = 0;
	saveall();
	inglobal = 2;
	for (a1 = one; a1 <= dol; a1++) {
		*a1 &= ~01;
		if (a1 >= addr1 && a1 <= addr2 && execute(0, a1) == k)
			*a1 |= 01;
	}
	if (inopen)
		inopen = -1;
	/*
	 * Now for each marked line, set dot there and do the commands.
	 * Note the n^2 behavior here for lots of lines matching.
	 * This is really needed: in some cases you could delete lines,
	 * causing a marked line to be moved before a1 and missed if
	 * we didn't restart at zero each time.
	 */
	for (a1 = one; a1 <= dol; a1++) {
		if (*a1 & 01) {
			*a1 &= ~01;
			dot = a1;
			globp = globuf;
			commands((short)1, (short)1);
			a1 = zero;
		}
	}
	globp = oglobp;
	inglobal = oinglobal;
	endline = 1;
	Command = Cwas;
	netchHAD(nlines);
	setlastchar(EOF);
	if (inopen) {
		ungetchar(EOF);
		inopen = 1;
	}
}

static short	cflag;
static int	scount, slines, stotal;

int substitute(int c)
{
	register line *addr;
	register int n;
	int gsubf, hopcount;

	gsubf = compsub(c);
	if(FIXUNDO)
		save12(), undkind = UNDCHANGE;
	stotal = 0;
	slines = 0;
	for (addr = addr1; addr <= addr2; addr++) {
		scount = hopcount = 0;
		if (dosubcon((short)0, addr) == 0)
			continue;
		if (gsubf) {
			/*
			 * The loop can happen from s/\</&/g
			 * but we don't want to break other, reasonable cases.
			 */
			hopcount = 0;
			while (*loc2) {
				if (++hopcount > sizeof linebuf)
					error(MSGSTR(M_114, "substitution loop"), DUMMY_INT);
				if (dosubcon((short)1, addr) == 0)
					break;
			}
		}
		if (scount) {
			stotal += scount;
			slines++;
			putmark(addr);
			n = append(getsub, addr);
			addr += n;
			addr2 += n;
		}
	}
	if (stotal == 0 && !inglobal && !cflag)
		error(MSGSTR(M_115, "Fail|Substitute pattern match failed"), DUMMY_INT);
	snote(stotal, slines);
	return (stotal);
}

static int compsub(int ch)
{
	register int seof, c, uselastre;
	static int gsubf;
	if (!value(EDCOMPATIBLE))
		gsubf = cflag = 0;
	uselastre = 0;
	switch (ch) {

	case 's':
		ignore(skipwh());
		seof = ex_getchar();
		if (endcmd(seof) || any(seof, "gcr")) {
			ungetchar(seof);
			goto redo;
		}
		if (iswalnum(seof))
			error(MSGSTR(M_116, "Substitute needs re|Missing regular expression for substitute"), DUMMY_INT);
		seof = compile(seof, 1);
		uselastre = 1;
		comprhs(seof);
		if (!value(EDCOMPATIBLE))
			gsubf = cflag = 0;
		break;

	case '~':
		uselastre = 1;
		/* fall into ... */
	case '&':
	redo:
		if (re.Expbuf[0] == 0)
			error(MSGSTR(M_117, "No previous re|No previous regular expression"), DUMMY_INT);
		if (subre.Expbuf[0] == 0)
			error(MSGSTR(M_118, "No previous substitute re|No previous substitute to repeat"), DUMMY_INT);
		break;
	}
	for (;;) {
		c = ex_getchar();
		switch (c) {

		case 'g':
			gsubf = !gsubf;
			continue;

		case 'c':
			cflag = !cflag;
			continue;

		case 'r':
			uselastre = 1;
			continue;

		default:
			ungetchar(c);
			setcount();
			donewline();
			if (uselastre)
				savere(subre);
			else
				resre(subre);
			return (gsubf);
		}
	}
}

static void comprhs(int seof)
{
	register wchar_t *rp, *orp;
	register int c;
	wchar_t orhsbuf[RHSSIZE + 1];

	rp = rhsbuf;
	CP(orhsbuf, rp);
	for (;;) {
		c = ex_getchar();
		if (c == seof)
			break;
		switch (c) {

		case '\\':
			c = ex_getchar();
			if (c == EOF) {
				ungetchar(c);
				break;
			}
			if (value(MAGIC)) {
				/*
				 * When "magic", \& turns into a plain &,
				 * and all other chars work fine quoted.
				 */
				if (c != '&')
					*rp++ = '\\';
				break;
			}
magic:
			if ((c == '%') && (value(EDCOMPATIBLE))) {
				for (orp = orhsbuf; *orp; *rp++ = *orp++)
					if (rp > &rhsbuf[RHSSIZE + 1])
						goto toobig;
				continue;
			}

			if (c == '~') {
				for (orp = orhsbuf; *orp; *rp++ = *orp++)
					if (rp > &rhsbuf[RHSSIZE + 1])
						goto toobig;
				continue;
			}
			*rp++ = '\\';
			break;

		case '\n':
		case EOF:
			if (!(globp && globp[0])) {
				ungetchar(c);
				goto endrhs;
			}

		case '~':
		case '%':
		case '&':
			if (value(MAGIC))
				goto magic;
			break;
		}
		if (rp > &rhsbuf[RHSSIZE + 1]) {
toobig:
			*rp = 0;
			error(MSGSTR(M_119, "Replacement pattern too long@- limit 256 characters"), DUMMY_INT);
		}
		*rp++ = c;
	}
endrhs:
	*rp++ = 0;
}

int getsub(void)
{
	register wchar_t *p;

	if ((p = linebp) == 0)
		return (EOF);
	strcLIN(p);
	linebp = 0;
	return (0);
}

static int dosubcon(short f, line *a)
{

	if (execute(f, a) == 0)
		return (0);
	if (confirmed(a)) {  
		dosub();
		scount++;
	}
	return (1);
}

#define YNSIZ	32
static int confirmed(line *a)
{
	register int c;
	char ynbuf[YNSIZ];
	char *ynptr;

	if (cflag == 0)
		return (1);
	pofix();
	pline(lineno(a));
	if (inopen) {
		ex_putchar(QUOTE_NL);
	}
	c = column(loc1 - 1);
	ugo(c - 1 + (inopen ? 1 : 0), ' ');
	ugo(column(loc2 - 1) - c, '^');
	flush();
	ynptr = ynbuf;
	while ((c = getkey()) != EOF && c != '\n' && c != '\r') {
		if (ynptr < &ynbuf[YNSIZ-2]) {
			ynptr += wctomb(ynptr, (wchar_t)c);
			if (inopen) {
				ex_putchar(c);
				flush();
			}
		}
	}
	if (c == '\r')
		c = '\n';
	if (inopen) {
		ex_putchar(c);
		flush();
	}
	*ynptr = '\0';
	noteinp();
	return (rpmatch(ynbuf) == 1);
}
static void ugo(int cnt, int with)
{

	if (cnt > 0)
		do
			ex_putchar(with);
		while (--cnt > 0);
}

static int	casecnt;
static short	destuc;

static void dosub(void)
{
	register wchar_t *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	casecnt = 0;
	while (c = *rp++) {
		/* ^V <return> from vi to split lines */
		if (c == '\r')
			c = '\n';

		if (c == '\\') {
			c = *rp++;
			switch (c) {

			case '&':
				sp = place(sp, loc1, loc2);
				if (sp == 0)
					goto ovflo;
				continue;

			case 'l':
				casecnt = 1;
				destuc = 0;
				continue;

			case 'L':
				casecnt = LBSIZE;
				destuc = 0;
				continue;

			case 'u':
				casecnt = 1;
				destuc = 1;
				continue;

			case 'U':
				casecnt = LBSIZE;
				destuc = 1;
				continue;

			case 'E':
			case 'e':
				casecnt = 0;
				continue;
			}
			if (c >= '1' && c < nbra + '1') {
				sp = place(sp, braslist[c - '1'], braelist[c - '1']);
				if (sp == 0)
					goto ovflo;
				continue;
			}
		}
		if (casecnt)
			c = fixcase(c);
		*sp++ = c;

		if (sp > &genbuf[LBSIZE])
ovflo:
			error(MSGSTR(M_120, "Line overflow@in substitute"), DUMMY_INT);
	}
	lp = loc2;
	loc2 = sp + (linebuf - genbuf);
	while (*sp++ = *lp++)
		if (sp >= &genbuf[LBSIZE])
			goto ovflo;
	strcLIN(genbuf);
}

static int fixcase(register int c)
{

	if (casecnt == 0)
		return (c);
	casecnt--;
	if (destuc) {
		if (iswlower(c))
			c = towupper(c);
	} else
		if (iswupper(c))
			c = towlower(c);
	return (c);
}

static wchar_t *place(register wchar_t *sp, register wchar_t *l1, register wchar_t *l2)
{

	while (l1 < l2) {
		*sp++ = fixcase(*l1++);
		if (sp > &genbuf[LBSIZE])
			return (0);
	}
	return (sp);
}

static void snote(register int total, register int nlines)
{

	if (!notable(total))
		return;
	ex_printf(mesg(MSGSTR(M_121, "%d subs|%d substitutions")), total);
	if (nlines != 1 && nlines != total)
		ex_printf(MSGSTR(M_122, " on %d lines"), nlines);
	noonl();
	flush();
}

static wchar_t compress_chandle(wchar_t wc)
{
int i=1,j=0;
while (!(i & wc)) {
	i = i<<1;
	j++;
	}
return (wchar_t)j;
}

static wchar_t expand_chandle(wchar_t wc)
{
int i=1;
while ((int)wc) {
	i = i<<1;
	wc--;
	}
return (wchar_t)i;
}

static wchar_t re_classcode(wchar_t *id, wchar_t *handle)
{

	char mbbuf[64];

	if (wcstombs(mbbuf, id, 64) == -1)
		error(MSGSTR(M_651, "Invalid wide character string, conversion failed."), DUMMY_INT);
	*handle = get_wctype(mbbuf);
	if (*handle == (wctype_t) -1)
	{
		error(MSGSTR(M_670, "Property invalid for locale."), DUMMY_INT);
		return (CL_BADCLASS);
	}
	*handle=compress_chandle(*handle);
	return (CL_GOODCLASS);

}

#ifdef DEBUG

static wchar_t * debug_show_brack(wchar_t *p, FILE *fp)
{
wchar_t * end_pt;

end_pt = p + *p;

fprintf(fp,"size=%x ",*p++);
while (p<end_pt) {
	if (*p!='\\')
		fprintf(fp,"<%C> ",*p++);
	else {
		p++;
		switch (*p++) {
			case CL_BACKSLASH :
				fprintf(fp,"CL_BACKSLASH ");
				break;
			case CL_RANGE:
				if (*p!=0 && p[1]!=0) {
					fprintf(fp,"CL_RANGE <%C>-<%C> ",*p,p[1]);
					p+=2;
					}
				else {
					wchar_t * ptr=p+2;
					fprintf(fp,"CL_RANGE ");
					if (*p) 
						fprintf(fp,"<%C>-",*p);
					else {
						fprintf(fp,"[.");
						for (;*ptr;ptr++)
							fprintf(fp,"%C",*ptr);
						fprintf(fp,".]-");
						ptr++;
						}
					if (p[1])
						fprintf(fp,"<%C> ",p[1]);
					else {
						fprintf(fp,"[.");
						for (;*ptr;ptr++)
							fprintf(fp,"%C",*ptr);
						fprintf(fp,".] ");
						ptr++;
						}
					p=ptr;
					}
				break;
			case CL_GOODCLASS:
				fprintf(fp,"CL_GOODCLASS %x ",
							expand_chandle(*p));
				p++;
				break;
			case CL_EQUIVCLASS:
				fprintf(fp,"CL_EQUIVCLASS %x ",*p);
				p++;
				break;
			case CL_COLLEL: {
				fprintf(fp,"CL_COLLEL ");
				for (;*p;p++)
					fprintf(fp,"<%C> ",*p);
				p++;
				break;
				}
			default:
				fprintf(fp,"unknown bracket (%x) ",*p++);
				break;
			}
		}
	}
return p;
}


static void debug_show_exp(wchar_t * p)
{
FILE * fp;
fp=fopen("debug.pat","w");
if (circfl) fprintf(fp,"CIRCUMFLEX ");
for (;;) {
	switch (*p++) {
		case CBRA | INTERVAL:
			fprintf (fp,"CBRA INTERVAL (%d,%d) (%x) ",*p++,*p++,*p++);
			break;
		case CBRA | STAR:
			fprintf (fp,"CBRA STAR(%x) ",*p++);
			break;
		case CBRA:
			fprintf (fp,"CBRA(%x) ",*p++);
			break;
		case CDOT|STAR:
			fprintf (fp,"CDOT STAR ");
			break;
		case CDOT|INTERVAL:
			fprintf (fp,"CDOT INTERVAL (%d,%d) ",*p++,*p++);
			break;
		case CDOT:
			fprintf (fp,"CDOT ");
			break;
		case CCL|STAR:
			fprintf (fp,"CCL STAR ");
			p=debug_show_brack(p,fp);
			break;
		case CCL|INTERVAL:
			fprintf (fp,"CCL INTERVAL ");
			fprintf (fp,"(%d,%d) ",*p++,*p++);
			p=debug_show_brack(p,fp);
			break;
		case CCL:
			fprintf (fp,"CCL ");
			p=debug_show_brack(p,fp);
			break;
		case NCCL|STAR:
			fprintf (fp,"NCCL STAR ");
			p=debug_show_brack(p,fp);
			break;
		case NCCL|INTERVAL:
			fprintf (fp,"NCCL INTERVAL ");
			fprintf (fp,"(%d,%d) ",*p++,*p++);
			p=debug_show_brack(p,fp);
			break;
		case NCCL:
			fprintf (fp,"NCCL ");
			p=debug_show_brack(p,fp);
			break;
		case CBOL:
			fprintf (fp,"CBOL ");
			break;
		case CDOL:
			fprintf (fp,"CDOL ");
			break;
		case CEOFC:
			fprintf (fp,"CEOFC\n");
			fclose(fp);
			return;
			break;
		case CKET|STAR:
			fprintf (fp,"CKET BAD STAR(%x) ",*p++);
			break;
		case CKET|INTERVAL:
			fprintf (fp,"CKET BAD INTERVAL (%d,%d) (%x) ",*p++,*p++,*p++);
			break;
		case CKET:
			fprintf (fp,"CKET(%x) ",*p++);
			break;
		case CCHR|STAR:
			fprintf (fp,"CCHR STAR(%C) ",*p++);
			break;
		case CCHR|INTERVAL:
			fprintf (fp,"CCHR INTERVAL (%d,%d) (%C) ",*p++,*p++,*p++);
			break;
		case CCHR:
			fprintf (fp,"CCHR(%C) ",*p++);
			break;
		case CBRC|STAR:
			fprintf (fp,"CBRC STAR ");
			break;
		case CBRC|INTERVAL:
			fprintf (fp,"CBRC INTERVAL (%d,%d) ",*p++,*p++);
			break;
		case CBRC:
			fprintf (fp,"CBRC ");
			break;
		case CLET | STAR:
			fprintf (fp,"CLET STAR ");
			break;
		case CLET | INTERVAL:
			fprintf (fp,"CLET INTERVAL (%d,%d) ",*p++,*p++);
			break;
		case CBACKREF|STAR:
			fprintf (fp,"CBACKREF STAR(%x) ",*p++);
			break;
		case CBACKREF|INTERVAL:
			fprintf (fp,"CBACKREF INTERVAL (%d,%d) (%x) ",*p++,*p++,*p++);
			break;
		case CBACKREF:
			fprintf (fp,"CBACKREF(%x) ",*p++);
			break;
		case CFERKCAB:
			fprintf (fp,"CFERKCAB(%x) ",*p++);
			break;
		case CFERKCAB|STAR:
			fprintf (fp,"CFERKCAB BAD STAR(%x) ",*p++);
			break;
		case CFERKCAB|INTERVAL:
			fprintf (fp,"CFERKCAB BAD INTERVAL (%d,%d) (%x) ",*p++,*p++,*p++);
			break;
		default:
			fprintf (fp,"unknown (%x) ",*p++);
		}
	}
}


void show_matches()
{
int i;
wchar_t * ptr;
FILE * fp;
fp=fopen("debug.pat","a");
fprintf(fp,"Subexpression matches:\n");
for (i=0;i<nbra;i++) {
	fprintf(fp,"%d: ",i);
	for (ptr=braslist[i];ptr<braelist[i];ptr++)
		fprintf(fp,"%C",*ptr);
	fprintf(fp,"\n");
	}
fclose(fp);
}
#endif

static wchar_t * base_offset;   /* start of string to match */

int compile(int eof, int oknl)
{
	register int c;
	int c_tmp;
	register wchar_t *ep;
	wchar_t *lastep=expbuf; /* last expr command */
	wchar_t bracket[NBRA], *bracketp, *rhsp;
	wchar_t *bracket_ep[NBRA];
	wchar_t *lastcp; /* beginning of last range expr */
	wchar_t *lastkp; /* store tmp pntr to charclass */

	if (iswalnum(eof))
		error(MSGSTR(M_123, "Regular expressions cannot be delimited by letters or digits"), DUMMY_INT);
	ep = expbuf;
	c = ex_getchar();
	if (eof == '\\')
		switch (c) {

		case '/':
		case '?':
			if (scanre.Expbuf[0] == 0)
error(MSGSTR(M_124, "No previous scan re|No previous scanning regular expression"), DUMMY_INT);
			resre(scanre);
			return (c);

		case '&':
			if (subre.Expbuf[0] == 0)
error(MSGSTR(M_118, "No previous substitute re|No previous substitute regular expression"), DUMMY_INT);
			resre(subre);
			return (c);

		default:
error(MSGSTR(M_126, "Badly formed re|Regular expression \\ must be followed by / or ?"), DUMMY_INT);
		}
	if (c == eof || c == '\n' || c == EOF) {
		if (*ep == 0)
error(MSGSTR(M_117, "No previous re|No previous regular expression"),	DUMMY_INT);
		if (c == '\n' && oknl == 0)
error(MSGSTR(M_128, "Missing closing delimiter@for regular expression"), DUMMY_INT);
		if (c != eof)
			ungetchar(c);
		return (eof);
	}
	bracketp = bracket;
	nbra = 0;
	circfl = 0;
	if (c == '^') {
		c = ex_getchar();
		circfl++;
	}
	ungetchar(c);
	for (;;) {
		if (ep >= &expbuf[ESIZE - 2])
complex:
			cerror(MSGSTR(M_129, "Re too complex|Regular expression too complicated"));
		c = ex_getchar();
		if (c == eof || c == EOF) {
			if (bracketp != bracket)
				cerror(MSGSTR(M_130, "Unmatched \\(|More \\('s than \\)'s in regular expression"));
			*ep++ = CEOFC;
			if (c == EOF)
				ungetchar(c);
			#ifdef DEBUG
			debug_show_exp(expbuf);
			#endif
			return (eof);
		}
jumpin:
		if (value(MAGIC)) {
			if (c != '*' || ep == expbuf ||
					 (nbra>0 && (ep[-2]==CBRA || (ep[-1]==CBOL && ep[-3]==CBRA))))
				if (c != '\\' || peekchar() != '{')
					lastep = ep;
		} else
			if (c != '\\' || peekchar() != '*' 
				 || ep == (wchar_t *)expbuf 
				 || (nbra>0 && (ep[-2]==CBRA || (ep[-1]==CBOL && ep[-3]==CBRA))))
				if (c != '\\' || peekchar() != '{')
					lastep = ep;
/* comment to add corresponding curely braces: }} */

		switch (c) {

		case '\\':
			c = ex_getchar();
			switch (c) {

			case '(':
				if (nbra >= NBRA) {
cerror(MSGSTR(M_131, "Awash in \\('s!|Too many \\('d subexressions in a regular expression"));
				}
				*bracketp++ = nbra;
				bracket_ep[nbra]=ep;
				*ep++ = CBRA;
				*ep++ = nbra++;
				continue;

			case ')':
				if (bracketp <= bracket)
cerror(MSGSTR(M_132, "Extra \\)|More \\)'s than \\('s in regular expression"));
				*ep++ = CKET;
				*ep++ = *--bracketp;
				lastep=bracket_ep[*bracketp];
				continue;

			case '{': {
				int val1=0,curval=0;
				int seen_comma=0;
				wchar_t * ptr;
				if (ep >= &expbuf[ESIZE-2]) goto complex;
				switch (*lastep) {
					case CBRA:
					case CCL:
					case NCCL:
					case CCHR:
					case CBACKREF:
						/* first make room by shifting by 2 */
						for (ptr=ep-1;ptr>lastep;ptr--)
							ptr[2]=*ptr;
						*lastep |= INTERVAL;
						break;
					default:
						*lastep |= INTERVAL;
						lastep=ep-1;
					}
				ep+=2;
				/* next two values in ep buffer are the min and max */
				/* allowable repetitions.  -1 for max is open bound */
				while (1) {
					c = ex_getchar();
					if (!any(c,"0123456789\\,"))
						error(MSGSTR(M_673,"invalid character in interval expression"), DUMMY_INT);
						/* invalid interval sequence */
					if (c==',') {
						if (seen_comma)
							error(MSGSTR(M_674,"too many commas in interval expression"), DUMMY_INT);
							/* only one comma allowed in inverval sequence */
						seen_comma=1;
						val1=curval;
						curval=-1;
						continue;
						}
					if (c=='\\') {
						c=ex_getchar();
						if (c!='}')
							error(MSGSTR(M_675,"improperly terminated interval expression"), DUMMY_INT);
							/* improperly terminated inverval sequence */
						if (curval>RE_DUP_MAX || val1>RE_DUP_MAX)
							error(MSGSTR(M_676,"interval expression too large"), DUMMY_INT);
							/* maximum interval repeat number exceeded  */
						if (!seen_comma)
							val1 = curval;
						lastep[1] = val1;
						lastep[2] = curval;
						break;
						}
					if (curval==-1)
						curval=0;
					curval=curval*10+(c-'0');
					}
				}
				continue;

			case '<':
				*ep++ = CBRC;
				continue;

			case '>':
				*ep++ = CLET;
				continue;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if ((c-'0') > nbra)/* could generate error, but just */
					continue;  /* ignore invalid back reference   */
				*ep++ = CBACKREF;
				*ep++ = c-'1';
				continue;
			}
			if (value(MAGIC) == 0)
magic:
			switch (c) {

			case '.':
				*ep++ = CDOT;
				continue;

			case '~':
				rhsp = rhsbuf;
				while (*rhsp) {
					c = *rhsp++;
					if (c == '\\') {
						c = *rhsp++;
						if (c == '&')
error(MSGSTR(M_133, "Replacement pattern contains &@- cannot use in re"), DUMMY_INT);
						if (c >= '1' && c <= '9')
error(MSGSTR(M_134, "Replacement pattern contains \\d@- cannot use in re"), DUMMY_INT);
					}
					if (ep >= &expbuf[ESIZE-2])
						goto complex;
					*ep++ = CCHR;
					*ep++ = c;
				}
				continue;

			case '*':
				if ((ep == expbuf) || ((nbra>0) &&
							 (ep[-2]==CBRA || (ep[-1]==CBOL && ep[-3]==CBRA))))
					break;
				*lastep |= STAR;
				continue;

			case '[': {
				int in_sub=0;
				if (ep >= &expbuf[ESIZE-2]) goto complex;
				*ep++ = CCL;
				*ep++ = 0;
				lastcp = lastkp = NULL;
				c = ex_getchar();
				if (c == '^') {
					c = ex_getchar();
					ep[-2] = NCCL;
				}
				if (c == ']') {
					if ((c_tmp=peekchar())=='\n' || c_tmp==EOF)
cerror(MSGSTR(M_137, "Bad character class|Empty character class '[]' or '[^]' cannot match"));
					*ep++ = CCHR;
					lastcp = ep;
					*ep++ = c;
					c = ex_getchar();
				}
				while (c != ']' || in_sub) {
					if (c == '-' && lastcp != NULL && peekchar() != ']' && !in_sub) {
						int first_was_collel=0;
						wchar_t *update=0;
						if (ep >= &expbuf[ESIZE-4]) goto complex;
						if (*lastcp=='\\' && lastcp[1]==CL_COLLEL) {
							/* need to shift by two bytes */
							wchar_t * eos=lastcp+2;
							first_was_collel=1;
							while (*eos)
								eos++;
							update=eos+3;
							while (eos>=lastcp+2)
								eos[2]=*eos--;
							c=0; /* indicate collel */
							}
						else
							c=*lastcp;
						ep = lastcp;
						*ep++ = '\\';
						*ep++ = CL_RANGE;
						*ep++ = c;
						c = ex_getchar();
						if (c=='[' && peekchar()=='.') {
							/* may be collel... */
							*ep++ = 0;
							if (first_was_collel)
								while (*ep++);
							(void) ex_getchar();
							while (1) {
								c = ex_getchar();
								if (c==eof || c == EOF || c == '\n')
									error(MSGSTR(M_138, "Missing ]"), DUMMY_INT);
								if (c=='.' && peekchar()==']') {
									/* end of collel */
									(void)ex_getchar();
									break;
									}
								*ep++=c;
								} /* while (1) */
							update=ep+1;
							c=0;
							}
						*ep++ = c;
						if (update)
							ep=update;
						lastcp = NULL;
					} else {
						lastcp = ep;
if (lastkp != NULL && ((any(c,":=.") && lastkp[-1]==c && peekchar() == ']'))) {
							wchar_t match_code;
							*ep = '\0';
							switch (c) {
								case ':' :
							if ((c = re_classcode(lastkp,&match_code)) == CL_BADCLASS)
cerror(MSGSTR(M_252, "Unrecognized ctype@class in [: ... :] construct"));
									(void)ex_getchar();
									lastkp[-2]='\\';
									lastkp[-1]=c;
									ep = lastkp;
									lastcp = NULL;
									c = match_code;
									break;
								case '=' :
									(void)ex_getchar();
									lastkp[-2]='\\';
									lastkp[-1]=CL_EQUIVCLASS;
									ep = lastkp;
									lastcp = NULL;
									c = colval(*lastkp);
									in_sub=0;
									break;
								case '.' :
									(void)ex_getchar();
									lastkp[-2]='\\';
									lastkp[-1]=CL_COLLEL;
									lastcp = lastkp-2;
									in_sub=0;
									c=0;
									break;
								default:
									error(MSGSTR(M_141,"Re internal error"), DUMMY_INT);
									break;
							}
							lastkp=NULL;
						}
						if (ep >= &expbuf[ESIZE-2]) goto complex;
						*ep++ = c;
						if (c == '\\') *ep++ = CL_BACKSLASH;
						if (c == '[' && any(peekchar(),":=.")) {
							lastcp = ep;
							*ep++ = ex_getchar();
							if (ep[-1]=='.' || ep[-1]=='=')
								in_sub = 1;
							lastkp = ep;
						}
					}
					c = ex_getchar();
					if (c==eof || c == EOF || c == '\n')
						error(MSGSTR(M_138, "Missing ]"), DUMMY_INT);
				}
				lastep[1] = ep - &lastep[1];
				continue;
				}
			}
			if (c == EOF) {
				ungetchar(EOF);
				c = '\\';
				goto defchar;
			}
			*ep++ = CCHR;
			if (c == '\n')
cerror(MSGSTR(M_139, "No newlines in re's|Can't escape newlines into regular expressions"));
			*ep++ = c;
			continue;

		case '\n':
			if (oknl) {
				ungetchar(c);
				*ep++ = CEOFC;
				#ifdef DEBUG
				debug_show_exp(expbuf);
				#endif
				return (eof);
			}
cerror(MSGSTR(M_140, "Badly formed re|Missing closing delimiter for regular expression"));

		case '$':
			if (peekchar() == eof || peekchar() == EOF || oknl && peekchar() == '\n') {
				*ep++ = CDOL;
				continue;
			}
			if (peekchar() == '\\') {
				wchar_t local_c;
				c=ex_getchar();
				local_c=peekchar();
				if (local_c == ')') {
					*ep++ = CDOL;
					if (ep >= &expbuf[ESIZE-2]) goto complex;
					goto jumpin;
					}
				else {
					*ep++ = CCHR;
					*ep++ = '$';
					if (ep >= &expbuf[ESIZE-2]) goto complex;
					goto jumpin;
					}
				}
			goto defchar;

  case '^':
			if (nbra>0 && ep[-2] == CBRA) {
				*ep++ = CBOL;
				continue;
				}
			goto defchar;

		case '.':
		case '~':
		case '*':
		case '[':
			if (value(MAGIC))
				goto magic;
defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
			continue;
		}
	}
}

static void cerror(char *s)
{

	expbuf[0] = 0;
	error(s, DUMMY_INT);
}

static int same(register int a, register int b)
{

	return (a == b || value(IGNORECASE) &&
	   ((iswlower(a) && towupper(a) == b) || (iswlower(b) && towupper(b) == a)));
}

static wchar_t	*locs, *floc;

static wchar_t *start_of_line; /* used for subexp ^ anchoring */
int execute(int gf, line *addr)
{
	register wchar_t *p1, *p2;
	register int c;

	int i;
	if (gf) {
		if (circfl)
			return (0);
		locs = p1 = loc2;
		start_of_line = NULL;
	} else {
		if (addr == zero)
			return (0);
		p1 = start_of_line = linebuf;
		getline(*addr);
		locs = 0;
	}
	for (i=0;i<NBRA;i++)
		braelist[i]=braslist[i]=0;
	floc = p1;
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		base_offset = p1;
		return (advance(p1, p2, 0, 0));
	}
	/* fast check for first character */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (c != *p1 && (!value(IGNORECASE) ||
			   !((iswlower(c) && towupper(c) == *p1) ||
			   (iswlower(*p1) && towupper(*p1) == c))))
				continue;
			base_offset = p1;
			if (advance(p1, p2, 0, 0)) {
				loc1 = p1;
				#ifdef DEBUG
				show_matches();
				#endif
				return (1);
			}
		} while (*p1++);
		return (0);
	}
	/* regular algorithm */
	do {
		base_offset = p1;
		if (advance(p1, p2, 0, 0)) {
			loc1 = p1;
			#ifdef DEBUG
			show_matches();
			#endif
			return (1);
		}
	} while (*p1++);
	return (0);
}


static int advance(wchar_t *lp, wchar_t *ep, int within_sub, int backref_star)
{
	register wchar_t *curlp;
	wchar_t *stored_ptr=NULL;
	wchar_t *malloc_ptr=NULL;
	static short skip[2048];
	short skip_value;
	int repeats=0;
	int max, min;
	static wchar_t * last_lp;
	static wchar_t * last_ep;
	static int temp;

	for (;;) 
	switch (*ep++) {

	case CCHR:
		if (!same(*ep, *lp)) {
			if (malloc_ptr)
				free(malloc_ptr);
			return (0);
			}
		ep++, lp++;
		skip[lp-base_offset]=1;
		continue;

	case CDOT:
		if (*lp++) {
			skip[lp-base_offset]=1;
			continue;
			}
		return (0);

	case CDOL:
		if (*lp == 0)
			continue;
		return (0);

	case CBOL:
		if (start_of_line == lp)
			continue;
		return (0);

	case CEOFC:
		loc2 = lp;
		return (1);

	case CCL:
		if (cclass(ep, lp, 1, &skip_value)) {
			ep += *ep;
			lp += skip_value;
			skip[lp-base_offset]=skip_value;
			continue;
		}
		return (0);

	case NCCL:
		if (cclass(ep, lp, 0, &skip_value)) {
			ep += *ep;
			lp += skip_value;
			skip[lp-base_offset]=skip_value;
			continue;
		}
		return (0);

	case CBRA:
		min = 1;
		max = 1;
		goto cbra_star;

	case CBRA|INTERVAL:
		min = *ep++;
		max = *ep++;
		goto cbra_star;
		
	case CBRA|STAR:
		min = 0;
		max = -1;
cbra_star:
		curlp = lp;
		if (braslist[*ep]==0) {
			braelist[*ep] = 0;
			braslist[*ep] = lp;
			}
		ep++;
		repeats=0;
		{
		int matched=0;
		while ((repeats<max || max==-1) &&
		    (matched=advance(lp,ep,ep[-1]+1,backref_star))) {
			repeats++;
			skip[last_lp-base_offset]=last_lp-lp;
			lp=last_lp;
			}
		if (matched)
			ep=last_ep;
		else {
			wchar_t bra_num=ep[-1];
			while (1) {
				if ((ep[0]==CKET) && (ep[1]==bra_num)) {
					ep+=2;
					break;
					}
				ep++;
				}
			if (repeats==0 && braelist[bra_num]==0)
				braelist[bra_num]=braslist[bra_num]=0;
			}
		}

		repeats++;
		lp++;
		skip[lp-base_offset]=1;
		if (repeats <= min)
			return (0);
		goto star;

	case CKET:
		if (braelist[*ep]==0)
			braelist[*ep] = lp;
		ep++;
		last_lp = lp;
		last_ep = ep;
		if (within_sub==ep[-1]+1)
			return(1);
		continue;

	case CBACKREF:
		min = 1;
		max = 1;
		goto cbackref_star;

	case CBACKREF|INTERVAL:
		min = *ep++;
		max = *ep++;
		goto cbackref_star;

	case CBACKREF|STAR:
		min = 0;
		max = -1;
cbackref_star:
		repeats=0;
		curlp = lp;
		{
		wchar_t *ptr1, *ptr2;
		int i;
		if (braelist[*ep]==braslist[*ep]) { /* null */
			ep++;
			continue;
			}
		stored_ptr=ep+1;
		malloc_ptr=(wchar_t *)
		   malloc(sizeof(wchar_t)*2*(braelist[*ep]-braslist[*ep])+1);
		if (malloc_ptr==NULL) {
			#ifdef DEBUG
			debug_out_i("prob","malloc start",braslist[*ep]);
			debug_out_i("prob","malloc end",braelist[*ep]);
			#endif
			error(MSGSTR(M_141, "Re internal error"), DUMMY_INT);
			}
		ptr2=malloc_ptr;
		for (ptr1=braslist[*ep];ptr1<braelist[*ep];ptr1++) {
			*ptr2++ = CCHR;
			*ptr2++ = *ptr1;
		    }
		*ptr2=CFERKCAB;
		ep=malloc_ptr;
		}

		while (advance(lp,malloc_ptr,within_sub,1) && (repeats<max || max==-1)) {
			repeats++;
			skip[last_lp-base_offset]=last_lp-lp;
			lp=last_lp;
			}

		free(malloc_ptr);
		ep=stored_ptr;
		if (repeats==1 && max==1 && min==1)
			continue;
		repeats++;
		lp++;
		skip[lp-base_offset]=1;
		if (repeats <= min)
			return (0);
		goto star;

	case CFERKCAB:
		if (backref_star) {
			last_lp=lp;
			return(1);
			}
		ep=stored_ptr;
		free(malloc_ptr);
		continue;

	case CDOT|INTERVAL:
		min = *ep++;
		max = *ep++;
		goto cdot_star;

	case CDOT|STAR:
		max = -1;
		min = 0;
cdot_star:
		curlp = lp;
		repeats = 0;
		while (*lp && (repeats<max || max==-1)) {
			lp++;
			skip[lp-base_offset]=1;
			repeats++;
			continue;
			}
		lp++;
		skip[lp-base_offset]=1;
		repeats++;
		if (repeats <= min)
			return (0);
		goto star;

	case CCHR|INTERVAL:
		min = *ep++;
		max = *ep++;
		goto cchr_star;

	case CCHR|STAR:
		max = -1;
		min = 0;
cchr_star:
		curlp = lp;
		repeats = 0;
		while (same(*lp, *ep) && (repeats<max || max==-1)) {
			lp++;
			repeats++;
			skip[lp-base_offset]=1;
			}
		repeats++;
		lp++;
		skip[lp-base_offset]=1;
		ep++;
		if (repeats <= min)
			return (0);
		goto star;

	case CCL|INTERVAL:
		temp = 1;
		min = *ep++;
		max = *ep++;
		goto ccl_star;

	case NCCL|INTERVAL:
		temp = 0;
		min = *ep++;
		max = *ep++;
		goto ccl_star;

	case CCL|STAR:
		temp = 1;
		max = -1;
		min = 0;
		goto ccl_star;

	case NCCL|STAR:
		temp = 0;
		max = -1;
		min = 0;

ccl_star:
		curlp = lp;
		repeats = 0;
		{
		int match=0;
		while ((match=cclass(ep, lp, temp, &skip_value))
				&& (repeats<max || max==-1)) {
			lp += skip_value;
			skip[lp-base_offset]=skip_value;
			repeats++;
			}
		lp++;
		skip[lp-base_offset]=1;
		repeats++;
		}
		ep += *ep;
		if (repeats <= min)
			return (0);
		goto star;
star:
		do {
			int i;
			repeats--;
			if (repeats<min)
				return(0);
			lp-=skip[lp-base_offset];
			for (i=0;i<nbra;i++)
				if (lp>=braslist[i] && lp<braelist[i] && braelist[i]!=0)
					braelist[i]=lp;
			if (lp <= locs)
				break;
			if (advance(lp, ep, within_sub, backref_star))
				return (1);
		} while (lp > curlp);
		return (0);

	case CBRC:
		if (lp == linebuf) {
			if (wordch(lp))
				continue;
			else
				return(0);
		}
		if (!wordch(&lp[-1]) && wordch(lp))
			continue;
		return (0);

	case CLET:
		/*
		 * If this is the first match attempt, then floc == linebuf,
		 * and we are not at the end of a word, otherwise floc point
		 * to the remainder of the string to be matched.  In that
		 * case, an end of word at this position has already been
		 * matched once by a previous call to advance().
		 */
		if (lp == floc)
			return(0);
		if (wordch(&lp[-1]) && !wordch(lp))
			continue;
		return (0);

	default:
		#ifdef DEBUG
		debug_out_i("prob","unknown pattern[-1]",ep[-1]);
		debug_out_i("prob","unknown pattern[0]",*ep);
		debug_out_i("prob","unknown pattern[1]",ep[1]);
		debug_out_i("prob","within_sub",within_sub);
		debug_out_i("prob","backref_star",backref_star);
		#endif
		error(MSGSTR(M_141, "Re internal error"), DUMMY_INT);
	}
}

static int
inrange(wchar_t c, wchar_t * lo, wchar_t * hi) 
{
        wchar_t tc;
	wchar_t wcs_c[2];
	
	wcs_c[0]=c;
	wcs_c[1]='\0';

	if ((wcscoll(wcs_c,lo)>=0) && (wcscoll(wcs_c,hi)<=0))
		return (1);
	if (!value(IGNORECASE))
		return (0);
	if ((tc = towupper(c)) == c) 
		tc = towlower(c);
	wcs_c[0]=tc;
	return ((wcscoll(wcs_c,lo)>=0) && (wcscoll(wcs_c,hi)<=0));
}

static wchar_t colval(wchar_t c)
{
    wchar_t cvalue[MAXCOLVAL], colvalue[MAXCOLVAL];

	cvalue[0] = c;
	cvalue[1] = 0;
	if(wcsxfrm(colvalue, cvalue, MAXCOLVAL) >= MAXCOLVAL){
		error(MSGSTR(M_671, "Error in character collation value"), DUMMY_INT);
	}
    return (colvalue[0]);
}

static int cclass(register wchar_t *sset, wchar_t *s, int af, short *skip)
{
	register wchar_t c;
	register wchar_t *endset;

	c=*s;
	*skip = 1;

	if (c == 0)
		return (0);

	endset = sset;
	endset += *sset++;
	while (sset < endset) {
		if (*sset != '\\') {
			if (c == *sset++) return (af);
		} else {
			++sset;
			switch (*sset++) {
			case CL_BACKSLASH:
				if (c == '\\') return (af);
				break;
			case CL_RANGE: {
				wchar_t wcs0[2], wcs1[2];
				wcs0[0]=sset[0];
				wcs1[0]=sset[1];
				wcs0[1]=wcs1[1]=0;
				if (sset[0] != 0 && sset[1] != 0) {
					if (inrange(c, wcs0, wcs1))
						return (af);
					sset += 2;
					}
				else {
					wchar_t * ptr = sset + 2;
					wchar_t *p0,*p1;
					wchar_t colsym0=sset[0],colsym1=sset[1];
					sset+=2;
					if (colsym0 != 0)
						p0 = wcs0;
					else {
						p0=sset;
						for (;*sset;sset++) ;
						sset++;
						}
					if (colsym1 != 0)
						p1=wcs1;
					else {
						p1=sset;
						for (;*sset;sset++) ;
						sset++;
						}
					if (inrange(c, p0, p1))
						return (af);
					}
				break;
				}

			case CL_GOODCLASS:
				if (is_wctype(c, expand_chandle(sset[0])))
					return (af);
				sset++;
				break;

			case CL_EQUIVCLASS:
				if (colval(c) == sset[0])
					return (af);
				sset++;
				break;

			case CL_COLLEL: {
				wchar_t *ptr;
				wchar_t *t = s;

				for (ptr=sset;*ptr;ptr++,t++) {
					if (!same(*ptr,*t))
						goto notfound;
					}
				/* found */
				*skip = ptr-sset;
				return (af);
				
notfound:
				while (*sset++);
				if (*sset==0)
					sset+1;
				break;
				}

			default:
					error(MSGSTR(M_253, "Internal regexp error in cclass"), DUMMY_INT);
				}
			}
		}
		return (!af);
}
