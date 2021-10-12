#ifndef lint 
static char sccsid[] = "@(#)60	1.2 src/bos/usr/ccs/bin/make/lst.lib/lstReplace.c, cmdmake, bos411, 9428A410j 6/20/94 10:50:03";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Lst_Replace
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
static char rcsid[] = "@(#)$RCSfile: lstReplace.c,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 1991/11/14 10:34:45 $";
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
 * LstReplace.c --
 *	Replace the datum in a node with a new datum
 */

#include	"lstInt.h"

/*-
 *-----------------------------------------------------------------------
 * Lst_Replace --
 *	Replace the datum in the given node with the new datum
 *
 * Results:
 *	SUCCESS or FAILURE.
 *
 * Side Effects:
 *	The datum field fo the node is altered.
 *
 *-----------------------------------------------------------------------
 */
ReturnStatus
Lst_Replace (
    const LstNode	ln,
    const ClientData	  	d
    )
{
    if (ln == NILLNODE) {
	return (FAILURE);
    } else {
	((ListNode) ln)->datum = d;
	return (SUCCESS);
    }
}

