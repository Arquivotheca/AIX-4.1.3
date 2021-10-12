static char sccsid[] = "@(#)11	1.9.2.4  src/bos/kernel/lfs/uname.c, syslfs, bos411, 9428A410j 3/23/94 13:10:37";

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS: uname, unamex, unameu, utsinit
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/utsname.h"
#include "sys/priv.h"

/*
 *
 * NAME:        uname()
 *
 * FUNCTION:    Return utsname info to user
 *
 * INPUT:       address of the user's uname buffer
 *
 * RETURNS:     0 if success, else -1 with u_error set
 *
 */

uname(up)
struct utsname *up;
{
	if (copyout(&utsname, up, sizeof(utsname)))
	{
		u.u_error = EFAULT;
		return -1;
	}
	return 0;
}

/*
 *
 * NAME:        unamex()
 *
 * FUNCTION:    Return extended utsname info to user
 *
 * INPUT:       address of the user's unamex buffer
 *
 * RETURNS:     0 if success, else -1 with u_error set
 *
 */

unamex(xup)
struct xutsname *xup;
{
	if( copyout(&xutsname, xup, sizeof(xutsname)) )
	{
		u.u_error = EFAULT;
		return -1;
	}
	return 0;
}

unameu(sup,swflg)
struct setuname *sup;
short swflg;
{
	struct setuname su;
	extern kernel_lock;
	int    rc = 0;
	int    lockt;

	lockt = lockl(&kernel_lock, LOCK_SHORT);

	if (!priv_req(FS_CONFIG)) {
		rc = EPERM;
		goto out;
	}

	if( copyin(sup,&su,sizeof(su)) )
	{
		rc = EFAULT;
		goto out;
	}

	if( su.target != UUCPNODE )
	{
		rc = EACCES;
		goto out;
	}

	if(swflg) {			/* change node name */
		if( su.len > sizeof(utsname.nodename) || su.len <= 0 )
		{
			rc = EFAULT;
			goto out;
		}
	
		if( copyin(su.newstr, utsname.nodename, su.len) )
		{
			rc = EFAULT;
			goto out;
		}
	
		if( su.len < sizeof(utsname.nodename) )
			utsname.nodename[su.len] = '\0';
	}
	else {		/* change the system name */
		if( su.len > sizeof(utsname.sysname) || su.len <= 0 )
		{
			rc = EFAULT;
			goto out;
		}
	
		if( copyin(su.newstr, utsname.sysname, su.len) )
		{
			rc = EFAULT;
			goto out;
		}
	
		if( su.len < sizeof(utsname.sysname) )
			utsname.sysname[su.len] = '\0';
	}
out:
	if (lockt != LOCK_NEST)
		unlockl(&kernel_lock);
	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}

/*
* NAME:		utsinit
*
* FUNCTION:	initialize the utsname structure
*
* INPUT:	none
*
* RETURNS:	none
*
* NOTES:	Typically the utsname struture is initialized staticly
*		in conf.c.
*/

utsinit()
{
	if (!strncmp(utsname.nodename, "nodename", 8))
		strcpy(utsname.nodename, utsname.machine);
}
