static char sccsid[] = "@(#)06	1.5  src/bos/usr/bin/localedef/sem_xlat.c, cmdnls, bos411, 9428A410j 3/10/94 11:28:07";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS:
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
#include <sys/types.h>
#include <sys/localedef.h>
#include "semstack.h"
#include "symtab.h"
#include "err.h"


/*
*  FUNCTION: add_upper
*
*  DESCRIPTION:
*  Build the 'upper' character translation tables from the symbols on the
*  semantic stack.
*/
void add_upper(_LC_ctype_t *ctype)
{
    extern wchar_t max_wchar_enc;
    item_t *it;
    int i;

    /* check if upper array allocated yet - allocate if NULL */
    if (ctype->upper == NULL)
	ctype->upper = MALLOC(wchar_t, max_wchar_enc+1);

    /* set up default translations - which is identity */
    for (i=0; i <= max_wchar_enc; i++)
	ctype->upper[i] = i;

    /* for each range on stack - the min is the FROM pc, and the max is */
    /* the TO pc.*/
    while ((it = sem_pop()) != NULL) {
	ctype->upper[it->value.range->min] = it->value.range->max;
    }
}    


/*
*  FUNCTION: add_lower
*
*  DESCRIPTION:
*  Build the 'lower' character translation tables from the symbols on the
*  semantic stack.
*/
void add_lower(_LC_ctype_t *ctype)
{
    extern wchar_t max_wchar_enc;
    item_t *it;
    int i;

    /* check if lower array allocated yet - allocate if NULL */
    if (ctype->lower == NULL)
	ctype->lower = MALLOC(wchar_t, max_wchar_enc+1);

    /* set up default translations which is identity */
    for (i=0; i <= max_wchar_enc; i++)
	ctype->lower[i] = i;

    /* for each range on stack - the min is the FROM pc, and the max is */
    /* the TO pc.*/
    while ((it = sem_pop()) != NULL) {
	ctype->lower[it->value.range->min] = it->value.range->max;
    }
}	      

/* 
*  FUNCTION: sem_push_xlat
*
*  DESCRIPTION:
*  Creates a character range item from two character reference items.
*  The routine pops two character reference items off the semantic stack.
*  These items represent the "to" and "from" pair for a character case
*  translation.  The implementation uses a character range structure to
*  represent the pair.
*/
void sem_push_xlat(void)
{
  item_t   *it0, *it1;
  item_t   *it;
  it1 = sem_pop();		/* this is the TO member of the pair */
  it0 = sem_pop();		/* this is the FROM member of the pair */

  if ((!it1->is_bogus) && (!it0->is_bogus)) { /* only if both chars are valid */
    if (it0->type == SK_CHR && it1->type == SK_CHR)
       it = create_item(SK_RNG, it0->value.chr->wc_enc, it1->value.chr->wc_enc);
    else if (it0->type == SK_INT  && it1->type == SK_INT)
       it = create_item(SK_RNG, it0->value.int_no, it1->value.int_no);
    else if (it0->type == SK_CHR && it1->type == SK_INT)
       it = create_item(SK_RNG, it0->value.chr->wc_enc, it1->value.int_no);
    else if (it0->type == SK_INT && it1->type == SK_CHR)
       it = create_item(SK_RNG, it0->value.int_no, it1->value.chr->wc_enc);
    else 
      INTERNAL_ERROR;

    /* this creates the item and sets the min and max to wc_enc */

    sem_push(it);
    }

  destroy_item(it1);
  destroy_item(it0);

}
