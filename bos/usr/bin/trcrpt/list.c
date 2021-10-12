static char sccsid[] = "@(#)50  1.1  src/bos/usr/bin/trcrpt/list.c, cmdtrace, bos411, 9428A410j 8/19/93 08:27:39";

/*
 * COMPONENT_NAME: CMDTRACE
 * 
 * FUNCTIONS: Listc List
 * 
 * ORIGINS: 83 
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include "rpt.h"

extern FILE *Listfp;

/*
 * Print the character 'c' to the list file, if open
 */
Listc(c)
{

        Debug("%c",c);
        if(Listfp)
                fputc(c,Listfp);
}

/*
 * Print to the list file, if open
 */
List(s,a,b,c,d,e,f,g)
{

        Debug(s,a,b,c,d,e,f,g);
        if(Listfp)
                /* print data for cmd line option */
                fprintf(Listfp,s,a,b,c,d,e,f,g);
}

