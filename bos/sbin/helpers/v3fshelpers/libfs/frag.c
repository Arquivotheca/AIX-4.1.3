static char sccsid[] = "@(#)71	1.2  src/bos/sbin/helpers/v3fshelpers/libfs/frag.c, cmdfs, bos411, 9428A410k 7/14/94 10:09:55";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: rwfrag
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
#include <unistd.h>
#include <libfs/libfs.h>
#include <sys/lvdd.h>

/*
 *  rwfrag(fd, buf, frag, mode)
 *	fd:	file descriptor for disk device
 *	buf:	buffer to read into or write from
 *	frag:	disk address to read from or write to
 *	mode:	tells us to read or write
 *	size:	size of thing we're reading or writing
 *		(if < 0, read or write length of frag)
 *
 *  FUNCTION
 *	read or write a fragment
 *		
 *  RETURN VALUES
 *	success: number of bytes read or written
 *	failure: negative number (LIBFS_INTERNAL, LIBFS_READFAIL,
 *		 LIBFS_WRITEFAIL, LIBFS_SEEKFAIL, LIBFS_BADFRAG)
 */
  
int
rwfrag (int    fd,	/* file descriptor for disk device		*/
	char   *buf,	/* buffer to read/write into/from		*/
	frag_t frag,	/* disk address to read/write from/to		*/
	int	mode)	/* are we reading (Get) or writing (Put)	*/
{		
	int	flen;		/* length of frag in bytes		*/
	int	rc;		/* return code for lseek/read/write	*/

	if (!FragSize)
		return LIBFS_INTERNAL;
	/*
	 *  check validity of frag
	 *  determine size of frag we're reading or writing
	 *  lseek to the frag's offset
	 */
	if (frag.nfrags >= FragPerBlk)
		return LIBFS_BADFRAG;
	flen = FRAGLEN(frag);

#ifdef _BLD
	if (lseek(fd, (off_t)FRAG2BYTE(frag.addr), SEEK_SET) < 0)
#else
	if (llseek(fd, (offset_t)FRAG2BYTE(frag.addr), SEEK_SET) < 0)
#endif
		return LIBFS_SEEKFAIL;

	switch(mode)
	{
	case PUT:
		rc = writex(fd, buf, flen, WRITEV);
		break;
	case GET:
		rc = read(fd, buf, flen);
		break;
	default:
		return LIBFS_INTERNAL;
	}
	if (rc != flen)
		return mode == GET ? LIBFS_READFAIL : LIBFS_WRITEFAIL;
        return flen;
}
