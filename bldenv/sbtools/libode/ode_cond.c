/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: CondCvtArg
 *		CondDoDefined
 *		CondDoEmpty
 *		CondE
 *		CondF
 *		CondKWDefined
 *		CondKWEmpty
 *		CondPushBack
 *		CondT
 *		CondToken
 *		Cond_AddKeyword
 *		Cond_End
 *		Cond_Eval
 *		Cond_GetArg
 *		Cond_Init
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: cond.c,v $
 * Revision 1.1.9.2  1993/11/10  16:56:45  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:55:50  root]
 *
 * Revision 1.1.9.1  1993/11/08  20:18:04  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:21  damon]
 * 
 * Revision 1.1.7.2  1993/04/28  20:21:43  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:21:31  damon]
 * 
 * Revision 1.1.2.8  1992/12/09  21:06:19  damon
 * 	CR 329. Removed const in keyword declaration
 * 	[1992/12/09  21:06:08  damon]
 * 
 * Revision 1.1.2.7  1992/12/03  17:20:32  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:07:57  damon]
 * 
 * Revision 1.1.2.6  1992/11/11  15:48:02  damon
 * 	CR 329. Removed NO_PROTO stuff
 * 	[1992/11/11  15:47:17  damon]
 * 
 * Revision 1.1.2.5  1992/11/09  20:32:55  damon
 * 	CR 329. Changed NO_TYPEDEF_PROTO_STRUCT to NO_TYPEDEF_IN_PROTO
 * 	[1992/11/09  20:01:48  damon]
 * 
 * Revision 1.1.2.4  1992/11/06  18:34:51  damon
 * 	CR 329. Made more portable
 * 	[1992/11/06  18:32:52  damon]
 * 
 * Revision 1.1.2.3  1992/11/06  17:31:44  damon
 * 	CR 329. Added NO_TYPEDEF_PROTO_STRUCT
 * 	[1992/11/06  17:31:33  damon]
 * 
 * Revision 1.1.2.2  1992/09/24  19:28:02  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:38:56  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:14  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:36:48  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  15:42:33  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)95  1.1  src/bldenv/sbtools/libode/ode_cond.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:33";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)cond.c	5.6 (Berkeley) 6/1/90";
#endif /* not lint */

/*-
 * cond.c --
 *	Functions to handle conditionals in a makefile.
 *
 * Interface:
 *	Cond_Eval 	Evaluate the conditional in the passed line.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ode/odedefs.h>

typedef int Boolean;

char *(*CondVarParse)(const char *, Boolean *, int *);
char *(*CondVarValue)(const char *);
void (*CondError)(const char *, ...);
Boolean CondDebug;

struct keyword {
    char *name;
    int len;
    Boolean (*func)(char **, int *, char **, Boolean);
    Boolean (*eval)(int, char *);
    struct keyword *next;
} *keywords;

/*
 * The parsing of conditional expressions is based on this grammar:
 *	E -> F || E
 *	E -> F
 *	F -> T && F
 *	F -> T
 *	T -> defined(variable)
 *	T -> empty(varspec)
 *	T -> symbol
 *	T -> $(varspec) op value
 *	T -> $(varspec) == "string"
 *	T -> $(varspec) != "string"
 *	T -> ( E )
 *	T -> ! T
 *	op -> == | != | > | < | >= | <=
 *
 * 'symbol' is some other symbol to which the default function (condDefProc)
 * is applied.
 *
 * Tokens are scanned from the 'condExpr' string. The scanner (CondToken)
 * will return And for '&' and '&&', Or for '|' and '||', Not for '!',
 * LParen for '(', RParen for ')' and will evaluate the other terminal
 * symbols, using either the default function or the function given in the
 * terminal, and return the result as either True or False.
 *
 * All Non-Terminal functions (CondE, CondF and CondT) return Err on error.
 */
typedef enum {
    And, Or, Not, True, False, LParen, RParen, EndOfFile, None, Err
} Token;

/*-
 * Structures to handle elegantly the different forms of #if's. The
 * last two fields are stored in condInvert and condDefProc, respectively.
 */
static Boolean	  CondDoDefined(int, char *);
static Boolean	  CondDoEmpty(int, char *);

static struct If {
    const char	*form;	      /* Form of if */
    int		formlen;      /* Length of form */
    Boolean	doNot;	      /* TRUE if default function should be negated */
    Boolean	(*defProc)(int, char *); /* Default function to apply */
} ifs[] = {
    { "ifdef",	  5,	  FALSE,  CondDoDefined },
    { "ifndef",	  6,	  TRUE,	  CondDoDefined },
    { "if",	  2,	  FALSE,  CondDoDefined },
    { (char *)0,	  0,	  FALSE,  (Boolean (*)(int, char *))0 },
};

static Boolean	  condInvert;	    	/* Invert the default function */
static Boolean	  (*condDefProc)(int, char *); /* Default function to apply */
static char 	  *condExpr;	    	/* The expression to parse */
static Token	  condPushBack=None;	/* Single push-back token used in
					 * parsing */

#define	MAXIF		30	  /* greatest depth of #if'ing */

static Boolean	  condStack[MAXIF]; 	/* Stack of conditionals's values */
static int  	  condTop = MAXIF;  	/* Top-most conditional */
static int  	  skipIfLevel=0;    	/* Depth of skipped conditionals */
static Boolean	  skipLine = FALSE; 	/* Whether the parse module is skipping
					 * lines */

static Token	  CondT(Boolean);
static Token	  CondF(Boolean);
static Token	  CondE(Boolean);

/*-
 *-----------------------------------------------------------------------
 * CondPushBack --
 *	Push back the most recent token read. We only need one level of
 *	this, so the thing is just stored in 'condPushback'.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	condPushback is overwritten.
 *
 *-----------------------------------------------------------------------
 */
static void
CondPushBack (Token t)	/* Token to push back into the "stream" */
{
    condPushBack = t;
}

/*-
 *-----------------------------------------------------------------------
 * Cond_GetArg --
 *	Find the argument of a built-in function.
 *
 * Results:
 *	The length of the argument and the address of the argument.
 *
 * Side Effects:
 *	The pointer is set to point to the closing parenthesis of the
 *	function call.
 *
 *-----------------------------------------------------------------------
 */
int
Cond_GetArg (
    char    	  **linePtr,
    char    	  **argPtr,
    const char	  *func,
    Boolean 	  parens)   	/* TRUE if arg should be bounded by parens */
{
    register char *cp;
    int	    	  argLen;
    char    *string, *estring;
    int     stringlen;

    cp = *linePtr;
    if (parens) {
	while (*cp != '(' && *cp != '\0') {
	    cp++;
	}
	if (*cp == '(') {
	    cp++;
	}
    }

    if (*cp == '\0') {
	/*
	 * No arguments whatsoever. Because 'make' and 'defined' aren't really
	 * "reserved words", we don't print a message. I think this is better
	 * than hitting the user with a warning message every time s/he uses
	 * the word 'make' or 'defined' at the beginning of a symbol...
	 */
	*argPtr = cp;
	return (0);
    }

    while (*cp == ' ' || *cp == '\t') {
	cp++;
    }

    /*
     * Allocate a buffer for the argument.
     */
    stringlen = 63;
    estring = string = (char *) malloc(stringlen + 1);
    
    while ((strchr(" \t)&|", *cp) == (char *)NULL) && (*cp != '\0')) {
	if (*cp == '$') {
	    /*
	     * Parse the variable spec and install it as part of the argument
	     * if it's valid. We tell CondVarParse to complain on an undefined
	     * variable, so we don't do it too. Nor do we return an error,
	     * though perhaps we should...
	     */
	    char  	*cp2;
	    int		len, newlen;
	    Boolean	errorState;

	    errorState = TRUE;
	    cp2 = (*CondVarParse)(cp, &errorState, &len);
	    cp += len;
	    newlen = strlen(cp2);
	    if (estring + newlen >= string + stringlen) {
		len = stringlen + 64;
		while (estring + newlen >= string + len)
		    len += 64;
		string = realloc(string, len + 1);
		estring = string + stringlen;
		stringlen = len;
	    }
	    memcpy(estring, cp2, newlen);
	    estring += newlen;
	    free(cp2);
	    continue;
	}
	if (estring == string + stringlen) {
	    string = realloc(string, stringlen + 65);
	    estring = string + stringlen;
	    stringlen += 64;
	}
	*estring++ = *cp++;
    }
    *estring = '\0';

    *argPtr = string;
    argLen = estring - string;

    while (*cp == ' ' || *cp == '\t') {
	cp++;
    }
    if (parens) {
	if (*cp != ')') {
	    (*CondError) ("Missing closing parenthesis for %s()", func);
	    return (0);
	}
	/*
	 * Advance pointer past close parenthesis.
	 */
	cp++;
    }
    
    *linePtr = cp;
    return (argLen);
}

/*-
 *-----------------------------------------------------------------------
 * CondDoDefined --
 *	Handle the 'defined' function for conditionals.
 *
 * Results:
 *	TRUE if the given variable is defined.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
CondDoDefined (int argLen, char *arg)
{
    char    savec = arg[argLen];
    Boolean result;

    arg[argLen] = '\0';
    if ((*CondVarValue)(arg) != (char *)NULL) {
	result = TRUE;
    } else {
	result = FALSE;
    }
    arg[argLen] = savec;
    if (CondDebug)
	printf("CondDoDefined: %s %s\n", arg, result ? "TRUE" : "FALSE");
    return (result);
}

/*-
 *-----------------------------------------------------------------------
 * CondDoEmpty --
 *	Handle the 'empty' function for conditionals.
 *
 * Results:
 *	TRUE if the given variable is empty.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
static Boolean
CondDoEmpty (int argLen, char *arg)
{
    if (arg && *arg != '\0')
	return (FALSE);
    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * CondCvtArg --
 *	Convert the given number into an integer. If the number begins
 *	with 0x, or just x, it is interpreted as a hexadecimal integer
 *	and converted to an integer from there. All other strings just have
 *	atoi called on them.
 *
 * Results:
 *	The integer value of string.
 *
 * Side Effects:
 *	
 *
 *-----------------------------------------------------------------------
 */
static int
CondCvtArg(register char *str)
{
    int	    	  	sign = 1;
    
    if (*str == '-') {
	sign = -1;
	str++;
    } else if (*str == '+') {
	str++;
    }
    if (((*str == '0') && (str[1] == 'x')) ||
	(*str == 'x'))
    {
	register int i;
	
	str += (*str == 'x') ? 1 : 2;

	i = 0;

	while (isxdigit(*str)) {
	    i *= 16;
	    if (*str <= '9') {
		i += *str - '0';
	    } else if (*str <= 'F') {
		i += *str - 'A' + 10;
	    } else {
		i += *str - 'a' + 10;
	    }
	    str++;
	}
	if (sign < 0) {
	    return(-i);
	} else {
	    return(i);
	}
    } else if (sign < 0) {
	return(- atoi(str));
    } else {
	return(atoi(str));
    }
}

/*-
 *-----------------------------------------------------------------------
 * CondToken --
 *	Return the next token from the input.
 *
 * Results:
 *	A Token for the next lexical token in the stream.
 *
 * Side Effects:
 *	condPushback will be set back to None if it is used.
 *
 *-----------------------------------------------------------------------
 */
static Token
CondToken(Boolean doEval)
{
    Token	  t;

    if (condPushBack != None) {
	t = condPushBack;
	condPushBack = None;
	return(t);
    }
    while (*condExpr == ' ' || *condExpr == '\t') {
	condExpr++;
    }
    switch (*condExpr) {
	case '(':
	    t = LParen;
	    condExpr++;
	    break;
	case ')':
	    t = RParen;
	    condExpr++;
	    break;
	case '|':
	    if (condExpr[1] == '|') {
		condExpr++;
	    }
	    condExpr++;
	    t = Or;
	    break;
	case '&':
	    if (condExpr[1] == '&') {
		condExpr++;
	    }
	    condExpr++;
	    t = And;
	    break;
	case '!':
	    t = Not;
	    condExpr++;
	    break;
	case '\n':
	case '\0':
	    t = EndOfFile;
	    break;
	case '$': {
	    char	*lhs;
	    char	*rhs;
	    char	*op;
	    int	varSpecLen;
	    Boolean	errorState;

	    /*
	     * Parse the variable spec and skip over it, saving its
	     * value in lhs.
	     */
	    t = Err;
	    errorState = doEval;
	    lhs = (*CondVarParse)(condExpr, &errorState, &varSpecLen);
	    if (errorState) {
		/*
		 * Even if !doEval, we still report syntax errors, which
		 * is what getting var_Error back with !doEval means.
		 */
		return(Err);
	    }
	    condExpr += varSpecLen;

	    /*
	     * Skip whitespace to get to the operator
	     */
	    while (isspace(*condExpr)) {
		condExpr++;
	    }
	    /*
	     * Make sure the operator is a valid one. If it isn't a
	     * known relational operator, pretend we got a
	     * != 0 comparison.
	     */
	    op = condExpr;
	    switch (*condExpr) {
		case '!':
		case '=':
		case '<':
		case '>':
		    if (condExpr[1] == '=') {
			condExpr += 2;
		    } else {
			condExpr += 1;
		    }
		    break;
		default:
		    op = (char *)"!="; /* LINT */
		    rhs = (char *)"0"; /* LINT */

		    goto do_compare;
	    }
	    while (isspace(*condExpr)) {
		condExpr++;
	    }
	    if (*condExpr == '\0') {
		(*CondError)("Missing right-hand-side of operator");
		goto error;
	    }
	    rhs = condExpr;
do_compare:
	    if (*rhs == '"') {
		/*
		 * Doing a string comparison. Only allow == and != for
		 * operators.
		 */
		char    *string, *estring;
		char    *cp;
		int     stringlen;

		if (((*op != '!') && (*op != '=')) || (op[1] != '=')) {
		    (*CondError)("String comparison operator should be either == or !=");
		    goto error;
		}

		stringlen = 63;
		estring = string = (char *) malloc(stringlen + 1);
		for (cp = rhs+1; *cp != '"' && *cp != '\0'; cp++) {
		    if (*cp == '$') {
			char *cp2;
			int	len, newlen;
			Boolean errorState2;

			errorState2 = doEval;
			cp2 = (*CondVarParse)(cp, &errorState2, &len);
			if (errorState2) {
			    if (estring == string + stringlen) {
				string = realloc(string, stringlen + 65);
				estring = string + stringlen;
				stringlen += 64;
			    }
			    *estring++ = *cp;
			    continue;
			}
			cp += len - 1;
			newlen = strlen(cp2);
			if (estring + newlen >= string + stringlen) {
			    len = stringlen + 64;
			    while (estring + newlen >= string + len)
				len += 64;
			    string = realloc(string, len + 1);
			    estring = string + stringlen;
			    stringlen = len;
			}
			memcpy(estring, cp2, newlen);
			estring += newlen;
			free(cp2);
			continue;
		    }
		    if ((*cp == '\\') && (cp[1] != '\0')) {
			/*
			 * Backslash escapes things -- skip over next
			 * character, if it exists.
			 */
			cp++;
		    }
		    if (estring == string + stringlen) {
			string = realloc(string, stringlen + 65);
			estring = string + stringlen;
			stringlen += 64;
		    }
		    *estring++ = *cp;
		}
		*estring = '\0';

		if (CondDebug) {
		    printf("lhs = \"%s\", rhs = \"%s\", op = %.2s\n",
			   lhs, string, op);
		}
		/*
		 * Null-terminate rhs and perform the comparison.
		 * t is set to the result.
		 */
		if (*op == '=') {
		    t = strcmp(lhs, string) ? False : True;
		} else {
		    t = strcmp(lhs, string) ? True : False;
		}
		free(string);
		if (rhs == condExpr) {
		    condExpr = cp + 1;
		}
	    } else {
		/*
		 * rhs is an integer. Convert both the lhs and the rhs
		 * to an int and compare the two.
		 */
		int  	left, right;
		char    	*string;

		left = CondCvtArg(lhs);
		if (*rhs == '$') {
		    int 	len;
		    Boolean errorState2;

		    errorState2 = doEval;
		    string = (*CondVarParse)(rhs, &errorState2, &len);
		    if (errorState2) {
			right = 0;
		    } else {
			right = CondCvtArg(string);
			free(string);
			if (rhs == condExpr) {
			    condExpr += len;
			}
		    }
		} else {
		    right = CondCvtArg(rhs);
		    if (rhs == condExpr) {
			/*
			 * Skip over the right-hand side
			 */
			while(!isspace(*condExpr) && (*condExpr != '\0')) {
			    condExpr++;
			}
		    }
		}

		if (CondDebug) {
		    printf("left = %f, right = %f, op = %.2s\n", (double) left,
			   (double) right, op);
		}
		switch(op[0]) {
		case '!':
		    if (op[1] != '=') {
			(*CondError)("Unknown operator");
			goto error;
		    }
		    t = (left != right ? True : False);
		    break;
		case '=':
		    if (op[1] != '=') {
			(*CondError)("Unknown operator");
			goto error;
		    }
		    t = (left == right ? True : False);
		    break;
		case '<':
		    if (op[1] == '=') {
			t = (left <= right ? True : False);
		    } else {
			t = (left < right ? True : False);
		    }
		    break;
		case '>':
		    if (op[1] == '=') {
			t = (left >= right ? True : False);
		    } else {
			t = (left > right ? True : False);
		    }
		    break;
		}
	    }
error:
	    free(lhs);
	    break;
	}
	default: {
	    Boolean (*evalProc)(int, char *)=NULL;
	    Boolean invert = FALSE;
	    char	*arg;
	    int	arglen;
	    struct keyword *keyw;

	    for (keyw = keywords; keyw != NULL; keyw = keyw->next) {
		if (strncmp (condExpr, keyw->name, keyw->len) != 0)
		    continue;
		if ((*keyw->func)(&condExpr, &arglen, &arg, doEval)) {
		    keyw = NULL;
		    break;
		}
		evalProc = keyw->eval;
		break;
	    }
	    if (keyw == NULL) {
		invert = condInvert;
		evalProc = condDefProc;
		arglen = Cond_GetArg(&condExpr, &arg, "", FALSE);
	    }

	    /*
	     * Evaluate the argument using the set function. If invert
	     * is TRUE, we invert the sense of the function.
	     */
	    t = (!doEval || (* evalProc) (arglen, arg) ?
		 (invert ? False : True) :
		 (invert ? True : False));
	    free(arg);
	    break;
	}
    }
    return (t);
}

/*-
 *-----------------------------------------------------------------------
 * CondT --
 *	Parse a single term in the expression. This consists of a terminal
 *	symbol or Not and a terminal symbol (not including the binary
 *	operators):
 *	    T -> defined(variable) | make(target) | exists(file) | symbol
 *	    T -> ! T | ( E )
 *
 * Results:
 *	True, False or Err.
 *
 * Side Effects:
 *	Tokens are consumed.
 *
 *-----------------------------------------------------------------------
 */
static Token
CondT(Boolean doEval)
{
    Token   t;

    t = CondToken(doEval);

    if (t == EndOfFile) {
	/*
	 * If we reached the end of the expression, the expression
	 * is malformed...
	 */
	t = Err;
    } else if (t == LParen) {
	/*
	 * T -> ( E )
	 */
	t = CondE(doEval);
	if (t != Err) {
	    if (CondToken(doEval) != RParen) {
		t = Err;
	    }
	}
    } else if (t == Not) {
	t = CondT(doEval);
	if (t == True) {
	    t = False;
	} else if (t == False) {
	    t = True;
	}
    }
    return (t);
}

/*-
 *-----------------------------------------------------------------------
 * CondF --
 *	Parse a conjunctive factor (nice name, wot?)
 *	    F -> T && F | T
 *
 * Results:
 *	True, False or Err
 *
 * Side Effects:
 *	Tokens are consumed.
 *
 *-----------------------------------------------------------------------
 */
static Token
CondF(Boolean doEval)
{
    Token   l, o;

    l = CondT(doEval);
    if (l != Err) {
	o = CondToken(doEval);

	if (o == And) {
	    /*
	     * F -> T && F
	     *
	     * If T is False, the whole thing will be False, but we have to
	     * parse the r.h.s. anyway (to throw it away).
	     * If T is True, the result is the r.h.s., be it an Err or no.
	     */
	    if (l == True) {
		l = CondF(doEval);
	    } else {
		(void) CondF(FALSE);
	    }
	} else {
	    /*
	     * F -> T
	     */
	    CondPushBack (o);
	}
    }
    return (l);
}

/*-
 *-----------------------------------------------------------------------
 * CondE --
 *	Main expression production.
 *	    E -> F || E | F
 *
 * Results:
 *	True, False or Err.
 *
 * Side Effects:
 *	Tokens are, of course, consumed.
 *
 *-----------------------------------------------------------------------
 */
static Token
CondE(Boolean doEval)
{
    Token   l, o;

    l = CondF(doEval);
    if (l != Err) {
	o = CondToken(doEval);

	if (o == Or) {
	    /*
	     * E -> F || E
	     *
	     * A similar thing occurs for ||, except that here we make sure
	     * the l.h.s. is False before we bother to evaluate the r.h.s.
	     * Once again, if l is False, the result is the r.h.s. and once
	     * again if l is True, we parse the r.h.s. to throw it away.
	     */
	    if (l == False) {
		l = CondE(doEval);
	    } else {
		(void) CondE(FALSE);
	    }
	} else {
	    /*
	     * E -> F
	     */
	    CondPushBack (o);
	}
    }
    return (l);
}

/*-
 *-----------------------------------------------------------------------
 * Cond_Eval --
 *	Evaluate the conditional in the passed line. The line
 *	looks like this:
 *	    .<cond-type> <expr>
 *	where <cond-type> is any of if, ifmake, ifnmake, ifdef,
 *	ifndef, elif, elifmake, elifnmake, elifdef, elifndef
 *	and <expr> consists of &&, ||, !, make(target), defined(variable)
 *	and parenthetical groupings thereof.
 *
 * Results:
 *	COND_PARSE	if should parse lines after the conditional
 *	COND_SKIP	if should skip lines after the conditional
 *	COND_INVALID  	if not a valid conditional.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
int
Cond_Eval (char *line)    /* Line to parse */
{
    struct If	    *ifp;
    Boolean 	    isElse;
    Boolean 	    value;

    /*
     * Find what type of if we're dealing with. The result is left
     * in ifp and isElse is set TRUE if it's an elif line.
     */
    if (line[0] == 'e' && line[1] == 'l') {
	line += 2;
	isElse = TRUE;
    } else if (line[0] == 'e' && strncmp (line, "endif", 5) == 0) {
	/*
	 * End of a conditional section. If skipIfLevel is non-zero, that
	 * conditional was skipped, so lines following it should also be
	 * skipped. Hence, we return COND_SKIP. Otherwise, the conditional
	 * was read so succeeding lines should be parsed (think about it...)
	 * so we return COND_PARSE, unless this endif isn't paired with
	 * a decent if.
	 */
	if (skipIfLevel != 0) {
	    skipIfLevel -= 1;
	    return (0);
	} else {
	    if (condTop == MAXIF) {
		(*CondError)("if-less endif");
		return (-1);
	    } else {
		skipLine = FALSE;
		condTop += 1;
		return (1);
	    }
	}
    } else if (line[0] == 'i') {
	isElse = FALSE;
    } else {
	/*
	 * Not a valid conditional type. No error...
	 */
	return (-1);
    }
    
    /*
     * Figure out what sort of conditional it is -- what its default
     * function is, etc. -- by looking in the table of valid "ifs"
     */
    for (ifp = ifs; ifp->form != (char *)0; ifp++) {
	if (strncmp (ifp->form, line, ifp->formlen) == 0) {
	    break;
	}
    }

    if (ifp->form == (char *) 0) {
	/*
	 * Nothing fit. If the first word on the line is actually
	 * "else", it's a valid conditional whose value is the inverse
	 * of the previous if we parsed.
	 */
	if (isElse && (line[0] == 's') && (line[1] == 'e')) {
	    if (condTop == MAXIF) {
		(*CondError)("if-less else");
		return (-1);
	    } else if (skipIfLevel == 0) {
		value = !condStack[condTop];
	    } else {
		return (0);
	    }
	} else {
	    /*
	     * Not a valid conditional type. No error...
	     */
	    return (-1);
	}
    } else {
	if (isElse) {
	    if (condTop == MAXIF) {
		(*CondError)("if-less elif");
		return (-1);
	    } else if (skipIfLevel != 0) {
		/*
		 * If skipping this conditional, just ignore the whole thing.
		 * If we don't, the user might be employing a variable that's
		 * undefined, for which there's an enclosing ifdef that
		 * we're skipping...
		 */
		return(0);
	    }
	} else if (skipLine) {
	    /*
	     * Don't even try to evaluate a conditional that's not an else if
	     * we're skipping things...
	     */
	    skipIfLevel += 1;
	    return(0);
	}

	/*
	 * Initialize file-global variables for parsing
	 */
	condDefProc = ifp->defProc;
	condInvert = ifp->doNot;
	
	line += ifp->formlen;
	
	while (*line == ' ' || *line == '\t') {
	    line++;
	}
	
	condExpr = line;
	condPushBack = None;
	
	switch (CondE(TRUE)) {
	    case True:
		if (CondToken(TRUE) == EndOfFile) {
		    value = TRUE;
		    break;
		}
		goto err;
		/*FALLTHRU*/
	    case False:
		if (CondToken(TRUE) == EndOfFile) {
		    value = FALSE;
		    break;
		}
		/*FALLTHRU*/
	    case Err:
	    err:
		(*CondError)("Malformed conditional (%s)", line);
		return (-1);
	    default:
		value = FALSE;
		break;
	}
    }
    if (!isElse) {
	condTop -= 1;
    } else if ((skipIfLevel != 0) || condStack[condTop]) {
	/*
	 * If this is an else-type conditional, it should only take effect
	 * if its corresponding if was evaluated and FALSE. If its if was
	 * TRUE or skipped, we return COND_SKIP (and start skipping in case
	 * we weren't already), leaving the stack unmolested so later elif's
	 * don't screw up...
	 */
	skipLine = TRUE;
	return (0);
    }

    if (condTop < 0) {
	/*
	 * This is the one case where we can definitely proclaim a fatal
	 * error. If we don't, we're hosed.
	 */
	(*CondError)("Too many nested if's. %d max.", MAXIF);
	return (-1);
    } else {
	condStack[condTop] = value;
	skipLine = !value;
	return (value ? 1 : 0);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Cond_End --
 *	Make sure everything's clean at the end of a makefile.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	CondError will be called if open conditionals are around.
 *
 *-----------------------------------------------------------------------
 */
void
Cond_End(void)
{
    if (condTop != MAXIF) {
	(*CondError)("%d open conditional%s", MAXIF-condTop,
		     MAXIF-condTop == 1 ? "" : "s");
    }
    condTop = MAXIF;
}

Boolean
CondKWDefined(char **condExprPtr, int *arglenPtr, char **argPtr,
	      Boolean doEval)
{
    /*
     * Use CondDoDefined to evaluate the argument and
     * Cond_GetArg to extract the argument from the 'function
     * call'.
     */
    *condExprPtr += 7;
    *arglenPtr = Cond_GetArg (condExprPtr, argPtr, "defined", TRUE);
    if (*arglenPtr == 0) {
	*condExprPtr -= 7;
	return(TRUE);
    }
    return(FALSE);
}

Boolean
CondKWEmpty(char **condExprPtr, int *arglenPtr, char **argPtr,
	    Boolean doEval)
{
    /*
     * Use Var_Parse to parse the spec in parens and return
     * True if the resulting string is empty.
     */
    int	    length;
    char    *val;
    char    *condExpr2 = *condExprPtr;
    int	    arglen;

    condExpr2 += 5;

    for (arglen = 0;
	 condExpr2[arglen] != '(' && condExpr2[arglen] != '\0';
	 arglen += 1)
    {
	/* void */ ;
    }
    if (condExpr2[arglen] != '\0') {
	Boolean errorState = doEval;

	val = (*CondVarParse)(&condExpr2[arglen - 1], &errorState, &length);
	if (errorState)
	    *argPtr = (char *)"";
	else
	    *argPtr = val;
	/*
	 * Advance condExpr to beyond the closing ). Note that
	 * we subtract one from arglen + length b/c length
	 * is calculated from condExpr[arglen - 1].
	 */
	*condExprPtr = condExpr2 + arglen + length - 1;
	*arglenPtr = arglen;
	return(FALSE);
    }
    return(TRUE);
}

void
Cond_AddKeyword(const char *name,
		Boolean (*func)(char **, int *, char **, Boolean),
		Boolean (*eval)(int, char *))
{
    struct keyword *keyw;

    keyw = malloc(sizeof(struct keyword));
    keyw->name = strdup((char *)name);
    keyw->len = strlen(name);
    keyw->func = func;
    keyw->eval = eval;
    keyw->next = keywords;
    keywords = keyw;
}

void
Cond_Init(char *(*VarFunc)(const char *, Boolean *, int *),
	  char *(*ValueFunc)(const char *),
	  void (*ErrorFunc)(const char *, ...),
	  Boolean debug)
{
    CondVarParse = VarFunc;
    CondVarValue = ValueFunc;
    CondError = ErrorFunc;
    CondDebug = debug;

    Cond_AddKeyword("defined", CondKWDefined, CondDoDefined);
    Cond_AddKeyword("empty", CondKWEmpty, CondDoEmpty);
}
