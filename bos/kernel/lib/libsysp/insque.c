static char sccsid[] = "@(#)89	1.1  src/bos/kernel/lib/libsysp/insque.c, libsysp, bos411, 9428A410j 12/13/89 08:55:55";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: insque, remque
 *
 * ORIGINS: 27
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
 */

/* Struct used to manipulate circular doubly linked lists when the
 * pointers of interest are at offset 0.
 */
struct args {
	struct args *forw;	/* forward pointer */
	struct args *back;	/* back pointer */
};

/*
 * Insert into queue, where the queue pointers are
 * in the first two longwords.
 * Should be in assembler.
 */
void
insque (struct args *p2, struct args *p1)
{
	p2->forw = p1->forw;
	p2->back = p1;
	p1->forw->back = p2;
	p1->forw = p2;
}

/*
 * Remove from queue
 */
void
remque(struct args *p)
{
	p->back->forw = p->forw;
	p->forw->back = p->back;
}
