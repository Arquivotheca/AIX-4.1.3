static char sccsid[] = "@(#)35	1.8  src/bos/kernel/pfs/xix_strat.c, syspfs, bos411, 9428A410j 7/7/94 16:54:58";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_strategy
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/errno.h"

/*
 * NAME: jfs_strategy (vp,bp)
 *
 * FUNCTION:	None
 *
 * PARAMETERS:	Unused
 *
 * RETURN :	Always EINVAL
 */

jfs_strategy(vp, bp)
struct vnode *vp;
struct buf *bp;
{
	return EINVAL;
}
