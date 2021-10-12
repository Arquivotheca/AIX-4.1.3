static char sccsid[] = "@(#)07	1.4  src/bos/usr/bin/localedef/semstack.c, cmdnls, bos411, 9428A410j 1/4/94 15:43:36";
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
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
#include <sys/localedef.h>
#include <stdarg.h>
#include "err.h"
#include "semstack.h"

static int stack_top=SK_MAX_DEPTH;
static item_t *stack[SK_MAX_DEPTH];


/*
*  FUNCTION: sem_push
*
*  DESCRIPTION:
*  Pushes the item 'i' onto the semantic stack.
*/
int sem_push(item_t *i)
{
    if (stack_top > 0) {
	stack[--stack_top] = i;
	return SK_OK;
    } else
	error(STACK_OVERFLOW);
}


/*
*  FUNCTION: sem_pop
*
*  DESCRIPTION:
*  Removes the item from the top of the stack and returns it's address
*  to the caller.
*/
item_t *sem_pop(void)
{
    if (stack_top < SK_MAX_DEPTH)
	return stack[stack_top++];
    else 
	return NULL;
}


/*
*  FUNCTION: create_item
*
*  DESCRIPTION:
*  Creates a typed 'item_t' suitable for pushing on the semantic stack.
*  A value is assigned from arg_list based on the 'type' of the item being 
*  created.
*
*  This routine performs a malloc() to acquire memory necessary to hold
*  string and range data types.
*/
item_t *create_item(int type, ... )
{
    va_list ap;
    item_t *i;
    char   *s;
    
    va_start(ap, type);

    i = MALLOC(item_t, 1);
    i->type = type;
    
    switch (type) {
      case SK_INT:
	i->value.int_no = va_arg(ap, int);
	break;

      case SK_STR:
	s = va_arg(ap, char *);
	i->value.str = MALLOC(char, strlen(s)+1);
	strcpy(i->value.str, s);
	break;

      case SK_RNG:
	i->value.range = MALLOC(range_t, 1);
	i->value.range->min = va_arg(ap, int);
	i->value.range->max = va_arg(ap, int);
	break;

      case SK_CHR:
	/* 
	  make sure symbol data is globally consistent by not copying symbol
	  passed but referencing it directly.
	*/
	i->value.chr = va_arg(ap,chr_sym_t *);
	break;

      case SK_SUBS:
	i->value.subs = va_arg(ap, _LC_subs_t *);
	break;

      case SK_SYM:
	i->value.sym = va_arg(ap, symbol_t *);
	break;
    }

    va_end(ap);

    return i;
}


/*
*  FUNCTION: destory_item
*
*  DESCRIPTION:
*  Destroys an item created with create_item.  
*
*  This routine free()s memory for the string and range data types.  All 
*  semantic stack items should therefore be created with 'create_item' to
*  ensure malloc()ed memory integrity.  
*/
void destroy_item(item_t *i)
{
    switch (i->type) {
      case SK_INT:
	break;
      case SK_STR:
	free(i->value.str);
	break;
      case SK_RNG:
	free(i->value.range);
	break;
      case SK_SUBS:
      case SK_SYM:
      case SK_CHR:
	/* don't free pointer to symbol data, create_item() did not malloc
	   this memory */
	break;
    }
    
    free(i);
}
