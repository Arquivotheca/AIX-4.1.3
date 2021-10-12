/* @(#)99	1.1  src/bos/usr/bin/src/cmds/srcselect.c, cmdsrc, bos411, 9428A410j 11/10/89 16:13:28 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS: src_local_or_remote
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <src.h>
#include <srcsocket.h>

static int maxfd;
static fd_set sockmask;

int src_local_or_remote(src_socket)
struct src_socket src_socket[];
{
	int rc;

	/* wait for something to come in on our socket 
	 * ignore any signal interups that we may have received
	 */
	do {
		FD_ZERO(&sockmask);
		/* do we have remote operations ? */
		if(src_socket[1].open!=0)
			FD_SET(src_socket[1].sock_id, &sockmask);

		FD_SET(src_socket[0].sock_id,&sockmask);
		if(src_socket[0].sock_id > src_socket[1].sock_id)
			maxfd=src_socket[0].sock_id;
		else
			maxfd=src_socket[1].sock_id;
		rc=select(maxfd+1,&sockmask,0,0,0);
		if(rc == -1 && errno != EINTR)
			logerr(0,0,SRC_SOCK,errno);
	   } while(rc == -1);

	/* do we have a remote request? */
	if(FD_ISSET(src_socket[1].sock_id,&sockmask))
		return(src_socket[1].sock_id);

	/* we have a local request? */
	return(src_socket[0].sock_id);

}
