static char sccsid[] = "@(#)97	1.59.1.20  src/bos/kernel/pfs/disubs.c, syspfs, bos41J, 9516A_all 4/17/95 13:17:16";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: iptovaddr, ibindseg, iallocind, iread, inofree,
 *            ialloc, allociblk, freeiblk, iclose, iwrite,
 *            iflush, ifreeind, isetsize, ismodified, isreadonly
 *
 * ORIGINS: 3, 26, 27, 83
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
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include  "jfs/fsdefs.h" 
#include  "jfs/jfslock.h" 
#include  "jfs/inode.h"
#include  "jfs/commit.h"
#include  "sys/errno.h"
#include  "sys/syspest.h"
#include  "vmm/vmsys.h"

BUGVDEF(isyncdatadbg, 0);

/*
 * NAME:	iptovaddr (ip, journ, vaddr)
 *
 * FUNCTION:	This is a attempt to give some of the file system 
 *		code a flavor of portability.  This subroutine 
 *		could in theory be replaced with whatever code is required
 *		to map an inode to a virtual address.
 *
 * PARAMETERS:	ip	- inode to be mapped into virtual memory
 *		journ	- Non-zero if journalled
 *		vaddr	- returned virtual address
 *
 * SERIALIZATION: This routine can be called with any locks held for meta-data
 *		  inodes, since their segments are bound at mount time and are
 *		  not changed, and since threads have their own address space, 
 *		  the vm_att() is serialized as well.
 *
 * RETURN :	0	- success
 *		errors from ibindseg()
 */

#include "sys/adspace.h"

iptovaddr (ip, journ, vaddr)
struct 	inode *ip;		/* Inode to map into virtual memory */
int	journ;			/* Non-zero if journalled	*/
caddr_t *vaddr;			/* Returned virtual address for ip */
{
	int rc = 0;		/* Return code		*/
	
	/* Bind this inode to a virtual memory address
	 * and load free segment register with seg id in i_seg.
	 */
	if (ip->i_seg || (rc = ibindseg (ip)) == 0)
	{	
		/* this may not be the best thing to use here. vm_att() 
		 * may fail(panic). in general there should be enough scratch
		 * segment registers to go around, but given the level of
		 * nesting allowed in version 3 we may run out. if
		 * so, this code could use more fundamental vm_geth() and 
		 * vm_seth() to save and restore a particular sr.
		 */
		*vaddr = vm_att (SRVAL(ip->i_seg, 0, journ), 0);
	}
	return rc;
}


/*
 * NAME:	ipundo (vaddr)
 *
 * FUNCTION:	This is a attempt to give some of the file system 
 *		code a flavor of portability.  This subroutine 
 *		could in theory be replaced with whatever code is required
 *		to unmap an inode from a virtual address.
 *
 * PARAMETERS:	vaddr	- free vaddr
 *
 * RETURN :	0	- success
 */

void
ipundo (vaddr)
caddr_t	vaddr;
{
	(void) vm_det (vaddr);
}


/*
 * NAME:	ibindseg (ip)
 *
 * FUNCTION:	Creates a virtual memory segment and associates it
 *		with the persistent segment specified by ip. on entry
 *		the inode is locked by caller and the disk inode part  
 *		of the inode must be valid. if the sid field in the 
 *		inode is not zero the VM segment is assumed already to 
 *		exist.
 *		
 *		Returns 0. ok. ENOMEM if there is insufficient space
 *		to allocate the virtual memory data structures. EIO if
 *		there is a permanent i/o error.
 *		
 *
 * PARAMETERS:	ip	- pointer to inode to bind to VM
 *
 * RETURN :	errors from subroutines
 *
 * SERIALIZATION: caller must be holding the inode read/write lock 
 *		  upon entry/exit
 *
 */

ibindseg (ip)
struct inode *ip;
{
	int sr12save,savevmsr,rc;
	label_t jbuf;           
	int sid, sidx, device, type, size, lastp;
	struct inode *ipi, *iplog, *ipmnt;

	/* already bound to a VM segment ?
	 */
	if (ip->i_seg)
		return 0;

	INODE_LOCK(ip);
	if (ip->i_seg)
	{
		INODE_UNLOCK(ip);
		return 0;
	}

	/* save sregs and make vmmdseg addressable
	 */
	sr12save = mfsr(12);
	savevmsr = chgsr(VMMSR,vmker.vmmsrval);

	/* set up for i/o exception.
	 */
	if (rc = setjmpx(&jbuf))
		goto closeout;

	/* create vm segment. 
	 */
	type = (ip->i_mode & IFJOURNAL) ? V_PERSISTENT | V_JOURNAL 
	       : (ip->i_flag & IDEFER) ? V_PERSISTENT | V_DEFER : V_PERSISTENT;
	device = ip->i_dev;
	size = ip->i_size;
	if (rc = vms_create(&sid,type,device,size,0,0))
		goto closeout;

	/* record info in inode and in scb
	 */
	ipmnt = ip->i_ipmnt;
	ip->i_seg = sid;
	sidx = STOI(sid);
	scb_gnptr(sidx) = &ip->i_gnode;
	scb_agsize(sidx) = ipmnt->i_agsize;
	scb_iagsize(sidx) = ipmnt->i_iagsize;

	/* if its a special system segment set the system bit in
	 * the scb.
	 */
	if (ip->i_number <= SPECIAL_I && ip->i_number != ROOTDIR_I)
		scb_system(sidx) = 1;
		
	/* if its journalled, find out log sid and put into
	 * scb. 
	 */
	if (type & V_JOURNAL)
	{
		iplog = ipmnt->i_iplog;
		scb_logsidx(sidx) = (iplog != NULL) ? STOI(iplog->i_seg) : 0;
	}

	/* if file system supports compression, compression applies
	 * for non-journalled non-special files.
	 */
	if (ipmnt->i_fscompress)
	{
		if (!(type & V_JOURNAL) && (ip->i_number > SPECIAL_I))
		{
			scb_compress(sidx) = 1;
			ip->i_compress = ipmnt->i_fscompress;
		}
	}

	/* if the segment is .inodes it doesn't have indirect 
	 * blocks.
	 */
	if (ip->i_number == INODES_I)
	{
		scb_inoseg(sidx) = 1;
		goto closeout;
	}

	/* identify indirect segment to VMM.
	 */
	if (ip->i_number == INDIR_I)
	{
		scb_indseg(sidx) = 1;
	}

	/* identify disk map segment to VMM.
	 */
	if (ip->i_number == DISKMAP_I)
	{
		scb_dmapseg(sidx) = 1;
	}

	/* map the segments indirect blocks into .indirect if
	 * necessary.
	 */
	rc = iallocind(ip);

	/* restore state and return
	 */
	clrjmpx(&jbuf);

closeout:
	INODE_UNLOCK(ip);
	(void)chgsr(12,sr12save);
	(void)chgsr(VMMSR,savevmsr);
	return(rc);
}


/*
 *	isegdel(ip)
 *
 * deletes inode's segment and clears segment id in the inode.
 */

int
isegdel(ip)
struct inode *ip; 
{
	int sid;

	if (ip->i_seg == 0)
		return(0);

	/* free any movedfrag structures.
	 */
	vcs_freefrags(ip);

	sid = ip->i_seg;
	ip->i_seg = 0;
	return(vms_delete(sid));
}


/*
 * iallocind(ip)
 *
 * allocate pages in .indirect to represent the indirect blocks
 * of the segment whose inode table entry is pointed to by ip.
 *
 * on entry the inode table entry is assumed to be locked by caller.
 *
 * Return values
 *		0 - ok
 *		ENOSPC - .indirect is full.
 */

int
iallocind(ip)
struct inode *ip;
{
	int  p, rc, k, maxp, sr12save, iblk, daddr;
	struct inode *ipi;
	struct idblock *idptr;

	/* nothing to do if no indirect blocks.
	 */
	if (NOIND(ip->i_size))
		return 0;

	/* allocate a page in .indirect for the indirect block
	 */
	daddr = ip->i_rindirect;
	assert(daddr != 0);
	if(rc = allociblk(ip, &ip->i_vindirect, daddr))  
		return rc;	

	/* no more to do unless it has double indirect block
	 */
	if (!DBLIND(ip->i_size))
		return 0;

	/* map .indirect into virtual memory without special bit.
	 */
	ipi = ip->i_ipmnt->i_ipind;
	sr12save = chgsr(12,SRVAL(ipi->i_seg,0,0));

	/* point to the double indirect block
	 */
	k = SR12ADDR + (PAGESIZE * ip->i_vindirect);
	idptr = (struct idblock *)k;
	maxp = BTOPN(ip->i_size);
	for (k = 0; k <= maxp; k += PAGESIZE/4, idptr++)
	{
		if ((daddr = idptr->id_raddr) == NULL)
		{
			idptr->id_vaddr = NULL;
			continue;
		}
		if (rc = allociblk(ip, &idptr->id_vaddr, daddr)) 
			break;	
	}

	/* restore sreg 12 and return
	 */
	(void)chgsr(12,sr12save);
	return rc;
}


/*
 * NAME:	ifreeind (ip)
 *
 * FUNCTION:	Frees the pages in .indirect corresponding to
 *		indirect blocks of ip. 
 *		
 * PARAMETERS:	ip	- pointer to inode whose indirect blocks we're freeing
 *
 * RETURN :	0
 *		EIO  - permanent i/o error.
 *			
 */

ifreeind (ip)
struct inode *ip;  
{
	struct inode *ipind;
	struct idblock *idptr;
	int sr12save, maxp, k, rc, keep;
	label_t jbuf;

	/* save sr12. modified below or by freeiblk().
	 */
	sr12save = mfsr(12);

	/* set up for i/o error
 	 */
	if (rc = setjmpx(&jbuf))
	{
		(void)chgsr(12,sr12save);
		return rc;
	}
	
	/* is it a double indirect geometry?
	 */
	if (!SGLIND(ip->i_size))
	{
		/* map .indirect into VM.
		 */
		ipind = ip->i_ipmnt->i_ipind;
		(void)chgsr(12,SRVAL(ipind->i_seg,0,0));

		maxp = BTOPN(ip->i_size);
		idptr = (struct idblock *)(SR12ADDR +
				ip->i_vindirect * PAGESIZE);

		/* free double indirect blocks.
		 */
		for (k = 0; k <= maxp; k += PAGESIZE/4, idptr++)
		{
			if (idptr->id_raddr == NULL)
				continue;
			freeiblk(ip, idptr->id_vaddr);
		}
	}

	/* free the root indirect block (double or single)
	 */
	freeiblk(ip, ip->i_vindirect);
	ip->i_vindirect = 0;

	(void)chgsr(12,sr12save);
	clrjmpx(&jbuf);
	return 0;
}


/*
 * NAME:	allociblk (ip,indblk,daddr)
 *
 * FUNCTION:	Allocates an indirect block for the file specified
 *		sets *indblk to its page number in .indirect. the
 *		allocated page is mapped to daddr. 
 *
 * PARAMETERS:	ip	- pointer to inode of file
 *		indblk	- set to page in .indirect that was allocated
 *		daddr	- disk address to which page is mapped.
 *
 * RETURN :	
 *		0 - ok
 *		ENOSPC - out of space in .indirect
 *			
 */

static
allociblk (ip, indblk, daddr)
struct inode *ip;
int *indblk;	
int  daddr;
{ 
	struct inode * ipind;
	struct indir *ptr;
	int  sr12save, p, rc;

	/* allocate an indirect block.
	 * get the inode table entry for .indirect
	 */
	ipind = ip->i_ipmnt->i_ipind;
	if (rc = vcs_allociblk(ipind->i_seg, &p))
		return rc;

	/* 
	 * map it into virtual memory without special bit 
	 */
	sr12save = chgsr(12, SRVAL(ipind->i_seg,0,0));
	ptr = (struct indir *)SR12ADDR;

	/* set pointer in .indirect of itself to daddr.
	 * the "NEWBIT" is turned on to make it possible
	 * to check for freeing the same indirect block
	 * twice in freeiblk(). NEWBIT does not effect
	 * behavior otherwise.
	 */
	ptr->indptr[p] = daddr | NEWBIT;
	*indblk = p;			

	(void)chgsr(12,sr12save);
	return 0;
}


/*
 * NAME:	freeiblk (ip, indblk)
 *
 * FUNCTION:	Frees the indirect block specified. the page
 *		is discarded from memory and put on .indirects
 *		free-list.
 *
 * PARAMETERS:	ip	- inode pointer for a file
 *		indblk	- page number in .indirect
 *
 * RETURN :	
 *		0  - success
 *			
 */

freeiblk (ip, indblk)
struct inode * ip;
int indblk;	
{
	struct indir *ptr;
	struct inode *ipind;
	int  sr12save;

	/* get the inode table entry for .indirect
	 */
	ipind = ip->i_ipmnt->i_ipind;

	/* map into virtual memory without special bit
	 */
	sr12save = chgsr(12,SRVAL(ipind->i_seg,0,0));
	ptr = (struct indir *)SR12ADDR;

	/* page release the page. free lock word.
	 * assertion says that the current contents
	 * of indptr[indblk] is a disk block address.
	 */
	assert (NEWBIT & ptr->indptr[indblk]);

	vcs_freeiblk(ipind->i_seg, indblk);

	/* restore sreg 12 
	 */
	(void)chgsr(12,sr12save);
	return 0;
}


/*
 * NAME:	ialloc (pip, ino)
 *
 * FUNCTION:	Allocates an inode in the device specified and sets
 *		ino to its number. on entry the value of ino may be 
 *		given as a hint: if non-zero, an attempt is made to 
 *		allocate an inode which is "near by". returns 0 if ok  
 *		and ENOSPC if out of inodes.
 *		
 *		When the .inode segment must be extended in order
 *		to satisfy the request, it is extended by 32 inodes
 *		and the extension to the segment is committed here.
 *
 * PARAMETERS:	pip	- parent inode
 *		ino	- on entry a hint. if > 0 the inode
 *			  is allocated in the same allocation
 *			  group, if possible. if  0 , in the
 *			  first allocation group with fewer
 *			  than average number of disk blocks
 *			  allocated.
 *			
 *		ino     - on exit set to the inumber allocated.
 *
 * RETURN :	errors from subroutines
 *		ENOSPC	- no more inodes.
 *			
 */

ialloc (pip, ino)
struct inode	*pip;
ino_t   *ino;		
{
	struct inode *ipm, *ipdmap, *ipmnt;
	int rc, sr13save, word, hint;
	struct vmdmap * ptr;
	dev_t   dev;			
	label_t jbuf;           

	/* find the inode table entry for .inodemap and lock inode.
	 */
	ipm = pip->i_ipmnt->i_ipinomap;

	IWRITE_LOCK(ipm);		

	/* save sreg 13.
	 * set up for i/o exception.
	 */
	sr13save = mfsr(13);
	if(rc = setjmpx(&jbuf))
		goto out;

	/* if hint is 0, find an allocation group with greater
	 * than average disk free space. the value returned 
	 * by v_nextag() is the first block number in the ag
	 * and must be converted to the first inode number.
	 */
	if ((hint = *ino) == 0)
	{
		ipmnt = ipm->i_ipmnt;
		ipdmap = ipmnt->i_ipdmap;
		(void)chgsr(13,SRVAL(ipdmap->i_seg,0,0));
		ptr = (struct vmdmap *)SR13ADDR;
		hint = v_nextag(ptr, 0);
		hint = (hint / ipmnt->i_agsize) * ipmnt->i_iagsize;
	}

	/* allocate an inode.
	 */
	(void)chgsr(13,SRVAL(ipm->i_seg,0,0));
	ptr = (struct vmdmap *)SR13ADDR;
	rc = v_alloc(ptr,1,hint,&word);
	if (rc == 0)
		*ino = word;

	/* restore state and return
	 */
	clrjmpx(&jbuf);

out:
	IWRITE_UNLOCK(ipm);		
	(void)chgsr(13,sr13save);
	return rc;
}


/*
 * NAME:	inofree (ip, ino)
 *
 * FUNCTION:	Puts disk inode associated with ip on free list
 *		in work map.
 *
 * PARAMETERS:	ip	- inode to free
 *		ino	- inode number
 *
 * RETURN :	Zero
 *			
 */
inofree(ip, ino)
struct inode	*ip;
ino_t	ino;
{
	struct vmdmap *ptr;
	struct inode *ipm;
	int    sr13save, rc;
	label_t jbuf;           

	/* find the inode table entry for .inodemap 
	 * map inodemap into virtual memory
	 */
	ipm = ip->i_ipmnt->i_ipinomap;

	sr13save = chgsr(13, SRVAL(ipm->i_seg,0,0));
	ptr = (struct vmdmap *)SR13ADDR;

	IWRITE_LOCK(ipm);
	
	/* set up for i/o exception.
         */
	if (rc = setjmpx(&jbuf))
		goto out;

	/* free the inode number ino.
	 */
	v_bitfree(ptr,ino,1);

	/* restore state and return
	 */
	clrjmpx(&jbuf);

out:
	IWRITE_UNLOCK(ipm);
	(void)chgsr(13, sr13save);			
	return 0;
}


/*
 * NAME:	iread (ip)
 *
 * FUNCTION:	Reads the disk inode into the inode table entry
 *		pointed to by ip. on entry i_dev and i_number specify
 *		the device and inode number. the inode numbers INODES_I
 *		and INDIR_I and SUPER_I are read using the buffer pool.
 *		all others using .inodes.
 *
 * PARAMETERS:	ip	- pointer to inode to read
 *
 * RETURN :	0 - ok
 *		EIO - error reading inode.
 */

iread (ip)              
struct inode *ip;		/* pointer to inode table entry */
{
	int	rc = 0;
	struct buf *bp, *bread(dev_t, daddr_t);
	struct dinode *diptr;
	struct inode *ipi;
	int sr12save, sid, sidx;
	label_t jbuf;           

	/* special case i_number = 0 for mount or log inode
	 */
	if (ip->i_number == 0)
	{	
		bzero(&ip->i_dinode,DILENGTH);
		return 0;
	}

	/* read on-disk inodes for file system meta-data files 
	 * through the buffer pool at mount time.
	 */
	if (ip->i_number == INODES_I || ip->i_number == INDIR_I || 
	    ip->i_number == SUPER_I)
	{
		bp = bread((dev_t)ip->i_dev,
			(daddr_t)((INODES_B * PAGESIZE) / SBUFSIZE));

		if (bp->b_error)
		{
			brelse(bp);
			return EIO;
		}
		else
		{
			diptr = (struct dinode *)(bp->b_un.b_addr);
			diptr += ip->i_number;
			bcopy(diptr, &ip->i_dinode, DILENGTH);  
			brelse(bp);
			return 0;
		}
        }

	/* inode for non-meta-data file.  
	 * map the segment of .inodes containing the specified inode.
	 */   
	ipi = ip->i_ipmnt->i_ipinode;
	sidx = STOI(ipi->i_seg);
	sid = ITOS(sidx, ip->i_number / (PSIZE/DILENGTH));
	sr12save = chgsr(12,SRVAL(sid,0,1));

	/* if error occurs , restore state and return.
	 */
	if (rc = setjmpx(&jbuf))
	{
		(void)chgsr(12,sr12save);
		return rc;
	}

	/* copy the on-disk inode from .inodes to in-memory inode
	 */
	diptr = (struct dinode *) SR12ADDR;
	diptr = diptr + (ip->i_number & ((SEGSIZE/DILENGTH) - 1));
	bcopy(diptr, &ip->i_dinode, DILENGTH);        

	/* restore state and return
	 */
	(void)chgsr(12,sr12save);
	clrjmpx(&jbuf);

	return rc;
}


/*
 * NAME:	iwrite (ip)
 *
 * FUNCTION:	Write the disk inode part of ip to .inodes
 *		if the inode is a regular file or directory
 *		or long symbolic link the size and disk address
 *		fields are NOT updated unless the IFSYNC flag
 *		in ip is set.
 *		
 *		this procedure is only called from finicom().
 *
 * PARAMETERS:	ip	- pointer to inode whose disk inode we need to write
 *
 * RETURN :	Zero
 *			
 */

iwrite (ip)
struct inode *ip;
{
	int sr12save, update, type, oldsize, oldnblocks, oldrindirect;
	int k, sid, sidx;
	uint daddr, olddaddr[NDADDR];
	struct dinode *diptr;
	struct inode *ipi;

	ASSERT(ip->i_gen != 0);

	/* map the appropriate segment of .inodes into VM
	 */
	ipi = ip->i_ipmnt->i_ipinode;
	sidx = STOI(ipi->i_seg);
	sid = ITOS(sidx, ip->i_number / (PSIZE/DILENGTH));
	sr12save = chgsr(12,SRVAL(sid,0,1));

	/* get pointer to the disk inode.
	 */
	diptr = (struct dinode *) SR12ADDR;
	diptr = diptr + (ip->i_number & ((SEGSIZE/DILENGTH) - 1));

	/* Determine if we should update the disk addresses, size field,
	 * and block count.  This code handles the case where allocation
	 * has occurred on the file (possibly growing from direct to
	 * indirect), yet the allocation has not been committed yet.  If
	 * we are incurring a trivial commit (such as changing the mode)
	 * we don't want to write the uncommitted allocation.  Therefore
	 * we save the old size, old nblocks, old direct addresses and 
	 * old rindirect from .inodes in order to restore them after
	 * the trivial update of the inode.
	 */
	update = (ip->i_flag & IFSYNC || ip->i_cflag & CMNEW);
	if (!update)
	{
		type = ip->i_mode & IFMT;
		switch(type) {
		case IFREG:
		case IFDIR:
			break;
		case IFLNK:
			if (ip->i_size <= D_PRIVATE)
				update = 1;
			break;
		default:
			update = 1;
		}

		if (!update)
		{
			oldsize = diptr->di_size;
			oldnblocks = diptr->di_nblocks;
			oldrindirect = diptr->di_rindirect;

			for (k = 0; k < NDADDR; k++)
				olddaddr[k] = diptr->di_rdaddr[k];
		}
	}

	/* get transaction lock on the disk inode in .inode
	 */
	vm_gettlock(diptr, DILENGTH); 

	/* copy the inode.
	 */
	bcopy(&ip->i_dinode, diptr, DILENGTH);

	/* If no update then restore old size, nblocks, direct addresses
	 * and rindirect.  We do not want any uncommitted inode allocation
	 * in the inode.
	 */
	if (!update)
	{
		diptr->di_size = oldsize;
		diptr->di_nblocks = oldnblocks;
		for (k = 0; k < NDADDR; k++)
		{
			diptr->di_rdaddr[k] = olddaddr[k];
		}
		diptr->di_rindirect = oldrindirect;
	}

	(void)chgsr(12,sr12save); 
	return 0;
}


/*
 * NAME:	iflush(ip)
 *
 * FUNCTION:	Initiate i/o to page out modified pages of segment
 *
 * PARAMETERS:	ip	- pointer to inode whose pages we want to write out
 *
 * RETURN :	errors from subroutines
 *			
 */

iflush(ip)
struct inode *ip; 
{
	if (ip->i_seg == 0)
		return 0;

	if (ip->i_mode & IFJOURNAL)
		return(vcs_sync(ip, NULL));

	return(vcs_write(ip->i_seg,0,MAXFSIZE/PSIZE,FORCE));
}


/*
 * NAME:	isetsize(ip,size)
 *
 * FUNCTION:	if the file is mapped read-write (via shmat system
 *		call) and size specifies a byte in the last page
 *		of the file, i_size is set to size and the last
 *		page is made read-only by changing its storage protect
 *		key. otherwise i_size is just set to size.
 *
 * PARAMETERS:	ip	- pointer to inode whose pages we want to write out
 *		size 	- size of file.
 *
 * RETURN :	0
 *			
 */

isetsize(ip,size)
struct inode *ip; 
uint	size;
{
	struct gnode *gp;

	/* check for write-mappers.
	 */
	gp = ITOGP(ip);
	if (gp->gn_mwrcnt == 0)
	{
		ip->i_size = size;
		return 0;
	}

	/* map for writing
	 */
	vcs_setsize(ip,size);
	return 0;
}


/*
 * ismodified(ip)
 * returns 1 if file was possibly modified by a store
 * from a process which had mapped file via shmat().
 */

int
ismodified(ip)
struct inode *ip; 
{
	struct gnode *gp;

	/* check for write-mappers at any time
	 */
	gp = ITOGP(ip);
	if (gp->gn_flags & GNF_WMAP)
		return(vcs_qmodify(ip->i_seg));
	return 0;
}


/*
 * isreadonly(ip)
 *
 * returns 1 if file-system containing file is mounted read-only
 * 0 if mounted read-write.
 */

int
isreadonly(ip)
struct inode *ip;
{
	struct inode * ipmnt;

	ipmnt = ip->i_ipmnt;
	return ((ipmnt->i_iplog) ? 0 : 1);
}


/*
 * isyncdata(ip, offset, len, osize, syncall)
 *
 * does a syncronous write on file data, and indoe if new blocks
 * have been added to the file, or the write was to the last page
 * of the file
 */
int
isyncdata(ip, offset, len, osize, syncall)
struct inode *ip;
uint offset;
uint len;
int osize;
int syncall;
{
	int savevmsr;
	int sid;
	int sidx;
	int newflag;
	int pfirst;
	int plast;
	int rc;

	/*
	 * if write was into the last page of the file then
	 * commit the inode.  There is no way of knowing what
	 * i_size was when last committed.
	 */
	if (offset + len > (osize & ~(PAGESIZE-1)))
		goto commit_inode;

	/*
	 * if there are new blocks for the file then commit
	 * the inode
	 */
	sid = ip->i_seg;
	sidx = STOI(sid);

	savevmsr = chgsr(VMMSR, vmker.vmmsrval);
	newflag = (scb_newdisk(sidx) != 0);
	(void)chgsr(VMMSR, savevmsr);

	if (newflag)
		goto commit_inode;

	if (syncall)
	{
		/* sync all data ignoring the length
		 */
		pfirst = 0;
		plast = MAXFSIZE/PSIZE - 1;
	}
	else
	{
		/*
		 * calculate the first and last page in range to be written
		 */
		pfirst = offset >> L2PSIZE;
		plast = (offset+len == 0) ? 0 : BTOPN(offset+len);
	}
	/* do a synchronous write on data only
	 */
	BUGLPR(isyncdatadbg, 1, ("isyncdata: calling vm_writep %x %x %x\n",
		sid, pfirst, plast - pfirst + 1));

	rc = vm_writep(sid, pfirst, plast - pfirst + 1);
	ASSERT(rc == 0);
	rc = vms_iowait(sid);

	return(rc);

commit_inode:
	BUGLPR(isyncdatadbg, 1, ("isyncdata: calling commit\n"));
	ip->i_flag |= IFSYNC;
	rc = commit(1, ip);
	return(rc);
}
