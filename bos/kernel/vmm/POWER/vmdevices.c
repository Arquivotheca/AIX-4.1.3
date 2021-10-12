static char sccsid[] = "@(#)03	1.53  src/bos/kernel/vmm/POWER/vmdevices.c, sysvmm, bos412, 9445B412 10/26/94 11:29:52";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS: 	vm_mount, vm_mountx, vm_umount, vm_definemap, defineps,
 *		vm_defineps, vm_extendps, freebufs, vm_growmap
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * routines for defining and extending paging devices or file
 * system devices to the VMM.
 */

#include "vmsys.h"
#include "vmpfhdata.h"
#include "mplock.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/malloc.h"
#include <sys/user.h>
#include <sys/intr.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/sysmacros.h>
#include <sys/priv.h>
#include <sys/inline.h>
#include <sys/syspest.h>
#ifdef _VMM_MP_EFF
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _VMM_MP_EFF */

int ps_not_defined = 1;		/* cleared when paging space is defined */

/*
 * vm_mount(type,devid,nbufstr)
 *
 * allocates an entry in the paging device table for the device
 * specified and allocates nbufstr buf  structs for the device. type
 * must not be a paging space device (D_PAGING, use vm_defineps) or
 * file system device (D_FILESYSTEM, use vm_mountx) . only one device
 * of type D_SERVER is supported in the system.
 *
 * input parameters:
 *
 *  type  - type of device. D_LOG, D_REMOTE or D_SERVER.	
 *
 *  devid -  devt of the block device if type is D_FILESYSTEM
 *	     or D_LOG. pointer to strategy routine if type
 *	     is D_REMOTE or D_SERVER.	
 *
 *  nbufstr -  number of buf structs to allocate for the device.
 *
 * return values:
 *	0 - ok
 *
 *	ENOMEM -	no memory for buf structs
 *
 *	EINVAL -	devid already in paging device table.
 *			or in case of D_SERVER, a server is 
 *			already defined.
 */

vm_mount(type,devid,nbufstr)
int     type;  /* type of device */
union {
	dev_t dev;    /* dev_t of block device (D_LOG) */
	int (*ptr)(); /* pointer to strategy routine for D_REMOTE or D_SERVER */
	} devid;
int	nbufstr;
{
	/* type can not be D_PAGING or D_FILESYSTEM.
	 */
	if (type == D_PAGING || type == D_FILESYSTEM)
		return(EINVAL);

	/* vm_mountx() does the work.
	 */
	return(vm_mountx(type,devid,nbufstr,PSIZE,0));
}

/*
 * vm_mountx(type,devid,nbufstr,fragsize,comptype)
 *
 * allocates an entry in the paging device table for the device specified
 * and allocates nbufstr buf structs for the device. type must not be a
 * paging space device (D_PAGING, use vm_defineps for paging space devices).
 * only one device of type D_SERVER is supported in the system.  fragsize
 * must be PSIZE for all device other than file system (D_FILESYSTEM).
 *
 * input parameters:
 *
 *	type  	 - type of device. D_FILESYTEM, D_LOG, D_REMOTE
 *	    	   or D_SERVER.	
 *
 *	devid 	 - devt of the block device if type is D_FILESYSTEM
 *	     	   or D_LOG. pointer to strategy routine if type
 *	     	   is D_REMOTE or D_SERVER.	
 *
 *	nbufstr	 - number of buf structs to allocate for the device.
 *
 *	fragsize - fragment size in bytes.
 *
 * 	comptype - compression algorithm type
 *
 * return values:
 *	0 - ok
 *
 *	ENOMEM -	no memory for buf structs
 *
 *	EINVAL -	devid already in paging device table, or
 *			type is D_SERVER and a server is already defined, or
 *			fragsize not PSIZE and type is not D_FILESYSTEM.
 */

vm_mountx(type,devid,nbufstr,fragsize,comptype)
int     type;  		/* type of device */
union {
	dev_t dev;    	/* dev_t of block device (D_FILESYSTEM or D_LOG) */
	int (*ptr)(); 	/* pointer to strat routine for D_REMOTE or D_SERVER */
	} devid;
int	nbufstr;
int	fragsize;
ushort	comptype;
{
	int rc;
	struct buf * bp, *allocbufs();
	int pdtx, savevmm;

	/* type can not be D_PAGING.
	 */
	if (type == D_PAGING)
		return(EINVAL);

	/* check fragsize and type.
	 */
	if (fragsize != PSIZE && type != D_FILESYSTEM)
		return(EINVAL);

	/* allocate buf structures. nbufstr = 0 is ok.
	 */
	if ((bp = allocbufs(nbufstr)) == NULL && nbufstr > 0)
		return(ENOMEM);

	/* insert device in paging device table
	 */
	if (rc = vcs_pdtinsert(type,devid,bp,fragsize,comptype))
	{
		freebufs(bp);
		return rc;
	}

#ifdef _VMM_MP_EFF
	/*
	 * Initialize FS lock instrumentation.  This can't be done
	 * in the critical section.  We must make VMMDSEG addressible
	 * to do the initialization of the FS lock.
	 */
	pdtx = vcs_devpdtx(type, devid);
	assert(pdtx != -1);

        savevmm = chgsr(VMMSR,vmker.vmmsrval);

	lock_alloc(&lv_lock(pdtx), LOCK_ALLOC_PIN, VMM_LOCK_LV, pdtx);
	simple_lock_init(&lv_lock(pdtx));

        (void)chgsr(VMMSR,savevmm);
#endif	/* _VMM_MP_EFF */
	return 0;
}

/*
 * vm_umount(type,devid)
 *
 * waits for all i/o scheduled for the device to finish
 * and then frees the entry in the paging device table 
 * and associated buf-structs. type can NOT be D_PAGING.
 *
 * input parameters:
 *
 *  type  - type of device. D_FILESYTEM, D_LOG ,D_REMOTE
 *	    or D_SERVER. 	
 *
 *  devid -  devt of the block device if type is D_FILESYSTEM
 *	     or D_LOG. pointer to strategy routine if type
 *	     is D_REMOTE or D_SERVER..	
 *
 * return values:
 *	0 - ok
 *	EINVAL -	devid not in paging device table.
 */

vm_umount(type,devid)
int     type;  /* type of device */
union {
	dev_t dev;      /* dev_t of block device (D_FILESYSTEM or D_LOG) */
	int (*ptr)();   /* pointer to strategy routine if type is D_REMOTE */
	} devid;
{
	int rc;
	struct buf * bp;
	extern dev_t rootdev;

	/* type can not be D_PAGING
	 */
	if (type == D_PAGING)
		return(EINVAL);

	/* delete device in paging device table after i/o
	 * is done and set bp to point to first buf-struct.
	 */
	if (rc = vcs_pdtdelete(type,devid,&bp))
		return rc;

	/* free the buf-structs and return.
	 */
	freebufs(bp);

	/* first rootdev is v3 ram file system.  when it is umounted
	 * free vmm resources
	 */
	if (type == D_FILESYSTEM && devid.dev == rootdev && vmker.ramdsrval)
	{	vms_delete (SRTOSID(vmker.ramdsrval));
		vmker.ramdsrval = 0;
	}
	return 0;

}
/*
 * vm_definemap(devid,mapsid)
 *
 * defines mapsid as the segment containing the disk 
 * allocation map for a previously mounted device.
 *
 * input parameters:
 *
 *  devid -  devt of the block device.
 *
 *  mapsid - segment id of segment containing disk map.
 *
 * return values:
 *	0 - ok
 *	EINVAL -	devid not in paging device table
 */

vm_definemap(devid,mapsid)
int	devid;
int	mapsid;
{
	int pdtx,srsave;

	/* get pdtx of device
	 */
	if((pdtx = vcs_devpdtx(D_FILESYSTEM,devid)) < 0)
		return(EINVAL);

	/* make vmmdseg addressable and fill in pdt entry
	 */
	srsave = chgsr(VMMSR,vmker.vmmsrval);
	pdt_dmsrval(pdtx) = SRVAL(mapsid,0,0);
	(void)chgsr(VMMSR,srsave);

	return(0);
}

/*
 * defineps(devid,nblocks,nbufstr)
 *
 * defines device as a paging device to the VMM.
 * if a new device, calls vm_defineps to define
 * the new paging device or if an existing device,
 * calls vm_extendps to extend it.
 *
 * input parameters:
 *
 *  devid -  devt of the block device.
 *
 *  nblocks - total size of device in PAGESIZE blocks.
 *
 *  nbufstr - total number of buf structs to allocate for the device.
 *
 * return values:
 *      0 - ok
 *      EINVAL -        nblocks or nbufstr invalid.
 *      ENOMEM -        no memory for buf structs or PDT is full
 *			or no memory for disk map.
 *	others -	return values from rdevopen.
 */

defineps(devid,nblocks,nbufstr)
dev_t   devid;			/* device id of device to define	*/
int     nblocks;		/* total size in PAGESIZE blks		*/
int     nbufstr;		/* total # of buf structs for dev	*/
{
        int rc,pdtx,srvmsave,srtemp,dummy,cnt;
	struct buf *bp, *bufp;

	/* do some parameter checking.
	 */
	if (nblocks > MAXMAPSIZE || nblocks < 0)
		return(EINVAL);

        /* make vmmdseg and paging space disk maps addressable.
         */
        srvmsave = chgsr(VMMSR,vmker.vmmsrval);
        srtemp   = chgsr(TEMPSR,vmker.dmapsrval);

        /* determine if device already exists as paging device.
         */
        if ((pdtx = vcs_devpdtx(D_PAGING,devid)) < 0)
        {
                /* new paging device
                 * the first paging device is handled as a special case.
                 */
                if (pf_npgspaces < 1)
                {
                        /* first paging device -- handle this as an
                         * extension of the paging space installed by vmsi.
                         */
                        /* open the device.
                         * we pass rdevopen a dummy parameter instead of a
                         * pointer to a field in an inode table entry.
                         * this is ok as long as we don't try to close the
                         * paging device.  The value of dummy must be zero
			 * so that rdevopen will allocate resources.
                         */
			dummy = 0;
                        if (rc = rdevopen(devid,FWRITE | FMOUNT,0,NULL,&dummy))
                                goto closeout;

                        /* extend the paging device installed by vmsi. we
                         * assume that the pdt index for this is 0.
                         */
                        if (rc = vm_extendps(0,nblocks))
                                goto closeout;

			/* allocate buf structs, none were allocated
			 * during vmsi.c
	 		 */
			if ((bp = allocbufs(nbufstr)) == NULL && nbufstr > 0)
			{
				rc = ENOMEM;
                                goto closeout;
			}

			/* count and record number of buf structs
		 	* and chain them
		 	*/
			pdt_bufstr(0) = bp;
			for (cnt=0, bufp=bp; bufp!=NULL; cnt++, bufp=bufp->av_forw)
				;
			pdt_nbufs(0) = cnt;

                        /* update the pdt entry and indicate that we
                       	 * now have a paging device.
                         */
                        pdt_device(0) = devid;
                        pf_npgspaces += 1;

			/* inform kernel components that phase 2 ipl
			 * has begun
			 */
			ps_not_defined = 0;
			(void)phasetwoinit();
                }
                else
                {
                        /* new paging device
                         */
                        rc = vm_defineps(devid,nblocks,nbufstr);
                }
        }
        else
        {
                /* existing paging device
                 */
                rc = vm_extendps(pdtx,nblocks);
        }

closeout:
        (void)chgsr(VMMSR,srvmsave);
        (void)chgsr(TEMPSR,srtemp);
        return rc;
}

/*
 * vm_defineps(devid,nblocks,nbufstr)
 *
 * defines a new paging device to the VMM.
 * allocates the number of bufstructs specified for
 * the device specified and initializes the disk
 * allocation map for a size of nblocks.
 * on entry the vmmdseg and paging space disk maps
 * segments must be addressable.
 *
 * input parameters:
 *
 *  devid -  devt of the block device.
 *
 *  nblocks - total size of device in PAGESIZE blocks.
 *
 *  nbufstr - total number of buf structs allocated for the device.
 *
 * return values:
 *	0	- ok
 *	ENOMEM	- no memory for buf structs or PDT is full.
 *	others	- return values from rdevopen.
 */

vm_defineps(devid,nblocks,nbufstr)
dev_t	devid;
int	nblocks;
int	nbufstr;
{
	int rc,dummy,pdtx;
	struct buf *allocbufs(), *bp = NULL;

	/* allocate buf structs.
	 * the first paging device, created at vmsi time, comes here with
	 * nbufstr == 0.  at vmsi time, kernel_heap and its TOC are not
	 * initialized yet, so you can't call allocbufs.  the compiler
	 * optimizes aggressively and a kernel_heap dereference will fail.
	 */
	if (nbufstr > 0)
		if ((bp = allocbufs(nbufstr)) == NULL)
			return(ENOMEM);

	/* insert device in paging device table.
	 */
	if (rc = vcs_pdtinsert(D_PAGING,devid,bp,PSIZE,0))
	{
		freebufs(bp);
		return(rc);
	}

	pdtx = vcs_devpdtx(D_PAGING,devid);
	assert(pdtx != -1);

	/* open the device. a devid of -1 is used by vmsi before
	 * devices are actually available.
	 */
	if (devid != -1)
	{
		/* we pass rdevopen a dummy parameter instead of a
		 * pointer to a field in an inode table entry.
		 * this is ok as long as we don't try to close the
		 * paging device.  The value of dummy must be zero
		 * so that rdevopen will allocate resources.
		 */
		dummy = 0;
		if (rc = rdevopen(devid, FWRITE | FMOUNT,0,NULL,&dummy))
		{
			vcs_pdtdelete(D_PAGING,devid,&bp);
			freebufs(bp);
			return(rc);
		}

#ifdef _VMM_MP_EFF
		/*
		 * Initialize PG lock instrumentation.  This can't be done
		 * in the critical section.  The lock for the initial
		 * paging device is handled in init_vmmlocks() rather than
		 * here because instrumentation isn't available yet here
		 * during vmsi.
		 */
		lock_alloc(&lv_lock(pdtx), LOCK_ALLOC_PIN, VMM_LOCK_LV, pdtx);
		simple_lock_init(&lv_lock(pdtx));
#endif	/* _VMM_MP_EFF */
	}

	/* initialize map.
     	 */
	initmap(pdtx,nblocks);
	
	/* mark new paging space as available for use
	 * (device is open and map is initialized).
	 */
	pdt_avail(pdtx) = 1;

	/* update stats for calls other than one from vmsi.
	 */	
	if (devid != -1)
		pf_npgspaces += 1;

	return(0);
}

/*
 * vm_extendps(pdtx,nblocks)
 *
 * extends an existing paging device and grows
 * the disk map as necessary.
 * this routine currently does not allow reducing the
 * number of blocks or bufstructs.
 * on entry the vmmdseg and paging space disk maps
 * segments must be addressable.
 * this process never allocates more buf structs for the page space.
 *
 * input parameters:
 *
 *  pdtx - paging device table index of device to extend.
 *
 *  nblocks - total size of device in PAGESIZE blocks.
 *
 * return values:
 *	0	- ok
 *	ENOMEM	- no memory for buf structs or for disk map.
 *	EINVAL	- nbufstr or nblocks invalid.
 */

vm_extendps(pdtx,nblocks)
int	pdtx;
int	nblocks;
{
	int rc;
	struct vmdmap *p0;

	/* extend the disk map.
	 */
	p0 = (TEMPSR << L2SSIZE) + pdtx*DMAPSIZE;
	rc = vm_growmapx(p0,nblocks,PGDISKMAP,0,pdtx);

	return(rc);
}

/*
 * allocbufs(nbufs)
 *
 * Allocates nbufs buf structures, pins them and releases the backing
 * storage, and returns a pointer to first.  The buf structs are pinned
 * and are chained together thru av_forw with the last pointer null.
 *
 * Return value
 *		bp - pointer to first bufstr
 *		     NULL if insufficient memory or nbufs < 0.
 */

#define	SZ_BUF	(sizeof(struct buf))

static struct buf *
allocbufs(nbufs)
int nbufs;
{
	int		 nbytes, rc;
	struct buf	*bp, *first, *next;

	if ( nbufs <= 0 )
		return( NULL );

	/* Round requested allocation up to the next whole page worth
	 * of buffers.  
	 * number of bufs must remain as requested (in particular for
	 * log file systems)
	 */
	nbytes = ( ( nbufs * SZ_BUF ) + ( PAGESIZE - 1 ) ) & (~(PAGESIZE-1));

	if (( bp = xmalloc( nbytes, PGSHIFT, kernel_heap)) != NULL )
	{
		/* Free the backing storage
		 */
		if ((rc = ltpin( bp, nbytes )) != 0 )
		{
			(void)xmfree( (void *)bp, kernel_heap );
			return( NULL );
		}

		bzero( bp, nbytes );

		/* Chain together the buf structs
		 */
		first = bp;
		for(; nbufs - 1; bp++, nbufs--)
		{
			bp->av_forw = bp + 1;
		}
		bp->av_forw = NULL;
	}

	return( first );
}

/*
 * freebufs(bp)
 *
 * Frees a list of buf structs from pageable heap.  On entry bp points to
 * the first in the list of buf structs which are chained thru av_forw.
 *
 * Since the buf structs were allocated with a single call, search the
 * passed chain for the lowest addressed buf, which is the only one that
 * needs to be freed.  The passed buf area is first long-term unpinned,
 * which reallocates backing storage for the page(s).
 */

freebufs(bp)
struct buf * bp;
{
	unsigned long	 smallest;
	int		 nbufs, nbytes, rc;

	smallest = 0xFFFFFFFF;
	for ( nbufs = 0; bp != NULL; bp = bp->av_forw )
	{
		nbufs += 1;
		if ( (unsigned long)bp < smallest )
			smallest = (unsigned long)bp;
	}

	nbytes = ( ( nbufs * SZ_BUF ) + ( PAGESIZE - 1 ) ) & ~(PAGESIZE - 1);

	if ( smallest != 0xFFFFFFFF )
	{
		rc = ltunpin( (void *)smallest, nbytes );
		assert( rc == 0 );
		xmfree( (void *)smallest, kernel_heap );
	}

	return 0;
}

/*
 * initmap(pdtx,nblocks)
 *
 * initializes the allocation map for a paging space disk
 * map corresponding to paging device table index pdtx and
 * of size nblocks.
 *
 * this proceduere is only called from vmdefineps() above.
 * on entry the vmmdseg and the paging spage disk maps
 * segments are addressable.
 */

static
initmap(pdtx,nblocks)
int pdtx;  /* index in pdt */
int nblocks; /* number of blocks */
{
	int rc;
	struct vmdmap * p0;

	/* zero and pin the first page of the map
	 */
	p0 = (TEMPSR << L2SSIZE) + pdtx*DMAPSIZE;
	bzero(p0, PAGESIZE);
	if (rc = pin(p0,1))
		return rc;

	/* initialize page 0 of map.
	 * setting mapsize to one makes it look like block 0
	 * is allocated. note that cluster-size is 1.
	 */
	p0->mapsize = 1;
	p0->maptype = PGDISKMAP;
	p0->agcnt = 1;
	p0->totalags = 1;
	p0->version = ALLOCMAPV3;
	imappage(p0,0, AGDEFAULT, 1);

	/* grow the map to size nblocks.
	 */
	vmker.numpsblks += 1;
	return(vm_growmapx(p0,nblocks,PGDISKMAP,0,pdtx));

}

/* 
 * imappage(p0,p,nblocks,agsize,clsize)
 *
 * initialize one page of a disk or inode map.
 *
 * for version V3 maps, page is assumed to be zeroed on entry
 * except for p = 0 (see initmap). for version V3, p is the 
 * page number in the map considered as a file, but for version V4
 * it is the page number computed as if there were no control
 * information pages.
 *
 * input parameters
 *	p0 - pointer to page zero of map
 *	p  - page number in map to init	
 *	agsize	- allocation group size.
 *	clsize  - cluster size - longest sequence to allocate.
 *	
 */

imappage(p0,p,agsize, clsize)
struct vmdmap *p0;
uint p;
uint agsize;
uint clsize;
{

	struct vmdmap * p1;
	uint k;
	uint version, *wmap, *pmap, wperpage;

	/* set allocation group size and cluster size in the
	 * map page.
	 */
	version = p0->version;
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + p;
		wmap = (uint *)(p1) + LMAPCTL/4;
		wperpage = WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *) (p1 + 1 + (p & 0x7));
		p1 = VMDMAPPTR(p1, p);
		wperpage = WPERPAGEV4;
	}
	p1->agsize = agsize;
	p1->clsize = clsize;

	/* set clmask in the map page if this is a V3 allocation
	 * map.  clmask is encoding of clsize in the format of
	 * dmptab[].
	 */
	if (p0->version == ALLOCMAPV3)
		p1->clmask = (ONES << (8 - clsize)) & 0xff;
	
	/* set version from page 0 of the map.
	 */
	p1->version = p0->version;

	/* init words in maps to allocated.
	 */
	pmap = wmap + wperpage;
	for (k = 0; k < wperpage; k ++)
	{
		wmap[k] = pmap[k] = ONES;
	}

	return 0;
}

/*
 * vm_growmap(p0,newsize,type,minsize)
 *
 * pass thru to vm_growmapx
 */
int
vm_growmap(p0,newsize,maptype,minsize)
struct vmdmap * p0;  /* pointer to first page of map */
int newsize;  	     /* new size in blocks */
int  maptype;        /* map type */
int  minsize;	     /* min num of resources required for an ag */
{
	int srsave, pdtx;

	/*
	 * Compute pdtx from pointer to map.
	 */
	srsave = chgsr(VMMSR, vmker.vmmsrval);
	pdtx = scb_devid(STOI(SRTOSID(mfsri(p0))));
	(void) chgsr(VMMSR, srsave);
	return(vm_growmapx(p0,newsize,maptype,minsize,pdtx));
}

/*
 * vm_growmapx(p0,newsize,type,minsize,pdtx)
 *
 * common routine for extending a disk map or an inode map.
 *
 * if the map is a file system disk map or an inode map
 * the value of the mapsize field is NOT changed so that
 * space is not allocated prematurely (see xix_cntl.c for
 * extend file system code).
 *
 * for paging space disk maps the mapsize field is set and
 * the free paging space in the system kept in vmker.psfreeblks
 * is updated.
 *
 * INPUT PARAMETERS:
 *
 * (1) p0 	- pointer to first page of map
 *
 * (2) newsize 	- the newsize in units of fragments (FSDISKMAP or
 *		  PGDISKMAP) or inodes (INODEMAP) covered by the map.
 *		  must be at least as big as the current mapsize.
 *         
 * (3) type 	- map type.
 *         
 * (4) minsize 	- minimum number of fragment required for an ag.  valid
 *		  for type FSDISKMAP only.
 *
 * (5) pdtx	- device index
 *
 * Return values
 *
 *	0	- ok
 *
 *	EINVAL	- newsize is less than current size or bigger
 *                than MAXMAPSIZE.
 *
 *	ENOMEM	- no memory for addition to map.
 */

int
vm_growmapx(p0,newsize,maptype,minsize,pdtx)
struct vmdmap * p0;  /* pointer to first page of map */
int newsize;  	     /* new size in blocks */
int  maptype;        /* map type */
int  minsize;	     /* min num of resources required for an ag */
int  pdtx;
{
	struct vmdmap *p1;
	int nb, nextblk, nag, agsize, nleft;
	int lastp, blkshere, newlastp, k, rc;
	uint version, np, dbperpage;

	/* check for too small or too big a newsize.
	 */
	if(p0->mapsize > newsize || newsize > MAXMAPSIZE)
	{
		return(EINVAL);
	}

	/* initialize any additional pages required for map
	 * they are initialized with zero free blocks.
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	lastp = (p0->mapsize - 1)/dbperpage;
	newlastp = (newsize - 1)/dbperpage;
	for (k = lastp + 1; k <= newlastp; k++)
	{
		/* np is the page number in the "file"
		 */

		if (version == ALLOCMAPV3)
		{
			np = k;
		}
		else
		{
			/* make a control page ?
			 */
			if ((k & 0x7) == 0)
			{
				bzero(p0 + 9*(k >> 3), PSIZE);
			}
			np = 1 + 9*(k >> 3) + (k & 0x7);
		}
		bzero(p0 + np, PSIZE);
		imappage(p0,k,p0->agsize,p0->clsize);
		if (maptype == PGDISKMAP)
		{
			if (rc = pin(p0 + k, 1))
				return rc;
		}
	}
		
	/* remaining blocks and first of them
	 */
	nleft = newsize - p0->mapsize;
	nextblk = p0->mapsize;

	/* map size is only set for paging space.
	 * for other types mapsize is set by caller.
	 * this is necessary because extension 
	 * of file-system requires co-ordinated 
	 * growth of inode map and disk map and the
	 * inodes as well. leaving size as is prevents
	 * others from allocating space too soon.
	 */
	if (maptype == PGDISKMAP)
	{
		FETCH_AND_ADD(vmker.psfreeblks, newsize - p0->mapsize);
		vmker.numpsblks += newsize - p0->mapsize;
		/* increase number of kernel_reserved paging space disk blocks
		 * to about 1/2% of total paging space.
		 */
		vmker.psrsvdblks = MAX(vmker.psrsvdblks, (vmker.numpsblks >> 8));
		pf_npskill = MAX(pf_npskill, vmker.psrsvdblks << 1);
		pf_npswarn = MAX(pf_npswarn, pf_npskill << 2);
		p0->mapsize = newsize;
	}

	/* first see if last allocation group was
	 * incomplete. if so complete as much of it as
	 * possible.
	 */
	if (version == ALLOCMAPV3)
	{
		p1 = p0 + lastp;
	}
	else
	{
		p1 = p0 + 9*(lastp >> 3);
		p1 = VMDMAPPTR(p1, lastp);
	}
	nag = p1->agcnt;
	agsize = p1->agsize;
	blkshere = nextblk - lastp*dbperpage;

	/* nb is number of blocks to complete the ag.
	 */
	if (nb = nag*agsize - blkshere)
	{
		/* nb can be less than zero in the case when
		 * there were insufficient blocks previously
		 * for inode blocks. in this case we get to use 
		 * these blocks now ( they were previously
		 * marked as allocated in the map).
		 */
		if (nb < 0)
		{
			assert(maptype == FSDISKMAP);

			nleft += -nb;
			nextblk += nb;
			nb = MIN(agsize,nleft);

			/* can we cover the inode blocks for filesystem
			 * disk maps ?
			 */
			if (nb < minsize)
				return 0;

			/* adjust mapsize so that allocates do not
			 * happen prematurely.
			 */
			p0->mapsize += nag*agsize - blkshere;
		}
		else
		{
			nb = MIN(nb,nleft);
		}

		/* ok to extend it
		 */
		vcs_extendag(p0,nb,nextblk,pdtx);
		nleft -=  nb;
		nextblk += nb;
	}

	/* last allocation group is full or nleft is zero
	 * for filesystem disk space maps we don't start a new
	 * allocation group unless there are enough blocks
	 * to cover the inodes.
	 */
	while(nleft > 0)
	{
		if (maptype == FSDISKMAP && nleft < minsize)
			break;
		nb = MIN(agsize,nleft);
		vcs_extendag(p0,nb,nextblk,pdtx);
		nleft -= nb;
		nextblk += nb;
	}

	return 0;
}
