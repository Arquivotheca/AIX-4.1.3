static char sccsid[] = "@(#)88	1.21.1.10  src/bos/kernel/pfs/xix_pcl.c, syspfs, bos411, 9439C411e 9/30/94 12:59:50";
/*
 * COMPONENT_NAME:  (SYSPFS) Physical File System
 *
 * FUNCTIONS:  jfs_getpcl(), jfs_setpcl()
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

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/malloc.h"
#include "sys/user.h"

#define DPRINTF(args)

static int pcl_check(struct pcl *pcl, int len);
static int uiosiz(struct uio *uiop);

/*
 * NAME:	jfs_getpcl(vp, uiop, crp)
 *
 * FUNCTION:	Return PCL of file
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  to get the PCL from
 *		uiop	- address of a uio structure in which to put the PCL
 *		crp	- credential
 *
 * RETURN :	0 if successful, errno otherwise
 */

int
jfs_getpcl(struct vnode	*vp,
	   struct uio	*uiop,
	   struct ucred	*crp)
{
	int	rc;
	struct inode	*ip;	/* the inode of interest */
	struct inode	*ixip = NULL;	/* the .inodex segment */
	caddr_t	saddr;		/* address of inodex segment */
	struct pcl	*pclptr;	/* PCL on disk */
	struct pcl	pclhead;	/* If a disk PCL doesn't exist */ 
	label_t	jb;		/* jump buf pointer */

	DPRINTF(("jfs_getpcl() starting\n"));
	ip = VTOIP(vp);
	IWRITE_LOCK(ip);

	/*
	 * if a PCL doesn't exist for this file or this is a symbolic
	 * link then just return the header
	 */
	if (((ip->i_privflags & PCL_EXTENDED) == 0) ||
	     (S_ISLNK(ip->i_mode)))
	{
		pclhead.pcl_len = PCL_SIZ;
		pclhead.pcl_mode = ip->i_mode;
		if (S_ISLNK(ip->i_mode))
		{
			pclhead.pcl_default.pv_priv[0] = 0;
			pclhead.pcl_default.pv_priv[1] = 0;
		}
		else
			pclhead.pcl_default = ip->i_priv;
		pclptr = &pclhead;
	}
	else
	{
		DPRINTF(("jfs_getpcl():  ip->i_privoffset = 0x%x\n", 
							ip->i_privoffset));
		ixip = ip->i_ipmnt->i_ipinodex;
		/* get addressibility to the pcl */
		if (iptovaddr(ixip, 1, &saddr))
		{
			ixip = NULL;
			goto xg_outx;
		}
		pclptr = (struct pcl *)((int)saddr + (int)(ip->i_privoffset));

		IWRITE_LOCK(ixip);
	}

	/*
	 * pclptr may or may not be pointing into a mapped file.
	 * regardless, set up error handler just in case.
	 */
	if (rc = setjmpx(&jb))
		/* an error occurred */
		switch (rc)
		{
		    case ENOSPC:
			rc = ENOMEM;	/* ENOSPC means something special */
		    case ENOMEM:
		    case EIO:
			goto xg_outx;
		    default:
			assert(0);
		}

	if (rc = pcl_check(pclptr, pclptr->pcl_len))
		goto xg_out;

	/*
	 * if the user's recieve buffer is smaller than the PCL, try
	 * to inform the user of the necessary size and return ENOSPC
	 */

	DPRINTF (("jfs_getpcl(): checking size\n"));
	if (uiosiz(uiop) < pclptr->pcl_len)
	{
		int	len;

		DPRINTF(("jfs_getpcl():  uiosiz=%d, len=%d\n", uiosiz(uiop), pclhead.pcl_len));
		len = pclptr->pcl_len;
		uiop->uio_offset = 0;
		uiop->uio_iov->iov_len = uiop->uio_resid = sizeof(len);
		rc = uiomove((caddr_t)&len, (int)sizeof(len), 
				(enum uio_rw)UIO_READ, (struct uio *)uiop);
		if (rc == 0)
			rc = ENOSPC;
		goto xg_out;
	}

	rc = uiomove((caddr_t)pclptr, (int)pclptr->pcl_len, 
				(enum uio_rw)UIO_READ, (struct uio *)uiop);

xg_out:
	clrjmpx(&jb);	/* pop exception return */
xg_outx:
	if (ixip)
	{
		IWRITE_UNLOCK(ixip);
		DPRINTF(("jfs_getpcl():  unbinding \".inodex\"\n"));
		ipundo(saddr);
	}

	if (rc)
	{
		DPRINTF(("jfs_getpcl() failed:  errno=%d\n", rc));
	}
	else
	{
		DPRINTF(("jfs_getpcl() successful\n"));
	}

	IWRITE_UNLOCK(ip);
	return(rc);
}


/*
 * NAME:	jfs_setpcl(vp, uiop, crp)
 *
 * FUNCTION:	Store PCL of file
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  the PCL is for
 *		uiop	- address of a uio structure containing the PCL
 *		crp	- credential
 *
 * RETURN :	0 if successful, errno otherwise
 */

int
jfs_setpcl(struct vnode	*vp,
	   struct uio	*uiop,
	   struct ucred	*crp)
{
	int	rc=0;
	struct inode *ip;
	int	len;
	caddr_t	saddr;
	struct inode	*ixip = NULL;	/* the inode extension segment */
	struct pcl	*newpcl = NULL;
	ulong	newmode;
	label_t	jb;	/* jump buf pointer */

	DPRINTF(("jfs_setpcl() starting\n"));
	ip = VTOIP(vp);
	IWRITE_LOCK(ip);

	/* get the new PCL length */
	len = uiosiz(uiop);
	if (len < PCL_SIZ)
	{
		DPRINTF(("jfs_setpcl():  bad length:  %d\n", len));
		rc = EINVAL;
		goto out;
	}

	/* allocate space for the new pcl */
	newpcl = (struct pcl *)malloc((uint)len);
	if (newpcl == NULL)
	{
		DPRINTF(("setpcl():  malloc failed!!! (len=%d)\n", len));
		rc = ENOMEM;
		goto out;
	}

	/* copy the new pcl and check it out ... */
	DPRINTF (("jfs_setpcl():  copying down and checking it out\n"));
	if ((rc = uiomove((caddr_t)newpcl, (int)len, 
				(enum uio_rw)UIO_WRITE, (struct uio *)uiop))  ||
	    (rc = pcl_check(newpcl, len)))
	{
		DPRINTF(("setpcl():  copy or check failed!!! (rc=%d)\n", rc));
		goto out;
	}
	
	/* if an PCL is already present, free it */
	if ((ip->i_privflags & PCL_EXTENDED)	&&
	    (ip->i_privoffset))
	{
		DPRINTF(("jfs_setpcl():  (free) extended pcl\n"));
		ixip = ip->i_ipmnt->i_ipinodex;
		/* map .inodex into vm */
		iptovaddr(ixip, 1, &saddr);

                /* Since we will need a transaction block, gain it before 
		 * locking .inodex.  Otherwise we could deadlock due to
		 * blocking on syncwait and having remove/rename block on 
		 * .inodex lock.
                 */
                if (u.u_fstid == 0)
                        vcs_gettblk(&rc);

		IWRITE_LOCK(ixip);

		if ((rc = setjmpx(&jb)) == 0)
		{
			smap_free(ixip, saddr, ip->i_privoffset);
			clrjmpx(&jb);
		}
		else
			switch (rc)
			{
			    case ENOSPC:
			    case ENOMEM:
			    case EIO:
				break;
			    default:
				assert(0);
			}
		ip->i_privflags = 0;
		ip->i_privoffset = 0;
	}
			
	if (newpcl->pcl_len > PCL_SIZ)
	{
		DPRINTF(("jfs_setpcl():  extended pcl\n"));
		if (ixip == NULL)
		{
			ixip = ip->i_ipmnt->i_ipinodex;
			/* map .inodex into vm */
			iptovaddr(ixip, 1, &saddr);

                	if (u.u_fstid == 0)
                        	vcs_gettblk(&rc);

			IWRITE_LOCK(ixip);
		}
		if ((rc = setjmpx(&jb)) == 0)
		{
			smap_alloc(ixip, saddr, len, &(ip->i_privoffset));
			/* get transaction lock on pcl */
			vm_gettlock(saddr+ip->i_privoffset, newpcl->pcl_len);
			bcopy(newpcl, saddr+ip->i_privoffset, newpcl->pcl_len);
			ip->i_privflags |= PCL_EXTENDED;
			clrjmpx(&jb);
		}
		else
			switch (rc)
			{
			    case ENOSPC:
			    case ENOMEM:
			    case EIO:
				ip->i_priv = newpcl->pcl_default;
				ip->i_privflags &= ~PCL_EXTENDED;
				break;
			    default:
				assert(0);
			}
	}
	else
	{
		ip->i_priv = newpcl->pcl_default;
		ip->i_privflags &= ~PCL_EXTENDED;
	}
	
	ip->i_mode &= ~(S_ITCB|S_ITP);
	ip->i_mode |= newpcl->pcl_mode & (S_ITCB|S_ITP);

	if(newpcl->pcl_mode & S_ITCB){
        	ITOGP(ip)->gn_flags |= GNF_TCB;
	}
	else {
        	ITOGP(ip)->gn_flags &= ~GNF_TCB;
	}

out:
	if (newpcl)
		free((void *)newpcl);

	if (ixip)
	{
		ipundo(saddr);
		DPRINTF(("jfs_setpcl(): committing ixip and ip\n"));
		if (rc == 0)
		{
			imark(ixip, IFSYNC);
			imark(ip, ICHG);
			commit(2, ixip, ip);
		}
		IWRITE_UNLOCK(ixip);
		ipundo(saddr);
	}
	else
	{
		if (rc == 0)
		{
			DPRINTF(("jfs_setpcl(): committing ip\n"));
			imark(ip, ICHG);
			commit(1, ip);
		}
	}

	IWRITE_UNLOCK(ip);
	return(rc);
}


/*
 * NAME:	pcl_check(pcl, len)
 *
 * FUNCTION:	perform minimal correctness checking on an PCL:
 *			1) pcl->pcl_len == len and pcl_len > 0
 *			2) pcl->pcl_len == sum of pce lengths
 *			3) each pce->pce_len == sum of pce's id lengths
 *
 * PARAMETERS:	pcl	- a pointer to the access control list to
 *			  be checked
 *		len	- the advertised length of this list
 *
 * RETURN :	0 if successful, otherwise EINVAL
 */

pcl_check(struct pcl	*pcl,
	  int		len)
{
	struct pcl_entry   *pce;
	struct pcl_entry   *pcl_end;
	int	pcl_len;	/* sum of pce lengths */

	DPRINTF(("pcl_check():\n"));
	if (pcl->pcl_len < PCL_SIZ)
	{
		DPRINTF (("pcl_check(): pcl_len < PCL_SIZ\n"));
		return (EINVAL);
	}
	if (pcl->pcl_len != len)
	{
		DPRINTF(("pcl_check(): lengths don't match\n"));
		return(EINVAL);
	}

	/* foreach pcl entry ... */
	pcl_len = 0;
	pcl_end = pcl_last(pcl);
	for (pce = pcl->pcl_ext; pce < pcl_end; pce = pcl_nxt(pce))
	{
		struct pce_id	*id;
		struct pce_id	*id_end;
		int	id_len;

		/* check total length of pcl entries */
		if (pce->pce_len < ((int) (((struct pcl_entry *) 0)->pce_id)))
		{
			DPRINTF (("pcl_check(): pce_len < PCE_SIZ\n"));
			return EINVAL;
		}
		pcl_len += pce->pce_len;
		if (pcl_len > len)
		{
			DPRINTF(("pcl_check(): pcl_len != sum(pce_len)\n"));
			return(EINVAL);
		}

		/* foreach id in the pcl entry ... */
		id_len = 0;
		id_end = pcl_id_last(pce);
		for (id = pce->pce_id; id < id_end; id = pcl_id_nxt(id))
		{
			if (id->id_len < sizeof *id) {
				DPRINTF (("pcl_check(): pce_id_len < ID_SIZ"));
				return EINVAL;
			}
			id_len += id->id_len;
			if (id_len > len)
			{
				DPRINTF(("pcl_check(): pce != sum(id_len)\n"));
				return(EINVAL);
			}
		}
	}
	return (0);
}


/*
 * NAME:	uiosiz (uiop)
 *
 * FUNCTION:	Return the total length of the buffers in a uio
 *
 * PARAMETERS:	uiop	- the address of the uio structure
 *
 * RETURN :	the length of the uio structure
 */
uiosiz(struct uio	*uiop)
{
	struct iovec	*iov;
	int	i;
	int	len=0;

	iov = uiop->uio_iov;
	for (i=0; i<uiop->uio_iovcnt; i++)
		len += iov[i].iov_len;
	return(len);
}
