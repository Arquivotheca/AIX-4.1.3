static char sccsid[] = "@(#)52	1.6  src/bos/usr/bin/mh/sbr/fmtcompile.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:13";
/* 
 * COMPONENT_NAME: CMDMH fmtcompile.c
 * 
 * FUNCTIONS: ADDC, CERROR, LS, LV, MSGSTR, NEW, NEWCOMP, PUTC, 
 *            PUTCOMP, PUTLIT, compile, compile_error, do_expr, 
 *            do_func, do_if, do_name, do_spec, fmt_compile, lookup 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "fmtcompile.c	7.1 87/10/13 17:05:26"; */

/* fmtcompile.c - "compile" format strings for fmtscan */

#include "mh.h"
#include "addrsbr.h"
#include "formatsbr.h"
#include "tws.h"
#include "fmtcompile.h"
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

static struct format *formatvec;	/* array to hold formats */
static struct format *next_fp;		/* next free format slot */
static struct format *fp;		/* current format slot */
static struct comp *cm;			/* most recent comp ref */
static struct ftable *ftbl;		/* most recent func ref */
static int ncomp;
static int infunction;			/* function nesting cnt */

extern char *getusr();
extern struct mailname fmt_mnull;

struct ftable {
    char	*name;		/* function name */
    char	type;		/* argument type */
#define		TF_COMP 0  	    /* component expected */
#define		TF_NUM  1  	    /* number expected */
#define		TF_STR  2  	    /* string expected */
#define		TF_EXPR 3  	    /* component or func. expected */
#define		TF_NONE 4  	    /* no argument */
#define		TF_MYBOX 5 	    /* special - get current user's mbox */
#define		TF_NOW  6  	    /* special - get current unix time */
#define		TF_EXPR_SV 7	    /* like expr but save current str reg */
#define		TF_NOP  8	    /* like expr but no result */
    char	f_type;		/* fmt type */
    char	extra;		/* arg. type dependent extra info */
    char	flags;
#define		TFL_PUTS  1	    /* implicit putstr if top level */
#define		TFL_PUTN  2	    /* implicit putnum if top level */
};

static struct ftable functable[] = {
    "nonzero",	TF_EXPR,	FT_V_NE,	FT_IF_V_NE,	0,
    "zero",	TF_EXPR,	FT_V_EQ,	FT_IF_V_EQ,	0,
    "eq",	TF_NUM,		FT_V_EQ,	FT_IF_V_EQ,	0,
    "ne",	TF_NUM,		FT_V_NE,	FT_IF_V_NE,	0,
    "gt",	TF_NUM,		FT_V_GT,	FT_IF_V_GT,	0,
    "null",	TF_EXPR,	FT_S_NULL,	FT_IF_S_NULL,	0,
    "nonnull",	TF_EXPR,	FT_S_NONNULL,	FT_IF_S,	0,
    "match",	TF_STR,		FT_V_MATCH,	FT_IF_MATCH,	0,
    "amatch",	TF_STR,		FT_V_AMATCH,	FT_IF_AMATCH,	0,

    "putstr",	TF_EXPR,	FT_STR,		0,		0,
    "putstrf",	TF_EXPR,	FT_STRF,	0,		0,
    "putnum",	TF_EXPR,	FT_NUM,		0,		0,
    "putnumf",	TF_EXPR,	FT_NUMF,	0,		0,
    "putaddr",	TF_STR,		FT_PUTADDR,	0,		0,
    "void",	TF_NOP,		0,		0,		0,

    "comp",	TF_COMP,	FT_LS_COMP,	0,		TFL_PUTS,
    "lit",	TF_STR,		FT_LS_LIT,	0,		TFL_PUTS,
    "compval",	TF_COMP,	FT_LV_COMP,	0,		TFL_PUTN,
    "compflag",	TF_COMP,	FT_LV_COMPFLAG,	0,		TFL_PUTN,
    "num",	TF_NUM,		FT_LV_LIT,	0,		TFL_PUTN,
    "msg",	TF_NONE,	FT_LV_DAT,	0,		TFL_PUTN,
    "cur",	TF_NONE,	FT_LV_DAT,	1,		TFL_PUTN,
    "size",	TF_NONE,	FT_LV_DAT,	2,		TFL_PUTN,
    "width",	TF_NONE,	FT_LV_DAT,	3,		TFL_PUTN,
    "dat",	TF_NUM,		FT_LV_DAT,	0,		TFL_PUTN,
    "strlen",	TF_NONE,	FT_LV_STRLEN,	0,		TFL_PUTN,
    "me",	TF_MYBOX,	FT_LS_LIT,	0,		TFL_PUTS,
    "plus",	TF_NUM,		FT_LV_PLUS_L,	0,		TFL_PUTN,
    "minus",	TF_NUM,		FT_LV_MINUS_L,	0,		TFL_PUTN,
    "charleft",	TF_NONE,	FT_LV_CHAR_LEFT, 0,		TFL_PUTN,
    "timenow",	TF_NOW,		FT_LV_LIT,	0,		TFL_PUTN,

    "month",	TF_COMP,	FT_LS_MONTH,	FT_PARSEDATE,	TFL_PUTS,
    "lmonth",	TF_COMP,	FT_LS_LMONTH,	FT_PARSEDATE,	TFL_PUTS,
    "tzone",	TF_COMP,	FT_LS_ZONE,	FT_PARSEDATE,	TFL_PUTS,
    "day",	TF_COMP,	FT_LS_DAY,	FT_PARSEDATE,	TFL_PUTS,
    "weekday",	TF_COMP,	FT_LS_WEEKDAY,	FT_PARSEDATE,	TFL_PUTS,
    "tws",	TF_COMP,	FT_LS_822DATE,	FT_PARSEDATE,	TFL_PUTS,
    "sec",	TF_COMP,	FT_LV_SEC,	FT_PARSEDATE,	TFL_PUTN,
    "min",	TF_COMP,	FT_LV_MIN,	FT_PARSEDATE,	TFL_PUTN,
    "hour",	TF_COMP,	FT_LV_HOUR,	FT_PARSEDATE,	TFL_PUTN,
    "mday",	TF_COMP,	FT_LV_MDAY,	FT_PARSEDATE,	TFL_PUTN,
    "mon",	TF_COMP,	FT_LV_MON,	FT_PARSEDATE,	TFL_PUTN,
    "year",	TF_COMP,	FT_LV_YEAR,	FT_PARSEDATE,	TFL_PUTN,
    "yday",	TF_COMP,	FT_LV_YDAY,	FT_PARSEDATE,	TFL_PUTN,
    "wday",	TF_COMP,	FT_LV_WDAY,	FT_PARSEDATE,	TFL_PUTN,
    "zone",	TF_COMP,	FT_LV_ZONE,	FT_PARSEDATE,	TFL_PUTN,
    "clock",	TF_COMP,	FT_LV_CLOCK,	FT_PARSEDATE,	TFL_PUTN,
    "rclock",	TF_COMP,	FT_LV_RCLOCK,	FT_PARSEDATE,	TFL_PUTN,
    "sday",	TF_COMP,	FT_LV_DAYF,	FT_PARSEDATE,	TFL_PUTN,
    "dst",	TF_COMP,	FT_LV_TZONEF,	FT_PARSEDATE,	TFL_PUTN,
    "pretty",	TF_COMP,	FT_LS_PRETTY,	FT_PARSEDATE,	TFL_PUTS,
    "nodate",	TF_COMP,	FT_LV_COMPFLAG,	FT_PARSEDATE,	TFL_PUTN,

    "pers",	TF_COMP,	FT_LS_PERS,	FT_PARSEADDR,	TFL_PUTS,
    "mbox",	TF_COMP,	FT_LS_MBOX,	FT_PARSEADDR,	TFL_PUTS,
    "host",	TF_COMP,	FT_LS_HOST,	FT_PARSEADDR,	TFL_PUTS,
    "path",	TF_COMP,	FT_LS_PATH,	FT_PARSEADDR,	TFL_PUTS,
    "gname",	TF_COMP,	FT_LS_GNAME,	FT_PARSEADDR,	TFL_PUTS,
    "note",	TF_COMP,	FT_LS_NOTE,	FT_PARSEADDR,	TFL_PUTS,
    "proper",	TF_COMP,	FT_LS_822ADDR,	FT_PARSEADDR,	TFL_PUTS,
    "type",	TF_COMP,	FT_LV_HOSTTYPE,	FT_PARSEADDR,	TFL_PUTN,
    "ingrp",	TF_COMP,	FT_LV_INGRPF,	FT_PARSEADDR,	TFL_PUTN,
    "nohost",	TF_COMP,	FT_LV_NOHOSTF,	FT_PARSEADDR,	TFL_PUTN,
    "formataddr", TF_EXPR_SV,	FT_FORMATADDR,	FT_FORMATADDR,	0,
    "friendly",	TF_COMP,	FT_LS_FRIENDLY,	FT_PARSEADDR,	TFL_PUTS,

    "mymbox",	TF_COMP,	FT_LV_COMPFLAG,	FT_MYMBOX,	TFL_PUTN,

    (char *)0,	0,		0,		0,		0    
};

static struct ftable *lookup(name)
    register char *name;
{
    register struct ftable *t = functable;
    register char *nm;
    register char c = *name;

    while (nm = t->name) {
	if (*nm == c && strcmp (nm, name) == 0)
	    return (ftbl = t);

	t++;
    }
    return (struct ftable *)0;
}


#define NEWCOMP(cm,name)\
	cm = ((struct comp *)calloc(1, sizeof (struct comp)));\
	cm->c_name = name; ncomp++;\
 	i = CHASH(name); cm->c_next = wantcomp[i]; wantcomp[i] = cm;

#define NEWFMT (next_fp++)
#define NEW(type,fill,wid)\
	fp=NEWFMT; fp->f_type=(type); fp->f_fill=(fill); fp->f_width=(wid);

#define ADDC(name)\
	FINDCOMP( cm, name );\
	if ( ! cm ) {\
	    NEWCOMP(cm,name);\
	}\
	fp->f_comp = cm;

#define LV( type, value )	NEW(type,0,0); fp->f_value = (value);
#define LS( type, str )		NEW(type,0,0); fp->f_text = (str);
#define PUTCOMP( comp )		NEW(FT_COMP,0,0); ADDC(comp);
#define PUTLIT( str )		NEW(FT_LIT,0,0); fp->f_text = (str);
#define PUTC( c )		NEW(FT_CHAR,0,0); fp->f_char = (c);

static char *compile();
static char *do_spec();
static char *do_name();
static char *do_func();
static char *do_expr();
static char *do_if();

static char *format_string;
static char *usr_fstring;	/* for CERROR */

#define CERROR(str) compile_error (str, cp)

static compile_error (str, cp)
    char *str;
    char *cp;
{
    int errpos = cp - format_string;
    int errctx = errpos > 20 ? 20 : errpos;
    int i;
    char hlds[NL_TEXTMAX];

    usr_fstring[errpos] = '\0';
    for (i = errpos-errctx; i < errpos; i++)
	if (usr_fstring[i] < 32)
	    usr_fstring[i] = '_';
    strcpy(hlds, &usr_fstring[errpos-errctx]);
    advise(NULLCP, MSGSTR(FCERR, "\"%s\": format compile error - %s"), hlds, str); /*MSG*/
    adios (NULLCP, "%*s", errctx+1, "^");
}

/*
 * Compile format string "fstring" into format list "fmt".
 * Return the number of header components found in the format
 * string.
 */
fmt_compile( fstring, fmt )
    register char *fstring;
    struct format **fmt;
{
    register char *cp;
    int i;

    if (format_string)
	(void) free (format_string);
    format_string = getcpy (fstring);
    usr_fstring = fstring;

    /* init the component hash table. */
    for (i = 0; i < sizeof(wantcomp)/sizeof(wantcomp[0]); i++)
	wantcomp[i] = 0;

    bzero (&fmt_mnull, sizeof fmt_mnull);

    /* it takes at least 4 char to generate one format so we
     * allocate a worst-case format array using 1/4 the length
     * of the format string.  We actually need twice this much
     * to handle both pre-processing (e.g., address parsing) and
     * normal processing.
     */
    i = strlen(fstring)/2 + 1;
    next_fp = formatvec = (struct format *)calloc ((unsigned) i,
						   sizeof(struct format));
    if (next_fp == NULL)
	adios (NULLCP, MSGSTR(NOFSTOR, "unable to allocate format storage")); /*MSG*/

    ncomp = 0;
    infunction = 0;

    cp = compile(format_string);
    if (*cp) {
	CERROR(MSGSTR(EX1, "extra '%>' or '%|'")); /*MSG*/
    }
    NEW(FT_DONE,0,0);
    *fmt = formatvec;

    return (ncomp);
}

static char *compile (sp)
    register char *sp;
{
    register char *cp = sp;
    register int  c;

    for (;;) {
	sp = cp;
	while ((c = *cp) && c != '%')
	    cp++;
	*cp = 0;
	switch (cp-sp) {
	case 0:
	    break;
	case 1:
	    PUTC(*sp);
	    break;
	default:
	    PUTLIT(sp);
	    break;
	}
	if (c == 0)
	    return (cp);

	switch (c = *++cp) {
	case '%':
	    break;

	case '|':
	case '>':
	    return (cp);

	case '<':
	    cp = do_if(++cp);
	    break;

	default:
	    cp = do_spec(cp);
	    break;
	}
    }
}


static char *do_spec(sp)
    register char *sp;
{
    register char *cp = sp;
    register int c;
    register int ljust = 0;
    register int wid = 0;
    register char fill = ' ';

    c = *cp++;
    if (c == '-') {
	ljust++;		/* should do something with this */
	c = *cp++;
    }
    if (c == '0') {
	fill = c;
	c = *cp++;
    }
    while (isdigit(c)) {
	wid = wid*10 + (c - '0');
	c = *cp++;
    }
    if (c == '{') {
	cp = do_name(cp, 0);
	if (! infunction)
	    fp->f_type = wid? FT_COMPF : FT_COMP;
    }
    else if (c == '(') {
	cp = do_func(cp);
	if (! infunction) {
	    if (ftbl->flags & TFL_PUTS) {
		LV( wid? FT_STRF : FT_STR, ftbl->extra);
	    }
	    else if (ftbl->flags & TFL_PUTN) {
		LV( wid? FT_NUMF : FT_NUM, ftbl->extra);
	    }
	}
    }
    else {
	CERROR(MSGSTR(NEEDNM, "component or function name expected")); /*MSG*/
    }
    fp->f_width = wid;
    fp->f_fill = fill;

    return (cp);
}

static char *do_name(sp, preprocess)
    char *sp;
    int  preprocess;
{
    register char *cp = sp;
    register int c;
    register int i;
    static int primed = 0;

    while (isalnum(c = *cp++) || c == '-') 
	;
    if (c != '}') {
	CERROR(MSGSTR(NEEDBRC, "'}' expected")); /*MSG*/
    }
    cp[-1] = '\0';
    PUTCOMP(sp);
    switch (preprocess) {

    case FT_PARSEDATE:
	if (cm->c_type & CT_ADDR) {
	    CERROR(MSGSTR(DTADDR, "component used as both date and address")); /*MSG*/
	}
	if (! (cm->c_type & CT_DATE)) {
	    cm->c_tws = (struct tws *)
				calloc((unsigned) 1, sizeof *cm -> c_tws);
	    fp->f_type = preprocess;
	    PUTCOMP(sp);
	    cm->c_type |= CT_DATE;
	}
	break;

    case FT_MYMBOX:
	if (!primed) {
	    (void) ismymbox ((struct mailname *) 0);
	    primed++;
	}
	cm->c_type |= CT_MYMBOX;
	/* fall through */
    case FT_PARSEADDR:
	if (cm->c_type & CT_DATE) {
	    CERROR(MSGSTR(DTADDR, "component used as both date and address")); /*MSG*/
	}
	if (! (cm->c_type & CT_ADDRPARSE)) {
	    cm->c_mn = &fmt_mnull;
	    fp->f_type = preprocess;
	    PUTCOMP(sp);
	    cm->c_type |= (CT_ADDR | CT_ADDRPARSE);
	}
	break;

    case FT_FORMATADDR:
	if (cm->c_type & CT_DATE) {
	    CERROR(MSGSTR(DTADDR, "component used as both date and address")); /*MSG*/
	}
	cm->c_type |= CT_ADDR;
	break;
    }
    return (cp);
}

static char *do_func(sp)
    char *sp;
{
    register char *cp = sp;
    register int c;
    register struct ftable *t;
    register int n;
    char *orname;

    infunction++;

    while (isalnum(c = *cp++)) 
	;
    if (c != '(' && c != '{' && c != ' ' && c != ')') {
	CERROR(MSGSTR(EX2, "'(', '{', ' ' or ')' expected")); /*MSG*/
    }
    cp[-1] = '\0';
    if ((t = lookup (sp)) == 0) {
	CERROR(MSGSTR(UNKFUNC, "unknown function")); /*MSG*/
    }
    if (isspace(c))
	c = *cp++;

    switch (t->type) {

    case TF_COMP:
	if (c != '{') {
	    CERROR(MSGSTR(EX3, "component name expected")); /*MSG*/
	}
	cp = do_name(cp, t->extra);
	fp->f_type = t->f_type;
	c = *cp++;
	break;

    case TF_NUM:
	n = 0;
	while (isdigit(c)) {
	    n = n*10 + (c - '0');
	    c = *cp++;
	}
	LV(t->f_type,n);
	break;

    case TF_STR:
	sp = cp - 1;
	while (c && c != ')')
	    c = *cp++;
	cp[-1] = '\0';
	LS(t->f_type,sp);
	break;

    case TF_NONE:
	LV(t->f_type,t->extra);
	break;

    case TF_MYBOX:
	LS(t->f_type, (orname=m_find("orname")) ? orname : getusr());
	break;

    case TF_NOW:
	LV(t->f_type, time((long *) 0));
	break;

    case TF_EXPR_SV:
	LV(FT_SAVESTR, 0);
	/* fall through */
    case TF_EXPR:
	*--cp = c;
	cp = do_expr(cp, t->extra);
	LV(t->f_type, t->extra);
	c = *cp++;
	ftbl = t;
	break;

    case TF_NOP:
	*--cp = c;
	cp = do_expr(cp, t->extra);
	c = *cp++;
	ftbl = t;
	break;
    }
    if (c != ')') {
	CERROR(MSGSTR(EX4, "')' expected")); /*MSG*/
    }
    --infunction;
    return (cp);
}

static char *do_expr (sp, preprocess)
    char *sp;
{
    register char *cp = sp;
    register int  c;

    if ((c = *cp++) == '{') {
	cp = do_name (cp, preprocess);
	fp->f_type = FT_LS_COMP;
    } else if (c == '(') {
	cp = do_func (cp);
    } else if (c == ')') {
	return (--cp);
    } else if (c == '%' && *cp == '<') {
	cp = do_if (cp+1);
    } else {
	CERROR (MSGSTR(EX5, "'(', '{', '%<' or ')' expected")); /*MSG*/
    }
    return (cp);
}

static char *do_if(sp)
    register char *sp;
{
    register char *cp = sp;
    register struct format *fexpr, *fif, *felse;
    register int c;

    if ((c = *cp++) == '{') {
	cp = do_name(cp, 0);
	fp->f_type = FT_LS_COMP;
	LV (FT_IF_S, 0);
    }
    else if (c == '(') {
	cp = do_func(cp);
	/* see if we can merge the load and the "if" */
	if (ftbl->f_type >= IF_FUNCS)
	    fp->f_type = ftbl->extra;
	else {
	    LV (FT_IF_V_NE, 0);
	}
    }
    else  {
	CERROR(MSGSTR(EX6, "'(' or '{' expected")); /*MSG*/
    }
    fexpr = fp;
    cp = compile (cp);
    if ((c = *cp++) == '|') {
	LV(FT_GOTO, 0);
	fif = fp;
	felse = next_fp;
	cp = compile(cp);
	fif->f_skip = next_fp - fif;
	c = *cp++;
    }
    else
	felse = next_fp;

    if (c != '>') {
	CERROR(MSGSTR(EX7, "'>' expected.")); /*MSG*/
    }
    fexpr->f_skip = felse - fexpr;

    return (cp);
}
