#ifndef lint 
static char sccsid[] = "@(#)56	1.3 src/bos/usr/ccs/bin/make/lst.lib/lstMember.c, cmdmake, bos412, 9447B 11/18/94 16:18:11";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Lst_Member
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
static char rcsid[] = "@(#)$RCSfile: lstMember.c,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 1991/11/14 10:34:25 $";
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
 * lstMember.c --
 *	See if a given datum is on a given list.
 */

#include    "lstInt.h"

LstNode
Lst_Member (
    const	Lst	    	  	l,
    const	ClientData	  	d
    )
{
    List    	  	list = (List) l;
    ListNode	lNode;

    if (list)
    {
	lNode = list->firstPtr;
	if (lNode == NilListNode) {
	    return NILLNODE;
	}
	
	do {
	    if (lNode->datum == d) {
		return (LstNode)lNode;
	    }
	    lNode = lNode->nextPtr;
	} while (lNode != NilListNode && lNode != list->firstPtr);
    }

    return NILLNODE;
}
