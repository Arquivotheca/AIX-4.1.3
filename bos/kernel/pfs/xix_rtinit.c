static char sccsid[] = "@(#)77	1.13.1.9  src/bos/kernel/pfs/xix_rtinit.c, syspfs, bos41J, 9507C 2/14/95 13:45:14";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_rootinit, logform, itoa
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

#include "jfs/inode.h"
#include "jfs/filsys.h"
#include "jfs/jfslock.h"
#include "jfs/log.h"
#include "sys/vfs.h"
#include "sys/uio.h"
#include "sys/device.h"
#include "sys/malloc.h"
#include "sys/sysinfo.h"

extern dev_t 		rootdev;
extern int		root_vfstype;
extern struct vnode	*rootdir;
extern struct vfs	*rootvfs;
extern struct gfs	*gfsindex[];
extern time_t 		time;
extern struct galloc	gpa_vfs;

static	itoa();
static	logform();

#define	ROUNDUP(x)	(((x) + 3) & ~3)

/*
 *	jfs_rootinit()
 *
 * called at vfsinit() time to initialize root fs after jfs_init() 
 */
jfs_rootinit()
{
	struct vmount *		vmountp;
	struct inode *		ip;		/* root inode ptr */
	struct hinode *		hip;		/* hash list */ 
	struct gnode *		dgp;		/* root inode ptr */
	int			error;
	int			size;		/* size of root vmount */
	int			rc;		/* return code */
	dev_t			logdev;		/* log device number */
	char			device[16];	/* name of rootdev */
	extern int		ram_open();

	/* Try to determine the name of the root device */
	if( devsw[major(rootdev)].d_open == ram_open )
	{
		strcpy(device,"/dev/ram");
		itoa(device+8,minor(rootdev));
	}
	else if( major(rootdev) == 13 )	/* XXX - root volume group */
	{
		strcpy(device,"/dev/hd");
		itoa(device+7,minor(rootdev));
	}
	else
	{
		strcpy(device,"/dev/??");
		itoa(device+7,minor(rootdev));
	}

	/* Initialize root vmount structure */ 
	/* size is size of struct vmount plus four bytes for each data field */

	size = sizeof(struct vmount) + ROUNDUP(strlen(device) + 1) + 5*4;

	/* malloc the vmount structure */
	vmountp = (struct vmount *) malloc(size);
	if (vmountp == NULL)
		panic ("jfs_rootinit: no vmount");

	/* zero out the vmount structure */
	bzero (vmountp, size);
 
	/* transfer info into the vmount structure */
	vmountp->vmt_revision = VMT_REVISION;
	vmountp->vmt_length = size;
	vmountp->vmt_fsid.fsid_dev = rootdev;
	vmountp->vmt_fsid.fsid_type = MNT_JFS;
	vmountp->vmt_vfsnumber = 0;
	vmountp->vmt_time = time;
	vmountp->vmt_flags = VFS_DEVMOUNT;	/* read/write permission */
	vmountp->vmt_gfstype = MNT_JFS;
	vmountdata(vmountp, device, "/", "-", "-", "", "rw");
 
	/* Initialize root vfs structure */

	if ((rootvfs = (struct vfs *) gpai_alloc(&gpa_vfs)) == NULL)
		panic("jfs_rootinit: no vfs");

	/* Handcraft the root vfs */ 
	bzero(rootvfs, sizeof(struct vfs));
	rootvfs->vfs_gfs = gfsindex[root_vfstype];
	rootvfs->vfs_bsize = PAGESIZE;		/* XXX. */
	rootvfs->vfs_mdata = vmountp;
	rootvfs->vfs_mntdover = NULL;
	rootvfs->vfs_count = 1;
	rootvfs->vfs_number = 0;

	/* build ram log device */
	logdev = makedev (major(rootdev), LOGMINOR);

	/* open log and hold until after pmount(). (in
	 * anticipation of the day ram_close will return vm
	 * resources)
	 */
	dgp = NULL;
	if (rc = rdevopen(logdev, 0, 0, NULL, &dgp))
	{	BUGPR(("jfs_rtinit: rdevopen failure=> %d\n", rc));
		panic ("jfs_rtinit: rdevopen failure");
	}

	/* format ram log device */
	logform (logdev, 4);

	/* open root device */
	rootvfs->vfs_data = NULL;
	rc = rdevopen(rootdev, FMOUNT|FREAD|FWRITE, 0, 0, &rootvfs->vfs_data);
	if (rc)
		panic ("jfs_rootinit: rdevopen failure");

	if (rc = pmount (rootdev, 0, minor(logdev)))
		panic ("jfs_rootinit: pmount failure");

	/* allocate and hold the root vnode permanently.
	 */
	IHASH(rootdev, ROOTDIR_I, hip);
	ICACHE_LOCK();
	rc = _iget(rootdev, ROOTDIR_I, hip, &ip, 1, rootvfs);
	ICACHE_UNLOCK();
	sysinfo.iget++;
	cpuinfo[CPUID].iget++;
	if (rc)
		panic("jfs_rootinit: no root inode");
	rootdir = ip->i_gnode.gn_vnode;
	rootdir->v_flag |= V_ROOT;

	rootvfs->vfs_mntd = rootdir;

	/* close log device */
	rdevclose(dgp, 0, 0);

	return 0;
}


static
logform (dev, npages)
dev_t dev;
int npages;
{
	struct uio	uio;
	struct iovec	iovec;
	caddr_t		ptr;
	struct logpage *logp;
	struct logsuper *sup;
	int nbytes, k, rc = 0;
	caddr_t	vaddr;
	extern int ramsrval[];

	/* check to see if log already present.  If so, 
	 * return ok.
	 */
	sup = (struct logsuper *)malloc (sizeof (*sup));
	sup->magic = 0;

	/* setup uio structure */

	iovec.iov_len = sizeof (*sup);
	iovec.iov_base = (caddr_t)sup;

	uio.uio_iov = &iovec;
	uio.uio_iovcnt = 1;
	uio.uio_offset = SUPER_B * PAGESIZE;	/* XXX. FsBSIZE */
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_resid = sizeof (*sup);
	uio.uio_fmode = FREAD;

	/* try and read the log */
	rc = rdevread(dev, &uio, 0, 0);
	if (rc == 0)
	{	/* compare logmagic */
		if (sup->magic == LOGMAGIC || sup->magic == LOGMAGICV4)
		{	assert (sup->redone != 0)
			free (sup);
			return;
		}
	}

	free (sup);	/* free log superblock */

	/* the following disk-blocks are formatted:
 	 *
	 *	page 0 - not changed              
	 *	page 1 - log superblock
	 *	pages 2-N  set to empty log pages
	 */

	nbytes = npages * PAGESIZE;
	ptr = (caddr_t) malloc (nbytes);
	bzero (ptr, nbytes);

	/* init log superblock */

	sup = (struct logsuper *)(ptr + PAGESIZE);
	sup->magic = LOGMAGICV4;
	sup->version = LOGVERSION;
	sup->redone = 1;
	sup->size = npages;
	sup->logend = 2*PAGESIZE + 8;
	
	/* init  disk blocks 2 to npages-1  */
	logp = (struct logpage *)(ptr + (2*PAGESIZE));
	for(k = 2; k < npages; k++)
	{	
		logp->h.xor = logp->t.xor = 0;
		logp->h.eor = logp->t.eor = 8;
		logp->h.page = logp->t.page = k - 2 ;
	}

	/* setup uio structure */
	iovec.iov_len = nbytes;
	iovec.iov_base = ptr;

	uio.uio_iov = &iovec;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_resid = nbytes;
	uio.uio_fmode = FWRITE;

	/* write out the log */
	rc = rdevwrite(dev, &uio, 0, 0);
	if (rc)
	{	BUGPR(("logform: rdevwrite failure=> %d\n", rc));
		panic ("logform: rdevwrite failure");
	}

	vaddr = vm_att(ramsrval[minor(dev)], 0);
	pin(vaddr, nbytes);
	vm_det (vaddr);
}


static
itoa(ptr,i)
char *ptr; 
int i;
{
	char buf[16], *cp;

	cp = buf; *cp++ = '\0';
	
	do
	{
		*cp++ = (char) ((int) '0' + (i % 10));
		i /= 10;
	}
	while( i > 0 );
	
	while( *ptr++ = *--cp )
		;
}
