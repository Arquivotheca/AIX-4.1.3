/* @(#)32	1.1  src/bos/usr/lib/nls/loc/CN.im/cnimprof1.y, ils-zh_CN, bos41B, 9504A 12/19/94 14:34:22
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: list
 *		yyerror
 *		yylex
 *		yyparse
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*-----------------------------------------------------------------------*
*       parser
*
* defines syntax for JIM profile keywords.
* tokens in valid syntax will be stored token list via
* makeword() functions.
* otherwise, all information retrieved will be ignored
* by informing the caller of error
*-----------------------------------------------------------------------*/
%{
#include <sys/types.h>
#include "chinese.h"
#define YYSTYPE caddr_t
#define MAXWORDSIZE     256
extern int makewordtree();
extern void destroywordtree();
extern int nextchar();
extern void pushback();
%}
%token CHARSTR
%left '='
%%
list:
    | list '\n'
    | list CHARSTR ':' CHARSTR '\n' {if(makewordtree($2, $4) == 1)
                                        return(1);}
    ;
%%

/*-----------------------------------------------------------------------*
*       lexical analyzer
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
        while((c = nextchar()) != '\n' || c == EOF);
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
        profile_offset--;
        ccount = 0;
        while(isalnum(c = nextchar()) || c == '-' || c == '_') {
            ccount++;
            if(ccount > MAXWORDSIZE)
                return(0);
            buf[ccount - 1] = (char)c;
        }
        profile_offset--;
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
*       error handler does nothing
*-----------------------------------------------------------------------*/
yyerror(s)
char *s;
{
}
