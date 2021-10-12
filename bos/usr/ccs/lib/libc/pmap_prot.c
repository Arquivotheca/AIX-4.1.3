static char sccsid[] = "@(#)96  1.6  src/bos/usr/ccs/lib/libc/pmap_prot.c, libcrpc, bos411, 9428A410j 10/25/93 20:41:22";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: xdr_pmap
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
static char sccsid[] = 	"@(#)pmap_prot.c	1.4 90/07/17 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.18 88/02/08 
 */


/*
 * pmap_prot.c
 * Protocol for the local binder service, or pmap.
 */

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/pmap_prot.h>


bool_t
xdr_pmap(xdrs, regs)
	XDR *xdrs;
	struct pmap *regs;
{

	if (xdr_u_long(xdrs, &regs->pm_prog) && 
		xdr_u_long(xdrs, &regs->pm_vers) && 
		xdr_u_long(xdrs, &regs->pm_prot))
		return (xdr_u_long(xdrs, &regs->pm_port));
	return (FALSE);
}
