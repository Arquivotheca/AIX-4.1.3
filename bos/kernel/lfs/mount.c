static char sccsid[] = "@(#)55	1.25.1.10  src/bos/kernel/lfs/mount.c, syslfs, bos411, 9430C411a 7/26/94 12:56:31";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: vmount, smount, vmountdata, vfslock, vfsunlock
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/pri.h"
#include "string.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/fs_locks.h"
#include "sys/errno.h"
#include "sys/access.h"
#include "sys/pathname.h"
#include "sys/gpai.h"
#include "sys/syspest.h"
#include "sys/malloc.h"
#include "sys/var.h"
#include "sys/priv.h"
#include "sys/audit.h"

#define	MAXVMTSIZE	(MAXPATH * (VMT_LASTINDEX+1))
#define	ROUNDUP(x)	(((x) + 3) & ~3)

BUGVDEF(mdebug, 0);		/* define debug variable */

extern struct galloc gpa_vfs;
extern struct gfs *gfsindex[];	/* AIX virtual file systems */
extern int max_gfstype;

/************************************************************************
 *
 * NAME:	vmount system call
 *
 * FUNCTION:	The new AIX mount for mounting of local devices or
 *		local or remote directories over local directories.
 *		Also mounts local or remote files over local files.
 *
 * PARAMETERS:	vmountp	- pointer to a vmount structure filled in
 *				by a mount helper or the mount command.
 *		vlength	- length of the (variable sized) vmount structure.
 *
 * RETURNS:	zero on success, -1 on error. Also the requested
 *		mount operation is performed.
 *
 ************************************************************************/

vmount(uapvmountp, vlength)
struct vmount	*uapvmountp;	/* vmount structure pointer */
int		vlength;	/* length of vmount struct */
{
	extern          kernel_lock; /* global kernel lock */
	static int      svcnum = 0;
	struct vmount   *vmountp = NULL;
	struct vmt_data *vdp;
	struct vmt_data *endvdp;
	int		error = 0;
	int             lockt,       /* previous state of kernel lock */
	                svcrc = 0,
	                size,
	                offset,
			vfstype;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if (audit_flag)
		svcrc = audit_svcstart("FS_Mount",&svcnum,0);

	BUGLPR(mdebug, BUGNFO, ("Vmount entry: ptr=0x%x, len=%d\n",
		uapvmountp, vlength));

	/*
	 * This is a sanity check: malloc panics, if it gets too
	 * big a number.
	 */
	if (vlength < sizeof(struct vmount) || vlength > MAXVMTSIZE) {
		BUGLPR(mdebug, BUGACT, ("Bad vmount length\n"));
		goto einval;
	}
	if ((vmountp = (struct vmount *)malloc(vlength)) == NULL) {
		BUGLPR(mdebug, BUGACT, ("Malloc error in vmount\n"));
		error = ENOMEM;
		goto out;
	}
	if (copyin(uapvmountp, vmountp, vlength)) {
		error = EFAULT;
		goto out;
	}

	/*
	 * Verify that the system call was correctly invoked.
	 */
	if (vmountp->vmt_length != vlength ||
			vmountp->vmt_revision != VMT_REVISION) {
		goto einval;
	}

	/*
	 * Verify that the memory areas described by the
	 * offset and size fields in the vmount are within
	 * the memory:
	 *	[vmountp, vmountp + vlength)
	 * and check that the fields are not overlapping.
	 * The checks are done assuming that the fields will
	 * be assigned such that:
	 *	fields of size 0 have offset zero
	 *	fileds are integer aligned
	 *	vmountp->vmt_data[i].vmt_off < vmountp->vmt_data[j].vmt_off
	 *		(for i < j)
	 *	there is no U.Used memory at the end of the structure
	 */
	vdp = vmountp->vmt_data;
	endvdp = vdp + VMT_LASTINDEX;
	offset = ROUNDUP (sizeof (struct vmount));
	for (; vdp <= endvdp; vdp++) {
		size = vdp->vmt_size;
		if (size < 0)
			goto einval;
		if (size == 0) {
			vdp->vmt_off = 0;
			continue;
		}
		if (vdp->vmt_off != offset)
			goto einval;
		offset += ROUNDUP (size);
		if (offset >= vlength && vdp != endvdp)
			goto einval;
	}
	if (offset != vlength)
		goto einval;

	/*
	 * The VMT_STUB and VMT_OBJECT fields should be null
	 * terminated strings. The other fields are file system
	 * specific and should be validated by the file system
	 * implementation.
	 */
	if (! memchr (vmt2dataptr (vmountp, VMT_STUB), '\0',
		      vmt2datasize (vmountp, VMT_STUB)) ||
	    ! memchr (vmt2dataptr (vmountp, VMT_OBJECT), '\0',
		      vmt2datasize (vmountp, VMT_OBJECT))) {
einval:		error = EINVAL;
		goto out;
	}
	
	if(svcrc) {
		if (vmt2dataptr(vmountp, VMT_OBJECT))
			audit_svcbcopy(vmt2dataptr(vmountp, VMT_OBJECT),
				  strlen(vmt2dataptr(vmountp, VMT_OBJECT)) + 1);
		if (vmt2dataptr(vmountp, VMT_STUB))
			audit_svcbcopy(vmt2dataptr(vmountp, VMT_STUB),
				  strlen(vmt2dataptr(vmountp, VMT_STUB)) + 1);
	}

	/*
	 * restrict input flags to those that user of vmount() may set.
	 */
	vmountp->vmt_flags &= VMOUNT_MASK;

	/*
	 * Make sure there is a gfs for this type
	 * and put a hold on it so it can't go away.
	 */
	vfstype = vmountp->vmt_gfstype;
	GFS_LOCK();
	if (vfstype < 0 || vfstype > max_gfstype || gfsindex[vfstype] == NULL)
		error = ENOSYS;
	else
		gfsindex[vfstype]->gfs_hold++;
	GFS_UNLOCK();
	if (error)
		goto out;

	error = smount(vmountp);
	
	if (error)
	{
		GFS_LOCK();
		gfsindex[vfstype]->gfs_hold--;
		GFS_UNLOCK();
	}
out:
	if(svcrc)
		audit_svcfinis();

	if (error && vmountp != NULL)
		free(vmountp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (error)
		u.u_error = error;
	return error ? -1 : 0;
}

/*
 * smount -- Common code for all varieties of mount system call.
 */
int
smount(vmountp)
struct vmount	*vmountp;
{
	int error;
	register struct vfs	*vfsp, *tmp;
	extern struct vfs	*rootvfs;
	struct vnode		*stubvp = NULL;
	int			vfstype = vmountp->vmt_gfstype;
	struct ucred		*crp;
	static vfsnumber = 1;	/* stamps a unique number on each vfs */

	BUGLPR(mdebug, BUGACT,
		("Smount entry: type=%d, ptr=0x%x, len=%d bytes\n",
		   vfstype, vmountp, vmountp->vmt_length));

	/* Get creds for the VFS and vnode operations */
	crp = crref();

	/*
	 * Allocate and initialize new vfs struct.
	 */
	if ((vfsp = (struct vfs *) gpai_alloc(&gpa_vfs)) == NULL)
	{
		error = ENOMEM;
		goto outstub;
	}

	/* Initialize vfs structure */
	vfsp->vfs_mdata = vmountp;
	vfsp->vfs_gfs = gfsindex[vfstype];
	vfsp->vfs_date = time;
	vmountp->vmt_vfsnumber = fetch_and_add(&vfsnumber, 1);
	vfsp->vfs_number = vmountp->vmt_vfsnumber;
	vfsp->vfs_vnodes = NULL;
	vfsp->vfs_data = NULL;
	vfsp->vfs_count = 1;
	vfsp->vfs_bsize = 0;
	vfsp->vfs_rsvd1 = 0;
	vfsp->vfs_rsvd2 = 0;

try_again:

	/*
	 * Find the stub to be mounted upon.
	 */
	error = lookupname(vmt2dataptr(vmountp, VMT_STUB),
				SYS, L_SEARCH, NULL, &stubvp, crp);
	if (stubvp == NULL)
	{
		BUGLPR(mdebug, BUGACT, ("stubvp = NULL, error %d\n", error));
		goto outstub;
	}

	/* set stub in the vfs */
	vfsp->vfs_mntdover = stubvp;

	/*      write permission on stub is only permission required    */
	/*       to allow mounting of the object over the stub.         */

	/* VNOP_ACCESS returns error, therefore FALSE (0) means OK */
	if (!VNOP_ACCESS(stubvp, W_ACC, NULL, crp))
		vfsp->vfs_flag |= VFS_VMOUNTOK;	/* kludge to inform vfs_mount()
						   routines of stub permissions;
						   cleared at smount exit. */
	if (priv_req(FS_CONFIG))
		vfsp->vfs_flag |= VFS_SUSER;
	
	/*
	 * Lock the stub vnode to prevent multiple simultaneous
	 * mounts over the same point -- which would result in
	 * unreachable file systems.
	 */
	VN_LOCK(stubvp);
	if (stubvp->v_mvfsp)
	{
		VN_UNLOCK(stubvp);
		VNOP_RELE(stubvp);
		goto try_again;
	}

	/* Restrict mount ala System V for SVVS */
	if (vmountp->vmt_flags & VFS_SYSV_MOUNT)
	{
		if (stubvp->v_flag & V_ROOT || stubvp->v_count > 1)
		{
			error = EBUSY;
			goto outstub;
		}
		vmountp->vmt_flags &= ~VFS_SYSV_MOUNT;
	}

	/*
	 * Switch to file system dependent code.
	 */
	BUGLPR(mdebug, BUGACT, ("Switching to vfs_mount...\n"));
	BUGLPR(mdebug, BUGACT, ("  object: \"%s\", vfsp 0x%x\n",
		vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT), vfsp));

	error = VFS_MOUNT(vfsp, crp);

	if (error)
		goto outstub;

	/*
	 * initialization of the vfs is complete.  
	 * point the stub vnode at the new vfs and
	 * link the new vfs into the list of file systems.
	 */

	stubvp->v_mvfsp = vfsp;
	VN_UNLOCK(stubvp);

	vfsp->vfs_flag &= ~(VFS_VMOUNTOK|VFS_SUSER);

	VFS_LIST_LOCK();
	for (tmp = rootvfs; tmp->vfs_next != NULL; tmp = tmp->vfs_next)
		/* null */;
	tmp->vfs_next = vfsp;
	vfsp->vfs_next = NULL;
	VFS_LIST_UNLOCK();

	crfree(crp);
	return(0);

outstub:
	if (stubvp)
	{
		VN_UNLOCK(stubvp);
		VNOP_RELE(stubvp);
	}
	if (vfsp)
	{
		gpai_free(&gpa_vfs, vfsp);
	}
	crfree(crp);
	return(error);
}

/*
 *	vmountdata - stuffs it's arguments into the vmount
 *	structure pointed at by vmtp.  Only used by jfs_rootinit,
 *	who has to hand-build a vmount structure.
 */
vmountdata(vmtp, obj, stub, host, name, info, args)
register struct vmount	*vmtp;
char			*obj, *stub, *host, *name, *info, *args;
{
	register struct vmt_data *vdp, *vdprev;
	register int	size;

	BUGLPR(mdebug, BUGACT,
		("Entry to vmountdata 0x%x %s %s %s %s %s %s\n",
		vmtp, obj, stub, host, name, info, args));
	vdp = vmtp->vmt_data;

	vdp->vmt_off = sizeof(struct vmount);
	size = ROUNDUP(strlen(obj) + 1);
	vdp->vmt_size = size;
	strcpy(vmt2dataptr(vmtp, VMT_OBJECT), obj);

	vdprev = vdp;
	vdp++;
	vdp->vmt_off =  vdprev->vmt_off + size;
	size = ROUNDUP(strlen(stub) + 1);
	vdp->vmt_size = size;
	strcpy(vmt2dataptr(vmtp, VMT_STUB), stub);

	vdprev = vdp;
	vdp++;
	vdp->vmt_off =  vdprev->vmt_off + size;
	size = ROUNDUP(strlen(host) + 1);
	vdp->vmt_size = size;
	strcpy(vmt2dataptr(vmtp, VMT_HOST), host);

	vdprev = vdp;
	vdp++;
	vdp->vmt_off =  vdprev->vmt_off + size;
	size = ROUNDUP(strlen(name) + 1);
	vdp->vmt_size = size;
	strcpy(vmt2dataptr(vmtp, VMT_HOSTNAME), name);

	vdprev = vdp;
	vdp++;
	vdp->vmt_off =  vdprev->vmt_off + size;
	size = ROUNDUP(strlen(info) + 1);
	vdp->vmt_size = size;
	strcpy(vmt2dataptr(vmtp, VMT_INFO), info);

	vdprev = vdp;
	vdp++;
	vdp->vmt_off =  vdprev->vmt_off + size;
	size = ROUNDUP(strlen(args) + 1);
	vdp->vmt_size = size;
	strcpy(vmt2dataptr(vmtp, VMT_ARGS), args);
	BUGLPR(mdebug, BUGACT, ("Exit from vmountdata\n"));
}
