static char sccsid[] = "@(#)95	1.68  src/bos/kernel/pfs/comsubs.c, syspfs, bos41J, 145887.bos 3/3/95 13:20:56";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: comlist,	comlist1, 	commit,
 *            commit1,	commit2,	commit3,
 *            initcom,	logafter,	lognewpage,
 *            lognoredo,	makeiblk, 	dosingle,
 *            dlistadd,	setbitmaps,   finicom
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
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
#include  "vmm/vmsys.h"
#include  "sys/errno.h"
#include  "sys/malloc.h"
#include  "sys/syspest.h"

/*
 * NAME:	commit (n, va_ilist)
 *
 * FUNCTION:	Common routine to build comlist arguments
 *		from a variable # of arguments
 *
 * PARAMETERS:	n	 - the number of inodes to commit
 *		va_ilist - list of inodes
 *
 * RETURN :	Errors from comlist1.
 *			
 */

commit (n, va_ilist)
int n;				/* Number of ips */
va_list va_ilist;
{
	return comlist (&n);
}
 
/*
 * NAME:	comlist (clist)
 *
 * FUNCTION:	Commits the changes to the segments specified in
 * 		clist. Both the data and the inode on disk are changed
 * 		if the IFSYNC flag is set; otherwise only the  disk
 * 		inode is changed. For journalled segments only the 
 * 		changes of the caller are committed, ie by tid.
 * 		for non-journalled segments the data are flushed to 
 *		disk and then the change to the disk inode and indirect 
 *		blocks committed (so blocks newly allocated to the
 *		segment will be made a part of the segment atomically).
 *
 *		on entry the inode lock on each segment is assumed
 *		to be held.
 *
 * 		The i_flags IACC and ICHG are cleared. if IFSYNC is
 * 		set then IFSYNC and IUPD are also cleared.
 *
 * 		all of the segments specified in clist must be in
 * 		one file system. no more than 6 segments are needed  
 * 		to handle all unix svcs.
 *
 *		if the i_nlink field (i.e. disk inode link count) 
 *		is zero, and the type of inode is a regular file or  
 *		directory, or symbolic link , the inode is truncated
 *		to zero length. the truncation is committed but the
 *		VM resources are unaffected until it is closed (see 
 *		iput and iclose).
 *
 *
 * PARAMETERS:	clist	- comlist structure pointer.  # inodes and array
 *			  containing that # of inodes to commit.
 *
 * RETURN :	0	ok
 *		EROFS	- filesystem is read-only.
 *			
 */

comlist(clist)
struct commit *clist;		/* Array of inodes to commit */
{
	int sr12save,sr13save,savevmsr,rc;
	label_t jbuf;           
	struct comdata com;
	struct inode *ip;
	int  k, level, npages, rc1 = 0;

	/* check for read-only file system.
	 */
	if (isreadonly(clist->iptr[0]))
		return EROFS;

	/* save sregs and make vmmdseg addressable
	 */
	sr12save = mfsr(12);
	sr13save = mfsr(13);
	savevmsr = chgsr(VMMSR,vmker.vmmsrval);

	/* set up for i/o error. if i/o error restore sregs, 
	 * cleanup, and return.
	 */
	if (rc = setjmpx(&jbuf))
	{
		comfail(&com,rc);
		(void)chgsr(12,sr12save);
		(void)chgsr(13,sr13save);
		(void)chgsr(VMMSR,savevmsr);
		return rc;
	}

	/* initialize comdata
	 */
	if (rc = initcom (&com, clist))
		goto closeout;
	
	/* update inode and indirect blocks. flush non-journalled
	 * segments.
	 */
	for (com.current = 0; com.current < com.number; com.current++)
	{	ip = com.iptr[com.current];

		/* if disk link count is zero we truncate it
		 * unless we did it before. ctrunc will handle types.
		 */
		if (ip->i_nlink == 0)
		{
			if (!(ip->i_cflag & CMNOLINK ))
				if (rc = ctrunc(&com))
					goto closeout;
			continue;
		}

		/* changes to disk inode only ?
		 */
		if ((ip->i_flag & IFSYNC) == 0 || ip->i_seg == 0) 
			continue;

		/* flush pages of regular file. this prevents
		 * the file getting non-initialized disk blocks
		 * in case of crash.
		 */
		if ((ip->i_mode & IFJOURNAL) == 0)
		{
		 	iflush(ip);		
			rc = vms_iowait(ip->i_seg);
			if (rc1 == 0)
				rc1 = rc;
		}

		/* update inode and indirect blocks.
		 */
		if (rc = commit1 (&com))
			goto closeout;
	}

	/* call common routine for completing commit.
	 */
	rc = finicom(&com);

	/* restore state and return
	 */
	closeout:
	clrjmpx(&jbuf);

	if (rc != 0)
		comfail(&com,rc);
	else
		rc = rc1;

	(void)chgsr(VMMSR,savevmsr);
	(void)chgsr(12, sr12save);
	(void)chgsr(13, sr13save);
	return(rc);
}

/*
 * finicom(cd)
 *
 * common routine for commit processing. this is used by comlist
 * as well as delpages to commit changes to one or more files.
 *
 * on entry the comdata structure has been initialize and all changes
 * to journalled files except .inodes have been made and the lists
 * of disk blocks to free or allocate computed.
 *
 */

int
finicom(cd)
struct comdata *cd;
{
	int k, level, rc, logrc = 0;
	struct inode *ip;
	struct vmdlist *ptr;
	struct loginfo *lp;
	void  setbitmaps();
	char * dptr;
	
	/* write disk free log records if any
	 */
	cd->lr.type = DFREE;
	cd->lr.log.dfree.inode = DISKMAP_I;      

	for(ptr = cd->freep; ptr; ptr = ptr->next)
	{
		cd->lr.length = 4*ptr->nblocks;
		cd->lr.log.dfree.nblocks = ptr->nblocks;
		dptr = (char *) &ptr->da.dblock[0];
		cd->lr.backchain = logmvc(cd->ilog, &cd->lr, dptr, cd);
	}

	for(ptr = cd->freepw; ptr; ptr = ptr->next)
	{
		cd->lr.length = 4*ptr->nblocks;
		cd->lr.log.dfree.nblocks = ptr->nblocks;
		dptr = (char *) &ptr->da.dblock[0];
		cd->lr.backchain = logmvc(cd->ilog, &cd->lr, dptr, cd);
	}

	/* Put .inodes and .indirect in list to get commit2 to
	 * write after records for them 
	 */
	cd->iptr[cd->number++] = cd->ipi;
	cd->iptr[cd->number++] = cd->ipind;

	/* write out all disk inodes except those added above.
	 */
	for(k = 0; k < cd->number - 2 ; k++)
	{
		iwrite (cd->iptr[k]);
	}


	/* Write all after records to log
	 */
	if (rc = commit2 (cd)) 
		return rc;

	/* Write commit record to log 
	 */
	cd->lr.type = COMMIT;

	/* Check if loginfo exists for the transaction.  If
	 * so, add it to the commit record.
	 */
	lp = (struct loginfo *)u.u_loginfo;
 
	if (lp == (struct loginfo *)NULL)
	{
		cd->lr.length = 0;
		level = logmvc(cd->ilog, &cd->lr, (char *)NULL, cd); 
	}
	else
	{
		assert(lp->li_len > 0 && lp->li_len <= MAXLINFOLEN);
		cd->lr.length = lp->li_len;
		level = logmvc(cd->ilog, &cd->lr, lp->li_buf, cd);
	}

	/* force log only if more than one file was in commit
	 * list or the IFSYNC flag was set.
	 */
	if (cd->number != 3 || cd->iptr[0]->i_flag & IFSYNC)
		logrc = groupcommit(cd);

	/* free hardware locks and schedule i/o
	 */
	if (rc = commit3 (cd))
		return rc;

	/* take .inodes and .indirect out of list.
	 */
	cd->number -= 2;  

	/* update permanent inode map. clear IACC and ICHG
	 * bits and clear IFSYNC and IUPD if IFSYNC was set.
	 * update permanent and work disk maps. free lock 
	 * on VM segment if any.
	 */
	setbitmaps(cd);

	/* if an I/O error occurred on the log, return before
	 * freeing the transaction block but after freeing lockwords
	 * and updating the allocation maps. the transaction block
	 * will be freed by comfail().
	 */
	if (logrc)
		return logrc;

	/* free the transaction block if no more lockwords.
	 */
	if (lanch.tblk[cd->tid].next <= 0)
		vcs_frtblk(cd->tid);

	return(0);
}

/* 
 * initcom(cd , clist)
 *
 * Initialize the commit argument structure, the comdata struct, as 
 * prelude to commit processing.
 */

int
initcom (cd, clist)
struct comdata *cd;
struct commit *clist;
{
	struct	inode *ip;
	int 	k, n, top, rc , lsidx;
	dev_t	dev;

	/* copy info in clist to comdata
	 */
	cd->current = 0;
	cd->number = clist->number;
	for (k = 0; k < cd->number; k++)
		cd->iptr[k] = clist->iptr[k];

	/* get inode table entries for log, .inodes, and .indirect
	 * disk and inode map. 
	 */
	dev = cd->iptr[0]->i_dev;
	ip = cd->iptr[0]->i_ipmnt;
	cd->ilog = ip->i_iplog;
	cd->ipi	 = ip->i_ipinode;
	cd->ipind = ip->i_ipind;
	cd->ipimap  = ip->i_ipinomap;
	cd->ipdmap  = ip->i_ipdmap;

	/* get a transaction block if it doesn't have one.
	 * initialize its age.
	 */
	if ((cd->tid = u.u_fstid) == 0)
	{
		if (rc = vcs_gettblk(&cd->tid))
		{
			cd->number = 0; /* for comfail */
			return rc;
		}
		lsidx = STOI(cd->ilog->i_seg);
		lanch.tblk[cd->tid].lsidx = lsidx;
		lanch.tblk[cd->tid].logage = scb_loglast(lsidx);
	} 

	/* initialize log descriptor in comdata.
	 */
	cd->lr.transid = lanch.tblk[cd->tid].logtid;
	cd->lr.backchain = 0;
 
	/* sort the iptr array by i_number (biggest first).
	 */
	for (k = 0; k < cd->number; k++)
	{	top = cd->iptr[k]->i_number;
		for ( n = k + 1; n < cd->number; n++)
		{	ip = cd->iptr[n];
			if (ip->i_number > top)
			{	top = ip->i_number;
				cd->iptr[n] = cd->iptr[k];
				cd->iptr[k] = ip;
			}
		}

		/* lock the VM segment if the IFSYNC flag is set
		 * to prevent addition of new disk blocks.
		 */
		ip = cd->iptr[k];
		if (ip->i_flag & IFSYNC && ip->i_seg)
			COMBIT_LOCK(ip);

	}

	/* use dev_t for volid
	 */
	cd->lr.log.aft.volid = cd->ipi->i_dev;

	/* initialize anchors for vmdlist.
	 */
	cd->freep = cd->freepw = cd->alloc = NULL; 

	return 0;
}
 
/*
 * NAME:	commit1 (cd)
 *
 * FUNCTION:	scans the inode and indirect blocks for new disk 
 *		blocks. changes them to old and puts them in 
 *		list to be allocated. generates new page log 
 *		records for new indirect blocks and for new disk
 *		blocks if the file is journalled.
 *
 *		file processed is the current one in comdata.
 *
 * PARAMETERS:	cd	- Pointer to commit data structure.
 *
 * RETURN :	Errors from subroutines.
 *			
 */

commit1 (cd)
struct comdata *cd;
{
	int sid , sidx , rc , k, inum, maxp, sr12save, nfound;
	int  nnew, last , journal, indblk;
	struct inode *ip;
	union xptentry  *xpt ;
	struct idblock * idptr;
	int	x; /* index of entry in transaction line */
	int	xlock; /* boolean of transaction lock for line */
	int	newiblk, newsiblk; /* brand new indirect block ? */
 
	/* see if there are any new pages for the inode by checking
	 * its scb
	 */
	ip = cd->iptr[cd->current];
	sid = ip->i_seg;
	sidx = STOI(sid);

	/* If no new disk blocks just return.
	 */
	if (scb_newdisk(sidx) == 0)
		return 0;

	/* lock vm segment to prevent addition of new blocks.
	 * establish some variables.
	 */
	journal = scb_jseg(sidx);
	maxp = BTOPN(ip->i_size);
	inum = ip->i_number;
	nnew = scb_newdisk(sidx);
	rc = nfound = 0;
	sr12save = mfsr(12);

	/* free the inode's old committed allocation.
	 */
	freeoldfrags(cd,&cd->freepw);

	/* if the file is small the inode has all of the info
	 * we need.
	 */
	if (maxp < NDADDR)
	{
		xpt = (union xptentry *) (&ip->i_rdaddr[0]);
		for (k = 0; k < NDADDR; k++, xpt++)
		{
			if (xpt->newbit)
			{
				newblk(cd,xpt,inum,k,journal);
				nfound += 1;
			}
		}
		goto done;
	}

	/* map .indirect into virtual memory with special bit
	 */
	(void)chgsr(12, SRVAL(cd->ipind->i_seg,0,1));

	/* is the indirect block pointed to by the inode new ?
	 */
	indblk = ip->i_vindirect;
	xpt = (union xptentry *) (&ip->i_rindirect);
	newiblk = FALSE;
	if (xpt->newbit) {
		newiblk = TRUE;
		newblk(cd,xpt,INDIR_I,indblk,1);
	}

	/* single indirect case.
	 */
	if (SGLIND(ip->i_size))
	{
		rc = dosingle(cd,indblk,0,&nfound, newiblk);
		goto done;
	}

	/* double indirect case. 
	 * scan backwards on the double indirect block entries 
	 * because new blocks are more likely to be appended to file.
	 */
	last = maxp/(PAGESIZE/4); /* index of the last entry in dbl ind blk */
	idptr = (struct idblock *) (SR12ADDR + PAGESIZE*indblk);
	idptr = idptr + last;

	x = last % (LINESIZE/sizeof(*idptr));
	xlock = FALSE;
	for (k = last; k >= 0; k-- , idptr--, x--)
	{	
		/* null single indirect block entry
		 */
		if ((indblk = idptr->id_vaddr) == 0)
			goto xline;

		/* is the single indirect block new ?
		 */
		newsiblk = FALSE;
		xpt = (union xptentry *) (&idptr->id_raddr);
		if (xpt->newbit) {
			newsiblk = TRUE;

			if (xlock == FALSE) {
				vm_gettlock(xpt, sizeof(*xpt));
				xlock = TRUE;
			}

			newblk(cd,xpt,INDIR_I,indblk,1);
		}

		/* if we have found all the new data blocks we don't have
		 * to process further single indirect block except for 
		 * the first one. the first one must be fixed up because
		 * it can have gotten a copy of all old disk blocks
		 * from the inode as index structure has changed.
		 */
		if (nfound == nnew && k > 0)
			goto xline;

		/* process the single indirect block
		 */
		if (rc = dosingle(cd, indblk, k*PAGESIZE/4, &nfound, newsiblk))
			break;	

		/* if this is the first single indirect block of the brand
		 * new double indirect block, 
	 	 * take a linelock of the first line of of the double indirect 
		 * block (i.e., take log) in case it might have inherited  
		 * the first single indirect block from inode when index 
		 * geometry has changed.
	 	 */
		if (k == 0 && newiblk)
			vm_gettlock(xpt, sizeof(*xpt));

xline:
		if (x == 0) {
			x = LINESIZE/sizeof(*idptr);
			xlock = FALSE;
		}
	} /* end for */

done:

	(void)chgsr(12, sr12save);
	return rc;
}

/*
 * NAME:	dosingle(cd, indblk, firstp, nfound, newiblk);
 *
 * FUNCTION:	called by commit1() 
 *		to process a single indirect block 
 *		for the current file in comdata.
 *		
 * PARAMETERS:	cd	- pointer to comdata strurcture
 *		indblk  - page number of single indirect block in .indirect
 *		firstp  - first page number covered by indblk in fs object.
 *		nfound	- set to number of new blocks found.
 *		newiblk - brand new indirect block ?
 *
 * RETURN :	0 -ok
 *		
 */
static
dosingle(cd, indblk, firstp, nfound, newiblk)
struct comdata *cd;
int	indblk;
int	firstp;
int	*nfound;
int	newiblk;
{
	struct inode *ip;
	union xptentry * xpt, * xpt0;
	int	k, n, maxp, journal , inum, found, last;
	int	x; /* index of entry in transaction line */
	int	xlock; /* boolean of transaction lock for line */

	/* establish some variables 
	 */
	ip = cd->iptr[cd->current];
	maxp = BTOPN(ip->i_size);
	inum = ip->i_number;
	journal = ip->i_mode & IFJOURNAL;
	found = 0;

	/* scan backwards on the single indirect block entries. 
	 * last is the page number relative to firstp.
	 */
	last = MIN(maxp - firstp, PAGESIZE/4 -1); /* index of the last entry  */
						  /* in single indirect block */
	xpt0 = (union xptentry *) (SR12ADDR + indblk*PAGESIZE);
	xpt = xpt0 + last;

	x = last % (LINESIZE/sizeof(*xpt));
	xlock = FALSE;
	for (k = last; k >= 0; k--, xpt--, x--)
	{
		if (xpt->newbit)
		{
			if (xlock == FALSE) {
				vm_gettlock(xpt, sizeof(*xpt));
				xlock = TRUE;
			}

			newblk(cd,xpt,inum,firstp + k,journal);
			found += 1;
		}

		if (x == 0) {
			x = LINESIZE/sizeof(*xpt);
			xlock = FALSE;
		}
	} /* end for */

	*nfound += found;

	/* if this is first single indirect block, copy info to inode 
	 * (it's for logredo() - RTFS).
	 * if this is the brand new first single indirect block, take 
	 * a linelock of the first line of of the single indirect block 
	 * (i.e., take log) in case it might have inherited the first 
	 * NDADDR disk blocks from inode when index geometry has 
	 * changed.
	 */
	if (firstp == 0)
	{
		xpt = xpt0;
		for (k = 0; k < NDADDR; k ++, xpt++)
		{
			ip->i_rdaddr[k] = xpt->word;
		}

		if (newiblk)
			vm_gettlock(xpt0, sizeof(*xpt0));
	}

	return 0;

}
 
/*
 * NAME:	newblk(cd, xpt, inum , pno , journal)
 *
 * FUNCTION:	called by commit1() and its dosingle() 
 *		to processes a new page for commit1(). 
 *		add disk block to allocate list. 
 *		write a newpage log record if type is journalled.	
 *
 * PARAMETERS:	cd	- pointer to comdata structure
 *		xpt	- pointer to external page table entry
 *		inum	- inode number of file
 *		page	- number of new page
 *		journal - 1 if file is journalled
 *
 * RETURN :	0 -ok
 *		
 */

static 
newblk(cd, xpt, inum , pno, journal)
struct comdata *cd;
union xptentry  *xpt;		
int	inum;		
int 	pno;	
int 	journal;
{
	int daddr;

	/* reset newbit
	 */
	xpt->newbit = 0;
	daddr = xpt->word;

	/* if journalled segment log new page record.
	 */
	if (journal)
		lognewpage (cd, inum, pno, daddr);
 
	/* add disk block to allocate disk list
	 */
	dlistadd(&cd->alloc,daddr);

	return 0;
}

/*
 * NAME:	commit2 (cd)
 *
 * FUNCTION:	Writes to log the after records for all lines modified
 * 		by tid for segments specified by inodes in comdata.
 *		Code assumes only WRITELOCKS are recorded in lockwords.
 *
 * PARAMETERS:	cd	- pointer to comdata struct
 *
 * RETURN :	
 *			
 */

commit2 (cd)
struct comdata *cd; 
{
 
	struct inode *ip;
	int t, k, n, rc, first, last, sr12save, daddr;
 
	rc = 0;
	t = cd->tid;
	cd->lr.type = AFTER;
	sr12save = mfsr(12);

	/* compare lock words of this transaction for match
	 * with files being committed. on match build after
	 * records and move to log.
	 */
	for (k = lanch.tblk[t].next; k > 0; k = lword[k].tidnxt)
	{
		/* is lockword for a file being committed ?
		 */
		ip = NULL;
		for (n = 0; n < cd->number; n++)
			if (BASESID(lword[k].sid) == cd->iptr[n]->i_seg)
			{
				ip = cd->iptr[n];
				break;
			}

		/* ip == NULL means commit list was not complete.
		 */
		if (ip == NULL) 
			continue;
 
		/* mark inode as dirty
		 */
		ip->i_cflag |= DIRTY;

		/* lockword should only be processed one time.
		 * mark the lockword to be freed.
		 */
		ASSERT((lword[k].flag & FREELOCK) == 0);
		lword[k].flag |= FREELOCK;

		/* get disk address of page from lockword
		 */
		daddr = lword[k].home;
 
		/* map the page into VM
		 */
		(void)chgsr(12,SRVAL(lword[k].sid,0,1));
 
		/* fill in  disk inode in log record
		 */
		cd->lr.log.aft.inode = ip->i_number;

		/* if its a page in .indirect, distinguish between
		 * single and double indirect blocks.
		 */
		if (ip->i_number == INDIR_I)
			cd->lr.log.aft.indtype = getindtype(cd,daddr);

		/* write the after records.
		 */
		if (rc = logafter(cd, k, daddr))
			break; 
 
	}
 
	(void)chgsr(12,sr12save);
	return rc;
}

/*
 * NAME:	getindtype (cd, daddr)                 
 *
 * FUNCTION:	returns SINGLEIND if daddr is single indirect block 
 *		and DOUBLEIND if it is a double indirect block.
 *
 * PARAMETERS:	cd	- pointer comdata sturcture
 *		daddr	- home disk address of page
 *
 */
static 
getindtype (cd, daddr)                 
struct comdata *cd;
int daddr;		
{
	int k,small;
	struct inode *ip;

	/* daddr is an indirect block for one of the segments
	 * being committed. if it is directly pointed to by
	 * one of the inodes, we determine its type on the
	 * basis of its size. otherwise it is not directly
	 * pointed to by an inode so it must be single indirect.
	 */

	for (k = 0; k < cd->number; k++)
	{
		ip = cd->iptr[k];
		if (ip->i_rindirect == daddr)
		{
			small = (ip->i_size <= (PSIZE/4)*PSIZE);
			return ( (small) ? SINGLEIND : DOUBLEIND);
		}
	}

	return(SINGLEIND);
}
/*
 * NAME:	logafter (cd, lw, daddr)                 
 *
 * FUNCTION:	Writes after record to log for modified lines in lockword
 * 		lw. On entry segment is mapped into VM using SREG 12 and the
 * 		log record descriptor fields volid,inode, tid, and type are 
 * 		filled in.
 *
 * PARAMETERS:	cd	- pointer comdata sturcture
 *		lw	- current lockword representing page
 *		daddr	- home disk address of page
 *
 * RETURN :	0
 *			
 */

static 
logafter (cd, lw, daddr)                 
struct comdata *cd;
int lw;					/* index of lockword */
int daddr;				/* home disk address of page */
{
	int 	first, last, n, offset, ptr, shift;
	ulong  	modbits;
 
	modbits = lword[lw].bits;

	/* for NLOCKBITS = 16 the bits are stored in bits 16-31
	 */
	if (NLOCKBITS == 16)
		modbits = modbits << 16;

	for (first = clz32(modbits); first < NLOCKBITS; first = clz32(modbits))
	{
		/* 
		 * Determine last of sequence of modified lines.  The next
		 * zero bit is the end of sequence.
		 */      
		shift = clz32(~(modbits << first)) + first;
		last = shift - 1;

		/* Remove known bits */
		modbits = (modbits << shift) >> shift;

		/* Compute record length, addresses, and write log record.  
		 * "ptr" is relative offset since sr12 is loaded with 
		 * relative sid.  "psaddr" must be full offset since 
		 * logredo uses this to compute inode number.
		 */
		n = last - first + 1;  
		cd->lr.length = n * LINESIZE;
		offset = first * LINESIZE;
		cd->lr.log.aft.disk = daddr;

		cd->lr.log.aft.psaddr = PAGESIZE * 
				SCBPNO(lword[lw].sid, lword[lw].page) + offset;
		ptr = SR12ADDR + PAGESIZE * lword[lw].page + offset;
		cd->lr.backchain = logmvc(cd->ilog,&cd->lr,ptr,cd);
	}
	return 0;
}


/*
 * NAME:	commit3(cd)
 *
 * FUNCTION:	Initiates pageout of pages modified by tid in journalled
 *		segments in comdata and frees their lockwords.
 *
 * PARAMETERS:	cd	- pointer to comdata structure
 *
 * RETURN :	Errors from subroutines.
 *			
 */

commit3(cd)
struct comdata *cd;
{
	struct  inode *ip;
	int 	t, k, rc, sid, sidind, siddefer, force;
 
	/* Scan inodes to be committed for deferred update object
	 * which is only committed via fsync().   If deferred update 
	 * object indirect blocks are not forced out at commit time, 
	 * and if they are updated subsequently, and if the thread 
	 * working on the deferred update object takes a long sleep, 
	 * the page will block subsequent log forwarding.  
	 */
        siddefer = 0;
        for (k = 0; k < cd->number - 2; k++) 
	{
                ip = cd->iptr[k];
                if (ip->i_flag & IDEFER)
                        siddefer = 1;
        }

	t = cd->tid;
	sidind = cd->ipind->i_seg;
	for (k = lanch.tblk[t].next; k > 0; k = lword[k].tidnxt) 
	{
		/* skip lockword if it wasn't processed by commit2
		 */
		if ((lword[k].flag & FREELOCK) == 0)
			continue;

                /* Initiate i/o if page is in .indirect for deferred update
                 * object.  Otherwise leave it in memory (with pft_homeok set).
		 * These pages get written during v_sync() or v_purge().
                 */
                sid = lword[k].sid;
                if (siddefer == 1 && sid == sidind)
                        force = 1;
                else
                        force = 0;
		rc = vcs_write(BASESID(sid), SCBPNO(sid,lword[k].page),1,force);
		if (rc)
			return rc;
	}
 
	/* free the locks
	 */
	vcs_frlock(t);

	return 0;
}
 
 
/*
 * NAME:	lognewpage(cd, ino, page, daddr)
 *
 * FUNCTION:	Write new page record to log.
 *
 * PARAMETERS:	cd	- pointer to comdata structure
 *		ino	- inode number
 *		page	- page # in segment
 *		daddr	- home disk addr of page
 *
 * RETURN :	Always zero
 *			
 */

static 
lognewpage(cd, ino, page, daddr)
struct comdata *cd; 
int ino;				/* disk inode number */
int page;				/* page in segment */
int daddr;				/* its home disk address */ 
{
	/* write newpage log record */
	cd->lr.type = NEWPAGE;
	cd->lr.length = 0;
	cd->lr.log.new.inode = ino;      
	cd->lr.log.new.disk = daddr;
	cd->lr.log.new.psaddr = PAGESIZE*page;   
	cd->lr.backchain = logmvc(cd->ilog, &cd->lr, (char *)NULL, cd);
	return 0;
}

/*
 * NAME:	lognoredo (cd, ino)
 *
 * FUNCTION:	Write noredo log record for ino
 *
 * PARAMETERS:	cd	- pointer to comdata structure
 *		ino	- inode number
 *
 * RETURN :	Always zero
 *			
 */

lognoredo (cd, ino)
struct comdata *cd; 		/* Standard argument	*/
int ino;			/* disk inode number	*/
{
	/* write noredo for ino log record
	 */
	cd->lr.type = NOREDO;
	cd->lr.length = 0;
	cd->lr.log.nodo.inode = ino;      
	cd->lr.backchain = logmvc(cd->ilog, &cd->lr, (char *)NULL, cd);
	return 0;
}
 
/*
 * setbitmaps(cd)
 *
 * processes the disklists anchored in comdata structure.
 * and updates the permanent inode map. also resets IACC
 * and ICHG bits, and if IFSYNC was set clears IFSYNC and
 * IUPD.
 *
 * alloc list : the disk blocks are marked as allocated in
 * the permanent map.
 *
 * free list : the disk blocks are marked as free in the 
 * permanent map or in both permanent and work maps.
 *
 * newly committed inodes are marked as allocated and inodes
 * with zero disk link count are marked as free in the permanent
 * inode map.
 *
 * INPUT PARAMETERS:
 *
 * (1) cd - pointer to comdata structure.
 *
 * Return Values - none
 *
 */

static void
setbitmaps(cd)
struct comdata * cd;
{
	int logage, k, rc, inosid, mapsid, fperpage;
	struct vmdlist *ptr, *nextptr, inlist;
	struct inode *ip;

	/* initialize some variables.
	 */
	logage = lanch.tblk[cd->tid].logage;
	mapsid = cd->ipdmap->i_seg;
	inosid = cd->ipimap->i_seg;
	fperpage = cd->ipdmap->i_ipmnt->i_fperpage;

	/* update permanent inode map if necessary.
	 * clear bits in in-core inode table.
	 */
	for (k = 0; k < cd->number; k++)
	{	ip = cd->iptr[k];
		if (ip->i_cflag & CMNEW)
		{
			ip->i_cflag &= ~CMNEW;
			inlist.next = NULL;
			inlist.nblocks = 1; 
			inlist.fperpage = 1; 
			inlist.da.dblock[0] = ip->i_number;
			vcs_pbitalloc(inosid,&inlist,logage);
			cd->ipimap->i_cflag |= DIRTY;
		}
		else
		if (ip->i_nlink == 0 && !(ip->i_cflag & CMNOLINK))
		{
			ip->i_cflag |= CMNOLINK;
			inlist.next = NULL;
			inlist.nblocks = 1; 
			inlist.fperpage = 1; 
			inlist.da.dblock[0] = ip->i_number;
			vcs_pbitfree(inosid,&inlist,logage,V_PMAP);
			cd->ipimap->i_cflag |= DIRTY;
		}

		/* free VM segment lock
		 */
		if (ip->i_flag & IFSYNC && ip->i_seg)
			COMBIT_UNLOCKD(ip);
			
		ip->i_flag &= (ip->i_flag & IFSYNC) ? ~(IFSYNC|IUPD|IACC|ICHG)
						    : ~(IACC|ICHG);
	}	

	/* allocate disk blocks in permanent map.
	 */
	for (ptr = cd->alloc; ptr ; ptr = nextptr)
	{
		nextptr = ptr->next;
		cd->ipdmap->i_cflag |= DIRTY;
		ptr->fperpage = fperpage;
		vcs_pbitalloc(mapsid,ptr,logage);
		xmfree((void *)ptr,kernel_heap);
		cd->alloc = nextptr;
	}

	/* free blocks in permanent map 
	 */
	for (ptr = cd->freep; ptr ; ptr = nextptr)
	{
		nextptr = ptr->next;
		cd->ipdmap->i_cflag |= DIRTY;
		ptr->fperpage = fperpage;
		vcs_pbitfree(mapsid,ptr,logage,V_PMAP);
		xmfree((void *)ptr,kernel_heap);
		cd->freep = nextptr;
	}

	/* free blocks in both permanent and work map 
	 */
	for (ptr = cd->freepw; ptr ; ptr = nextptr)
	{
		nextptr = ptr->next;
		cd->ipdmap->i_cflag |= DIRTY;
		ptr->fperpage = fperpage;
		vcs_pbitfree(mapsid,ptr,logage,V_PWMAP);
		xmfree((void *)ptr,kernel_heap);
		cd->freepw = nextptr;
	}
}

/*
 * dlistadd(anchor,daddr) 
 *
 * adds daddr to list of disk blocks to allocate/free in a map.
 * storage for the vmdlist structures used to hold the list is 
 * allocated from the kernel_heap as needed.
 *
 * Return values - none
 */

int 
dlistadd(anchor,daddr)
struct vmdlist ** anchor;
uint daddr;	/* disk block number */
{
	int k;
	struct vmdlist *ptr;

	ptr = *anchor;
	if (ptr == NULL || ptr->nblocks == NDLIST)
	{
		ptr = xmalloc(sizeof(struct vmdlist),0,kernel_heap);
		assert (ptr != NULL);
		ptr->next = *anchor;
		ptr->nblocks = 0;
		*anchor = ptr;
	}

	k = ptr->nblocks;
	ptr->da.dblock[k] = daddr & (~NEWBIT);
	ptr->nblocks += 1;

	return 0;
			
} 

/*
 *	comfail(cd)
 *
 * called when commit fails.
 *
 * frees hardware line-locks and segment locks for all
 * segments in comdata structure. frees malloc storage
 * sets state of file-system to FM_MDIRTY in super-block.
 * log age of page-frames in memory for which caller has 
 * are reset to 0 (to avoid logwarap).
 *
 * on entry VMM segment is assumed to be addressable.
 */

comfail(cd,exval)
struct comdata * cd;
int exval;
{
	int rc, k, t,n;
	struct inode *ip, *ipmnt;
	struct vmdlist *ptr ,*nextptr ;
	label_t jb;

	assert(exval == EIO || exval == ENOMEM);

	/* if there are no segments just return
	 */
	if (cd->number == 0)
		return 0;

	/* free lock-words of transaction that correspond to
	 * files being committed.
	 */
	t = cd->tid;
	for (k = lanch.tblk[t].next; k > 0; k = lword[k].tidnxt)
	{
		/* is lockword for a file being committed ?
		 */
		ip = NULL;
		for (n = 0; n < cd->number; n++)
			if (BASESID(lword[k].sid) == cd->iptr[n]->i_seg)
			{
				ip = cd->iptr[n];
				break;
			}

		/* ip == NULL means commit list was not complete.
		 */
		if (ip == NULL) 
			continue;
		lword[k].flag |= FREELOCK;

		/* reset logage of page if its in memory.
		 */
		vcs_setlog(lword[k].sid, lword[k].page, 0, 1);
	}

	/* free the lockwords
	 * free the transaction block if no more lockwords.
	 */
	vcs_frlock(t);
	if (lanch.tblk[t].next <= 0)
		vcs_frtblk(t);

	/* free the segment locks.
	 */
	for (n = 0; n < cd->number; n++)
	{
		ip = cd->iptr[n];
		if (ip->i_flag & IFSYNC && ip->i_seg)
			COMBIT_UNLOCKD(ip);
	}

	/* free malloced storage
	 */
	for (ptr = cd->alloc; ptr ; ptr = nextptr)
	{
		nextptr = ptr->next;
		xmfree((void *)ptr,kernel_heap);
	}

	for (ptr = cd->freep; ptr ; ptr = nextptr)
	{
		nextptr = ptr->next;
		xmfree((void *)ptr,kernel_heap);
	}

	for (ptr = cd->freepw; ptr ; ptr = nextptr)
	{
		nextptr = ptr->next;
		xmfree((void *)ptr,kernel_heap);
	}
	cd->alloc = cd->freep = cd->freepw = NULL;

	/* mark super block as mounted dirty
	 */
	rc = dirtysblock(cd->iptr[0]->i_ipmnt);

	return rc;
}


/*
 *	transactionfail(ipmnt, dirty)
 *
 * Called when a directory modifcation fails.
 *
 * frees hardware line-locks and segment locks for all
 * segments in comdata structure. frees malloc storage
 * sets state of file-system to FM_MDIRTY in super-block.
 * log age of page-frames in memory for which caller has 
 * are reset to 0 (to avoid logwarap).
 */

transactionfail(ipmnt, dirty)
struct inode *ipmnt;
int	dirty;
{
	int rc, k, t;
	struct superblock *sb;
	int savevmsr;

	/* if no transaction started, nothing to clean up
	 */
	if ((t = u.u_fstid) == 0)
		return 0;

	savevmsr = chgsr(VMMSR, vmker.vmmsrval);

	/* mark lockwords of transaction to be freed
	 */
	for (k = lanch.tblk[t].next; k > 0; k = lword[k].tidnxt)
	{
		lword[k].flag |= FREELOCK;

		/* reset logage of page if its in memory.
		 */
		vcs_setlog(lword[k].sid, lword[k].page, 0, dirty);
	}

	/* free the lockwords and transaction block
	 */
	vcs_frlock(t);
	assert(lanch.tblk[t].next <= 0);
	vcs_frtblk(t);

	(void)chgsr(VMMSR, savevmsr);

	/* mark superblock as dirty
	 */
	if (dirty)
		rc = dirtysblock(ipmnt);

	return rc;
}

/* 
 *	dirtysblock(ipmnt)
 *
 * Mark the super block as mounted dirty.
 */
int
dirtysblock(ipmnt)
struct inode *ipmnt;
{
	int rc;
	label_t jb;
	struct inode *ip;
	struct superblock *sb;

	ip = ipmnt->i_ipsuper;
	if (rc = iptovaddr(ip, 0, &sb))
		return rc;

	IWRITE_LOCK(ip);

	if (rc = setjmpx(&jb))
		goto out;

	/* mark super-block as mounted dirty.
	 */
	sb->s_fmod = FM_MDIRTY;
	iflush(ip);
	rc = vms_iowait(ip->i_seg);

	clrjmpx(&jb);

out:
	IWRITE_UNLOCK(ip);
	ipundo(sb);
	return rc;
}
