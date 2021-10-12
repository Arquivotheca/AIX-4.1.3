static char sccsid[] = "@(#)29        1.19.1.12  src/bos/kernel/s/auth/uid.c, syssauth, bos411, 9428A410j 5/11/94 07:38:15";

/*
 *   COMPONENT_NAME:  SYSSAUTH
 *
 *   FUNCTIONS: getuidx
 *              seteuid
 *              setreuid
 *              setuid
 *              setuidx
 *
 *   ORIGINS: 27 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/types.h"
#include "sys/user.h"
#include "sys/priv.h"
#include "sys/id.h"
#include "sys/errno.h"
#include "sys/audit.h"
#include "sys/var.h"
#include "sys/syspest.h"
#include "crlock.h"

extern  Simple_lock cred_lock;

/*
 * setuidx() implements AIX security policy and is the basis for the
 * compatiblity interfaces: setuid(), setreuid() and seteuid().
 *
 * The policy can be stated:
 *      The 3 id's (saved, real and effective) form a hierarchy.  If
 *      the saved id is set then so are the real and effective.  If the
 *      real is set then the effective is set also.  The effective id can
 *      be set to the current real or saved id, or to any arbitrary value
 *      if the user has the SET_PROC_DAC privilege.  In all other cases
 *      the SET_PROC_DAC privilege is required.  Setting the login id
 *      requires both SET_PROC_DAC and SET_PROC_AUDIT privilege.
 *
 *	Privileges:
 *		SET_PROC_AUDIT is required to set ID_LOGIN
 *		SET_PROC_DAC is required to set ID_SAVED or ID_REAL
 *		SET_PROC_DAC is required to set ID_EFFECTIVE to an id
 *			other than the real or saved.
 *
 * ARGUMENTS:
 *	mask    these are the only 5 valid values:
 *		   ID_EFFECTIVE
 *		   ID_EFFECTIVE|ID_REAL
 *		   ID_EFFECTIVE|ID_REAL|ID_SAVED
 *		   ID_EFFECTIVE|ID_REAL|ID_SAVED|ID_LOGIN
 *		   ID_LOGIN
 *
 *	uid	value the uid(s) are to be set
 */
setuidx(int	mask, 
	uid_t	uid)
{
	int rc;
	static int svcnumR = 0;
	static int svcnumA = 0;
	unsigned long	r_val;
	unsigned long	l_val;

	r_val =  U.U_cred->cr_ruid;
	l_val =  U.U_cred->cr_luid;

	if(audit_flag && audit_svcstart("PROC_RealUID", &svcnumR, 1, r_val)){
		audit_svcfinis();
	}
	
	if(audit_flag && audit_svcstart("PROC_AuditID", &svcnumA, 1, l_val)){ 
		audit_svcfinis();
	}

	CRED_LOCK();
	rc = _setuidx (mask, uid);
	CRED_UNLOCK();
	return rc;
}

_setuidx(int	mask, 
	uid_t	uid)
{
	uid_t  ruid, suid;

	if (uid >= UID_MAX)
	{
		u.u_error = EINVAL;
		return(-1);
	}

	/*
	 * Set the effective, real, saved, and login UIDs.  Process
	 * must have both SET_PROC_DAC and SET_PROC_AUDIT privileges.
	 * Modify the four IDs in the credentials structure, plus
	 * the set-user ID and real user ID in the proc structure.
	 */

	if (mask == (ID_EFFECTIVE|ID_REAL|ID_SAVED|ID_LOGIN))
	{
		if ((u.u_error = _privcheck(SET_PROC_DAC))  ||
		    (u.u_error = _privcheck(SET_PROC_AUDIT)))
			return(-1);

		/*
		 * Setting the real UID causes a check of the number of 
	  	 * processes currently being executed by this UID to be made. 
	 	 * Fail if we exceed the current limit. 
	 	 */

		if (new_uidl(U.U_procp, uid, mask)) 
		{
			return(-1);
		}

		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_luid = uid;
		U.U_cred->cr_suid = uid;
		U.U_cred->cr_ruid = uid;
		U.U_cred->cr_uid  = uid;
		return(0);
	}

	/*
	 * Set the effective, real, and saved UIDs.  Process  must
	 * have SET_PROC_DAC privileges.  Modify the three IDs in
	 * the credentials structure, plus the set-user ID and
	 * real user ID in the proc structure.
	 */

	if (mask == (ID_EFFECTIVE|ID_REAL|ID_SAVED))
	{
		if (u.u_error = _privcheck(SET_PROC_DAC))
			return(-1);

		/*
		 * Setting the real UID causes a check of the number of 
	  	 * processes currently being executed by this UID to be made. 
	 	 * Fail if we exceed the current limit. 
	 	 */

		if (new_uidl(U.U_procp, uid, mask)) 
		{
			return(-1);
		}

		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_suid = uid;
		U.U_cred->cr_ruid = uid;
		U.U_cred->cr_uid  = uid;
		return(0);
	}

	/*
	 * Set the effective and real UIDs.  Process  must
	 * have SET_PROC_DAC privilege.  Modify the two IDs in
	 * the credentials structure, plus the real user ID
	 * in the proc structure.
	 */

	if (mask == (ID_EFFECTIVE|ID_REAL))
	{
		if (u.u_error = _privcheck(SET_PROC_DAC))
			return(-1);

		/*
		 * Setting the real UID causes a check of the number of 
	  	 * processes currently being executed by this UID to be made. 
	 	 * Fail if we exceed the current limit. 
	 	 */

		if (new_uidl(U.U_procp, uid, mask)) 
		{
			return(-1);
		}

		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_ruid = uid;
		U.U_cred->cr_uid  = uid;
		return(0);
	}

	/*
	 * Set the effective UID.  Process does not need any
	 * privilege for this operation, but may only set the
	 * effective to the real or saved UID.
	 */

	if (mask == ID_EFFECTIVE)
	{
		ruid = U.U_cred->cr_ruid;
		suid = U.U_cred->cr_suid;
		if ((uid != ruid) && 
                    (uid != suid) &&
                    _privcheck(SET_PROC_DAC))
		{
			u.u_error = EPERM;
			return(-1);
		}
		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_uid = uid;
		return(0);
	}

	/*
	 * Set only the login UID.  Process must have both the
	 * SET_PROC_DAC and SET_PROC_AUDIT privileges.  Only the
	 * login UID in the credentials structure is affected.
	 */

	if (mask == ID_LOGIN)
	{
		if ((u.u_error = _privcheck(SET_PROC_AUDIT)) ||
		    (u.u_error = _privcheck(SET_PROC_DAC)))
			return(-1);

		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_luid = uid;
		return(0);
	}

	/*
	 * None of the valid groups of UIDs were selected to be
	 * set, so return an error indicating the mask is invalid.
	 */

	u.u_error = EINVAL;
	return(-1);
}


/*
 * Note: This comment is for the setuid() and setreuid() system calls.
 *
 * In order to maintain the flavor of previous privilege mechanisms 
 * the privileges may need to be reset.  When changing the effective 
 * uid to the real uid, the effective privilege vector is set to
 * the inherited privilege vector.  Likewise, when changing the effective 
 * uid to the saved uid, the effective privilege vector is set to the 
 * maximum privilege vector.
 *
 *         uid <- real       implies     eff_priv <- inh_priv
 *         uid <- saved      implies     eff_priv <- max_priv
 *
 * The rational behind this behaviour is that when switching back to
 * the real UID, the program intends to execute with the privileges
 * which were inherited from the invoker, who probably is an unprivileged
 * user.  When switching back to the saved UID, the program intends to
 * executed with any acquired privileges associated with the executable
 * file.
 *
 * General rules of thumb:  Call setuidx() before changing the privilege
 * sets - doing this the other way can cause setuidx() to fail needlessly.
 * There is no need to copy the credentials structure - setuidx() does it
 * for you.  Do not modify the privileges if setuidx() fails - no point
 * in doing things half-way.
 *
 * Exception to general rule of thumb:  POSIX requires setuid(euid) to
 * always succeed.  Since setuidx(ID_REAL|ID_EFFECTIVE) may fail in some
 * situations, we must LIE and say the call passes - AND - we must perform
 * the privilige set changes as normally performed.
 */

int
setuid(uid_t	uid)
{
	int rc;
	static int svcnumR = 0;
	static int svcnumA = 0;
        unsigned long   r_val;
        unsigned long   l_val;

        r_val =  U.U_cred->cr_ruid;
        l_val =  U.U_cred->cr_luid;

        if(audit_flag && audit_svcstart("PROC_RealUID", &svcnumR, 1, r_val)){
                audit_svcfinis();
        }

        if(audit_flag && audit_svcstart("PROC_AuditID", &svcnumA, 1, l_val)){
                audit_svcfinis();
         }

	CRED_LOCK();
	rc = _setuid(uid);
	CRED_UNLOCK();
	return rc;
}

int
_setuid(uid_t	uid)
{
	int	rc;

        if (uid >= UID_MAX)
        {
                u.u_error = EINVAL;
                return(-1);
        }


	if (_privcheck(SET_PROC_DAC) == 0)
	{
                if (new_uidl(U.U_procp, uid, ID_REAL|ID_SAVED))
                {
                        return(-1);
                }

                U.U_cred = _crcopy(U.U_cred);
                U.U_cred->cr_suid = uid;
                U.U_cred->cr_ruid = uid;
                U.U_cred->cr_uid  = uid;

		/*
	 	 * Check the value of uid so that the root user does not
	 	 * have his priveleges cleared out.  This will ensure that
		 * the value of LIBPATH can be taken from the environment.
	 	 */
		if (!(uid == 0 && v.v_leastpriv == 0)) {
			priv_clr (&U.U_cred->cr_mpriv);
			U.U_cred->cr_epriv = U.U_cred->cr_mpriv;
			U.U_cred->cr_ipriv = U.U_cred->cr_mpriv;
			U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;
		}
		return 0;
	
	}

	/*
	 * We are sure of not having SET_PROC_DAC privilege here
	 */

	/*
	 * Change to the privilege domain of the program invoker
	 * temporarily.  Change the effective and bequeath privileges
	 * to those of the invoker.  This does not require any
	 * privilege since only the effective UID is being changed.
	 */
	if (uid == U.U_cred->cr_ruid)
	{
		/*
		 * Down here we are sure of not having SET_PROC_DAC 
		 * privilege. Also, uid is equal to the real uid of the
		 * process 
		 */
		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_uid = uid; 

		U.U_cred->cr_epriv = U.U_cred->cr_ipriv;
		U.U_cred->cr_bpriv = U.U_cred->cr_ipriv;
		return 0;
	}
	
	/*
	 * Change to the privilege domain of the program itself.
	 */

	if (uid == U.U_cred->cr_suid)
	{
		if (uid == U.U_cred->cr_uid)
		{
			/*
			 * Down here we are sure of not having SET_PROC_DAC
			 * privilege. But the call should return success for
			 * the above given reason 
			 */
			U.U_cred = _crcopy (U.U_cred);

			U.U_cred->cr_ipriv = U.U_cred->cr_mpriv;	
			U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;	
			U.U_cred->cr_epriv = U.U_cred->cr_mpriv;	
			return 0;
		}

		/*
		 * We have no privilege also uid equals saved_uid of the process
		 */
		U.U_cred = _crcopy(U.U_cred);
                U.U_cred->cr_uid = uid;

		U.U_cred->cr_epriv = U.U_cred->cr_mpriv;
		U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;
		return 0;
	}

	/*
	 * We don't have SET_PROC_DAC privilege, so return -1 with errno
	 * set to EPERM
	 */
	u.u_error = EPERM;
	return(-1);
}


/*
 * BSD functionality added:  arg of -1 => use current uid in it's place
 *
 * For this system call, both real and effective UIDs are specified.
 * However, because the real is only set when the requested real and
 * effective UIDs match, there is no need to check the real parameter.
 *
 * There are a number of circumstances where SET_PROC_DAC kernel
 * privilege is required to change the real UID.  You must always be
 * careful to preserve the privilege sets prior to calling setuidx()
 * lest that call fail needlessly!
 *
 * The privilege sets are modified regardless of the return code from
 * setuidx.
 */
int
setreuid(uid_t	real, 
	 uid_t	eff)
{
	int rc;
	static int svcnumR = 0;
	static int svcnumA = 0;
        unsigned long   r_val;
        unsigned long   l_val;

        r_val =  U.U_cred->cr_ruid;
        l_val =  U.U_cred->cr_luid;

        if(audit_flag && audit_svcstart("PROC_RealUID", &svcnumR, 1, r_val)){
                audit_svcfinis();
        }

        if(audit_flag && audit_svcstart("PROC_AuditID", &svcnumA, 1, l_val)){
                audit_svcfinis();
         }

	CRED_LOCK();
	rc = _setreuid (real, eff);
	CRED_UNLOCK();
	return rc;
}

int
_setreuid(uid_t	real, 
	 uid_t	eff)
{
	int	rc;

	/*
	 * BSD args
	 */
	if (eff == -1)
		eff = U.U_cred->cr_uid;
	if (real == -1)
		real = U.U_cred->cr_ruid;

	/*
	 * Change the effective UID to the current real UID to return
	 * to the privilege state of the invoker.
	 */

	if (eff == U.U_cred->cr_ruid)
	{
		if (eff == real)
		{
			/*
			 * this change requires SET_PROC_DAC since the
			 * real and saved UIDs is being permanently
			 * changed.
			 */

			if (rc = _setuidx (ID_EFFECTIVE|ID_REAL|ID_SAVED, eff))
				return rc;

			U.U_cred->cr_epriv = U.U_cred->cr_ipriv;
			U.U_cred->cr_bpriv = U.U_cred->cr_ipriv;
			U.U_cred->cr_mpriv = U.U_cred->cr_ipriv;
			return 0;
		}

		/*
		 * this requires no privilege since the effective UID
		 * is being changed to the current value of the real
		 * UID.
		 */

		if (rc = _setuidx (ID_EFFECTIVE, eff))
			return rc;

		U.U_cred->cr_epriv = U.U_cred->cr_ipriv;
		U.U_cred->cr_bpriv = U.U_cred->cr_ipriv;
		return 0;
	}

	/*
	 * Change the effective to the current saved UID to return
	 * to the initial privilege state of the program.
	 */

	if (eff == U.U_cred->cr_suid)
	{
		if (eff == real)
		{
			/*
			 * this change requires SET_PROC_DAC since the
			 * real and saved UIDs is being permanently
			 * changed.
			 */

			if (rc = _setuidx(ID_SAVED|ID_REAL|ID_EFFECTIVE, eff))
				return rc;

			U.U_cred->cr_epriv = U.U_cred->cr_mpriv;
			U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;
			U.U_cred->cr_ipriv = U.U_cred->cr_mpriv;
			return 0;
		}

		/*
		 * this requires no privilege since the effective UID
		 * is being changed to the current value of the saved
		 * UID.
		 */

		if (rc = _setuidx(ID_EFFECTIVE, eff))
			return rc;

		U.U_cred->cr_epriv = U.U_cred->cr_mpriv;
		U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;
		return 0;
	}

	/*
	 * Change the effective, real, and saved to some new, arbitrary
	 * value.  The new privilege sets will be empty.  The same real
	 * and effective UIDs must be requested in order to comply with
	 * the security policy.
	 */

	if (eff != real)
	{
		u.u_error = EPERM;
		return(-1);
	}
	if (rc = _setuidx (ID_EFFECTIVE|ID_REAL|ID_SAVED, eff))
		return rc;

	priv_clr (&U.U_cred->cr_mpriv);
	U.U_cred->cr_ipriv = U.U_cred->cr_mpriv;
	U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;
	U.U_cred->cr_epriv = U.U_cred->cr_mpriv;
	return rc;
}

/*
 * Return the value of one of the four UIDs.
 */

uid_t
getuidx(int	which)
{
	unsigned long	rval;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	if (which == ID_LOGIN)
	{
		rval = U.U_cred->cr_luid;
		goto out;
	}

	if (which == ID_SAVED)
	{
		rval = U.U_cred->cr_suid;
		goto out;
	}

	if (which == ID_REAL)
	{
		rval = U.U_cred->cr_ruid;
		goto out;
	}

	if (which == ID_EFFECTIVE)
	{
		rval = U.U_cred->cr_uid;
		goto out;
	}

	if (tlock) CRED_UNLOCK();

	/* which is not valid */
	u.u_error = EINVAL;
	return(-1);
out:
	if (tlock) CRED_UNLOCK();
	return(rval);
}

/*
 * Sets the effective UID to either the real UID or the saved
 * UID and toggles the effective and bequeath privilege sets
 * between the inherited [ for the real UID ] and the maximum
 * [ for the saved UID ].  This call can be used to toggle back
 * and forth arbitrarily between the invoker's domain and the
 * program's domain.
 *
 * This call does not require privilege since the effective
 * UID is changed to permissible values.  EPERM results if an
 * illegal change is requested.
 */

int
seteuid(uid_t	uid)
{
	int rc;
	static int svcnumR = 0;
	static int svcnumA = 0;
        unsigned long   r_val;
        unsigned long   l_val;

        r_val =  U.U_cred->cr_ruid;
        l_val =  U.U_cred->cr_luid;

        if(audit_flag && audit_svcstart("PROC_RealUID", &svcnumR, 1, r_val)){
                audit_svcfinis();
        }

        if(audit_flag && audit_svcstart("PROC_AuditID", &svcnumA, 1, l_val)){
                audit_svcfinis();
         }

	CRED_LOCK();
	rc = _seteuid (uid);
	CRED_UNLOCK();
	return rc;
}

int
_seteuid(uid_t	uid)
{
	int	rc;

	/*
	 * Change back to the privilege domain of the program
	 * invoker.  This is not a permanent change.
	 */

	if (uid == U.U_cred->cr_ruid)
	{
		if (rc = _setuidx (ID_EFFECTIVE, uid))
			return rc;

		U.U_cred->cr_epriv = U.U_cred->cr_ipriv;
		U.U_cred->cr_bpriv = U.U_cred->cr_ipriv;
		return 0;
	}
	
	/*
	 * Change back to the privilege domain of the program
	 * itself.  This is not a permanent change.
	 */

	if (uid == U.U_cred->cr_suid)
	{
		if (rc = _setuidx (ID_EFFECTIVE, uid))
			return rc;

		U.U_cred->cr_epriv = U.U_cred->cr_mpriv;
		U.U_cred->cr_bpriv = U.U_cred->cr_mpriv;
		return 0;
	}

        /*
         * If the user has sufficient privilege then we may change the
         * effective id.
         */
        if (_privcheck(SET_PROC_DAC) == 0)
        {
                if (rc = _setuidx(ID_EFFECTIVE, uid))
                        return rc;

                /*
                 * If we are changing to someone other than uid 0 then
                 * clear the privilege vectors.
                 */
                if (!(uid == 0 && v.v_leastpriv == 0))
                {
                        priv_clr(&U.U_cred->cr_epriv);
                        priv_clr(&U.U_cred->cr_bpriv);
                }
                return 0;
        }

	if ( uid == U.U_cred->cr_uid )
	{
		/* NO OP */
		return 0;
	}

	/*
	 * This violates the security policy since the requested
	 * effective UID is neither the saved nor real UID.
	 */

	u.u_error = EPERM;
	return -1;
}
