static char sccsid[] = "@(#)90	1.1  src/bos/kernel/lib/libsysp/insque2.c, libsysp, bos411, 9428A410j 12/13/89 08:56:27";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: insque2, remque2
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

/* Struct used to manipulate circular doulbly linked lists when the
 * pointers of interest are at offset 8.
 */
struct args2 {
	struct args2 *x[2];	/* unused */
	struct args2 *forw;	/* forward pointer */
	struct args2 *back;	/* back pointer */
};

/*
 * Insert into queue, where the queue pointers are
 * in the second two longwords.
 * Should be in assembler.
 */
void
insque2 (struct args2 *p2, struct args2 *p1)
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
remque2 (struct args2 *p)
{
	p->back->forw = p->forw;
	p->forw->back = p->back;
}
