/* @(#)08	1.6  src/bos/usr/bin/localedef/semstack.h, cmdnls, bos411, 9428A410j 1/11/94 15:46:27 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SEMSTACK
#define _H_SEMSTACK

#include "symtab.h"

typedef struct {
  wint_t min;
  wint_t max;
} range_t;

typedef struct {
  int  type;
  char is_bogus;             /* ignore if symbol based on undefined char */
  
  union {		     /* type =  */
    int        int_no;	     /*   SK_INT */
    char       *str;	     /*   SK_STR */
    range_t    *range;	     /*   SK_RNG */
    chr_sym_t  *chr;	     /*   SK_CHR */
    _LC_subs_t *subs;        /*   SK_SUBS */
    symbol_t   *sym;         /*   SK_SYM */
  } value;

} item_t;

/* valid types for item_type above */
#define SK_INT  1
#define SK_STR  2
#define SK_RNG  3
#define SK_CHR  4
#define SK_SUBS 5
#define SK_SYM  6

/* semstack errors */
#define SK_OK       0
#define SK_OVERFLOW 1

/* semstack limits */
#define SK_MAX_DEPTH 65536

int sem_push(item_t *);
item_t *sem_pop(void);
item_t *create_item(int, ...);

#endif

