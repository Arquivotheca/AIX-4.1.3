static char sccsid[] = "@(#)81	1.2  src/bos/usr/ccs/lib/libc/insque.c, libcproc, bos411, 9428A410j 3/4/94 10:32:02";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: insque, remque
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>			/* for NULL */


/*
 * NAME:	insque
 *                                                                    
 * FUNCTION:	insert queue element after specified predecessor
 *
 * RETURN VALUE DESCRIPTION:	none
 */  

/*
 *   queue element
 */
struct qelem {
	struct qelem *next;         /* ptr to next queue element     */
	struct qelem *prev;         /* ptr to previous queue element */
	char         data[1];       /* queue data                    */
	};

void
insque ( void *elem_arg, void *pred_arg )
{
	struct qelem *elem = (struct qelem *)elem_arg;
	struct qelem *pred = (struct qelem *)pred_arg;

	if ( pred != NULL ) {
		if ( (elem->next = pred->next) != NULL )
			elem->next->prev = elem;
		elem->prev = pred;
		pred->next = elem;
	} else
		elem->next = elem->prev = NULL;
}

/*
 * NAME:	remque
 *                                                                    
 * FUNCTION:	remove queue element
 *                                                                    
 * RETURN VALUE DESCRIPTION:	none
 */  

void
remque ( void *elem_arg )
{

	struct qelem *elem = (struct qelem *)elem_arg;

	if ( elem->next != NULL )
		elem->next->prev = elem->prev;
	if ( elem->prev != NULL )
		elem->prev->next = elem->next;
} 
