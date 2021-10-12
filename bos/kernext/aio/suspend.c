static char sccsid[] = "@(#)07  1.8  src/bos/kernext/aio/suspend.c, sysxaio, bos41J, 9521A_all 5/23/95 16:12:35";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: suspend_set
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "aio_private.h"

/*
 * suspend_set -- set the suspend bit of the request which
 *		  matches the control block passed in.
 */
int
suspend_set(int cnt, struct aiocb *cbpa[], pid_t pid, suspender *susp)
{
	struct aiocb   *cbp;
	int		all_null = 1; /* is every pointer in the array NULL */
	queue	       *qp;
	request	       *rp;
	int		cba_index;
	int		rc;
	int 		 i;

	/*
	 * For each aiocb we want to set the suspend bit on.
	 */
	for (cba_index = 0; cba_index < cnt; ++cba_index) {

		/*
		 * make sure the pointer is not NULL. If
		 * it is, simply continue to the next aiocb
		 */
		if (!(cbp = cbpa[cba_index]))
			continue;

		/*
		 * If we found a non-NULL aiocb pointer,
		 * set all_null to zero.
		 */
		all_null = 0;

		if (arl_suspend(susp, cbp, pid))
			continue;       /* the request was found, search
					 * for the next request
					 */

		/* 
		 * To check if we have a matching request on
		 * the queues, instead of using find_queue() which
		 * requires a validated file pointer, we 
		 * simply search all the queues looking for requests
		 * with matching aiocb pointers.
		 */
		AIOQ_LOCK();
		for (i = 0; i < QTABSIZ; ++i) {
			qp = &qtab[i];

			QUEUE_LOCK(qp);

			/*
			 * If we found the request on this queue.
			 */
			if (rp = find_request(qp, cbp, pid)) {
				/* 
				 * If the request in not done
				 * set up a suspender on that request
				 */
				if (rp->inprog != AIO_DONE) {
					rp->susp_gen = susp->gen;
					QUEUE_UNLOCK(qp);
					break;
				} else {
					/*
					 * The request has completed 
					 * so we return immediately.
					 */
					remove_suspender(susp);
					rc = cba_index;
					QUEUE_UNLOCK(qp);
					AIOQ_UNLOCK();
					goto exit;
				}

			/*
			 * If we didn't find the request on this queue,
			 * we need to go the next queue.
			 */
			} else {
				QUEUE_UNLOCK(qp);
				continue;
			}

		}

		if (i == QTABSIZ) {
			/*
		 	 * We never found the request on
		 	 * any queue. Remove ourselves from
		 	 * the list of supenders and return
		 	 * the index number.
		 	 */
			remove_suspender(susp);
			rc = cba_index;
			AIOQ_UNLOCK();
			goto exit;
		}

		AIOQ_UNLOCK();
	}

	if (all_null)
		rc = SS_ALL_NULL;
	else
		rc = SS_OK;

      exit:
	return rc;
}
