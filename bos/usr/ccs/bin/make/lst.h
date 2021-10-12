/* @(#)16	1.3  src/bos/usr/ccs/bin/make/lst.h, cmdmake, bos412, 9445A165578  10/25/94  10:21:21 */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: none
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
/*
 * @(#)$RCSfile: lst.h,v $ $Revision: 1.2.2.3 $ (OSF) $Date: 1992/03/23 22:36:39 $
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

#include	<sprite.h>

typedef struct ListNode {
   struct ListNode   *prevPtr;   /* previous element in list */
   struct ListNode   *nextPtr;   /* next in list */
   int         useCount:8, /* Count of functions using the node.
                 * node may not be deleted until count
                 * goes to 0 */
               flags:8;    /* Node status flags */
   ClientData  datum;       /* datum associated with this element */
} *ListNode;

typedef enum {
    Head, Middle, Tail, Unknown
} Where;

typedef struct {
   ListNode    firstPtr; /* first node in list */
   ListNode    lastPtr;  /* last node in list */
   Boolean     isCirc;    /* true if the list should be considered
               * circular */
/*
 * fields for sequential access
 */
   Where    atEnd;     /* Where in the list the last access was */
   Boolean     isOpen;    /* true if list has been Lst_Open'ed */
   ListNode    curPtr;    /* current node, if open. NilListNode if
               * *just* opened */
   ListNode    prevPtr;  /* Previous node, if open. Used by
               * Lst_Remove */
} *List;

/*
 * basic typedef. This is what the Lst_ functions handle
 */

typedef	struct	Lst	*Lst;
typedef	struct	LstNode	*LstNode;

#define	NILLST		((Lst) NIL)
#define	NILLNODE	((LstNode) NIL)

/*
 * NOFREE can be used as the freeProc to Lst_Destroy when the elements are
 *	not to be freed.
 */
#define NOFREE		((void (*)()) 0)
#define NOCOPY		((ClientData (*)()) 0)

#define LST_CONCNEW	0   /* create new LstNode's when using Lst_Concat */
#define LST_CONCLINK	1   /* relink LstNode's when using Lst_Concat */

/*
 * Creation/destruction functions
 */
Lst		  Lst_Init(const Boolean);   	/* Create a new list */

/* 
Destroy an old one.
*/
void Lst_Destroy(const Lst,void (*)());

int	    	  Lst_Length();	    	/* Find the length of a list */

/*-
 *-----------------------------------------------------------------------
 * Lst_IsEmpty --
 * Return TRUE if the given list is empty.
 *
 * Results:
 * TRUE if the list is empty, FALSE otherwise.
 *
 * Side Effects:
 * None.
 *
 * A list is considered empty if its firstPtr == NilListNode (or if
 * the list itself is NILLIST).
 *-----------------------------------------------------------------------
Boolean
Lst_IsEmpty (
    Lst  l
    )
{
    return ( ! LstValid (l) || LstIsEmpty(l));
}
 */
#define	Lst_IsEmpty(l)	(!LstValid(l)||LstIsEmpty(l))

/*
 * Functions to modify a list
 */
ReturnStatus	  Lst_Insert(const Lst, const LstNode, const ClientData);	/* Insert an element 
					 * before another */
ReturnStatus	  Lst_Append(const Lst, const LstNode, const ClientData);	/* Insert an element 
					 * after another */

/*
 * LstValid (l) --
 * Return TRUE if the list l is valid
 */
#define LstValid(l)  (((Lst)(l)==NILLST)?(FALSE):(TRUE))

#define NilListNode  ((ListNode)-1)

/*
 * LstIsEmpty (l) --
 * TRUE if the list l is empty.
 */
#define LstIsEmpty(l)   (((List)(l))->firstPtr==NilListNode)

/*-
 *-----------------------------------------------------------------------
 * Lst_First --
 * Return the first node on the given list.
 *
 * Results:
 * The first node or NILLNODE if the list is empty.
 *
 * Side Effects:
 * None.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_First(l)	((!LstValid(l)||LstIsEmpty(l))?(NILLNODE):((LstNode)((List)(l))->firstPtr))

/*-
 *-----------------------------------------------------------------------
 * Lst_ForEach --
 * Apply the given function to each element of the given list. The
 * function should return 0 if Lst_ForEach should continue and non-
 * zero if it should abort.
 *
 * Results:
 * None.
 *
 * Side Effects:
 * Only those created by the passed-in function.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_ForEach(l,proc,d)	(Lst_ForEachFrom((l),Lst_First(l),(proc),(d)))

/*-
 *-----------------------------------------------------------------------
 * Lst_AtFront --
 * Place a piece of data at the front of a list
 *
 * Results:
 * SUCCESS or FAILURE
 *
 * Side Effects:
 * A new ListNode is created and stuck at the front of the list.
 * hence, firstPtr (and possible lastPtr) in the list are altered.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_AtFront(l,d)	(Lst_Insert ((l), Lst_First(l), (d)))

/*-
 *-----------------------------------------------------------------------
 * Lst_Last --
 * Return the last node on the list l.
 *
 * Results:
 * The requested node or NILLNODE if the list is empty.
 *
 * Side Effects:
 * None.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_Last(l)	((!LstValid(l)||LstIsEmpty(l))?(NILLNODE):((LstNode)((List)(l))->lastPtr))

/*-
 *-----------------------------------------------------------------------
 * Lst_AtEnd --
 * Add a node to the end of the given list
 *
 * Results:
 * SUCCESS if life is good.
 *
 * Side Effects:
 * A new ListNode is created and added to the list.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_AtEnd(l,d)	(Lst_Append((l),Lst_Last(l),(d)))

ReturnStatus	  Lst_Remove(const Lst, const LstNode);    	/* Remove an element */
ReturnStatus	  Lst_Replace(const LstNode, const ClientData); /* Replace a node with 
					 * a new value */
ReturnStatus	  Lst_Move();	    	/* Move an element to another place */
ReturnStatus	  Lst_Concat(const Lst, const Lst, const int);	/* Concatenate two lists */

/*
 * Node-specific functions
 */

/*-
 *-----------------------------------------------------------------------
 * Lst_Succ --
 * Return the sucessor to the given node on its list.
 *
 * Results:
 * The successor of the node, if it exists (note that on a circular
 * list, if the node is the only one in the list, it is its own
 * successor).
 *
 * Side Effects:
 * None.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_Succ(ln)	(((ln)==NILLNODE)?(NILLNODE):((LstNode)((ListNode)(ln))->nextPtr))

LstNode		  Lst_Pred();	    	/* Return predecessor to given
					 * element */

/*-
 *-----------------------------------------------------------------------
 * Lst_Datum --
 * Return the datum stored in the given node.
 *
 * Results:
 * The datum or (ick!) NIL if the node is invalid.
 *
 * Side Effects:
 * None.
 *
 *-----------------------------------------------------------------------
 */
#define	Lst_Datum(ln)	(((ln)!=NILLNODE)?(((ListNode)(ln))->datum):((ClientData) NIL))

/*
 * Functions for entire lists
 */

/*-
 *-----------------------------------------------------------------------
 * Lst_Find --
 * Find a node on the given list using the given comparison function
 * and the given datum.
 *
 * Results:
 * The found node or NILLNODE if none matches.
 *
 * Side Effects:
 * None.
 *
 *-----------------------------------------------------------------------
*/
#define	Lst_Find(l,d,cProc)	(Lst_FindFrom((l),Lst_First(l),(d),(cProc)))

/*
Find an element starting from somewhere.
*/
LstNode Lst_FindFrom(const Lst,const LstNode,const ClientData,int (*)());

LstNode	    	  Lst_Member(const Lst, const ClientData); 	/* See if the given datum 
					 * is on the list. Returns the 
					 * LstNode containing the datum */
int	    	  Lst_Index();	    	/* Returns the index of a datum in the
					 * list, starting from 0 */

/* 
Apply a function to all elements of a lst starting from a certain
point. If the list is circular, the application will wrap around to the
beginning of the list again.
*/
void Lst_ForEachFrom(const Lst,const LstNode,int (*)(),const ClientData);

/*
 * these functions are for dealing with a list as a table, of sorts.
 * An idea of the "current element" is kept and used by all the functions
 * between Lst_Open() and Lst_Close().
 */
ReturnStatus	  Lst_Open(const Lst);    	/* Open the list */
LstNode		  Lst_Prev();	    	/* Previous element */
LstNode		  Lst_Cur();	    	/* The current element, please */
LstNode		  Lst_Next(const Lst);	/* Next element please */
void		  Lst_Close(const Lst);    	/* Finish table access */

/*
 * for using the list as a queue
 */
ClientData	  Lst_DeQueue(const Lst);	/* Remove an element from head of
					 * queue */

#endif /* _LST_H_ */
