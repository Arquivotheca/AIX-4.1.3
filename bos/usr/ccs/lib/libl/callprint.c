static char sccsid[] = "@(#)69 1.2 src/bos/usr/ccs/lib/libl/callprint.c, libl, bos411, 9428A410j 4/14/94 12:56:27";
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
#define _ILS_MACROS    /* 139785 - use macros for better performance */
#include <stddef.h>
#include <sys/localedef.h>
#include <ctype.h>

extern struct ostream *yyout;
static int printable(int c)
{
    return(iswprint(c));
}

void allprint__FUs(wchar_t c)
{
    char buffer [10];

    switch(c)
    {
    case '\n':
	__ls__7ostreamFPCc (yyout,"\\n");
	break;
    case '\t':
	__ls__7ostreamFPCc (yyout,"\\t");
	break;
    case '\b':
	__ls__7ostreamFPCc (yyout,"\\b");
	break;
    case ' ':
	__ls__7ostreamFPCc (yyout,"\\\bb");
	break;
    default:
	if(!printable(c))
	{
	    if (c<=0xFF)
		sprintf(buffer, "\\%-3o",c);
	    else
		sprintf(buffer, "\\%-6o",c);
	}
	else 
	    buffer[wctomb (buffer, c)] = '\0';
	__ls__7ostreamFPCc (yyout, buffer);
	break;
    }
}

void sprint__FPUs(wchar_t *s)
{
    while(*s)
	allprint__FUs(*s++);
}
