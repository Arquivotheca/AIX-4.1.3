static char sccsid[] = "@(#)08  1.2  src/bos/usr/ccs/lib/libc/authunix_prot.c, libcrpc, bos411, 9428A410j 2/2/94 11:00:50";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: xdr_authkern
 *		xdr_authunix_parms
 *		
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
static char sccsid[] = 	"@(#)authunix_prot.c	1.5 90/07/17 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.17 88/02/08
*/

/*
 * authunix_prot.c
 * XDR for UNIX style authentication parameters for RPC
 */

#ifdef _KERNEL
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/cred.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#else
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#endif


/*
 * XDR for unix authentication parameters.
 */
bool_t
xdr_authunix_parms(xdrs, p)
	register XDR *xdrs;
	register struct authunix_parms *p;
{

	if (xdr_u_long(xdrs, &(p->aup_time))
	    && xdr_string(xdrs, &(p->aup_machname), MAX_MACHINE_NAME)
	    && xdr_u_int(xdrs, &(p->aup_uid))
	    && xdr_u_int(xdrs, &(p->aup_gid))
	    && xdr_array(xdrs, (caddr_t *)&(p->aup_gids),
		    &(p->aup_len), NGRPS, sizeof(int), xdr_u_int) ) {
		return (TRUE);
	}
	return (FALSE);
}

#ifdef _KERNEL
/*
 * XDR kernel unix auth parameters.
 * NOTE: this is an XDR_ENCODE only routine.
 */
xdr_authkern(xdrs,tcred)
	register XDR *xdrs;
	struct	ucred *tcred;
{
	gid_t	*gp;
	u_int	 uid;
	u_int	 gid;
	int	 len;
	int	 groups[NGROUPS];
	int	hostnamelen=MAXHOSTNAMELEN;
	char	hostname[MAXHOSTNAMELEN+1];
	char	*name = hostname;

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}

	uid = tcred->cr_uid;
	gid = tcred->cr_gid;
	gp = tcred->cr_groups;

	kgethostname(name, &hostnamelen);

	len = (tcred->cr_ngrps > NGRPS ? NGRPS : tcred->cr_ngrps);

        if (xdr_u_long(xdrs, (u_long *)&time)
            && xdr_string(xdrs, &name, MAX_MACHINE_NAME)
            && xdr_int(xdrs, &uid)
            && xdr_int(xdrs, &gid)
	    && xdr_array(xdrs, (caddr_t)gp, (u_int *)&len, NGRPS, sizeof (int), xdr_u_int) ) {
                return (TRUE);
	}
	return (FALSE);
}
#endif
