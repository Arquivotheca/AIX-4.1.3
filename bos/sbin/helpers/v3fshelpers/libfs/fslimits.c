static char sccsid[] = "@(#)71	1.3  src/bos/sbin/helpers/v3fshelpers/libfs/fslimits.c, cmdfs, bos411, 9428A410j 2/21/94 10:32:32";
/*
 *   COMPONENT_NAME: CMDFS
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 
 *  Functions that know the JFS size limitations and express these 
 *  limitations in 512 byte blocks.
 */

#include <stdio.h>
#include <lvm.h>
#include <libfs/fslimits.h>

size_t	get_abs_sz (int, int, int);
void	print_jfs_limits (int);


/*
 * NAME:	get_abs_sz
 *
 * FUNCTION:	Return the absolute maximum jfs filesystem size given
 *		a nbpi, a fragment size, and a partition size.
 *
 *		The number returned is limited by the maximum
 *		supported filesystem size.
 *
 * ASSUMPTIONS: LVM max size assumes 1 physical partition per
 *		logical partition (ie no mirrorring).
 *
 * PARAMETERS:	int	nbpi	- nbpi (what more can ya say)
 *		int	frag	- fragment size
 *		int	ps	- partition size in 512 byte blocks
 *
 * RETURN:	max fs size for given geometry as a size_t
 *
 */
size_t
get_abs_sz (int	nbpi,		
	    int	frag,
	    int	ps)		
{
	size_t	nbpi_max	= FS_NBPI_LIM(nbpi);
	size_t	frag_max	= FS_ADDR_LIM(frag);
	size_t	lv_max		= LVM_MAXPVS * LVM_MAXPPS * ps;
	size_t	fs_max		= nbpi_max;

	if (fs_max > frag_max)
		fs_max = frag_max;
	if (fs_max > lv_max)
		fs_max = lv_max;

	return (fs_max > MAX_SUPPORTED_FS ? MAX_SUPPORTED_FS : fs_max);
}


/*
 * NAME:	print_jfs_limits
 *
 * FUNCTION:	print the entire range of jfs filesystem size limitations.
 *		This does not include the LVM limitations.
 *
 * PARAMETERS:	
 *	int	fd	- an open file descriptor (generally stderr or stdio)
 *
 * RETURN:	void
 *
 */
void
print_jfs_limits (int	fd)
{
	fprintf (fd, "%5d%7d,%5d,%5d,%5d%19u\n",
		 512, 512, 1024, 2048, 4096, FS_NBPI_LIM(512));

	fprintf (fd, "%5d%7d,%5d,%5d,%5d%19u\n",
		 1024, 512, 1024, 2048, 4096, FS_NBPI_LIM(1024));

	fprintf (fd, "%5d%7d,%5d,%5d,%5d%19u\n",
		 2048, 512, 1024, 2048, 4096, FS_NBPI_LIM(2048));

	fprintf (fd, "%5d%7d,%5d,%5d,%5d%19u\n",
		 4096, 512, 1024, 2048, 4096, FS_NBPI_LIM(4096));

	/*
	 * Since we are only officially supporting upto 64G
	 * these messages will be supressed. When we can test > 64G
	 * filesystems then they can be uncommented.
	 *
	 *
	 * fprintf (fd, "%5d%7d,%5d,%5d,%5d%19u\n",
	 *	 8192, 512, 1024, 2048, 4096, FS_NBPI_LIM(8192));
	 *
	 * fprintf (fd, "%5d%7d%37u\n", 16384, 512, FS_ADDR_LIM(512));
	 *
	 * fprintf (fd, "%5d%13d,%5d,%5d%19u\n",
	 *	 16384, 1024, 2048, 4096, FS_NBPI_LIM(16384));
	 */
}	 
