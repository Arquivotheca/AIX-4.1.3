static char sccsid[] = "@(#)32        1.25  src/bos/usr/bin/trcrpt/lex.c, cmdtrace, bos411, 9428A410j 4/1/94 05:16:57";

/*
*/
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: pass1, pass2, yylex, gettd, freetd, stracpy, funcdef
 *            strdump, lexptstats, numchk
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *
 */

/*
 * Lexical analyzer for yyparse
 */

#define _ILS_MACROS

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/trchkid.h>
#include "rpt.h"
#include "sym.h"
#include "td.h"
#include "parse.h"

extern pass2excp();
extern char *Default_tmplt;
extern FILE *Logfp;

#define ISDELIM(c) ((c) == '\n' || (c) == '\t' || (c) == ' ')
#define QUOTE    '\''
#define QUOTE2   '"'
#define XQUOTE '`'

int Logtraceid_real;

static char *matchstr = "******";	/* special string for \* */
static lasttok;						/* used for string compaction */
static string0flg;					/* used to detect the Template Label */
static xquotflg;					/* `quoted string` */	
static formatchar;					/* formatted $macros */

/*
 * Simplify allocation by "pre-allocating" 1 MByte of
 * string space and descriptors.
 */
#define TBUFSIZE (2 * 1024 * 1024)
static char tbuf[TBUFSIZE];
static char *stringp = tbuf;
static union td *Tdp = (union td *)&tbuf[TBUFSIZE];

/*
 * NAME:     pass1
 * FUNCTION: Call yyparse for each template in the template file.
 * INPUTS:   none
 * RETURNS:  none
 *
 * pass1 is a continuation of main().
 * It genrerates an in-memory linked list of parsed templates.
 * This can be thought of as a first pass.
 * It then enters a second pass which reads in events from the logfile
 * and prints them out through prevent();
 *
 * For each template in the template file:
 *   Call gettmplt() to read in the template, so that yygetchar() can
 *     read it. The return value should be good since gettmplt() has
 *     already been called once before through gettmpltinit().
 *   Call parsetemplate(), while will call yyparse() for this template.
 *   Fill in the head of the parsed template (linked list Tdp) in the
 *     Tindexes structure for that template. This will be used to match
 *     the event's traceid to the correct template.
 *
 * Call pr_hdr() to print a header and file name.

 * For each event in the log:  (obtained through getevent)
 *   Get the head of the parsed template from the trace id of the event.
 *     If not there, use HKWD_TRACE_NULL. trcrpt expects this template to
 *     be there.
 *   Then call prevent with the head of the linked list. prevent knows
 *     how to scan down the list looking for format codes. When it finds
 *     one, it gets the next value from the Eventbuf[], which was filled
 *     in by getevent().
 */

int Funclevel;

pass1()
{
	int rv;
	int errsv;
	int traceid;
	int debugsv;
	int ti;

	vprint("Compiling templates\n");
	errsv = 0;
	debugsv = Debugflg;
	Debugflg = Debugflg > 1 ? Debugflg-1 : 0;
	for(ti = 0; ti < Ntemplates; ti++) {
		if((traceid = Traceids[ti]) == 0)
			continue;
		errsv += Errflg;
		Errflg = 0;
		rv = gettmplt(traceid);		/* sets Lineno to start of template */
		switch(rv) {
		case RV_GOOD:
			break;
		default:
			Debug("pass1: unexpected return code %d from gettmplt()",rv);
			genexit(1);
		}
		gldebug();					/* print raw input to yylex */
		parsetemplate();
		Tindexes[traceid].t_tdp = Tdp0;
		if(checkflag)
			yylist();
		if(jflag)
			prtracelbl(Tdp0);
	}
	Debugflg = debugsv;
	Errflg = errsv;
}


#ifdef TRCRPT

pass2(stop_off)
int	stop_off;			/* file offset to stop processing at */
{
	int errsv;
	int rv;
	union td *tdp;

	signal(SIGSEGV,(void(*)(int)) pass2excp);
	vprint("Starting report\n");
	if(rawflag && !jflag) {
		for(;;) {
			rv = getevent();		/* sets Logtraceid */
			switch(rv) {
			case RV_BADFORMAT:
				continue;
			case RV_EOF:
				return;
			}
			Debug("Logtraceid=%03x\n",Logtraceid);
			Logtraceid_real = Logtraceid;
			prevent(0);
			if(Hookword == 0 || (Endtime && ftell(Logfp) >= stop_off))
				return;					/* treat as EOF */
		}
	}
	pr_hdr();
	errsv = 0;
	for(;;) {
		errsv += Errflg;
		Errflg = 0;
		rv = getevent();		/* sets Logtraceid */
		switch(rv) {
		case RV_BADFORMAT:
			continue;
		case RV_EOF:
			Errflg = errsv;
			return;
		}
		Debug("Logtraceid=%03x\n",Logtraceid);
		Logtraceid_real = Logtraceid;
		if(Logtraceid == 0)
			Logtraceid = HKWDTOHKID(HKWD_TRACE_NULL);
		if((tdp = Tindexes[Logtraceid].t_tdp) == NULL) {
			Logtraceid = HKWDTOHKID(HKWD_TRACE_UNDEFINED);
			if((tdp = Tindexes[Logtraceid].t_tdp) == 0) {
/*
				settmplt(Default_tmplt);
				parsetemplate();
				Tindexes[traceid].t_tdp = Tdp0;
				tdp = Tdp0;
*/
			}
		}
		prevent(tdp);
		if(Hookword == 0 || (Endtime && ftell(Logfp) > stop_off))
			return;					/* treat as EOF */
	}
}

#endif


/*
 * NAME:     parsetemplate
 * FUNCTION: reinitialize the lexical analyzer (yylex) by setting
 *           all its state flags, etc to 0. Then call yyparse.
 * INPUTS:   none  input to yylex is from yygetchar()
 * RETURNS:  none  yyparse will set the global Errflg on error.
 */
static parsetemplate()
{

	lasttok = 0;
	string0flg = 0;
	xquotflg = 0;
	symclear();		/* clear out previous symbols */
	Tdp0 = 0;
	yyparse();
}

/*
 * NAME:     yylist
 * FUNCTION: print out the template in "token" form. -L option
 * INPUTS:   none   The input is the global head of the current parsed
 *                  list 'Tdp0'
 * RETURNS:  none
 */
static yylist()
{

	if(!(Listflg || Errflg))
		return;
	if(Errflg) {
		gllist(1);
		List("\n");		/* separate by a blank line */
		return;
	}
	if(Listflg > 1) {
		gllist(0);		/* list out the raw template */
		List("\n");		/* separate by a blank line */
	}
	prtree(Tdp0);		/* list out the parsed template */
	List("\n");			/* separate by a blank line */
}

/*
 * NAME:     yylex
 * FUNCTION: lexical analyzer for yyparse
 * INPUT:    none   Input is through call to yygetchar() (GETC() macro)
 * RETURNS:  token (return) and yylval (global) for yyparse
 */

#define UNGETC(c) yyungetchar(c)
#define GETC()    yygetchar()

yylex()
{
	register c;
	register rv;
	register char *cp;
	int cc;
	char *buflim;
	char buf[5120];

	buflim = &buf[sizeof(buf)-2];
loop:
	for(;;) {
		c = GETC();
		switch(c) {
		case '\n':
			Lineno++;
			lasttok = c;
			continue;
		case EOF:
			rv = EOF;
			goto lexexit;
		case '\t':
			Debug("lex: unexpected char %x",c);
			c = ' ';
		case ' ':
			lasttok = c;
			if(xquotflg)
				break;
			continue;
		}
		if(xquotflg) {
			UNGETC(c);
			rv = lexdbl();
			goto lexexit;
		}
		if(rv = specialchk(c)) {
			switch(rv) {
			case QUOTE:						/* form a single string */
			case QUOTE2:
				goto loop;
			case IXQUOTE:					/* don't set lasttok to XQUOTE */
				Debug("TOKEN3: XQUOTE\n");	/* so last DELIM is not */
				return(rv);					/* overwritten */
			default:
				goto lexexit;
			}
		}
		UNGETC(c);
		cp = buf;
		formatchar = 0;
		for(;;) {
			c = GETC();
			switch(c) {
			case '[':
			case ']':
			case ')':
			case '(':
			case '$':
				if(cp == buf) {	/* These are delimiters if they */
					*cp++ = c;	/* don't occur at that start of a word */
					continue;
				}
				UNGETC(c);
				break;
			case '.':
				cc = nextc();
				if(cp == buf || !ISDELIM(cc)) {	/* '.' is delimiter if not */
					*cp++ = c;					/* at start/end of word */
					continue;
				}
				UNGETC(c);
				break;
			case '\\':
			case ',':
			case '\n':
			case '\t':
			case QUOTE:
			case QUOTE2:
			case XQUOTE:
			case '{':
			case '}':
			case '\0':
			case ' ':
				UNGETC(c);					/* always a delimiter */
				break;
			case '=':						/* special case for L=XXXX */
				*cp++ = c;
				if(buf[0] == 'L' && buf[1] == '=' && cp == &buf[2])
					continue;
				break;
			case '%':
				if(buf[0] == '$') {				/* $rr%X4 */
					formatchar = '%';
					break;
				}
			default:
				*cp = c;
				cp++;
				if(cp >= buflim) {
					Debug("String at line %d > %d. Truncated\n",
						Lineno,sizeof(buf));
					break;
				}
				continue;
			}
			*cp = '\0';							/* NULL terminate */
			if(buf[0] == '$') {
				rv = regchk(buf);
				goto lexexit;
			}
			rv = reschk(buf);
			if(rv == ISTRING)
				tdstring(buf);
			goto lexexit;
		}
	}
lexexit:
	Debug("dblq=%d ",xquotflg);
	if(rv == ISTRING && !string0flg++) {	/* first STRING is STRING0 */
		yylval.ystringd->u_type = ISTRING0;
		rv = ISTRING0;						/* this helps the parser */
	}
	if(xquotflg && rv == ISTRING && lasttok == ISTRING) {
		cp = yylval.ystringd->s_string;
		Debug("TOKEN2: %s '%s'\n",tokstr(rv),cp);
		stringp = cp-1;
		string_install(cp);
		Tdp++;					/* free the td struct */
		goto loop;
	}
	if(rv == ISTRING)
		Debug("TOKEN0: %s %s\n",tokstr(rv),yylval.ystringd->s_string);
	else
		Debug("TOKEN0: %s\n",tokstr(rv));
	lasttok = rv;
	return(rv);
}

/*
 * NAME:       tdstring
 * FUNCTION:   Add the string to the parse tree
 * INPUTS:     str  string to add
 * RETURNS:    none
 *
 * Fill in the global yylval.stringd with the pointer to the
 * symbol table entry this string. Copy the string contents into the
 * stringp buffer.
 */
static tdstring(str)
char *str;
{
	struct stringd *sp;

	sp = (struct stringd *)gettd();
	sp->u_type = (str == matchstr) ? IMATCH : ISTRING;
	sp->s_string = stringp;
	string_install(str);
	yylval.ystringd = sp;
}

/*
 * Allocate a symbol table entry.
 * Initialize to all zeroes.
 */
union td *gettd()
{

	if((int)stringp >= (int)Tdp)
		cat_lfatal(CAT_TPT_STACKOVF,
"The total size of the templates has exceeded the\n\
internal memory capacity of trcrpt.\n");
	Tdp--;
	memset(Tdp,0,sizeof(*Tdp));
	return(Tdp);
}


#ifdef TRCRPT

freetd(tdp)
union td *tdp;
{
}

#endif


/*
 * NAME:       reschk
 * FUNCTION:   Look up the string in the list of reserved words
 *             and create a filled-in symbol table structure if so.
 * INPUTS:     str  string to lookup
 * RETURNS:    token according to the type of the symbol. If not a reserved,
 *             it is ISTRING.
 *
 * The types ILEVEL, IFORMAT, and IREG get allocated a additional symbol table
 * entry and reschk() fills it in.
 * The other reserved word is LOOP and it does not need a symbol.
 */
static reschk(str)
char *str;
{
	int n;
	symbol *symp;
	struct formatd *fp;
	struct macdefd *mp;

	if((symp = reslookup(str)) == NULL)
		return(ISTRING);
	switch(symp->s_type) {
	case ILEVEL:
		if(xquotflg)
			return(ISTRING);
		fp = (struct formatd *)gettd();
		fp->f_fld1 = symp->s_fld1;
		fp->u_type = ILEVEL;
		yylval.yformatd = fp;
		return(ILEVEL);
	case IFORMAT:
		fp = (struct formatd *)gettd();
		yylval.yformatd = fp;
		fp->f_fmtcode = symp->s_fmtcode;
		fp->u_type = IFORMAT;
		if(symp->s_fld1) {
			fp->f_fld1 = symp->s_fld1;
			fp->f_fld2 = 0;
		} else {
			n = atoi(Numbuf2);
			if(n && symp->s_xcode < 2) {
				cat_lwarn(CAT_TPT_UNDEFFMT,
"Undefined format code %s in line %d.\n\
The code will be interpreted as a string.\n",symp->s_name);
				return(ISTRING);
			}
			fp->f_fld1 = atoi(Numbuf1);
			fp->f_fld2 = n;
		}
		if(symp->s_fmtcode == 'i' && symp->s_fld1 == RES_DEFAULT)
			fp->f_val = HKWDTOHKID(HKWD_TRACE_DEFAULT);
		Debug("code=%c width=%d.%d\n",
			fp->f_fmtcode,fp->f_fld1,fp->f_fld2);
		return(IFORMAT);
	case IREG:							/* internal registers */
		fp = (struct formatd *)gettd();
		yylval.yformatd = fp;
		fp->u_type = IFORMAT;
		fp->f_fmtcode = symp->s_fmtcode;	/* $ */
		fp->f_val = symp->s_fld1;			/* symtab index */
		fp->f_fld1 = 2;
		fp->f_fld2 = 0;
		Debug("reserved label %s is register %d\n",str,fp->f_val);
		return(formatchar == '%' ? IREGFMT : IREG);
	default:
		if(xquotflg)
			return(ISTRING);
		return(symp->s_type);
	}
}

static regchk(buf)
char *buf;
{
	int rv;
	struct formatd *fp;
	symbol *sp;
	char *name;

	name = buf+1;
	if(gotochk(name)) {			/* template subroutine */
		fp = (struct formatd *)gettd();
		yylval.yformatd = fp;
		fp->u_type = IFORMAT;
		fp->f_fmtcode = 'i';
		fp->f_val  = strtol(name,0,16);
		fp->f_fld1 = RES_DEFAULT;
		fp->f_fld2 = 0;
		Debug("label %s is goto %03x\n",name,fp->f_val);
		return(IFORMAT);
	}
	if((sp = lookup(name)) == NULL) {		/* new register */
		if((rv = reschk(buf)) != ISTRING)	/* note 'buf', not 'name' */
			return(rv);
		sp = install(name);
		string_install(name);
	}
	fp = (struct formatd *)gettd();
	yylval.yformatd = fp;
	fp->u_type = IFORMAT;
	fp->f_fmtcode = sp->s_fmtcode;	/* $ */
	fp->f_val = sp->s_fld1;			/* symtab index */
	fp->f_fld1 = 2;
	fp->f_fld2 = 0;
	Debug("label %s is register %d\n",name,fp->f_val);
	return(formatchar == '%' ? IREGFMT : IREG);
}

/*
 * return the next character in the input for lookahead.
 */
static nextc()
{
	int cc;

	cc = GETC(); /* see if this is a separate character */
	UNGETC(cc);
	return(cc);
}

/*
 * return true if the string is a 3 hex digit number.
 */
static gotochk(s)
char *s;
{
	int i;

	for(i = 0; i < 3; i++)
		if(!isxdigit(s[i]))
			return(0);
	return(s[3] == '\0');
}

/*
 * Read the next character.
 * If it is equal to 'testc' return the token 'tok1'.
 * Otherwise, put back the character and return 'tok2'.
 * This is useful for tokens like <=, ==, etc.
 */
static follow(testc,tok1,tok2)
{

	int c;

	c = GETC();
	if(c == testc) {
		return(tok1);
	} else {
		UNGETC(c);
		return(tok2);
	}
}

/*
 * NAME:     getstring
 * FUNCTION: read in the quoted string and add to symbol table.
 * INPUTS:   none
 * RETURNS:  ISTRING if string is OK
 *           0       string was unterminated
 *
 * Convert the two character \n into newline
 * Convert the two character \t into tab
 * Convert the two character \\ into \
 */
static getstring()
{
	char *cp;
	int c;
	int cc;
	char buf[5120];
	char *buflim;

	buflim = &buf[sizeof(buf)];
	cp = buf;
	for(;;) {
		c = GETC();
		switch(c) {
		case '\n':
			Lineno++;
			break;
		case '\\':
			switch(c = GETC()) {
			case 'n':  c = '\n'; break;
			case 't':  c = '\t'; break;
			case '\\': c = '\\'; break;
			}
			break;
		case XQUOTE:
			if(string0flg)
				break;
		case QUOTE:
		case QUOTE2:
			goto strexit;
		case '\0':
			Debug("Unterminated string at line %d\n",Lineno);
			UNGETC(c);
			goto strexit;
		case EOF:
			Debug("Unexpected EOF in string at line %d\n",Lineno);
			goto strexit;
		}
		if(cp >= buflim) {
			Debug("String at line %d > %d. Truncated\n",
				Lineno,sizeof(buf));
			goto strexit;
		}
		*cp = c;
		cp++;
	}
strexit:
	*cp = '\0';
	tdstring(buf);
	if(ISDELIM(lasttok))
		yylval.ystringd->s_flags |= SFLG_LDELIM;
	cc = nextc();
	if(ISDELIM(cc))
		yylval.ystringd->s_flags |= SFLG_RDELIM;
	yylval.ystringd->s_flags |= SFLG_QUOTED;
	return(ISTRING);
}


#ifdef TRCRPT

char *stracpy(str)
char *str;
{
	char *cp;

	cp = jalloc(strlen(str) + 1);
	strcpy(cp,str);
	return(cp);
}

#endif


static specialchk(c)
{
	int rv;

	switch(c) {
	case '\0': return(IEOD);
	case '(':  return('(');
	case ')':  return(')');
	case ',':  return(',');
	case '{':  return(follow('{',ILBRACEM,ILBRACE));
	case '}':  return(follow('}',IRBRACEM,IRBRACE));
	case XQUOTE:
		if(string0flg) {
			xquotflg = 1;
			return(IXQUOTE);
		}
	case QUOTE:
	case QUOTE2:
		getstring();
		return(ISTRING);
	case '=':				/* allow count=X4 */
	case '&':
	case '+':
	case '-':
	case '*':
	case '/':
		if(nextc() == ' ')
			return(c);
		break;
	case '\\':
		switch(c = GETC()) {
		case '*': tdstring(matchstr); return(IMATCH);
		case 'n': tdstring("\n");     return(ISTRING);
		case 't': tdstring("\t");     return(ISTRING);
		}
		break;
	}
	return(0);
}

static lexdbl()
{
	int c;
	int rv;
	char buf[5120];
	char *cp;

	Debug("lexdbl: lasttok=%s\n",tokstr(lasttok));
loop:
	cp = buf;
	c = GETC();
	switch(c) {
	case '\0':
		rv = IEOD;
		goto ret;
	case XQUOTE:				/* terminate quoted string */
		xquotflg = 0;
		if(lasttok == ISTRING) {
			c = nextc();
			if(ISDELIM(c))
				yylval.ystringd->s_flags |= SFLG_RDELIM;
		}
		rv = IXQUOTE;
		goto ret;
	case '\\':
		switch(c = GETC()) {
		case 'n': c = '\n'; break;
		case 't': c = '\t'; break;
		}
	}
	if(!isalpha(c) && c != '$') {
		while(!isalpha(c) && c != '$') {
			switch(c) {
			case '\0':
			case XQUOTE:
				break;
			default:
				*cp++ = c;
				c = GETC();
				continue;
			}
			break;
		}
		while(ISDELIM(c)) {
			*cp++ = c;
			c = GETC();
		}
		UNGETC(c);
		*cp = '\0';
		rv = ISTRING;
		if(lasttok == ISTRING) {
			stringp--;
			string_install(buf);
			goto loop;
		}
		goto ret;
	}
	formatchar = 0;
	if(c == '$') {
		*cp++ = c;
		c = GETC();
		while(isalnum(c) || c == '%') {
			if(c == '%') {
				formatchar = '%';		/* communicate to regchk() */
				c = GETC();				/* skip over the '%' */
				break;
			}
			*cp++ = c;
			c = GETC();
		}
		*cp = '\0';
		UNGETC(c);
		rv = regchk(buf);
		goto ret;
	}
	while(isalnum(c) || c == '%' || c == '.') {
		*cp++ = c;
		c = GETC();
	}
	*cp = '\0';
	UNGETC(c);
	rv = reschk(buf);
ret:
	if(rv == ISTRING) {
		if(lasttok == ISTRING) {
			stringp--;
			string_install(buf);
			goto loop;
		}
		tdstring(buf);
		if(ISDELIM(lasttok))
			yylval.ystringd->s_flags |= SFLG_LDELIM;
		yylval.ystringd->s_flags |= SFLG_QUOTED;
	}
	return(rv);
}


#ifdef TRCRPT

strdump(limflg)
{
	int i;
	char *cp;
	int c;

	if(!Debugflg)
		return;
	cp = (char *)tbuf;
	Debug("strdump:\n");
	for(i = 0; i < 64; i++) {
		c = *cp++;
		Debug("%c",c >= ' ' ? c : '-');
		if(cp == stringp) {
			Debug("|");
			if(!limflg)
				break;
		}
	}
	Debug("\n");
}

#endif



#ifdef TRCRPT

static strcpy(s1,s2)
register char *s1,*s2;
{

	for(;;)
		if((*s1++ = *s2++) == 0)
			break;
}

#endif


static string_install(name) 
char *name;
{
	int len;

	len = strlen(name);
	strcpy(stringp,(name));
	stringp += len+1;
	stringp[-1] = '\0';
	/*if(Debugflg)
		strdump(0);*/
}


#ifdef TRCRPT

lexprstats()
{
	int v_str;
	int v_sym;
	int v_tot;
	int v_free;

	v_str = stringp-(char *)tbuf;
	v_sym = (char *)&tbuf[TBUFSIZE] - (char *)Tdp;
	v_tot = sizeof(tbuf);
	v_free = v_tot - v_sym - v_str;
	vprint("\
%7d bytes string space\n\
%7d bytes symbol space\n\
%7d bytes free   space\n",
		v_str,v_sym,v_free);
}

#endif



#ifdef TRCRPT

numchk(str,defaultbase)
char *str;
{
	int c;
	char *cp;
	int xflg;

	xflg = 0;
	cp = str;
	if(!isxdigit(*cp))
		return(0);
	if(*cp == '0') {
		if(((c = cp[1]) == 'x' || c == 'X')) {
			cp += 2;
			defaultbase = 16;
			xflg++;
		} else {
			defaultbase = 8;
		}
	}
	while(c = *cp++) {
		if(!isxdigit(c)) {
			if(!xflg)
				return(0);
			return(-1);
		}
		if(defaultbase == 0)
			defaultbase = 16;
		else if(defaultbase != 16 && !isdigit(c))
			return(0);
	}
	return(defaultbase);
}

#endif


funcdef(n,name,arglist,desc)
struct stringd *name;
struct stringd *arglist;
union td *desc;
{
	struct stringd *sp;
	struct builtins *blp;
	int count;

	if(desc == 0) {
		Funclevel++;
		symclearl(Funclevel);
		Debug("funcdef %s\n",name->s_string);
	} else {
		sp = arglist;
		count = 0;
		while(sp) {
			count++;
			install(sp->s_string);
			sp = (struct stringd *)sp->u_next;
		}
		blp = finstall(name->s_string,0,desc);
		blp->bl_regcount = count;
		Funclevel--;
	}
}

