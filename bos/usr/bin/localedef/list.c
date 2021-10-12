static char sccsid[] = "@(#)95	1.2  src/bos/usr/bin/localedef/list.c, cmdnls, bos411, 9428A410j 6/1/91 14:42:40";
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * FUNCTIONS: create_list, create_list_element, add_list_element, 
 *            loc_list_element.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include "err.h"
#include "list.h"

/*
*  FUNCTION: create_list
*
*  DESCRIPTION:
*  Create a linked list.  The routine takes as an argument the comparator 
*  which is used to add items to the list.  List elements are added in
*  sorted order.
*/
list_t * create_list( int (*comparator)() )
{
  list_t *l;

  l = MALLOC(list_t, 1);

  l->comparator = comparator;
  l->head.next = NULL;

  return l;
}


/*
*  FUNCTION: create_list_element
*
*  DESCRIPTION:
*  Create an instance of a list element.  'key' is what will be passwd
*  to the comparator function if the list item is added to a list, and 
*  'data' is caller defined.
*/
listel_t * create_list_element(void *key, void *data)
{
  listel_t *el;

  el = MALLOC(listel_t, 1);
  el->key = key;
  el->data = data;
  el->next = NULL;

  return el;
}


/*
*  FUNCTION: add_list_element
*
*  DESCRIPTION:
*  Adds the list element 'el' to the linked list 'l'.  The function uses the
*  element key and the comparator for the list to add the element in 
*  sorted order.
*/
int add_list_element(list_t *l, listel_t *el)
{
  int      c;
  listel_t *lp;

  c = -1;
  for (lp = &(l->head); 
       lp->next != NULL && 
         (c=(*(l->comparator))(lp->next->key, el->key)) < 0;
       lp = lp->next);

  if (c==0)
    return -1;
  else {
    el->next = lp->next;
    lp->next = el;
    return 0;
  }
}


/*
*  FUNCTION: loc_list_element
*
*  DESCRIPTION:
*  Locates the first list element in list 'l', with a key matching 'key'. 
*  The list comparator will be used to determine key equivalence.
*/
int loc_list_element(list_t *l, void *key)
{
  int      c;
  listel_t *lp;

  c = -1;
  for (lp = &(l->head); 
       lp->next != NULL && 
         (c=(*(l->comparator))(lp->next->key, key)) < 0;
       lp = lp->next);

  if (c!=0)
    return NULL;
  else
    return lp->next;
}

