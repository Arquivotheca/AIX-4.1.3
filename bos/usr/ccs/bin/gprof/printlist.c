static char sccsid[] = "@(#)63	1.3.1.2  src/bos/usr/ccs/bin/gprof/printlist.c, cmdstat, bos41B, 9504A 12/21/94 13:42:17";
/*
* COMPONENT_NAME: (CMDSTAT) gprof
*
* FUNCTIONS: printlist
*
* ORIGINS: 26, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989, 1994
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
* Copyright (c) 1980 Regents of the University of California.
* All rights reserved.  The Berkeley software License Agreement
* specifies the terms and conditions for redistribution.
*/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *  
 *  #ifndef lint
 *  static char sccsid[] = "printlist.c	5.1 (Berkeley) 6/4/85";
 *  #endif not lint
 */

#include <demangle.h>
#include "gprof_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GPROF,n,s) 

#include "gprof.h"

    /*
     *	these are the lists of names:
     *	there is the list head and then the listname
     *	is a pointer to the list head
     *	(for ease of passing to stringlist functions).
     */
static struct stringlist	fhead = { 0 , 0 };
struct stringlist	*flist = &fhead;
static struct stringlist	Fhead = { 0 , 0 };
struct stringlist	*Flist = &Fhead;
static struct stringlist	ehead = { 0 , 0 };
struct stringlist	*elist = &ehead;
static struct stringlist	Ehead = { 0 , 0 };
struct stringlist	*Elist = &Ehead;

addlist( listp , funcname )
    struct stringlist	*listp;
    char		*funcname;
{
    struct stringlist	*slp;

    slp = (struct stringlist *) malloc( sizeof(struct stringlist));
    if ( slp == (struct stringlist *) 0 ) {
	fprintf( stderr, MSGSTR(NOROOM_PRLST, "gprof: ran out room for printlist\n") ); /*MSG*/
	done();
    }
    slp -> next = listp -> next;
    slp -> string = funcname;
    listp -> next = slp;
}

bool
onlist( listp , funcname )
    struct stringlist	*listp;
    char		*funcname;
{
    struct stringlist	*slp;
	struct Name *demname;
	char **rest, *sym_name, sym_save[100];

    for ( slp = listp -> next ; slp ; slp = slp -> next ) {
		if ( (sym_name = dem_conv( funcname )) 
				== INVAL )	
			sym_name = funcname;
		if ( ! strcmp( slp -> string , sym_name )) {
		   	return TRUE;
		} else {
			if ( funcname[0] == '.' && 
				! strcmp( slp -> string , ++sym_name) ) {
		   	 	return TRUE;
			}
		}
    }
    return FALSE;
}
