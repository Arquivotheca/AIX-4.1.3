/*static char *kp_y = "@(#)  Korean IM: 86 1.3 - 7/27/94 15:14:16";*/
/******************************************************************
 *
 *  Component:    Korean IM Profile 
 *
 *  Module:       kp.y
 *
 *  Description:  Korean Input Method profile parsing
 *
 *  History:      5/22/90  Initial Creation.     
 *
 *  ORIGINS :     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 ******************************************************************/
 
/*-----------------------------------------------------------------------*
*	parser 
*	
* defines syntax for KIM profile keywords.
* tokens in valid syntax will be stored token list via
* makeword() functions.
* otherwise, all information retrieved will be ignored
* by informing the caller of error
*-----------------------------------------------------------------------*/
%{
#include <sys/types.h>
#include "kimpro.h"
#define YYSTYPE caddr_t
extern int makeword();
extern void destroylist();
extern int nextchar();
extern void pushback();
%}
%token CHARSTR
%left '=' 
%%
list: 
    | list '\n' 
    | list CHARSTR ':' CHARSTR '\n' {if(makeword($2, $4) == KP_MAKEERR)
					return(1);}
    ;
%%

/*-----------------------------------------------------------------------*
*	lexical analyzer
*	
* on encountering token, this allocates space for such string,
* returning its address to parser.
* this is not responsible for freeing such allocated spaces, which
* will be freed somewhere else
* token can consists of alphanumeric characters, or
* undercore, hypen.
* not exceeding MAXWORDSIZE.
* comment line which begins with '#' (no restriction on its column position)
* or white space characters are removed within this function
*-----------------------------------------------------------------------*/
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
yylex()
{
    char buf[MAXWORDSIZE];
    int c;
    int ccount;
    char *word_ptr;

    /* skip over white spaces */
    while((c = nextchar()) == ' ' || c == '\t');

    /* ignore comment line */
    if(c == '#') {
	while((c = nextchar()) != '\n' && c != EOF);
	if(c == '\n')
	    return(c);
        else
	    return(0);
    }
    /* EOF reached ? */
    if(c == EOF)
	return(0);
	    
    /* token consists of alpha numeric character, underscore, hypen */
    /* comes here                                                   */
    if(isalnum(c) || c == '-' || c == '_') {
	pushback();
	ccount = 0;
	c = nextchar();
	while(isalnum(c) || c == '-' || c == '_') {
	    ccount++;
	    if(ccount > MAXWORDSIZE)
		return(0);
            buf[ccount - 1] = (char)c;
	    c = nextchar();
	}
	pushback();
	word_ptr = (char *)malloc(ccount + 1);
	memcpy(word_ptr, buf, ccount);
	word_ptr[ccount] = NULL;
	yylval = word_ptr;
	return(CHARSTR);
    }
    /* otherwise, return it as it is */
    return(c);
} /* end of yylex */

/*-----------------------------------------------------------------------*
*	error handler does nothing
*-----------------------------------------------------------------------*/
yyerror(s)
char *s;
{
}
