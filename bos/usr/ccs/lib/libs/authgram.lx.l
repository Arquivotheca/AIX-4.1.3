%{
static char sccsid[] = "@(#)13	1.3  src/bos/usr/ccs/lib/libs/authgram.lx.l, libs, bos41J, 9515B_all 4/13/95 08:42:36";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _grammar_keyword
 *		yylex
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "y.tab.h"
#include <usersec.h>
/*
 * Redefine input() macro to accept input from a string previously
 * read from the security database.  getgrammarc() replaces getc()
 * in the standard definition.  If I do not do this all input will
 * be expected from stdin.
 *
 * The parser can't be taught NULL terminated strings so I will appease 
 * it with the newline character.
 */
char *_yyinstring;
int   _yyinptr;

#undef  input
#define getgrammarc()\
	 ((_yyinstring[_yyinptr] == '\0') ? '\n' : _yyinstring[_yyinptr++])
#define input()\
	 (((yytchar=yysptr>yysbuf?U(*--yysptr):getgrammarc())==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#undef  output
#define output(c)
#define yywrap	myyywrap

void yywoutput();
void yywunput();
%}

ws		[ \t]+
lbracket	"["
rbracket	"]"
id		[a-zA-Z0-9\.\*]*
nl		\n

%%
{ws}		;
{lbracket}	{ return (yylval.cmd = LBRACKET); }
{rbracket}	{ return (yylval.cmd = RBRACKET); }
{id}		{ 
		  char *convert; /* Correct upper or lowercase spelling */

		  /*
		   * Check if the string is a keyword.  If it is
	 	   * a keyword return the type.  Otherwise if it
		   * is NONE (not a keyword), malloc space for the
		   * string and return it as a method.
		   */ 
		  if ((yylval.cmd = _grammar_keyword(yytext, &convert)) != NONE)
			return (yylval.cmd);
		  else
		  {
			if((yylval.string = strdup(convert)) == (char *)NULL) 
				return(-1);
			return (METHOD);
		  }
		}
{nl}		{ return yytext[0]; }
%%
static struct keyword
{
	char *CAPNAME;
	char *name;	/* reserved word */
	int  value;	/* token */
} keywords[] = {
		"COMPAT",	AUTH_COMPAT,	NONE,
		"FILES",	AUTH_FILES,	NONE,
		"NIS",		AUTH_NIS,	NONE,
		"DCE",		AUTH_DCE,	NONE,
		"NONE",		AUTH_NONE_SEC,	NONE,
		"SUCCESS", 	"SUCCESS",	SUCCESS,
		"FAILURE",	"FAILURE",	FAILURE,
		"NOTFOUND",	"NOTFOUND",	NOTFOUND,
		"UNAVAIL",	"UNAVAIL",	UNAVAIL,
		"*",		"*",		WILDCARD,
		"AND",		"AND",		AND,
		"OR",		"OR",		OR,
		(char *)NULL,	(char *)NULL,	NONE,
     	       };
/*
 * NAME: _grammar_keyword	
 *
 * FUNCTION: Ascertains if the given string is a reserved word or a well
 *	     known method.  If the string is a reserved word it's grammar
 *	     value is returned.  Otherwise the value "NONE" is returned.
 *	     The correct form of the string (either capitalized or 
 *	     uncapitalized is returned in the second argument).
 *
 * RETURNS:  grammar keyword value, or the value NONE in the case of 
 *	     well known methods.
 */
int _grammar_keyword(char *string, char **token)
{
	struct keyword *ptr;	
	char *buf;
	char *val = string;
	int  i;

	/* Set up for return as unknown string */
	*token = string;

	/* Buffer for modifying capitalization format */
	if ((buf = (char *)strdup(string)) == (char *)NULL)
		return(-1);

	/* Convert string to uppercase for comparison */
	if (val)
	{
		for (i=0; *val; i++, val++)
			if (*val >= 'a' && *val <= 'z')
				buf[i] = *val - 'a' + 'A';

		buf[i] = '\0';
		val = buf;
	}
	for (ptr = keywords; ptr->CAPNAME; ptr++)
		if (!strcmp(ptr->CAPNAME, val))
		{
			*token = ptr->name;
			break;
		}

	free(buf);
	return(ptr->value);
}
static int
myyywrap()
{
	return(1);
}
