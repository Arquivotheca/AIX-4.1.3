static char sccsid [] = "@(#)72 1.1  src/bos/sbin/helpers/v3fshelpers/libfs/inode.c, cmdfs, bos411, 9428A410j 2/2/94 14:12:27";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: rwinode, rwdaddr, fsmax
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <libfs/libfs.h>

/*
 *  buffers for double indirect blocks, single indirect blocks,
 *  and inode blocks
 */
static struct idblock	DblIndirBuf[INDIRPERBLK];
static int		DblIndirBlk = -1;

static frag_t 		SglIndirBuf[DADDRPERBLK];
static int		SglIndirBlk = -1;

static struct dinode	InoBuf[INOPERBLK];
static int 		InoBlk = -1;


static int	InodeAgsize;	/* inode allocation group size	*/
static int	DiskAgsize;	/* disk allocation group size	*/
static int	NumDevBlks;	/* size of fs in 512 byte blks	*/

/*
 *  set_inovars(dag, iag, devblks)
 *	dag	: disk allocation group size
 *	iag	: inode allocation group size
 *	devblks	: size of fs in 512 byte blocks
 *
 *  FUNCTION
 *	initialize inode constants (read in from superblock)
 *	This function called from validate_super after dag and iag
 *	have been validated.
 *
 */
void
set_inovars(int dag,
	    int iag,
	    int devblks)
{
	DiskAgsize  = dag;	
	InodeAgsize = iag;
	NumDevBlks  = devblks;
	InoBlk = SglIndirBlk = DblIndirBlk = -1;
}
	
 
/*
 *
 *  rwinode(fd, di, inum, mode)
 *	fd:	device file descriptor
 *	di:	inode struct to read/write
 *	inum:	number of inode to read/write
 *	mode:	are we reading or writing
 *
 *  FUNCTION
 *	Read or write an inode.
 *
 *  RETURN VALUES
 *	success: 0
 *	failure: negative number (any of the rwfrag errors)
 */
int
rwinode (int		fd,	
	 struct dinode 	*di,	
	 ino_t 		inum,	
	 int		mode)	
{
	int	ag;			/* alloc group inode is in	*/
	int	iblk;			/* 4k block inode is in		*/
	int	rc;
	frag_t	ifrag = {0,0,0};
	
	if (!FragSize)
		return LIBFS_INTERNAL;
	/*
	 *  where does the inode live?
	 */
	ag = inum / InodeAgsize;
	iblk = (ag ? FRAG2BLK(ag * DiskAgsize) : INODES_B) +
		INO2BLK(inum % InodeAgsize);
	ifrag.addr = BLK2FRAG(iblk);

	/*
	 *  is the inode already in our inode buffer?
	 */
	if (iblk != InoBlk)
	{
		InoBlk = -1;
		if ((rc = bread(fd, InoBuf, ifrag)) != BLKSIZE)
			return LIBFS_READFAIL;
		InoBlk = iblk;
	}
	
	switch (mode)
	{
	case GET:
		*di = InoBuf[INOINDEX(inum)];
		break;
	case PUT:
		InoBuf[INOINDEX(inum)] = *di;
		if ((rc = bwrite(fd, InoBuf, ifrag)) != BLKSIZE)
			return LIBFS_WRITEFAIL;
		break;
	default:
		return LIBFS_INTERNAL;
	}
        return LIBFS_SUCCESS;
}


/*
 *  rwdaddr(fd, frag, di, lbno, mode)
 *	fd: 	device file descriptor
 *	frag:	where we put the frag_t that corresponds to <lbno>
 *	di:	disk inode to get <frag> from
 *	lbno:	logical block number
 *	mode:	GET or PUT (read/write frag from/to inode)
 *
 *  FUNCTION
 *	read/write frag from/to an inode
 *
 *  RETURN VALUES
 *	success: 0
 *	failure: negative number (LIBFS_SEEKFAIL, LIBFS_READFAIL,
 *		 LIBFS_INTERNAL, LIBFS_BADFRAG)
 */
int
rwdaddr(int		fd,
        frag_t		*frag,	
        struct dinode	*di,	
        int		lbno,
	int		mode)
{
	int 		numdadr;	/* number of daddrs in inode */
	fdaddr_t	daddr;	
	int 		rc;	

	if (!FragSize)
		return LIBFS_INTERNAL;
	/*
	 *  get number of disk addresses in file & check against
	 *  requested logical block number.
	 *  	come up with better error code...
	 */
	numdadr = NUMDADDRS(*di);
	if (lbno >= numdadr || lbno < 0)
		return LIBFS_SEEKFAIL;

	if (NOINDIRECT(numdadr)) 
	{
		switch(mode)
		{
		case GET:
			daddr.d = di->di_rdaddr[lbno];
			*frag = daddr.f;
			break;
		case PUT:
			daddr.f = *frag;
			di->di_rdaddr[lbno] = daddr.d;
			break;
		default:
			return LIBFS_INTERNAL;
		}
		return LIBFS_SUCCESS;
	}

	/*
	 *  di_rindirect is either the single or double indirect block
	 *  if double,
	 *	get the single indir block number
	 *  read the single indirect block to get the disk address
	 *
	 *  When reading into the global single and double indirect block
	 *  buffers, invalidate the global buffer block numbers before doing
	 *  the bread.  (housekeeping in case bread bombs)
	 */
	daddr.d = di->di_rindirect;
	if (ISDOUBLEIND(numdadr))
	{
		if (daddr.d != DblIndirBlk)
		{
			DblIndirBlk = -1;
			if ((rc = bread(fd, DblIndirBuf, daddr.f)) < 0)
				return rc;
			DblIndirBlk = daddr.d;
		}
		daddr.d = DblIndirBuf[DIDNDX(lbno)].id_raddr;
	}
	if (daddr.d != SglIndirBlk)
	{
		SglIndirBlk = -1;
		if ((rc = bread(fd, SglIndirBuf, daddr.f)) < 0)
			return rc;
		SglIndirBlk = daddr.d;
	}
	switch(mode)
	{
	case GET:
		*frag = SglIndirBuf[SIDNDX(lbno)];
		break;
	case PUT:
		SglIndirBuf[SIDNDX(lbno)] = *frag;
		if ((rc = bwrite(fd, SglIndirBuf, daddr.f)) != BLKSIZE)
			return rc;
		break;
	default:
		return LIBFS_INTERNAL;
	}
	return LIBFS_SUCCESS;
}


/*
 *  fsmax(ino_t *imax, frag_t *fmax)
 *	imax:	ptr to max inode number
 *	fmax:	ptr to max frag_t address
 *
 *  FUNCTION
 *	Return the first invalid inode number and first invalid frag
 *
 *  RETURN VALUES
 *	-1 if fails, 0 if succeeds
 */
int
fsmax(ino_t *imax,
      frag_t *fmax)
{
	int	nag;
	int	inofrags, extra;

	if (!FragSize)
		return LIBFS_INTERNAL;
	/*
	 * convert from 512-byte blocks to bytes to frags
	 */
	fmax->new = fmax->nfrags = 0;
	fmax->addr = DEVBLK2FRAG(NumDevBlks);

	/*
	 *  to get number of ag's, divide number of frags by disk agsize
	 *  IF  numfrags not evenly divisible by disk agsize
	 *      AND
	 *      enough frags are left over to hold the ag's inodes
         *  THEN
         *      add 1 to the number of ag's
	 */
	nag = fmax->addr / DiskAgsize;
	if (fmax->addr % DiskAgsize >= INO2FRAG(InodeAgsize))
		nag++;
	
	*imax = nag * InodeAgsize;
	return LIBFS_SUCCESS;
}
