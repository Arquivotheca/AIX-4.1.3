static char sccsid[] = "@(#)47 1.5.1.1 src/bos/usr/ccs/lib/libl/yyless.c, libl, bos411, 9428A410j 2/15/93 09:45:40";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: yyless
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

# include <stddef.h>

yyless(x)
  int           x;
{
wchar_t         *lastch, *ptr;

extern wchar_t  yywtext[];
extern int      yywleng;
extern int      yyprevious;

    lastch  = &(yywtext[yywleng]);

    if ((x >= 0) && (x <= yywleng))
        ptr = &(yywtext[x]);
    else
        /*
         * The cast on the next line papers over an unconscionable nonportable
         * glitch to allow the caller to hand the function a pointer instead of
         * an integer and hope that it gets figured out properly.  But it's
         * that way on all systems .   
         */
        ptr = (wchar_t *) x;

    while (lastch > ptr)
    {
        yymbunput(*--lastch);
        yywleng -= 1;
    }
    *lastch = 0;
    /* EH next line added as fix for defect 63895 */
    yymbreturn(0);
    if (ptr > yywtext)
        yyprevious = *--lastch;
}
