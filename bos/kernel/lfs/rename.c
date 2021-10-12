static char sccsid[] = "@(#)60	1.22.1.5  src/bos/kernel/lfs/rename.c, syslfs, bos412, 9443A412c 10/20/94 11:03:15";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: rename
 *
 * ORIGINS: 26, 27
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

#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/pathname.h"
#include "sys/audit.h"
#include "sys/malloc.h"

BUGXDEF(dbdir);
BUGVDEF(mvdbg,0);

/*************************************************************************
 *
 * NAME:  rename()    (system call entry point)
 *
 * FUNCTION:	renames a link to a file. If the target name exists it is
 *		removed first. The links must exist on the same virtual
 *		file system. The two names must be of the same type (i.e.
 *		directory to directory, file to file, etc.). If the final
 *		component of the from name is a symbolic link, then the
 *		link is renamed, not the file it points to.
 *
 * PARAMETERS:	frompath - the current name of the file
 *		topath   - the new name of the file
 *
 * RETURNS:	A zero is returned if there are no errors. If an error
 *		occurs it returns one of the following error codes:
 *		EXDEV if the topath and the frompath are on different
 *		file systems. EROFS if the named file resides on a read-
 *		only file system. EBUSY if the frompath or topath 
 *		directory is currently in use by the system.  Note:
 *		we allow renames over files and directories in use
 *		by other processes.
 *
 **************************************************************************/


rename(frompath, topath)
char    *frompath;
char    *topath;
{
	struct vnode *	fvp;	/* vnode for frompath		*/
	struct vnode *	fdvp;	/* directory vnode for frompath	*/
	struct pathname	fpn;	/* pathname struct for frompath	*/
	struct vnode *	tvp;	/* vnode for topath		*/
	struct vnode *	tdvp;	/* directory vnode for topath	*/
	struct pathname	tpn;	/* pathname struct for topath	*/
	extern kernel_lock;	/* global kernel lock		*/
	int lockt;		/* previous state of kernel lock*/
	struct ucred *  crp;	/* current credentials          */
	int rc;
	static int svcnum = 0;
	static int tcbmod = 0;
	int tcbrc = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	crp = crref();

	if ((audit_flag) && (audit_svcstart("FILE_Rename", &svcnum, 0)))
	{
                if(frompath){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                rc = ENOMEM;
                                goto out;
                        }
                        else if(copyinstr(frompath, ptr, MAXPATHLEN, &len)){
                                rc = EFAULT;
                                free(ptr);
                                goto out;
                        }
                        else
                        	audit_svcbcopy(ptr, len);
                        free(ptr);
                }
                if(topath){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                rc = ENOMEM;
                                goto out;
                        }
                        else if(copyinstr(topath, ptr, MAXPATHLEN, &len)){
                                rc = EFAULT;
                                free(ptr);
                                goto out;
                        }
                        else
                        	audit_svcbcopy(ptr, len);
                        free(ptr);
                }
		audit_svcfinis();
	}

	BUGLPR(dbdir, BUGNTF, ("rename(frompath=%s)\n",frompath)) ;

	if (rc = pn_get(frompath, USR, &fpn)) {
		goto out;
	}

	if (rc = pn_get(topath, USR, &tpn)) {
		pn_free(&fpn);
		goto out;
	}

	if (rc = lookuppn(&fpn, L_DEL | L_NOFOLLOW, &fdvp, &fvp, crp)) {
		pn_free(&fpn);
		pn_free(&tpn);
		goto out;
	}

	BUGLPR(mvdbg, BUGACT, ("after #1 look:  fdvp cnt=%d, fvp cnt=%d\n",
		fdvp->v_count, fvp->v_count));

	/* disallow renames of mounted objects */

	if (fvp->v_flag & V_ROOT) {
		rc = EBUSY;
		VNOP_RELE(fvp);
		if (fdvp)	/* frompath might have been "/" */
			VNOP_RELE(fdvp);
		pn_free(&fpn);
		pn_free(&tpn);
		goto out;
	}

	if((fvp->v_gnode->gn_flags & GNF_TCB))
		tcbrc = 1;

	if (rc = lookuppn(&tpn, L_CREATE | L_NOFOLLOW | L_EROFS, &tdvp, &tvp, crp)) {
		VNOP_RELE(fvp);
		VNOP_RELE(fdvp);
		pn_free(&fpn);
		pn_free(&tpn);
		goto out;
	}

	BUGLPR(mvdbg,BUGACT,("after #2 look: tdvp cnt==%d  tvp cnt==%d\n",
		tdvp->v_count, tvp->v_count));

	/* may not exist */
	if(tvp){
		if((tvp->v_gnode->gn_flags & GNF_TCB))
			tcbrc = 1;
	}

	/* disallow cross device rename as far as can be determined here */
	if (fvp->v_vfsp->vfs_type != tdvp->v_vfsp->vfs_type)
		rc = EXDEV;
	/* disallow rename over a mount point */
	else if (tvp && (tvp->v_flag & V_ROOT))
		rc = EBUSY;
	else {
		struct vnode *	fvp_pfs = fvp;
		struct vnode *	tvp_pfs = tvp;

		/* Make sure we only pass PFS vnodes to the vnode operation
		 * and do any specfs checking that needs to be done.
		 */
		if ((rc = specchk_rename(&fvp_pfs, &tvp_pfs)) == 0)
			rc = VNOP_RENAME(fvp_pfs, fdvp, fpn.pn_path, 
					tvp_pfs, tdvp, tpn.pn_path, crp);
	}

	VNOP_RELE(fvp);
	VNOP_RELE(fdvp);
	if (tdvp)	/* target may have been "/"	*/
		VNOP_RELE(tdvp);
	if (tvp)	/* target may not have existed	*/
		VNOP_RELE(tvp);
	pn_free(&tpn);
	pn_free(&fpn);

	/*
	 * If no error then TCB_Mod has occured
	 */

	if ((rc == 0) && (tcbrc) && (audit_flag))
	{
		if (audit_svcstart("TCB_Mod", &tcbmod, 0))
			audit_svcfinis();
        }

out:
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
