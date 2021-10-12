static char sccsid[] = "@(#)43 1.5 src/bos/usr/ccs/lib/libl/main.c, libl, bos411, 9428A410j 6/18/91 11:31:39";
/*
 * COMPONENT_NAME: (CMDLANG) Language Utilities
 *
 * FUNCTIONS: main
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
# include <stdio.h>
# include <locale.h>
main()
{
    setlocale(LC_ALL,"");
    yylex();
    exit(0);
}
