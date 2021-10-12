static char sccsid[] = "@(#)42 1.5 src/bos/usr/ccs/lib/libl/allprint.c, libl, bos411, 9428A410j 4/14/94 12:56:47";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: allprint, printable, sprint
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_COPYRIGHT@
*/
# define _ILS_MACROS    /* 139785 - use macros for better performance */
# include <sys/localedef.h>
# include <stdio.h>
# include <ctype.h>
allprint(c)
wchar_t c;
{
extern FILE *yyout;

    switch(c)
    {
    case '\n':
        fprintf(yyout,"\\n");
        break;
    case '\t':
        fprintf(yyout,"\\t");
        break;
    case '\b':
        fprintf(yyout,"\\b");
        break;
    case ' ':
        fprintf(yyout,"\\\bb");
        break;
    default:
        if(!printable(c))
        {
            if (c<=0xFF)
                fprintf(yyout, "\\%-3o",c);
            else
                fprintf(yyout, "\\%-6o",c);
        }
        else
            putwc(c,yyout);
        break;
    }
    return;
}
sprint(s)
wchar_t *s;
{
    while(*s)
        allprint(*s++);
    return;
}
printable(c)
int c;
{
    return(iswprint(c));
}
