#ifndef lint 
static char sccsid[] = "@(#)49	1.2 src/bos/usr/ccs/bin/make/lst.lib/lstForEachFrom.c, cmdmake, bos411, 9428A410j 6/20/94 10:49:25";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Lst_ForEachFrom
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: lstForEachFrom.c,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 1991/11/14 10:33:49 $";
#endif
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*-
 * lstForEachFrom.c --
 *	Perform a given function on all elements of a list starting from
 *	a given point.
 */

#include	"lstInt.h"
#include	<stdlib.h>

/*-
 *-----------------------------------------------------------------------
 * Lst_ForEachFrom --
 *	Apply the given function to each element of the given list. The
 *	function should return 0 if traversal should continue and non-
 *	zero if it should abort. 
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Only those created by the passed-in function.
 *
 *-----------------------------------------------------------------------
 */
/*VARARGS2*/
void
Lst_ForEachFrom (
    const Lst	    	    	l,
    const LstNode    	  	ln,
    int	(*proc)(),
    const ClientData	d
    )
{
    ListNode	tln = (ListNode)ln;
    List 	list = (List)l;
    ListNode	next;
    Boolean 	    	done;
    int     	    	result;
    
    if (!LstValid (list) || LstIsEmpty (list)) {
	return;
    }
    
    do {
	/*
	 * Take care of having the current element deleted out from under
	 * us.
	 */
	
	next = tln->nextPtr;
	
	tln->useCount = tln->useCount + 1;
	result = (*proc) (tln->datum, d);
	tln->useCount = tln->useCount - 1;

	/*
	 * We're done with the traversal if
	 *  - nothing's been added after the current node and
	 *  - the next node to examine is the first in the queue or
	 *    doesn't exist.
	 */
	done = (next == tln->nextPtr &&
		(next == NilListNode || next == list->firstPtr));
	
	next = tln->nextPtr;

	if (tln->flags & LN_DELETED) {
	    free((char *)tln);
	}
	tln = next;
    } while (!result && !LstIsEmpty(list) && !done);
    
}

