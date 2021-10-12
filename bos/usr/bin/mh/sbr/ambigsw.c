static char sccsid[] = "@(#)38	1.5  src/bos/usr/bin/mh/sbr/ambigsw.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:01";
/* 
 * COMPONENT_NAME: CMDMH ambigsw.c
 * 
 * FUNCTIONS: MSGSTR, ambigsw 
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
/* static char sccsid[] = "ambigsw.c	7.1 87/10/13 17:03:15"; */

/* ambigsw.c - report an ambiguous switch */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

void ambigsw (arg, swp)
register char   *arg;
register struct swit *swp;
{
    advise (NULLCP, MSGSTR(AMBIG, "-%s ambiguous.  It matches"), arg); /*MSG*/
    printsw (arg, swp, "-");
}
