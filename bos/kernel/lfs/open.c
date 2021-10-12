static char sccsid[] = "@(#)06	1.26.1.13  src/bos/kernel/lfs/open.c, syslfs, bos41J, 9510A_all 12/21/94 18:20:52";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: open, creat, copen, fp_open, openpath, openpnp, fp_xopen
 *
 * ORIGINS: 3, 27
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
 *
 */

#include "sys/lockf.h"
#include "sys/syspest.h"
#include "sys/proc.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/sysinfo.h"
#include "sys/syspest.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/vattr.h"
#include "sys/pathname.h"
#include "sys/stat.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/fp_io.h"
#include "sys/trchkid.h"

/*
 * DEBUG !!!
 */
BUGVDEF(opndbg, 0);
BUGVDEF(rodbg, 0);

extern struct fileops vnodefops;

/*
 * NAME:	open()		(system call entry point)
 *
 * FUNCTION:	Calls copen routine. Eventually verifies access to a file
 *		or calls the device open routine to do so.
 *
 * PARAMETERS:	fname, oflags, mode. Fname is the name of the
 *		desired file. Oflags, is the type of open this is, FREAD,
 *		FWRITE, etc. Mode is the mode the file should be created with
 * 		if this is an FCREAT.
 *
 * RETURNS:	Any errors that may occur are reported. If there
 *		are no errors open will return a valid file descriptor 
 * 		that may be used by user process to reference this open file.
 */

/*
 * Debug defs.
 */
BUGVDEF(copen_debug, 0);

open(fname, oflags, mode)
char	*fname;
int	oflags;
int	mode;
{
	int fd;			/* file descriptor to be returned       */
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	BUGLPR(copen_debug, BUGACT, ("uapargs: fname=%s, oflags=%d, mode=%d\n",
				     fname, oflags, mode));

	rc = copen(fname,
		  oflags,
		  mode,
		  (caddr_t)0,
		  &fd);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : fd;
}

/*
 * NAME:	openx()		(system call entry point)
 *
 * FUNCTION:	Calls copen routine. Eventually verifies access to a file
 *		or calls the device open routine to do so.
 *
 * PARAMETERS:	fname, oflags, mode, ext. Fname is the name of the
 *		desired file. Oflags, is the type of open this is, FREAD,
 *		FWRITE, etc. Mode is the mode the file should be created with
 * 		if this is an FCREAT. Ext is external data passed to the
 * 		device driver.
 *
 * RETURNS:	Any errors that may occur are reported. If there
 *		are no errors open will return a valid file descriptor 
 * 		that may be used by user process to reference this open file.
 */

openx(fname, oflags, mode, ext)
char	*fname;
int	oflags;
int	mode;
caddr_t ext;
{
	int fd;			/* file descriptor to be returned       */
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	BUGLPR(copen_debug, BUGACT, ("uapargs: fname=%s, oflags=%d, mode=%d\n",
				     fname, oflags, mode));

	rc = copen(fname,
		  oflags,
		  mode,
		  ext,
		  &fd);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : fd;
}

/*
 * NAME:	creat() 	(system call entry point)
 *
 * FUNCTION:	Calls copen, eventually creates and opens a file.
 *
 * PARAMETERS:	fname, mode. Fname is the name of the new file. Mode
 *		is the new mode for said file.
 *
 * RETURNS:	Any errors that may occur are reported. If there
 *		are no errors creat will return a valid file descriptor 
 * 		that may be used by user process to reference this newly 
 *		open file.
 */
creat(fname, mode)
char	*fname;
int	mode;
{
	int fd;			/* file descriptor to be returned       */
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	rc = copen(fname,
		  O_WRONLY|O_CREAT|O_TRUNC,
		  mode,
		  NULL,
		  &fd);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : fd;
}

/*
 * NAME: copen()
 *
 * FUNCTION: Common code shared by open and create. Calls fp_open to
 * 	     do the actual work.
 *
 * PARAMETERS: path, flags, mode, ext. Path is the pathname of the file.
 *	       Flags is the open flags (FREAD, FTRUNC, etc.). Mode is the
 * 	       create mode of the file. Ext is external data used by
 *	       some of the device drivers.
 *
 * RETURN VALUES:	Explicitly returns any error codes that occur. Will
 * 			also set the user's file descriptor array entry and
 *			return the index for this open file in fdp.
 */

static
copen(path, flags, mode, ext, fdp)
char *		path;		/* name of file to open			*/
int		flags;		/* file open flags			*/
int		mode;		/* permission bits to create file with	*/
int		ext;		/* device driver ext value		*/
int *		fdp;		/* address to return file descriptor to	*/
{
	int		rc;	/* return code value			*/
	int		fd;	/* file descriptor of opened file	*/
	struct file *	fp;	/* file pointer of opened file		*/
	int		tlock;	/* is multi-thread locking required?	*/
	static int	svcnum = 0;	
	int		svcrc = 0;

	if (audit_flag)
		svcrc = audit_svcstart("FILE_Open", &svcnum, 2, flags, 
				(flags & O_CREAT ? mode : 0));

	/* allocate a user file descriptor */
	rc = ufdalloc(0, &fd);

	if(svcrc){
		audit_svcbcopy(&fd, sizeof(fd));
                if(path){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
				audit_svcfinis();	
				return (ENOMEM);
                        }
                        else if(copyinstr(path, ptr, MAXPATHLEN, &len)){
				free(ptr);
				audit_svcfinis();	
				return (EFAULT);
                        }
                        else
                        	audit_svcbcopy(ptr, len - 1);
                        free(ptr);
                }
		audit_svcfinis();
	}

	if(rc)
		return (rc);

	if ((rc = openpath(path, flags & ~FKERNEL, mode, ext, FP_USR, &fp)))
	{
		ufdfree(fd);
		return rc;
	}

	/* trace file descriptor and open mode	*/
	if (ext)
                TRCHKL3T(HKWD_SYSC_OPENX, fd, flags, mode);
	else
                TRCHKL3T(HKWD_SYSC_OPEN, fd, flags, mode);

	/* set the return value, and
	 * initialize the file pointer in the u block */
	*fdp = fd;
	U.U_ufd[fd].fp = fp;
	return 0;
}

int
fp_open(
char *		path,		/* name of file to open			*/
int		oflags,		/* open() style flags			*/
int		mode,		/* permission bits to create file with	*/
int		ext,		/* device driver ext value		*/
int		fpflag,		/* file pointer flags from fp_io.h	*/
struct file **	fpp)		/* address to return file pointer to	*/
{
	int rc;
	int klock;              /* save kernel_lock state */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	rc = openpath(path,oflags|FKERNEL,mode,ext,fpflag,fpp);

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}

static
openpath(path,oflags,mode,ext,fpflag,fpp)
char *		path;
int		oflags;
int		mode;
int		ext;
int		fpflag;
struct file **	fpp;
{
	struct pathname pn;
	int rc;

	for (;;)
	{
		if ((rc = pn_get(path, fpflag & FP_SYS ? SYS : USR, &pn)) != 0)
			return rc;

		rc = openpnp(&pn, oflags, mode, ext, fpp);
		pn_free(&pn);

		/* Open retry logic is signaled by special errno ERESTART */
		if (rc != ERESTART)
			break;
	}
	return rc;
}

static
int
openpnp(pnp, oflags, mode, ext, fpp)
struct pathname *	pnp;	/* pathname structure of file to open	*/
int			oflags;	/* open() style flags			*/
int			mode;	/* permission bits to create file with	*/
int			ext;	/* device driver ext value		*/
struct file **		fpp;	/* address to return file pointer to	*/
{
	register int rc;	/* return code				*/
	int lookup_mode;	/* modes for lookup call		*/
	int flags;		/* converted open flags			*/
	struct vnode *dvp;	/* directory vnode pointer		*/
	struct vnode *vp;	/* vnode ptr for the file itself	*/
	struct file *fp;	/* file structure pointer		*/
	int saveflags;		/* save the flags paramter		*/
	struct vattr va;	/* vattr to use in getattr		*/
	static int tcbmod = 0;
	static int tcbleak = 0;
	int tcbread = 0;
	int tcbwrite = 0;
	struct ucred *crp;

	BUGLPR(opndbg,BUGNTF,("copen(path=%s, flags=0%o, mode=0%o, ext=0x%x)\n",
				pnp->pn_path, flags,mode,ext));

	/* convert open flags to file flags */
	flags = oflags & ~O_ACCMODE;

	switch (oflags &  O_ACCMODE)
	{
	case O_RDONLY:
		flags |= FREAD;
		break;
	case O_WRONLY:
		flags |= FWRITE;
		break;
	case O_RDWR:
		flags |= (FREAD|FWRITE);
		break;
	case O_NONE:
		break;
	}

	/* If we're not here to do anything get out */
	if ((flags & (FREAD | FWRITE | FEXEC)) == 0)
		return (EINVAL);

	vp = dvp = NULL;

	/* allocate a file pointer to use */
	if ((rc = fpalloc (NULL, flags & FMASK, DTYPE_VNODE, &vnodefops, &fp)))
		return rc;
	
	/* get the credentials to use for file access */
	crp = crref();

	/* the lookup method is strange based on what flags is. */
	lookup_mode =	(flags & FCREAT) ?
				(flags & FEXCL) ?
					L_CREATE | L_NOFOLLOW
				:
					L_CREATE
			:	(flags & (FWRITE|FTRUNC)) ?
					L_SEARCH | L_EROFS
				:
					L_SEARCH;


	/* associate the path name with a vnode, saving the parent dir */
	rc = lookuppn(pnp, (lookup_mode | L_OPEN), &dvp, &vp, crp);

	if (rc)
		goto error;

	if (vp != NULL)
	{
		if (flags & FEXEC)
		{
			/* disallow open for exec on non-regular files */
			if (vp->v_type != VREG)
			{
				rc = EACCES;
				goto error;
			}
		}

		/* enforce nodev mount option */
		if (vp->v_vfsp->vfs_flag & VFS_NODEV &&
				((vp->v_type == VBLK) ||
				(vp->v_type == VCHR) ||
				(vp->v_type == VMPC)))
		{
			rc = EACCES;
			goto error;
		}

		/* disallow open of sockets in the file system name space */
		if (vp->v_type == VSOCK)
		{
			rc = EOPNOTSUPP;
			goto error;
		}
	}

	if((flags & (FWRITE))		&&
	(vp) 				&&
	(vp->v_gnode->gn_flags & GNF_TCB)){

		tcbwrite = 1;
	}

	if((flags & (FREAD))	&&
	(vp) 			&&
	(vp->v_gnode->gn_flags & GNF_TCB)){

		tcbread = 1;
	}

	saveflags = flags;

	/* if this is a create we need to call VN_CREATE */
	if (flags & FCREAT) {
		if (vp != NULL) {
			/* 
			 * If the file already exists and this is an
			 * exclusive create, then fail with EEXIST.
			 */
			if (flags & FEXCL) {
				rc = EEXIST;
				goto error;
			}
			flags &= ~FCREAT;
		}
		else {
			/* if file is in directory on removable *
			 * device then set the FSYNC flag       */
			if (dvp->v_vfsp->vfs_flag & VFS_REMOVABLE) {
				FP_LOCK(fp);
				fp->f_flag |= FSYNC;
				FP_UNLOCK(fp);
				flags |= FSYNC;
			}

			/* apply the umask to the mode */
			mode &= ~U.U_cmask;

			/* call the vnode op to create it. */
			rc = VNOP_CREATE(dvp, &vp, flags, pnp->pn_path, 
				((mode & 07777) & (~S_ISVTX)),
				&(fp->f_vinfo), crp);

			VNOP_RELE(dvp);

			/* if all works well the new vnode will be in vp */ 
			if (rc == 0) 
			{
				TRCHKL1T(HKWD_SYSC_CREAT, vp);
				fp->f_vnode = vp; /* save it in the fp */
				fp->f_cred = crp; /* save creds for it */
				*fpp = fp;
			}
			else 
			{
				crfree(crp);
				ffree(fp);
			}

			return (rc);
		}
	}

	/* won't need this any more, thank you (sanity check) */
	if (dvp != NULL) {
		VNOP_RELE(dvp);
		dvp = NULL;
	}

	/* 
	 * if file is on removable device
	 * then set the FSYNC flag
	 */
	if (vp->v_vfsp->vfs_flag & VFS_REMOVABLE) {
		FP_LOCK(fp);
		fp->f_flag |= FSYNC;
		FP_UNLOCK(fp);
		flags |= FSYNC;
	}

	/* save this vnode pointer and the creds for it */
	fp->f_vnode = vp;
	fp->f_cred = crp;

	/* open this vnode */
	rc = VNOP_OPEN(vp, flags, ext, &(fp->f_vinfo), crp);

	/*
	 * was this an attempted clone open?
	 */
	if (rc == ECLONEME)
		if (vp->v_vntype != VCHR)
			rc = EINVAL;	/* driver is confused */
		else 
		{
			rc = spec_clone(&vp, flags, ext, &(fp->f_vinfo), crp);
			if (rc == 0)
				fp->f_vnode = vp;
		}

	if (!rc)
	{
		/* 
		 * Success - TCB_Leak has occured
		 */

		if (tcbread && audit_flag)
		{
			if (audit_svcstart("TCB_Leak", &tcbleak, 0))
                        	audit_svcfinis();
               	}

		/* 
		 * Success - TCB_Mod has occured
		 */

		if (tcbwrite && audit_flag)
		{
                	if (audit_svcstart("TCB_Mod", &tcbmod, 0))
				audit_svcfinis();
               	}

		/* only modify *fpp on success */
		*fpp = fp;

		return rc;	/* open was successful */
       	}

	/* 
	 * If this was a create of a file that existed then there is
	 * a chance that between the lookup and the open the file
	 * was removed or unmounted. In that case we'll get an
	 * ESTALE or ENOENT error and we need to try the whole thing over again.
	 */
	if ((rc == ESTALE || rc == ENOENT) && (saveflags & FCREAT))
		rc = ERESTART;

error:
	/* Any other errors will fall through to here, clean up and leave. */

	if (vp)
		VNOP_RELE(vp);

	if (dvp)
		VNOP_RELE(dvp);

	crfree(crp);
	ffree(fp);

	return (rc);
}

fp_xopen(
char *		fname,		/* file name to open			*/
int		fpflag,		/* see FP_IO flags in fp_io.h		*/
int		oflag,		/* open() style flags			*/
caddr_t		basename,	/* return for final basename		*/
uint		basenamelen,	/* length of basename buffer		*/
struct file **	fpp)		/* address to return file pointer to	*/
{
	int rc;
	struct pathname	pn;
	int klock;	        /* save kernel_lock state */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	if ((rc = pn_get(fname, fpflag&FP_SYS ? SYS : USR, &pn)) == 0)
	{
		if ((rc = openpnp(&pn, oflag, 0, 0, fpp)) == 0)
			if (basename != NULL && basenamelen)
				copyname(basename, pn.pn_path, basenamelen);
		pn_free(&pn);
	}

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}
