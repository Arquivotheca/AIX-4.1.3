/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: PAlloc
 *		PAllocNode
 *		PFree
 *		PFreeNode
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
 * $Log: lstAlloc.c,v $
 * Revision 1.2.2.3  1992/12/03  19:05:38  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:26  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:24:44  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:32  gm]
 * 
 * Revision 1.2  1991/12/05  20:43:02  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:37:18  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:04:18  mckeen]
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
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)89  1.4  src/bldenv/make/lst.lib/lstAlloc.c, bldprocess, bos412, GOLDA411a 1/19/94 16:28:03";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "src/bldenv/make/lst.lib/lstAlloc.c, bldprocess, bos412, GOLDA411a (Berkeley) 1/19/94";
#endif /* not lint */

/*-
 * LstAlloc.c --
 *	Allocation and Deallocation of List and ListNode structures
 */

#include	<stdlib.h>
#include	"lstInt.h"

struct List ListFreeList;
struct List ListNodeFreeList;

List
PAlloc(void)
{
    List l, list = &ListFreeList;
    register ListNode front;

    if (list->numNodes == 0)
	return((List)malloc(sizeof(*l)));
    front = list->firstPtr;
    l = (List) front->datum;
    list->firstPtr = front->nextPtr;
    PFreeNode (front);
    list->numNodes--;
    return(l);
}

ListNode
PAllocNode(void)
{
    register ListNode front;
    register List list = &ListNodeFreeList;

    if (list->numNodes == 0)
	return((ListNode)malloc(sizeof(*front)));
    front = list->firstPtr;
    list->firstPtr = front->nextPtr;
    list->numNodes--;
    return(front);
}

void
PFree(List l)
{
    register List list = &ListFreeList;
    register ListNode ln;

    ln = PAllocNode();
    ln->datum = (ClientData)l;
    ln->nextPtr = list->firstPtr;
    list->firstPtr = ln;
    list->numNodes++;
}

void
PFreeNode(ListNode ln)
{
    register List 	list = &ListNodeFreeList;

    ln->nextPtr = list->firstPtr;
    list->firstPtr = ln;
    list->numNodes++;
}
