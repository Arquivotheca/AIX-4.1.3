static char sccsid[] = "@(#)28        1.11.1.4  src/bos/kernel/s/priv/syspriv.c, sysspriv, bos411, 9428A410j 3/28/94 03:01:51";

/*
 * COMPONENT_NAME:  TCBPRIV
 *
 * FUNCTIONS:  setpriv(), getpriv()
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential and Proprietary
 */

#include "sys/priv.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/audit.h"
#include "../auth/crlock.h"

extern  Simple_lock cred_lock;


int
getpriv(which, privset, privsiz)
int	which;
priv_t	*privset;
int	privsiz;
{
	priv_t	priv;

	int	rc = 0;

        CRED_LOCK();
	switch (which)
	{
	    case PRIV_EFFECTIVE:
		priv = U.U_cred->cr_epriv;
		break;
	    case PRIV_INHERITED:
		priv = U.U_cred->cr_ipriv;
		break;
	    case PRIV_BEQUEATH:
		priv = U.U_cred->cr_bpriv;
		break;
	    case PRIV_MAXIMUM:
		priv = U.U_cred->cr_mpriv;
		break;
	    default:
        	CRED_UNLOCK();
		u.u_error = EINVAL;
		return(-1);
	}
        CRED_UNLOCK();

	if (privsiz != sizeof(priv_t))
	{
		u.u_error = EINVAL;
                return(-1);
	}

	if (copyout(&priv, privset, sizeof(priv_t)))
	{
		u.u_error = EFAULT;
                return(-1);
	}
	
	return(0);
}


int
setpriv(which, privset, privsiz)
int	which;
priv_t	*privset;
int	privsiz;
{
	long	cmd;
	priv_t	priv;
	priv_t	priv_max;
	priv_t	priv_inh;
	priv_t	priv_eff;
	priv_t	priv_beq;
	static int svcnum = 0;

	if(audit_flag && audit_svcstart("PROC_Privilege", &svcnum, 1, which)){ 
		audit_svcbcopy(privset, sizeof(priv_t));
		audit_svcfinis();
	}


	if (privset == NULL)
		priv.pv_priv[0] = priv.pv_priv[1] = 0;
	else
	{
		if (privsiz != sizeof(priv_t))
		{
			u.u_error = EINVAL;
			return(-1);
		}

		if (copyin(privset, &priv, sizeof(priv_t)))
		{
			u.u_error = EFAULT;
			return(-1);
		}
	}

        CRED_LOCK();
	priv_max = U.U_cred->cr_mpriv;
	priv_inh = U.U_cred->cr_ipriv;
	priv_eff = U.U_cred->cr_epriv;
	priv_beq = U.U_cred->cr_bpriv;

	switch (which & PRIV_COMMANDS)
	{
	    case PRIV_SET:
		if (which & PRIV_MAXIMUM)
			priv_max = priv;
		if (which & PRIV_INHERITED)
			priv_inh = priv;
		if (which & PRIV_EFFECTIVE)
			priv_eff = priv;
		if (which & PRIV_BEQUEATH)
			priv_beq = priv;
		break;
	    case PRIV_ADD:
		if (which & PRIV_MAXIMUM)
			priv_add(&priv_max, &priv);
		if (which & PRIV_INHERITED)
			priv_add(&priv_inh, &priv);
		if (which & PRIV_EFFECTIVE)
			priv_add(&priv_eff, &priv);
		if (which & PRIV_BEQUEATH)
			priv_add(&priv_beq, &priv);
		break;
	    case PRIV_SUB:
		if (which & PRIV_MAXIMUM)
			priv_sub(&priv_max, &priv);
		if (which & PRIV_INHERITED)
			priv_sub(&priv_inh, &priv);
		if (which & PRIV_EFFECTIVE)
			priv_sub(&priv_eff, &priv);
		if (which & PRIV_BEQUEATH)
			priv_sub(&priv_beq, &priv);
		break;
	    default:
		u.u_error = EINVAL;
        	CRED_UNLOCK();
		return(-1);
	}

	/*
	 * now make sure that the security policy is still enforced
	 */
	if ((priv_le(&priv_max, &U.U_cred->cr_mpriv))	&&
	    (priv_le(&priv_eff, &priv_max))		&&
	    (priv_le(&priv_inh, &priv_max))		&&
	    (priv_le(&priv_beq, &priv_max)))
	{
		U.U_cred = _crcopy(U.U_cred);
		U.U_cred->cr_mpriv = priv_max;
		U.U_cred->cr_epriv = priv_eff;
		U.U_cred->cr_ipriv = priv_inh;
		U.U_cred->cr_bpriv = priv_beq;
        	CRED_UNLOCK();
		return(0);
	}

	u.u_error = EINVAL;
        CRED_UNLOCK();
	return(-1);
}
		
