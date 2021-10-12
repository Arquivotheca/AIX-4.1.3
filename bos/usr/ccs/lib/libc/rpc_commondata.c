static char sccsid[] = "@(#)18  1.1  src/bos/usr/ccs/lib/libc/rpc_commondata.c, libcrpc, bos411, 9428A410j 10/25/93 20:53:55";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)rpccommondata.c	1.3 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * @(#) from SUN X.X
 */

#include <rpc/rpc.h>
/*
 * This file should only contain common data (global data) that is exported 
 * by public interfaces 
 */
struct opaque_auth _null_auth;
fd_set svc_fdset;
struct rpc_createerr rpc_createerr;
