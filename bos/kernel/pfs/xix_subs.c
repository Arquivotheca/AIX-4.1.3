static char sccsid[] = "@(#)36	1.15.1.3  src/bos/kernel/pfs/xix_subs.c, syspfs, bos411, 9428A410j 7/7/94 16:55:02";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: power2, pfs_memsize, pfs_exception
 *
 * ORIGINS: 3, 26, 27
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

#include "sys/vmker.h"		/* Get size of real memory		*/
#include "jfs/commit.h"

/*
 * NAME:	power2 (num)
 *
 * FUNCTION:	return power of 2 >= to num.
 *	        does not work for 0x80000000
 *
 * PARMETERS:	nbytes current # 
 *
 * RETURN VALUE: power of 2 >= num
 *
 */

power2 (num)
int num;
{
	int p2;
	unsigned long mask = 0x80000000;

	p2 =  mask >> clz32 (num);	/* XXX change to assembler */
	return ((num & (p2 - 1)) ? p2 << 1 : p2);
}


/*
 * NAME:	pfs_memsize
 *
 * FUNCTION:	get real memory size
 *
 * NOTES:	There should be a service for this value.  The only
 *		way to get it is from the VMM, or calculate it from
 *		the ram bit map
 *
 * RETURN VALUE: size of real memory in bytes
 *
 */

uint
pfs_memsize()
{
	return((vmker.nrpages - vmker.badpages) * PAGESIZE);
}


/*
 * NAME:	pfs_exception (rc, elist)
 *
 * FUNCTION:	check return code:  
 *		if PFS_EXCEPTION is set, scan for acceptable error condition. 
 *		if none found longjmp() to next handler.
 */
pfs_exception (rc, elist)
int rc;				/* return code to validate */
int *elist;			/* null terminated error list */
{
	int *err;

	if (rc & PFS_EXCEPTION)
	{	
		rc &= ~PFS_EXCEPTION;
		
		for (err = elist; *err; err++)
			if (*err == rc)
				return rc;

		/* may want to do some error logging here */

		longjmpx (rc);
		panic ("pfs_exception: unexpected returned from longjmp()");
	}

	return rc;
}
