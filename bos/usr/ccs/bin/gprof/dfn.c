static char sccsid[] = "@(#)53	1.4  src/bos/usr/ccs/bin/gprof/dfn.c, cmdstat, bos41B, 9504A 12/21/94 13:42:06";
/*
* COMPONENT_NAME: (CMDSTAT) gprof
*
* FUNCTIONS: dfn
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
 */

/*
#ifndef lint
static char sccsid[] = "dfn.c	5.1 (Berkeley) 6/4/85";
#endif not lint
*/

#include "gprof_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_GPROF,n,s) 

#include <stdio.h>
#include "gprof.h"

#define	DFN_DEPTH	100
struct dfnstruct {
    nltype	*nlentryp;
    int		cycletop;
};
typedef struct dfnstruct	dfntype;

static dfntype	dfn_stack[ DFN_DEPTH ];
static int	dfn_depth = 0;

static int	dfn_counter = DFN_NAN;

    /*
     *	given this parent, depth first number its children.
     */
dfn( parentp )
    nltype	*parentp;
{
    arctype	*arcp;

#   ifdef _DEBUG
	if ( debug & DFNDEBUG ) {
	    printf( "[dfn] dfn(" );
	    printname( parentp );
	    printf( ")\n" );
	}
#   endif _DEBUG
	/*
	 *	if we're already numbered, no need to look any furthur.
	 */
    if ( dfn_numbered( parentp ) ) {
	return;
    }
	/*
	 *	if we're already busy, must be a cycle
	 */
    if ( dfn_busy( parentp ) ) {
	dfn_findcycle( parentp );
	return;
    }
	/*
	 *	visit yourself before your children
	 */
    dfn_pre_visit( parentp );
	/*
	 *	visit children
	 */
    for ( arcp = parentp -> children ; arcp ; arcp = arcp -> arc_childlist ) {
	    dfn( arcp -> arc_childp );
    }
	/*
	 *	visit yourself after your children
	 */
    dfn_post_visit( parentp );
}

    /*
     *	push a parent onto the stack and mark it busy
     */
static dfn_pre_visit( parentp )
    nltype	*parentp;
{

    dfn_depth += 1;
    if ( dfn_depth >= DFN_DEPTH ) {
	fprintf( stderr , MSGSTR(OVERFLOW, "[dfn] out of my depth (dfn_stack overflow)\n") ); /*MSG*/
	exit( 1 );
    }
    dfn_stack[ dfn_depth ].nlentryp = parentp;
    dfn_stack[ dfn_depth ].cycletop = dfn_depth;
    parentp -> toporder = DFN_BUSY;
#   ifdef _DEBUG
	if ( debug & DFNDEBUG ) {
	    printf( "[dfn_pre_visit]\t\t%d:" , dfn_depth );
	    printname( parentp );
	    printf( "\n" );
	}
#   endif _DEBUG
}

    /*
     *	are we already numbered?
     */
static bool
dfn_numbered( childp )
    nltype	*childp;
{
    
    return ( childp -> toporder != DFN_NAN && childp -> toporder != DFN_BUSY );
}

    /*
     *	are we already busy?
     */
static bool
dfn_busy( childp )
    nltype	*childp;
{

    if ( childp -> toporder == DFN_NAN ) {
	return FALSE;
    }
    return TRUE;
}

    /*
     *	MISSING: an explanation
     */
static dfn_findcycle( childp )
    nltype	*childp;
{
    int		cycletop;
    nltype	*cycleheadp;
    nltype	*tailp;
    int		index;

    for ( cycletop = dfn_depth ; cycletop > 0 ; cycletop -= 1 ) {
	cycleheadp = dfn_stack[ cycletop ].nlentryp;
	if ( childp == cycleheadp ) {
	    break;
	}
	if ( childp -> cyclehead != childp &&
	    childp -> cyclehead == cycleheadp ) {
	    break;
	}
    }
    if ( cycletop <= 0 ) {
	fprintf( stderr , MSGSTR(NOCYCLEHEAD, "[dfn_findcycle] couldn't find head of cycle\n") ); /*MSG*/
	exit( 1 );
    }
#   ifdef _DEBUG
	if ( debug & DFNDEBUG ) {
	    printf( "[dfn_findcycle] dfn_depth %d cycletop %d " ,
		    dfn_depth , cycletop  );
	    printname( cycleheadp );
	    printf( "\n" );
	}
#   endif _DEBUG
    if ( cycletop == dfn_depth ) {
	    /*
	     *	this is previous function, e.g. this calls itself
	     *	sort of boring
	     */
	dfn_self_cycle( childp );
    } else {
	    /*
	     *	glom intervening functions that aren't already
	     *	glommed into this cycle.
	     *	things have been glommed when their cyclehead field
	     *	points to the head of the cycle they are glommed into.
	     */
	for ( tailp = cycleheadp ; tailp -> cnext ; tailp = tailp -> cnext ) {
	    /* void: chase down to tail of things already glommed */
#	    ifdef _DEBUG
		if ( debug & DFNDEBUG ) {
		    printf( "[dfn_findcycle] tail " );
		    printname( tailp );
		    printf( "\n" );
		}
#	    endif _DEBUG
	}
	    /*
	     *	if what we think is the top of the cycle
	     *	has a cyclehead field, then it's not really the
	     *	head of the cycle, which is really what we want
	     */	
	if ( cycleheadp -> cyclehead != cycleheadp ) {
	    cycleheadp = cycleheadp -> cyclehead;
#	    ifdef _DEBUG
		if ( debug & DFNDEBUG ) {
		    printf( "[dfn_findcycle] new cyclehead " );
		    printname( cycleheadp );
		    printf( "\n" );
		}
#	    endif _DEBUG
	}
	for ( index = cycletop + 1 ; index <= dfn_depth ; index += 1 ) {
	    childp = dfn_stack[ index ].nlentryp;
	    if ( childp -> cyclehead == childp ) {
		    /*
		     *	not yet glommed anywhere, glom it
		     *	and fix any children it has glommed
		     */
		tailp -> cnext = childp;
		childp -> cyclehead = cycleheadp;
#		ifdef _DEBUG
		    if ( debug & DFNDEBUG ) {
			printf( "[dfn_findcycle] glomming " );
			printname( childp );
			printf( " onto " );
			printname( cycleheadp );
			printf( "\n" );
		    }
#		endif _DEBUG
		for ( tailp = childp ; tailp->cnext ; tailp = tailp->cnext ) {
		    tailp -> cnext -> cyclehead = cycleheadp;
#		    ifdef _DEBUG
			if ( debug & DFNDEBUG ) {
			    printf( "[dfn_findcycle] and its tail " );
			    printname( tailp -> cnext );
			    printf( " onto " );
			    printname( cycleheadp );
			    printf( "\n" );
			}
#		    endif _DEBUG
		}
	    } else if ( childp -> cyclehead != cycleheadp /* firewall */ ) {
		fprintf( stderr ,
			MSGSTR(DFNBUSY, "[dfn_busy] glommed, but not to cyclehead\n") ); /*MSG*/
	    }
	}
    }
}

    /*
     *	deal with self-cycles
     *	for lint: ARGSUSED
     */
static dfn_self_cycle( parentp )
    nltype	*parentp;
{
	/*
	 *	since we are taking out self-cycles elsewhere
	 *	no need for the special case, here.
	 */
#   ifdef _DEBUG
	if ( debug & DFNDEBUG ) {
	    printf( "[dfn_self_cycle] " );
	    printname( parentp );
	    printf( "\n" );
	}
#   endif _DEBUG
}

    /*
     *	visit a node after all its children
     *	[MISSING: an explanation]
     *	and pop it off the stack
     */
static dfn_post_visit( parentp )
    nltype	*parentp;
{
    nltype	*memberp;

#   ifdef _DEBUG
	if ( debug & DFNDEBUG ) {
	    printf( "[dfn_post_visit]\t%d: " , dfn_depth );
	    printname( parentp );
	    printf( "\n" );
	}
#   endif _DEBUG
	/*
	 *	number functions and things in their cycles
	 *	unless the function is itself part of a cycle
	 */
    if ( parentp -> cyclehead == parentp ) {
	dfn_counter += 1;
	for ( memberp = parentp ; memberp ; memberp = memberp -> cnext ) {
	    memberp -> toporder = dfn_counter;
#	    ifdef _DEBUG
		if ( debug & DFNDEBUG ) {
		    printf( "[dfn_post_visit]\t\tmember " );
		    printname( memberp );
		    printf( " -> toporder = %d\n" , dfn_counter );
		}
#	    endif _DEBUG
	}
    } else {
#	ifdef _DEBUG
	    if ( debug & DFNDEBUG ) {
		printf( "[dfn_post_visit]\t\tis part of a cycle\n" );
	    }
#	endif _DEBUG
    }
    dfn_depth -= 1;
}
