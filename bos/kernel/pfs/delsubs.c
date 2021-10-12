static char sccsid[] = "@(#)96	1.44.1.21  src/bos/kernel/pfs/delsubs.c, syspfs, bos41J, 9518A_all 4/28/95 11:15:02";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: 	itrunc, ctrunc, ctrunc1, iclear, delpages, deletep,
 *		lognodo, bclear, ipgrlse, freedisk, freeoldfrags,
 *		findptr, ifreeseg, ifreenew, freesingle, iextend
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

#include  "jfs/fsdefs.h" 
#include  "jfs/jfslock.h"
#include  "jfs/log.h"
#include  "jfs/ino.h" 
#include  "jfs/inode.h"
#include  "jfs/commit.h"
#include  "sys/errno.h"
#include  "sys/malloc.h"
#include  "vmm/vmsys.h"
#include  "sys/syspest.h"

/*
 * NAME:	itrunc (ip, length, crp)
 *
 * FUNCTION:    truncate up/down a regular file to specified size, or 
 *		truncate down directory or symbolic link to zero length 
 *		(length is ignored and assumed to be zero if the object
 * 		is a directory or symbolic link). 
 *		if length is > 0, the file must be open (i.e.bound to a 
 *		VM segment). 
 *		return 0 if type is not one of these.
 *
 * PARAMETERS:	ip	- inode to truncate
 *		length	- new length
 *		crp	- credential
 *
 * RETURN :	0 	- ok
 *		EMFILE  - file is opened deferred update.
 *		E2BIG 	- length too big.
 *		EIO	- permanent i/o error.
 *		ENOSPC	- out of disk space
 *		EDQUOT	- reached quota limit
 *			
 * SERIALIZATION: the IWRITE_LOCK is held on entry/exit.
 */

itrunc(ip, length, crp)  
struct inode *ip; 
ulong length;			/* new length (size) of file */
struct ucred *crp;		/* cred pointer 	     */
{

	int type, rc, pfirst, plast, nxtpage, nbytes, k;
	uint newsize;
	label_t jbuf;           
	uint	srvmsave, sr12save, sr13save;
	struct comdata com;
	int wrc;
	struct ucred *ucp = NULL;

	/* set newsize. return if not reg,dir,or symbolic link
	 */
	type = ip->i_mode & IFMT;

	switch(type) {
	case IFREG:
		newsize = length;
		break;
	case IFDIR:
		newsize = 0;   
		break;
	case IFLNK:
		/* if it's all in the inode do it here
		 */
		if (ip->i_size <= D_PRIVATE)
		{
			ip->i_size = 0;
			imark(ip, ICHG|IUPD);
			return (commit(1,ip));
		}
		newsize = 0;
		break;
	default:
		return 0;
	}

	/* not allowed for deferred update files
	 */
	if (ip->i_flag & IDEFER)
		return (EMFILE);

	/* check u_limit.
	 */
	if (newsize > U.U_limit)
		return(EFBIG);

	/* file must be open if newsize is not zero
	 */
	assert(newsize == 0 || ip->i_seg );

	/* check for truncate up (extension).
	 */
	if (newsize >= ip->i_size)
	{
		/* simply mark the inode as changed and updated
		 * if no change in size (ala BSD).
		 */
		if (newsize == ip->i_size)
		{
			imark(ip, ICHG|IUPD);
			return 0;
		}

		/* extend the inode to newsize.
		 */
		return (iextend(ip,newsize,crp));
	}
	
	/* truncate down.
	 * set up for i/o and no space exceptions.
 	 */
	com.number = 0;
	sr12save = mfsr(12);
	sr13save = mfsr(13);
	srvmsave = chgsr(VMMSR, vmker.vmmsrval);
	if (rc = setjmpx(&jbuf))
	{
		/* zero'ing partial pages can cause new blocks to be added.
		 * this is done before any commit processing begins.  If
		 * ENOSPC or EDQUOT exception occurs do not call comfail().
		 */
		if (!(rc == ENOSPC || rc == EDQUOT))
			comfail(&com,rc);

		(void)chgsr(VMMSR,srvmsave);
		(void)chgsr(12,sr12save);
		(void)chgsr(13,sr13save);

		if (ucp)
			U.U_cred = ucp;
		return rc;
	}

        /* Page fault quota allocation check looks at uid but NFS
         * server which is mono threaded always runs as uid zero.
         */
        if (U.U_procp->p_threadcount == 1 && crp->cr_uid != U.U_uid)
        {
                ucp = U.U_cred;
                U.U_cred = crp;
        }

	/* if the new length is not an integral number of pages
	 * clear the file between newsize and next page boundary.
	 * if we are truncating into a file hole, this will cause
	 * a full block to be allocated for the logical block.
	 */
	pfirst = BTOPN(newsize) + 1;
	if (newsize & POFFSET)
	{
		nbytes = pfirst*PSIZE - newsize;
		bclear (ip, newsize, nbytes);
	}

	/* if the file was commited with zero link count before
	 * its permanent resources were already truncated.
	 */
	if (ip->i_cflag & CMNOLINK)
	{
		COMBIT_LOCK(ip);
		rc = ctrunc1(ip,newsize);
		COMBIT_UNLOCKD(ip);
		goto fini;
	}

	/* delete pages and set new size.
	 */
	imark(ip, ICHG|IUPD);
	plast = MAXFSIZE/PSIZE - 1;
	if (rc = delpages(ip,pfirst,plast,newsize,&com,&wrc))
		comfail(&com,rc);
	else
		rc = wrc;

	/* restore segment register
	 */
fini:
	clrjmpx(&jbuf);
	(void)chgsr(VMMSR,srvmsave);

	if (ucp)
		U.U_cred = ucp;

	return rc;
}

/*
 * NAME:	ctrunc (cd)
 *
 * FUNCTION:    truncate a regular file, directory or symbolic
 *		link to zero length. return 0 if type is not 
 *		one of these.
 *
 *		if the file is currently associated with a VM segment
 *		only permanent disk and inode map resources are freed,
 *		and neither the inode nor indirect blocks are modified
 *		so that the resources can be later freed in the work
 *		map by ctrunc1.
 *
 *		if there is no VM segment on entry, the resources are
 *		freed in both work and permanent map.
 *
 *		this procedure is only called from comlist when
 *              an inode is committed with i_nlink = 0.
 *
 * PARAMETERS:	cd	- pointer to commit data structure.
 *			  current inode is the one to truncate.
 *
 * RETURN :	Errors from subroutines
 *			
 */

ctrunc(cd)  
struct comdata * cd;
{
	struct inode *ip;
	int type, plast, rc;

	/* return if not reg,dir,or symbolic link or if size is
	 * already ok.
	 */
	ip = cd->iptr[cd->current];
	type = ip->i_mode & IFMT;

	switch(type) {
	case IFREG:
	case IFDIR:
		if (ip->i_size == 0)
			return 0;
		break;
	case IFLNK:
		/* if contained in the disk inode just set size to zero.
		 */
		if (ip->i_size <= D_PRIVATE)
		{
			ip->i_size = 0;
			return 0;
		}
		break;
	default:
		return 0;
	}

	/* set the IFSYNC flag to cause iwrite to update inode.
	 * free resources in both work and permanent if there
	 * is no segment associated with file, otherwise
	 * just permanent resources. 
	 */
	ip->i_flag |= IFSYNC;
	plast = MAXFSIZE/PSIZE - 1;
	if (ip->i_seg)
	{
		type = V_PMAP;
		COMBIT_LOCK(ip);
	}
	else
	{
		type = V_PWMAP;
	}
	rc = deletep(cd,0,plast,0,type);
	return rc;
}

/*
 * NAME:	ctrunc1 (ip,newsize)
 *
 * FUNCTION:    free resources of a file in working map for a 
 *		file previously committed with zero link count
 *		while associated with a VM segment.
 *
 *		the disk blocks of the file and its indirect blocks
 *              are marked as free in the work map, and the pages
 *		in .indirect, if any, are freed.
 *
 * PARAMETERS:	ip	- pointer to inode of file.
 *		newsize - size to truncate to.
 *
 * RETURN :	0 -ok
 *			
 */

ctrunc1(ip,newsize)  
struct inode * ip;
uint	newsize;
{
	struct comdata com;
	int rc, type, pfirst, plast, nbytes;

	/* return if not reg,dir,or symbolic link or if size is
	 * already ok.
	 */
	type = ip->i_mode & IFMT;

	switch(type) {
	case IFREG:
	case IFDIR:
		if (ip->i_size == 0)
			return 0;
		break;
	case IFLNK:
		/* if its contained in inode nothing to do.
		 */
		if (ip->i_size <= D_PRIVATE)
			return 0;
		break;
	default:
		return 0;
	}

	ASSERT(ip->i_seg);

	/* release whole pages beyond newsize.
	 */
	pfirst = BTOPN(newsize) + 1;
	plast = MAXFSIZE/PSIZE - 1;

	/* if the new length is not an integral number of pages
	 * clear the file between newsize and next page boundary.
	 */
	if (newsize & POFFSET)
	{
		nbytes = pfirst*PSIZE - newsize;
		bclear (ip, newsize, nbytes);
	}

	/* setup to call deletep().
	 * only free things from the work maps.
	 */
	com.iptr[0] = ip;
	com.ilog = (struct inode *)(com.current = 0);
	com.ipind = ip->i_ipmnt->i_ipind;
	rc = deletep(&com,pfirst,plast,newsize,V_WMAP);
	return rc;

}

/*
 * NAME:	iclear (ip, offset, length, crp)
 *
 * FUNCTION:	On entry ip points to the inode table entry for
 * 		a regular file. offset specifies the byte offset in
 * 		the file of the first byte to zero and length the
 * 		number of bytes to clear to zero.
 *
 * PARAMETERS:	ip	- inode to truncate
 *		offset	- where to start clearing
 *		length	- length to clear (possibly zero).
 *		crp	- credential
 *
 * RETURN :	0	- ok
 *		EINVAL	- not a regular file 
 *		E2BIG 	- offset + length too big.
 *		EMFILE	- file is opened deferred update.
 *		EIO	- permanent i/o error.
 *		ENOSPC	- out of disk space
 *		EDQUOT	- reached quota limit
 *
 * SERIALIZATION: the IWRITE_LOCK held upon entry/exit
 *			
 */

int
iclear(struct inode *ip, uint offset, uint length, struct ucred *crp)
{
	int 	 rc, pfirst, plast, nbytes, wrc, newsize;
	uint 	 endpage, beginpage, endclear;
	volatile uint srvmsave, sr12save, sr13save;
	struct 	 comdata com;
	label_t  jbuf;           
	struct 	 ucred *ucp = NULL;

	/* type must be regular file.
	 */
	if (IFREG != (ip->i_mode & IFMT) )
		return (EINVAL);

	/* can't be opened deferred update
	 */
	if (ip->i_flag & IDEFER)
		return (EMFILE);

	if (length == 0) 	/* No work to do */
		return 0;

	/* "endclear" is the offset of the last byte to clear in the file.
	 */
	endclear = offset + length - 1;
	if (endclear > U.U_limit)
		return(EFBIG);

	/* bind to a vm segment if not already bound.
	 */
	if (ip->i_seg == 0)
		if (rc = ibindseg(ip))
			return rc;

	/* check if last byte to be cleared is beyond the
	 * current inode size.  if so, extend the inode
	 * size to include this byte.
	 */
	if (endclear >= ip->i_size)
	{
		if (rc = iextend(ip,endclear + 1,crp))
			return rc;
	}

	/* set up for i/o exceptions.
 	 */
	com.number = 0;
	sr12save = mfsr(12);
	sr13save = mfsr(13);
	srvmsave = chgsr(VMMSR, vmker.vmmsrval);
	if (rc = setjmpx(&jbuf))
	{
		/* zero'ing partial pages can cause new blocks to be added.
		 * this is done before any commit processing begins.  If
		 * ENOSPC or EDQUOT exception occurs do not call comfail
		 */
		if (!(rc == ENOSPC || rc == EDQUOT))
			comfail(&com,rc);
		(void)chgsr(VMMSR,srvmsave);
		(void)chgsr(12,sr12save);
		(void)chgsr(13,sr13save);

	        if (ucp)
			U.U_cred = ucp;

		return rc;
	}
        /* Page fault quota allocation check looks at uid but NFS
         * server which is mono threaded always runs as uid zero.
         */
        if (U.U_procp->p_threadcount == 1 && crp->cr_uid != U.U_uid)
        {
                ucp = U.U_cred;
                U.U_cred = crp;
        }

	newsize = ip->i_size;

	/* "endpage" is the offset of the last byte in the "offset" page.
	 * "beginpage" is the beginning offset of the last page to clear.
	 */ 
	endpage   = offset | (PSIZE - 1);
	beginpage = endclear & ~(PSIZE - 1);

	/* If "beginpage" is greater than the starting "offset" then we 
	 * will potentially be clearing from two pages.
	 */
	if (beginpage > offset)
	{
		/* Calculate the number of bytes to clear in the first page.  
		 * If "offset" is the beginning byte of the page, then 
		 * "nbytes" will be zero.  No clearing should be done on 
		 * this page since it will be released below in deletep().
		 */
		if (nbytes = (endpage - offset + 1) & (PSIZE - 1))
			bclear(ip, offset, nbytes);

		/* Calculate and clear bytes from the second page
		 */
		nbytes = endclear - beginpage + 1;
		bclear(ip, beginpage, nbytes);
	}
	else
	{
		/* We are only clearing bytes from one page.  If "nbytes" 
		 * doesn't equal a full page or if "nbytes" does equal a
		 * full page, and this is the last page of the file, then
		 * clear the data.
		 */
		nbytes = endclear - offset + 1;
		if ((nbytes != PSIZE) || (BTOPN(newsize) == BTOPN(offset + 1)))
			bclear(ip, offset, nbytes);
	}
		
	/* lock the segment to prevent addition of new pages. 
	 */
	COMBIT_LOCK(ip);

	/* re-establish i_size after the bclear()s (above) with
	 * the segment locked.
	 */
	isetsize(ip, newsize);
		
	/* Release whole pages between offset and endclear.
	 * "pfirst" is the first page to the right of the 
	 * page containing the byte at "offset - 1".  "plast" 
	 * is the first page to the left of the page containing 
	 * the byte at "endclear".  "plast" is adjusted down if 
	 * this is the last page of the file (geometry rules  
	 * require allocation on that page), or "endclear" is
	 * within the last page.
	 */
	pfirst = BTOPN(offset) + 1;
	plast  = BTOPN(endclear + 1);
	 
	if (((endclear + 1) & (PSIZE - 1)) || BTOPN(newsize) == plast)
		plast--;

	/* if the file was commited with zero link count before
	 * its permanent resources were already cleared. so we
	 * use deletep to free things in work map.
	 */
	if (ip->i_cflag & CMNOLINK)
	{
		com.iptr[0] = ip;
		com.ilog = (struct inode *)(com.current = 0);
		com.ipind = ip->i_ipmnt->i_ipind;
		rc = deletep(&com,pfirst,plast,ip->i_size,V_WMAP);
		COMBIT_UNLOCKD(ip);
	}
	else
	{
		/* Mark the file as changed or updated.
		 */
		imark( ip, ICHG|IUPD );

		/* delete pages. delpages will vcs_unlockseg.
	 	 */
		if(rc = delpages(ip,pfirst,plast,ip->i_size,&com,&wrc))
			comfail(&com,rc);
		else
			rc = wrc;
	}

	clrjmpx(&jbuf);
	(void)chgsr(VMMSR,srvmsave);

	if (ucp)
		U.U_cred = ucp;
	return rc;
}

/*
 * NAME:	delpages (ip, pfirst, plast, newsize, cd, wrstat)
 *
 * FUNCTION:	pfirst specifies the first page and plast the last
 *		page to delete from the file, and newsize the size
 *		of the file after the deletion.
 *		
 *		the disk blocks of the file and its indirect
 *		blocks are freed in both the permanent and work
 *		map.
 *
 *		all changes to the file are committed.
 *
 *		on entry VMM segment is assumed to be mapped.
 *
 * PARAMETERS:	ip	- pointer to inode
 *		pfirst	- first page to delete
 *		plast	- last page to delete
 *		newsize - new file size.
 *		cd      - pointer to comdata structure.
 *		wrstat - pointer to returned file write status
 *
 * RETURN :	Errors from subrouties
 *			
 */

static
delpages(ip, pfirst, plast,newsize, cd, wrstat)
struct inode *ip;
int pfirst;			/* first page to delete */
int plast;			/* last page to delete */
uint newsize;			/* new file size */
struct comdata *cd;
int *wrstat;
{
	int rc;
	struct commit clist;

	/* set up for commit processing
	 * initcom will lock the VM segment.
	 */
	*wrstat = 0;
	clist.number = 1;
	clist.iptr[0] = ip;
	ip->i_flag |= IFSYNC;
	if ( rc = initcom(cd, &clist)) 
		return rc;

	/* delete the pages from both working and permanent
	 */
	if ( rc = deletep(cd,pfirst,plast,newsize, V_PWMAP))
		return rc;

	/* if the newsize of the file is not zero add to
	 * the file any new disk blocks and also write 
	 * the pages out.
	 */
	if (newsize)
	{
		iflush(ip);
		*wrstat = vms_iowait(ip->i_seg);
		if (rc = commit1(cd))
			return rc;
	}

	/* finicom is the last half of commit processing.
	 */ 
	rc = finicom(cd);
	return(rc);
}

/*
 * NAME:	deletep (cd, pfirst, plast, newsize, type)
 *
 * FUNCTION:	pfirst specifies the first page and plast the last
 *		page to delete from the file, and newsize the size
 *		of the file after the deletion. newsize must be 
 *		not greater than the current size in i_size.
 *
 *		if file has a segment it should be vcs_locked
 *		by caller.
 *
 *		the type parameter specifies whether permanent, 
 *		or work, or both types of resources are to be
 *		freed. the option perm only is only used by 
 *		ctrunc (newsize = 0). the option work only is
 *		only used by ctrunc1. ctrunc is always called     
 *		before ctrunc1.
 *
 * PARAMETERS:	cd	- pointer to initialized comdata structure
 *			- with current iptr designating the file.
 *		pfirst	- first page to delete
 *		plast	- last page to delete
 *		newsize - new file size.
 *		type	- permanent only (V_PMAP) , work only (V_WMAP)
 *			  both perm and work (V_PWMAP).
 *
 * RETURN :	Errors from subroutines
 *			
 */

static
deletep(cd, pfirst, plast,newsize,type)
struct comdata *cd;
int pfirst;			/* first page to delete */
int plast;			/* last page to delete */
uint newsize;			/* new file size */	
int  type;			/* V_WMAP, V_PWMAP, or V_PMAP */
{
	struct idblock *idptr;
	struct inode *ipind, *ip;
	uint * ptr, *findptr(), daddr, page, *ptr0, rind, vind;
	int rc, k, n, interval, npages, sr12save, newlastp, oldlastp;
	int lastpage, noind, sglind, notneeded, log, nfreed;
	int fperpage, nfrags, oldnfrags;
	union xptentry *xpt, oldf;
	struct indir * indptr;
	struct vmdlist ** anchor;
	struct vmdlist * wanchor;
	int	xd;	/* index of entry in transaction line */
	int	xdlock; /* boolean of transaction lock for line */
	int	xs;	/* index of entry in transaction line */
	int	xslock; /* boolean of transaction lock for line */

	/* init some variables 
	 */
	wanchor = 0;
	nfreed = 0;
	log = (type != V_WMAP); /* boolean to journal */
	ip = cd->iptr[cd->current];
	fperpage = ip->i_ipmnt->i_fperpage;
	anchor = (type == V_PWMAP) ? &cd->freepw :
			(type == V_PMAP) ? &cd->freep : &wanchor;

	/* release page frames if it has a segment unless type
	 * is V_PMAP.
	 */
	if (ip->i_seg && type != V_PMAP)
		ipgrlse(ip,pfirst,plast);

	/* If compression, wait for any pending pageouts to complete.
	 * This is necessary because re-allocation of disk may occur 
	 * during pageout.  Additionally if the frames have not been
	 * deleted above, the COMBIT ensures LRU replacement does not
	 * cause further reallocation after this vms_iowait().
	 */
	if (ip->i_seg && ip->i_compress)
		vms_iowait(ip->i_seg);

	/* if the segment is journalled then newsize should be zero.
	 * in this case a noredo log record is needed.
	 */
	if (ip->i_mode & IFJOURNAL && log)
	{
		assert(newsize == 0);
		if (rc = lognoredo(cd,ip->i_number))
			return rc;
	}

	/* 
	 * free the disk addresses from the incore inode and .indirect. 
	 */

	/* can work be done against inode alone?
	 */
	oldlastp = BTOPN(ip->i_size);
	newlastp = BTOPN(newsize);
	plast = MIN(plast, oldlastp);
	if (oldlastp < NDADDR)
	{
		xpt = (union xptentry *) (&ip->i_rdaddr[0]);
		for (k = pfirst; k <= plast; k++)
		{
			if ((xpt+k)->word)
			{
				dlistadd(anchor,(xpt+k)->word);
				nfreed += (fperpage - (xpt+k)->fptr.nfrags);
			}
		}
		goto finish;
	}

	/* must compute on .indirect. 
	 * map the indirect blocks into .indirect if necessary. 
	 * we don't have to lock its inode because the pages
	 * of it are unique to ip.
	 */
	if (ip->i_seg == 0)
	{
		assert(newsize == 0);
		if (rc = iallocind(ip))
			return rc;
	}

	ipind = cd->ipind;
	sr12save = chgsr(12,SRVAL(ipind->i_seg,0,log));
	indptr = (struct indir *) (SR12ADDR);

	/* free the disk addresses from the single indirect blocks.
	 * free the single indirect blocks which aren't needed in new 
	 * sized file. 
	 * the outer loop processes in steps of the pages covered by 
	 * one single indirect block. 
	 * note that .indirect is only updated for the blocks retained 
	 * in the new sized file. also if type is V_PMAP the indirect 
	 * blocks are not free and their disk pointers left unmodified.
	 */
	noind = NOIND(newsize); /* no indirect blocks file ? */
	sglind = SGLIND(newsize); /* single indirect block file ? */

	xd = (plast/(PAGESIZE/4)) % (LINESIZE/sizeof(*idptr));
	xdlock = FALSE;
	for (k = plast; k >= pfirst; k = k - npages, xd--)
	{
		/* help on variables:
		 * k - page number of either plast or the last page
		 * covered by the single indirect block being processed.
		 * interval - page number of the first page covered
		 * by the single indirect block being processed. 
		 * lastpage - page number of the lowest page to process
		 * covered by the single indirect block being processed.
		 * npages - number of pages to be processed covered by
		 * the single indirect block being processed.
		 */ 
		/* end inner loop on  PAGESIZE/4 boundary or pfirst
		 */
		interval = k  & (~(PAGESIZE/4 - 1));
		lastpage = MAX (interval, pfirst);
		npages = k - lastpage + 1;  

		/* set ptr to single indirect block covering page k
		 */
		ptr0  = findptr(ip, k);	
		if (ptr0 == NULL) 
			goto xline;

		/* don't update indirect block if it won't be needed
		 * after pages are deleted. if the new-sized file 
		 * is single-indirect we have to keep the indirect 
		 * block even if it has no non-null disk pointers.
		 */
		if (noind)
		{
			notneeded = 1;
		}
		else
		if (sglind && (k < PAGESIZE/4))
		{
			notneeded = 0;
		}
		else
		{
			notneeded = (npages == PAGESIZE/4);
			notneeded |= newlastp < interval;
		}

		/* process the single indirect block backward.
		 */
		ptr = ptr0 + (k - interval);
		xs = k % (LINESIZE/sizeof(*ptr));
		xslock = FALSE;
		for (n = k; n >= lastpage; n--, ptr--, xs--) 
		{
			if (*ptr) 
			{
				dlistadd(anchor, *ptr);
				xpt = (union xptentry *) ptr;
				nfreed += fperpage - xpt->fptr.nfrags;
				if (!notneeded) 
				{
					if (log) 
					{
						if (xslock == FALSE) 
						{
							vm_gettlock(ptr,
								sizeof(*ptr));
							xslock = TRUE;
						}
					}

					/* reset single indirect block entry.
					 */
					*ptr = 0;
				}
			}
			
			if (xs == 0) 
			{
				xs = LINESIZE/sizeof(*ptr);
				xslock = FALSE;
			}
		} /* end for singleindirect block */

		if (notneeded)
		{
			/* always free the disk block associated with 
			 * the single indirect block 
			 */
			ptr = ptr0;
			page = (SOFFSET & (uint) ptr) >> L2PSIZE;
			daddr = indptr->indptr[page];
			lognodo(cd, daddr);
			dlistadd(anchor,daddr);

			/* if this is the first single indirect block
			 * copy the blocks covered by the inode
			 */
			if (k < PAGESIZE/4 )
				for (n = 0; n < NDADDR; n++, ptr++)
					ip->i_rdaddr[n] = *ptr;

			/* free the page associated with the single indirect 
			 * block and update the corresponding double indirect 
			 * block entry unless permanent only
			 */
			if (type != V_PMAP)
			{
				/* delete single indirect block
				 */
				freeiblk(ip,page);

				/* no double indirect block now ?
	 			*/
				if (SGLIND(ip->i_size)) 
				{
					ip->i_vindirect = 0;
					ip->i_rindirect = 0;
				}	

				/* has a double indirect block and will keep 
				 * double indirect in new-sized file.  update
	 			 * the double indirect block entry  to reflect
				 * delete of the single indirect block.
	 			 */
				else if (DBLIND(newsize)) 
				{
					idptr = (struct idblock *)(SR12ADDR + 
						ip->i_vindirect*PAGESIZE);
					idptr = idptr + k/(PAGESIZE/4);       
					if (log) 
					{
						if (xdlock == FALSE) 
						{
							vm_gettlock(idptr,
								sizeof(*idptr));
							xdlock = TRUE;
						}
					}

					idptr->id_vaddr = 0;
					idptr->id_raddr = 0;
				}
			} 
		} /* end (notneeded) */

xline:
		if (xd == 0) 
		{
			xd = LINESIZE/sizeof(*idptr);
			xdlock = FALSE;
		}
	} /* end for double indirect block */

	/* free double indirect block ?
	 */
	if (DBLIND(ip->i_size) && !DBLIND(newsize))
	{
		/* always free the disk block associated with the double 
		 * indirect block. 
		 */
		page = ip->i_vindirect;
		daddr = indptr->indptr[page];
		lognodo(cd,daddr);
		dlistadd(anchor,daddr);

		/* free the page associated with the double indirect block 
		 * unless permanent only.
		 */
		if (type != V_PMAP)
		{
			/* any indirect blocks in new sized file ?
		 	 */
			if (noind)
			{
			 	vind = rind = 0;
			}
			else
			{
				/* retain single indirect.
				 */
				ptr = (uint *)(SR12ADDR + PAGESIZE*page);
				idptr = (struct idblock *)ptr;
				vind = idptr->id_vaddr;
				rind = idptr->id_raddr;
			}

			/* we don't want to update the root indirect block
			 * addressses in the inode until we have freed the
			 * double indirect block since freeiblk() may take
			 * an exception.
			 */
			freeiblk(ip,page);
			ip->i_vindirect = vind;
			ip->i_rindirect = rind;
		}
	}

	(void)chgsr(12,sr12save);

	finish:

	/* free the inode's old committed allocation.  if freeing
	 * from the permanent map (V_PMAP or V_PWMAP), we'll free
	 * the old allocation from both the permanent and working
	 * map at this time.  freeing the old allocation from the
	 * working map when we are processing the permanent map only
	 * (V_PMAP) allows us to free the old allocation sooner.  
	 * also, if we did not free from the working map now we 
	 * would have to remember the old allocation so that it
	 * could be freed later when we process the working map.
	 */
	freeoldfrags(cd, (type == V_WMAP) ? &wanchor : &cd->freepw);

	/* update i_size and clear disk addrs in inode unless perm
	 * only. free disk blocks here if work only.
	 */
	if (type != V_PMAP)
	{
		oldnfrags = BTOFR(ip->i_size,fperpage);

		/* set size. ok to do this because scb_combit is set.
		 * also, vcs_relfrag() expects i_size to reflect the
		 * new inode geometery.
		 */
		ip->i_size = newsize;

		/* clear disk addresses.
		 */
		for (n = pfirst; n < NDADDR && n <= plast; n++) 
			ip->i_rdaddr[n] = 0;

		/* truncate the new last page of the file's allocation if
		 * the page should be partially back and is previous to
		 * the old last page or is the old last page and should
		 * have fewer fragments. (not for compression).
		 */
		if (ip->i_compress == 0)
		{
			nfrags = BTOFR(newsize,fperpage);
			if (nfrags != fperpage && (newlastp < oldlastp ||
		    	    (newlastp == oldlastp && nfrags < oldnfrags)))
			{
				vcs_relfrag(ip->i_seg,newlastp,nfrags,&oldf);
				dlistadd(anchor,oldf.word);
				nfreed += (fperpage - oldf.fptr.nfrags);
			}
		}

		/* update nblocks and disk quota info to reflect freed
		 * blocks.
	 	 */
		ip->i_nblocks -= nfreed;
		vcs_freedq(ip,nfreed,fperpage);

		/* make last page read-only if necessary.
		 */
		if (ip->i_seg)
			isetsize(ip, newsize);

		/* free blocks if working map only.
		 */
		if (type == V_WMAP)
			freedisk(ip,*anchor);
	}

	return 0;
}

/*
 * NAME:	lognodo(cd,daddr)
 *
 * FUNCTION:	writes no disk redo records into log for the disk
 *		block specified.
 *
 * PARAMETERS:	cd	- pointer to comdata structure.
 *		daddr   - disk block number.
 *			
 */

static 
lognodo(cd, daddr)
struct comdata *cd;
int	daddr;
{
	struct inode *ip;

	/* return if no log
	 */
	if (cd->ilog == 0)
		return 0;

	ip = cd->iptr[cd->current];
	cd->lr.type = NODREDO; /* no disk redo */
	cd->lr.length = 0;
	cd->lr.log.nodisk.inode = INDIR_I;
	cd->lr.log.nodisk.disk = daddr & (~NEWBIT);
	cd->lr.log.nodisk.volid = ip->i_dev;
	cd->lr.backchain = logmvc(cd->ilog, &cd->lr, (char *)NULL, 
		(struct comdata *)NULL); 

	return 0;
}

/*
 * NAME:	bclear (ip, offset, nbytes)
 *
 * FUNCTION:	On entry ip points to the inode table entry for
 * 		a regular file. offset specifies the byte offset in
 * 		the file of the first byte to zero and nbytes the
 * 		number of bytes to clear to zero.
 *
 *		exceptions can occur for permanent i/o error or
 *		no disk space (EIO or ENOSPC). caller is expected
 *		to have exception handler.
 *
 * PARAMETERS:	ip	- pointer to inode 
 *		offset	- offset of first byte to zero
 *		nbytes	- number of bytes to zero
 *
 * NOTE:	The range to clear must be wholly contained within
 *		a single page.
 *
 * RETURN :	0
 *			
 */

static 
bclear(ip, offset, nbytes)
struct inode *ip;
int offset;		
int nbytes;	
{
	int sr12save, rc, sid, sidx, fperpage;
	char *ptr;

	/* nothing to do ?
	 */
	if (nbytes <= 0)
		return 0;

	/* the clear operation must be wholly contained within
	 * a page.
	 */
	assert(((uint)offset >> L2PSIZE) ==
		 ((uint)(offset + nbytes - 1) >> L2PSIZE));

	/* get sidx, relative sid 
	 */
	sidx = STOI(ip->i_seg);
	sid = ITOS(sidx, (uint)offset >> L2PSIZE);

	/* if we are clearing the last page of the file and the page
	 * is partially backed by disk resources, use vcs_clrfrag()
	 * to clear the page.  otherwise, simply bzero() the page. 
	 * vcs_clrfrag() clears the partially back page while guaranteeing
	 * that a protection fault and a resulting full block allocation
	 * will not occur. (not for compression).
	 */
	if (ip->i_compress == 0)
	{
		fperpage = ip->i_ipmnt->i_fperpage;
		if (BTOPN(ip->i_size) == BTOPN(offset + nbytes) &&
	    	    BTOFR(ip->i_size,fperpage) != fperpage)
		{
			vcs_clrfrag(sid,offset,nbytes);
				return 0;
		}
	}

	sr12save = chgsr(12, SRVAL(sid,0,0));
	ptr = (char *) SR12ADDR + (offset & SOFFSET);
	bzero(ptr,nbytes);
	(void)chgsr(12, sr12save);

	return 0;
}

/*
 * NAME:	ipgrlse (ip, pfirst, plast)
 *
 * FUNCTION:	releases  pages in interval pfirst to plast.
 *		only the page frames are freed; disk blocks
 *		are not affected.
 *
 * PARAMETERS:	ip	- pointer to inode of file
 *		pfirst	- first page to release
 *		plast	- last page to release
 *
 * RETURN :	errors from subroutines
 *			
 */

static int
ipgrlse (ip, pfirst, plast)
struct inode *ip;
int pfirst;	/* first page */
int plast;	/* last page */
{
	int  npages;

	/* ok if plast is less than pfirst
	 */
	npages = plast - pfirst + 1;
	if (npages <= 0)
		return 0;

	return (vm_releasep(ip->i_seg,pfirst,npages));
}

/*
 * freedisk(ip,anchor)
 *
 * marks as free the disk fragment in the work map for the device
 * specified by ip and the fragment in the list pointed to by
 * anchor. xmfrees the storage representing the list.
 */

int
freedisk(ip,anchor)
struct inode * ip;
struct vmdlist * anchor;
{
	struct inode *ipdmap, *ipmnt;
	struct vmdlist *ptr, *nextptr;
	int mapsid, fperpage;

	/* get sid of disk map.
	 */
	ipmnt = ip->i_ipmnt;
	ipdmap = ipmnt->i_ipdmap;
	mapsid = ipdmap->i_seg;

	/* get fragments per page.
	 */
	fperpage = ipmnt->i_fperpage;

	/* free the disk blocks in work map (vcs_pbitfree
	 * accepts option V_WMAP to free in work map only)
	 */
	for(ptr = anchor; ptr; ptr = nextptr)
	{
		nextptr = ptr->next;
		ptr->fperpage = fperpage;
		vcs_pbitfree(mapsid,ptr,0,V_WMAP);
		xmfree((void *)ptr,kernel_heap);
	}

	/* mark map as dirty
	 */
	ipdmap->i_cflag |= DIRTY;

	return 0;
}

/*
 * freeoldfrags(cd,anchor)
 *
 * adds the specified inode's old fragments to the list
 * specified by anchor. also, writes a NODREDO record to
 * the log if the fragments are being freed from a journaled
 * object (directory).
 *
 */

freeoldfrags(cd,anchor)
struct comdata *cd;
struct vmdlist **anchor;
{
	struct inode *ip;
	struct vmdlist tlist;
	struct movedfrag * position;
	int journal, k;
	
	/* get inode pointer from comdata.
	 */
	ip = cd->iptr[cd->current];

	/* simply return if the inode has no old fragments.
	 */
	if (ip->i_movedfrag == NULL)
		return;

	/* add the old fragments to the list
	 */
	position = NULL;
	journal = ip->i_mode & IFJOURNAL;
	while(1)
	{
		vm_oldfrags(ip, &tlist, &position);
		for (k = 0; k < tlist.nblocks; k++)
		{
			dlistadd(anchor, tlist.da.dblock[k]);
			if (journal)
				lognodo(cd,tlist.da.dblock[k]);
		}
		if (position == NULL) break;
	}

	/* free the movedfrag structures 
	 */
	vcs_freefrags(ip);
	return 0;

}

/*
 * NAME:	findptr (ip, page)
 *
 * FUNCTION:	Returns pointer to the page in .indirect where 
 *		disk addr for page is stored. on entry .indirect is
 *		mapped into VM with SREG 12.
 *		
 * PARAMETERS:	ip	- inode whose indirect blocks we are interested in
 *		page	- page number
 *
 * RETURN :	Returns NULL if the page is NULL. it is assumed
 *		that an indirect block or doubly indirect block is
 *		associated with the segment.
 *		
 */
 
static uint *
findptr (ip, page)
struct inode *ip;
int page;
{
	struct idblock *idptr;
	int k, n;
 
	ASSERT(ip->i_vindirect);

	/* if single indirect inode has pointer to indirect block.
	 */
	if (SGLIND(ip->i_size))
		k = SR12ADDR + PAGESIZE * ip->i_vindirect;
	else
	{
		/* inode points to double indirect block.
		 */
		n =  SR12ADDR + PAGESIZE * ip->i_vindirect;
		idptr = (struct idblock *)n;		/* ptr to double */
		n = page/(PAGESIZE/4);			/* index in double */
		idptr = idptr + n;
		k = (idptr->id_raddr) ? SR12ADDR + PAGESIZE*idptr->id_vaddr : 0;
	}
	return (uint *) k;
}

/*
 * ifreeseg(ip)
 *
 * frees all uncommitted disk blocks, indirect blocks,
 * and the VM segment associated with the inode. rereads
 * the disk-inode.
 * 
 * return values 
 *		0 - ok
 *		EIO i/o error
 *
 * this procedure is only used to backout changes for deferred update 
 * segments (by jfs_close() and jfs_unmap()). 
 * however it can be used for other types. 
 * 
 * SERIALIZATION: the IWRITE_LOCK held on entry/exit.
 */

ifreeseg(ip)
struct inode *ip;
{
	int rc, sr12save;
	struct inode tip;
	label_t jbuf;

	/* if no segment just return.
	 */
	if (ip->i_seg == 0)
		return 0;
	
	/* set up for i/o exception.
	 */
	sr12save = mfsr(12);
	if (rc = setjmpx(&jbuf))
	{
		(void)chgsr(12,sr12save);
		return(rc);
	}

	/* read the old disk inode into a temporary
	 * buffer. 
	 */
	tip.i_dev = ip->i_dev;
	tip.i_number = ip->i_number;
	tip.i_ipmnt = ip->i_ipmnt;
	rc = iread(&tip);
	if (rc)
		goto out;
		
	/* free new disk blocks and free indirect blocks.
	 * ctrunc1() does the work if the file is unlinked;
	 * otherwise, ifreenew() is used.
	 */
	if (ip->i_cflag & CMNOLINK)
		ctrunc1(ip,0);
	else
	{
		/* If a compressed segment go ahead and release the 
		 * page frames and wait for any io to finish.  This 
		 * will ensure that LRU page replacement will not 
		 * cause reallocation to occur.   Since deferred
	         * update pages can be written to their home disk 
		 * location if they are new, and since compressed
		 * segments have realloction done during pageout,
		 * then we have to make this special effort.
		 */
        	if (ip->i_compress)
		{
			ipgrlse(ip, 0, MAXFSIZE/PSIZE - 1);
			vms_iowait(ip->i_seg);
		}
		ifreenew(ip,&tip);
	}
	
	/* restore the old disk inode.
	 */
	bcopy(&tip.i_dinode, &ip->i_dinode, DILENGTH);

	/* free the segment.
	 */
	isegdel(ip);

out:
	clrjmpx(&jbuf);
	return(rc);
}

/*
 * NAME:	ifreenew
 * 
 * FUNCTION: 	Frees all new (uncommitted) disk blocks associated with
 * 		a file.  Frees the indirect blocks of the file.  
 *
 */
int
ifreenew(struct inode *ip, struct inode *oldip)
{
	struct inode *ipind;
	union xptentry *xpt, old, new;
	struct idblock * idptr;
	struct vmdlist * anchor;
	int k, indblk, last, sr12save, nfreed, pno, fperpage;

	/* init variables
	 */
	anchor = NULL;
	fperpage = ip->i_ipmnt->i_fperpage;

	/* check for old committed fragments waiting to be freed.
	 * if they exist, they should be retained as part of
	 * the abandonment, but the movedfrag structures need to
	 * be freed.
	 */
	if (ip->i_movedfrag)
	{
		vcs_freefrags(ip);
	}
 
	/* if the file is small the inode has all of the info
	 * we need.
	 */
	if (NOIND(ip->i_size))
	{
		xpt = (union xptentry *) (&ip->i_rdaddr[0]);
		for (k = 0; k < NDADDR; k++, xpt++)
		{
			if (xpt->newbit)
			{
				dlistadd(&anchor, xpt->word);
			}
		}
		goto done;
	}

	/* map .indirect into virtual memory 
	 */
	ipind = ip->i_ipmnt->i_ipind;
	sr12save = chgsr(12, SRVAL(ipind->i_seg,0,0));

	/* is the indirect block pointed to by the inode new ?
	 */
	indblk = ip->i_vindirect;
	xpt = (union xptentry *) (&ip->i_rindirect);
	if (xpt->newbit)
		dlistadd(&anchor,xpt->word);

	/* is it a single indirect case ?
	 */
	if (SGLIND(ip->i_size))
	{
		freesingle(indblk,&anchor);
	}
	else
	{
		/* double indirect case. process loop in backwards direction.
	 	 */
		last = BTOPN(ip->i_size)/(PAGESIZE/4);
		idptr = (struct idblock *) (SR12ADDR + PAGESIZE*indblk);
		idptr = idptr + last;
		for (k = last; k >= 0; k-- , idptr--)
		{	
			/* if direct block is null continue
			 */
			if ((indblk = idptr->id_vaddr) == 0)
				continue;

			/* is the direct block new ?
			 */
			xpt = (union xptentry *) (&idptr->id_raddr);
			if (xpt->newbit)
				dlistadd(&anchor, xpt->word);

			/* process the direct block
			 */
			freesingle(indblk,&anchor);
		}
	}

	/* free the indirect page in .indirect.
	 */
	ifreeind(ip);
	(void)chgsr(12, sr12save);

	done:

	/* adjust i_nblocks and the quotas.
	 */
	nfreed = ip->i_nblocks - oldip->i_nblocks;
	ip->i_nblocks = oldip->i_nblocks;
	vcs_freedq(ip,nfreed,fperpage);

	/* free the resources from the disk map.
	 */
	freedisk(ip,anchor);

	return 0;
}


/*
 * NAME:	freesingle(indblk, anchor);
 *
 * FUNCTION:	add new disk blocks to list of blocks to free.
 *		
 * PARAMETERS:	
 *		indblk  - page number in .indirect of indirect block
 *		anchor   - addr of pointer to vmdlist
 *
 *	        on entry .indirect is mapped into VM with SREG 12.
 *
 * RETURN :	0 -ok
 *		
 */
static
freesingle(indblk,anchor)
int indblk;   
struct vmdlist ** anchor;
{
	union xptentry * xpt;
	int k;

	/* process all blocks.
	 */
	xpt = (union xptentry *) (SR12ADDR + indblk*PAGESIZE);

	for (k = 0; k < PAGESIZE/4; k++, xpt++)
	{
		if (xpt->newbit)
			dlistadd(anchor, xpt->word);
	}

	return 0;
}

/*
 * NAME:	iextend (ip, newsize, crp)
 *
 * FUNCTION:	extend inode size for ip to newsize.  extension is
 * 		performed by writing a null bytes to offset (newsize-1)
 *		within the file through a call to writei().
 *
 *		this routine is used by itrunc and iclear to 'trunc up'
 *		a regular file.
 *
 * PARAMETERS:	ip	- pointer to inode 
 *		newsize	- new file size.
 *		crp	- credential
 *
 * RETURN:	0	- file sucessfully extended.
 *
 *		E2BIG	- new size is beyond maximum file size
 *
 *		errnos returned by writei().
 *
 */

static 
iextend(ip,newsize,crp)
struct inode *ip;
int newsize;		
struct ucred *crp;
{
	struct iovec aiov;
	struct uio auio;
	int zero = 0;

	/* check if the new size is beyond the maximum file 
	 * size.
	 */
	if (newsize > MAXFSIZE)
		return(E2BIG);

	/* fill out the uio and iov structures for writei().
	 */
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = (caddr_t)&zero;
	aiov.iov_len = 1;
	auio.uio_resid = 1;
	auio.uio_offset = newsize - 1;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_fmode = FWRITE;

	/* extend the file.
	 */
	return(writei(ip,FWRITE,NULL,&auio,crp));
}
