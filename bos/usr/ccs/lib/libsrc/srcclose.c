static char sccsid[] = "@(#)01	1.1  src/bos/usr/ccs/lib/libsrc/srcclose.c, libsrc, bos411, 9428A410j 11/10/89 16:24:44";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	src_close_socket
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
** IDENTIFICATION:
**    Name:	src_close_socket
**    Title:	Close SRC communication port
** PURPOSE:
**    Closes SRC communication port AF_UNIX/AF_INET
** 
** SYNTAX:
**    src_close_socket(src_socket);
**    Parameters:
**       u struct src_socket - src_socket structure;
**
** INPUT/OUTPUT SECTION:
**	SOCKET: fd
**
** PROCESSING:
**	close the socket
**	if socket type is AF_UNIX remove the socket file
**
** RETURNS:
**	None:
**/
#include <stdio.h>
#include <sys/socket.h>
#include "srcsocket.h"
void src_close_socket(src_socket)
struct src_socket *src_socket;
{
	if(src_socket->open == 0)
		return;

	close(src_socket->sock_id);
	src_socket->open=0;

	/* we need to remove the socket file for an AF_UNIX socket */
	if(src_socket->sun.sun_family == AF_UNIX)
		unlink(src_socket->sun.sun_path);
}
