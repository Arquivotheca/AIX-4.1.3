/* @(#)96	1.2  src/bos/usr/bin/localedef/list.h, cmdnls, bos411, 9428A410j 6/1/91 14:42:46 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
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

#ifndef _H_LIST
#define _H_LIST
typedef struct _LISTEL_T listel_t;

struct _LISTEL_T {
  void   *key;
  void   *data;
  
  listel_t *next;
};

typedef struct {
  int      (*comparator)();

  listel_t head;
} list_t;

list_t *   create_list( int (*comparator)() );
int        add_list_element(list_t *, listel_t *);
int        loc_list_element(list_t *, void *);
listel_t  *create_list_element(void *key, void *data);

#endif

