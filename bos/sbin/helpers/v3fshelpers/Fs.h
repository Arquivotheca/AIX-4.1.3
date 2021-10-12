/* @(#)80	1.9  src/bos/sbin/helpers/v3fshelpers/Fs.h, cmdfs, bos411, 9428A410j 6/15/90 21:19:59 */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * All of the following will eventually go to fsparam.h and filsys.h
 * 
 * aix3 filesystem constants (soon to be in a real include file!)
 */

#define FSBSIZE         (4096)		/* filesystem block size	*/
#define FSBSHIFT        (12)		/* log2(FSBSIZE)		*/
#define FSBMASK         (FSBSIZE - 1)	/* FSBSIZE mask			*/

#define MIN_FSIZE	DISKMAP_B	/* minimum fs size (FSblocks)   */
#define LAST_RSVD_I	15		/* last reserved inode          */
#define LAST_RSVD_B	DISKMAP_B	/* last reserved block          */

/*
** the maximum number of reserved blocks for a given special "file"
*/
#define MAX_RSVD_SPEC_B	2

/*
** use a sensible name for this (from filsys.h, later fsparam.h?)
*/
#define IPL_I	NON_B

/*
 * return codes for bread() ...
 */
#define	SEEK_FAILED	(-1)
#define	READ_FAILED	(-2)

/*
 * To convert to/from UBSIZE(512) and Fsize(4096)
 */
#define	UFFU_SHIFT	3

/*
 * btofb = bytes to filesystem blocks
 */
#define btofb(x)        (((unsigned)(x) + (FSBSIZE - 1)) >> FSBSHIFT)

/*
** FSBSIZE ==> bytes
*/
#define fbtob(fsblcks)	((fsblcks) << (FSBSHIFT))

/*
** UBSIZE ==> bytes
*/   
#define ubtob(ublcks)	((ublcks) << (UBSHIFT))

/*
** FSBSIZE ==> UBSIZE
*/    
#define fbtoub(fsblcks)	( (fsblcks) << UFFU_SHIFT )


/*
** UBSIZE ==> FSBSIZE
*/
#define ubtofb(ublcks)	( (ublcks) >> UFFU_SHIFT )


/*
 * INOPB - inodes per block
 */
#undef INOPB
#define INOPB           (FSBSIZE / sizeof (struct dinode))

/*
 * itod - inode number to disk block
 */
#undef itod
#define itod(x)         (daddr_t) ((unsigned)(x) / INOPB)

/*
 * itoo - inode number to offset within disk block
 */
#undef itoo
#define itoo(x)         (int) ((unsigned)(x) % INOPB)

/*
 * DADDRPERBLOCK - disk addresses per block
 */
#define DADDRPERBLOCK   (FSBSIZE / sizeof(daddr_t))

/*
 * INDIRPERBLOCK - indirect disk addresses per block
 */
#define INDIRPERBLOCK   (FSBSIZE / sizeof(struct idblock))

/*
** maximum file size, in blocks
*/
#define AIX_FILE_MAXB	((DADDRPERBLOCK * FSBSIZE) * (INDIRPERBLOCK))

/*
 * NOINDIRECT - true if 'nb' (file size in blocks) does not need indirection
 */
#define	NOINDIRECT(nb)  ((nb) <= NDADDR)

/*
 * DOUBLEIND - true if 'nb' needs double indirection
 */
#undef DOUBLEIND    
#define	DOUBLEIND(nb)   ((unsigned) (nb) > DADDRPERBLOCK)

/*
 * DIDNDX - double indirect block 'pg' can be found in
 */
#define	DIDNDX(pg)      ((daddr_t) (pg) / DADDRPERBLOCK)

/*
 * SIDNDX - single indirect block 'pg' can be found in
 */
#define	SIDNDX(pg)      ((daddr_t) (pg) % DADDRPERBLOCK)

