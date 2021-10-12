/* @(#)25	1.9  src/bos/usr/include/IN/LSdefs.h, libIN, bos411, 9428A410j 6/7/91 15:54:25 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * Services for singly and doubly linked lists with link fields in
 * the first (and second) positions in the record.  End of list,
 * in each direction, marked with NULL.  Both ends accessible.
 */

#ifndef _H_LSdefined
#define _H_LSdefined

#ifndef NULL
#define NULL 0
#endif

/*	LS[12]node is a generic definition for whatever is in the list
	List1 and List2 are list header objects */

typedef struct LS1tag {struct LS1tag *next;} LS1node;
typedef struct {LS1node *head,*tail;} List1;
#define ListSingle List1	/* temporary internal synonym */

typedef struct LS2tag {struct LS2tag *next,*prev;} LS2node;
typedef struct {LS2node *head,*tail;} List2;
#define ListDouble List2	/* temporary internal synonym */

/* these are appopropriate initializers for List1 and List2 objects if
   they are defined in non-automatic storage.  Otherwise use LS[12]init() */
#define LS1initial {NULL,NULL}
#define LS2initial {NULL,NULL}

/* operations on List1 objects */
#define LS1init(Lx) {register ListSingle *L=(Lx); L->head=L->tail=(LS1node *)NULL;}
#define LS1term(Lx) {LS1killall(Lx);}
#define LS1isempty(L) ((L)->head==(LS1node *)NULL)
extern LS1append(/* L,e */) /* ListSingle *L; LS1node *e; */;
extern LS1prepend(/* L,e */) /* ListSingle *L; LS1node *e; */;
extern LS1insert(/* L,e,after */) /* ListSingle *L; LS1node *e,after */;
#define LS1first(L) ((L)->head)
#define LS1last(L) ((L)->tail)
#define LS1next(L,e) (((LS1node *)(e))->next)
extern LS1node *LS1rmvfirst(/* L */) /* ListSingle *L; */;
extern LS1node *LS1rmvnext(/* L,e */) /* ListSingle *L; LS1node *e; */;
#define LS1killfirst(L) free((char *)LS1rmvfirst(L))
#define LS1killnext(L,e) free((char *)LS1rmvnext(L,e))
extern LS1killall(/* L */) /* ListSingle *L; */;
extern LS1concat(/* L1,L2 */) /* ListSingle *L1,*L2; */;
#define LS1generate(L,p,t) \
	for (p=(t *)LS1first(L); p!=NULL; p=(t *)LS1next(L,p))
extern int LS1count(/* L */) /* ListSingle *L; */;

/* operations on List1 objects */
#define LS2init(Lx) {register List2 *L=(Lx); L->head=L->tail=NULL;}
#define LS2term(Lx) {LS2killall(Lx);}
#define LS2isempty(L) ((L)->head == NULL)
extern LS2append(/* L,e */) /* List2 *L; LS2node *e; */;
extern LS2prepend(/* L,e */) /* List2 *L; LS2node *e; */;
extern LS2insbefore(/* L,e,before */) /* List2 *L; LS2node *e,*before;*/;
extern LS2insafter(/* L,e,after */) /* List2 *L; LS2node *e,*after; */;
#define LS2first(L) ((L)->head)
#define LS2last(L) ((L)->tail)
#define LS2next(L,e) (((LS2node *)(e))->next)
#define LS2prev(L,e) (((LS2node *)(e))->prev)
extern LS2node *LS2rmvfirst(/* L */) /* List2 *L; */;
extern LS2node *LS2rmvlast(/* L */) /* List2 *L; */;
extern LS2node *LS2remove(/* L,e */) /* List2 *L; LS2node *e; */;
#define LS2killfirst(L) (free((char *)LS2rmvfirst(L)))
#define LS2killlast(L) (free((char *)LS2rmvlast(L)))
#define LS2kill(L,e) (free(LS2rmv(L,e)))
extern LS2killall(/* L */) /* List2 *L; */;
extern LS2concat(/* L1,L2 */) /* List2 *L1,*L2; */;
#define LS2generate(L,p,t) \
	for (p=(t *)LS2first(L); p!=NULL; p=(t *)LS2next(L,p))
extern int LS2count(/* L */) /* List2 *L; */;

#endif
