static char sccsid[] = "@(#)%M  1.4  src/bos/usr/bin/adb/input.c, cmdadb, bos411, 9428A410j  6/15/90  20:05:52";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: eocmd, getformat, nextchar, quotchar, rdc, readchar
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  Input routines
 */

#include "defs.h"

LOCAL char line[LINSIZ];

/* TRUE if end of command has been reached */
#ifndef _NO_PROTO
BOOL eocmd(char c)
#else
BOOL eocmd(c)
char c;
#endif
{
    return (c == '\n' || c == ';');
}

char rdc()
{
    char c;

    do
		   c = readchar();
    while (c == ' ' || c == '\t');

    return (c);
}

char readchar()
{
    if (eof)
		   lastc = EOF;
    else {
		   if (lp == 0) {
		       lp = line;
		       do {
			   eof = (0 == read(infile, lp, 1));
			   if (mkfault)
			       error((STRING) NULL);
		       } while (( ! eof) && *lp++ != '\n');
		       *lp = 0;
		       lp = line;
		   }
		   if (lastc = peekc)
		       peekc = 0;
		   else if (lastc = *lp)
		       lp++;
    }
    return (lastc);
}

int nextchar()
{
    if (eocmd(rdc())) {
		   lp--;
		   return (0);
    }
    else
		   return (lastc);
}

char quotchar()
{
    if (readchar() == '\\')
		   return (readchar());
    else if (lastc == '\'')
		   return ('\0');
    else
		   return (lastc);
}

void getformat(deformat)
STRING deformat;
{
    STRING fptr = deformat;
    BOOL quote = FALSE;

    while (quote ? readchar() != '\n' : ! eocmd(readchar()))
		   if ((*fptr++ = lastc) == '"')
		       quote = !quote;
    lp--;
    if (fptr != deformat)
		   *fptr++ = '\0';
}
