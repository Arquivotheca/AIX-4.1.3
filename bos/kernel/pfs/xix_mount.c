static char sccsid[] = "@(#)22	1.48.1.24  src/bos/kernel/pfs/xix_mount.c, syspfs, bos41J, 145887.bos 3/3/95 09:03:49";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: copymap, mtopen, pmount, jfs_mount
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/commit.h"
#include "sys/jfsc.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/syspest.h"
#include "sys/sysinfo.h"
#include "sys/malloc.h"

/* Definitions */
# define	UZBIT	((uint)(1<<31))

/* Declarations */
extern struct inode *ifind();
int pmount();
static int validcomp(int);

Simple_lock jfs_comp_lock;       /* global JFS compress lock */
static int kprocinit = 0;

/*
 * The following declaration is present for historical purposes, and
 * allows administrators to tweak the buf structure allocation during
 * runtime.  Otherwise the allocation is computed based upon the 
 * real memory size of the machine.
 */ 
int numfsbufs = 0;

/*
 * NAME:	jfs_mount (vfsp, crp)
 *
 * FUNCTION:	Support for mounts of objects whose paths begin
 *		locally.
 *
 * PARAMETERS:	vfsp	- virtual file system pointer
 *		crp	- credential
 *
 * RETURN :	EBUSY	- device already mounted or open for write
 *		EBUSY	- stubvp already mounted;
 *		EBUSY	- mount table full
 *		ENOTDIR	- stubvp not directory on a device mount
 *		ENXIO	- device open failure
 *			
 */

jfs_mount(vfsp, crp)
struct vfs	*vfsp;		/* Vfs to be mounted		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int	rc, flag, ronly;	/* Return code		*/
	struct vnode *stubvp;		/* Stub vnode		*/
	struct vnode *objectvp = NULL;	/* Mount object vnode	*/
	struct gnode *gp;		/* gnode for objectvp	*/
	fsid_t	fsid;			/* File system id	*/
	dev_t dev;
	dev_t logdev;			/* log device number	*/
	struct inode *ip;		/* root inode		*/
	struct hinode *hip;		/* hash list of inode	*/
	struct vnode *rootvp;		/* root vnode		*/

	/* retrieve covered/stub vnode
	 */
	stubvp = vfsp->vfs_mntdover;

	/*
	 * Resolve mounted object path to a vnode;
	 * inhibit the search from crossing to remote nodes.
	 */

	if (rc = lookupname(vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT),
			SYS, L_SEARCHLOC, NULL, &objectvp, crp))
		return rc;
		
	switch(objectvp->v_vntype)
	{
	case VBLK:

		if (stubvp->v_vntype != VDIR)
		{
			/* The stub for a device mount must
			 * (currently) be a directory.
			 */
			rc = ENOTDIR;
			goto out;
		}

		/* Must be super user to perform 
		 * device mounts.
		 */
		if (privcheck_cr(FS_CONFIG, crp))
		{	
			rc = EPERM;
			goto out;
		}

		gp = VTOGP(objectvp);
		dev = brdev(gp->gn_rdev);
		ronly = (vfsp->vfs_flag & VFS_READONLY);
		flag = FMOUNT | ((ronly)? FREAD: FREAD|FWRITE);

		/* Mount structure is not required, gnode contains all
		 * info needed to keep things straight
		 * Register the mounted device directly through the device
		 * interfaces in order to simply the check for open files
		 * during unmount, see iactivity().
		 */
		vfsp->vfs_data = NULL;
		if (rc = rdevopen(dev, flag, 0, NULL, &vfsp->vfs_data))
			goto out;

		/* pull logdev out of the vfs struct
		 * (*only* if we're mounting read/write...)
		 */
		if (ronly)
			logdev = 0;		/* or -1 or something... */

		else 
		{
			/*
			 * be sure something's there!
			 */
			if (vmt2datasize(vfsp->vfs_mdata, VMT_INFO) < sizeof(logdev)) {
				rdevclose(vfsp->vfs_data, flag, NULL);
				rc = EFORMAT;
				goto out;
				}

			/*
			 * copy log device number out
			 */
			bcopy(vmt2dataptr(vfsp->vfs_mdata, VMT_INFO),
			      (void *) &logdev, sizeof(logdev));
		}

		if (rc = pmount (dev, ronly, minor(logdev)))
		{
			rdevclose(vfsp->vfs_data, flag, NULL);
			goto out;
		}

		/* mark device mount
		 */
		vfsp->vfs_flag |= VFS_DEVMOUNT;

		/* get root inode/vnode
		 */
		IHASH(dev, ROOTDIR_I, hip);
		ICACHE_LOCK();
		rc = _iget(dev, ROOTDIR_I, hip, &ip, 1, vfsp);
		ICACHE_UNLOCK();
		sysinfo.iget++;
		cpuinfo[CPUID].iget++;
		if (rc)
		{
			pumount(dev, ronly);
			rdevclose(vfsp->vfs_data, flag, NULL);
			goto out;
		}
                rootvp = ip->i_gnode.gn_vnode;
		break;

	case VREG:
	case VDIR:

		/* check that the vfs type of the object to be mounted is
		 * right
		 */
		if (objectvp->v_vfsp->vfs_gfs->gfs_type != MNT_JFS)
		{
			rc = EINVAL;
			goto out;
		}

		/* we must have either the mountok flag or suser flag 
		 * to proceed.
		 */
		if (!(vfsp->vfs_flag & (VFS_VMOUNTOK|VFS_SUSER))) {
			rc = EPERM;
			goto out;
		}

		/* object and stub type must match (no dir/reg combo!)
		 */
		if (objectvp->v_vntype != vfsp->vfs_mntdover->v_vntype) {
			rc = ENOTDIR;
			goto out;
		}

		/* if we're doing dir over dir, fail if it we're trying
		 * to mount over a sticky directory, unless of course
		 * we own it, or if we have BYPASS_DAC_WRITE privs.
		 */
		if (stubvp->v_vntype == VDIR &&
		    ((ip = VTOIP(stubvp))->i_mode & S_ISVTX) &&
		      (crp->cr_uid != ip->i_uid) &&
		      (rc = privcheck_cr(BYPASS_DAC_WRITE, crp)))
			goto out;

		vfsp->vfs_data = NULL;

		/* mark directory-on-directory or file-on-file mount
		 */
		vfsp->vfs_flag &= ~VFS_DEVMOUNT;
		vfsp->vfs_flag |= VFS_SOFT_MOUNT;

		/* get root inode/vnode 
		 * inode: acquire reference of mounted object inode
		 * vnode: instantiate a new root vnode linked to
		 *        mounted object inode
		 * Note: object can not be a remote file.
		 */
		ip = VTOIP(objectvp);
		IHASH(brdev(ip->i_dev), ip->i_number, hip);
		ICACHE_LOCK();
		rc = _iget(brdev(ip->i_dev),ip->i_number,hip,&ip,1,NULL);
		ICACHE_UNLOCK();
		sysinfo.iget++;
		cpuinfo[CPUID].iget++;
		if (rc || (rc = iptovp(vfsp, ip, &rootvp)))
			goto out;
		break;			/* switch */

	default:
		rc = ENOTBLK;		/* Generic "bad object" return */
		goto out;
	}

	/* Finish off device specific mount info */
	fsid.fsid_dev = ip->i_dev;
	fsid.fsid_type = MNT_JFS;

	vfsp->vfs_fsid = fsid;		/* fill in the fsid field */
	vfsp->vfs_bsize = PAGESIZE;

	/* fill in the stubs mounted over vfs
	 */
	stubvp->v_mvfsp = vfsp;

	/* identify mounted/root vnode of mounted file system 
	 */
	rootvp->v_flag |= V_ROOT;
	vfsp->vfs_mntd = rootvp;

out:
	if (objectvp)
		VNOP_RELE(objectvp);

	return rc;
}

/*
 * NAME:	pmount (dev, ronly) 
 *
 * FUNCTION:	jfs version of pmount. this code is part of jfs_mount
 *		which does the physical mount of a device.  Bind all special
 *		files to virtual memory.  Initialize .indirect and .diskmap
 *
 * PARAMETERS:	mp	- mount table pointer.
 *
 * RETURN :	Errors from subroutines.
 *			
 */

pmount (dev, ronly, logminor) 
dev_t	dev;
int	ronly;
dev_t	logminor;
{
	dev_t	logdev;
	int	rc, k, bit, word, clearjmp = 0;
	int     logserial, fsize, agsize, iagsize, compress;
	char    *p;
	struct  inode *ip, *iplog, *ipsup, *ipmnt;
	struct 	inode *iopened[16], **ipp = &iopened[0];
	struct  indir *ptr = NULL;
	struct superblock *sb = NULL;
	struct jfsmount *jmp = NULL;
	label_t	jb;
	extern	uint pfs_memsize();

	/* check the superblock and get fragment size and
	 * allocation group sizes.  (binval in case fsck made changes
	 * to sup block)
	 */
	binval(dev);
	if (rc = chksupblock(dev,&fsize,&agsize,&iagsize, &compress))
		return(rc);

	/* zero iopened to make "cleanup a snap"
	 */
	bzero (ipp, sizeof (iopened));

	/*
	 * open the log unless it was mounted read-only.
	 *
	 * logdev is supposed to passed as a paramter to pmount.
	 * for now we use the (old) convention that there is a
	 * reserved minor number for the log device.
	 * logopen also puts device in its active list.
	 */

	if (ronly)
	{
		iplog = NULL;
		logdev = 0;
	}
	else
	{
		logdev = makedev(major(dev), logminor);
		if (rc = logopen (logdev,dev,&logserial))
			return rc;
		ICACHE_LOCK();
		iplog = ifind (logdev);
		ICACHE_UNLOCK();
	}

	/* put dev in paging device table.
	 * allocate numfsbufs buf structs for it.
	 * If numfsbufs does not equal to zero, then it has
	 * already been computed, or changed thru /dev/kmem.
	 * If it is not set then call pfs_memsize():
	 * If the real memory is less than or equal 16 meg
	 * then allocate one page, otherwise allocate two pages.
	 */
	if (!numfsbufs) 
	{
		if (pfs_memsize() <= 0x1000000)
			numfsbufs = PAGESIZE / sizeof(struct buf);
		else
			numfsbufs = 2 * PAGESIZE / sizeof(struct buf);
	}

	if (rc = vm_mountx(D_FILESYSTEM, dev, numfsbufs, fsize, compress))
	{
		if (iplog)
		{
			logclose (iplog,dev);
		}
		return rc;
	}

	/* get "mount inode" to represent mounted filesystem.
	 *
	 * we use a conventional inode number of 0 and the devt of
	 * the mounted filesystem. fill in ip of log now so that
	 * info is available for subsequent ibindsegs. it is necessary
	 * to get this before looking at superblock because the
	 * superblock ip needs the pointer to it.
	 */
	if (rc = mtopen(dev, 0, ipp))
		goto bad;

	ipmnt = *ipp++;
	ipmnt->i_iplog = iplog;

	/* initialize the number of fragments per block in the mount
	 * inode.  fragment size is maintained in this manner for
	 * interpreting the nfrags portion of file system disk 
	 * addresses (i.e. fperpage - nfrags). initialize the disk
	 * fragment and disk inode allocation group sizes.
	 */
	ipmnt->i_fperpage = PAGESIZE / fsize;
	ipmnt->i_agsize = agsize;
	ipmnt->i_iagsize = iagsize;
	ipmnt->i_fscompress = compress;

	/* initialize mount structure.
	 */
	if ((jmp = (struct jfsmount *)malloc(sizeof(struct jfsmount))) == NULL)
	{
		 	rc = ENOMEM;
			goto bad;
	}

	jmp->jm_dev = dev;
	for (k = 0; k < MAXQUOTAS; k++) {
		jmp->jm_quotas[k] = NULL;
		jmp->jm_qflags[k] = 0;
	}
	ipmnt->i_jmpmnt = jmp;

	/* open the superblock. 
	 */
	if (rc = mtopen (dev, SUPER_I, ipp))
		goto bad;

	ipmnt->i_ipsuper = ipsup = *ipp++;

	/* map superblock into VM. 
	 */
	if (rc = iptovaddr (ipsup, 0, &sb))
		goto bad;

	/* establish exception return point for 
	 * permanent io errors. errors may occur as result of loads
	 * or stores into memory mapped files.
	 */
	if (rc = setjmpx (&jb))
	{	rc |= PFS_EXCEPTION;
		clearjmp = 0;
		goto bad;
	}

	clearjmp = 1;

	/* this check was all ready done, but this time do it with
	 * the superblock mapped.
	 */
	if (strncmp (sb->s_magic, fsv3pmagic, 4) != 0 &&
	    strncmp (sb->s_magic, fsv3magic, 4) != 0)
	{
		rc = EINVAL;
		goto bad;
	}

	/* if compression initialize storage. 
	 * if not read-only create compression kproc.
	 */
	if (compress)
	{
		if (validcomp(compress))
		{	
			rc = ENOSYS;
			goto bad;
		}
		if (rc = initcomp(ronly))
			goto bad;
	}

	/* if it is mounted read-write. set state in superblock:
	 * if it was cleanly unmounted (or fsck was run) mark
	 * it as mounted-cleanly. otherwise state is mounted-dirty.
	 * also fill in log's devt and its serial number.
	 */
	if (!ronly)
	{	
		/* Mark fs super block as mounted dirty or cleanly
		 */
	 	sb->s_ronly = 0;
		sb->s_fmod = (sb->s_fmod == FM_CLEAN) ? FM_MOUNT : FM_MDIRTY;
		sb->s_logdev = logdev;
		sb->s_logserial = logserial;

		/* write out superblock synchronously and unmap it.
	 	*/
		iflush(ipsup);
		if (rc = vms_iowait(ipsup->i_seg))
			goto bad;

		/* put mount record into log. it keeps logredo 
		 * from going past this point in log. it is harmless
		 * if mount fails.
		 */
		mountrecord(iplog, dev);

	}

	/* finished with super block 
	 */
	ipundo(sb);
	sb = NULL;

	/* open .indirect before the others
	 *
	 * (necessary since it is used in opening of all segments
	 * which have indirect blocks).
	 */
	if (rc = mtopen(dev, INDIR_I, ipp))
		goto bad;

	ipmnt->i_ipind = ip = *ipp++;

	if (rc = iptovaddr(ipmnt->i_ipind, 0, &ptr))
		goto bad;

	/* pages 0 to FIRSTIND-1 of .indirect are used for
	 * the indirect blocks of .indirect. page 0 is used
	 * for the double indirect block and 1 thru FIRSTIND -1
	 * for the single indirect blocks. 
	 */

	/* make page 0 the double indirect block.
	 * disk addresses for page 0 and 1 are in the inode.
	 */
	ip->i_vindirect = 0;
	ip->i_rindirect = ip->i_rdaddr[0];
	ip->i_disize = 1 << SEGSHIFT;
	for (k = 0; k < FIRSTIND - 1 ; k++)
	{	ptr->root[k].id_vaddr = k + 1;
		ptr->root[k].id_raddr = 0;
	}
	ptr->root[0].id_raddr = ip->i_rdaddr[0];

	/* fill in first two entries of the first single indirect
	 * block to reflect use of pages 0 and 1. set indptrs for
	 * the rest of the indirect blocks of .indirect itself
	 * to zero (makes them logically zero - no disk block).
	 */
	ptr->indptr[0] = ip->i_rdaddr[0];
	ptr->indptr[1] = ip->i_rdaddr[1];
	for ( k = 2; k < FIRSTIND; k++)
		ptr->indptr[k] = 0;

	/* put the remainder of the pages covered by the first
	 * indirect block on the free list.
	 */
	ptr->free = FIRSTIND;
	ptr->more = PAGESIZE/4;
	ptr->freebacked = 0;
	ptr->indptr[PAGESIZE/4 - 1] = 0;
	for (k = FIRSTIND; k < PAGESIZE/4 - 1; k++)
		ptr->indptr[k] = k + 1;

	/* finished with .indirect
	 */
	ipundo (ptr);
	ptr = NULL;

	clrjmpx (&jb);		/* pop exception return */
	clearjmp = 0;

	/* open .inodes next.
	 *
	 * ipmnt needs pointer to .inodes before the rest of
	 * the mtopens.
	 */
	if (rc = mtopen(dev, INODES_I, ipp))
		goto bad;
	ipmnt->i_ipinode = *ipp++ ;

	/* open .inodemap
	 */
	if (rc = mtopen(dev, INOMAP_I, ipp))
		goto bad;
	ipmnt->i_ipinomap = *ipp++ ;

	/* open .diskmap. 
	 */
	if (rc = mtopen(dev, DISKMAP_I, ipp))
		goto bad;
	ipmnt->i_ipdmap =  *ipp;

	/* make disk map segment id known to the VMM.
	 */
	if (rc = vm_definemap(dev,(*ipp++)->i_seg))
		goto bad;

	/* open .inodex. 
	 */
	if (rc = mtopen(dev, INODEX_I, ipp))
		goto bad;
	ipmnt->i_ipinodex =  *ipp;

	binval(dev);

	return 0;

bad:

	if (clearjmp)
		clrjmpx(&jb);
	if (sb)
		ipundo(sb);
	if (ptr)
		ipundo(ptr);

	if (iplog)
		logclose(iplog,dev);

	ICACHE_LOCK();
	for (ipp = iopened; *ipp; ipp++) {
		iuncache(*ipp);
		iunhash(*ipp);
	}
	ICACHE_UNLOCK();

	if (jmp)
		free(jmp);

	vm_umount(D_FILESYSTEM, dev);

	binval(dev);

	RETURNX(rc, reg_elist);
}

/*
 * chksupblock(dev,fragsize,agsize,iagsize,compress)
 *
 * check the super block of the file system to be mounted and get the
 * file system fragment size and disk fragment and disk inode allocation
 * group sizes.  the read is done through the buffer pool because the file
 * system can not be trusted at this point
 *
 * returns
 *	0 with fragsize set if check successful
 *	error code if not successful
 */
int
chksupblock(dev,fragsize,agsize,iagsize,compress)
int dev;
int *fragsize;
int *agsize;
int *iagsize;
int *compress;
{
	struct buf *bp, *bread();
	struct superblock *sb;
	int rc;

	rc = 0;
	bp = bread(dev, (daddr_t)(SUPER_B * PAGESIZE/SBUFSIZE));

	if (bp->b_error) {
		rc = EIO;
		goto bad;
	}

	sb = (struct superblock *)(bp->b_un.b_addr);

	/* check the superblock magic number.
	 */
	if (strncmp(sb->s_magic, fsv3pmagic, 4) == 0)
	{
		/* the version number must also checkout for fsv3p
		 * file systems.
		 */
		if (sb->s_version != fsv3pvers)
		{
			rc = EINVAL;
			goto bad;
		}

		*fragsize = sb->s_fragsize;
		*agsize = sb->s_agsize;
		*iagsize = sb->s_iagsize;
		*compress = sb->s_compress;
	}
	else if (strncmp(sb->s_magic, fsv3magic, 4) == 0)
	{
		*fragsize = PAGESIZE;
		*agsize = *iagsize = sb->s_agsize;
		*compress = 0;
	}
	else
	{
		rc = EINVAL;
		goto bad;
	}

	/*
	 * Check that the entire fs does not exceed 
	 * FS_MAXSIZE (frag, ag, iag) which returns max fs size in frags.
	 */
	if ((uint) (sb->s_fsize / (*fragsize / UBSIZE))  > 
	    		       FS_MAXSIZE(*fragsize, *agsize, *iagsize))
	{	
		BUGPR(("sb->s_fsize cannot exceed FS_MAXSIZE(fs,ag,iag)\n"));
		rc = EFBIG;
		goto bad;
	}


	/*
	 * Check that the fs isn't dirty (fsck failed to fix it).
	 */
	if (sb->s_fmod != FM_CLEAN)
	{
		BUGPR(("sb->s_fmod isn't clean: %d\n", sb->s_fmod));
		rc = EFORMAT;
		goto bad;
	}

bad:
	brelse(bp);
	return(rc);

}

/*
 *	mtopen (dev, ino, ipp)
 *
 * open inode specified (_iget()/ibindseg())
 */
static 
mtopen (dev, ino, ipp)
dev_t	dev;
int	ino;
struct inode **ipp;
{
	struct hinode *hip;
	int rc;

	IHASH(dev, ino, hip);
	ICACHE_LOCK();
	rc = _iget(dev, ino, hip, ipp, 1, NULL);
	ICACHE_UNLOCK();
	sysinfo.iget++;
	cpuinfo[CPUID].iget++;

	/*
	 * skip ibindseg() for mount inode (i_number = 0)
	 */
	if (rc || ino == 0)
		return(rc);

	if ((rc = ibindseg(*ipp)) != 0)
	{
		ICACHE_LOCK();
		iuncache(*ipp);
		iunhash(*ipp);
		ICACHE_UNLOCK();
	}

	return rc;
}

/*
 * mountrecord(iplog,device)
 *
 * put a mount record for device into log.
 *
 */

static
mountrecord(iplog, device)
struct inode *iplog;
dev_t device;
{
	struct logrdesc lr;

	lr.backchain = 0;
	lr.transid = 0;
	lr.type = MOUNT;
	lr.length = 0;
	lr.log.mnt.volid = device;
	logmvc(iplog, &lr, NULL, NULL);

	return 0;
}

/* 
 *  initialization for file compression.
 */
static
int
initcomp(rdonly)
int rdonly;
{

	pid_t pid;
	int vm_ckproc(), rc;

	JFS_COMP_LOCK();
	
	if (rc = vm_initcomp(rdonly))
		goto out;

	if (rdonly)
		goto out;

	/* done before ?
	 */
	if (kprocinit != 0)
		goto out;

	/* create process
	 */
	if ((pid = creatp()) == -1)
	{
		rc = ENOMEM;
		goto out;
	}

	/* initialize it to run vm_ckproc as its code
	 */
	if (rc = initp(pid, vm_ckproc, NULL, 0, "jfsc"))
		goto out;

	/* set its priority 
	 */
	setpri(pid, 30);
	kprocinit = 1;

out:
	JFS_COMP_UNLOCK();
	return rc;
}

/*
 * NAME: 	validcomp()
 *
 * FUNCTION: 	Determines if specified compression type is supported.
 *
 * RETURNS: 	 0 	- Compression algorithm supported
 *		-1	- Invalid compression type
 */
static int
validcomp(int type)
{
	/* 
	 * compp is an exported function pointer filled in by the 
	 * compression kernel extension
 	 */
	if (compp == NULL)
		return(-1);
	else
		return((*compp)(COMP_QUERY,type,NULL,0,NULL,0));
}
