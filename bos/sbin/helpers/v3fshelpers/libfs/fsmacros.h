/* static char sccsid [] = "@(#)79 1.8  src/bos/sbin/helpers/v3fshelpers/libfs/fsmacros.h, cmdfs, bos411, 9428A410j 2/21/94 17:26:39"; */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS:
 *	FRAG2BLK, BLK2FRAG, FRAG2BYTE, BYTE2FRAG, BLK2BYTE, BYTE2BLK,
 *	BYTE2DEVBLK, DEVBLK2BYTE, BLK2DEVBLK, DEVBLK2BLK, FRAG2DEVBLK,
 *	DEVBLK2FRAG, INO2BYTE, BYTE2INO, INO2BLK, BLK2INO, INO2FRAG,
 *	FRAG2INO, INOINDEX, NUMADDRS, FRAGLEN, FRAGSIN, NOINDIRECT,
 *	ISDOUBLEIND, DIDNDX, SIDNDX
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

#ifndef _H_FSMACROS
#define _H_FSMACROS

/*
 *  get definitions for:
 *	DEV_BSIZE	sys/dir.h 
 *	BSIZE 		jfs/fsparam.h
 *	NDADDR		jfs/fsdefs.h
 */
#include <sys/dir.h>
#include <jfs/fsparam.h>
#include <jfs/fsdefs.h>

/*
 *  file system constants with masks and shifts for bitwise ops
 */
#define BLKSIZE		BSIZE
#define BLKSHIFT	12			/* log2(4096)	*/
#define BLKMASK		(BLKSIZE - 1)

#define DEVBLKSIZE	DEV_BSIZE
#define DEVBLKSHIFT	9			/* log2(512)	*/
#define DEVBLKMASK	(DEVBLKSIZE - 1)


/*
 *  if sizeof(struct dinode), sizeof(daddr_t), sizeof(struct idblock)
 *	ever change from 128, 4, and 8, these macros will have to
 *	change too.  If the sizes change to numbers that aren't powers
 *	of 2, then these will have to be rewritten without bit shifting
 */
#define DINODESHIFT	7		/* log2(sizeof(struct dinode)) */
#define INOPERBLK	(BLKSIZE >> DINODESHIFT) 	
#define INOPERBLKSHIFT	(BLKSHIFT - DINODESHIFT)			
#define INOPERBLKMASK	(INOPERBLK - 1)				

#define DADDRTSHIFT		2   	/* log2(sizeof(daddr_t)) */
#define DADDRPERBLK		(BLKSIZE >> DADDRTSHIFT)
#define DADDRPERBLKSHIFT	(BLKSHIFT - DADDRTSHIFT)
#define DADDRPERBLKMASK		(DADDRPERBLK - 1)

#define IDBLOCKSHIFT		3	/* log2(sizeof(struct idblock) */
#define INDIRPERBLK		(BLKSIZE >> IDBLOCKSHIFT)
#define INDIRPERBLKSHIFT	(BLKSHIFT - IDBLOCKSHIFT)
#define INDIRPERBLKMASK		(INDIRPERBLK - 1)

#ifdef _IN_SUPER_C
uint FragSize	= 0;
uint FragShift	= 0;
uint FragMask	= 0;

uint FragPerBlk		= 0;
uint FragPerBlkShift	= 0;
uint FragPerBlkMask	= 0;
#else
extern uint FragSize;
extern uint FragShift;
extern uint FragMask;

extern uint FragPerBlk;
extern uint FragPerBlkShift;
extern uint FragPerBlkMask;
#endif



/*
 *  FRAG2BLK(fno)
 *	fno:	fragment number (frag.addr)
 *
 *  FUNCTION:
 *	return the full block number that contains frag <fno>
 */
#define FRAG2BLK(fno)	((fno) >> FragPerBlkShift)

/*
 *  BLK2FRAG(bno)
 *	bno:    full block number
 *
 *  FUNCTION
 *	return the number of the first frag in block <bno>
 */
#define BLK2FRAG(bno)	((bno) << FragPerBlkShift)



/*
 *  FRAG2BYTE(fno)
 *	fno:	fragment number (frag.addr)
 *
 *  FUNCTION
 *	return the byte offset of frag number <fno>
 */
#define FRAG2BYTE(fno)	((long long)(fno) << FragShift)

/*
 *  BYTE2FRAG(off)
 *	off:	byte offset 
 *
 *  FUNCTION
 *	return the frag number that contains byte offset <off>
 *		
 */
#define BYTE2FRAG(off)	((off) >> FragShift)



/*
 *  BLK2BYTE(bno)
 *	bno:	full block number
 *
 *  FUNCTION
 *	return the byte offset of full block <bno>
 */
#define BLK2BYTE(bno)	((long long)(bno) << BLKSHIFT)

/*
 *  BYTE2BLK(off)
 *	bno:	byte offset
 *
 *  FUNCTION
 *	return the block number that contains byte offset <off>
 */
#define BYTE2BLK(off)	((off) >> BLKSHIFT)



/*
 *  BYTE2DEVBLK(off)
 *	off:	byte offset
 *
 *  FUNCTION
 *	return the device block number that contains byte offset <off>
 */
#define BYTE2DEVBLK(off)	((off) >> DEVBLKSHIFT)

/*
 *  DEVBLK2BYTE(dbno)
 *	dbno:	device block number
 *
 *  FUNCTION
 *	return the byte offset of device block <dbno>
 */
#define DEVBLK2BYTE(dbno)	((long long)(dbno) << DEVBLKSHIFT)



/*
 *  BLK2DEVBLK(bno)
 *	bno:	full block number
 *
 *  FUNCTION
 *	return the first device block number in full block <bno>
 */
#define BLK2DEVBLK(bno) ((bno) << (BLKSHIFT - DEVBLKSHIFT))

/*
 *  DEVBLK2BLK(dbno)
 *	dbno:	device block number
 *
 *  FUNCTION
 *	return the full block number that contains device block <dbno>
 */
#define DEVBLK2BLK(dbno)	((dbno) >> (BLKSHIFT - DEVBLKSHIFT))



/*
 *  FRAG2DEVBLK(fno)
 *	fno:	frag number
 *
 *  FUNCTION
 *	return the first device block number in frag <fno>
 */
#define FRAG2DEVBLK(fno)	((fno) << (FragShift - DEVBLKSHIFT))

/*
 *  DEVBLK2FRAG(dbno)
 *	dbno:	device block number
 *
 *  FUNCTION
 *	return the first device block number in full block <bno>
 */
#define DEVBLK2FRAG(dbno)	((dbno) >> (FragShift - DEVBLKSHIFT))



/*
 *  INO2BYTE(inum)
 *	inum:	inode number
 *
 *  FUNCTION
 *	return the byte offset of inode <inum>
 */
#define INO2BYTE(inum)	((long long)(inum) << DINODESHIFT)

/*
 *  BYTE2INO(off)
 *	off:	byte offset
 *
 *  FUNCTION
 *	return the inode number that contains byte offset <off>
 */
#define BYTE2INO(off)		((off) >> DINODESHIFT)



/*
 *  INO2BLK(inum)
 *	inum:	inode number
 *
 *  FUNCTION
 *	return the logical inode block number that contains inode <inum>
 */
#define INO2BLK(inum)	((inum)  >> INOPERBLKSHIFT)

/*
 *  BLK2INO(ibno)
 *	ibno:	logical inode block number
 *
 *  FUNCTION
 *	return first inode number in logical inode block <ibno>
 */
#define BLK2INO(ibno)	((ibno)  << INOPERBLKSHIFT)



/*
 *  INO2FRAG(inum)
 *	inum:	inode number
 *
 *  FUNCTION
 *	return the logical inode block number that contains inode <inum>
 */
#define INO2FRAG(inum)	((inum) >> (FragShift - DINODESHIFT))

/*
 *  FRAG2INO(ifno)
 *	ifno:	logical inode frag number
 *
 *  FUNCTION
 *	return number of first inode in logical inode frag <ifno>
 */
#define FRAG2INO(inum)	((inum) << (FragShift - DINODESHIFT))



/*
 *  INOINDEX(inum)
 *	inum:	inode number
 *
 *  FUNCTION
 *	return the index of inode <inum> within a BLKSIZE inode block
 */
#define INOINDEX(inum)	((inum) & INOPERBLKMASK)

/*
 *  NUMDADDRS(inode)
 *	inode:	struct dinode
 *
 *  FUNCTION
 *	return the number of disk addresses in <inode>
 */
#define NUMDADDRS(inode) \
	BYTE2BLK((inode).di_size) + ((inode).di_size & BLKMASK ? 1 : 0)

/*
 *  FRAGSIN(frag)
 *	frag:	a frag_t
 *
 *  FUNCTION
 *	return the number of FRAGSIZE fragments in <frag>
 */
#define FRAGSIN(frag)	(FragPerBlk - (frag).nfrags)

/*
 *  FRAGLEN(frag)
 *	frag:	a frag_t
 *
 *  FUNCTION
 *	return the length of <frag> in bytes
 */
#define FRAGLEN(frag)	(FRAGSIN(frag) << FragShift)



/*
 *  NDADDR - max number of disk addresses (frag_t's) stored directly in inode
 *	(defined to be 8 in jfs/fsdefs.h)
 */

/*
 *  NOINDIRECT(ndaddr)
 *	ndaddr: number of disk addresses needed
 *
 *  FUNCTION
 *  	return true if <ndaddr> does not need single or double indirect blocks
 */
#define	NOINDIRECT(ndaddr)  ((ndaddr) <= NDADDR)

/*
 *  ISDOUBLEIND(ndaddr)
 *	ndaddr: number of disk addresses needed
 * 
 *  FUNCTION
 *	return true if <ndaddr> needs double indirect blocks
 */
#define	ISDOUBLEIND(ndaddr)   ((unsigned) (ndaddr) > DADDRPERBLK)

/*
 *  DIDNDX(lbno)
 *	lbno: logical block number in a file
 *
 *  FUNCTION
 *	returns an index (into the double indirect block) that points
 *	to the single indirect block that contains logical block
 *	number <lbno>
 */
#define	DIDNDX(lbno)      ((lbno) >> DADDRPERBLKSHIFT)

/*
 *  SIDNDX(lbno)
 *	lbno: logical block number in a file
 *
 *  FUNCTION
 *	returns an index (into a single indirect block) that points
 *	to the frag_t for logical block number <lbno>
 */
#define	SIDNDX(lbno)      ((lbno) & DADDRPERBLKMASK)

/*
 *  maximum number of disk addresses contained in an inode
 *  (not counting the double and single indirect blocks themselves)
 */
#define AIX_FILE_MAXBLK	(DADDRPERBLK * INDIRPERBLK)

/*
 *  for op_check, op_make
 */
#define LAST_RSVD_I	15		/* last reserved inode          */

#endif
