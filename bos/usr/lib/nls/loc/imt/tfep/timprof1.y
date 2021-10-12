/* @(#)27  1.3  src/bos/usr/lib/nls/loc/imt/tfep/timprof1.y, libtw, bos411, 9428A410j 4/21/94 02:32:04 */
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: yyerror
 *              yylex
 *              yyparse
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
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
#include "taiwan.h"
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
    if(isalnum(c) || c == '-' || c == '_' || c == '/') {
        profile_offset--;
        ccount = 0;
        while(isalnum(c = nextchar()) || c == '-' || c == '_' || c == '/') {
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
