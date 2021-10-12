static char sccsid[] = "@(#)93	1.26  src/bos/kernel/pfs/dqsubs.c, syspfs, bos41J, 9511A_all 3/7/95 13:53:06";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: 	getinoquota, putinoquota, freeiq, allociq, ckiq, chowndq,
 *		dqinit, quotaon, quotaoff, getquota, setquota, setuse, qsync,
 *		dqget, dqput, dqsync, dqread, dqwrite, dqflush,
 *		dquhash, getdq, qfino, dqumount, dqactivity, dqmsg
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include  "jfs/jfslock.h"
#include  "jfs/commit.h"
#include  "jfs/fsvar.h"
#include  "sys/systm.h"
#include  "sys/unixmsg.h"
#include  "sys/lkup.h"
#include  "sys/errno.h"
#include  "sys/uprintf.h"
#include  "sys/vfs.h"
#include  "sys/sleep.h"
#include  "sys/syspest.h"
#include  "sys/sysinfo.h"
#include  "sys/malloc.h"

extern int nhino;
extern struct hinode *hinode;

/*
 *	dquot cache
 */
int dqcachemax;			/* maximum number of dquots in system */
int dqcachesize;		/* number of dquots currently in cache */
int dqmaxcachesize;		/* maximum number of dquot allowed in cache */

struct genalloc *dquot_ap;	/* dquot allocation pointer */

/*
 *	lru dquot cache list.
 *
 * offsets of dqc_next and dqc_prev must be same as in the dquot.
 */
struct {
	struct dquot *dqc_forw;		/* hash list offset, not used */
	struct dquot *dqc_back;		
	struct dquot *dqc_next;		/* dquot cache list offsets */
	struct dquot *dqc_prev;
} dqcache;

/*
 *	dquot hash list anchor table
 */
struct hdquot *hdquot;		/* dquot hash table */

# define	NULL_DQHASH2(dp)	((dp)->dq_next = (dp)->dq_prev = (dp))

/*
 *	global quota lock
 */
Complex_lock	jfs_quota_lock;	

int dqinitial = 0;

int qfile_syncing = 0;		/* dquot for quota file being written */
int qfile_event = EVENT_NULL;


/*
 * NAME:	dqinit ()
 *
 * FUNCTION:	initialize the dquot table.
 *
 * PARAMETERS:	NONE
 *
 * RETURN :	0 	- success
 * 		ENOMEM	- out of memory 
 *			
 */

dqinit()
{
	extern struct fsvar fsv;

	struct hdquot *dhp;
	struct dquot *dqp;
	int n, rc;
	extern uint pfs_memsize();

	/* get storage for the hash table.
	 */
	hdquot = (struct hdquot *) malloc(sizeof(struct hdquot) * NHDQUOT);
	if (hdquot == NULL)
		return(ENOMEM);

	/* initialize the hash table.
	 */
	for (dhp = hdquot; dhp < &hdquot[NHDQUOT]; dhp++)
	{
	 	dhp->dqh_forw = (struct dquot *) dhp;
		dhp->dqh_back = (struct dquot *) dhp;
	}

	/* null hash dqcache.
	 */
	dqcache.dqc_next = (struct dquot *) &dqcache;
	dqcache.dqc_prev = (struct dquot *) &dqcache;

	/* dqcachemax is now set in inoinit() */
	assert(dqcachemax != 0);        

	rc = geninit (dqcachemax, fsv.v_ndquot, sizeof (struct dquot),
		(caddr_t) &dqp->dq_forw - (caddr_t) dqp,"DQUOTS", &fsv.v_dquot);
	if (rc)
	{
		free(hdquot);
		return(ENOMEM);
	}

	/* save dquot allocation pointer */
	dquot_ap = fsv.v_dquot;

	/* Limit maximum dquot cache size to initial allocation */
	dqmaxcachesize = fsv.v_ndquot;

	return(0);
}

/*
 * NAME:	quotaon (jmp, type, fname, crp)
 *
 * FUNCTION:	VFS_QUOTACTL(Q_QUOTAON)
 *		enable the specified type of quotas for a file system.
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *		fname	- user supplied path name of quota file
 *		crp	- credential
 *
 * RETURN :	0 	- success
 *		EACCES	- quota file is not a reqular file 
 *		errors from subroutines
 *			
 */

quotaon(jmp, type, fname, crp)
struct jfsmount	*jmp;
int		type;
caddr_t		fname;
struct ucred	*crp;		/* pointer to credential structure */
{
	struct vnode *vp;
	struct dquot *dp;
	struct inode *ip, *next, **ipp;
	struct hinode *hip;
	uid_t id;
	caddr_t vinfo;
	time_t btime;
	dev_t  dev;
	int rc;

	QUOTA_WRITE_LOCK();
	QUOTA_LOCK_RECURSIVE();

	/* perform quota initialization if not yet performed.
	 */
	if (dqinitial == 0)
	{
		if (rc = dqinit())
			goto out1;
		dqinitial = 1;
	}

	/* lookup the quota file.
	 */
	if (rc = lookupname(fname, USR, L_SEARCH, NULL, &vp, crp))
		goto out1;

	/* check for a regular file.
	 */
	if (vp->v_vntype != VREG)
	{
		VNOP_RELE(vp);
		goto out1;
	}

	/* open the quota file.
	 */
	if (rc = VNOP_OPEN(vp, FREAD|FWRITE, 0, &vinfo, crp))
	{
		VNOP_RELE(vp);
		goto out1;
	}

	crhold(crp);

	/* if quotas are currently enabled with a different
	 * quota file, disable them.
	 */
	if (jmp->jm_quotas[type] != NULL)
		quotaoff(jmp,type);

	ICACHE_LOCK();

	/* hold and record the credentials for the user enabling
	 * quotas.  the credentials will be required when reading
	 * and writing the quota file.  save the vnode pointer for
	 * the quota file.
	 */
	jmp->jm_cred[type] = crp;	
	jmp->jm_quotas[type] = vp;

	/* set up quotas for the quota file if it resides within
	 * the file system.
	 */
	if (qfino(jmp,type,&ip))
	{
		id = (type == USRQUOTA) ? ip->i_uid : ip->i_gid;
		if (rc = dqget(jmp, id, type, &ip->i_dquot[type]))
			goto out;
	}

	/* get the dquot for the superuser.
	 */
	if (rc = dqget(jmp, 0, type, &dp))
		goto out; 

	/* set time limits for soft limits.  if the time limits for
	 * the superuser are non-zero, use these time limits for
	 * the soft limits.
	 */
	if ((btime = DQBTIME(dp->dq_btime)) > 0)
		jmp->jm_btime[type] = btime;
	else
		jmp->jm_btime[type] = MAX_DQ_TIME;

	if (dp->dq_itime > 0)
		jmp->jm_itime[type] = dp->dq_itime;
	else
		jmp->jm_itime[type] = MAX_IQ_TIME;

	dqput(dp);

	/*
	 * scan inode cache, getting dquots for active inodes.
	 */
	dev = jmp->jm_dev;
	for (hip = hinode; hip < &hinode[nhino]; hip++)
	{
	loop:
		for (ip = hip->hi_forw; ip != (struct inode *)hip; ip = next)
		{
			next = ip->i_forw;

			/* check if the inode is within the file system.
			 */
			if (ip->i_dev != dev)
				continue;

			/* ignore special inodes.
			 */
			if (ISPECIAL(ip))
				continue;

			/* check if the inode has a dquot.
			 */
			if (ip->i_dquot[type] != NODQUOT)
				continue;

			/* Skip inodes where uid or gid is out of range 
			 */
			id = (type == USRQUOTA) ? ip->i_uid : ip->i_gid;
			if (!DQVALID(id))
				continue;

			/* get and lock the inode.
			 * _iget() is easy way out for waiting for
			 * inode in transformation instead of
			 * replicating _iget()'s wait logic.
			 * we are ok if _iget() fails - the inode will
			 * get a dquot on subsequent _iget()s.
			 */
			sysinfo.iget++;
			cpuinfo[CPUID].iget++;
			if (_iget(dev, ip->i_number, hip, &ipp, 1, NULL))
				goto loop;

			if (ip->i_mode != 0)
			{
				ICACHE_UNLOCK();
				IWRITE_LOCK(ip);
				COMBIT_LOCK(ip);
				ICACHE_LOCK();

				/* get the dquot for this inode.
				 */
				rc = getinoquota(ip);

				COMBIT_UNLOCK(ip);
				IWRITE_UNLOCK(ip);
			}

			iput(ip,NULL);

			if (rc)
				goto out; 

			/* start from the top of the chain.
			 */	
			goto loop;
		}
	}

out:
	ICACHE_UNLOCK();
	if (rc)
		quotaoff(jmp,type);
out1:
	QUOTA_LOCK_UNRECURSIVE();
	QUOTA_UNLOCK();
	return(rc);
}

/*
 * NAME:	quotaoff (jmp, type)
 *
 * FUNCTION:	VFS_QUOTACTL(Q_QUOTAOFF)
 *		disable the specified type of quotas for a file system.
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *
 * RETURN :	0
 *			
 */

quotaoff(jmp, type)
struct jfsmount *jmp;
int type;
{
	struct vnode *vp;
	struct hinode *hip;
	struct inode *ip, *next, **ipp, *ipdq = NULL;
	struct dquot *dp;
	struct ucred *crp;
	dev_t  dev;
	caddr_t vinfo;
	int rc, lockt;

	/* grab global disk quota lock.
	 */
	QUOTA_WRITE_LOCK();

	/* check if the quotas are already disabled.
	 */
	if ((vp = jmp->jm_quotas[type]) == NULL)
	{
		QUOTA_UNLOCK();
		return(0);
	}

	ICACHE_LOCK();

	/* mark quota file closing so no more dquots will be gotten.
	 */ 
 	jmp->jm_qflags[type] |= QTF_CLOSING;

	/* get ip for the quota file if it is within the file 
	 * system.
	 */
	if (qfino(jmp,type,&ip))
		ipdq = ip;

	/*
	 * scan inode cache, releasing dquots.
	 */
	dev = jmp->jm_dev;
	for (hip = hinode; hip < &hinode[nhino]; hip++)
	{
	loop:
		for (ip = hip->hi_forw; ip != (struct inode *)hip; ip = next)
		{
			next = ip->i_forw;

			if (ip->i_dev != dev)
				continue;

			/* check if the inode has a dquot.
			 */
			if (ip->i_dquot[type] == NODQUOT &&
						!(ip->i_locks & IQUOTING))
				continue;

			/* ignore the quota file. we will take care of
			 * it later.  
			 */
			if (ip == ipdq)
				continue;

			/* get and lock the inode.  if _iget() fails
			 * the dquot for this inode has been put.
			 */
			sysinfo.iget++;
			cpuinfo[CPUID].iget++;
			if (_iget(dev, ip->i_number, hip, &ipp, 1, NULL))
				goto loop;

			ICACHE_UNLOCK();
			IWRITE_LOCK(ip);
			COMBIT_LOCK(ip);
			ICACHE_LOCK();

			/* must check for a dquot again - it might have
			 * been put.
			 */
			if ((dp = ip->i_dquot[type]) != NODQUOT)
				ip->i_dquot[type] = NODQUOT;

			COMBIT_UNLOCK(ip);
			IWRITE_UNLOCK(ip);

			iput(ip,NULL);

			/* put the dquot and start over.
			 */
			if (dp != NODQUOT)
				dqput(dp);

			goto loop;
		}
	}

	/* take care of dquot for quota file inode. we handle it now
	 * because we might have been modifying it above (dqput()s).
	 * we must check if the inode has a dquot - quotaoff() might
	 * have been called by quotaon() because a dquot could not
	 * be gotten for the quota file inode.
	 */
	if (ipdq && ipdq->i_dquot[type] != NODQUOT )
	{
		/* sync it before putting it.
		 */
		dp = ipdq->i_dquot[type];
		dqsync(dp);

		/* mark inode as having no dquot.
		 */
		ICACHE_UNLOCK();
		IWRITE_LOCK(ipdq);
		COMBIT_LOCK(ipdq);
		ICACHE_LOCK();

		ipdq->i_dquot[type] = NODQUOT;

		COMBIT_UNLOCK(ipdq);
		IWRITE_UNLOCK(ipdq);

		/* put it.
		 */
		dqput(dp);
	}

	/* remove all dquot hash and cache entries for this file
	 * system and quota type.
	 */
	dqflush(dev,type);

	/* quotas disabled.
	 */
	crp = jmp->jm_cred[type];
	jmp->jm_cred[type] = NULL;
	jmp->jm_quotas[type] = NULL;
        jmp->jm_qflags[type] &= ~QTF_CLOSING;

	ICACHE_UNLOCK();

	/* close the quota file and release the vnode.
	 */
	VNOP_CLOSE(vp, FREAD|FWRITE, vinfo, crp);
	VNOP_RELE(vp);

	crfree(crp);

	QUOTA_UNLOCK();
	return(0);
}

/*
 * NAME:	dqflush (dev, type)
 *
 * FUNCTION:	uncache and unhash all dquots for a file system and
 *		quota type.
 *		when disabling a quota type for a file system. dquots
 *		with i/o in progess will be added to the pending delete
 *		list.
 *
 *		called only by quotaoff().
 *
 * PARAMETERS:	dev	- devid of the file system
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *
 * RETURN :	NONE
 *
 */

dqflush(dev,type)
dev_t dev;
int type;
{
	struct hdquot *dhp;
	struct dquot *dp, *next;

	/* 
	 * scan dquots cache, uncaching and unhashing dquots
	 * for the file system and quota type.
	 */
	for (dhp = hdquot; dhp < &hdquot[NHDQUOT]; dhp++)
	{
		for (dp = dhp->dqh_forw; dp != (struct dquot *)dhp; dp = next)
		{
			next = dp->dq_forw;

			if (dp->dq_dev != dev || dp->dq_type != type)
				continue;

			assert(dp->dq_cnt == 0);
			assert(!(dp->dq_flags & DQ_MOD));

			dqcachesize--;
			remque2(dp);
			NULL_DQHASH2 (dp);
			dquhash(dp);
		}
	}
}

/*
 * NAME:	getquota (jmp, id, type, addr)
 *
 * FUNCTION:	VFS_QUOTACTL(Q_GETQUOTA)
 *		return current quota limit and usage values for an
 *		id and quota type within a file system. values are
 *		returned in a dqblk structure.
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		id	- uid or gid (interpetted based on type)
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *		addr	- address of user supplied dqblk buffer
 *
 * RETURN :	0 - success
 *		errors from subroutines
 *			
 */

getquota(jmp, id, type, addr)
struct jfsmount *jmp;
uid_t id;
int type;
caddr_t addr;
{
	struct dquot *dp;
	struct dqblk dq;
	int rc;

	QUOTA_READ_LOCK();

	/* get the dquot for the id and quota type.
	 */
	ICACHE_LOCK();
	rc = dqget(jmp, id, type, &dp);
	ICACHE_UNLOCK();
	if (rc)
		goto out;

	/* get a copy of the dqblk.
	 */
	DQUOT_LOCK(dp);

	dq = dp->dq_dqb;

	DQUOT_UNLOCK(dp);

	ICACHE_LOCK();
	dqput(dp);
	ICACHE_UNLOCK();

	/* if the dqblk has a partial usage block, report it as a
	 * full block.
	 */
	if (DQCARRY(dq.dqb_btime))
	{
		dq.dqb_curblocks++;
		dq.dqb_btime = DQBTIME(dq.dqb_btime);
	}

	/* copy the dqblk info to the user buffer.
	 */
	rc = copyout((caddr_t)&dq, addr, sizeof (struct dqblk));

out:
	QUOTA_UNLOCK();
	return(rc);
}

/*
 * NAME:	setquota (jmp, id, type, addr)
 *
 * FUNCTION:	VFS_QUOTACTL(Q_SETQUOTA)
 *		set new quota limit values for an id and quota type within
 *		a file system.  new values are obtained from a user supplied
 *		dqblk structure.
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		id	- uid or gid (interpetted based on type)
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *		addr	- address of user supplied dqblk buffer
 *
 * RETURN :	0	- success
 *		errors from subroutines
 *			
 */

setquota(jmp, id, type, addr)
struct jfsmount *jmp;
uid_t id;
int type;
caddr_t addr;
{
	struct dquot *dp;
	struct dqblk newlim;
	int rc = 0;

	/* copy the new limits from the user buffer.
	 */
	if (rc = copyin(addr, (caddr_t)&newlim, sizeof (struct dqblk)))
		return(rc);

	QUOTA_READ_LOCK();

	/* get the dquot for the id and quota type.
	 */
	ICACHE_LOCK();
	rc = dqget(jmp, id, type, &dp);
	ICACHE_UNLOCK();
	if (rc)
		goto out;	

	DQUOT_LOCK(dp);

	/* set time limits for soft limits if over new soft limits
	 * and under old soft limits.
	 */
	if (newlim.dqb_bsoftlimit &&
	    dp->dq_curblocks >= newlim.dqb_bsoftlimit &&
	    (dp->dq_bsoftlimit == 0 || dp->dq_curblocks < dp->dq_bsoftlimit))
		dp->dq_btime = DQSBTIME(DQCARRY(dp->dq_btime), 
						time + jmp->jm_btime[type]);

	if (newlim.dqb_isoftlimit &&
	    dp->dq_curinodes >= newlim.dqb_isoftlimit &&
	    (dp->dq_isoftlimit == 0 || dp->dq_curinodes < dp->dq_isoftlimit))
		dp->dq_itime = time + jmp->jm_itime[type];

	/* update hard and soft limits.
	 */
	dp->dq_bhardlimit = newlim.dqb_bhardlimit;
	dp->dq_bsoftlimit = newlim.dqb_bsoftlimit;
	dp->dq_ihardlimit = newlim.dqb_ihardlimit;
	dp->dq_isoftlimit = newlim.dqb_isoftlimit;

	/* if quota is for superuser, update time limits.  the
	 * superuser's time limits are used to set the file system
	 * level time limits when quotas are enabled.
	 */
	if (dp->dq_id == 0)
	{
		dp->dq_btime = DQSBTIME(DQCARRY(dp->dq_btime),
						newlim.dqb_btime);
		dp->dq_itime =  newlim.dqb_itime;
	}

	/* reset over limit flags if under new soft limit.
	 */
	if (dp->dq_curblocks < dp->dq_bsoftlimit)
		dp->dq_flags &= ~DQ_BLKS;

	if (dp->dq_curinodes < dp->dq_isoftlimit)
		dp->dq_flags &= ~DQ_INODS;

	/* mark as modified.
	 */
	dp->dq_flags |= DQ_MOD;

	DQUOT_UNLOCK(dp);
	
	ICACHE_LOCK();
	dqput(dp);
	ICACHE_UNLOCK();

out:
	QUOTA_UNLOCK();
	return(rc);
}

/*
 * NAME:	setuse (jmp, id, type, addr)
 *
 * FUNCTION:	VFS_QUOTACTL(Q_SETUSE)
 *		set current usage values for an id and quota type within
 *		a file system.  new usage values are obtained from a user
 *		supplied dqblk structure.
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		id	- uid or gid (interpetted based on type)
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *		addr	- address of user supplied dqblk buffer
 *
 * RETURN :	0	- success
 *		errors from subroutines
 *
 */

setuse(jmp, id, type, addr)
struct jfsmount *jmp;
uid_t id;
int type;
{
	struct dquot *dp;
	struct dqblk usage;
	int rc = 0;

	/* copy the new usage values from the user buffer.
	 */
	if (rc = copyin(addr, (caddr_t)&usage, sizeof (struct dqblk)))
		return(rc);

	QUOTA_READ_LOCK();

	/* get the dquot for the id and quota type.
	 */
	ICACHE_LOCK();
	rc = dqget(jmp, id, type, &dp);
	ICACHE_UNLOCK();
	if (rc)
		goto out;

	DQUOT_LOCK(dp);

	/* set time limits for soft limits if previously under soft limit
	 * and now over soft limit due to new current usage.
	 */
	if (dp->dq_bsoftlimit && dp->dq_curblocks < dp->dq_bsoftlimit &&
	    usage.dqb_curblocks >= dp->dq_bsoftlimit)
		dp->dq_btime = time + jmp->jm_btime[type];

	if (dp->dq_isoftlimit && dp->dq_curinodes < dp->dq_isoftlimit &&
	    usage.dqb_curinodes >= dp->dq_isoftlimit)
		dp->dq_itime = time + jmp->jm_itime[type];

	/* update current usage.
	 */
	dp->dq_curblocks = usage.dqb_curblocks;
	dp->dq_btime = DQSBTIME(0,dp->dq_btime);
	dp->dq_curinodes = usage.dqb_curinodes;

	/* reset over limit flags if new current usage is
	 * under the soft limits.
	 */
	if (dp->dq_curblocks < dp->dq_bsoftlimit)
		dp->dq_flags &= ~DQ_BLKS;

	if (dp->dq_curinodes < dp->dq_isoftlimit)
		dp->dq_flags &= ~DQ_INODS;

	/* mark as modified.
	 */
	dp->dq_flags |= DQ_MOD;

	DQUOT_UNLOCK(dp);

	ICACHE_LOCK();
	dqput(dp);
	ICACHE_UNLOCK();

out:
	QUOTA_UNLOCK();
	return(rc);
}

/*
 * NAME:	qsync (jmp)
 *
 * FUNCTION:	VFS_QUOTACTL(Q_QSYNC)
 *		sync in-core quota information to the quota files.  this
 *		routine is called by jfs_quotactl() to sync the quotas for
 *		a specific file system (a pointer to the mount structure of
 *		the file system is passed) or by jfs_sync() to sync the quotas
 *		for all file systems (zero is passed).
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system or
 *			  zero
 *
 * RETURN :	0
 *			
 */

qsync(jmp)
struct jfsmount *jmp;
{
	struct hdquot *dhp;
	struct dquot *dp, *next;
	dev_t  dev;

	/* return if quotas have been enabled.
	 */
	if (dqinitial == 0)
		return(0);

	/* grab global disk quota lock.
	 */
	QUOTA_READ_LOCK();

	/* get the devid for the file system (zero if quotas are to
	 * be sync for all file systems).
	 */
	if (jmp)
		dev = jmp->jm_dev;
	else
		dev = NODEVICE;

	ICACHE_LOCK();

	/* scan dquots cache, sync the dquots.
	 * as we travel down each chain, the next dquot's count is 
	 * incremented before putting the current dquot - this allows
	 * us to maintain our position within the chain if dqput() sleeps.
	 */
	for (dhp = hdquot; dhp < &hdquot[NHDQUOT]; dhp++)
	{
		/* anything on the chain ?
		 */
		if ((dp = dhp->dqh_forw) == (struct dquot *)dhp)
			continue;

		/* increment count. remove from cache if
		 * count was 0.
		 */
		dp->dq_cnt++;
		if (dp->dq_cnt == 1)
		{
			dqcachesize--;
			remque2(dp);
			NULL_DQHASH2 (dp);
		}

		do
		{
			/* sync the dquot. ignore cached and unmodified
			 * dquots and dquots not associated with the dev.
			 */
			if ((dev == NODEVICE || dp->dq_dev == dev) &&
			    !(dp->dq_flags & (DQ_QFLOCK|DQ_IOERROR)) &&
			    (dp->dq_flags & DQ_MOD))
				dqsync(dp);

			/* get next before putting the dquot.
			 */
			if ((next = dp->dq_forw) != (struct dquot *)dhp)
			{
				next->dq_cnt++;
				if (next->dq_cnt == 1)
				{
					dqcachesize--;
					remque2(next);
					NULL_DQHASH2 (next);
				}
			}

			dqput(dp);
			dp = next;

		} while (dp != (struct dquot *) dhp);
	}

	ICACHE_UNLOCK();
	QUOTA_UNLOCK();
	return(0);
}

/*
 * NAME:	getinoquota (ip)
 *
 * FUNCTION:	setup user and group quotas for an inode.
 *
 *		called by _iget(), dir_ialloc(), quotaon(), chowndq()
 *
 * PARAMETERS:	ip - pointer to the inode
 *
 * RETURN :	0 - ok
 *		errors from subroutines
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */

getinoquota(ip)
struct inode *ip;
{
	struct jfsmount *jmp;
	int rc = 0;

	/* return if quotas have not been enabled.
	 */
	if (dqinitial == 0)
		return(0);

	/* no dquots if special inode.
	 */
	if (ISPECIAL(ip))
	{
		ip->i_dquot[USRQUOTA] = NODQUOT;
		ip->i_dquot[GRPQUOTA] = NODQUOT;
		return(0);
	}

	/* get mount structure pointer for the file system.
	 */
	assert(ip->i_ipmnt);
	jmp = ip->i_ipmnt->i_jmpmnt;
	assert(jmp);
	
	ip->i_locks |= IQUOTING;

	/* setup user quota.  EINVAL is returned by dqget() if quotas
	 * are not enabled or quotas do not exist for the uid.
	 */
	if (ip->i_dquot[USRQUOTA] == NODQUOT)
	{
		if (rc = dqget(jmp,ip->i_uid,USRQUOTA,&ip->i_dquot[USRQUOTA]))
		{
			if (rc != EINVAL)
				goto out;
			else
				rc = 0;
		}
	}

	/* setup group quota.  EINVAL is returned by dqget() if quotas
	 * are not enabled or quotas do not exist for the gid.
	 */
	if (ip->i_dquot[GRPQUOTA] == NODQUOT)
	{
		if (rc = dqget(jmp,ip->i_gid,GRPQUOTA,&ip->i_dquot[GRPQUOTA]))
		{
			if (rc != EINVAL)
				putinoquota(ip);
			else
				rc = 0;
		}
	}

out:
	ip->i_locks &= ~IQUOTING;

	return(rc);

}

/*
 * NAME:	putinoquota (ip)
 *
 * FUNCTION:	remove quotas from an inode.
 *
 *		called by _iget(), iunhash(), dir_ialloc() and getinoquota()
 *
 * PARAMETERS:	ip - pointer to the inode
 *
 * RETURN :	1 - slept while putting the quotas
 *		0 - no sleep
 *
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */

putinoquota(ip)
struct inode *ip;
{
	struct dquot *dp;
	int type;
	int rc = 0;

	/* return if quotas have not been enabled.
	 */
	if (dqinitial == 0)
		return(0);

	/* put the dquot and remove dquot pointer from the
	 * inode for each quota type.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
	{
		if ((dp = ip->i_dquot[type]) == NODQUOT)
			continue;

		ip->i_dquot[type] = NODQUOT;
		rc |= dqput(dp);
	}
	
	return(rc);
}

/*
 * NAME:	allociq (ip, newinos, crp)
 *
 * FUNCTION:	check if new disk inodes can be allocated for an inode
 *		and update disk inode quota info for the inode if new
 *		resources can be allocated.
 *
 *		called only by dir_ialloc().
 *
 * PARAMETERS:	ip 	- pointer to the inode
 *		newinos	- number of inode to be allocated
 *		crp	- credential
 *
 * RETURN :	0 	- disk inode allocation should proceed
 *
 *		EDQUOT	- disk inode allocation should not be allowed 
 *			  to proceed
 *			
 * SERIALIZATION: The ICACHE_LOCK is released in dir_ialloc() and taken back
 *		  immediately upon successful return to dir_ialloc().
 */

allociq(ip, newinos, crp)
struct inode	*ip;
int		newinos;
struct ucred	*crp;		/* pointer to credential structure */
{
	int type, rc = 0;
	struct dquot *dp;
	uid_t uid;

	/* return if no disk inodes are being allocated.
	 */
	if (newinos == 0)
		return(0);

	/* if not superuser, determine if the new inodes can be
	 * can be allocated.
	 */
	uid = crp->cr_uid;
	if (uid != 0)
	{
		for (type = 0; type < MAXQUOTAS; type++)
		{
			if ((dp = ip->i_dquot[type]) == NODQUOT)
				continue;
	
			DQUOT_LOCK(dp);
			rc = ckiq(dp, newinos, uid == ip->i_uid);
			DQUOT_UNLOCK(dp);
			if (rc)
				return EDQUOT;
		}
	}

	/* update the quota information for the inode to
	 * reflect the new inodes to be allocated.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
	{
		if ((dp = ip->i_dquot[type]) == NODQUOT)
			continue;

		DQUOT_LOCK(dp);
		dp->dq_curinodes += newinos;
		dp->dq_flags |= DQ_MOD;
		DQUOT_UNLOCK(dp);
	}

	return(rc);
}

/*
 * NAME:	freeiq (ip, nfreed)
 *
 * FUNCTION:	update disk inode usage quota info for an inode
 *		to reflected freed inodes.
 *
 * PARAMETERS:	ip 	- pointer to the inode
 *		nfreed	- number of inode freed
 *
 * RETURN :	NONE
 *			
 */

freeiq(ip, nfreed)
struct inode *ip;
int nfreed;
{
	struct dquot *dp;
	int type;

	/* return if no disk inodes are being freed.
	 */
	if (nfreed == 0)
		return;

	/* update the quota info.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
	{
		if ((dp = ip->i_dquot[type]) == NODQUOT)
			continue;

		DQUOT_LOCK(dp);
		/* cannot have negative allocated inodes
		*/
		if (dp->dq_curinodes >= nfreed)
			dp->dq_curinodes -= nfreed;
		else
			dp->dq_curinodes = 0;
		dp->dq_flags &= ~DQ_INODS;
		dp->dq_flags |= DQ_MOD;
		DQUOT_UNLOCK(dp);
	}

	return;
}

/*
 * NAME:	ckiq ()
 *
 * FUNCTION:	check for a valid change to a user's inode block allocation.
 *		issue a error message if a hard limit is reached or a warning
 *		message if the soft limit is reached.
 *
 *		called by chowndq() and allociq().
 *
 * PARAMETERS:	dp 	- dquot pointer
 *		newinos - number of disk inodes to be allocated
 *		uid	- uid associated with the allocation request.
 *
 * RETURN :	0 	- under disk inode hard limit
 *		EDQUOT	- reached disk inode hard limit
 *			
 */

ckiq(dp, newinos, sendmsg)
struct dquot *dp;
int newinos;
int sendmsg;
{
	int ninodes, msgno;

	ninodes = dp->dq_curinodes + newinos;

	/* check if the new inodes will cause the hard limit to be reached.
	 */
	if (ninodes >= dp->dq_ihardlimit && dp->dq_ihardlimit)
	{
		/* send hard limit error message if appropriate.
		 */
		if (!(dp->dq_flags & DQ_INODS) && sendmsg)
		{
			 msgno = (dp->dq_type == USRQUOTA) ?
                                MSG_SYSPFS_07 : MSG_SYSPFS_08;
                        dqmsg(msgno,dp->dq_dev);
			dp->dq_flags |= DQ_INODS;
		}
		return(EDQUOT);
	}

	/* check if the new inodes will cause the soft limit to be reached.
	 */
	if (ninodes >= dp->dq_isoftlimit && dp->dq_isoftlimit)
	{
		/* send soft limit warning message if appropriate.
		 */
		if (dp->dq_curinodes < dp->dq_isoftlimit)
		{
			dp->dq_itime = time + dp->dq_jmp->jm_itime[dp->dq_type];
			if (sendmsg)
			{
				msgno = (dp->dq_type == USRQUOTA) ?
                                	MSG_SYSPFS_11 : MSG_SYSPFS_12;
                        	dqmsg(msgno,dp->dq_dev);
			}
			return(0);
		}

		/* check if the hard limit has been reached because the
		 * time limit has expired for the soft limit.
		 */
		if (time > dp->dq_itime)
		{
			/* send hard limit error message if appropriate.
			 */
			if (!(dp->dq_flags & DQ_INODS) && sendmsg)
			{
				msgno = (dp->dq_type == USRQUOTA) ?
                                	MSG_SYSPFS_15 : MSG_SYSPFS_16;
                        	dqmsg(msgno,dp->dq_dev);
				dp->dq_flags |= DQ_INODS;
			}
			return(EDQUOT);
		}
	}

	return(0);
}

/*
 * NAME:	ckdq(dp, newfrags, fperpage, uid)
 *
 * FUNCTION:	check for a valid change to a user's or group's disk fragment
 *		allocation. issue a error message if a hard limit is reached
 *		or a warning message if the soft limit is reached.
 *
 *		called only by chowndq().
 *
 * PARAMETERS:	dp 	- dquot pointer.
 *		newfrags - number of disk fragments to be allocated.
 *		fperpage - number of disk fragments per block.
 *		uid	- uid associated with the allocation request.
 *
 * RETURN :	0 	- under disk block hard limit
 *		EDQUOT	- reached disk block hard limit
 *			
 */

ckdq(dp, newfrags, fperpage, sendmsg)
struct dquot *dp;
int newfrags;
int fperpage;
int sendmsg;
{
	int nbytes, npart, nblocks, msgno;

	/* determine the number of DQBSIZE blocks to be check against
	 * the limits.
	 */
	nbytes = newfrags * (PAGESIZE / fperpage);
	nblocks = dp->dq_curblocks + nbytes / DQBSIZE;
	npart = (nbytes & (DQBSIZE-1)) ? 1 : 0;
	nblocks += (npart & DQCARRY(dp->dq_btime));

	/* check if the new blocks will cause the hard limit to be reached.
	 */
	if (nblocks >= dp->dq_bhardlimit && dp->dq_bhardlimit)
	{
		/* send hard limit error message if appropriate.
		 */
		if (!(dp->dq_flags & DQ_BLKS) && sendmsg)
		{
			msgno = (dp->dq_type == USRQUOTA) ?
				MSG_SYSPFS_01 : MSG_SYSPFS_02;
			dqmsg(msgno,dp->dq_dev);
			dp->dq_flags |= DQ_BLKS;
		}
		return(EDQUOT);
	}

	/* check if the new blocks will cause the soft limit to be reached.
	 */
	if (nblocks >= dp->dq_bsoftlimit && dp->dq_bsoftlimit)
	{
		/* send soft limit warning message if appropriate.
		 */
		if (dp->dq_curblocks < dp->dq_bsoftlimit)
		{
			dp->dq_btime = DQSBTIME(DQCARRY(dp->dq_btime),
				time + dp->dq_jmp->jm_btime[dp->dq_type]);
			if (sendmsg)
			{
				msgno = (dp->dq_type == USRQUOTA) ?
					MSG_SYSPFS_05 : MSG_SYSPFS_06;
				dqmsg(msgno,dp->dq_dev);
			}
			return(0);
		}

		/* check if the hard limit has been reached because the
		 * time limit has expired for the soft limit.
		 */
		if (time > DQBTIME(dp->dq_btime))
		{
			/* send hard limit error message if appropriate.
		 	 */
			if (!(dp->dq_flags & DQ_BLKS) && sendmsg)
			{
				msgno = (dp->dq_type == USRQUOTA) ?
					MSG_SYSPFS_13 : MSG_SYSPFS_14;
				dqmsg(msgno,dp->dq_dev);
				dp->dq_flags |= DQ_BLKS;
			}
			return(EDQUOT);
		}
	}

	return(0);
}

/*
 * NAME:	chowndq (ip, newuid, newgid, crp)
 *
 * FUNCTION:	check if disk block and disk inode resources can be
 *		reassigned for an inode.  if so, update quota resource
 *		usage data to reflect the reassignment.
 *
 *		called only by jfs_setattr().
 *
 * PARAMETERS:	ip 	- pointer to the inode
 *		newuid	- new owner uid
 *		newgid	- new owner gid
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		EDQUOT	- hard limit reached 
 *		errors from subroutines 
 *			
 * SERIALIZATION: inode is locked on entry/exit.
 */

chowndq(ip, newuid, newgid, crp)
struct inode	*ip;
uid_t		newuid;
gid_t		newgid;
struct ucred	*crp;		/* pointer to credential structure */
{
	struct inode tino;
	int newid[MAXQUOTAS];
	int oldid[MAXQUOTAS];
	struct dquot *ndp, *odp;
	int type, nblocks, fperpage, nbytes, npart, cpart;
	int sendmsg, rc;
	uid_t uid;

	/* check if there is anything to do.
	 */
	if (ip->i_dquot[USRQUOTA] == NODQUOT &&
	    ip->i_dquot[GRPQUOTA] == NODQUOT)
		return(0);

	/* return (0) if devino.
         */
	if  (ip->i_mode == 0)
		return(0);

	/* save away id info in a form that we can work with.
	 */
	newid[USRQUOTA] = newuid;
	newid[GRPQUOTA] = newgid;
	oldid[USRQUOTA] = ip->i_uid;
	oldid[GRPQUOTA] = ip->i_gid;

	/* fixup a fake inode for the new owner to work with.
	 * get dquots for this inode.  getinoquota() will require
	 * that the fake inode contain the uid, gid, and pointer
	 * to the mount inode.
	 */
	tino.i_uid = newuid;
	tino.i_gid = newgid;
	tino.i_ipmnt = ip->i_ipmnt;
	tino.i_mode = ip->i_mode;
	tino.i_number = ip->i_number;
	tino.i_dquot[USRQUOTA] = NODQUOT;
	tino.i_dquot[GRPQUOTA] = NODQUOT;

	ICACHE_LOCK();
	rc = getinoquota(&tino);
	ICACHE_UNLOCK();
	if (rc)
		return(rc);
 
	/* lock all the dquots that will be accessed. uid dquots
	 * will be locked first, followed by gid dquots.  for each
	 * type of id (uid or gid), the dquot with the largest id
	 * value will be locked first.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
	{
		ndp = tino.i_dquot[type];
		odp = ip->i_dquot[type];

		if (newid[type] > oldid[type])
		{
			if (ndp != NODQUOT)
				DQUOT_LOCK(ndp);
			if (odp != NODQUOT)
				DQUOT_LOCK(odp);
			continue;
		}

		if (newid[type] < oldid[type])
		{
			if (odp != NODQUOT)
				DQUOT_LOCK(odp);
			if (ndp != NODQUOT)
				DQUOT_LOCK(ndp);
		}
	}

	/* get the number of fragments per block.
	 */
	fperpage = ip->i_ipmnt->i_fperpage;

	/* check if the new owner can handle the resources.
	 */
	uid = crp->cr_uid;
	if (uid != 0)
	{
		sendmsg = (uid = newuid) ? 1 : 0;
		for (type = 0; type < MAXQUOTAS; type++)
		{
			/* check for ownership change for this quota type.
			 */
			if (newid[type] == oldid[type])
				continue;

			if ((ndp = tino.i_dquot[type]) == NODQUOT)
				continue;

			/* check for disk blocks.
			 */
			if (rc = ckdq(ndp, ip->i_nblocks, fperpage, sendmsg))
				goto bad;

			/* check for disk inode.
		 	 */
			if (rc = ckiq(ndp, 1, sendmsg))
				goto bad;
		}
	}

	/* get the number of resource to be exchanged.
	 */
	nbytes = ip->i_nblocks * (PAGESIZE / fperpage);
	nblocks = nbytes / DQBSIZE;
	npart = (nbytes & (DQBSIZE-1)) ? 1 : 0;

	/* new owner can handle the resources.  reassign the resources.
	 */
	for (type = 0; type < MAXQUOTAS; type++)
	{
		/* check for a change in ownership for this quota type.
		 */
		if (newid[type] != oldid[type])
		{

			/* take resources from old owner.
			 */
			if ((odp = ip->i_dquot[type]) != NODQUOT)
			{
				/* cannot have negative inodes */
				if (odp->dq_curinodes > 0)
					odp->dq_curinodes -= 1;
				else
					odp->dq_curinodes = 0;
				odp->dq_flags &= ~DQ_INODS;
				if (nbytes != 0)
				{
					cpart = DQCARRY(odp->dq_btime);
					/* cannot have negative blocks */
					if (odp->dq_curblocks >= 
					   (nblocks + (npart & cpart) - npart))
						odp->dq_curblocks -= (nblocks +
						      (npart & cpart) - npart);
					else
						odp->dq_curblocks = 0;
					odp->dq_btime =
						DQSBTIME((npart ^ cpart),
						odp->dq_btime);
					odp->dq_flags &= ~DQ_BLKS;
				}
				odp->dq_flags |= DQ_MOD;
			}

			/* give resources to new owner.
			 */
			if ((ndp = tino.i_dquot[type]) != NODQUOT)
			{
				ndp->dq_curinodes += 1;
				cpart = DQCARRY(ndp->dq_btime);
				ndp->dq_curblocks += (nblocks +
							(npart & cpart));
				ndp->dq_btime = DQSBTIME((npart ^ cpart),
							ndp->dq_btime);
				ndp->dq_flags |= DQ_MOD;
			}

			/* update the inode's dquot for the quota type
			 * to reflect change in ownership.
			 */
			COMBIT_LOCK(ip); 
			ip->i_dquot[type] = tino.i_dquot[type];
			COMBIT_UNLOCK(ip); 
			tino.i_dquot[type] = odp;

			/* free dquot locks for this quota type.
			 */
			if (ndp != NODQUOT)
				DQUOT_UNLOCK(ndp);
			if (odp != NODQUOT)
				DQUOT_UNLOCK(odp);
		}
		ICACHE_LOCK();
		dqput(tino.i_dquot[type]);
		ICACHE_UNLOCK();
	}
	return(0);


	/* new owner could not handle new resources. free
	 * up any locks held on dquots and put the new
	 * owner's dquots.
	 */
bad:
	for (type = 0; type < MAXQUOTAS; type++)
	{
		if (newid[type] != oldid[type])
		{
			if (tino.i_dquot[type] != NODQUOT)
				DQUOT_UNLOCK(tino.i_dquot[type]);
			if (ip->i_dquot[type] != NODQUOT)
				DQUOT_UNLOCK(ip->i_dquot[type]);
		}
		ICACHE_LOCK();
		dqput(tino.i_dquot[type]);
		ICACHE_UNLOCK();
	}
	return(EDQUOT);
}

/*
 * NAME:	dqget (jmp, id, type, dpp)
 *
 * FUNCTION:	get the dquot for an id, quota type and file system,
 *		increment the reference count by 1, and return a pointer
 *		to the dquot.  a lookup is perform for the dquot within
 *		the dquot hash.  if found, the reference count is incremented
 *		and the dquot pointer returned; otherwise, a dquot is
 *		allocated and initialized from the quota file.  The dquot
 *		is placed in the hash will a reference count of 1 and a
 *		pointer to the dquot is returned.
 *
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		id	- uid or gid (interpetted based on type)
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 *		dpp     - returned dquot pointer
 *
 * RETURN :	0	- success
 *		EINVAL	- invalid disk quota id, quotas disabled or being
 *			  disabled
 *		EIO	- error in reading on-disk quota data
 *		EUSERS	- out of free dquots
 *			
 * SERIALIZATION: Must be holding the ICACHE_LOCK.
 */

dqget (jmp, id, type, dpp)
struct jfsmount *jmp;	
uid_t id;	
int type;
struct dquot **dpp;		
{ 
	struct dquot *dp;
	struct hdquot *dhp;
	dev_t dev;
	int rc;

	assert(jmp);

	/* check for valid disk quota id.
	 */
	if (DQVALID(id) == 0)
		return(EINVAL);

	*dpp = NODQUOT;
	dev = jmp->jm_dev;
	
loop:
	/* check if quotas are disabled or are in the process
	 * of being disabled.
	 */
	if (jmp->jm_quotas[type] == NULL || jmp->jm_qflags[type] & QTF_CLOSING)
		return(EINVAL);

	/* 
	 * search the dquot hash list for the dquot.
	 */
	dhp = DQHASH(dev,id); 	
	for (dp = dhp->dqh_forw; dp != (struct dquot *) dhp; dp = dp->dq_forw)
	{
		if (dp->dq_id != id || dp->dq_type != type || dp->dq_dev != dev)
			continue;

		/* dquot found.  increment reference count.
		 */
		dp->dq_cnt++;

		/* if the dquot is lock, we must wait for it to be unlocked.
		 * another dqget() may be in progress for this dquot and we
		 * must insure that the dquot is valid.
		 */
		if (dp->dq_flags & DQ_QFLOCK)
		{	
			do {
				dp->dq_flags |= DQ_QFWANT;
				e_sleep_thread(&dp->dq_event,&jfs_icache_lock,
                                                              	LOCK_SIMPLE);

				/* check if the dquot is valid. if not
				 * valid, put it and start over.
				 */
				if (dp->dq_flags & DQ_IOERROR)
				{
				 	dqput(dp);
					goto loop;
				}

			} while (dp->dq_flags & DQ_QFLOCK);

			*dpp = dp;
			return(0);
		}
		
		/* take it off cache list if the reference count
	 	 * was 0.
		 */
		if (dp->dq_cnt == 1)
		{
		 	remque2 (dp);
			NULL_DQHASH2 (dp);
			dqcachesize--;
		}

		*dpp = dp;
		return(0);
	}

	/* dquot not found in the hash. allocate a free dquot.
	 */
	if (rc = getdp (&dp)) 
		return(EUSERS);

	/* connect it to the hash list.
	 */
	insque (dp, dhp);

	/* fill out the dquot.
	 */
	dp->dq_dev = dev;
	dp->dq_id = id;
	dp->dq_type = type;
	dp->dq_jmp = jmp;
	dp->dq_cnt = 1;
	dp->dq_flags = DQ_QFLOCK;
	dp->dq_event = EVENT_NULL;

	/* read on-disk data from the quota file.
	 */
 	rc = dqread (dp);

	/* wakeup sleepers.
	 */
	dp->dq_flags &= ~DQ_QFLOCK;
	if (dp->dq_flags & DQ_QFWANT)
	{
	 	dp->dq_flags &= ~DQ_QFWANT;
		e_wakeupx(&dp->dq_event, E_WKX_NO_PREEMPT);
	}

	/* mark the dquot as invalid and put it if an
	 * error occurred in reading the on-disk data.
	 */
	if (rc)
	{
	 	dp->dq_flags |= DQ_IOERROR;
		dqput(dp);
		return(EIO);
	}

	*dpp = dp;
	return(0);
}

/*
 * NAME:	getdq (dpp)
 *
 * FUNCTION:	allocate a free dquot.  the dquot will be allocated from 
 *		the cache list if the cache size has reached the maximum
 *		size; otherwise it will be allocated from the free list.
 *		
 * PARAMETERS:	dpp     - returned dquot pointer
 *
 * RETURN :	0       - success
 *              EUSERS  - no free dquots
 *			
 */

getdp (dpp) 
struct dquot **dpp;
{
	struct dquot *dp;

	/* at or above the maximum cache size ?  if so, allocate
	 * from the cache.
	 */
	if (dqcache.dqc_next != (struct dquot *) &dqcache &&
		dqcachesize >= dqmaxcachesize)
	{
		dp = (struct dquot *) dqcache.dqc_next;
		assert(dp->dq_cnt == 0);
		assert(!(dp->dq_flags & DQ_MOD));

		/* remove from cache list and previous
		 * hash chain
		 */
		dqcachesize--;
		remque2(dp);
		remque (dp);
 	}		
	else
	{
		/* must use free list.
		 */
		if ((dp = (struct dquot *) genalloc (dquot_ap)) == NULL)
			return(EUSERS);
	}

	NULL_DQHASH2(dp);

	*dpp = dp;
	return(0);
}

/*
 * NAME:	dqput (dp)
 *
 * FUNCTION:	decrement the reference count of a dquot.  on last
 *		reference, sync the dquot and add it to the cache list.
 *
 * PARAMETERS:	dp 	- pointer to dquot to put
 *
 * RETURN :	0 	- dquot put without sleeping 
 *		1	- slept while putting the dquot
 *			
 */

dqput(dp)
struct dquot *dp;
{
	int rc;

	/* return if no dquot.
	 */
	if (dp == NODQUOT)
		return(0);

	assert(dp->dq_cnt > 0);

	/* decrement the reference count.  return if dquot
	 * is still in use.
	 */
	if (dp->dq_cnt > 1)
	{
		dp->dq_cnt--;
		return(0);
	}
 
	dp->dq_flags |= DQ_QFLOCK;

	/* sync the dquot.
	 */
	dqsync(dp);

	/* decrement the reference count and check if it is non-zero.
	 * if it is non-zero we slept while syncing the dquot and a
	 * dqget() occurred for the dquot.
	 */
	dp->dq_flags &= ~DQ_QFLOCK;
	if (--dp->dq_cnt > 0)
	{
	 	dp->dq_flags &= ~DQ_QFWANT;
		e_wakeupx(&dp->dq_event, E_WKX_NO_PREEMPT);
		return(1);
	}

	/* check if the dquot is valid. if so, add to the
	 * cache list.
	 */
	if (!(dp->dq_flags & DQ_IOERROR))
	{
		dqcachesize++;
		insque2 (dp, dqcache.dqc_prev);
		return(rc);
	}

	/* unhash the dquot - the dquot is invalid.
	 */
	dquhash(dp);
	return(1);
}

/*
 * NAME:	dqsync (dp)
 *
 * FUNCTION:	sync a modified dquot to the quota file.
 *		
 * PARAMETERS:	dp 	- pointer to dquot to sync
 *
 * RETURN :	0 	- dquot synced without sleeping 
 *		1	- slept while syncing the dquot
 *			
 * SERIALIZATON: Must be holding the ICACHE_LOCK upon entry.
 */

dqsync(dp)
struct dquot *dp;
{
	struct inode *ip;
	struct dquot tdp;
	int rc = 0;

	ASSERT(dp != NODQUOT);
	ASSERT(lock_mine(&jfs_icache_lock));

	/* write the modified dquot to the quota file.  check if the
	 * dquot is quota file's dquot.  if so, we must use a temporary
	 * dquot to write it to prevent deadlock with the VMM.
	 */
	if (qfino(dp->dq_jmp,dp->dq_type,&ip) && dp == ip->i_dquot[dp->dq_type])
	{
		/* quota file's dquot.  copy the dquot to a temporary
		 * dquot, mark the true dquot as unmodified, and unlock
		 * the true dquot before performing the write for the
		 * temporary.  also, set quota file lock before releasing
		 * the lock.
		 */

		while(qfile_syncing)
			e_sleep_thread(&qfile_event,&jfs_icache_lock,LOCK_SIMPLE);
		qfile_syncing = 1;

		ICACHE_UNLOCK();
		DQUOT_LOCK(dp);

		bcopy(dp,&tdp,sizeof(struct dquot));
                dp->dq_flags &= ~DQ_MOD;

		DQUOT_UNLOCK(dp);

		rc = dqwrite(&tdp);

		ICACHE_LOCK();
		qfile_syncing = 0;
		if (qfile_event != EVENT_NULL)
			e_wakeupx(&qfile_event,E_WKX_NO_PREEMPT);
	}
	else
	{
		ICACHE_UNLOCK();
		DQUOT_LOCK(dp);

		/* write the dquot.
		 */
		if ((dp->dq_flags & DQ_MOD))
		{
			rc = dqwrite(dp);
			dp->dq_flags &= ~DQ_MOD;
		}

		DQUOT_UNLOCK(dp);
		ICACHE_LOCK();
	}

	return(rc);
}

/*
 * NAME:	dquhash (dp)
 *
 * FUNCTION:	remove a dquot from the hash list and place it
 *		on the free list.
 *
 * PARAMETERS:	dp 	- pointer to dquot to unhash
 *
 * RETURN :	NONE
 *			
 */

dquhash(dp)
struct dquot *dp;
{
	remque (dp);
	dp->dq_forw = dp->dq_back = NULL;
	dp->dq_flags = 0;
	dp->dq_type = 0;
	dp->dq_id = 0;
	dp->dq_jmp = 0;
	dp->dq_dev = 0xdeadbeef;
	genfree (dquot_ap, dp);
}

/*
 * NAME:	dqread (dp)
 *
 * FUNCTION:	reads on-disk quota data from the quota file into the dquot
 *		pointed to by dp.  on entry, dq_jmp contains a pointer to the
 *		file system mount structure, dq_type specifies the quota type,
 *		and dq_id specifies the id for the dquot.  before performing
 *		the read operation, the current user credential are switched
 *		to those of the user that enabled quotas; the creds are 
 *		restored on return from the read.
 *
 * PARAMETERS:	dp 	- pointer to dquot to read
 *
 * RETURN :	0 	- success
 *		EIO	- error reading on-disk quota data
 *			
 */
 
dqread(dp)
struct dquot *dp;
{
	struct vnode *vp;
	struct iovec aiov;
	struct uio auio;
	struct ucred *crp;
	caddr_t vinfo;
	int rc;

	ICACHE_UNLOCK();

	/* get vnode pointer and ucred for the quota file.
	 */
	vp = dp->dq_jmp->jm_quotas[dp->dq_type];
	crp = dp->dq_jmp->jm_cred[dp->dq_type];
	assert(vp);
	assert(crp);

	/* fill out uio structure for the read operation.
	 */
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = (caddr_t)&dp->dq_dqb;
	aiov.iov_len = sizeof (struct dqblk);
	auio.uio_resid = sizeof (struct dqblk);
	auio.uio_offset = (off_t)(dp->dq_id * sizeof (struct dqblk));
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_fmode = FREAD|FWRITE|FKERNEL;

	/* perform the read.
	 */
	rc = VNOP_RDWR(vp, UIO_READ, FREAD|FWRITE, &auio, 0, vinfo, NULL, crp);

	/* zero the dqblk if we read beyond end of file.
	 */
 	if (auio.uio_resid == sizeof(struct dqblk) && rc == 0)
                bzero((caddr_t)&dp->dq_dqb, sizeof(struct dqblk));

	ICACHE_LOCK();
	return(rc);
}

/*
 * NAME:	dqwrite (dp)
 *
 * FUNCTION:	writes on-disk quota data into the quota file from the dquot
 *		pointed to by dp.  on entry, dq_jmp contains a pointer to the
 *		file system mount structure, dq_type specifies the quota type,
 *		and dq_id specifies the id for the dquot.  before performing
 *		the write operation, the current user credential are switched
 *		to those of the user that enabled quotas.  the creds are 
 *		restored on return from the write and the dquot is marked
 *		as unmodified.
 *		
 * PARAMETERS:	dp 	- pointer to dquot to write
 *
 * RETURN :	0 	- success
 *		EIO	- error writing on-disk quota data
 *			
 */

dqwrite(dp)
struct dquot *dp;
{
	struct vnode *vp;
	struct iovec aiov;
	struct uio auio;
	struct ucred *crp;
	caddr_t vinfo;
	int rc;

	/* get vnode pointer and ucred for the quota file.
	 */
	vp = dp->dq_jmp->jm_quotas[dp->dq_type];
	crp = dp->dq_jmp->jm_cred[dp->dq_type];
	assert(vp);
	assert(crp);

	/* fill out uio structure for the write operation.
	 */
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = (caddr_t)&dp->dq_dqb;
	aiov.iov_len = sizeof (struct dqblk);
	auio.uio_resid = sizeof (struct dqblk);
	auio.uio_offset = (off_t)(dp->dq_id * sizeof (struct dqblk));
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_fmode = FREAD|FWRITE|FKERNEL;

	/* perform the write.
	 */
	rc = VNOP_RDWR(vp, UIO_WRITE, FREAD|FWRITE, &auio, 0, vinfo, NULL, crp);

	return(rc);
}

/*
 * NAME:	qfino (jmp, type, ipp)
 *
 * FUNCTION:	determines if the quota file for the specified type of 
 *		quota is within the file system (jmp).  if the quota file
 *		is within the file system a value of 1 is returned plus
 *		the inode pointer of the quota file; otherwise a value of
 *		0 is returned.
 *		
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *		type	- type of quota (USRQUOTA or GRPQUOTA)
 * 		ipp     - returned inode pointer
 *
 * RETURN :	0       - quota file does not reside within the file system
 *              1	- quota file resides within the file system
 *			
 */

qfino(jmp,type,ipp)
struct jfsmount *jmp;
int type;
struct inode **ipp;
{
	struct vnode *vp;
	struct inode *ip;

	*ipp = NULL;

	if ((vp = jmp->jm_quotas[type]) == NULL)
		return(0);

	if (vp->v_vfsp->vfs_type == MNT_JFS)
	{
		ip = VTOIP(vp);
		if (ip->i_dev == jmp->jm_dev)
		{
			*ipp = ip;
			return(1);
		}
	}
	
	return(0);
}

/*
 * NAME:	dqumount (jmp)
 *
 * FUNCTION:	disables disk quotas for an unmounting file system and
 *		cleans up inodes for the file system's quota files if
 *		they reside within the file system. 
 *
 *		called only by jfs_umount().
 *		
 * PARAMETERS:	jmp	- mount structure pointer for the file system
 *
 * RETURN :	NONE
 *			
 * SERILAIZATION: ICACHE_LOCK() held on entry/exit
 */

dqumount(jmp)
struct jfsmount *jmp;
{
	struct inode *ip;
	int type;

	for (type = 0; type < MAXQUOTAS; type++)
	{
		if (qfino(jmp,type,&ip))
			ip->i_count++;
		
		ICACHE_UNLOCK();
		quotaoff(jmp,type);
		ICACHE_LOCK();

		if (ip != NULL)
		{
			if (ip->i_count == 1)
			{
				iuncache(ip);
				iunhash(ip);
			}
			else
			{
				ip->i_count--;
			}
		}
	}
}

/*
 * NAME:	dqactivity (ip)
 *
 * FUNCTION:	determines if an inode reference count is accounted
 *		for by it use as the quota file(s) for the file system
 *		in which the inode resides.  this routines is called
 *		by iactivity() in checking inode activity at umount.
 *		
 *		called only by iactivity().
 *
 * PARAMETERS:	ip	- inode pointer
 *
 * RETURN :	0 	- reference count not accounted for
 * 		1 	- reference count account for (quota file)
 *			
 * SERIALIZATION: ICACHE_LOCK() held on entry/exit
 */

dqactivity(ip)
struct inode *ip;
{
	struct inode *qfip;
	struct gnode *gp;
	int type, count = 0;

	if (ip->i_count > 2)
		return(0);

	gp = ITOGP(ip);

	if (gp->gn_excnt || gp->gn_mrdcnt || gp->gn_mwrcnt)
		return(0);

	for (type = 0; type < MAXQUOTAS; type++)
	{
		if (qfino(ip->i_ipmnt->i_jmpmnt,type,&qfip) == 0)
			continue;

      		if (ip == qfip)
			count++;
	}

	if (count == 0 || count != gp->gn_wrcnt || count != gp->gn_rdcnt)
		return(0);

	return(1);

}

/*
 * NAME:        dqmsg (msgno, devid)
 *
 * FUNCTION:    construct and print kernel disk quota message.
 *
 * PARAMETERS:  msgno   - NLS message number
 *              devid   - devid of file system associated with the
 *                        disk quota message
 *
 * RETURN :     NONE
 *
 */

static
dqmsg(msgno,devid)
int msgno;
dev_t devid;
{
        struct uprintf up;

        /* fill out the uprintf struct with message info.  Note
         * that defmsg will be provided by the uprintfd for
         * SET_SYSPFS messages; however, defmsg in uprintf struct must
         * contain a non-string conversion specification so that
         * devid (fsid) will be handled correctly by NLuprintfx.
         */
        up.upf_defmsg = "%x";
        up.upf_NLcatname = MF_UNIX;
        up.upf_NLsetno = SET_SYSPFS;
        up.upf_NLmsgno = msgno;
        up.upf_args[0] = (void *) devid;

        /* print the message.
         */
        NLuprintf(&up);
}
