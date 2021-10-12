/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Lst_Insert
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
 * $Log: lstInsert.c,v $
 * Revision 1.2.2.3  1992/12/03  19:06:24  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:54  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:25:46  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:57:30  gm]
 * 
 * Revision 1.2  1991/12/05  20:43:51  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:37:57  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:05:12  mckeen]
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
static char sccsid[] = "@(#)78  1.4  src/bldenv/make/lst.lib/lstInsert.c, bldprocess, bos412, GOLDA411a 1/19/94 16:29:22";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)lstInsert.c	5.3 (Berkeley) 6/1/90";
#endif /* not lint */

/*-
 * LstInsert.c --
 *	Insert a new datum before an old one
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_Insert --
 *	Insert a new node with the given piece of data before the given
 *	node in the given list.
 *
 * Results:
 *	SUCCESS or FAILURE.
 *
 * Side Effects:
 *	the firstPtr field will be changed if ln is the first node in the
 *	list.
 *
 *-----------------------------------------------------------------------
 */
ReturnStatus
Lst_Insert (l, ln, d)
    Lst	    	  	l;	/* list to manipulate */
    LstNode	  	ln;	/* node before which to insert d */
    ClientData	  	d;	/* datum to be inserted */
{
    register ListNode	nLNode;	/* new lnode for d */
    register ListNode	lNode = (ListNode)ln;
    register List 	list = (List)l;

    /*
     * check validity of arguments
     */
    if ((ln == NilListNode) ^ (Lst_IsEmpty (l)))
	return (FAILURE);

    nLNode = PAllocNode ();
    
    nLNode->datum = d;
    nLNode->useCount = nLNode->flags = 0;
    
    if (ln == NILLNODE) {
	nLNode->prevPtr = nLNode->nextPtr = NilListNode;
	list->firstPtr = list->lastPtr = nLNode;
    } else {
	nLNode->prevPtr = lNode->prevPtr;
	nLNode->nextPtr = lNode;
	
	if (nLNode->prevPtr != NilListNode) {
	    nLNode->prevPtr->nextPtr = nLNode;
	}
	lNode->prevPtr = nLNode;
	
	if (lNode == list->firstPtr) {
	    list->firstPtr = nLNode;
	}
    }
    list->numNodes++;
    
    return (SUCCESS);
}
	
