static char sccsid[] = "@(#)11  1.16  src/bos/usr/bin/errlg/errupdate/lex.c, cmderrlg, bos411, 9428A410j 3/29/94 19:27:28";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: lexinit, yylex, numchk, tokstr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FUNCTION:       yylex()
 *     Modified generic lexical analyzer, called by lex.
 *     Main tokens are:
 * reserved words (Error_Description, Detail_data, etc)
 * INUMBER
 * STRING
 * IEQUAL  '='
 * IMINUS  '-'
 * IPLUS   '+'
 *
 *     There are 2 special mechanisms between yylex and yypase.
 *     1. 'instanza' is set by yyparse when it has detected the start of
 *        a stanza. This lets lex treat as a label (ISTRING) the
 *          + LABEL
 *        field of the start of the stanza, so that the label can use a
 *        reserved word here.
 *
 *     2. 'detail_flag' is set by yyparse when it knows that it is in
 *        a detail_data stanza. This lets lex know that the input:
 *        'DEC' is interpreted as 'DECIMAL' and not the hex number 0xdec.
 * 
 *  Note: No ILS characters are processed here.  The 'nlc' nomenclature was
 *	  implemented erroneously.
 */

#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <errupdate.h>
#include <parse.h>

extern char *strlcpy();

int detailflg;
int Lineno0;
int instanza;
int lasttok;
char ybuf[256];

static eofflg;
static nlflg;
static badc;

lexinit()
{

	eofflg   = 0;
	nlflg    = 1;
	Lineno   = 0;
	Lineno0  = 1;
	instanza = 0;
	lasttok  = '\n';
}

yylex()
{
	register nlc;	/* NL character */
	register rv;
	unsigned long un;
	int base;		/* base (10,16) of a possible number  */
	int count;
	symbol *symp;	/* symbol pointer returned by reslookup */
	int bolflg;		/* true if we are the beginning of the line */

	bolflg = nlflg;
	nlflg = 0;
	for(;;) {
		nlc = nlc_getc();
		switch(nlc) {
		case '*':			/* ignore comments */
comment:
			while((nlc = nlc_getc()) != '\n')
				if(nlc == EOF)
					mainreturn(EXCP_UNEXPEOF);
		case '\n':			/* end of line */
			Lineno++;
			nlflg++;
			rv = '\n';
			goto lexexit;
		case EOF:
			rv = eofflg++ ? 0 : '\n';
			goto lexexit;
		case ' ':		/* skip whitespace */
		case '\t':
			continue;
		case '#':		/* comment if at beginning of line */
			if(bolflg)
				goto comment;
			break;
		}
		switch(nlc) {		/* special meaning if at beginning of line */
		case '=':
		case '+':
		case '-':
			if(lasttok != '\n') {
				rv = nlc;
				goto lexexit;
			}
			Lineno0 = Lineno;
			switch(nlc) {
			case '=': rv = IEQUAL ; goto lexexit;
			case '+': rv = IPLUS  ; goto lexexit;
			case '-': rv = IMINUS ; goto lexexit;
			}
		}
		switch(nlc) {			/* special delimiters */
		case ',':
		case ':':
			rv = nlc;
			goto lexexit;
		case '"':
			if(lstring(ybuf,sizeof(ybuf)) == 0)
				continue;
			yylval.ysym = getsym();
			yylval.ysym->s_string = strlcpy(ybuf);
			rv = ISTRING;
			goto lexexit;
		}
		nlc_ungetc(nlc);
		nlc_putinit(ybuf);
		count = 0;
		for(;;) {					/* get a string of the form: aaa_aaa */
			nlc = nlc_getc();
			if(isalnum(nlc) || nlc == '_') {
				count++;
				nlc_put(nlc);
				continue;
			}
			nlc_ungetc(nlc);
			break;
		}
		if(count == 0) {			/* this will probably lead to syntax err */
			for(;;) {				/* get a string of the form: xxx */
				nlc = nlc_getc();
				if(nlc != 0 && nlc != EOF && !isspace(nlc)) {
					nlc_put(nlc);
					continue;
				}
				nlc_ungetc(nlc);
				break;
			}
		}
		nlc_put(0);					/* null terminate */
		base = numchk(ybuf,16);
		switch(base) {				/* test for number */
		case 16:
			if(detailflg && streq_c(ybuf,"DEC"))	/* special case */
				break;
		case 10:					/* number */
			un = strtoul(ybuf,0,base);
			yylval.ysym = getsym();
			yylval.ysym->s_string = strlcpy(ybuf);
			yylval.ysym->s_type = INUMBER;
			yylval.ysym->s_number = un;
			rv = INUMBER;
			goto lexexit;
		case 0:						/* not a number */
			break;
		default:
			cat_lerror(CAT_UPD_BADC,
				"Bad character '%c' in '%s' in the template for %s.\n",badc,ybuf,yylval.ysym->s_string);
			rv = IERROR;
			goto lexexit;
		}
		if(instanza && (symp = reslookup(ybuf))) {
			yylval.ysym = symp;
			rv = symp->s_type;
			goto lexexit;
		}
		yylval.ysym = getsym();
		yylval.ysym->s_type = ISTRING;
		yylval.ysym->s_string = strlcpy(ybuf);
		rv = ISTRING;
		goto lexexit;
	}
lexexit:
	lasttok = rv;
	return(rv);
}

/*
 * FUNCTION:     lstring
 *               Fill in buf with the string from the input.
 *               Ensure that the string is terminated by '"'.
 * RETURN VALUE: true if the next token is a string terminated by a '"',
 *               false otherwise.
 */
static lstring(buf,length)
char *buf;		/* target for data */
int length;		/* max length of buf */
{
	int nlc;
	int n;

	n = 0;
	nlc_putinit(buf);
	for(;;) {
		nlc = nlc_getc();
		switch(nlc) {
		case '\\':
			nlc = nlc_getc();
			if(nlc == '\n')
				Lineno++;
			continue;
		case '\n':
			Lineno++;
		case '"':
			nlc_put(0);		/* null terminate */
			return(1);
		}
		nlc_put(nlc);
		n++;
		if(n >= length-1) {
			nlc_put(0);		/* null terminate */
			return(1);
		}
	}
}

#define CASE(X) \
	case X: s = "X"; break;

char *tokstr(tok)
{
	char *s;
	static char buf[16];

	if(s = symtokstr(tok))
		return(s);
	switch(tok) {
	case '\n':
		return("INL");
	case 0:
		return("IEOF");
	CASE(INUMBER);
	CASE(ISTRING);
	CASE(IEQUAL);
	CASE(IMINUS);
	CASE(IPLUS);
	default:
		if(' ' < tok && tok < 0x7F)
			sprintf(buf,"'%c'",tok);
		else
			sprintf(buf,"---%d---",tok);
		s = buf;
	}
	return(s);
}

/*
 * FUNCTION:     numchk
 *      Return the base of the number in 'str'.
 *      If the base is ambiguous, use 'defaultbase' as the base.
 *      If the number is invalid, return -1.
 */
numchk(str,defaultbase)
char *str;			/* number */
int defaultbase;	/* use this base if not overrided by 0x */
{
	int c;
	char *cp;
	int xflg;

	xflg = 0;
	cp = str;
	if(!isxdigit(*cp))
		return(0);
	if(*cp == '0' && ((c = cp[1]) == 'x' || c == 'X')) {
		cp += 2;
		defaultbase = 16;
		xflg++;
	}
	while(c = *cp++) {
		if(!isxdigit(c)) {
			if(!xflg)
				return(0);
			badc = c;
			return(-1);
		}
		if(defaultbase == 10 && !isdigit(c))
			defaultbase = 16;
	}
	return(defaultbase);
}

