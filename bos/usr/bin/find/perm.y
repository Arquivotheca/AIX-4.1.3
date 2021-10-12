/* @(#)12	1.3  src/bos/usr/bin/find/perm.y, cmdscan, bos412, 9445B412 11/8/94 13:35:33 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 *
 * rcsid[] = "@( # )$RCSfile: perm.y,v $  $Revision: 1.3.2.2 $  (OSF) $Date: 91/11/18 18:44:32 $"
 *
 */
%{

#include <sys/mode.h>
#include <sys/types.h>
#include <nl_types.h>
#include <stdio.h>
#include "find_msg.h"


extern nl_catd	catd;

static mode_t	permission, who;	/* computed permission bits */

#define	MSGSTR(Num, Str) catgets(catd, MS_FIND, Num, Str)
#define ALL	( S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID )
%}

%start		symbolic_mode
%%

symbolic_mode	: clause
  	       	| symbolic_mode ',' clause
  		;

clause		: { who = ALL; } actionlist
  		| wholist { who = $1; } actionlist
  		;

wholist		: who		{ $$ = $1; }
		| wholist who	{ $$ = $1 | $2; }
		;

who		: 'u'	{ $$ = S_IRWXU|S_ISUID; }
		| 'g'	{ $$ = S_IRWXG|S_ISGID; }
		| 'o'	{ $$ = S_IRWXO; }
		| 'a'	{ $$ = ALL; }
		;

actionlist	: action
		| actionlist action
		;

action		: operation		{ do_action($1,0); }
		| operation permlist	{ do_action($1,$2); }
		| operation permcopy	{ do_action($1,$2); }
		;

permcopy	: 'u'	{ mode_t t = (permission&S_IRWXU); $$ = (t>>3) | (t>>6); }
		| 'g'	{ mode_t t = (permission&S_IRWXG); $$ = (t<<3) | (t>>3); }
		| 'o'	{ mode_t t = (permission&S_IRWXO); $$ = (t<<3) | (t<<6); }
		;

operation	: '+'	{ $$ = '+'; }
		| '-'	{ $$ = '-'; }
		| '='	{ $$ = '='; }
		;

permlist	: perm			{ $$ = $1; }
		| perm permlist		{ $$ = $1 | $2; }
		;

perm		: 'r'	{ $$ = S_IRUSR | S_IRGRP | S_IROTH; }
		| 'w'	{ $$ = S_IWUSR | S_IWGRP | S_IWOTH; }
		| 'x'	{ $$ = S_IXUSR | S_IXGRP | S_IXOTH; }
		| 'X'	{ $$ = S_IXUSR | S_IXGRP | S_IXOTH; }
		| 's'	{ $$ = S_ISUID | S_ISGID; }
		;

%%

static void yyerror(char *s)
{
  fprintf( stderr, MSGSTR(BADPERM, "find: invalid -perm argument") );
  exit(2);
}

static const char *permstring;

static int yylex()
{
  if(!permstring || !*permstring) return EOF;

  return *permstring++;
}

static do_action(operation, perm)
char operation;
mode_t perm;
{
  switch(operation) {
	  case '+':	permission |= (perm & who);
			break;
	  case '-':	permission &= ~(perm & who);
			break;
	  case '=':	permission &= ~who;
			permission |= (perm & who);
			break;
  }
  return;
}

mode_t permissions( const char *perm )
{
  permstring = perm;

  permission = 0;

  yyparse();

  return permission;
}

