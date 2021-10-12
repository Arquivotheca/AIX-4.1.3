#ifndef lint 
static char sccsid[] = "@(#)39	1.3 src/bos/usr/ccs/bin/make/lst.lib/lstConcat.c, cmdmake, bos412, 9445A165578 10/25/94 10:21:25";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Lst_Concat
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
static char rcsid[] = "@(#)$RCSfile: lstConcat.c,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 1991/11/14 10:32:56 $";
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
 * listConcat.c --
 *	Function to concatentate two lists.
 */

#include    "lstInt.h"
#include    <stdlib.h>

/*-
 *-----------------------------------------------------------------------
 * Lst_Concat --
 *	Concatenate two lists. New elements are created to hold the data
 *	elements, if specified, but the elements themselves are not copied.
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
ReturnStatus
Lst_Concat (
    const Lst    	  	l1, 	/* The list to which l2 is to be appended */
    const Lst    	  	l2, 	/* The list to append to l1 */
    const int	   	  	flags   /* LST_CONCNEW if LstNode's should be duplicated
				 * LST_CONCLINK if should just be relinked */
    )
{
    ListNode  	ln;     /* original LstNode */
    ListNode  	nln;    /* new LstNode */
    ListNode  	last;   /* the last element in the list. Keeps
				 * bookkeeping until the end */
    List 	list1 = (List)l1;
    List 	list2 = (List)l2;

    if (!LstValid (l1) || !LstValid (l2)) {
	return (FAILURE);
    }

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
	}
	if (list1->isCirc && list1->firstPtr != NilListNode) {
	    /*
	     * If the first list is supposed to be circular and it is (now)
	     * non-empty, we must make sure it's circular by linking the
	     * first element to the last and vice versa
	     */
	    list1->firstPtr->prevPtr = list1->lastPtr;
	    list1->lastPtr->nextPtr = list1->firstPtr;
	}
	free ((Address)l2);
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
	    PAlloc (nln, ListNode);
	    nln->datum = ln->datum;
	    if (last != NilListNode) {
		last->nextPtr = nln;
	    } else {
		list1->firstPtr = nln;
	    }
	    nln->prevPtr = last;
	    nln->flags = nln->useCount = 0;
	    last = nln;
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
	if (list1->isCirc) {
	    list1->lastPtr->nextPtr = list1->firstPtr;
	    list1->firstPtr->prevPtr = list1->lastPtr;
	} else {
	    last->nextPtr = NilListNode;
	}

	if (list2->isCirc) {
	    list2->lastPtr->nextPtr = list2->firstPtr;
	}
    }

    return (SUCCESS);
}
	
