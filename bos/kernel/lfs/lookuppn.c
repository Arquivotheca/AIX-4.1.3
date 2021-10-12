static char sccsid[] = "@(#)51	1.36.1.11  src/bos/kernel/lfs/lookuppn.c, syslfs, bos411, 9431A411a 8/3/94 16:24:47";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: lookupname, lookuppn, lookupvp, follow_symlink
 *
 * ORIGINS: 3, 24, 26, 27
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

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/fs_locks.h>
#include <sys/dir.h>
#include <sys/errno.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/pathname.h>
#include <sys/malloc.h>
#include <sys/trchkid.h>
#include <sys/sleep.h>

BUGVDEF(lookdbg,0);
BUGXDEF(mdebug);

static int follow_symlink();
extern int umount_elist;	/* global umount event list */

/*
 * lookup the user file name,
 * Handle allocation and freeing of pathname buffer, return error.
 */
lookupname( char	  *namep,	/* user pathname */
	    int		   seg,		/* addr space that name is in */
	    int		   flags,	/* various flags */
	    struct vnode **dirvpp,	/* ptr to parent dir vnode */
	    struct vnode **compvpp,	/* ptr to component vnode */
	    struct ucred  *crp )	/* ptr to credentials */
{
	struct pathname lookpn;
	register int error;
	int klock;	                /* save kernel_lock state */

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	error = pn_get(namep, seg, &lookpn);
	if (error == 0) {
		error = lookuppn(&lookpn, flags, dirvpp, compvpp, crp);
		pn_free(&lookpn);
	} 
	else {
		if (dirvpp)
			*dirvpp = (struct vnode *)NULL;
		if (compvpp)
			*compvpp = (struct vnode *)NULL;
	}
	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);
	return (error);
}

/*
 * Starting at current directory, translate pathname pnp to end.
 * Leave pathname of final component in pnp, return the vnode
 * for the final component in *compvpp, and return the vnode
 * for the parent of the final component in dirvpp.
 *
 * This is the central routine in pathname translation and handles
 * multiple components in pathnames, separating them at /'s.  It also
 * implements mounted file systems and processes symbolic links.
 */
lookuppn( struct pathname *pnp,		/* pathname to lookup */
	  int		   flags,	/* various flags, see dir.h */
	  struct vnode   **dirvpp,	/* ptr for parent vnode */
	  struct vnode   **compvpp,	/* ptr for entry vnode */
	  struct ucred    *crp )	/* ptr to credentials */
{
	/* Parent directory pointer */
	register struct vnode *dvp = (struct vnode *)NULL;
	struct	vnode *vp = (struct vnode *)NULL;  /* Directory pointer */
	char	component[_D_NAME_MAX+1]; /* Next pathname component */
	struct	vnode *nvp;     	/* Secondary directory pointer */
	int	nlink;     		/* # links processed so far */
	struct	vfs *vfsp;
	int	endslash;		/* path ends with a '/' */
	int	len;			/* actual length of a component */
	char	*pathbuf = pnp->pn_buf;	/* tag for tracing */
	int	tlock;			/* is multi-thread locking required? */
	int	error = 0;		/* substitute for u_error  */
	int     state;                  /* current processing mode */

#ifdef STSAVE
	struct stsave
	{
		int	      state;
		struct vnode *dvp;
		int	      dvpcount;
		int	      dvptype;
		int	      name;
		struct vnode *vp;
		int	      vpcount;
		enum vtype    vptype;
	} *stsp;
	int maxsts = 24;
	int *namep = (int *)component;
	stsp = (struct stsave *)&pnp->pn_buf[256];
	if (pnp->pn_pathlen < 256)
		bzero(stsp, maxsts * sizeof(struct stsave));
#endif

	/* trace pathname and (later) vnode */
	TRCGEN(0, HKWD_LFS_LOOKUP, pathbuf, pnp->pn_pathlen, pathbuf);

	BUGLPR(lookdbg, BUGNTF,
	    ("lookuppn(path=%s, flg=%d, dirvpp=0x%x, compvpp=0x%x)\n",
	    pnp->pn_path, flags, dirvpp, compvpp));

	sysinfo.namei++;
	cpuinfo[CPUID].namei++;
	nlink = 0;                         /* no links processed yet */
	if (dirvpp)
		*dirvpp = (struct vnode *)NULL;
	if (compvpp)
		*compvpp = (struct vnode *)NULL;

	/*
	 * If name ends with '/'s, strip them off first.  Unfortunately, some
	 * mpx drivers see the '/'s as a valid channel identifier.  So we
	 * have to remember how many we took off.
	 */
	endslash = strip_slash(pnp);

/*
 * Processing states for lookuppn
 */
#define INITIAL        0	/* At entry, and after a symbolic link      */
#define FIND_STATE     1	/* Figure out what to do with the component */
#define GET_COMPONENT  2	/* Read next component of path              */
#define MOUNT_POINT    3	/* Cross a mount point                      */
#define SYMLINK        4	/* Follow a symbolic link                   */
#define MPX_DEVICE     5	/* Get specfs vnode for multiplexed device  */
#define TERMINATE      6	/* Last component has been processed        */
#define ERROUT         7	/* Exit with an error                       */

  /**********************************************************************
   * STATE TRANSITIONS                                                  *
   * When processing in the state indicated on the top row, the next    *
   * state will be one of those marked with an X in the column beneath. *
   *                                                                    *
   *               state 0   1    2    3    4    5    6                 *
   *             ----|------------------------------------              *
   *        new   0  |                      X                           *
   *       state  1  |   X        X    X                                *
   *              2  |       X    X                                     *
   *              3  |       X                                          *
   *              4  |       X                                          *
   *              5  |       X                                          *
   *              6  |       X    X              X                      *
   *              7  |   X        X    X    X    X    X                 *
   **********************************************************************/

	state = INITIAL;

	for (;;)
	{
#ifdef STSAVE
		if (pnp->pn_pathlen >= 256)
			maxsts = 0;
		if (maxsts)
		{
			stsp->state = state;
			stsp->name = *namep;
			stsp->dvp = dvp;
			if (dvp)
			{
				stsp->dvpcount = dvp->v_count;
				stsp->dvptype = dvp->v_type;
			}
			stsp->vp = vp;
			if (vp)
			{
				stsp->vpcount = vp->v_count;
				stsp->vptype = vp->v_type;
			}
			maxsts--;
			stsp++;
		}
#endif

		switch(state)
		{
		/*
		 * If name starts with '/' start from root;
		 * otherwise start from current directory.
		 *
		 * If this is the first time through the loop, then vp
		 * will be NULL.  Otherwise, vp refers to the directory
		 * that contained a symbolic link.
		 */
		case (INITIAL):
			/* Empty pathnames are no good */
			if (!pn_pathleft(pnp))
			{
				error = ENOENT;
				state = ERROUT;
				break;
			}

			/* Is this process multi-threaded? */
			tlock = (U.U_procp->p_active > 1);

			if (pn_peekchar(pnp) == '/')
			{
				/* strip off leading slashes */
				pn_skipslash(pnp);

				if (vp)
					VNOP_RELE(vp);
				if (tlock)
					U_FSO_LOCK();
				vp = U.U_rdir ? U.U_rdir : rootdir;
				VNOP_HOLD(vp);
				if (tlock)
					U_FSO_UNLOCK();
			}
			else
			{	/* relative path name */
				if (vp == NULL)
				{
					if (tlock)
						U_FSO_LOCK();
					vp = U.U_cdir;

			/*
			 * Catch kprocs trying to lookup relative pathnames
			 * if they don't have a current directory.
			 */
					if (vp == NULL)
					{
						error = ENOTDIR;
						state = ERROUT;
						if (tlock)
							U_FSO_UNLOCK();
						break;
					}
					VNOP_HOLD(vp);
					if (tlock)
						U_FSO_UNLOCK();
				}
			}
			state = FIND_STATE;
			break;

		case (FIND_STATE):
			/*
			 * Here vp contains pointer
			 * to last component matched.
			 */

			/*
			 * Check for mounted over vnode.
			 */
			if ((vp->v_mvfsp != (struct vfs *)NULL)
			    && (!(flags & L_NOXMOUNT) || pn_pathleft(pnp)))
			{
				state = MOUNT_POINT;
				break;
			}

			/*
			 * Check for symbolic link that is to be followed.
			 */
			if (vp->v_vntype == VLNK
			    && (!(flags & L_NOFOLLOW)
				|| pn_pathleft(pnp)
				|| endslash))
			{
				state = SYMLINK;
				break;
			}

			/*
			 * Check for multiplexed device vnode.
			 */
			if (vp->v_vntype == VMPC)
			{
				state = MPX_DEVICE;
				break;
			}

			if (!pn_pathleft(pnp))
				state = TERMINATE;
			else
				state = GET_COMPONENT;
			break;

		case (GET_COMPONENT):
			/*
			 * If there is another component,
			 * gather up name into users' dir buffer.
			 */
			BUGLPR(lookdbg, BUGGID, ("lookuppn: gather up name\n"));
			pn_skipslash(pnp);
			if (error = pn_getcomponent(pnp, component,
					sizeof(component)-1, &len))
			{
				state = ERROUT;
				break;
			}

			/* ditch old directory vnode pointer */
			if (dvp)
			{
				VNOP_RELE(dvp);
				dvp = NULL;
			}

			/*
			 * If we have a ".." component, loop until we find
			 *  the final parent vfs vnode.
			 */
			if (component[0] == '.' &&
			    component[1] == '.' &&
			    component[2] == '\0')
			{
				for (;;)
				{
					if (vp == rootdir || vp == U.U_rdir)
						break;

					if ((vp->v_flag & V_ROOT) == 0)
						break;

					nvp = vp->v_vfsp->vfs_mntdover;
					VNOP_HOLD(nvp);
					VNOP_RELE(vp);
					vp = nvp;
				}
				if (vp == rootdir || vp == U.U_rdir)
				{
				/* One would expect to return to FIND_STATE
				 * at this point.  However, there is an
				 * oddity wherein the boot process wants to
				 * access the ram disk after having mounted
				 * over it, and it does this by referring
				 * to '/../whatever', where '/whatever'
				 * is a ram disk file.  The following code
				 * makes that work.
				 */
					if (!pn_pathleft(pnp))
						state = TERMINATE;
					else
						state = GET_COMPONENT;
					break;
				}
			}

			if (vp->v_vntype == VDIR)
				error = VNOP_LOOKUP(vp, &nvp, component, 
						flags, NULL, crp);
			else
				error = ENOTDIR;

			BUGLPR(lookdbg, BUGNTF,
				("return from vn_lookup: nvp=%x, u_error=%d\n",
					nvp, error));

			if (error)
			{
				if (error==ENOENT
				    && (flags & L_CRT)
				    && !pn_pathleft(pnp))
				{
					if (vp->v_vfsp->vfs_flag & VFS_READONLY)
					{
						error = EROFS;
						state = ERROUT;
					}
					else
					{
						error = 0;
						dvp = vp;
						vp = NULL;
						state = TERMINATE;
					}
				}
				else
					state = ERROUT;
				break;
			}

			/* exchange PFS special vnodes for specfs vnodes */
			if (nvp->v_vntype == VBLK
			    || nvp->v_vntype == VCHR
			    || nvp->v_vntype == VMPC
			    || nvp->v_vntype == VFIFO)
				if ((error = spec_vp(&nvp)) != 0)
				{
					VNOP_RELE(nvp);
					state = ERROUT;
					break;
				}

			dvp = vp;
			vp = nvp;

			state = FIND_STATE;
			break;

		case (MOUNT_POINT):

			VFS_LIST_LOCK();

			/*
			 * Traverse a mount point.
			 */
			vfsp = vp->v_mvfsp;
			if (vfsp == NULL)
			{
				/* an unmount happened since we first 
				 * looked at the vnode; check it again */
				state = FIND_STATE;
				VFS_LIST_UNLOCK();
				break;
			}

			if (vfsp->vfs_flag & VFS_UNMOUNTING)
			{
				/* sleep until unmount completes */
				(void) e_sleep_thread(&umount_elist, 
						      &vfs_list_lock,
						      LOCK_SIMPLE);
				/*
				 * mount activity has occurred or is in
				 * progress; look at the vnode again
				 */
				state = FIND_STATE;
				VFS_LIST_UNLOCK();
				break;
			}

			/* prevent unmount of this vfs */
			vfs_hold(vfsp);

			VFS_LIST_UNLOCK();

			if ((flags & L_LOC) && 
			    (vfsp->vfs_gfs->gfs_flags & GFS_REMOTE))
			{
				error = ENOTDIR;
			}
			else
			{
				/* stop lookuppn infinite recursion
				 * through VFS_ROOT when a disconnected
				 * file is mounted over itself         */
				struct vfs *temp_mvfsp;
				if (vp->v_vntype == VREG)
				{
					VN_LOCK(vp);
					temp_mvfsp = vp->v_mvfsp;
					vp->v_mvfsp = (struct vfs *)NULL;
				}

				/* get the root vnode of the mounted
				 * file system                         */
				error = VFS_ROOT(vfsp, &nvp, crp);

				if (vp->v_vntype == VREG)
				{
					vp->v_mvfsp = temp_mvfsp;
					VN_UNLOCK(vp);
				}
			}

			vfs_unhold(vfsp);

			if (error)
				state = ERROUT;
			else
			{
				VNOP_RELE(vp);
				vp = nvp;
				state = FIND_STATE;
			}
			break;

		case (SYMLINK):
			/*
			 * If we hit a symbolic link and this operation
			 * wishes to follow the link (not apply to it)
			 * or there is more path to be translated, then
			 * place the contents of the link at the front
			 * of the remaining pathname.
			 */

			/*
			 * check for symbolic link loop (note that this
			 * arbitrarily says if you have more than a certain
			 * number of links then you must have a loop)
			 */
			if (++nlink > MAXSYMLINKS)
			{
				error = ELOOP;
				state = ERROUT;
				break;
			}
			error = follow_symlink(vp, pnp, crp);
			if (error)
			{
				state = ERROUT;
				break;
			}
			/* set vp back to directory vnode */
			VNOP_RELE(vp);
			vp = dvp;
			dvp = NULL;
			state = INITIAL;
			break;

		case (MPX_DEVICE):
			pn_skipslash(pnp);
			if ((pn_pathleft(pnp) || (flags & L_OPEN)))
			{
			/*
			 * copy pnp->pn_path into a temp buffer & NULL
			 * terminate it since pnp->pn_path is not necessarily
			 * NULL terminated but everyone below VNOP_LOOKUP
			 * expects it to be...
			 */
				int i;
				char tmppath[_D_NAME_MAX+1];

				bcopy(pnp->pn_path, tmppath, pnp->pn_pathlen);
				/*
				 * restore '/'s...
				 */
				for (i = pnp->pn_pathlen;
				    ((i < pnp->pn_pathlen + endslash)
				    && (i < _D_NAME_MAX));
				    i++)
					tmppath[i] = '/';
				tmppath[i] = '\0';

				/* lookup for the channel returns spec vnode */
				error = VNOP_LOOKUP(vp, &nvp, tmppath, 
						flags, NULL, crp);
				if (error)
				{
					state = ERROUT;
					break;
				}
				VNOP_RELE(vp);
				vp = nvp;
			}
			state = TERMINATE;
			break;

		case (TERMINATE):
			/*
			 * Callers who set the L_EROFS or L_CRT request
			 * bit are preparing to write or change the
			 * file they're looking up; the lookup should
			 * fail if the file lives in a read-only vfs.
			 */
			if ((flags & (L_EROFS | L_CRT))
			   && (vp != NULL)
			   && (vp->v_vfsp->vfs_flag & VFS_READONLY))
			{
				error = EROFS;
				state = ERROUT;
				break;
			}

			assert(!error);

			pn_set(pnp, component);

			if (dirvpp)
			{
				/* dvp will be NULL when vp is the root dir */
				if (dvp == NULL)
				{
					dvp = vp;
					VNOP_HOLD(dvp);
				}
				*dirvpp = dvp;
			}
			else if (dvp)
				VNOP_RELE(dvp);

			if (compvpp)
			{
				*compvpp = vp;
				/* trace vp and original path buffer */
				TRCHKL2T(HKWD_LFS_LOOKUP, vp, pathbuf);
			}
			else
			{
				if (vp)
					VNOP_RELE(vp);
			}

			return (error);

		case (ERROUT):
			if (dvp)
				VNOP_RELE(dvp);
			if (vp)
				VNOP_RELE(vp);

			/* trace the fact that an error occurred */
			TRCHKL2T(HKWD_LFS_LOOKUP, 0, pathbuf);

			return (error);
		}
	}
}

/*
* lookupvp	exported lookup interface for loadable file systems.
*
* Arguments:
* name		name to look up, must be in SYS space
* flags		lookup flags
* vpp		pointer to returned vnode pointer
*
* Return Value:
* 0		success
* !0		the error that occurred
*
* Assumptions:
*		The caller is holding the kernel lock.  This call may
*		page fault, hence it must be called at process level.
*/
lookupvp( char		*name,
	  int		 flags,
	  struct vnode **vpp,
	  struct ucred  *crp )
{
	return lookupname(name, SYS, flags, NULL, vpp, crp);
}

/*
 * Merges symbolic link into pathname.
 */
static int
follow_symlink( struct vnode    *vp,
		struct pathname *pnp,
		struct ucred    *crp )
{
	register int     error;
	struct iovec     aiov;
	struct uio       auio;
	struct pathname	 linkpath;
	struct pathname	*lnp = &linkpath;

	/*
	 * read the link into a pathname buffer
	 */
	aiov.iov_base = lnp->pn_buf;
	aiov.iov_len = MAXPATHLEN;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_resid = MAXPATHLEN;
	error = VNOP_READLINK(vp, &auio, crp);
	BUGLPR(lookdbg, BUGNTF, ("follow_symlink: path = %s\n", lnp->pn_buf));
	if (error)
		goto symerr;
	lnp->pn_path = lnp->pn_buf;
	lnp->pn_pathlen = MAXPATHLEN - auio.uio_resid;

	/* check for null link */
	if (!pn_pathleft(lnp))
		(void) pn_set(lnp, ".");

	/* concatenate the paths and remove any trailing slashes that may have
	 * been in the symlink name.
 	 */
	if (! (error = pn_combine(pnp, lnp)))
		(void) strip_slash(pnp);
symerr:
	return (error);
}
