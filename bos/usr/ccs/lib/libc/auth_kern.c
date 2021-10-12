static char sccsid[] = "@(#)73  1.13  src/bos/usr/ccs/lib/libc/auth_kern.c, libcrpc, bos411, 9428A410j 2/2/94 11:00:03";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: authkern_create
 *		authkern_destroy
 *		authkern_marshal
 *		authkern_nextverf
 *		authkern_refresh
 *		authkern_validate
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

#ifdef	_KERNEL

/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)auth_kern.c	1.7 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.23 88/02/08 
 */


/*
 * auth_kern.c, Implements UNIX style authentication parameters in the kernel. 
 *
 * Interfaces with svc_auth_unix on the server.  See auth_unix.c for the user
 * level implementation of unix auth.
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#include <netinet/in.h>

/*
 * Unix authenticator operations vector
 */
void	authkern_nextverf();
bool_t	authkern_marshal();
bool_t	authkern_validate();
bool_t	authkern_refresh();
void	authkern_destroy();

static struct auth_ops auth_kern_ops = {
	authkern_nextverf,
	authkern_marshal,
	authkern_validate,
	authkern_refresh,
	authkern_destroy
};


/*
 * Create a kernel unix style authenticator.
 * Returns an auth handle.
 */
AUTH *
authkern_create()
{
	register AUTH *auth;

	/*
	 * Allocate and set up auth handle
	 */
	auth = (AUTH *)kmem_alloc((u_int)sizeof(*auth));
	if ( auth != NULL ) {
		pin(auth, sizeof(*auth));
		auth->ah_ops = &auth_kern_ops;
		auth->ah_cred.oa_flavor = AUTH_UNIX;
		auth->ah_verf = _null_auth;
	}
	return (auth);
}

/*
 * authkern operations
 */
/*ARGSUSED*/
void
authkern_nextverf(auth)
	AUTH *auth;
{

	/* no action necessary */
}

bool_t
authkern_marshal(auth, xdrs)
	AUTH *auth;
	XDR *xdrs;
{
	char	*sercred;
	XDR	xdrm;
	struct	opaque_auth *cred;
	bool_t	ret = FALSE;
	register int gidlen, credsize;
	register long *ptr;
	int	index;
	int	hostnamelen=MAXHOSTNAMELEN;
	char	hostname[MAXHOSTNAMELEN+1];
	struct	timestruc_t	ltime;
	struct	ucred	*cur_cred = (struct ucred *)auth->ah_private;

	curtime(&ltime);
	gidlen = (cur_cred->cr_ngrps > NGRPS ? NGRPS : cur_cred->cr_ngrps);
	kgethostname(hostname, &hostnamelen);
	hostnamelen = strlen(hostname);
	credsize = 4 + 4 + roundup(hostnamelen, 4) + 4 + 4 + 4 + gidlen * 4;
	ptr = XDR_INLINE(xdrs, 4 + 4 + credsize + 4 + 4);
	if (ptr) {
		/*
		 * We can do the fast path.
		 */
		IXDR_PUT_LONG(ptr, AUTH_UNIX);	/* cred flavor */
		IXDR_PUT_LONG(ptr, credsize);	/* cred len */
		IXDR_PUT_LONG(ptr, ltime.tv_sec);
		IXDR_PUT_LONG(ptr, hostnamelen);
		bcopy(hostname, (caddr_t)ptr, (u_int)hostnamelen);
		ptr += roundup(hostnamelen, 4) / 4;
		IXDR_PUT_LONG(ptr, cur_cred->cr_uid);
		IXDR_PUT_LONG(ptr, cur_cred->cr_gid);
		IXDR_PUT_LONG(ptr, gidlen);
		for (index = 0; index < gidlen; index++)
			IXDR_PUT_LONG(ptr, (u_long)cur_cred->cr_groups[index]); 
		IXDR_PUT_LONG(ptr, AUTH_NULL);	/* verf flavor */
		IXDR_PUT_LONG(ptr, 0);	/* verf len */
		return (TRUE);
	}
	sercred = (char *)kmem_alloc((u_int)MAX_AUTH_BYTES);
	/*
	 * serialize u struct stuff into sercred
	 */
	xdrmem_create(&xdrm, sercred, MAX_AUTH_BYTES, XDR_ENCODE);
	if (! xdr_authkern(&xdrm,cur_cred)) {
		ret = FALSE;
		goto done;
	}

	/*
	 * Make opaque auth credentials that point at serialized u struct
	 */
	cred = &(auth->ah_cred);
	cred->oa_length = XDR_GETPOS(&xdrm);
	cred->oa_base = sercred;

	/*
	 * serialize credentials and verifiers (null)
	 */
	if ((xdr_opaque_auth(xdrs, &(auth->ah_cred)))
	    && (xdr_opaque_auth(xdrs, &(auth->ah_verf)))) {
		ret = TRUE;
	} else {
		ret = FALSE;
	}
done:
	kmem_free((caddr_t)sercred, (u_int)MAX_AUTH_BYTES);
	return (ret);
}

/*ARGSUSED*/
bool_t
authkern_validate(auth, verf)
	AUTH *auth;
	struct opaque_auth verf;
{

	return (TRUE);
}

/*ARGSUSED*/
bool_t
authkern_refresh(auth)
	AUTH *auth;
{
	return (FALSE);
}

void
authkern_destroy(auth)
	register AUTH *auth;
{

	auth->ah_private = 0;   /* paranoia: don't want to debug using a
				 * cred that's not held */
	unpin(auth, sizeof(*auth));
	kmem_free((caddr_t)auth, (u_int)sizeof(*auth));
}
#endif	/* _KERNEL */
