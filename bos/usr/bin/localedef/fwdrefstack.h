/* @(#)37	1.2  src/bos/usr/bin/localedef/fwdrefstack.h, cmdnls, bos411, 9428A410j 4/6/94 21:57:24 */
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FWDREFSTACK
#define _H_FWDREFSTACK


#include <sys/localedef.h>
#include <sys/lc_sys.h>


/* definition of elements used for forward reference stack */

typedef struct fwdref_struct {
  int      type; /* type of value on the left hand side                      */
                 /* only ST_CHR_SYM, ST_COLL_ELL, and ST_COLL_SYM are valid  */

  char     *symbol_text;   /* for error output */

  wchar_t  target_wc;	   /* process code for the target character          */

  char     *target_coll_ell_sym;

  int      one_many_offset;/* two byte offset into 1 to many string.         */
                           /* ie 0 specifies to first two bytes, 1 specifies */
                           /* the second set of twp bytes, etc.              */
                           /* if -1, then this is not a 1 to many mapping    */

  int	   order;	   /* order this is valid for                        */
  
  wchar_t  wc;		   /* for ST_CHR_SYM and ST_COLL_ELL                 */
  char     *coll_ell_sym;  /* for ST_COLL_ELL                                */
  
  int      is_UNDEFINED;   /* true only if this is for the UNDEFINED keyword */

  _LC_weight_t *coll_sym_wgt;	   /* for ST_COLL_SYM */

  struct fwdref_struct *next, *prev;

} fwdref_t;


/* functions for manipulation of normal forward reference stack */

void		fwdref_push(fwdref_t *);
fwdref_t      * fwdref_pop(void);
fwdref_t      * create_fwdref_pop(void);
fwdref_t      * fwdref_dup(fwdref_t *);
void		fwdref_start_scan(void);
fwdref_t      * fwdref_getnext(void);
void		fwdref_remove_element(fwdref_t *);

/* functions for manipulation of forward reference stack for ellipsis entries */

void		Efwdref_push(fwdref_t *);
fwdref_t      * Efwdref_getfirst(void);
fwdref_t      * Efwdref_getnext(void);
void		Efwdref_clearall(void);

#endif

