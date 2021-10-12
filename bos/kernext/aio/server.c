static char sccsid[] = "@(#)06	1.6  src/bos/kernext/aio/server.c, sysxaio, bos411, 9428A410j 11/15/93 14:24:02";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: new_server, add_server, get_free_server, find_work
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "aio_private.h"

/* new_server -- create a new server record and stick it in place
 *
 * allocate the server record
 * create the kproc
 * initialize the server record
 * initialize the kproc
 */
server *
new_server(void)
{
	pid_t	s_pid;
	server *sp;

	/*
	 * I need to be holding the servers lock upon entry
	 */
	ASSERT(lock_mine(&servers.lock));

	if (!(sp = (server *)malloc(sizeof(server))))
		return NULL;

	if ((s_pid = creatp()) == -1) {
		free(sp);
		return NULL;
	}

	sp->qp     = NULL;
	sp->next   = NULL;
	sp->tid    = -1;

	if (initp(s_pid, server_main, &sp, sizeof(server **), "aioserver")) {
		free(sp);
		return NULL;
	}

	/* 
	 * set the priority of the kproc
	 */
	(void)setpri(s_pid, s_priority);

	servers.scount++;

	return sp;
}

/* add_server -- stick a server record onto the avail list
 *
 * insert the server at the head
 */
void
add_server(server *sp)
{
	ASSERT(lock_mine(&servers.lock));
	sp->next = servers.s_avail;
	sp->qp = NULL;
	servers.s_avail = sp;
}

tid_t
get_free_server(queue *qp)
{
	server *sp = NULL;
	tid_t	s_tid = -1;

	/* We need to make sure we are holding the
	 * global queue lock and the queue lock since 
	 * we must guarantee that no 
	 * other server can be attached to this queue
	 */
	ASSERT(lock_mine(&aio_qlock));
	ASSERT(lock_mine(&qp->lock));

	SERVERS_LOCK();
	if (sp = servers.s_avail)
		/*
		 * there's a server available for work
		 * take it off the avail list
		 */
		servers.s_avail = servers.s_avail->next;
	else if (servers.scount < maxservers) {
		/*
		 * no available servers but we can create a new one
		 */
		sp = new_server();
		/*
		 * delay until new server is initialized
		 */
		while (fetch_and_add(&sp->tid,0) == -1)
			delay(1);
	}

	SERVERS_UNLOCK();

	/*
	 * if we got a server, assign it to this queue
	 */
	if (sp) {
		sp->qp = qp;
		sp->next = NULL;
		s_tid = sp->tid;
		qp->s_count++;
	}

	return s_tid;
}

queue *
find_work(void)
{
	int i;
	
	for (i = 0; i < QTABSIZ; ++i)
		if (qtab[i].s_count < qtab[i].req_count)
			return &qtab[i];
	return NULL;
}
