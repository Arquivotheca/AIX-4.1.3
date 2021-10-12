static char sccsid[] = "@(#)69	1.10  src/bos/kernext/dlc/head/dlcrq.c, sysxdlcg, bos411, 9428A410j 10/19/93 11:09:42";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlc_rqput() dlc_rqget() dlc_rqdelete() dlc_rqcreate()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/gdlextcb.h>
#include "dlcrq.h"
#include "dlcadd.h"

struct s_mpx_tab   dlcsum;      /* global table of the channels and adapters */

struct ring_queue * dlc_rqcreate()
{
struct ring_queue *qaddr;
int amt;

	/* LEHb defect 44460 */
	qaddr = xmalloc(((dlcsum.maxq)*sizeof(struct que_entry))+
			  sizeof(struct ring_head), 4, pinned_heap);
	/* LEHe */
	if (qaddr == (struct ring_queue *)DLC_NULL)
		return(DLC_NULL);
	qaddr->ring_head.in = (ulong_t)&(qaddr->que_entry.entry[0]);
	qaddr->ring_head.out = (ulong_t)&(qaddr->que_entry.entry[0]);
	/* LEHb defect 44460 */
	qaddr->ring_head.end = (ulong_t)&(qaddr->que_entry.entry[0])+
				 ((dlcsum.maxq)*sizeof(struct que_entry));
	/* LEHe */
	return (qaddr);
}

void dlc_rqdelete(qaddr)
struct ring_queue *qaddr;
{
	xmfree(qaddr, pinned_heap);
}

int dlc_rqput(qaddr, entaddr)
struct ring_queue *qaddr;
caddr_t entaddr;
{
caddr_t tout;

	tout = (caddr_t)qaddr->ring_head.out+sizeof(struct que_entry);
      	if /* the output pointer is at the end of the ring queue */
      	(tout == (caddr_t)qaddr->ring_head.end) {
         	/* set the output pointer to top of ring queue. */
         	tout = (caddr_t)&(qaddr->que_entry.entry[0]);
		if  /* the ring is full */
		(tout == (caddr_t)qaddr->ring_head.in)
			return(-1);
		bcopy(entaddr, qaddr->ring_head.out, sizeof(struct que_entry));
		qaddr->ring_head.out = (ulong_t)tout;
      	} else {
		if /* the ring is full */
		(tout == (caddr_t)qaddr->ring_head.in)
			return(-1);
		
		bcopy(entaddr, qaddr->ring_head.out, sizeof(struct que_entry));
		qaddr->ring_head.out = (ulong_t)tout;
      	}
	return(0);
}

int dlc_rqget(qaddr, entaddr)
struct ring_queue *qaddr;
caddr_t entaddr;
{
	if /* the ring queue is empty */
	(qaddr->ring_head.in == qaddr->ring_head.out)
		return(-1);

	bcopy(qaddr->ring_head.in, entaddr, sizeof(struct que_entry));

      	if /* the in pointer is at the end of the ring queue */
      	(qaddr->ring_head.in+sizeof(struct que_entry) == qaddr->ring_head.end)
         	/* set the in pointer to the top of the ring queue. */
         	qaddr->ring_head.in = (ulong_t)&(qaddr->que_entry.entry[0]);
      	else
		qaddr->ring_head.in += sizeof(struct que_entry);
	return(0);
}
