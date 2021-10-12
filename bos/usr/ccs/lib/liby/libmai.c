static char sccsid[] = "@(#)50 1.4 src/bos/usr/ccs/lib/liby/libmai.c, liby, bos411, 9428A410j 8/1/91 15:34:05";
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <locale.h>
main(){
        setlocale(LC_ALL,"");
        yyparse();
        }
