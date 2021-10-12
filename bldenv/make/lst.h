/* @(#)43       1.3  src/bldenv/make/lst.h, bldprocess, bos412, GOLDA411a 1/19/94 16:27:53
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Lst_AtEnd
 *		Lst_AtFront
 *		Lst_Datum
 *		Lst_Destroy
 *		Lst_Duplicate
 *		Lst_EnQueue
 *		Lst_Find
 *		Lst_FindFrom
 *		Lst_First
 *		Lst_ForEach
 *		Lst_ForEachFrom
 *		Lst_IsEmpty
 *		Lst_Last
 *		Lst_Length
 *		Lst_Replace
 *		Lst_Succ
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
 * $Log: lst.h,v $
 * Revision 1.2.2.3  1992/12/03  19:05:37  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:35:24  damon]
 *
 * Revision 1.2.2.2  1992/09/24  19:24:36  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:54:42  gm]
 * 
 * Revision 1.2  1991/12/05  20:42:59  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  16:00:57  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
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
 *
 *	@(#)lst.h	5.3 (Berkeley) 6/1/90
 */

/*-
 * lst.h --
 *	Header for using the list library
 */
#ifndef _LST_H_
#define _LST_H_

#include	"sprite.h"

/*
 * basic typedef. This is what the Lst_ functions handle
 */

typedef struct ListNode {
	struct ListNode	*prevPtr;   /* previous element in list */
	struct ListNode	*nextPtr;   /* next in list */
	short		useCount;   /* Count of functions using the node.
				     * node may not be deleted until count
				     * goes to 0 */
	short		flags;	    /* Node status flags */
	ClientData	datum;	    /* datum associated with this element */
} *ListNode;
/*
 * Flags required for synchronization
 */
#define LN_DELETED  	0x0001      /* List node should be removed when done */

#define NilListNode	((ListNode)-1)

typedef enum {
    Head, Middle, Tail, Unknown
} Where;

typedef struct List {
	ListNode  	firstPtr; /* first node in list */
	ListNode  	lastPtr;  /* last node in list */
/*
 * fields for sequential access
 */
	Where	  	atEnd;	  /* Where in the list the last access was */
	Boolean	  	isOpen;	  /* true if list has been Lst_Open'ed */
	ListNode  	curPtr;	  /* current node, if open. NilListNode if
				   * *just* opened */
	ListNode  	prevPtr;  /* Previous node, if open. Used by
				   * Lst_Remove */
	int		numNodes; /* Number of nodes on this list */
} *List;

#define NilList	  	((List)-1)

typedef	List		Lst;
typedef	ListNode	LstNode;

#define	NILLST		((Lst) NIL)
#define	NILLNODE	((LstNode) NIL)

/*
 * NOFREE can be used as the freeProc to Lst_Destroy when the elements are
 *	not to be freed.
 * NOCOPY performs similarly when given as the copyProc to Lst_Duplicate.
 */
#define NOFREE		((void (*)(ClientData)) 0)
#define NOCOPY		((ClientData (*)(ClientData)) 0)

#define LST_CONCNEW	0   /* create new LstNode's when using Lst_Concat */
#define LST_CONCLINK	1   /* relink LstNode's when using Lst_Concat */

/*
 * Creation/destruction functions
 */
Lst		  Lst_Init(void);	/* Create a new list */
Lst	    	  Lst_Duplicate(Lst, ClientData (*)(ClientData));
					/* Duplicate an existing list */
void		  Lst_Destroy(Lst, void (*)(ClientData));
					/* Destroy an old one */

/*
 * Functions to modify a list
 */
ReturnStatus	  Lst_Insert(Lst, LstNode, ClientData);
					/* Insert an element before another */
ReturnStatus	  Lst_Append(Lst, LstNode, ClientData);
					/* Insert an element after another */
void		  Lst_Remove(Lst, LstNode);
					/* Remove an element */
void		  Lst_Concat(Lst, Lst, int);
					/* Concatenate two lists */

/*
 * Functions for entire lists
 */
LstNode		  Lst_FindFrom(Lst, LstNode, ClientData,
			       int (*)(ClientData, ClientData));
					/* Find an element starting from
					 * somewhere */
LstNode	    	  Lst_Member(Lst, ClientData);
					/* See if the given datum is on the
					 * list. Returns the LstNode containing
					 * the datum */
void	    	  Lst_ForEachFrom(Lst, LstNode,
				  int (*)(ClientData, ClientData), ClientData);
					/* Apply a function to all elements of
					 * a lst starting from a certain point.
					 * If the list is circular, the
					 * application will wrap around to the
					 * beginning of the list again. */
/*
 * these functions are for dealing with a list as a table, of sorts.
 * An idea of the "current element" is kept and used by all the functions
 * between Lst_Open() and Lst_Close().
 */
void		  Lst_Open(Lst);	/* Open the list */
LstNode		  Lst_Next(Lst);	/* Next element please */
Boolean		  Lst_IsAtEnd(Lst);	/* Done yet? */
void		  Lst_Close(Lst);	/* Finish table access */

/*
 * for using the list as a queue
 */
ClientData	  Lst_DeQueue(Lst);	/* Remove an element from head of
					 * queue */

/*
 * Lst_IsEmpty (l) --
 *	TRUE if the list l is empty.
 */
#define Lst_IsEmpty(l)		((l)->firstPtr == NilListNode)

/*
 * Lst_Succ (ln) --
 *	Return successor to given element.
 */
#define Lst_Succ(ln)		((ln)->nextPtr)

/*
 * Lst_Replace (ln, d) --
 *	Replace the datum in the given node with the new datum
 */
#define Lst_Replace(ln, d)	(ln)->datum = (d)

/*
 * Lst_First (l) --
 *	Return the first node on the list l.
 */
#define Lst_First(l)		(Lst_IsEmpty(l) ? NILLNODE : (l)->firstPtr)

/*
 * Lst_Last (l) --
 *	Return the last node on the list l.
 */
#define Lst_Last(l)		(Lst_IsEmpty(l) ? NILLNODE : (l)->lastPtr)

/*
 * Lst_Find (l, d, p) --
 *	Find a node on the given list using the given comparison function
 *	and the given datum.
 */
#define Lst_Find(l, d, p)	Lst_FindFrom(l, Lst_First(l), d, p)

/*
 * Lst_ForEach (l, p, d) --
 *	Apply the given function to each element of the given list. The
 *	function should return 0 if Lst_ForEach should continue and non-
 *	zero if it should abort.
 */
#define Lst_ForEach(l, p, d)	Lst_ForEachFrom(l, Lst_First(l), p, d)

/*
 * Lst_EnQueue (l, d) --
 *	Add the datum to the tail of the given list.
 */
#define Lst_EnQueue(l, d)	Lst_Append(l, Lst_Last(l), d)

/*
 * Lst_Datum (ln) --
 *	Return the datum stored in the given node.
 */
#define Lst_Datum(ln)		((ln)->datum)

/*
 * Lst_AtFront (l, d) --
 *	Place a piece of data at the front of a list.
 */
#define Lst_AtFront(l, d)	Lst_Insert(l, Lst_First(l), d)

/*
 * Lst_AtEnd (l, d) --
 *	Add a node to the end of the given list.
 */
#define Lst_AtEnd(l, d)		Lst_Append(l, Lst_Last(l), d)

/*
 * Lst_Length (l) --
 *	Return the number of nodes in the list.
 */
#define Lst_Length(l)		((l)->numNodes)

#endif /* _LST_H_ */
