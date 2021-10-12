static char sccsid[] = "@(#)73	1.2  src/bos/sbin/helpers/v3fshelpers/libfs/super.c, cmdfs, bos411, 9428A410k 7/14/94 10:10:59";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: get_pow2mult, inrange, get_super, put_super, validate_super
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>

#define _IN_SUPER_C
#include <libfs/libfs.h>


/*
 *  this returns the first power-of-2 multiple of <mult> that is just
 *  greater than or equal to <num> 
 *	a power-of-2 multiple of i is:  i * [1, 2, 4, 8, ...]
 *
 *  another way of putting it:
 *	it returns <num> rounded up to a power-of-2 multiple of <mult>
 *
 *  NOTE: if someone calls this with mult equal to 0, get_pow2mult returns 0
 */
int
get_pow2mult(int mult,
	     int num)
{
	if (mult)
		for (; mult < num; mult <<= 1)
			;
	return mult;
}


/*
 *  log2(num)
 *	num:	number to take log base 2 of
 *
 *  FUNCTION
 *	return log base 2 of <num>
 *
 *  RETURN VALUES
 *	success: >= 0
 *	failure:  < 0 (LIBFS_INTERNAL)
 */
static int
log2(unsigned int num)
{
	int exp;
	uint mask;	/* unsigned because don't want sign extension */

	if (num <= 0)
		return LIBFS_INTERNAL;
	for (mask = 0x80000000, exp = 31; !(num & mask); mask >>= 1, exp--)
		;
	return exp;
}


/*
 *  inrange() checks to see that <num> is a power-of-2 multiple of <low>
 *  that is less than or equal to <high>.  If it is, it returns 1, else 0
 *
 *  NOTE: if low is 0, inrange returns 0
 */
int
inrange(int num,
	int low,
	int high)
{
	if (low)
		for (; low <= high; low <<= 1)
			if  (low == num)
				return 1;
	return 0;
}


/*
 * NAME:	validate_sizes
 *
 * FUNCTION:	Ensure that all configurable sizes fall within their
 *		respective version specific limits. 
 *
 * PARAMETERS:
 *	struct superblock	*sb
 *
 * RETURN VALUES
 *	success: >= 0
 *	failure:  < 0 (LIBFS_INTERNAL)
 */

static int
validate_sizes (struct superblock *sb)
{
	int i;

	if (sb->s_iagsize == sb->s_agsize && sb->s_fragsize == sb->s_bsize)
	{
		for (i = AGDEFAULT; i > 0; i -= MINAGSIZE)
			if (DBPERPAGE % i == 0 && sb->s_agsize == i)
				return LIBFS_SUCCESS;
	}
	else if (inrange(sb->s_agsize, MINAGSIZEV4, MAXAGSIZEV4) &&
		 inrange(sb->s_iagsize, MINAGSIZEV4, MAXAGSIZEV4) &&
		 inrange(sb->s_fragsize, MINFRAGSIZE, MAXFRAGSIZE))
	{
		return LIBFS_SUCCESS;
	}
	
	return LIBFS_INTERNAL;
}


/*
 *  validate_super(struct superblock *sblk)
 *
 *  FUNCTION
 *	validate superblock
 *	initialize block/frag size dependant constants
 *
 *  RETURN VALUES
 *	success: 0
 *	failure: LIBFS_CORRUPTSUPER, LIBFS_BADVERSION, LIBFS_BADMAGIC
 */
int
validate_super(struct superblock *sb)
{

	if (! memcmp((void *)sb->s_magic, (void *)fsv3magic,
		     sizeof(sb->s_magic)))
	{
		if (sb->s_version != fsv3vers)
			return LIBFS_BADVERSION;
		sb->s_fragsize = sb->s_bsize;
		sb->s_iagsize  = sb->s_agsize;
		if (validate_sizes(sb) == LIBFS_INTERNAL)
			return LIBFS_CORRUPTSUPER;			
	}
	else
	if (! memcmp((void *)sb->s_magic, (void *)fsv3pmagic,
		     sizeof(sb->s_magic)))
	{
		if (sb->s_version != fsv3pvers)
			return LIBFS_BADVERSION;
		if (validate_sizes(sb) == LIBFS_INTERNAL)
			return LIBFS_CORRUPTSUPER;	
		if (sb->s_compress && sb->s_fragsize > BLKSIZE/2)
			return LIBFS_CORRUPTSUPER;
	}
	else
		return (LIBFS_BADMAGIC);
	
	/*
	 *  now that we know fragsize is valid, don't bother 
	 *  checking for the impossibility of log2() failing
	 */
	FragSize  = sb->s_fragsize;
	FragMask  = FragSize - 1;	
	FragShift = log2(FragSize);
	
	FragPerBlk 	= BLKSIZE / FragSize;
	FragPerBlkMask	= FragPerBlk - 1;
	FragPerBlkShift = log2(BLKSIZE / FragSize);
	
	set_inovars(sb->s_agsize, sb->s_iagsize, sb->s_fsize);
	return LIBFS_SUCCESS;
}


/*
 *  get_superblk(fd, sb, is_primary)
 *	fd:		device file descriptor
 *	sb:		ptr to struct superblock
 *	is_primary:	are we getting primary or secondary supblk
 *
 *  FUNCTION
 *	read a superblock from device file descriptor <fd>
 *
 *  RETURN VALUES
 *	success: 0
 *	failure: negative number (LIBFS_SEEKFAIL, LIBFS_READFAIL,
 *		 LIBFS_BADMAGIC, LIBFS_BADVERSION, LIBFS_CORRUPTSUPER)
 */
int
get_superblk (int		fd,
	      struct superblock	*sb,
	      int		is_primary)  /* non0=SUPER_B, 0=SUPER_B1 */
{
	int		  rc;
	char		  buf[BLKSIZE];
	struct superblock *sblk = (struct superblock *)buf;

	if (lseek(fd, (off_t)BLK2BYTE(is_primary ? SUPER_B : SUPER_B1),
		   SEEK_SET) < 0)
		return LIBFS_SEEKFAIL;
        if ((rc = read(fd, sblk, DEV_BSIZE)) != DEV_BSIZE)
		return LIBFS_READFAIL;
	
	/*
	 *  validate magic numbers and version numbers,
	 *  set up fragsize dependant constants
	 */
	if (rc = validate_super(sblk))
		return rc;
	memcpy((void *)sb, (void *)sblk, sizeof(*sb));
	return LIBFS_SUCCESS;
}


/*
 *  put_superblk(fd, sb, is_primary)
 *	fd:		device file descriptor
 *	sb:		ptr to struct superblock
 *	is_primary:	are we putting primary or secondary supblk ?
 *
 *  FUNCTION
 *	write a ver 4 filesystem superblock.
 *
 *  RETURN VALUES
 *	success: 0
 *	failure: negative number (LIBFS_SEEKFAIL, LIBFS_WRITEFAIL,
 *		 LIBFS_CORRUPTSUPER)
 */
int
put_superblk (int 		fd,		
	      struct superblock	*sb,
	      int		is_primary) 	/* non0=SUPER_B, 0=SUPER_B1 */
{
	char	buf[BLKSIZE];
	int 	rc;

	if (rc = validate_super(sb))
		return rc;

	memset((void *)buf, 0, (size_t)BLKSIZE);
	memcpy((void *)buf, (void *)sb, (size_t)sizeof(*sb));
	/*
	 *  if v3, make a real v3 superblock by zeroing out v4 fields
	 */
	if (sb->s_version == fsv3vers)
	{
		((struct superblock *)buf)->s_iagsize = 0;
		((struct superblock *)buf)->s_fragsize = 0;
		((struct superblock *)buf)->s_version = 0;
	}
	if (lseek(fd, (off_t)BLK2BYTE(is_primary ? SUPER_B : SUPER_B1),
		   SEEK_SET) < 0)
		return LIBFS_SEEKFAIL;
        if (write(fd, buf, BLKSIZE) != BLKSIZE)
		return LIBFS_WRITEFAIL;
	return LIBFS_SUCCESS;
}
