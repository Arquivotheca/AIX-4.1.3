/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Lst_DeQueue
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
 * $Log: lstDeQueue.c,v $
 * Revision 1.2.2.3  1992/12/03  19:05:57  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:38  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:25:09  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:54  gm]
 * 
 * Revision 1.2  1991/12/05  20:43:22  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  16:37:35  gm]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:04:13  mckeen]
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
static char sccsid[] = "@(#)68  1.4  src/bldenv/make/lst.lib/lstDeQueue.c, bldprocess, bos412, GOLDA411a 1/19/94 16:28:24";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)lstDeQueue.c	5.3 (Berkeley) 6/1/90";
#endif /* not lint */

/*-
 * LstDeQueue.c --
 *	Remove the node and return its datum from the head of the list
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_DeQueue --
 *	Remove and return the datum at the head of the given list.
 *
 * Results:
 *	The datum in the node at the head or (ick) NIL if the list
 *	is empty.
 *
 * Side Effects:
 *	The head node is removed from the list.
 *
 *-----------------------------------------------------------------------
 */
ClientData
Lst_DeQueue (l)
    Lst	    	  l;
{
    ClientData	  rd;
    register ListNode	tln;
    
    tln = (ListNode) Lst_First (l);
    if (tln == NilListNode) {
	return ((ClientData) NIL);
    }
    
    rd = tln->datum;
    Lst_Remove (l, (LstNode)tln);
    return (rd);
}

