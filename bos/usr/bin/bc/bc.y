%{static char sccsid[] = "@(#)32	1.34  src/bos/usr/bin/bc/bc.y, cmdcalc, bos41J, 9521B_all 5/28/95 11:15:22"; %}
/*
 * COMPONENT_NAME: (CMDCALC) calculators
 *
 * FUNCTIONS: bc 
 *
 * ORIGINS: 3 26 27 71
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

/*
*
*  NAME: bc [-c][-l][file ...]
*                                                                     
*  FUNCTION: Provides an interpreter for a arbitrary-precision arithmetic
* 	      language.
* 	OPTIONS:
*	-c    Compiles file, but does not invoke dc.
*      -l    includes a library of math functions.
*
*  COMMENTS: This is a yacc file.  It should first be given to the
*	      yacc command, which will produce an LR(1) parsing
*	      program called y.tab.c.  This file must then be compiled
*	      with the cc command.
*            The yacc grammer file is divided into three sections,
*	      each of which is delimited by %%.  These sections are:
*		   1)  Declarations:  Defines the variables which are
*		       used in the rules. 
*		   2)  rules:  Contains strings and expressions to be
*		       matched in the file to yylex(provided in this
*		       program), and C commands to execute when a match
*		       is made. 
*		   3)  Programs:  Allows the user to define his/her 
*                      own subroutines.
*
*/

/*  "%{" and "%}" are used to enclose global variables. */
%{
	static void getout(int);
	static int bundle( int, ... );
	extern int optind;
	extern int getopt(int argc, char **argv, char *optstring);
%}

/*  The following declarations give the associativity and precedence of
 *  the operators.  The later the operators appear, the higher precedence
 *  they have.  For example, "+" is left-associative and is of lower 
 *  precedence than "*". */ 
%right '='
%left '+' '-'
%left '*' '/' '%'
%right '^'
%left UMINUS

%term LETTER DIGIT SQRT LENGTH _IF  FFF EQ
%term _WHILE _FOR NE LE GE INCR DECR
%term _RETURN _BREAK _DEFINE BASE OBASE SCALE
%term EQPL EQMI EQMUL EQDIV EQREM EQEXP
%term _AUTO DOT
%term QSTR

%{
#include <stdio.h>
#include <locale.h>
#include <stdarg.h> /*for osf. Used for vproducing variable argument list*/
#include <nl_types.h>
#include <langinfo.h>/*needed to define radix char*/
#include <limits.h> 
static nl_catd catd;                   /* global catalog descriptor */
#include "bc_msg.h"
#define MSGSTR(C,D)             catgets(catd, MS_BC, C, D)
#define	MATHPATH	"/usr/lib/lib.b"
#define MAXIBASE	16
#define MINBASE		2
static FILE *in;
static char cary[BC_STRING_MAX+1], *cp = { cary };
static char string[BC_STRING_MAX+1], *str = {string};
static int base = 10;
static int obase = 10;
static int crs = '0';
static int rcrs = '0';  /* reset crs */
static int bindx = 0;
static int lev = 0;
static int ln;
static char *ss;
static int bstack[10] = { 0 };
static char *numb[15] = {
  " 0", " 1", " 2", " 3", " 4", " 5",
  " 6", " 7", " 8", " 9", " 10", " 11",
  " 12", " 13", " 14" };
static int *pre, *post;
%}

/*  The Rules section. */
/*  This section is made up of grammer rules and actions.  The actions are
 *  performed each time the parser recognizes the rule in the input 
 *  stream.  For example, in the first rule below, start is a non-terminal
 *  name and "start stat tail" are rules.  If one of these rules are
 *  recognized, then the action "output( $2 )" is performed, where "$2"
 *  represents the value returned by the second rule("stat"). */
%%
start	: 
	|  start stat tail
		= output( $2 );
	|  start def dargs ')' '{' dlist slist '}'
		={	bundle( 6,pre, $7, post ,"0",numb[lev],"Q");
			conout( $$, $2 );
			rcrs = crs;
			output( "" );
			lev = bindx = 0;
			}
	;

dlist	:  tail
	| dlist _AUTO dlets tail
	;

stat	:  e 
		={ bundle(3, $1, "ps", dot); }
	| 
		={ bundle(1, "" ); }
	|  QSTR
		={ bundle(3,"[",$1,"]P");}
	|  LETTER '=' e
		={ bundle(3, $3, "s", $1 ); }
	|  LETTER '[' e ']' '=' e
		={ bundle(4, $6, $3, ":", geta($1)); }
	|  LETTER EQOP e
		={ bundle(6, "l", $1, $3, $2, "s", $1 ); }
	|  LETTER '[' e ']' EQOP e
		={ bundle(8,$3, ";", geta($1), $6, $5, $3, ":", geta($1));}
	|  _BREAK
		={ bundle(2, numb[lev-bstack[bindx-1]], "Q" ); }
	|  _RETURN '(' e ')'
		= bundle(4, $3, post, numb[lev], "Q" );
	|  _RETURN '(' ')'
		= bundle(4, "0", post, numb[lev], "Q" );
	| _RETURN
		= bundle(4,"0",post,numb[lev],"Q");
	| SCALE '=' e
		= bundle(2, $3, "k");
	| SCALE EQOP e
		= bundle(4,"K",$3,$2,"k");
	| BASE '=' e
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) > MAXIBASE) 
			yyerror(MSGSTR(BADIBASE, "Input base value cannot exceed 16"));
		    else if (newbase < MINBASE) 
			yyerror(MSGSTR(MINIBASE, "Input base value cannot be less than 2"));
		    else {
			base = newbase;
		 	bundle(2,$3, "i");
		    }
		  }
	| BASE EQOP e
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) > MAXIBASE) 
			yyerror(MSGSTR(BADIBASE, "Input base value cannot exceed 16"));
		    else if (newbase < MINBASE) 
			yyerror(MSGSTR(MINIBASE, "Input base value cannot be less than 2"));
		    else {
			base = newbase;
		 	bundle(4,"I",$3,$2,"i");
		    }
		  }
	| OBASE '=' e
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) < MINBASE) 
			yyerror(MSGSTR(MINOBASE, "Output base value cannot be less than 2"));
		    else {
			obase = newbase;
		 	bundle(2,$3, "o");
		    }
		  }
	| OBASE EQOP e
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) < MINBASE) 
			yyerror(MSGSTR(MINOBASE, "Output base value cannot be less than 2"));
		    else {
			obase = newbase;
			bundle(4,"O",$3,$2,"o");
		    }
		  }
	|  '{' slist '}'
		={ $$ = $2; }
	|  FFF
		={ bundle(1,"fY"); }
	|  error
		={ bundle(1,"c"); }
	|  _IF CRS BLEV '(' re ')' stat
		={	conout( $7, $2 );
			bundle(3, $5, $2, " " );
			}
	|  _WHILE CRS '(' re ')' stat BLEV
		={	bundle(3, $6, $4, $2 );
			conout( $$, $2 );
			bundle(3, $4, $2, " " );
			}
	|  fprefix CRS re ';' e ')' stat BLEV
		={	bundle(6, $7, $5, "s", dot, $3, $2 );
			conout( $$, $2 );
			bundle(6, $1, "s", dot, $3, $2, " " );
			}
	|  '~' LETTER '=' e
		={	bundle(3,$4,"S",$2); }
	;

EQOP	:  EQPL
		={ $$ = (int)"+"; }
	|  EQMI
		={ $$ = (int)"-"; }
	|  EQMUL
		={ $$ = (int)"*"; }
	|  EQDIV
		={ $$ = (int)"/"; }
	|  EQREM
		={ $$ = (int)"%"; }
	|  EQEXP
		={ $$ = (int)"^"; }
	;

fprefix	:  _FOR '(' e ';'
		={ $$ = $3; }
	;

BLEV	:
		={ --bindx; }
	;

slist	:  stat
	|  slist tail stat
		={ bundle(2, $1, $3 ); }
	;

tail	:  '\n'
		={ln++;}
	|  ';'
	;

re	:  e EQ e
		= bundle(3, $1, $3, "=" );
	|  e '<' e
		= bundle(3, $1, $3, ">" );
	|  e '>' e
		= bundle(3, $1, $3, "<" );
	|  e NE e
		= bundle(3, $1, $3, "!=" );
	|  e GE e
		= bundle(3, $1, $3, "!>" );
	|  e LE e
		= bundle(3, $1, $3, "!<" );
	|  e
		= bundle(2, $1, " 0!=" );
	;

e	:  e '+' e
		= bundle(3, $1, $3, "+" );
	|  e '-' e
		= bundle(3, $1, $3, "-" );
	| '-' e		%prec UMINUS
		= bundle(3, " 0", $2, "-" );
	|  e '*' e
		= bundle(3, $1, $3, "*" );
	|  e '/' e
		= bundle(3, $1, $3, "/" );
	|  e '%' e
		= bundle(3, $1, $3, "%" );
	|  e '^' e
		= bundle(3, $1, $3, "^" );
	|  LETTER '[' e ']'
		={ bundle(3,$3, ";", geta($1)); }
	|  LETTER INCR
		= bundle(4, "l", $1, "d1+s", $1 );
	|  INCR LETTER
		= bundle(4, "l", $2, "1+ds", $2 );
	|  DECR LETTER
		= bundle(4, "l", $2, "1-ds", $2 );
	|  LETTER DECR
		= bundle(4, "l", $1, "d1-s", $1 );
	| LETTER '[' e ']' INCR
		= bundle(7,$3,";",geta($1),"d1+",$3,":",geta($1));
	| INCR LETTER '[' e ']'
		= bundle(7,$4,";",geta($2),"1+d",$4,":",geta($2));
	| LETTER '[' e ']' DECR
		= bundle(7,$3,";",geta($1),"d1-",$3,":",geta($1));
	| DECR LETTER '[' e ']'
		= bundle(7,$4,";",geta($2),"1-d",$4,":",geta($2));
	| SCALE INCR
		= bundle(1,"Kd1+k");
	| INCR SCALE
		= bundle(1,"K1+dk");
	| SCALE DECR
		= bundle(1,"Kd1-k");
	| DECR SCALE
		= bundle(1,"K1-dk");
	| BASE INCR
		= { if (base < MAXIBASE) {
			base++;
		    	bundle(1,"Id1+i");
		    } else {
			yyerror(MSGSTR(BADIBASE, "Input base value cannot exceed 16"));
		    	bundle(1,"I");
		    }
		  }
	| INCR BASE
		= { if ( base < MAXIBASE ) {
			base++;
		 	bundle(1,"I1+di");
		    } else {
			yyerror(MSGSTR(BADIBASE, "Input base value cannot exceed 16"));
		    	bundle(1,"I");
		    }
		  }
	| BASE DECR
		= { if ( base > MINBASE ) {
			base--;
			bundle(1,"Id1-i");
		    } else {
			yyerror(MSGSTR(MINIBASE, "Input base value cannot be less than 2"));
		    	bundle(1,"I");
		    }
		  }
	| DECR BASE
		= { if ( base > MINBASE ) {
			base--;
			bundle(1,"I1-di");
		    } else {
			yyerror(MSGSTR(MINIBASE, "Input base value cannot be less than 2"));
		    	bundle(1,"I");
		    }
		  }
	| OBASE INCR
		= bundle(1,"Od1+o");
	| INCR OBASE
		= bundle(1,"O1+do");
	| OBASE DECR
		= { if ( obase > MINBASE ) {
			obase--;
			bundle(1,"Od1-o");
		    } else {
			yyerror(MSGSTR(MINOBASE, "Output base value cannot be less than 2"));
		    	bundle(1,"O");
		    }
		  }
	| DECR OBASE
		= { if ( obase > MINBASE ) {
			obase--;
			bundle(1,"O1-do");
		    } else {
			yyerror(MSGSTR(MINOBASE, "Output base value cannot be less than 2"));
		    	bundle(1,"O");
		    }
		  }
	|  LETTER '(' cargs ')'
		= bundle(4, $3, "l", getf($1), "x" );
	|  LETTER '(' ')'
		= bundle(3, "l", getf($1), "x" );
	|  cons
		={ bundle(2, " ", $1 ); }
	|  DOT cons
		={ bundle(3," ", dot, $2 ); }
	|  cons DOT cons
		={ bundle(4, " ", $1, dot , $3 ); }
	|  cons DOT
		={ bundle(3, " ", $1, dot ); }
	|  DOT
		={ bundle(2, "l", dot ); }
	|  LETTER
		= { bundle(2, "l", $1 ); }
	|  LETTER '=' e
		={ bundle(3, $3, "ds", $1 ); }
	|  LETTER EQOP e	%prec '='
		={ bundle(6, "l", $1, $3, $2, "ds", $1 ); }
	| LETTER '[' e ']' '=' e
		= { bundle(5,$6,"d",$3,":",geta($1)); }
	| LETTER '[' e ']' EQOP e
		= { bundle(9,$3,";",geta($1),$6,$5,"d",$3,":",geta($1)); }
	| LENGTH '(' e ')'
		= bundle(2,$3,"Z");
	| SCALE '(' e ')'
		= bundle(2,$3,"X");	/* must be before '(' e ')' */
	|  '(' e ')'
		= { $$ = $2; }
	|  '?'
		={ bundle(1, "?" ); }
	|  SQRT '(' e ')'
		={ bundle(2, $3, "v" ); }
	| '~' LETTER
		={ bundle(2,"L",$2); }
	| SCALE '=' e
		= bundle(2,$3,"dk");
	| SCALE EQOP e		%prec '='
		= bundle(4,"K",$3,$2,"dk");
	| BASE '=' e
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) > MAXIBASE) 
			yyerror(MSGSTR(BADIBASE, "Input base value cannot exceed 16"));
		    else if (newbase < MINBASE) 
			yyerror(MSGSTR(MINIBASE, "Input base value cannot be less than 2"));
		    else {
			base = newbase;
		 	bundle(2,$3,"di");
		    }
		  }
	| BASE EQOP e		%prec '='
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) > MAXIBASE) 
			yyerror(MSGSTR(BADIBASE, "Input base value cannot exceed 16"));
		    else if (newbase < MINBASE) 
			yyerror(MSGSTR(MINIBASE, "Input base value cannot be less than 2"));
		    else {
			base = newbase;
		 	bundle(4,"I",$3,$2,"di");
		    }
		  }
	| OBASE '=' e
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) < MINBASE) 
			yyerror(MSGSTR(MINOBASE, "Output base value cannot be less than 2"));
		    else {
			obase = newbase;
		 	bundle(2,$3,"do");
		    }
		  }
	| OBASE EQOP e		%prec '='
		= { int newbase;
		    if ((newbase = strtol(cary, "", (strlen(cary) > 1 ? base : MAXIBASE))) < MINBASE) 
			yyerror(MSGSTR(MINOBASE, "Output base value cannot be less than 2"));
		    else {
			obase = newbase;
		 	bundle(4,"O",$3,$2,"do");
		    }
		  }
	| SCALE
		= bundle(1,"K");
	| BASE
		= bundle(1,"I");
	| OBASE
		= bundle(1,"O");
	;

cargs	:  eora
	|  cargs ',' eora
		= bundle(2, $1, $3 );
	;
eora:	  e
	| LETTER '[' ']'
		=bundle(2,"l",geta($1));
	;

cons	:  constant
		={ *cp++ = '\0'; }

constant:
	  '_'
		={ $$ = (int)cp; *cp++ = '_'; }
	|  DIGIT
		={ $$ = (int)cp; *cp++ = $1; }
	|  constant DIGIT
		={ *cp++ = $2; }
	;

CRS	:
		={ $$ = (int)cp; *cp++ = crs++; *cp++ = '\0';
			if(crs == '[')crs+=3;
			if(crs == 'a')crs='{';
			if(crs >= 0241) {
			    yyerror(MSGSTR(TOOBIG,
			       "bc: program too big"));
				getout(1);
			}
			bstack[bindx++] = lev++; }
	;

def	:  _DEFINE LETTER '('
		={	$$ = (int)getf($2);
			pre = (int *)"";
			post = (int *)"";
			lev = 1;
			bstack[bindx=0] = 0;
			}
	;

dargs	:
	|  lora
		={ pp( $1 ); }
	|  dargs ',' lora
		={ pp( $3 ); }
	;

dlets	:  lora
		={ tp($1); }
	|  dlets ',' lora
		={ tp($3); }
	;
lora	:  LETTER
	|  LETTER '[' ']'
		={ $$ = (int)geta($1); }
	;

/*  The Program section. */
/*  This section contains the C language programs which perform the functions
 *  used by the actions in the rules section. */
%%
# define error 256


static int peekc = -1;
static int sargc;
static int ifile = 0;
static char **sargv;

/* The 'dc' command allows any character to be used as a register name.
   The 'bc' command uses characters for specific purposes.

   (See the yacc rule for CRS to see how names for control statements
   are generated.)

   0			Not used
   01-032		Function names
   033-047(')
   048('0')-0132('Z')	Control (if, while, ...) statements
   0133('[')-0135(']')
   0136('^')-0140('`')	Control statements
   0141('a')-0172('z')	Simple variables
   0173('{')-0240	Control statements
   0241-0272		Array names
   0273-0377

*/

/* Function names [a-z] are mapped to 'dc' register names 01-032.
   This range must be distinct from the range used for array names
   and simple variable names. */
#define x(x) {x-0140,'\0'}
static char funtab[26][2] = {
	x('a'),x('b'),x('c'),x('d'),x('e'),x('f'),x('g'),x('h'),x('i'),
	x('j'),x('k'),x('l'),x('m'),x('n'),x('o'),x('p'),x('q'),x('r'),
	x('s'),x('t'),x('u'),x('v'),x('w'),x('x'),x('y'),x('z')};
/* Array names [a-z] are mapped to 'dc' register names 0241-0272.
   This range must be distinct from the range used for function names
   and simple variable names. */
#undef x
#define x(x) {x+0100,'\0'}
static char atab[26][2] = {
	x('a'),x('b'),x('c'),x('d'),x('e'),x('f'),x('g'),x('h'),x('i'),
	x('j'),x('k'),x('l'),x('m'),x('n'),x('o'),x('p'),x('q'),x('r'),
	x('s'),x('t'),x('u'),x('v'),x('w'),x('x'),x('y'),x('z')};
#undef x
/* Simple variable names are mapped to themselves.
   Array names and function names cannot map to themselves. */
static char *letr[26] = {
  "a","b","c","d","e","f","g","h","i","j",
  "k","l","m","n","o","p","q","r","s","t",
  "u","v","w","x","y","z" } ;
static char *dot; /*This character is used for the decimal(radix) character*/


/*
 *  NAME:  yylex 
 *
 *  FUNCTION:  This is the lexical analyzer.  It reads the input
 *	       stream and sends tokens (with values, if required)
 *	       to the parser that yacc generates. 
 *
 *  RETURN VALUE:  Returns an integer (called a token number) which 
 * 		   represents the kind of token that was read.
 *
 */
yylex(){
	int c, ch;

    while(1) {
	c = getch();
	peekc = -1;
	while( c == ' ' || c == '\t' ) c = getch();
	if(c == '\\'){
		int chr = getch();
		/* The following code supports multi-line number tokens   */
		/* If you encounter a backslash followed by a newline and */
		/* The previous character was a digit and the next charac */
		/* is a digit, then ignore the backslash and the newline. */
		if ( chr == '\n' ) {
			chr = getch();
			if( chr>= '0' && chr <= '9' || chr>= 'A' && chr<= 'F' ){
				yylval = chr;
				return( DIGIT );
			} else
				c = chr;
		}
	}
	if( c<= 'z' && c >= 'a' ) {
		/* look ahead for reserved words */
		peekc = getch();
		if( peekc >= 'a' && peekc <= 'z' ){ /* must be reserved word */
			if( c=='i' && peekc=='f' ){ c=_IF; }
			else if( c=='w' && peekc=='h' ){ c=_WHILE; }
			else if( c=='f' && peekc=='o' ){ c=_FOR; }
			else if( c=='s' && peekc=='q' ){ c=SQRT; }
			else if( c=='r' && peekc=='e' ){ c=_RETURN; }
			else if( c=='b' && peekc=='r' ){ c=_BREAK; }
			else if( c=='d' && peekc=='e' ){ c=_DEFINE; }
		 	else if( c=='s' && peekc=='c' ){ c= SCALE; }
			else if( c=='b' && peekc=='a' ){ c=BASE; }
			else if( c=='i' && peekc == 'b'){ c=BASE; }
			else if( c=='o' && peekc=='b' ){ c=OBASE; }
			else if( c=='d' && peekc=='i' ){ c=FFF; }
			else if( c=='a' && peekc=='u' ){ c=_AUTO; }
			else if( c == 'l' && peekc=='e'){ c=LENGTH; }
			else if( c == 'q' && peekc == 'u'){getout(0);}
			/* could not be found */
			else return( error );
			/* skip over rest of word. */
			peekc = -1;
			while( (ch = getch()) >= 'a' && ch <= 'z' );
			peekc = ch;
			return( c );
		}

		/* usual case; single letter */
		yylval = (int)letr[c-'a'];
		return( LETTER );
	}
	if( c>= '0' && c <= '9' || c>= 'A' && c<= 'F' ){
		yylval = c;
		return( DIGIT );
	}
	/* Parse out the operators. */
	switch( c ){
	case '=':	
		switch( peekc = getch() ){
			case '=': c=EQ; peekc = -1; return(c);
			case '+': c=EQPL; peekc = -1; return(c);
			case '-': c=EQMI; peekc = -1; return(c);
			case '*': c=EQMUL; peekc = -1; return(c);
			case '/': c=EQDIV; peekc = -1; return(c);
			case '%': c=EQREM; peekc = -1; return(c);
			case '^': c=EQEXP; peekc = -1; return(c);
			default:   return( '=' );
		}
	case '+':	return( cpeek2( '+', INCR, '=', EQPL, '+' ) );
	case '-':	return( cpeek2( '-', DECR, '=', EQMI, '-' ) );
	case '<':	return( cpeek( '=', LE, '<' ) );
	case '>':	return( cpeek( '=', GE, '>' ) );
	case '!':	return( cpeek( '=', NE, '!' ) );
	case '%':	return( cpeek( '=', EQREM, '%' ) );
	case '^':	return( cpeek( '=', EQEXP, '^' ) );
	case '*':	return( cpeek( '=', EQMUL, '*' ) );
	case '/':
		if((peekc = getch()) == '*'){
			peekc = -1;
			while((getch() != '*') || ((peekc = getch()) != '/'));
			peekc = -1;
			break;
		} else if (peekc == '='){
					peekc=-1;
					c=EQDIV;
			}
			return(c);
	case '"':	/* If a string is encountered, then read it in until the
		  * second set of double quotes is found.  */
		 yylval = (int)str;
		 while((c=getch()) != '"') {
			*str++ = c;
			if (str > &string[BC_STRING_MAX]) {
			   yyerror(MSGSTR(NOSTRSPC,
			  	 "bc:string space exceeded"));
		        }
	         }
	         *str++ = '\0';
	         return(QSTR);
	default:	 
		if (c == *dot)
			return( DOT );
		else
			return( c );
	}
    }
}



/*
 *  NAME:  cpeek
 *
 *  FUNCTION:  This function is used to parse operators which are made up
 *		of more than one character.  For example, a call to this
 *		function might be "cpeek( "=", LE, "<" )".  When this call
 *		is made, it has already been determined that a "<" sign
 *		has been found.  This function says that if the next 
 *		character is an equal sign, then the operator is "<=".
 *		If the next character is not an equal sign, the the 
 *		operator is simply "<". 
 *
 *  RETURN VALUE:  yes)  The next character read is the same as the first
 *				arguement. 
 *		   no)   The next character read is not the same as the    
 *				first arguement.
 *
 */
static cpeek( c, yes, no ){
	if( (peekc=getch()) != c ) return( no );
	else {
		peekc = -1;
		return( yes );
	}
}

/*
 *  NAME:  cpeek2
 *
 *  FUNCTION:  This function is used to parse operators which are made up
 *		of more than one character.  This function determines whether there
 *      is a two character operator, what the value of the second character
 *      is and returns the rule which corresponds to the two character
 *      operator or decrements the character counter and returns
 *      the original character.
 *  RETURN VALUE:  return1)  The first specified rule.
 *		   return2)   The second specified rule.
 *		   default)   The next character is not part of the operator.
 *
 */
 static cpeek2( next1,return1,next2,return2,orig) {

	if( cpeek(next1,return1,orig)==return1)
		return(return1);
	else if(peekc == next2){
				peekc = -1;
				return(return2);
		}
	else 
		return(orig);
}

/*
 *  NAME:  getch  
 *
 *  FUNCTION: Reads in a character. 
 *
 *  RETURN VALUE:  The character read in.
 *
 */
static getch(){
	int ch;
	int cont=1;

	while(cont) {
		ch = (peekc < 0) ? getc(in) : peekc;
		peekc = -1;
		if(ch != EOF)return(ch);
		
		++ifile;
		if (ifile <= sargc+1) {
			fclose(in);
			if(sargv[ifile] == NULL){
				in = stdin;
				ss = "stdin";
				ln = 0;
			} else if((in = fopen(sargv[ifile],"r")) != NULL) {
				ln = 0;
				ss = sargv[ifile];
			} else {
				fprintf(stderr, "%s %s\n", MSGSTR(NOINFILE,
					"bc: cannot open input file"),
					sargv[ifile]);
				cont = 0;
			}
		} else
			getout(0);
	}
}

/*
 *  NAME:  bundle
 *
 *  FUNCTION:  Stores the arguements into the bundling space. 
 *
 *  RETURN VALUE:  The location in the bundling space where the args 
 *			are stored. Allows for variable length arg list.
 *
 */
# define b_sp_max 9000 /* must be this large to handle worst case expression */
                       /* containing 1+1+1+... up to LINE_MAX bytes */
static int b_space [ b_sp_max ];
static int * b_sp_nxt = { b_space };

static int	bdebug = 0;


bundle(int i,...){
  	va_list ap;

	int *q;

  	va_start(ap, i);

	q = b_sp_nxt;
	if( bdebug ) fprintf(stderr, MSGSTR(BUND, "bundle %d elements at %x\n"),i,  q );
	while(i-- > 0){
		if( b_sp_nxt >= & b_space[b_sp_max] )
			yyerror(MSGSTR(NOBUNDLE,
			   "bc: bundling space exceeded"));
		* b_sp_nxt++ = va_arg(ap, int);
	}
	* b_sp_nxt++ = 0;
	yyval = (int)q;
	return( (int)q );
}

/*
 *  NAME:  routput
 *
 *  FUNCTION:  This is a recursive function.  It prints the contents of the
 *		bundling space from "p" until the end.
 *
 *  RETURN VALUE:  none
 *
 */
static routput(p) int *p; {
	if( bdebug ) fprintf(stderr, MSGSTR(ROUT, "routput(%o)\n"), p );
	if( p >= &b_space[0] && p < &b_space[b_sp_max]){
		/* part of a bundle */
		while( *p != 0 ) routput( *p++ );
	}
	else printf("%s",p );	 /* character string */
}

/*
 *  NAME:  output
 *
 *  FUNCTION:  Prints out the contents of the budling space from "*p" on.
 *
 *  RETURN VALUE: none
 *
 */
static output( p ) int *p; {
	routput( p );
	b_sp_nxt = & b_space[0];
	printf( "\n" );
	fflush(stdout);
	cp = cary;
	crs = rcrs;
}

static conout( p, s ) int *p; char *s; {
	printf("[");
	routput( p );
	printf("]s%s\n", s );
	fflush(stdout);
	lev--;
}

/*
 *  NAME:  yyerror
 *
 *  FUNCTION:  Handles errors which occur during parser operation.
 *
 *  RETURN VALUE:  none
 *
 */
yyerror( s ) char *s; {
	char errmsg[256];

	sprintf(errmsg, MSGSTR(INVINP, "%s on line %d %s"), s, ln+1, ss );
	fprintf(stderr, "%s\n", errmsg );
	fflush(stderr);
	cp = cary;
	crs = rcrs;
	bindx = 0;
	lev = 0;
	b_sp_nxt = &b_space[0];
}

/*
 *  NAME:  pp
 *
 *  FUNCTION: Puts the relevant stuff on pre and post for the letter s.
 *
 *  RETURN VALUE:  none
 *
 */
static pp( s ) char *s; {
	bundle(3, "S", s, pre );
	pre = (int *)yyval;
	bundle(5, post, "L", s, "s", dot );
	post = (int *)yyval;
}

/*
 *  NAME:  tp 
 *
 *  FUNCTION:  Same as pp, but for temps.
 *
 *  RETURN VALUE:  none
 *
 */
static tp( s ) char *s; { 
	bundle(3, "0S", s, pre );
	pre = (int *)yyval;
	bundle(5, post, "L", s, "s", dot );
	post = (int *)yyval;
}



/*
 *  NAME:  getout
 *
 *  FUNCTION:  exits the program.
 *
 *  RETURN VALUE: none 
 *
 */
static void getout(int stat){
	printf("q");
	fflush(stdout);
	exit(stat);
}

static int *
getf(p) char *p;{
	return((int *)&funtab[(*p - 'a')]);
}

static int *
geta(p) char *p;{
	return((int *)&atab[(*p - 'a')]);
}


/*
 *  NAME:  main
 *
 *  FUNCTION:  Parses out the options which are used to call bc.
 *
 *  RETURN VALUE:  none
 *
 */
main(argc, argv)
char **argv;
{
	int p[2];
	int opt;
	unsigned short cflag=0;
	unsigned short sargv_indx = 0;
	pid_t pid;


	(void)setlocale(LC_ALL, "");
	(void)setlocale(LC_NUMERIC, "C");
	/*define radix */
	dot = "."; 
	/*open message catalogue*/
	catd = catopen(MF_BC, NL_CAT_LOCALE);

	/* Takes care of any specified options.  These can be either
	 * "c" for binary calculator ('d' for backward compatibility)
	 * or "l" to include a library. Check here for readable file
	 * name on input line. This is a bit faster than waiting until 
	 * after the call to dc and eliminates a core dump caused due
	 * to the fork/execl. Also values greater than 0 are returned
	 * on error conditions.*/
	while ((opt = getopt(argc, argv, "cdl")) != EOF) {
		switch(opt) {
		case 'c':
		case 'd':
			cflag=1;
			break;
		case 'l':
			ss = MATHPATH;
			if((in = fopen(MATHPATH,"r")) == NULL)
		     		fprintf(stderr, "%s %s\n", MSGSTR(NOINFILE, "bc: cannot open input file"), MATHPATH);
			break;
		case '?':
			fprintf(stderr, MSGSTR(USAGE,"usage:bc [-cl] [files...]\n"));
			fflush(stderr);
			exit(1);
			break;
		}
	}

	/* XPG4 says all files on the command line must be accessible
	   or we exit without taking an action. */
	{
		int	filename_index;
		FILE	*file_stream;

		/* Loop through the file names. */
		for(filename_index=optind; argv[filename_index] !=
				NULL; filename_index++)
		{
			/* If we can't read the file then exit non-zero. */
			if ((file_stream = fopen(argv[filename_index],"r")) == NULL)
			{
				fprintf(stderr, "%s %s\n", MSGSTR(NOINFILE, "bc: cannot open input file"), argv[filename_index]);
				exit(1);
			}

			/* Close the file and keep going. */
			(void)fclose(file_stream);
		}
	}

	sargc = argc - optind;	/* the number of files on command line */
	sargv = argv + optind - 1;

	if ((in == NULL) && (argv[optind] != NULL)) {
		if((in = fopen(argv[optind],"r")) == NULL) {
		     fprintf(stderr, "%s %s\n", MSGSTR(NOINFILE, "bc: cannot open input file"), argv[optind]);
		     in = stdin;
		     ss = "stdin";
		} else {
		     ++ifile;
		     ss = argv[optind];
		}
	}
	else if ( in == NULL ) {
		++ifile;
		in = stdin;
		ss = "stdin";
	}

	if (cflag == 1) {
		signal( 2, (int(*)())1 );	/* ignore all interrupts */
		ln = 0;
		yyparse();
		exit(0);
	}

	pipe(p);
	pid = fork();
	if (pid < 0) {
		perror("bc");
		exit(1);
	}
	if (pid == 0) {			/* The child process */
		close(1);
		dup(p[1]);
		close(p[0]);
		close(p[1]);
		signal( 2, (int(*)())1 );	/* ignore all interrupts */
		ln = 0;
		yyparse();
		exit(0);
	}
	close(0);
	dup(p[0]);
	close(p[0]);
	close(p[1]);
	catclose(catd);
	execl("/usr/bin/dc", "dc", "-", 0);
}
