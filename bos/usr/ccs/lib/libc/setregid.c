static char sccsid[] = "@(#)37	1.6  src/bos/usr/ccs/lib/libc/setregid.c, libcs, bos411, 9428A410j 6/11/91 13:27:43";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: setregid 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/priv.h>
#include <sys/types.h>
#include <sys/id.h>
#include <sys/errno.h>

extern	gid_t	getgidx (int);

/*
 * NAME: setregid
 *
 * FUNCTION: Provide BSD-style support for setting group IDs
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Library subroutine.
 *
 * NOTES:
 *	From the BSD manual page -
 *
 *	The real and effective group ID's of the current process
 *	are set to the arguments.  Unprivileged users may change
 *	the real group ID to the effective group ID and vice-versa;
 *	only the super-user may may other changes.
 *
 *	Supplying a value of -1 for either the real or effective
 *	group ID forces the system to substitute the current ID
 *	in place of the -1 parameter.
 *
 *	INPUT:
 *		(int) rgid - new real group ID
 *		(int) egid - new effective group ID
 *	OUTPUT:
 *		Modifies the current process' group ID values.
 *	ERRORS:
 *		Returns -1 if the current process does not have the
 *		appropriate privilege (SET_PROC_DAC) and a change
 *		other that setting the real or effective group ID to
 *		either the real or effective group ID.
 *
 *	Since AIX uses the POSIX 3-ID style of group IDs, it is
 *	impossible to faithfully minic this behavior.  What this code
 *	does is try to implement the setregid() call as closely as is
 *	possible, given the constraint of working within AIX V3
 *	behavior.
 *
 * RETURNS:
 *	Zero for success, -1 for failure and errno set to
 *	indicate the reason.  Errno is not modified when this
 *	function succeeds.
 */

int
setregid (int rgid, int egid)
{
	gid_t	real;
	gid_t	eff;
	gid_t	saved;
	int	mask;
	int	priv;

	/*
	 * The BSD behavior for rgid and egid being -1 is such that
	 * it depends on the IDs being swapped or not, so I need to
	 * get the IDs to be able to figure it out.  It is impossible
	 * to figure out what the actual BSD group IDs would be, so
	 * I use the actual AIX group IDs instead.
	 */

	real = getgidx (ID_REAL);
	eff = getgidx (ID_EFFECTIVE);
	saved = getgidx (ID_SAVED);

	/*
	 * BSD group IDs work differently than AIX V3 group IDs.
	 * So, we follow the letter (and not the spirit) of the
	 * definition.  Fortunately, BSD is going to a POSIX
	 * scheme and these calls are being depreciated.
	 */

	if (rgid == -1)
		rgid = real;

	if (egid == -1)
		egid = eff;

	/*
	 * The real group ID must be the current saved, real or
	 * effective unless the user has the appropriate privilege.
	 */

	if ((gid_t) rgid != eff && (gid_t) rgid != real
			&& (gid_t) rgid != saved) {
		if (privcheck (SET_PROC_DAC) || rgid != egid) {
			errno = EPERM;
			return -1;
		}
	}

	/*
	 * If the group IDs are not being set to the same values, the
	 * effective group ID is the only one to set.  All three IDs
	 * are only set when both IDs are set to the same value.
	 */

	if (rgid != egid)
		return setgidx (ID_EFFECTIVE, egid);

	/*
	 * The user wants to change to an entirely new domain, which
	 * could easily be the same domain they are presently in.
	 * Figure out which IDs must be changed, and only do those.
	 * This prevents the case of "setregid (rgid, rgid)" from
	 * failing when it should succeed.
	 */

	if ((gid_t) egid != saved && (gid_t) egid != real)
		mask = ID_EFFECTIVE|ID_REAL|ID_SAVED;
	else if ((gid_t) egid != eff)
		mask = ID_EFFECTIVE;
	else
		return 0;

	return setgidx (mask, (gid_t) egid);
}
