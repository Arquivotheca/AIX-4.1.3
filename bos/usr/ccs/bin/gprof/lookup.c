static char sccsid[] = "@(#)59	1.5  src/bos/usr/ccs/bin/gprof/lookup.c, cmdstat, bos411, 9428A410j 4/25/91 17:47:42";
/*
* COMPONENT_NAME: (CMDSTAT) gprof
*
* FUNCTIONS: lookup
*
* ORIGINS: 26, 27
*
* This module contains IBM CONFIDENTIAL code. -- (IBM
* Confidential Restricted when combined with the aggregated
* modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
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
 *  static char sccsid[] = "lookup.c	5.1 (Berkeley) 6/4/85";
 *  #endif not lint
 */

#include "gprof_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GPROF,n,s) 

#include "gprof.h"

    /*
     *	look up an address in a sorted-by-address namelist
     *	    this deals with misses by mapping them to the next lower 
     *	    entry point.
     */
nltype *
nllookup( address )
    unsigned long	address;
{
    register long	low;
    register long	middle;
    register long	high;
#   ifdef DEBUG
	register int	probes;

	probes = 0;
#   endif DEBUG
    for ( low = 0 , high = nname  ; low != high ; ) {
#	ifdef DEBUG
	    probes += 1;
#	endif DEBUG
	middle = ( high + low ) >> 1;
	if ( nl[ middle ].value <= address && nl[ middle+1 ].value > address ) {
#	    ifdef DEBUG
		if ( debug & LOOKUPDEBUG ) {
		    printf( "[nllookup] %d (%d) probes\n" , probes , nname-1 );
		}
#	    endif DEBUG
	    return &nl[ middle ];
	}
	if ( nl[ middle ].value > address ) {
	    high = middle;
	} else {
	    low = middle + 1;
	}
    }

    if (nl[ middle ].value <= address) 
	    return &nl[ middle ];
    fprintf( stderr , MSGSTR(BADSEARCH, "[nllookup] binary search fails???\n") ); /*MSG*/
    return (0);
}

arctype *
arclookup( parentp , childp )
    nltype	*parentp;
    nltype	*childp;
{
    arctype	*arcp;

    if ( parentp == 0 || childp == 0 ) {
	fprintf( stderr, MSGSTR(ARCLOOKUP, "[arclookup] parentp == 0 || childp == 0\n") ); /*MSG*/
	return 0;
    }
#   ifdef _DEBUG
	if ( debug & LOOKUPDEBUG ) {
	    printf( "[arclookup] parent %s child %s\n" ,
		    parentp -> name , childp -> name );
	}
#   endif _DEBUG
    for ( arcp = parentp -> children ; arcp ; arcp = arcp -> arc_childlist ) {
#	ifdef _DEBUG
	    if ( debug & LOOKUPDEBUG ) {
		printf( "[arclookup]\t arc_parent %s arc_child %s\n" ,
			arcp -> arc_parentp -> name ,
			arcp -> arc_childp -> name );
	    }
#	endif _DEBUG
	if ( arcp -> arc_childp == childp ) {
	    return arcp;
	}
    }
    return 0;
}
