static char sccsid[] = "@(#)05	1.14.1.16  src/bos/kernel/pfs/xix_cntl.c, syspfs, bos41J, 9516A_all 4/17/95 13:17:32";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_cntl, extendfs, addgroups
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/fscntl.h"
#include "sys/devinfo.h"
#include "sys/ioctl.h"
#include "sys/device.h"
#include "sys/malloc.h"
#include "sys/sysinfo.h"
#include "vmm/vmsys.h"

int extendfs();

/*
 * NAME:	jfs_cntl(vfsp, cmd, arg, argsize, crp)
 *
 * FUNCTION:	implement VFS_CNTL
 *
 * PARAMETERS:
 *		crp	- credential
 *
 * RETURN:	EINVAL on invalid 'cmd', else return valid from function
 *			
 */

int
jfs_cntl(vfsp, cmd, arg, argsize, crp)
struct vfs	*vfsp;
int		cmd;
caddr_t		arg;
size_t		argsize;
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;

	switch (cmd) {
		case FS_EXTENDFS:
			rc = extendfs(vfsp, arg, argsize, crp, 0);
			break;
		case FS_MOVEBLKS:
			rc = moveblocks(vfsp, arg, argsize, crp);
			break;
		case FS_EXTENDFS_EXTRA:
			/* overallocate maps by 1 map page */
			rc = extendfs(vfsp, arg, argsize, crp, 1);
			break;
		default:
			rc = EINVAL;
			break;
		}

	return (rc);
}

/*
 * NAME:	extendfs(vfsp, arg, argsize, crp, extra)
 *
 * FUNCTION:	implement extendfs function for VFS_CNTL()
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:	0 on success, else errno
 */

BUGVDEF(xtdebug, 0)		/* define debug variable */

int wont_fit();

static int
extendfs(vfsp, arg, argsize, crp, extra)
struct vfs	*vfsp;
caddr_t		arg;
size_t		argsize;
struct ucred	*crp;
int		extra;	/* number of map pages to overallocate */
{
	int rc;
	size_t ublocks = (size_t) arg;	/* UBSIZE blocks 	*/
	size_t fragments;		/* ublocks in fragments */
	struct vnode *devvp;		/* vnode to dev name	*/
	struct inode *ipd;		/* .diskmap inode	*/
	struct inode *ipi;		/* .inodes inode        */
	struct inode *ipm;		/* .inode map inode	*/
	struct inode *mounti;		/* mount inode		*/
	struct inode *superi;		/* super block inode	*/
	struct superblock *sb, *sb1;	/* superblock		*/
	struct vmdmap *vmd;		/* 1st disk map page	*/
	struct vmdmap *vmimap;		/* 1st inode map page	*/
	struct inode *vmi;		/* 1st page of .inodes */
	label_t   jb;      		/* setjmpx/longjmpx buffer 	*/
	int	imapsize;		/* new inode map size 	*/
	int	fragsize;		/* fragment size	*/


	/*
	 * be sure this was a block device mount...
	 */
	if (vfsp->vfs_data == NULL)
		return (ENOTBLK);

	/*
	 * check for readonly filesystems here.  we apparently don't 
	 * cleanup on eio's very well down below (TODO!)
	 */
	if (vfsp->vfs_flag & VFS_READONLY)
		return (EROFS);

	/* round ublocks to PAGESIZE.
	 */
	ublocks = (ublocks / (PAGESIZE/UBSIZE)) * (PAGESIZE/UBSIZE);
	devvp = NULL;
	superi = NULL;
	sb = NULL;
	vmi =  NULL;
	vmd = vmimap =  NULL;
	ipm = NULL;

	/* get a vnode on the vfs
	 */
	if (rc = jfs_root(vfsp, &devvp, crp))
		return (rc);

	/* get the 'mount' inode
	 */
	mounti = VTOIP(devvp)->i_ipmnt;

	/* map inode map into VM
	 * the lock on ipm is used to serialize operation.
	 */
	ipm = mounti->i_ipinomap;
	IWRITE_LOCK(ipm);

	/* check perms
	 */
	if (privcheck_cr(FS_CONFIG, crp))
	{
		rc = EPERM;
		goto out1;
	}

	/* see if we're trying to extend pass the end of the lv...
	 */
	if (rc = wont_fit(vfsp, ublocks))
		goto out1;

	if (rc = iptovaddr(ipm, 0, (caddr_t *) &vmimap))
		goto out1;

	/* map diskmap into `vmd'
	 */
	ipd = mounti->i_ipdmap;
	if (rc = iptovaddr(ipd, 0, (caddr_t *) &vmd))
		goto out1;

	/* map inodes into VM
	 */
	ipi = mounti->i_ipinode;
	if (rc = iptovaddr(ipi, 0, (caddr_t *) &vmi))
		goto out1;

	/* get the superblock inode 
	 * map it into virtual memory.
	 */
	superi = mounti->i_ipsuper;
	if (rc = iptovaddr(superi, 0, (caddr_t *) &sb))
		goto out1;

	/* establish an execption handler in case of i/o error.
	 * or running out of disk-space.
	 */
	if (rc = setjmpx (&jb))
	{	
		goto out1;
	}
	
	/* get fragment size and determine the number of fragments.
	 */
	fragsize = PAGESIZE / mounti->i_fperpage;
	fragments = ublocks / (fragsize / UBSIZE);

	/* see if the filesystem is too large.
	 */
	if ((uint) fragments > 
	    FS_MAXSIZE(fragsize, mounti->i_agsize, mounti->i_iagsize))
	{
		rc = EINVAL;
		goto out;
	}

	/* check requested fragments against current size
	 */
	if (fragments < vmd->mapsize) {
		/* smaller than what we already have - can't shrink yet
		 */
		rc = EINVAL;
		goto out;
	}

	if (fragments == vmd->mapsize) {
		/* same size.  leave quietly
		 */
		rc = 0;
		goto out;
	}

	/* add allocation groups 
	 */
	if (rc = addgroups(fragments, vmd, vmi, vmimap,&imapsize,fragsize,
			   extra))
		goto out;

	/* commit changes to the two maps as files.
	 */
	ipd->i_flag |= IFSYNC;
	ipm->i_flag |= IFSYNC;
	if (rc = commit(2, ipd, ipm))
		goto out;

	/* to make sure the inodes for the maps get written
	 * out we sync .inodes.
	 */
	vcs_sync(ipi, NULL);
	if (rc = vms_iowait(ipi->i_seg))
		goto out;


	/* update primary superblock.
	 * update secondary superblock
	 * Change the version and magic numbers if this is a v3
	 * filesystem and the size is now > 2G.
	 */
	sb->s_fsize = fragments * (fragsize/UBSIZE);
	sb1 = (struct superblock *) ( (char *)sb + PAGESIZE);
	sb1->s_fsize = fragments * (fragsize/UBSIZE);
	
	if (strncmp (sb->s_magic, fsv3magic, sizeof (sb->s_magic)) == 0 &&
	    sb->s_fsize > FS_V3MAXSIZE)
	{
		strncpy (sb->s_magic, fsv3pmagic, sizeof (sb->s_magic));
		sb->s_version = fsv3pvers;
		sb->s_fragsize = PAGESIZE;
		sb->s_iagsize = sb->s_agsize;
		sb->s_compress = 0;

		strncpy (sb1->s_magic, fsv3pmagic, sizeof (sb->s_magic));
		sb1->s_version = fsv3pvers;
		sb1->s_fragsize = PAGESIZE;
		sb1->s_iagsize = sb->s_agsize;
		sb1->s_compress = 0;				
	}
	 
	/* write the two superblocks out.
	 * if this write is successful the extension of the
	 * file system is committed. the writes below of the
	 * diskmap and inodemap are redone in logredo if the
	 * system crashes before doing them.
	 */
	vm_write(sb, SEGSIZE, V_FORCE);
	if (rc = vms_iowait(superi->i_seg))
		goto out;

	/* update size of inode map. write it out.
	 */
	vmimap->mapsize = imapsize;
	vm_write(vmimap, PAGESIZE, V_FORCE);
	if (rc = vms_iowait(ipm->i_seg))
		goto out;

	/* update size of diskmap. write it out
	 */
	vmd->mapsize = fragments;
	vm_write(vmd, PAGESIZE, V_FORCE);
	rc = vms_iowait(ipd->i_seg);

out:
	clrjmpx(&jb);

out1:

	if (vmd) 
		ipundo(vmd);
	if (sb)
		ipundo(sb);
	if (vmi)
		ipundo(vmi);
	if (vmimap)
		ipundo(vmimap);

	IWRITE_UNLOCK(ipm);

	jfs_rele(devvp);

	return (rc);
}

/*
 * addgroups(newsize, vmd, vmi, vmimap, imapsize, fragsize, extra)
 *
 * formats any additional allocation groups needed to grow
 * to newsize: new inode blocks are allocated and cleared,
 * and the inode map extended. the mapsize field in the disk
 * and inode maps are NOT updated.
 *
 * return value
 *	0 	- ok.
 *	EIO	- error writing out disk map, inode map, or .inodes.
 *	E2BIG	- new file system size make .inode too large.
 */

static int
addgroups(newsize, vmd, vmi, vmimap, imapsize, fragsize, extra)
int newsize;		/* new size in fragments  */
struct vmdmap * vmd;	/* pointer to disk map    */
struct inode *vmi;	/* pointer to .inodes 	  */
struct vmdmap * vmimap;	/* pointer to inode map   */
int *imapsize;		/* new size of inode  map */
int fragsize;		/* fragment size	  */
int extra;		/* num mp to overallocate */
{
	int agsize, iagsize, oldags, newags, k, n, dblk, iaglen;
	int rc, sid, numinodes, minfrags;

	/* get the current number of allocation groups and the
	 * allocation group sizes.  get sid of .inodes.
	 */
	oldags = vmd->totalags;
	agsize = vmd->agsize;
	iagsize = vmimap->agsize;
	sid = SRTOSID(mfsri(vmi));

	/* determine the length in bytes of each group's disk inodes
	 * and the number of fragments required to hold these disk
	 * inodes.
	 */
	iaglen = iagsize * sizeof(struct dinode);
	minfrags = iaglen / fragsize;

	/* determine the number of new allocation groups.
	 */
	newags = newsize / agsize;
	newags = (newsize - newags * agsize >= minfrags) ? newags + 1 : newags;

	/* determine inode map size and make sure that the new file
	 * system size will not make .inodes too large.
	 */
	numinodes = newags * iagsize;
	if (numinodes * sizeof(struct dinode) > MAXFSIZE)
		return(E2BIG);

	/* get backing resources for the new map pages.
	 */
	extendmap(newsize,vmd,extra);
	extendmap(numinodes,vmimap,extra);

	/* zero out the new disk inodes.
	 */
	for (k = oldags; k < newags; k++)
	{
		if (rc = zeroinos(vmi,k,iaglen))
			return(rc);
	}

	/* grow the diskmap. vm_growmap() does not update mapsize.
	 */
	if (rc = vm_growmap(vmd, newsize, FSDISKMAP, minfrags))
		return rc;

	/* mark as allocated the disk blocks in the disk map
	 * covering the new inodes.
	 */
	for (k = oldags; k < newags; k++)
	{
		dblk = k * agsize;
		for (n = 0; n < minfrags; n++)
		{
			vcs_mapd(sid,dblk + n);
		}
	}

	/* extend inode map ?
	 */
	*imapsize = numinodes;
	if (vmimap->mapsize != numinodes);
	{
		if (rc = vm_growmap(vmimap, numinodes, INODEMAP, 0))
			return rc;
	}

	/* write out the diskmap and the inodemap.
	 */
	vm_write(vmd,SEGSIZE,V_FORCE);
	if (rc = vms_iowait(SRTOSID(mfsri(vmd))))
		return(rc);
	vm_write(vmimap,SEGSIZE,V_FORCE);
	rc = vms_iowait(SRTOSID(mfsri(vmimap)));

	return(rc);
}

/* 
 * extendmap(newsize, vmp, extra)
 *
 * store into any new pages needed for specified map causing
 * backing disk resources to be allocated.
 *
 */
static
extendmap(newsize, vmp, extra)
int newsize;
struct vmdmap * vmp;
int extra;		/* num map pages to overallocate */
{
	int old, new, k;
	uint version, dbperpage, np;

	/* store into any additional pages of specified map to
	 * allocate backing resources.  if there are insufficient
	 * resources an exception will take us back to the main
	 * procedure.
	 */
	version = vmp->version;
	dbperpage = WBITSPERPAGE(version);
	old = (vmp->mapsize  + dbperpage - 1)/dbperpage;
	new = (newsize + dbperpage - 1)/dbperpage + extra;
	for (k = old; k < new; k++)
	{
		/* np is page in file
		 */
		if (version == ALLOCMAPV3)
		{
			np = k;
		}
		else
		{
			/* need a control page ? */
			if ((k & 0x7) == 0)
			{
				np = 9*(k >> 3);
				(vmp + np)->mapsize = 0;
			}
			np = 1 + 9*(k >> 3) + (k & 0x7);
		}
		/* touch last word in page to guarantee 4k allocation
		 * (this is the only place that touches overallocated block)
		 */
		*((int *)(vmp + np) + sizeof(*vmp) / sizeof(int) - 1) = 0;
	}
	return;
}

/*
 * zeroinos(vmi,ag,iaglen)
 *
 * zero the disk fragments (pages within .inodes) for the inodes of an
 * allocation group. 
 *
 * return values:
 *
 *	0	- ok
 *	EIO	- I/O error on .inodes
 *
 */

static
zeroinos(vmi, ag, iaglen)
uint vmi;		/* address of .inodes		*/ 
int ag;			/* allocation group number	*/
int iaglen;		/* len of disk inode allocation group in bytes */
{
	int npages, pfirst, sid, sidx, n, page, target, sidnext, rc;

	/* calculate number pages, get sid currently associated 
	 * with .inodes.
	 */
	npages = iaglen >> L2PSIZE; 
	sid = SRTOSID(mfsri(vmi));
	sidx = STOI(sid);

	/* zero one page at a time just in case we cross a segment
	 * boundary.
 	 */
	pfirst = ag * npages;
	for (n = 0; n < npages; n++)
	{
		page = pfirst + n;
		sidnext = ITOS(sidx, page);

		/* check if we are crossing a boundry.  if so, load
		 * the next segment register value.
		 */
		if (sidnext != sid) 
		{
			sid = sidnext;
			(void)chgsr((uint)vmi >> L2SSIZE,SRVAL(sid,0,0));
		}
		target = vmi + ((page << L2PSIZE) & SOFFSET);
		bzero (target, PAGESIZE);
	}

	/* write pages to disk 
	 */
	vm_writep(sid, pfirst, npages);
	rc = vms_iowait(sid);
	vm_releasep(sid, pfirst, npages);
	return rc;
}

/*
 *	wont_fit
 *
 *	- return 0 if their request will fit on this logical volume,
 *	  else return error code
 */
static int
wont_fit(struct vfs *vfsp, size_t ublocks)
{
	dev_t devno;
	struct file *fp;
	register int rc;
	struct devinfo devinfo;

	devno = brdev(((struct gnode *) vfsp->vfs_data)->gn_rdev);

	/*
	 * open the dev (vfs_data is the gnode for the object device)
	 */
	if ((rc = fp_opendev(devno, DREAD, (caddr_t) NULL, 0, &fp)) == 0) {
		/*
		 * ioctl it
		 */
		if ((rc = fp_ioctl(fp, IOCINFO, &devinfo, 0)) == 0)
			/*
			 * check the size
			 */
			rc = (ublocks > devinfo.un.dk.numblks) ? EFBIG : 0;

		(void) fp_close(fp);
		}

	return (rc);
}

/*
 * NAME:	moveblocks(vfsp, arg, argsize, crp)
 *
 * FUNCTION:	implement moveblks function for VFS_CNTL().
 * 		moves a logical block of a file from its current
 *		disk address to a new address, optionally commiting
 *		the operation.
 *
 * PARAMETERS:  
 *	vfsp - pointer to vfs for filesystem
 *	arg  - pointer to plist as defined below.
 *	argsize - should be size of plist, but is ignored.
 *	crp	- credential
 *
 * plist is a structure that specifies the operation:
 *
 *	struct plist {
 *		int	ino; 	
 *		int	gen;
 *		int	bno;
 *		frag_t  oldfrag;
 *		frag_t  newfrag; 
 *		int	commit; 
 *	};
 *
 * The ino field specifies the file which may be a regular file,
 * directory or long symbolic link. bno is the logical block
 * number. The oldfrag field should be bno's current disk assignment,
 * and the newfrag field is the proposed place to move the logical
 * block. The move fails if either oldfrag is not current, or if
 * newfrag can not be allocated. In addition, the operation fails
 * if the file is currently being modified. if commit is non-zero,
 * the operation is committed.
 *
 * if oldfrag == newfrag, and commit is non-zero, the operation
 * is just a commit.
 *
 * RETURN VALUES:
 *	0    - ok
 *	EFAULT - bad address for plist
 *	EINVAL - invalid parameters in plist
 *	ESTALE - oldfrag not current or newfrag not free
 *	EBUSY  - file currently being modified.
 *	EIO    - I/O error encountered.
 *	EROFS  - file system is mounted read-only.
 *	EACCESS - access permissions 
 *
 * this service is used by the command defragfs.
 *
 */

static int
moveblocks(vfsp, arg, argsize, crp)
struct vfs	*vfsp;
caddr_t		arg;
size_t		argsize;
struct ucred	*crp;
{
	struct hinode *hip;
	label_t   jb;      
	uint   rc, inode, type, diskallocated, sid;
	struct vnode *devvp;		
	struct inode *ipm, *ipmnt, *ip;
	struct vmdmap *vmimap;	
	struct gnode * gp;
	struct plist {
		uint	ino; 	/* inode number of file */
		uint	gen;	/* inode generation number */
		uint	bno;	/* page number in file */
		frag_t  oldfrag; /* current frag_t for bno */
		frag_t  newfrag; /* new proposed frag_t for bno */
		int	commit;  /* 0 no commit, otherwise commit */
 	} p;

	/* check for readonly filesystems. 
	 */
	if (vfsp->vfs_flag & VFS_READONLY)
		return (EROFS);

	/* get a vnode on the vfs
	 */
	if (rc = jfs_root(vfsp, &devvp, crp))
		return (rc);

	/* init pointers and flags for error cleanup.
	 */
	ip = NULL;
	vmimap = NULL;
	diskallocated = 0;

	/* copy in the plist
	 */
	if (rc = copyin(arg, (caddr_t)&p, sizeof (struct plist)))
		goto out1;

	/* test for illegal ino.
	 */
	inode = p.ino;
	if (inode <= SPECIAL_I && inode != ROOTDIR_I)
	{
		rc = EINVAL;
		goto out1;
	}

	/* test for silly values
	 */
	if (p.oldfrag.nfrags != p.newfrag.nfrags)
	{
		rc = EINVAL;
		goto out1;
	}

	/* check permissions
	 */
	if (privcheck_cr(FS_CONFIG, crp))
	{
		rc = EPERM;
		goto out1;
	}

	/* get the 'mount' inode
	 */
	ipmnt = VTOIP(devvp)->i_ipmnt;

	/* map inode map into VM
	 */
	ipm = ipmnt->i_ipinomap;
	if (rc = iptovaddr(ipm, 0, (caddr_t *) &vmimap))
		goto out1;

	/* establish an exception handler in case of i/o error.
	 */
	if (rc = setjmpx (&jb))
	{	
		goto out1;
	}

	/* test that ino is not too big.
	 */
	if (inode >= vmimap->mapsize)
	{
		rc = EINVAL;
		goto out;
	}

	/* find the hash list
	 */
	IHASH(ipmnt->i_dev, inode, hip);

	/* lock the file 
	 */
	ICACHE_LOCK();
	rc = _iget(ipmnt->i_dev, inode, hip, &ip, 1, NULL);
	ICACHE_UNLOCK();
	sysinfo.iget++;
	cpuinfo[CPUID].iget++;
	if (rc)
		goto out;
	IWRITE_LOCK(ip);

	/* test that it has a positive disk link count and generation 
	 * number is OK.
	 */
	if (ip->i_nlink == 0 || ip->i_gen != p.gen)
	{
		rc = ESTALE;
		goto out;
	}

	/* test that its a regular file, or directory, or long
	 * symbolic link.
	 */
	type = ip->i_mode & IFMT;
	switch (type) {
	case IFREG:
	case IFDIR:
		break;
	case IFLNK:
		if (ip->i_size > D_PRIVATE)
			break;
	default:
		rc = EINVAL;
		goto out;
	}

	/* test that page is ok
	 */
	if (p.bno > BTOPN(ip->i_size))
	{
		rc = EINVAL;
		goto out;
	}
		
	/* test for others trying to write if not directory.
	 * also don't allow deferred update.
	 */
	if (ip->i_count > 1 && type != IFDIR)
	{
		gp = ITOGP(ip);
		if (gp->gn_wrcnt || ip->i_flag & IDEFER)
		{
			rc = EBUSY;
			goto out;
		}
	}

	/* associate with virtual memory segment if necessary
	 */
	if (ip->i_seg == 0)
	{
		if (rc = ibindseg(ip))
			goto out;
	}
	sid = ip->i_seg;

	/* is it only a commit request
	 */
	if (p.oldfrag.addr == p.newfrag.addr)
		goto commit;

	/* try to allocate newfrag
	 */
	if (rc = vcs_allocfree(sid, p.newfrag, 1))
		goto out;
	diskallocated = 1;

	/* if file is not a directory, copy oldfrag to newfrag.
	 * copy is essential for compressed files to avoid problems
	 * that can arise if there was a change in compression
	 * algorithms. it is a good strategy for regular files
	 * because we probably don't want to keep the pages in
	 * memory afterwards . however, the directory case requires 
	 * that the page be brought into memory for journalling.
	 */ 
	if (type != IFDIR)
	{
		/* flush the page in case it was in memory
		 */
		if (rc = vm_writep(sid, p.bno, 1))
			goto out;
		if (rc = vms_iowait(sid))
			goto out;
		/* copy it to newfrag
		 */
		if (rc = copydisk(ipmnt, p.oldfrag, p.newfrag))
			goto out;
	}

	/* ask vmm to move it
	 */
	if (rc = vcs_movedaddr(sid, p.bno, p.oldfrag, p.newfrag))	
		goto out;

	/* mark inode as having been updated. 
	 * commit it if asked for. reset diskallocated in case commit fails.
	 */
	ip->i_flag |= IUPD;

commit:
	if (p.commit)
	{
		diskallocated = 0;
		ip->i_flag |= IFSYNC;
		rc = commit(1,ip);
	}
		
out:
	clrjmpx(&jb);

out1:
	if (vmimap) 
		ipundo(vmimap);

	if (rc && diskallocated)
		vcs_allocfree(sid, p.newfrag, 0);

	if (ip)
	{
		IWRITE_UNLOCK(ip);

		ICACHE_LOCK();
		iput(ip, NULL);
		ICACHE_UNLOCK();
	}

	jfs_rele(devvp); 

	return (rc);
}

/*
 * copy source fragments to target fragments
 */
static int
copydisk(ipmnt, source, target)
struct inode * ipmnt;
frag_t source;
frag_t target;
{
	dev_t dev;
	int fperpage, nbytes, fragsize, rc;
	char *buffer;
	struct uio uio;
	struct iovec iov;

	/* allocate page size buffer. it is aligned on a 
	 * page boundary.
	 */
	if ((buffer = xmalloc(PSIZE, L2PSIZE, kernel_heap)) == NULL)
		return ENOMEM;

	/* calculate length of operation and fill in the iov
	 */
	fperpage = ipmnt->i_fperpage;
	fragsize = PSIZE/fperpage;
	nbytes = (fperpage - source.nfrags)*fragsize;
	iov.iov_len = nbytes;
	iov.iov_base = buffer;
	dev = ipmnt->i_dev;

	/* read the source
	 */
	uio.uio_iov = &iov;
	uio.uio_segflg = SYS_ADSPACE;
	uio.uio_fmode = 0;
	uio.uio_iovcnt = 1;
	uio.uio_resid = 0;
	uio.uio_offset = (offset_t)fragsize * source.addr;
	if (rc = rdevread(dev, &uio, 0, 0))
		goto out;

	/* write the target
	 */
	uio.uio_iov = &iov;
	uio.uio_segflg = SYS_ADSPACE;
	uio.uio_fmode = 0;
	uio.uio_iovcnt = 1;
	uio.uio_resid = 0;
	uio.uio_offset = (offset_t)fragsize * target.addr;
	rc = rdevwrite(dev, &uio, 0, 0);

	out: 
	xmfree(buffer, kernel_heap);
	return rc;
}
