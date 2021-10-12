/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Lst_Concat
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: lstConcat.c,v $
 * Revision 1.2.2.3  1992/12/03  19:05:52  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:35  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:25:03  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:48  gm]
 * 
 * Revision 1.2  1991/12/05  20:43:17  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:37:30  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:04:02  mckeen]
 * 
 * $EndLog$
 */
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

#ifndef lint
static char sccsid[] = "@(#)66  1.4  src/bldenv/make/lst.lib/lstConcat.c, bldprocess, bos412, GOLDA411a 1/19/94 16:28:29";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)lstConcat.c	5.3 (Berkeley) 6/1/90";
#endif /* not lint */

/*-
 * listConcat.c --
 *	Function to concatentate two lists.
 */

#include    "lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_Concat --
 *	Concatenate two lists. New elements are created to hold the data
 *	elements, if specified, but the elements themselves are not copied.
 *	If the elements should be duplicated to avoid confusion with another
 *	list, the Lst_Duplicate function should be called first.
 *	If LST_CONCLINK is specified, the second list is destroyed since
 *	its pointers have been corrupted and the list is no longer useable.
 *
 * Results:
 *	SUCCESS if all went well. FAILURE otherwise.
 *
 * Side Effects:
 *	New elements are created and appended the the first list.
 *-----------------------------------------------------------------------
 */
void
Lst_Concat (
    Lst    	  	l1, 	/* The list to which l2 is to be appended */
    Lst    	  	l2, 	/* The list to append to l1 */
    int	   	  	flags)  /* LST_CONCNEW if LstNodes should be duplicated
				 * LST_CONCLINK if should just be relinked */
{
    register ListNode  	ln;     /* original LstNode */
    register ListNode  	nln;    /* new LstNode */
    register ListNode  	last;   /* the last element in the list. Keeps
				 * bookkeeping until the end */
    register List 	list1 = (List)l1;
    register List 	list2 = (List)l2;

    if (flags == LST_CONCLINK) {
	if (list2->firstPtr != NilListNode) {
	    /*
	     * We set the nextPtr of the
	     * last element of list two to be NIL to make the loop easier and
	     * so we don't need an extra case should the first list turn
	     * out to be non-circular -- the final element will already point
	     * to NIL space and the first element will be untouched if it
	     * existed before and will also point to NIL space if it didn't.
	     */
	    list2->lastPtr->nextPtr = NilListNode;
	    /*
	     * So long as the second list isn't empty, we just link the
	     * first element of the second list to the last element of the
	     * first list. If the first list isn't empty, we then link the
	     * last element of the list to the first element of the second list
	     * The last element of the second list, if it exists, then becomes
	     * the last element of the first list.
	     */
	    list2->firstPtr->prevPtr = list1->lastPtr;
	    if (list1->lastPtr != NilListNode) {
 		list1->lastPtr->nextPtr = list2->firstPtr;
	    }
	    list1->lastPtr = list2->lastPtr;
	    list1->numNodes += list2->numNodes;
	}
	PFree ((List)l2);
    } else if (list2->firstPtr != NilListNode) {
	/*
	 * We set the nextPtr of the last element of list 2 to be nil to make
	 * the loop less difficult. The loop simply goes through the entire
	 * second list creating new LstNodes and filling in the nextPtr, and
	 * prevPtr to fit into l1 and its datum field from the
	 * datum field of the corresponding element in l2. The 'last' node
	 * follows the last of the new nodes along until the entire l2 has
	 * been appended. Only then does the bookkeeping catch up with the
	 * changes. During the first iteration of the loop, if 'last' is nil,
	 * the first list must have been empty so the newly-created node is
	 * made the first node of the list.
	 */
	list2->lastPtr->nextPtr = NilListNode;
	for (last = list1->lastPtr, ln = list2->firstPtr;
	     ln != NilListNode;
	     ln = ln->nextPtr)
	{
	    nln = PAllocNode ();
	    nln->datum = ln->datum;
	    if (last != NilListNode) {
		last->nextPtr = nln;
	    } else {
		list1->firstPtr = nln;
	    }
	    nln->prevPtr = last;
	    nln->flags = nln->useCount = 0;
	    last = nln;
	    list1->numNodes++;
	}

	/*
	 * Finish bookkeeping. The last new element becomes the last element
	 * of list one. 
	 */
	list1->lastPtr = last;

	/*
	 * The circularity of both list one and list two must be corrected
	 * for -- list one because of the new nodes added to it; list two
	 * because of the alteration of list2->lastPtr's nextPtr to ease the
	 * above for loop.
	 */
	last->nextPtr = NilListNode;
    }

}
	
