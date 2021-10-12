static char sccsid[] = "@(#)36	1.2  src/bos/usr/bin/localedef/fwdrefstack.c, cmdnls, bos411, 9428A410j 4/6/94 21:57:12";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS: fwdref_push, fwdref_pop,
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

#include "fwdrefstack.h"
#include "err.h"			/* get MALLOC */

static fwdref_t *fwdref_head=NULL;
static fwdref_t *fwdref_tail=NULL;

static fwdref_t *Efwdref_head=NULL;
static fwdref_t *Efwdref_tail=NULL;
static fwdref_t *Efwdref_ptr=NULL;

static fwdref_t *next_ptr=NULL;    /* for fwdref_getnext() */


/*
*  Push an element onto the forward reference stack.
*/

void fwdref_push(fwdref_t * new_element)
{
	/* first element */
	if (fwdref_head == NULL) {
		fwdref_head = fwdref_tail = new_element;
		new_element->next = new_element->prev = NULL;
		return;
	}

	new_element->next = NULL;
	new_element->prev = fwdref_tail;
	fwdref_tail->next = new_element;
	fwdref_tail = new_element;
}


/*
*  Pop an element off of the forward reference stack.
*/

fwdref_t *fwdref_pop(void)
{
	fwdref_t * ret_val;

	/* no values to pop */
	if (fwdref_tail==NULL) return NULL;

	/* only one value to pop */
	if (fwdref_tail==fwdref_head) {
		ret_val = fwdref_head;
		fwdref_tail = fwdref_head = NULL;
		return ret_val;
		}

	/* return last of several entries */
        ret_val = fwdref_tail;
	fwdref_tail = fwdref_tail->prev;
	fwdref_tail->next = NULL;
	return (ret_val);
}


/*
*  Create an forward reference element, and initialize
*  contents.
*/

fwdref_t *create_fwdref(void)
{
	fwdref_t *ptr = MALLOC(fwdref_t,1);

	ptr->type = 0;
	ptr->symbol_text = NULL;
	ptr->one_many_offset = -1;
	ptr->order = 0;
	ptr->target_wc = 0;
	ptr->target_coll_ell_sym = NULL;
	ptr->wc = 0;
	ptr->coll_ell_sym = NULL;
	ptr->coll_sym_wgt = NULL;
	ptr->is_UNDEFINED = 0;
	ptr->next = ptr->prev = NULL;

	return (ptr);
}


/*
*  duplicate a forward reference element.
*  This is used to make it easier to implement
*  forward references as weights on the right hand
*  side of an ellipsis.  (That reference must be
*  duplicated for each character in the range)
*/

fwdref_t *fwdref_dup(fwdref_t *orig)
{
	fwdref_t *ret_val = MALLOC(fwdref_t,1);

	ret_val->type = orig->type;
	ret_val->symbol_text = orig->symbol_text;
	ret_val->one_many_offset = orig->one_many_offset;
	ret_val->order = orig->order;
	ret_val->target_wc = orig->target_wc;
	ret_val->target_coll_ell_sym = orig->target_coll_ell_sym;
	ret_val->wc = orig->wc;
	ret_val->coll_ell_sym = orig->coll_ell_sym;
	ret_val->coll_sym_wgt = orig->coll_sym_wgt;
	ret_val->is_UNDEFINED = orig->is_UNDEFINED;
	ret_val->next = orig->next;
	ret_val->prev = orig->prev;

	return (ret_val);
}


/*
*  simply initialize next_ptr to head for 
*  fwdref_getnext()
*/

void fwdref_start_scan(void)
{
	next_ptr = fwdref_head;
}


/*
*   Gets the next forward reference.
*   This is used for a non-desctructive
*   scan of the forward reference stack.
*/

fwdref_t *fwdref_getnext(void)
{
	fwdref_t *ret_val;

	if (next_ptr == NULL)
		return (NULL);

	ret_val = next_ptr;
	next_ptr = next_ptr->next;
	return (ret_val);
}


/*
*  Remove a specified element from the stack.
*/

void fwdref_remove_element(fwdref_t *element)
{
	/* no items on list */
	if (element==NULL)
		return;
	
	/* only item on list */
	if ((element == fwdref_tail) && (element == fwdref_head)) {
		free(element);
		next_ptr = fwdref_tail = fwdref_head = NULL;
		return;
		}

	/* last item on list */
	if (element == fwdref_tail) {
		fwdref_tail->prev->next = NULL;
		fwdref_tail = fwdref_tail->prev;
		free(element);
		return;
		}

	/* first item on list */
	if (element == fwdref_head) {
		fwdref_head->next->prev = NULL;
		fwdref_head = fwdref_head->next;
		free(element);
		return;
		}
		
	/* item in middle of list */
	element->next->prev = element->prev;
	element->prev->next = element->next;
	free(element);
	return;
}


/*
* Push an element on the ellipsis forward reference
* stack.
*/

void Efwdref_push(fwdref_t *new_element)
{
	/* first element */
	if (Efwdref_head == NULL) {
		Efwdref_head = Efwdref_tail = new_element;
		new_element->next = new_element->prev = NULL;
		return;
	}

	new_element->next = NULL;
	new_element->prev = Efwdref_tail;
	Efwdref_tail->next = new_element;
	Efwdref_tail = new_element;
}


/*
*  Get the first element on the ellipsis forward reference
*  stack.  (non-destructive read)
*/

fwdref_t *Efwdref_getfirst(void)
{
	fwdref_t * ret_val;

	Efwdref_ptr = Efwdref_head;    /* start at beginning */

	/* no values to pop */
	if (Efwdref_ptr==NULL) return NULL;

	ret_val = Efwdref_ptr;
	Efwdref_ptr = Efwdref_ptr->next;  /* move down to next element */
	return (ret_val);
}


/*
*  Get the next element on the ellipsis forward reference
*  stack.  (non-destructive read)
*/

fwdref_t *Efwdref_getnext(void)
{
	fwdref_t * ret_val;

	/* no values to pop */
	if (Efwdref_ptr==NULL) return NULL;

	ret_val = Efwdref_ptr;
	Efwdref_ptr = Efwdref_ptr->next;  /* move down to next element */
	return (ret_val);
}


/*
*   Remove all entries from the ellipsis stack.
*/
void Efwdref_clearall(void)
{

	fwdref_t * tmp_ptr;

	Efwdref_ptr = Efwdref_head;
	
	while (Efwdref_ptr != NULL) {
		tmp_ptr = Efwdref_ptr;
		Efwdref_ptr = Efwdref_ptr->next;
		free(tmp_ptr);
		}

	Efwdref_head = Efwdref_tail = Efwdref_ptr = NULL;
}

