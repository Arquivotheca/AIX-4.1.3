/* @(#)00	1.7  src/bos/usr/bin/src/include/srcsocket.h, cmdsrc, bos411, 9428A410j 4/16/91 11:09:03 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
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


#ifndef _H_SRCSOCKET
#define _H_SRCSOCKET

/* base name for afunix family */
#define SRC_BASE_DIR_AF_UNIX	"/dev/.SRC-unix/"
#define SRC_BASE_AF_UNIX	"/dev/.SRC-unix/SRCXXXXXX"
#define SRC_DESTROY_TMP_SOCKETS	"rm -rf /dev/.SRC-unix"
#define SRC_MASTER_AF_UNIX	"/dev/SRC"
#define MAXSOCKBUFSIZE		45000

#include <sys/types.h>
#include <sys/un.h>
/* SRC socket management stucture */
struct src_socket
{
	int sock_id; 		/* Socket file descriptor */
	int open; 		/* Socket is open */
	struct sockaddr_un sun;	/*  socket Address used sockaddr_un because
				 *  it takes up the largest amount of 
				 *  space (this could be a union of them?)
				 */
};

#endif
