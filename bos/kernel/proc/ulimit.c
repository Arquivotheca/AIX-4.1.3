static char sccsid[] = "@(#)47	1.16.2.6  src/bos/kernel/proc/ulimit.c, sysproc, bos411, 9428A410j 6/2/94 12:00:30";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: PGBNDDN
 *		PGBNDUP
 *		ulimit
 *		
 *
 *   ORIGINS: 27,3,26
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/param.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/pseg.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/lockl.h>
#include <sys/lock.h>
#include <sys/trchkid.h>
#include <ulimit.h>
#include <sys/priv.h>
#include <sys/uio.h>

/*
 * NAME: ulimit()
 *
 * FUNCTION: Sets/Gets process limits
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *
 * RETURN VALUES:
 *	retval	Value set/get.
 *      -1	failed, errno indicates cause of failure
 */

#define	PGBNDUP(x)	( ((x)+PAGESIZE-1) & ~((long)(PAGESIZE-1)) )
#define	PGBNDDN(x)	( (x) & ~((long)(PAGESIZE-1)) )

long
ulimit( int cmd, off_t newlimit )
/* int cmd,			* command to run */
/* off_t newlimit 		* new resource size to set */
{
	long	retval = 0;
	long	rc;
	long delta;
	struct rlimit *rlp,*rlp2;
	struct rlimit alim;

	TRCHKGT_SYSC(ULIMIT, cmd, newlimit, NULL, NULL, NULL);

	switch(cmd) {
	case SET_FSIZE:
		rlp = &U.U_rlimit[ RLIMIT_FSIZE ];
		if ((unsigned long)newlimit > (RLIM_INFINITY >> UBSHIFT))
		{
			retval = RLIM_INFINITY >> UBSHIFT;
			newlimit = RLIM_INFINITY;
		}
		else
		{
			retval = newlimit;
			newlimit <<= UBSHIFT;
		}
		if (newlimit > rlp->rlim_cur &&
		    (privcheck(SET_PROC_RAC) == EPERM))
			u.u_error = EPERM;
		else
			rlp->rlim_cur = rlp->rlim_max = newlimit;
		break;

	case GET_FSIZE:
		rlp = &U.U_rlimit[ RLIMIT_FSIZE ];
		retval = rlp->rlim_cur >> UBSHIFT;
		break;

	case SET_DATALIM:
		newlimit = PGBNDUP(newlimit)-PRIVORG;
		alim.rlim_cur =	alim.rlim_max = newlimit;

		simple_lock(&U.U_handy_lock);
		xsetrlimit(RLIMIT_DATA,&alim);
		simple_unlock(&U.U_handy_lock);
		/* if u_error is set by xsetrlimit code below will
		 * set retval to -1
		 */
		/* fall into GET_DATALIM */

	case GET_DATALIM:
		retval = U.U_dmax + PRIVORG;
		break;

	case SET_STACKLIM:
		newlimit = PRIVORG+U_REGION_SIZE-PGBNDDN(newlimit);
		alim.rlim_cur =	alim.rlim_max = newlimit;
		simple_lock(&U.U_handy_lock);
		xsetrlimit(RLIMIT_STACK,&alim);
		simple_unlock(&U.U_handy_lock);
		/* if u_error is set by xsetrlimit code below will
		 * set retval to -1
		 */
		/* fall into GET_STACKLIM */

	case GET_STACKLIM:
		retval = PRIVORG + U_REGION_SIZE - U.U_smax;
		break;

	case GET_REALDIR:	/* query u.u_compatibility */
		retval = U.U_compatibility & PROC_RAWDIR;
		break;

	case SET_REALDIR:	/* set u.u_compatiblity for dirstyle = sysV */
		retval = U.U_compatibility & PROC_RAWDIR;
		if ( newlimit )
			U.U_compatibility |= PROC_RAWDIR;
		else
			U.U_compatibility &= ~PROC_RAWDIR;
		break;

	default:
		u.u_error = EINVAL;
	}

	if (u.u_error)
		return(-1);
	else
		return(retval);
}

/*
 * NAME: getfslimit
 *
 * FUNCTION: Returns current file size limit as a 64 bit integer.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 */

offset_t
getfslimit(void)
{
	return (offset_t)U.U_rlimit[RLIMIT_FSIZE].rlim_cur;
}
