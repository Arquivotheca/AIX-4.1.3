static char sccsid[] = "@(#)55	1.5  src/bos/usr/ccs/lib/libdbx/lists.c, libdbx, bos411, 9428A410j 1/31/94 17:07:46";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: generic_list_item, list_alloc, list_append, list_concat,
 *	      list_delete, list_insert
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*
 * General list definitions.
 *
 * The assumption is that the elements in a list are words,
 * usually pointers to the actual information.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "lists.h"

/*
 * Allocate and initialize a list.
 */

public List list_alloc()
{
    List list;

    list = new(List);
    list->nitems = 0;
    list->head = nil;
    list->tail = nil;
    return list;
}

/*
 * Create a list item from an object (represented as pointer or integer).
 */

public ListItem generic_list_item(element)
ListElement element;
{
    ListItem i;

    i = new(ListItem);
    i->element = element;
    i->next = nil;
    i->prev = nil;
    return i;
}

/*
 * Insert an item before the item in a list.
 */

public list_insert(item, after, list)
ListItem item;
ListItem after;
List list;
{
    ListItem a;

    checkref(list);
    list->nitems = list->nitems + 1;
    if (list->head == nil) {
	list->head = item;
	list->tail = item;
    } else {
	if (after == nil) {
	    a = list->head;
	} else {
	    a = after;
	}
	item->next = a;
	item->prev = a->prev;
	if (a->prev != nil) {
	    a->prev->next = item;
	} else {
	    list->head = item;
	}
	a->prev = item;
    }
}

/*
 * Append an item after the given item in a list.
 */

public list_append(item, before, list)
ListItem item;
ListItem before;
List list;
{
    ListItem b;

    checkref(list);
    if (!(strcmp(item->element,"+"))) {
	/* append to the existing list */
	extern struct List *sourcepath;
	if (list->head == nil) {
	    list->head   = sourcepath->head;
	    list->tail   = sourcepath->tail;
            list->nitems = sourcepath->nitems;
	} else {
	    list_concat(list,sourcepath);
	}
	dispose(sourcepath);
	return;
    }
    list->nitems = list->nitems + 1;
    if (list->head == nil) {
	list->head = item;
	list->tail = item;
    } else {
	if (before == nil) {
	    b = list->tail;
	} else {
	    b = before;
	}
	item->next = b->next;
	item->prev = b;
	if (b->next != nil) {
	    b->next->prev = item;
	} else {
	    list->tail = item;
	}
	b->next = item;
    }
}

/*
 * Delete an item from a list.
 */

public list_delete(item, list)
ListItem item;
List list;
{
    if ((item == nil) || (list == nil))
	 return;
    checkref(item);
    checkref(list);
    assert(list->nitems > 0);
    if (item->next == nil) {
	list->tail = item->prev;
    } else {
	item->next->prev = item->prev;
    }
    if (item->prev == nil) {
	list->head = item->next;
    } else {
	item->prev->next = item->next;
    }
    list->nitems = list->nitems - 1;
    dispose(item);
}

/*
 * Concatenate one list onto the end of another.
 */

public List list_concat(first, second)
List first, second;
{
    List r;

    if (first == nil) {
	r = second;
    } else if (second == nil) {
	r = first;
    } else {
	second->head->prev = first->tail;
	first->tail->next = second->head;
	first->tail = second->tail;
	first->nitems = first->nitems + second->nitems;
	r = first;
    }
    return r;
}
