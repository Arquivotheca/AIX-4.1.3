static char sccsid[] = "@(#)27        1.31.1.9  src/bos/kernel/s/priv/priv.c, sysspriv, bos411, 9428A410j 4/25/94 03:37:36";

/*
 * COMPONENT_NAME:  TCBPRIV
 *
 * FUNCTIONS:  priv_clr(), priv_null(), priv_add(), priv_sub(), priv_and(),
 *	       priv_not(), priv_le(), privcheck(), priv_req(), priv_chk(),
 *	       suser(), exec_priv(), privinit(), privnotify()
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/priv.h"
#include "sys/user.h"
#include "sys/auditk.h"
#include "sys/audit.h"
#include "sys/lockl.h"
#include "sys/uio.h"
#include "sys/stat.h"
#include "sys/mode.h"
#include "sys/malloc.h"
#include "sys/sysconfig.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/syspest.h"
#include "../auth/crlock.h"

extern  Simple_lock cred_lock;


static int runpcl(struct vnode *vp, priv_t *privp);
static int pce_match(struct pcl_entry	*pcep);

priv_t	allprivs;

/*
 * This is a set of utility functions for dealing with privilege structures.
 *
 * priv_clr(p)			Zeroes out the vector 'p'. 
 *
 * priv_null(p)			Returns non-zero if and only if the privilege
 *				vector 'p' contains no privileges.
 *
 * priv_add(p1, p2)		Sets vector 'p1' to the union of vectors 'p1' 
 * 				and 'p2'.
 * 
 * priv_sub(p1, p2)		Removes the privileges specified by vector 
 *				'p2' from vector 'p1'.
 * 
 * priv_and(p1, p2)		Sets vector 'p1' to the intersection of vectors 
 *				'p1' and 'p2'.
 * 
 * priv_not(p)			Complements the vector 'p';
 *
 * priv_le(p1, p2)		Returns non-zero if vector 'p1' is a subset of
 *				vector 'p2'.
 * 
 * privcheck(p)			Returns zero if the process has the privilege 
 *				specified by 'p', otherwise, EPERM is returned.
 *
 *** the following 2 routines are obsoleted by privcheck()
 *
 * priv_chk(p)			Returns non-zero if and only if the process
 *				(in its effective privilege vector) has the
 *				single privilege specified by 'p'.  This 
 *				function has no side effects.
 *
 * priv_req(p)			Returns non-zero if and only if the process
 *				(in its effective privilege vector) has the
 *				single privilege specified by 'p'.  This 
 *				function generates a PRIV_USE audit record 
 *				the process has the specified privilege, and
 *				a PRIV_FAIL audit record if the process does
 *				not have the specified privilege.
 *
 * suser(error)			If the calling process has any effective 
 *				privileges (a safer assumption might be ALL),
 *				suser generates a PRIV_USE audit record and
 *				returns zero.
 *
 * exec_priv(fp)		This is called by exec to inherit and/or
 *				acquire privilege.  "fp" is an open file
 *				pointer of the object being exec'd.  If
 *				the process has it's privileges amplified,
 *				1 is returned.  If we are on the Trusted Path 
 *				and the file to exec is not trusted, return -1.
 *				Else 0 is returned.
 */


void
priv_clr(priv_t	*p)
{
	p->pv_priv[0] = 0;
	p->pv_priv[1] = 0;
}


int
priv_null(priv_t *p)
{
	if (p->pv_priv[0] || p->pv_priv[1])
		return(0);
	return(1);
}


void
priv_add(priv_t	*p1,
         priv_t	*p2)
{
	p1->pv_priv[0] = p1->pv_priv[0] | p2->pv_priv[0];
	p1->pv_priv[1] = p1->pv_priv[1] | p2->pv_priv[1];
}


void
priv_sub(priv_t	*p1,
	 priv_t	*p2)
{
	p1->pv_priv[0] &= ~(p2->pv_priv[0]);
	p1->pv_priv[1] &= ~(p2->pv_priv[1]);
}


void
priv_and(priv_t	*p1,
	 priv_t	*p2)
{
	p1->pv_priv[0] = p1->pv_priv[0] & p2->pv_priv[0];
	p1->pv_priv[1] = p1->pv_priv[1] & p2->pv_priv[1];
}


void
priv_not(priv_t	*p)
{
	p->pv_priv[0] = ~p->pv_priv[0];
	p->pv_priv[1] = ~p->pv_priv[1];
}


int
priv_le(priv_t	*p1,
        priv_t	*p2)
{
	if (p1->pv_priv[0] != (p1->pv_priv[0] & p2->pv_priv[0]))
		return(0);
	if (p1->pv_priv[1] != (p1->pv_priv[1] & p2->pv_priv[1]))
		return(0);

	return(1);
}


int
privcheck(int	p)
{
	int rc;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	rc = privcheck_cr (p, U.U_cred);
	if (tlock) CRED_UNLOCK();
	return rc;

}

int
_privcheck(int	p)
{
	return privcheck_cr(p, U.U_cred);
}

int
privcheck_cr(int p, struct ucred *crp)
{
	int	rc;

	/*
	 * UID==0 implies superuser if "v_leastpriv" is zero
	 */
	if (v.v_leastpriv == 0)
		if ((crp->cr_uid == 0) && (p != TRUSTED_PATH))
		{
			return(0);
		}

        /* 
	 * pseudo privilege, TRUSTED_PATH, is a special case 
	 */
        if (p == TRUSTED_PATH)
	{
		/* if on the trusted path, succeed */
                if (U.U_ttyf && (*U.U_ttyf)(0, U.U_ttyid))
			return(0);
                return(EPERM);
	}

	if (p < 33)
		rc = crp->cr_epriv.pv_priv[0] & (1 << (p-1));
	else
		rc = crp->cr_epriv.pv_priv[1] & (1 << (p-33));

	return(rc ? 0 : EPERM);
}

int
priv_chk(int	p)
{
	int	rc;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	/*
	 * UID==0 implies superuser if "v_leastpriv" is zero
	 */
	if (v.v_leastpriv == 0)
		if ((U.U_cred->cr_uid == 0) && (p != TRUSTED_PATH))
		{
			if (tlock) CRED_UNLOCK();
			return(1);
		}
        /* 
	 * pseudo privilege, TRUSTED_PATH, is a special case 
	 */
        if (p == TRUSTED_PATH)
	{
		if (tlock) CRED_UNLOCK();
		/* if on the trusted path, succeed */
                if (U.U_ttyf && (*U.U_ttyf)(0, U.U_ttyid))
			return(1);
                return(0);
	}

	if (p < 33)
		rc = U.U_cred->cr_epriv.pv_priv[0] & (1 << (p-1));
	else
		rc = U.U_cred->cr_epriv.pv_priv[1] & (1 << (p-33));

	if (tlock) CRED_UNLOCK();

	return(rc);
}


int
priv_req(int	p)
{
        int     rc;
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	/*
	 * UID==0 implies superuser if "v_leastpriv" is zero
	 */
	if (v.v_leastpriv == 0)
		if ((U.U_cred->cr_uid == 0) && (p != TRUSTED_PATH))
                {
			if (tlock) CRED_UNLOCK();
			return(1);
		}
        /* 
	 * pseudo privilege, TRUSTED_PATH, is a special case 
	 */
        if (p == TRUSTED_PATH)
	{
		if (tlock) CRED_UNLOCK();
		/* if on the trusted path, succeed */
                if (U.U_ttyf && (*U.U_ttyf)(0, U.U_ttyid))
			return(1);
                return(0);
	}

        if (p < 33)
                rc = U.U_cred->cr_epriv.pv_priv[0] & (1 << (p-1));
        else
                rc = U.U_cred->cr_epriv.pv_priv[1] & (1 << (p-33));

	if (tlock) CRED_UNLOCK();

        return(rc);
}


int
suser(char	*uerr)
{
	int tlock;

	if (tlock = (U.U_procp->p_active > 1)) CRED_LOCK();
	if ((U.U_cred->cr_uid == 0) || (priv_null(&U.U_cred->cr_epriv) == 0))
	{
		if (tlock) CRED_UNLOCK();
		return(1);
	}
	if (tlock) CRED_UNLOCK();
	*uerr = EPERM;
	return(0);
}


int
exec_priv(struct file	*fp,
	  struct stat	*sp)
{
	struct vnode	*vp;
	int		acquired = 0;	/* flag indicating priv acquired */
	priv_t		progpriv;	/* privilege associated with prog */
	int		rc = 0;
	priv_t		oldepriv;
		
	/*
	 * if tracing blow this off...
	 */
	if (U.U_procp->p_flag & STRC)
	{
		return(0);
	}

	/* get the vnode from the file pointer */
	vp = fp->f_vnode;

	/*
	 * Here is the policy...
	 *
	 *    If we are in the TRUSTED PATH the exec'd object must
	 *    be in the TCB (i.e. the TCB bit set in the mode)
	 *
	 * acquisition:
	 *    The exec'd object must be a trusted program.  Period.
	 */

	/*
	 * fail exec if on TPATH and this is not a trusted process
	 */
	if ((U.U_ttyf && (*U.U_ttyf)(0, U.U_ttyid)) &&
	    ((sp->st_mode & S_ITCB) == 0))
	{
		rc = -1;
		goto out;
	}
	/*
	 * Fast path:  all 4 privs are NULL, TP is off and not SUID or SGID
	 * Need not take CR_LOCK here as this function is called from
	 * exec and so executed by a single-threaded process
	 */
	if (priv_null(&U.U_cred->cr_mpriv)	&&
	    priv_null(&U.U_cred->cr_ipriv)	&&
	    priv_null(&U.U_cred->cr_bpriv)	&&
	    priv_null(&U.U_cred->cr_epriv)	&&
	    (sp->st_mode & (S_ITP|S_ISUID|S_ISGID) == 0)) 
		/* rc was already initialized to zero */
	{
		goto out;
	}
	
	/*
	 * save old effective privilege to determine if
	 * privilege was amplified
	 */
	oldepriv = U.U_cred->cr_epriv;

	U.U_cred = crcopy(U.U_cred);

	/*
	 * do the SUID/SGID stuff
	 */
	if (!(vp->v_vfsp->vfs_flag & VFS_NOSUID))
	{
		if (sp->st_mode & S_ISUID)
			U.U_cred->cr_uid = sp->st_uid;
		if (sp->st_mode & S_ISGID)
			U.U_cred->cr_gid = sp->st_gid;
	}

	/* regardless, ensure that the saved==eff */
	U.U_procp->p_suid = U.U_cred->cr_suid = U.U_cred->cr_uid;

	/* regardless, ensure that the saved==eff */
	U.U_cred->cr_sgid = U.U_cred->cr_gid;
	
	/* acquire privileges (if TP bit is on) */
	if (sp->st_mode & S_ITP)
		runpcl(vp, &progpriv);
	else
		priv_clr(&progpriv);

	/*
	 * inherit privilege from parent
	 */
	U.U_cred->cr_ipriv = U.U_cred->cr_bpriv;

	/*
	 * if not set up for Least Privilege, go with uid==0
	 */
	if (v.v_leastpriv == 0)
	{
		if (U.U_cred->cr_uid == 0)
		{
			progpriv = allprivs;
			if (U.U_cred->cr_ruid == 0)
				U.U_cred->cr_ipriv = allprivs;
		}
		else
		{
			if ((sp->st_mode & S_ISUID) && (U.U_cred->cr_uid)
				&& (!(vp->v_vfsp->vfs_flag & VFS_NOSUID)))
				priv_clr(&progpriv);
		}
	}
	else
		priv_add(&progpriv, &U.U_cred->cr_bpriv);

	U.U_cred->cr_bpriv = 
	U.U_cred->cr_epriv = 
	U.U_cred->cr_mpriv = progpriv;

	/*
	 * determine if privilege was amplified...
	 */
	if (!priv_le(&U.U_cred->cr_epriv, &oldepriv))
		rc = 1;

out:

	return(rc);
}


/*
 * runpcl()
 *   If no extended priv simply use the default priv
 *   else, run through the PCL looking for matches; all
 *   applicable privs (including the default) are summed.
 */
runpcl(struct vnode	*vp,
       priv_t		*privp)
{
	char		buffer[64];	/* space to retrieve PCL */
	struct pcl	*pclp = (struct pcl *)buffer;	/* pointer PCL */
	int		pclsize = sizeof(buffer);
	struct uio	uio;
	struct iovec	iov;
	struct ucred	*crp;
	int		rc;
	struct pcl_entry	*pce;

	/* NO PRIVILEGE if file system doesn't support VNOP_GETPCL */
	if (vp->v_gnode->gn_ops->vn_getpcl == NULL)
	{
		priv_clr(privp);
		return;
	}

	/*
	 * create a uio structure to hold the PCL
	 */
	iov.iov_base = (char *)pclp;
	iov.iov_len = pclsize;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = pclsize;
	uio.uio_segflg = UIO_SYSSPACE;

	crp = crref();

	if (rc = VNOP_GETPCL(vp, &uio, crp))
	{
		if (rc != ENOSPC)
			goto out;

		/* the PCL has many extended entries */
		pclsize = pclp->pcl_len;
		if ((pclp = malloc(pclsize)) == NULL)
		{
			rc = ENOMEM;
			goto out;
		}
		
		iov.iov_base = (char *)pclp;
		iov.iov_len = pclsize;
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = 0;
		uio.uio_resid = pclsize;
		uio.uio_segflg = UIO_SYSSPACE;

		if (rc = VNOP_GETPCL(vp, &uio, crp))
		{
			rc = EINVAL;
			goto out;
		}
	}
	
	/* default always applies */
	*privp = pclp->pcl_default;

	/* run the pickle */
	for (pce=pclp->pcl_ext; pce<pcl_last(pclp); pce=pcl_nxt(pce))
		if (pce_match(pce))
			priv_add(privp, &pce->pce_privs);

out:
	crfree(crp);

	if ((char *)pclp != buffer)
		free(pclp);
}


int
pce_match(struct pcl_entry	*pcep)
{
	struct pce_id	*idp;
	struct pce_id	*id_end;
	
	id_end = pcl_id_last(pcep);
	for (idp=pcep->pce_id; idp<id_end; idp=pcl_id_nxt(idp))
	{
		switch (idp->id_type)
		{
		    case PCEID_USER:
			if (idp->id_data[0] != U.U_cred->cr_uid)
				return(0);
			continue;

		    case PCEID_GROUP:
			if (!groupmember(idp->id_data[0]))
				return(0);
			continue;
		}
	}
	return(1);
}


/*
 * privinit()
 *
 * This routine is called once at system initialization time.  It's
 * only function is to establish the address of the privnotify()
 * routine for sysconfig using cfgnadd(). and to initialize the 
 * priv_t "allprivs".
 *
 * RETURNS:  none
 */

struct cfgncb	privncb;
int	privnotify();

void
privinit()
{
	privncb.func = privnotify;
	cfgnadd(&privncb);

	allprivs.pv_priv[0] = -1;
	allprivs.pv_priv[1] = -1;
}


/*
 * privnotify()
 *
 * This routine is called each time there is an attempt to modify 
 * the var structure by sysconfig.
 * The only field privnotify() cares about is "v_leastpriv".  This
 * is the variable that privcheck looks at to see if UID==0 has
 * any significance (superuser).  The policy is that this variable
 * has only one valid state change; 0 -> 1 (i.e. enter permanently
 * the least privilege domain).
 *
 * RETURNS:  0 on success (no invalid state change for v_leastpriv)
 *	   or
 *	     byte-offset of v_leastpriv in var struct
 */

#define LEASTPRIV_OFFSET (int)(&((struct var *)0)->v_leastpriv)

int
privnotify(cmd, cur, new)
int		cmd;
struct var	*cur;
struct var	*new;
{
	if (cmd == CFGV_PREPARE)
		/* is a state change attempted? */
		if (cur->v_leastpriv != new->v_leastpriv)
		{
			/* there is only one valid state change */
			if ((cur->v_leastpriv != 0) ||
			    (new->v_leastpriv != 1))
				return(LEASTPRIV_OFFSET);
		}
	return(0);
}
