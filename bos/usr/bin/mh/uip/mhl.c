static char sccsid[] = "@(#)60	1.8  src/bos/usr/bin/mh/uip/mhl.c, cmdmh, bos411, 9428A410j 11/9/93 09:42:10";
/* 
 * COMPONENT_NAME: CMDMH mhl.c
 * 
 * FUNCTIONS: MSGSTR, Mmhl, OfficialName 
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


/* mhl.c - the MH message listing program */

#include "mh_msg.h" 
#include "mh.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

main (argc, argv)
int     argc;
char  **argv;
{
    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    done (mhl (argc, argv));
}

/*  */

/* Cheat:  we are loaded with adrparse, which wants a routine called
   OfficialName().  We call adrparse:getm() with the correct arguments
   to prevent OfficialName() from being called.  Hence, the following
   is to keep the loader happy.
 */

char   *OfficialName (name)
register char  *name;
{
    return name;
}
