static char sccsid[] = "@(#)38	1.27  src/bos/kernel/pfs/xix_sync.c, syspfs, bos41J, 9512A_all 3/22/95 07:38:43";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_sync, jfs_logsync
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include  "jfs/jfslock.h" 

/*
 * NAME:	jfs_sync()
 *
 * FUNCTION:	commit all regular files which have not been committed 
 *		since the last time jfs_sync() was invoked. 
 *
 *		initiates i/o for all modified journalled pages which 
 *		can be written to their home address or marks those which 
 *		can not so that they will be written when they are committed. 
 *		a new logsync value is computed for all logs.
 *
 *		sync disk quotas.	
 *
 * PARAMETERS:	none
 *
 * RETURN VALUE: none
 */

jfs_sync()
{
 	/* commits all regular files
	 */
	i_sync();

	/* sync all journalled segments 
	 */
	ilogsync(NULL);

	/* sync disk quotas.
	 */
	qsync(0);

	return 0;
}
