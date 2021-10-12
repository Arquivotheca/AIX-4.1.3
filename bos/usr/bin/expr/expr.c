static char sccsid[] = "@(#)69	1.18  src/bos/usr/bin/expr/expr.c, cmdsh, bos41J, bai15 4/11/95 09:49:29";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: expr
 *
 * ORIGINS: 3, 26, 27, 71
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

/*
 * NAME:	expr
 *
 * FUNCTION:	expr - evaluate arguments as an expression
 *
 * SYNOPSIS:	expr expression ...
 *
 */

/*
 * operator constants, most self-explanatory
 */
#define	A_STRING	258
#define	NOARG		259
#define	OR		260
#define	AND		261
#define	EQ		262
#define	LT		263
#define	GT		264
#define	GEQ		265
#define	LEQ		266
#define	NEQ		267
#define	ADD		268
#define	SUBT		269
#define	MULT		270
#define	DIV		271
#define	REM		272
#define	MCH		273
#define	MATCH		274
#define	SUBSTR		275
#define	LENGTH		276
#define	INDEX		277

#define	EQL(x,y)	(strcmp(x, y) == 0)	/* string collation compare */
 
/*
 * POSIX required exit values
 */
#define ERR_RE		2	/* Regular Expression syntax error	*/
#define ERR_SYNTAX	2	/* expr syntax error			*/
#define ERR_MALLOC	4	/* malloc error				*/
#define ERR_NUMBER	2	/* invalid number in numeric field	*/

/*
 * includes
 */
#include	<sys/types.h>
#include	<stdio.h>	/* for NULL	*/
#include	<string.h>
#include	<stdlib.h>
#include	<locale.h>
#include	<regex.h>
#include	<nl_types.h>

/*
 * message file stuff
 */
# include	"expr_msg.h"
# define	MSGSTR(Num, Str) 	catgets(catd, MS_EXPR, Num, Str)

/*
 * local functions
 */
char *ltoa(), *rel(), *strsave(), *arith(), *conj();
char *substr(), *length(), *expr_index(), *match();
int yylex(), number();
void yyerror();

/*
 * globals
 */
static char	**Av;			/* global 'argv'		*/
static int	Ac;			/* global 'argc'		*/
static int	Argi;			/* index of current argv	*/
static int	paren;			/* found a paren?		*/
static nl_catd	catd;			/* msg catalog descriptor	*/

/*
 * operators
 */
static char *operator[] = {
	"|", "&", "+", "-", "*", "/", "%", ":",
	"=", "==", "<", "<=", ">", ">=", "!=",
	"match", "substr", "length", "index", "\0" };

/*
 * operator values
 */
static int op[] = { 
	OR, AND, ADD,  SUBT, MULT, DIV, REM, MCH,
	EQ, EQ, LT, LEQ, GT, GEQ, NEQ,
	MATCH, SUBSTR, LENGTH, INDEX };

/*
 * operator precedence table
 */
static int pri[] = { 1,2,3,3,3,3,3,3,4,4,5,5,5,6,7,8,9,9,9 };

/*
 * NAME:	yylex
 *
 * FUNCTION:	yylex - determine next command line token type
 *
 * NOTES:	Yylex looks at the next command line token and returns
 *		a value indicating its type.
 *
 * RETURN VALUE DESCRIPTION:	NOARG if no more command line args exist,
 *		an operator constant if the token is an operator, '(' or ')'
 *		if the token is a parenthesis, else A_STRING indicating
 *		an operand.
 */

int
yylex()
{
	register char *p;
	register int i;

	/*
	 * any args left?
	 */
	if(Argi >= Ac)
		return NOARG;

	p = Av[Argi];

	/*
	 * paren?
	 */
	if((*p == '(' || *p == ')') && p[1] == '\0' )
		return (int)*p;

	/*
	 * operator?
	 */
	for(i = 0; *operator[i]; ++i)
		if(EQL(operator[i], p))
			return op[i];

	/*
	 * must be an operand
	 */

	return A_STRING;
}

/*
 * NAME:	number
 *
 * FUNCTION:	number - determine whether string is a number
 *
 * NOTES:	Number scans a string for non-digit characters.  If any
 *		are found, other than a leading minus, then the string is
 *		not a number.
 *
 * RETURN VALUE DESCRIPTION: "TRUE" if the string is a number, "FALSE" if not
 */

static int
number(s)
register char *s;
{
	if ( *s == '\0' )
		return(0);
	if (*s == '-')
		*s++;
	while (*s>='0' && *s<='9')
		*s++;
	return (*s == '\0');
}

/*
 * NAME:	rel
 *
 * FUNCTION:	rel - evaluate relational operators (=, !=, >, <, <=, >=)
 *
 * NOTES:	Rel evaluates a relational operator and 2 operands.
 *		If the operands are numerical, they are compared
 *		mathematically, else they are compared as strings
 *		using collation sequence.
 *
 * RETURN VALUE DESCRIPTION: "1" if the expression evaluates to true, else "0"
 */

static char *
rel(oper, r1, r2)
int oper;			/* operator	*/
register char *r1; 		/* operand 1	*/
register char *r2; 		/* operand 2	*/
{
	register double i;
	double atof();

	/*
	 * compare numerically or as strings?
	 */
	if(number(r1) && number(r2))
	{
		
		i = atof(r1) - atof(r2);
	}
	else
		i = (double) strcoll(r1, r2);

	/*
	 * evaluate
	 */
	switch(oper) {
	case EQ: 		/* equal			*/
		i = i==0; 
		break;
	case GT: 		/* greater than			*/
		i = i>0; 
		break;
	case GEQ: 		/* greater than or equal	*/
		i = i>=0; 
		break;
	case LT: 		/* less than			*/
		i = i<0; 
		break;
	case LEQ: 		/* less than or equal		*/
		i = i<=0; 
		break;
	case NEQ: 		/* not equal			*/
		i = i!=0; 
		break;
	}

	return i ? "1": "0";
}

/*
 * NAME:	arith
 *
 * FUNCTION:	arith - evaluate an arithmetic operator
 *
 * NOTES:	Arith evaluates an arithmetic operator (+, -, *, /, %).
 *		Both operands must be numeric.
 *
 * RETURN VALUE DESCRIPTION: string containing the answer
 */

static char *
arith(oper, r1, r2)
int oper;		/* operator	*/
char *r1;		/* operand 1	*/
char *r2; 		/* operand 2	*/
{
	register long i1, i2;

	/*
	 * make sure both operands are numeric
	 */
	if(!(number(r1) && number(r2)))
		yyerror(ERR_NUMBER, MSGSTR(NANARG,"non-numeric argument"));

	/*
	 * convert
	 */
	i1 = atol(r1);
	i2 = atol(r2);

	/*
	 * evaluate
	 */
	switch(oper) {
	case ADD: 			/* add		*/
		i1 = i1 + i2; 
		break;
	case SUBT: 			/* subtract	*/
		i1 = i1 - i2; 
		break;
	case MULT: 			/* multiply	*/
		i1 = i1 * i2; 
		break;
	case DIV: 			/* divide	*/
		if (i2 == 0)
			yyerror(ERR_NUMBER, MSGSTR(INVALID, "cannot divide by zero"));
		else
			i1 = i1 / i2; 
		break;
	case REM: 			/* remainder	*/
		if (i2 == 0)
			yyerror(ERR_NUMBER, MSGSTR(INVALID, "cannot divide by zero"));
		else
			i1 = i1 % i2; 
		break;
	}

	/*
	 * convert back to ascii and return
	 */
	return (strsave(ltoa(i1)));
}

/*
 * NAME:	conj
 *
 * FUNCTION:	conj - handle and/or operators
 *
 * NOTES:	Conj evaluates and/or operators (&,|).
 *
 * RETURN VALUE DESCRIPTION: result of evaluation
 */

static char *
conj(oper, r1, r2)
int oper;
char *r1;
char *r2; 
{
	register char *rv;

	switch(oper) {

	case OR:		/* or (|)	*/
		/*
		 * return r1 if it's not 0 or NULL, else
		 * return r2 if it's not 0 or NULL, else
		 * return "0"
		 */
		if(EQL(r1, "0") || EQL(r1, ""))
			if(EQL(r2, "0"))
				rv = "0";
			else if(EQL(r2, ""))
				rv = "";
			else
				rv = r2;
		else
			rv = r1;
		break;
	case AND:		/* and (&)	*/
		/*
		 * return "0" if either r1 or r2 is 0 or null, else
		 * return r1
		 */
		if(EQL(r1, "0") || EQL(r1, "") ||
		   EQL(r2, "0") || EQL(r2, ""))
			rv = "0";
		else
			rv = r1;
		break;
	}

	return rv;
}

/*
 * NAME:	substr
 *
 * FUNCTION:	substr - implement substring operation
 *
 * NOTES:	Substr implements the substring operation.  It
 *		requires 3 operators:  a string, a starting character index,
 *  		and number of characters.
 *		Example:    expr substr "Hi Mom" 1 3    prints "Hi ".
 *
 * RETURN VALUE DESCRIPTION:	the substring
 */

static char *
substr(v, s, w)
char *v;		/* string		*/
char *s;		/* start character index*/
char *w; 		/* # of characters	*/
{
	register long si, wi;
	register char *res, *rv;
	int len;

	/*
	 * make sure index and width are numeric
	 */
	if(!(number(s) && number(w)))
		yyerror(ERR_NUMBER, MSGSTR(NANARG,"non-numeric argument"));

	/*
	 * convert index and width to numbers
	 * make sure index is positive and width not negative
	 */
	si = atol(s);
	wi = atol(w);
	if (si <= 0 || wi < 0)
		yyerror(ERR_SYNTAX, MSGSTR(SYNTAX,"syntax error"));

	/*
	 * find starting character location using start index
	 * MB_CUR_MAX == 1 is the path for single-byte code sets
	 */
	if (MB_CUR_MAX == 1)
		{
		while(--si)
			if(*v)
				++v;

		/*
	 	* and save it
	 	*/
		res = v;

		/*
	 	* find ending location using width
	 	*/
		while(wi--)
			if(*v)
				++v;
		}
	/*
	 * MB_CUR_MAX != 1 is the path for multi-byte code set
	 */
	else
		{
		while (--si > 0 && (len = mblen(v, MB_CUR_MAX)) > 0)
			v += len;

		res = v;

		while (wi-- > 0 && (len = mblen(v, MB_CUR_MAX)) > 0)
			v += len;
		
		}

	/*
 	* allocate space and return value
 	*/
	if ((rv = malloc((size_t) (v - res + 1))) == NULL)
		yyerror(ERR_MALLOC, MSGSTR(MALLOC, "malloc error"));

	(void) strncpy(rv, res, (size_t)(v - res));
	rv[v - res] = '\0';

	return rv;
}

/*
 * NAME:	length
 *
 * FUNCTION:	length - compute character length of a string
 *
 * NOTES:	Length computes the number of characters of a string and returns
 *		it.  Illegal multibyte characters are treated as a NUL.
 *
 * RETURN VALUE DESCRIPTION: string containing character length of input string
 */

static char *
length(s)
char *s; 	/* input string	*/
{
	register long count;	/* # characters in string */
	register int  len;	/* # bytes in character */

	if (MB_CUR_MAX == 1)
		return (strsave(ltoa((long) strlen(s))));

	/*
	 * count characters one at a time
	 * note: could use mbstowcs but that needs a process code array of
	 * unknown length
	 */
	count = 0;
	while ((len = mblen(s, MB_CUR_MAX)) > 0)
		{
		count++;
		s += len;
		}
	/*
	 * convert character count to string, malloc and return
	 */
	return (strsave(ltoa((long) count)));
}

/*
 * NAME:	expr_index
 *
 * FUNCTION:	expr_index - find part of one string in another
 *
 * NOTES:	Index looks in one string for any character
 *		that matches the other.
 *
 * RETURN VALUE DESCRIPTION: if found, a string containing the index of the
 *		char found in the first string, else "0"
 */

static char *
expr_index(s, t)
char *s;		/* string to look at	*/
char *t; 		/* string to look for	*/
{
	/*
	 * compare next s character to all t characters
	 * return first index in s where any t character is found
	 */
	if (MB_CUR_MAX == 1)
		{
		register int i, j;

		for(i = 0; s[i] ; ++i)
			for(j = 0; t[j] ; ++j)
				if(s[i]==t[j])
					return (strsave(ltoa((long) ++i)));
		}
	else
		{
		long	count;
		int	slen;
		int	tlen;
		wchar_t	wcs;
		wchar_t	wct;
		char	*tt;

		count = 0;
		while ((slen = mbtowc(&wcs, s, MB_CUR_MAX)) > 0)
			{
			count++;
			s += slen;
			tt = t;
			while ((tlen = mbtowc(&wct, tt, MB_CUR_MAX)) > 0)
				{
				tt += tlen;
				if (wcs == wct)
					return strsave(ltoa(count));
				}
			}
		}

	return "0";
}

/*
 * NAME:	match
 *
 * FUNCTION:	match - implement match ("match" or ":") function
 *
 * NOTES:	Match implements the 'match' function.  This function
 *		is called by using either of the match operators.  E.g.
 *		"expr match string expr" or "expr string : expr".
 *
 * RETURN VALUE DESCRIPTION:	either the number of chars matched, or
 *		a portion of 'string' (if "\(" and "\)" are used in expr)
 */

static char *
match(s, p)
char *s;
char *p;
{
	long	 count;		/* multibyte character count	*/
	char	*ra;		/* subexpression return address	*/
	int	 rtn;		/* regcomp() errror code	*/
	long     num;		/* # of matched charcters	*/
	regmatch_t off[2];	/* subexpression offsets	*/
	regex_t  reg;		/* RE structure			*/
	char	 errbuf[128];	/* RE error buffer		*/

	/*
	 * compile the basic RE, stop on syntax error
	 */
	rtn = regcomp(&reg, p, 0);
	if (rtn != 0)
		{
		regerror(rtn, &reg, errbuf, sizeof(errbuf));
		yyerror(ERR_RE, errbuf);
		}

	/*
	 * determine if string is matched by RE pattern
	 */
	if ((regexec(&reg, s, 2, off, 0) == 0) && (off[0].rm_so == 0))
	{
		/*
		 * if \(subexpression\) exists, return ptr to matched substring
		 */
		if (reg.re_nsub != 0)
			{
			num = off[1].rm_eo - off[1].rm_so;
			if ((ra = (char *)malloc((size_t) num+1)) == NULL)
				yyerror(ERR_MALLOC, MSGSTR(MALLOC, "malloc error"));
			(void)strncpy(ra, s+off[1].rm_so, (size_t)num);
			ra[num] = '\0';
			return ra;
			}
		/*
		 * else return ptr to string which is number of matched characters
		 */
		else
			{
			num = off[0].rm_eo - off[0].rm_so;
			if (MB_CUR_MAX == 1)
				return strsave(ltoa(num));
			else
				{
				ra = s+off[0].rm_so;
				count = 0;
				while (num > 0 && (rtn = mblen(ra, MB_CUR_MAX)) > 0)
					{
					count++;
					ra += rtn;
					num -= rtn;
					}
				return strsave(ltoa(count));
				}
			}
	}
	else
	{
	/* Determine if the failed matched string by RE pattern contained
 	   a sub-expression */
		if(reg.re_nsub >= 1) 
			return "";
		else
			return "0";
	}
}


/*
 * NAME:	yyerror
 *
 * FUNCTION:	yyerror - print an error and exit
 *
 * NOTES:	Yyerror prints an error and exits with value of 2.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
yyerror(i, s)
int 	i;	/* exit status	*/
char	*s;	/* error text	*/
{
	(void) write(2, "expr: ", 6);
	(void) write(2, s, (unsigned) strlen(s));
	(void) write(2, "\n", 1);

	exit(i);

	/* NOTREACHED */
}

/*
 * NAME:	ltoa
 *
 * FUNCTION:	ltoa - long to ascii
 *
 * NOTES:	Ltoa formats a long integer into a string and returns
 *		it.  The return value is static.
 *
 * RETURN VALUE DESCRIPTION:	the formatted string
 */

static char *
ltoa(l)
long l;
{
	static char str[20];	/* must be big enuf to hold a long... */

	(void) sprintf(str, "%ld", l);

	return str;
}

/*
 * NAME:	expres
 *
 * FUNCTION:	expres
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 */

static char *
expres(prior, par)
int prior;
int par; 
{
	static int noarg = 0;
	int ylex, temp, op1;
	char *r1, *ra, *rb, *rc;
	long i1;

	/*
	 * retrieve next token and check validity
	 */
	if ((ylex = yylex()) >= NOARG && ylex < MATCH)
		yyerror(ERR_SYNTAX, MSGSTR(SYNTAX, "syntax error"));

	/*
	 * string?
	 */
	if (ylex == A_STRING) {
		r1 = Av[Argi++];
		temp = Argi;
	}

	/*
	 * left paren?
	 */
	else if (ylex == '(') {
		paren++;
		Argi++;
		/*
		 * get embedded expression
		 */
		r1 = expres(0, Argi);
		Argi--;
	}

lop:
	if ((ylex = yylex()) > NOARG && ylex < MATCH) {
		op1 = ylex;
		Argi++;

		if (pri[op1-OR] <= prior ) 
			return r1;

		else {
			switch(op1) {
			case OR:
			case AND:
				r1 = conj(op1,r1,expres(pri[op1-OR],0));
				break;
			case EQ:
			case LT:
			case GT:
			case LEQ:
			case GEQ:
			case NEQ:
				r1=rel(op1,r1,expres(pri[op1-OR],0));
				break;
			case ADD:
			case SUBT:
			case MULT:
			case DIV:
			case REM:
				r1=arith(op1,r1,expres(pri[op1-OR],0));
				break;
			case MCH:
				r1=match(r1,expres(pri[op1-OR],0));
				break;
			}
			if(noarg == 1) {
				return r1;
			}
			Argi--;
			goto lop;
		}
	}

	/*
	 * right paren?
	 */
	if((ylex = yylex()) == ')') {
		if(par == Argi)
			yyerror(ERR_SYNTAX, MSGSTR(SYNTAX,"syntax error"));

		if(par != 0) {
			paren--;
			Argi++;
		}

		Argi++;
		return r1;
	}

	if((ylex = yylex()) > MCH && ylex <= INDEX) {
		if (Argi == temp)
			return r1;

		op1 = ylex;
		Argi++;

		switch(op1) {
		case SUBSTR: 
			rc = expres(pri[op1-OR],0);
		case MATCH:
		case INDEX: 
			rb = expres(pri[op1-OR],0);
		case LENGTH: 
			ra = expres(pri[op1-OR],0);
		}

		switch(op1) {
		case MATCH: 
			r1 = match(rb,ra); 
			break;
		case INDEX: 
			r1 = expr_index(rb,ra); 
			break;
		case SUBSTR: 
			r1 = substr(rc,rb,ra); 
			break;
		case LENGTH: 
			r1 = length(ra); 
			break;
		}

		if(noarg == 1)
			return r1;

		Argi--;
		goto lop;
	}

	if ((ylex = yylex()) == NOARG)
		noarg = 1;

	if (number(r1)) {
	    /*
	     * convert string to number and back to remove leading
	     * zeros.  This is required by POSIX so that strings
	     * such as "000" are converted to "0", causing expr to
	     * correctly print "0" and exit with a value of 1.
	     */
	    i1 = atol(r1);
	    return (strsave(ltoa(i1)));
	} else
	    return r1;
}

/*
 * NAME:	strsave
 *
 * FUNCTION:	strsave - save a string somewhere
 *
 * NOTES:	Strsave allocates memory for a string.  The new memory
 *		is returned.  Inspired by K&R, pg 103.
 *
 * RETURN VALUE DESCRIPTION:	NULL if no memory could be allocated,
 *		else a pointer to the memory containing the new string.
 */

static char *
strsave(string)
char *string;
{
	register char *new;

	if ((new = (char *) malloc((size_t) (strlen(string) + 1))) == NULL)
		yyerror(ERR_MALLOC, MSGSTR(MALLOC, "malloc error"));

	return ((char *) strcpy(new, string));
}

/*
 * NAME:	main
 *
 * FUNCTION:	main - main routine
 *
 * NOTES:	Main inits globals, calls expres() to evaluate the
 *		command line, prints out the value, and exits with
 *		either a true (0) or false (1) value.
 *
 * RETURN VALUE DESCRIPTION:	0 if the expression is true, else 1
 */

int
main(argc, argv)
int argc;
char **argv; 
{
	char	*buf;

	(void) setlocale (LC_ALL, "");

	/*
	 * initialization
	 */

	catd = catopen(MF_EXPR, NL_CAT_LOCALE);

	if (strcmp(argv[1], "-?") == 0){
		fprintf(stderr, MSGSTR(USAGE, "Usage: expr expression\n"));
		exit(3);
	}

	if (strcmp(argv[1], "--") == 0)
		Argi = 2;
	else	
		Argi = 1;
	Ac = argc;
	paren = 0;
	Av = argv;

	/* 
	 * evaluate expression on the command line
	 */
	buf = expres(0, 1);

	/*
	 * check syntax error
	 */
	if(Ac != Argi || paren != 0) {
		yyerror(ERR_SYNTAX, MSGSTR(SYNTAX,"syntax error"));
	}

	/*
	 * write result
	 */
	if(*buf != '\0')
	{
		(void) write(1, buf, (unsigned) strlen(buf));
		(void) write(1, "\n", 1);
	}

	/*
	 * exit true or false depending on result
	 */
	exit(EQL(buf, "0") || buf[0] == '\0' ? 1 : 0);
	/* NOTREACHED */
}
