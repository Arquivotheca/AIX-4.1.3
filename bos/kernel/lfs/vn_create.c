static char sccsid[] = "@(#)88	1.5.1.2  src/bos/kernel/lfs/vn_create.c, syslfs, bos411, 9428A410j 10/22/93 17:33:24";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: vn_create
 *
 * ORIGINS: 24, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* define _SUN to pick up Sun declaration of enum vcexcl */

#define	_SUN

#include "sys/param.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/stat.h"

/*
 * create a vnode
 *
 * This function is used only for binding a unix domain socket 
 * to a name in the file system.
 */
int
vn_create(pnamep, seg, vap, excl, mode, vpp)
char *pnamep;
int seg;		/* UIO_SYSSPACE, UIO_USERSPACE, ... */
struct vattr *vap;
enum vcexcl excl;	/* EXCL, NONEXCL */
int mode;		/* VREAD, VWRITE */
struct vnode **vpp;
{
	struct vnode *dvp;	/* ptr to parent dir vnode */
	struct vnode *vp;	/* ptr to vnode for created object */
	struct pathname pn;
	register int error;
	struct ucred *crp;

	/* This interface only available for exclusive use sockets */

	if( vap->va_type != VSOCK )
		return EINVAL;

	*vpp = dvp = vp = (struct vnode *)0;

	/* fetch the pathname */

	if( error = pn_get(pnamep, seg, &pn) )
		return error;
	
	/* get credentials for vnode ops */

	crp = crref();

	/* look it up to get the directory vnode */

	if( error = lookuppn(&pn, L_CREATE|L_NOFOLLOW, &dvp, &vp, crp) )
	{
		pn_free(&pn);
		goto out;
	}

	/*
	* Since the AIX VNOP_CREATE interface does not permit the
	* creation of arbitrary objects, this code calls VNOP_MKNOD.
	*/

	error = VNOP_MKNOD(dvp,pn.pn_path,vap->va_mode|S_IFSOCK,0,crp);
	VNOP_RELE(dvp);
	pn_free(&pn);
	if( error )
		goto out;	/* VNOP_MKNOD failed */

	/*
	* Since the caller wants a vnode and VNOP_MKNOD does not
	* return one, we have to call lookup again.
	*/

	if( error = pn_get(pnamep, seg, &pn) )	/* shouldn't fail */
		goto out;

	if( (error = lookuppn(&pn,L_NOFOLLOW,&dvp,vpp,crp)) == 0 )
		VNOP_RELE(dvp);

	pn_free(&pn);

out:
	crfree(crp);
	return error;
}
