static char sccsid[] = "@(#)93	1.6.2.4  src/bos/kernel/vmm/POWER/vmpspace.c, sysvmm, bos411, 9428A410j 2/9/94 15:08:55";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:   swapoff, swapon, swapqry
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <sys/user.h>
#include <sys/devinfo.h>
#include <sys/fp_io.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/dasd.h>
#include <sys/signal.h>
#include <sys/lockl.h>
#include <sys/inline.h>
#include "vmsys.h"

/*
 * NAME: swapoff
 *
 * FUNCTION: This is the swapoff system call.
 *	     It removes a paging device from the VMM.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      system call
 *
 * (NOTES:)
 *
 * NOT SUPPORTED IN RELEASE 1 -- RETURN EINVAL
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 *      sets u.u_error
 *	modifies PDT (paging device table) and paging space disk maps
 *
 * RETURNS:
 *
 *      -1 and sets u.u_error as follows:
 *
 *              errno values applicable for pathname resolution
 *		EPERM	- caller does not have proper authority
 *              EINVAL	- swapoff not supported yet
 */

int
swapoff(pathname)
char *pathname;				/* pathname of device 		*/
{
        int rc;

        /* if caller has appropriate authority then attempt
	 * to remove the paging space device.
         */
        if (rc = privcheck(VMM_CONFIG))
                u.u_error = rc;
	else
		u.u_error = psdel(pathname);

	return u.u_error ? -1 : 0;
}

/*
/*
 * NAME: swapon
 *
 * FUNCTION: This is the swapon system call.
 *	     It defines a device as a paging device to the VMM.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      system call
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 *      sets u.u_error
 *	modifies PDT (paging device table) and paging space disk maps
 *
 * RETURNS:
 *
 *      0 if successful
 *      -1 if not successful and sets u.u_error as follows:
 *
 *              errno values applicable for pathname resolution
 *		EPERM	- caller does not have proper authority
 *              EINVAL	- invalid argument (size of device invalid)
 *		ENOTBLK	- block device required
 *              ENOMEM	- the maximum number of paging space devices
 *                        are already defined or no memory for buf structs
 *			  or for disk map
 *		ENODEV	- device does not exist
 *		ENXIO	- no such device address
 *		EINTR	- signal was received while processing request
 *		others	- from dd open or ioctl
 */

int
swapon(pathname)
char *pathname;				/* pathname of device 		*/
{
        int rc;

        /* if caller has appropriate authority then attempt
	 * to add the device as a paging space.
         */
        if (rc = privcheck(VMM_CONFIG))
                u.u_error = rc;
	else
		u.u_error = psadd(pathname);

	return u.u_error ? -1 : 0;
}

/*
 * NAME: swapqry
 *
 * FUNCTION: This is the swapqry system call.
 *	     It provides information about paging devices.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      system call
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 *      sets u.u_error
 *
 * RETURNS:
 *
 *	0 if the device is an active paging device (and returns paging
 *	  device information if buffer pointer is non-NULL).
 *      -1 if not successful and sets u.u_error as follows:
 *
 *              errno values applicable for pathname resolution
 *		ENODEV	- device is not an active paging device
 *		EFAULT	- buffer pointer invalid
 *		ENOTBLK	- block device required
 *		ENODEV	- device does not exist
 *		ENXIO	- no such device address
 *		EINTR	- signal was received while processing request
 */

int
swapqry(pathname,buf)
char *pathname;				/* pathname of device 		*/
struct pginfo *buf;			/* pointer to paging info buffer*/ 
{
	/* query the paging device.
         */
	u.u_error = psqry(pathname, buf);

	return u.u_error ? -1 : 0;
}

/* the following routines are intended to be used as primitives
 * by whatever system call interfaces are defined above.
 */ 

/*
 * psadd(pathname)
 *
 * adds device specified by pathname as a paging device
 * (or, if already defined as a paging device, changes
 * the definition according to its current size -- note
 * that a decrease in size is not yet supported).
 *
 * return values:
 *      0	- ok
 *
 * from getdev():
 *
 *	ENOTBLK	- not a block device that is a logical volume on a disk
 *	others	- errors associated with pathname resolution
 *
 * from defineps():
 *
 *      EINVAL	- nblocks or nbufstr invalid.
 *      ENOMEM	- no memory for buf structs or disk map, or PDT is full.
 *	others	- errors from rdevopen.
 */

#define	LITE_THRESH_PS	(16*(1048576/PAGESIZE))		/* 16 Meg, in Pages */
#define	BUFS_PER_PAGE	(PAGESIZE/sizeof(struct buf))

static
psadd(pathname)
char *pathname;
{
	int rc;
	dev_t devno;			/* device number	*/
	int nblocks, nbufstr;		/* # of blocks, bufstr	*/
	int	memsize;		/* physical memory size in pages */

	/* first resolve pathname to a device.
	 */
	if (rc = getdev(pathname, &devno, &nblocks))
		return(rc);

	/* convert the number of blocks from 512-byte (DBSIZE)
	 * to 4K-byte (PAGESIZE) blocks which is what the VMM
	 * expects.
	 */
	nblocks = BLK2PG(nblocks);

	/* determine the number of buf structs to use.
	 * the limiting factor becomes the number allocated in the LVM.
	 * we want enough so that we don't limit the scheduling of paging
	 * i/o due to insufficient buf structs.
	 * the number of i/o's queued by v_flbru is at most
	 * pf_maxfree + pf_numsched.
	 */

	memsize = vmker.nrpages - vmker.badpages;
	nbufstr = nblocks / 256;

	if ( memsize <= LITE_THRESH_PS )		/* Light system */
	{
		nbufstr = BUFS_PER_PAGE;
	}
	else						/* Heavy system */
	{
		/* allocbufs always rounds up to a BUFS_PER_PAGE multiple.
		 * this imposes a ceiling only.
		 */
		if (nbufstr > (BUFS_PER_PAGE * 2))
			nbufstr = BUFS_PER_PAGE * 2;
	}

	/* define the paging device.
	 */
	rc = defineps(devno, nblocks, nbufstr);

	return(rc);
}

/*
 * psdel(pathname)
 *
 * deletes paging space device specified by pathname.
 *
 * NOTE: NOT SUPPORTED IN RELEASE 1 -- RETURNS EINVAL
 *
 * return values:
 *      0 - ok
 *
 * from getdev():
 *
 *	ENOTBLK	- not a block device that is a logical volume on a disk
 *	others	- errors associated with pathname resolution
 *
 * from deleteps():
 *
 */
static
psdel(pathname)
char *pathname;
{
	int rc;
	dev_t devno;			/* device number	*/
	int nblocks;			/* number of blocks	*/

	/* first resolve pathname to a device.
	 */
	if (rc = getdev(pathname, &devno, &nblocks))
		return(rc);

#ifdef NEVER
	/* now delete the paging device.
	 */
	rc = deleteps(devno);

	return(rc);
#endif
	return(EINVAL);
}

/*
 * psqry(pathname,buf)
 *
 * queries paging space device specified by pathname.
 *
 * return values:
 *      0	- ok
 *	ENODEV	- device not in paging device table.
 *
 * from getdev():
 *
 *	ENOTBLK	- not a block device that is a logical volume on a disk
 *	others	- errors associated with pathname resolution
 *
 * from copyout():
 *
 *	EFAULT	- buf points to a bad address.
 */
static
psqry(pathname, buf)
char *pathname;
struct pginfo *buf;
{
	int rc;				/* return code			*/
	dev_t devno;			/* device number		*/
	int nblocks;			/* number of blocks		*/
	int pdtx;			/* paging device table index	*/
	struct pginfo local;		/* local paging info struct	*/
	struct vmdmap *p0;		/* pointer to disk map		*/
	int savevmmsr, savetmpsr;	/* save current sreg values	*/

	/* first resolve pathname to a device.
	 */
	if (rc = getdev(pathname, &devno, &nblocks))
		return(rc);

	/* search for the device in the paging device table.
	 */
	if ((pdtx = vcs_devpdtx(D_PAGING, devno)) < 0)
	{
		/* indicate paging device is NOT in table.
		 */
		rc = ENODEV;
	}
	else
	{
		/* return paging device info if pointer to buffer
		 * is non-NULL.
		 */
		if (buf == NULL)
		{
			/* just indicate device is in PDT.
			 */
			rc = 0;
		}
		else
		{
			/* make vmm data segment and paging space
			 * disk map addressable.
			 */
			savevmmsr = chgsr(VMMSR, vmker.vmmsrval);
			savetmpsr = chgsr(TEMPSR, vmker.dmapsrval);

			p0 = (struct vmdmap *) ((TEMPSR << L2SSIZE) +
							pdtx * DMAPSIZE);

			/* fill out local paging device structure
			 */
			local.devno = pdt_device(pdtx);
			local.size  = p0->mapsize;
			local.free  = p0->freecnt;
			local.iocnt = pdt_iocnt(pdtx);

			/* restore segment registers.
			 */
			(void)chgsr(VMMSR, savevmmsr);
			(void)chgsr(TEMPSR, savetmpsr);

			/* copy paging device info to user buffer.
			 */
			rc = copyout(&local, buf, sizeof(struct pginfo));
		}
	}

	return(rc);
}

/*
 * getdev(pathname,devno,nblocks)
 *
 * resolves a pathname to a device and returns
 * the associated device number and size in blocks.
 *
 * input parameters:
 *
 *  pathname - path name to resolve
 *
 *  devno -  set on return to devno of the device.
 *
 *  nblocks - set on return to size of device in blocks.
 *
 * return values:
 *      0	- ok
 *	ENOTBLK	- not a block device that is a logical volume on a disk
 *	others	- errors associated with pathname resolution
 */
static
getdev(pathname,devno,nblocks)
char *pathname;				/* pathname of device		*/
dev_t  *devno;				/* set to device number		*/
int *nblocks;				/* set to size in blocks	*/

{
	struct vnode *vp;		/* Vnode pointer		*/
        struct stat status;             /* File status/information      */
        struct devinfo devinfo;         /* Device info structure        */
	struct file *fp;		/* File pointer			*/
	struct ucred *crp;              /* Creds for file system calls  */
	int rc;

	/* the following sequence of calls is used instead
	 * of fp_open, fp_fstat to bypass normal permission
	 * checking.  this is necessary since swapqry needs
	 * to convert a pathname to a device number and is
	 * callable by a normal user process which may not
	 * have permission to access the file.
	 */
	
	/* set up the creds
	 */
	crp = crref();

	/* get a vnode given the pathname.
	 */
	if ((rc = lookupname(pathname, USR, L_SEARCH, NULL, &vp, crp)) == 0)
	{

		/* get the file status given the vnode.
		 */
		rc = vstat(vp, &status, crp);

		/* release the vnode.
		 */
		VNOP_RELE(vp);
	}

	/* release creds, no longer needed
	 */
	crfree(crp);

	if (rc)
		return(rc);

	/* now get a file pointer given the device number.
	 */
	if (rc = fp_opendev(status.st_rdev, DREAD, (caddr_t)NULL, NULL, &fp))
		return(rc);

	/* set the device number.
 	 */
	*devno = status.st_rdev;

        /* handle depending on file mode.
         * for now we only support paging spaces on block
	 * special files that are logical volumes on disk devices.
         */
        switch(status.st_mode & S_IFMT)
        {
        case S_IFBLK:

                /* block special file.
                 */
                /* get info about this block special file.
                 */
                if (rc = fp_ioctl(fp, IOCINFO, &devinfo, NULL))
                {
                        fp_close(fp);
                        return(rc);
                }

                /* make sure that this is a logical volume on a
                 * disk device or that it is an NFS device to be used
		 * for paging.
                 */
                if (devinfo.devtype != DD_DISK ||
                    (devinfo.devsubtype != DS_LV && 
		     devinfo.devsubtype != DS_NFS))
		{
			fp_close(fp);
                        return(ENOTBLK);
		}

		/* set the number of blocks.
		 */
		*nblocks = devinfo.un.dk.numblks;

		rc = 0;
		
                break;

        case S_IFCHR:

                /* character special file.
                 * we could convert this from the raw to the block device.
                 */

        case S_IFREG:

                /* regular file.
                 * NOTE: we may someday allow specifying a regular file
                 * as a paging space to support diskless workstations.
                 */

        default:
                rc = ENOTBLK;
        }

	fp_close(fp);

	return(rc);
}

/*
 * NAME: psdanger
 *
 * FUNCTION:	This is the psdanger system call.
 *		Gives information on paging space conditions.
 *
 * RETURNS:	Returns the difference between the number of free
 *		paging space blocks and the danger or kill levels
 *		depending on the sig argument. If sig is not SIGKILL
 *              or SIGDANGER or zero then the number of free paging
 *              space blocks is returned.  If sig is zero, the total
 *              number of paging space blocks is returned.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      system call
 */
int
psdanger (sig)
int	sig;	/* SIGDANGER or SIGKILL */
{
	int	savevmmsr;
	int	delta;

	savevmmsr = chgsr(VMMSR, vmker.vmmsrval);
	delta = vmker.psfreeblks;
	if (sig == SIGDANGER)
		delta -= pf_npswarn;
	else if (sig == SIGKILL)
		delta -= pf_npskill;
        if (!sig)
                delta = vmker.numpsblks;
	(void)chgsr(VMMSR, savevmmsr);

	return delta;
}
