static char sccsid[] = "@(#)46 1.5 src/bos/usr/ccs/lib/libl/reject.c, libl, bos411, 9428A410j 8/12/91 14:41:26";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: yyracc, yyreject
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

        /* Multi-byte support added by Michael S. Flegel, August 1991 */

# include <stdio.h>
# include <stddef.h>

extern FILE     *yyout, *yyin;
extern int      yyprevious , *yyfnd;
extern char     yyextra[];
extern wchar_t  yywtext[];
extern int      yywleng;
extern struct {
    int *yyaa, *yybb;
    int *yystops;
} *yylstate [], **yylsp, **yyolsp;

yyreject ()
{
    for( ; yylsp < yyolsp; yylsp++)
        yywtext[yywleng++] = yymbinput();
    if (*yyfnd > 0)
        return(yymbreturn(yyracc(*yyfnd++)));
    while (yylsp-- > yylstate)
    {
        yymbunput(yywtext[yywleng-1]);
        yywtext[--yywleng] = 0;
        if (*yylsp != 0 && (yyfnd= (*yylsp)->yystops) && *yyfnd > 0)
            return(yymbreturn(yyracc(*yyfnd++)));
    }
    if (yywtext[0] == 0)
        return(yymbreturn(0));
    yymboutput(yyprevious = yymbinput());
    yywleng=0;
    return(yymbreturn(-1));
}

yyracc(m)
  int   m;
{
    yyolsp = yylsp;
    if (yyextra[m])
    {
        while (yyback((*yylsp)->yystops, -m) != 1 && yylsp>yylstate)
        {
            yylsp--;
            yymbunput(yywtext[--yywleng]);
        }
    }
    yyprevious = yywtext[yywleng-1];
    yywtext[yywleng] = 0;
    return(m);
}
