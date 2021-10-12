static char sccsid[] = "@(#)60	1.5  src/bos/usr/bin/mh/sbr/help.c, cmdmh, bos411, 9428A410j 3/27/91 17:50:49";
/* 
 * COMPONENT_NAME: CMDMH help.c
 * 
 * FUNCTIONS: MSGSTR, help 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "help.c	7.2 87/11/06 15:08:49"; */

/* help.c - print the usage line */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

void help (str, swp)
register char  *str;
register struct swit   *swp;
{
    printf (MSGSTR(SYN, "syntax: %s\n"), str); /*MSG*/
    printf (MSGSTR(SWITCHES, "  switches are:\n")); /*MSG*/
    printsw (ALL, swp, "-");

    printf (MSGSTR(VER, "\nversion: %s\n"), version); /*MSG*/
}
