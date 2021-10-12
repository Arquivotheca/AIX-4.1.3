/* static char sccsid[] = "@(#)63 1.2  src/bos/sbin/helpers/v3fshelpers/libfs/fslimits.h, cmdfs, bos411, 9428A410j 2/21/94 10:32:28"; */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS:	include file
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
 *
 *
 * 
 *  Functions that know the JFS size limitations and express these 
 *  limitations in 512 byte blocks.
 */

#include <sys/types.h>

/*
 * NAME:	FS_NBPI_LIM (nbpi)
 *
 * FUNCTION:
 *		Returns the maximum jfs filesystem size in 512 byte blocks,
 * 		based on the nbpi value.
 *	
 *		The jfs is limited to 8 segments for inodes. 
 *
 *		Inode limitation:
 *		NBPI * (Number of 512 byte blocks for inodes) =
 * 		NBPI * (SEG_SIZE * 8 / sizeof(inode))  / 2^9 = 
 * 		NBPI * (SEG_SIZE * 8) /  2^16) =
 *		NBPI * 2^31 / 2^16
 *		NBPI * 2^15
 *	
 *		Largest resultant:	 2^14 * 2^15
 * 
 * PARAMETERS:	nbpi	- nbpi ratio
 *	
 */

#define	FS_NBPI_LIM(nbpi) (size_t)(nbpi) << 15


/*
 * NAME:	FS_ADDR_LIM (fs)
 *
 * FUNCTION:	Returns the maximum filesystem size in 512 byte blocks,
 * 		based on the frag size.
 *	
 *		The jfs is limited to an addressability of 2^28 frags.
 *
 *		Address limitation:
 *		2^28 * Frag / 512 =
 *		2^19 * Frag
 *
 *		Largest resultant:	2^19 * 2^12
 *
 * PARAMETERS:	fs	- fragment size
 *	
 */

#define	FS_ADDR_LIM(fs) (size_t)(fs) << 19


/*
 * Define the offically supported maximum filesystem size in 512 byte
 * blocks. Since we can only test upto 64G, we don't want to spit
 * out messages that say the maximum fs size is > 64G.
 *	
 */
#define	MAX_SUPPORTED_FS	(1 << 27)
