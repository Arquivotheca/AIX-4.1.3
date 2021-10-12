static char sccsid[] = "@(#)79	1.19.1.17  src/bos/kernel/specfs/specsubs.c, sysspecfs, bos411, 9428A410j 6/10/94 16:50:15";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File System
 *
 * FUNCTIONS: dev_vp, devget, devput, devqry, devsetsth, devtosth,
 *            fdtosth, fifo_vp, ic_specaccess, smark, snput, spec_clone,
 *            spec_vp, specacc, specaccess, specchk_link, specchk_rename
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/errno.h"
#include "sys/device.h"
#include "sys/vattr.h"
#include "sys/sleep.h"
#include "sys/malloc.h"
#include "sys/specnode.h"
#include "sys/id.h"
#include "sys/gpai.h"
#include "sys/fs_locks.h"
#include "sys/lockname.h"
#include "sys/lock_alloc.h"

extern struct vnodeops spec_vnops;
extern struct vnodeops fifo_vnops;
extern int gn_reclk_count;
struct hnode  hnodetable[NHNODE];	/* specfs hash table	*/
int    spec_generation = 1;		/* generation counter for specnodes */

/* 
 * Set up pools for the interesting specfs structures.
 * Initial allocation for each structure type is approximately
 * one page of memory.
 */
extern struct galloc specnode_pool;
static struct galloc *snpool = &specnode_pool;
extern struct galloc devnode_pool;
static struct galloc *dnpool = &devnode_pool;
extern struct galloc fifonode_pool;
static struct galloc *fnpool = &fifonode_pool;


/*
 * NAME:	spec_vp(vpp)
 *
 * FUNCTION:	This function is used to exchange a PFS vnode for a
 *		spec vnode.  It is called by callers of the lookup
 *		vnode operations of physical file systems.  These callers
 *		pass in the PFS vnode returned by the lookup vnode
 *		operation after determining that the vnode represents
 *		a special file.
 *		This function examines character special devices to
 *		determine whether they are actually multiplexed devices.
 *
 * PARAMETERS:	vpp	- pointer to PFS vnode and spec vnode return
 *
 * RETURN:	0
 */
int
spec_vp(
	struct vnode **		vpp)	/* PFS & specfs (return) vnodes	*/
{
	int	rc;

	switch ((*vpp)->v_type)
	{
		case VBLK:
			rc = dev_vp(vpp, VBLK, 0);
			break;

		case VCHR:
		/* For mpx bases, the channel number is -1, all others
			 * have channel number zero */
			if (mpx_dev(VTOGP(*vpp)->gn_rdev))
				rc = dev_vp(vpp, VMPC, BASE_MPX);
			else
				rc = dev_vp(vpp, VCHR, 0);
			break;

		case VMPC:
		/* We cannot get a multiplexed device from the PFS. */
			rc = EINVAL;
			break;

		case VFIFO:
			rc = fifo_vp(vpp);
			break;

		default:
			rc = 0;
	}
	return rc;
}


/*
 * NAME:	dev_vp(vpp, vtype, chan)
 *
 * FUNCTION:	This function is used to exchange a PFS vnode for a
 *		spec vnode.  It is called by callers of the lookup
 *		vnode operations of physical file systems.  These callers
 *		pass in the PFS vnode returned by the lookup vnode
 *		operation after determining that the vnode represents
 *		a device special file.
 *		This function is also called by mpx_lookup to get a
 *		spec vnode for the specified mpx channel of the base
 *		mpx provided.
 *
 * PARAMETERS:	vpp	- pointer to PFS vnode and spec vnode return
 *		vtype	- vnode type of spec vnode to return
 *		chan	- channel number of device to find
 *
 * RETURN:	0
 */
int
dev_vp(
	struct vnode **		vpp,	/* PFS & specfs (return) vnodes	*/
	int			vtype,	/* vnode type of spec vnode	*/
	chan_t			chan)	/* channel number of device	*/
{
	struct vnode *		vp;	/* PFS vnode			*/
	struct gnode *		gp;	/* PFS gnode for special file	*/
	struct specnode *	snp;	/* specnode for device special	*/
	struct devnode *	dp;	/* devnode for device		*/
	struct gnode *		sgp;	/* spec gnode for special file	*/
	struct vnode *		tvp;	/* temp specfs vnode		*/
	int			sgen;   /* saved specnode generation    */
	int			rc;	/* return code			*/

	gp = VTOGP(*vpp);
	vp = *vpp;

	/* Get the devnode for the device. */
	if (rc = devget(gp->gn_rdev, chan, vtype, &dp))
		return rc;

	/*
	 * Upon successful completion of devget() the devnode will 
	 * be returned locked.
	 */

	/* Search for a specnode on the devnode which currently references
	 * the PFS's gnode.  Multiple channels of mpx devices reference the
	 * base's PFS gnode, so the channel numbers must match also.
	 */
	for (snp = dp->dv_specnodes; snp; snp = snp->sn_next)
	{
		if (snp->sn_pfsgnode == gp && STOSGP(snp)->gn_chan == chan)
		{
			/* Save the specnode generation to verify that we still
			 * have the same specnode after getting a lock on it.
			 */
			sgen = snp->sn_gen;

			/* Lock the specnode we found. */
			SPECNODE_LOCK(snp);

			/* Verify that this is still the specnode that we
			 * wanted and that it is not going away.
			 */
			if ((snp->sn_gen != sgen) || (snp->sn_count == 0))
			{
				/* Not what we need.  Continue the search
				 * with the next specnode on the list.
				 */
				SPECNODE_UNLOCK(snp);
				continue;
			}

			/* Bump the specnode count. */
			snp->sn_count++;

			devput(dp);

			/* Search for the appropriate vnode on the specnode's
			 * list.  The specnode must be locked during this
			 * search and the call to vn_get().
			 */
			for (tvp = snp->sn_vnode; tvp; tvp = tvp->v_next)
				if (tvp->v_pfsvnode == vp)
				{
					/* Hold the vnode and unlock and put
					 * the specnode.  Release the PFS vnode
					 * because we don't attach it.
					 */
					VNOP_HOLD(tvp);
					VNOP_RELE(vp);
					*vpp = tvp;
					snput(snp);
					return 0;
				}
			/* If we didn't find the vnode, then continue with
			 * the code which follows the specnode creation.
			 */
			 break;
		}
	}
	
	/* If no specnode found then create one. */
	if (snp == NULL)
	{
		/* Allocate memory for a specnode. */
		if ((snp = (struct specnode *)gpai_alloc(snpool)) == NULL)
		{
			devput(dp);
			return ENOMEM;
		}

		/* Set the specnode generation number. */
		snp->sn_gen = fetch_and_add(&spec_generation, 1);

		/* fill in the gnode */
		sgp = STOSGP(snp);
		sgp->gn_type =		vtype;
		sgp->gn_flags =		0;
		sgp->gn_seg =		INVLSID;
		sgp->gn_mwrcnt =	0;
		sgp->gn_mrdcnt =	0;
		sgp->gn_rdcnt =		0;
		sgp->gn_wrcnt =		0;
		sgp->gn_excnt =		0;
		sgp->gn_rshcnt =	0;
		sgp->gn_ops =		&spec_vnops;
		sgp->gn_vnode =		NULL;
		sgp->gn_rdev =		gp->gn_rdev;
		sgp->gn_chan =		chan;
		sgp->gn_data =		(caddr_t)snp;
		sgp->gn_reclk_event =	EVENT_NULL;
		sgp->gn_filocks =	NULL;
		sgp->gn_data =		(caddr_t)snp;
		lock_alloc(&sgp->gn_reclk_lock, LOCK_ALLOC_PAGED,
			   RECLK_LOCK_CLASS, gn_reclk_count++);
		simple_lock_init(&sgp->gn_reclk_lock);

		/* fill in the specnode */
		snp->sn_vnode =		NULL;		/* added later */
		snp->sn_count =		1;
		snp->sn_pfsgnode =	gp;
		snp->sn_attr =		NULL;
		
		/* The specnode must be locked at this point because
		 * it will be visible to other processes as soon as
		 * it is put on the devnode's list.
		 */
		SPECNODE_LOCK(snp);

		/* connect specnode to devnode */
		snp->sn_devnode = dp;
		snp->sn_next = dp->dv_specnodes;
		dp->dv_specnodes = snp;
		/* at last, unlock the devnode */
		DEVNODE_UNLOCK(dp);
	}

	/* 
	 * At this point in time, we have a locked specnode and 
	 * an unlocked devnode.
	 */

	/* Allocate the specfs vnode from the PFS vnode's vfs.
	 * This ensures that those in the LFS that use the vfs
	 * obtained from the vnode we return will get the right vfs.
	 */
	if (rc = vn_get((*vpp)->v_vfsp, STOSGP(snp), &tvp))
	{
		snput(snp);
		return rc;
	}

	/* Attach the PFS vnode to the specfs vnode and return the
	 * specfs vnode.
	 */
	tvp->v_pfsvnode = *vpp;
	*vpp = tvp;

	SPECNODE_UNLOCK(snp);

	return 0;
}

int
devget (
	dev_t		dev,		/* device number of device	*/
	chan_t		chan,		/* device channel number	*/
	int		vtype,		/* vnode type for gnode		*/
	struct devnode **dpp)		/* devnode for return		*/
{
	struct devnode *dp;		/* devnode to return		*/
	struct hnode *	hdp;		/* hash bucket for devnode	*/
	struct gnode *	gp;		/* dev gnode of device		*/
	int		status;		/* device switch status		*/
	int		rc;             /* return code			*/

	/* Find the hash list and lock it for the duration of the
	 * search and the addition of a new node (if necessary).
	 */
	hdp = DEVHASH(dev, chan);
	SPECHASH_LOCK(hdp);

	/* Search the hash for the matching devnode.  If the devnode
	 * is found, increment the devnode hold count and return
	 * the devnode.
	 */
	for (	dp = hdp->hdv_forw;
		dp != (struct devnode *)hdp;
		dp = dp->dv_forw)
	{
		if (dp->dv_dev == dev && dp->dv_chan == chan)
		{
			/* Lock the devnode, then make sure it is valid. */
			DEVNODE_LOCK(dp);
			if ((dp->dv_count == 0)
			   || (dp->dv_dev != dev)
			   || (dp->dv_chan != chan))
			{
				DEVNODE_UNLOCK(dp);
				continue;
			}

			/* Unlock the spechash_lock since we found it */
			SPECHASH_UNLOCK(hdp);

			/* We found the devnode in the hash table.  Increment
			 * the count and return the locked devnode.
			 */
			dp->dv_count++;
			*dpp = dp;

			return 0;
		}
	}

	/* Allocate memory for a devnode since we didn't find it. */
	if ((dp = (struct devnode *)gpai_alloc(dnpool)) == NULL)
		rc = ENOMEM;
	else
	{
		/* Initialize the fields of the devnode */
		bzero(&dp->dv_gnode, sizeof(struct gnode));
		dp->dv_lastr		= 0;
		dp->dv_pdata		= NULL;

		/* fill in the fields of the devnode */
		dp->dv_dev =		dev;
		dp->dv_flag =		0;
		dp->dv_count =		1;
		dp->dv_specnodes = 	NULL;
		/* must return locked devnode */
		DEVNODE_LOCK(dp);

		/* fill in the fields of the gnode */
		gp = DTODGP(dp);
		gp->gn_data =		(caddr_t)dp;
		gp->gn_rdev =		dev;
		gp->gn_chan =		chan;
		gp->gn_type =		vtype;
		gp->gn_seg =		INVLSID;
		gp->gn_ops =		NULL;
		gp->gn_vnode =		NULL;
		
		/* add the devnode to the spec hash */
		dp->dv_forw =		hdp->hdv_forw;
		dp->dv_back =		(struct devnode *)hdp;
		dp->dv_forw->dv_back =	dp;
		hdp->hdv_forw =		dp;

		rc = 0;
	}

	/* unlock spechash here since devnode is now on hash list */
	SPECHASH_UNLOCK(hdp);

	*dpp = dp;
	return rc;
}

int
devput (
	struct devnode *	dp)		/* devnode to put	*/
{
	int count;
        struct hnode *  hdp;            /* hash bucket for devnode      */

	/* check the devnode hold count */
	assert(dp->dv_count > 0);

	/* The caller must be holding the lock for this devnode */
	ASSERT(lock_mine(&dp->dv_lock));

	count = --dp->dv_count;

	/* NOTE:
	 * When the count on the devnode is decremented to zero, the
	 * devnode will not be reused.  (The devget logic enforces this.)
	 * Therefore, we do not need to re-acquire the devnode lock and
	 * test the count again after getting the spechash lock.
	 */
	if (count == 0)
	{
		/* Deallocate the channel while holding the DEVNODE_LOCK
		 * to allow the channel to be reused if a subsequent 
		 * lookup on the channel is in progress and waiting on
		 * the devnode lock in devget.
		 */
		if (MPX_CHANNEL(DTODGP(dp)))
		{
			DD_ENT((void), (*devsw[major(dp->dv_dev)].d_mpx)
			    (dp->dv_dev, &dp->dv_chan, NULL),
			    IPRI_NOPREMPT, major(dp->dv_dev));
		}

		/* get the hash chain the devnode resides in */
		hdp = DEVHASH(dp->dv_dev, dp->dv_chan);

		/* We can't take spechash lock while holding the devnode. */
		DEVNODE_UNLOCK(dp);

		/* Take the spechash lock for this hash chain. */
		SPECHASH_LOCK(hdp);

		/* remove the devnode from the hash */
		dp->dv_forw->dv_back = dp->dv_back;
		dp->dv_back->dv_forw = dp->dv_forw;

		/* Unlock the spechash chain */
		SPECHASH_UNLOCK(hdp);

		/* check the open counts */
		assert((dp->dv_rdcnt+dp->dv_wrcnt+dp->dv_mntcnt) == 0);

		/* free the devnode */
		gpai_free(dnpool, dp);
	}
	else
		DEVNODE_UNLOCK(dp);

	return 0;
}

/*
 * NAME:	fifo_vp(vpp)
 *
 * FUNCTION:	This function is used to exchange a PFS vnode for a
 *		spec vnode.  It is called by callers of the lookup
 *		vnode operations of physical file systems.  These callers
 *		pass in the PFS vnode returned by the lookup vnode
 *		operation after determining that the vnode represents
 *		a fifo.
 *
 * PARAMETERS:	vpp	- pointer to PFS vnode and spec vnode return
 *
 * RETURN:	0
 */
int
fifo_vp(
	struct vnode **		vpp)	/* PFS & specfs (return) vnodes	*/
{
	struct vnode *		tvp;	/* temp specfs vnode		*/
	struct vnode *		vp;	/* PFS vnode			*/
	struct gnode *		gp;	/* PFS gnode for special file	*/
	struct specnode *	snp;	/* specnode for device special	*/
	struct gnode *		sgp;	/* spec gnode for fifo		*/
	struct fifonode *	ffp;	/* fifonode for fifo file	*/
	struct hnode *		hffp;	/* hash bucket for fifonode	*/
	int			rc;	/* return code			*/
	int			sgen;   /* saved specnode generation    */

	vp = *vpp;
	gp = VTOGP(vp);
	snp = NULL;

	hffp = FIFOHASH(gp);

	/* Hold the spechash_lock before searching for the object */
	SPECHASH_LOCK(hffp);

	/* search for a fifo which references the gnode */
	for (	ffp = hffp->hff_forw;
		ffp != (struct fifonode *)hffp;
		ffp = ffp->ff_forw)
	{
		if (ffp->ff_dev == NODEVICE && ffp->ff_pfsgnode == gp)
		{
			/* lock the specnode for the fifonode we found */
			snp = ffp->ff_specnode;
			sgen = snp->sn_gen;
			SPECNODE_LOCK(snp);

			/* verify that the fifonode is still what we wanted 
			 * and that it is still valid
			 */
			if ((snp->sn_gen != sgen) || (snp->sn_count == 0))
			{
				/* if not, then continue the search */
				SPECNODE_UNLOCK(snp);
				snp = NULL;
				continue;
			}

			/* Bump the specnode count and stop the search. */
			snp->sn_count++;
			break;
		}
	}

	/* Create a fifonode/specnode pair if we didn't find them. */
	if (snp == NULL)
	{
		/* Allocate memory for the fifonode and specnode
		 * since we didn't find the fifonode.
		 */
		if (((ffp = (struct fifonode *)gpai_alloc(fnpool)) == NULL)
		   || ((snp = (struct specnode *)gpai_alloc(snpool)) == NULL))
		{
			SPECHASH_UNLOCK(hffp);
			if (ffp)
				gpai_free(fnpool, ffp);
			return ENOMEM;
		}
		snp->sn_gen = fetch_and_add(&spec_generation, 1);

		/* Initialize fields in the fifonode */
		ffp->ff_size = 		0;
		ffp->ff_wptr = 		0;
		ffp->ff_rptr = 		0;
		ffp->ff_poll = 		0;
		ffp->ff_flag = 		0;
		ffp->ff_rcnt = 		0;
		ffp->ff_wcnt = 		0;

		/* Initialize the fields in the specnode */
		snp->sn_next =		NULL;
		bzero(&snp->sn_gnode, sizeof(struct gnode));

		/* fill in the fifonode */
		ffp->ff_dev =		NODEVICE;
		ffp->ff_pfsgnode =	gp;
		ffp->ff_wevent =	EVENT_NULL;
		ffp->ff_revent =	EVENT_NULL;

		/* put fifonode in spec hash */
		ffp->ff_forw =		hffp->hff_forw;
		ffp->ff_back =		(struct fifonode *)hffp;
		ffp->ff_forw->ff_back =	ffp;
		hffp->hff_forw =	ffp;

		sgp = STOSGP(snp);

		/* fill in the gnode */
		sgp->gn_type =		gp->gn_type;
		sgp->gn_ops =		&fifo_vnops;
		sgp->gn_rdev =		NODEVICE;
		sgp->gn_data =		(caddr_t)snp;
		sgp->gn_seg =		INVLSID;
		lock_alloc(&sgp->gn_reclk_lock, LOCK_ALLOC_PAGED,
			   RECLK_LOCK_CLASS, gn_reclk_count++);
		simple_lock_init(&sgp->gn_reclk_lock);

		/* fill in the specnode */
		snp->sn_vnode =		NULL;		/* added later */
		snp->sn_count =		1;
		snp->sn_pfsgnode =	gp;
		snp->sn_attr =		NULL;
		SPECNODE_LOCK(snp);

		/* connect specnode to fifonode */
		snp->sn_fifonode =	ffp;
		ffp->ff_specnode =	snp;
	}

	/* Everything is connected.  Unlock spechash lock */
	SPECHASH_UNLOCK(hffp);

	/* At this point, we have a locked specnode. */

	/* Search for the appropriate vnode on the specnode's list.
	 * The specnode must be locked during this search and the
	 * call to vn_get().
	 */
	for (tvp = snp->sn_vnode; tvp; tvp = tvp->v_next)
		if (tvp->v_pfsvnode == vp)
		{
			/* Hold the vnode and unlock and put the specnode.
			 * Release the PFS vnode because we don't attach it.
			 */
			VNOP_HOLD(tvp);
			VNOP_RELE(vp);
			*vpp = tvp;
			snput(snp);
			return 0;
		}

	/* Allocate the specfs vnode from the PFS vnode's vfs.
	 * This ensures that those in the LFS that use the vfs
	 * obtained from the vnode we return will get the right vfs.
	 */
	if (rc = vn_get((*vpp)->v_vfsp, STOSGP(snp), &tvp))
	{
		snput(snp);
		return rc;
	}

	/* Attach the PFS vnode to the specfs vnode and return the
	 * specfs vnode.
	 */
	tvp->v_pfsvnode = *vpp;

	SPECNODE_UNLOCK(snp);

	*vpp = tvp;
	return 0;
}

int
specchk_rename (
	struct vnode **	svpp,	/* rename source vnode		*/
	struct vnode **	dvpp)	/* rename destination vnode	*/
{
	struct vnode *	svp;	/* rename source vnode		*/
	struct vnode *	dvp;	/* rename destination vnode	*/
	int		rc = 0;	/* return code			*/

	svp = *svpp;
	dvp = *dvpp;

	/* Check the file to be renamed and translate spec vnodes
	 * to PFS vnodes.
	 */
	switch (svp->v_type)
	{
		case VMPC:
			/* It is invalid to try to rename
			 * an mpx device channel.
			 */
			if (VTOGP(svp)->gn_chan != BASE_MPX)
			{
				rc = EINVAL;
				break;
			}

			/* fall through */

		case VBLK:
		case VCHR:
		case VFIFO:
			if (!svp->v_pfsvnode)
				return EINVAL;

			/* only pass the PFS vnode to the PFS */
			*svpp = svp->v_pfsvnode;
			break;
	
		default:
			/* renaming a non-special file */
			break;
	}

	/* Check the file to be renamed over, if any, and translate
	 * spec vnodes to PFS vnodes.
	 */
	if (!rc && dvp)
		switch (dvp->v_type)
		{
			case VMPC:
				/* It is invalid to try to rename
				 * over an mpx device channel.
				 */
				if (VTOGP(dvp)->gn_chan != BASE_MPX)
				{
					rc = EINVAL;
					break;
				}

				/* fall through */

			case VBLK:
			case VCHR:
			case VFIFO:
				/* only pass the PFS vnode to the PFS */
				*dvpp = dvp->v_pfsvnode;
				break;

			default:
				/* renaming over a non-special file */
				break;
		}

	return rc;
}

int
specchk_link(
	struct vnode **	vpp)
{
	struct vnode *	tvp;
	int		vtype;

	tvp = *vpp;
	vtype = tvp->v_type;

	if (		vtype != VCHR &&
			vtype != VBLK &&
			vtype != VMPC &&
			vtype != VFIFO)
		return 0;

	/* can't link to mpx channels */
	if (vtype == VMPC && MPX_CHANNEL(VTOGP(tvp)))
		return EINVAL;

	if (!tvp->v_pfsvnode)
		return EINVAL;

	*vpp = tvp->v_pfsvnode;

	/* Hold the PFS vnode because we are returning it,
	 * and release the spec vnode as the LFS no longer has it.
	 */
	VNOP_HOLD(tvp->v_pfsvnode);
	VNOP_RELE(tvp);

	return 0;
}

/*
 * NAME:	devqry(dev, chan)
 *
 * FUNCTION:	This function determines whether a given device is currently
 *              open.  A boolean value is returned indicating if it is.
 *		This will indicate whether any minor number or channel of
 *		the device is open.  If a channel other than BASE_MPX is
 *		specified, then the minor number and channel number are
 *		also checked.
 *
 * PARAMETERS:	dev	- device number to search for (both major and minor)
 *              chan    - specific channel to search for.  
 *                        if set to BASE_MPX, then ignore it.
 *
 * RETURN :	1	- if the device is open
 *		0	- if the argument is invalid or the device is not open.
 */

/* arguments for search routine called by gpai_srch */
struct srchargs {
	dev_t	dev;		/* device number to search for	*/
	chan_t	chan;		/* channel number to search for	*/
};

/* search routine called by gpai_srch */
static int devsrch(struct devnode *, struct srchargs *);

int
devqry (
	dev_t		dev,  		/* device number to search for	*/
	chan_t		chan)		/* channel number to search for	*/
{
	struct devnode *dp;		/* devnode being examined	*/
	struct hnode   *hdp;		/* hash chain being examined	*/
	struct srchargs sa;		/* arguments for search routine */
	int		rc = 0;		/* return code			*/

	if (major(dev) < DEVCNT) /* validate the device number argument */
	{
		/*
		 * Search all devnodes for one that
		 * matches the device number and channel.
		 */
		sa.dev = dev;
		sa.chan = chan;
		rc = gpai_srch(dnpool, 0, 0, devsrch, &sa);
	}
	return(rc);
}

static int
devsrch(struct devnode *dp, struct srchargs *dsa)
{
	int rc = 0;

	if (major(dp->dv_dev) == major(dsa->dev))
	{
		if ((dsa->chan != BASE_MPX)
		    && (dp->dv_dev != dsa->dev || dp->dv_chan != dsa->chan))
			rc = 0;

		/* make sure the device is open */
		else if ((dp->dv_rdcnt + dp->dv_wrcnt + dp->dv_mntcnt) > 0)
			rc = 1;
	}
	return rc;
}


int
snput(struct specnode *snp)
{
	struct gnode *	gp;		/* spec gnode of specnode	*/
	struct specnode *tsnp;		/* temp specnode ptr in list	*/
	struct specnode *bsnp;		/* back specnode ptr in list	*/
	struct devnode *dp;		/* devnode for specnode		*/
	struct fifonode *ffp;		/* fifonode for specnode	*/
	struct hnode   *fhp;		/* hash chain for fifo node	*/
	int		rc = 0;		/* return code			*/
	int 		count;		/* local specnode count		*/

	assert(snp->sn_count > 0);

	/* The caller should be holding the specnode lock for this specnode */
	ASSERT(lock_mine(&snp->sn_lock));

	/* Decrement the specnode hold count and check for last access. */
	if (--snp->sn_count != 0)
		/* specnode still in use; unlock it and return */
		SPECNODE_UNLOCK(snp);
	else
	{
		/* clear the generation number so that this specnode
		 * can't be reclaimed.
		 */
		snp->sn_gen = 0;

		switch (snp->sn_type)
		{
		case VMPC:
		case VBLK:
		case VCHR:
			/* Remove the specnode from the devnode list and
			 * release the devnode.
			 */
			SPECNODE_UNLOCK(snp);

			/* devnode must still be valid because of the
			 * specnode reference to it */
			dp = STODP(snp);
			DEVNODE_LOCK(dp);

			bsnp = NULL;
			for (		tsnp = dp->dv_specnodes;
					tsnp != snp;
					tsnp = tsnp->sn_next)
				bsnp = tsnp;
			if (bsnp)
				bsnp->sn_next = snp->sn_next;
			else
				dp->dv_specnodes = snp->sn_next;
			/* devput unlocks devnode_lock */
			devput(dp);
			break;

		case VFIFO:
			/* Get the fifonode.  The specnode must then be
			 * unlocked before we can get the spechash lock.
			 * The specnode will not be reused because its
			 * count is zero.  This is enforced in fifo_vp.
			 */
			ffp = STOFP(snp);
			SPECNODE_UNLOCK(snp);

			/* There is only one specnode per fifonode.
			 * If the fifonode is on a hash chain (i.e.,
			 * it is a fifo and not a pipe) remove it.
			 */
			if (ffp->ff_forw != NULL)
			{
				/* Get the hash chain and lock it. */
				fhp = FIFOHASH(ffp->ff_pfsgnode);
				SPECHASH_LOCK(fhp);

				ffp->ff_forw->ff_back = ffp->ff_back;
				ffp->ff_back->ff_forw = ffp->ff_forw;

				SPECHASH_UNLOCK(fhp);
			}

			/* free the fifonode */
			gpai_free(fnpool, ffp);

			break;

		default:
			rc = EINVAL;
		}

		/* free any in-core attributes */
		if (snp->sn_attr)
		{
			/* free any in-core ACLs */
			if (snp->sn_acl)
				free((long)snp->sn_acl & ~ACL_INCORE);
			free(snp->sn_attr);
		}

		/* free the gn_reclk_lock */
		gp = STOSGP(snp);
		lock_free(&gp->gn_reclk_lock);

		/* free the specnode */
		gpai_free(snpool, snp);
	}
	return rc;
}

int
smark(	struct specnode *snp,
	int		atype,
	struct ucred    *crp)
{
	struct timestruc_t	ts;

	/* The caller must be holding the specnode lock */
	ASSERT(lock_mine(&snp->sn_lock));

	curtime(&ts);
	/* if no in-core attributes, change PFS times */
	if (snp->sn_attr == NULL)
		return VNOP_SETATTR(snp->sn_vnode->v_pfsvnode,
				V_STIME,
				(atype & IACC)? &ts : 0,
				(atype & IUPD)? &ts : 0,
				(atype & ICHG)? &ts : 0,
				crp);

	/* update in-core times */
	if (atype & IACC)
		snp->sn_atime = ts;
	if (atype & IUPD)
		snp->sn_mtime = ts;
	if (atype & ICHG)
		snp->sn_ctime = ts;

	return 0;
}

/*
 * NAME:	ic_specaccess(snp, mode, who, crp)
 *
 * FUNCTION:	Check rwx permissions:  If file has an ACL and I_SACL
 *		is on then use the ACL, otherwise use the mode bits.
 *
 * PARAMETERS:	snp	- specnode to check permissions on
 *		mode	- mode to check for ie rwx
 *		who	- one of:	ACC_SELF 
 *				 	ACC_OTHERS 
 *				 	ACC_ANY 
 *				 	ACC_ALL
 *
 * RETURNS:	0      - success
 *		EINVAL - invalid mode or who argument
 */

int
ic_specaccess (
	struct specnode *snp,		/* specnode for in-core check	*/
	int		mode,		/* access mode(s) to check	*/
	int		who,		/* who to check access for	*/
	struct ucred    *crp)		/* ptr to relevant credentials	*/
{
	int		rc;		/* return code			*/

	/* 
	 * We must be holding the specnode lock of this specnode
	 */

	ASSERT(lock_mine(&snp->sn_lock));

	/* check the validity of "mode" */
	if (mode & ~0x7)
		return EINVAL;

	if (who == ACC_SELF)
			rc = specacc(snp, mode, crp);
	else if (who == ACC_OTHERS || who == ACC_ANY || who == ACC_ALL)
	{
		switch (mode)
		{
			case R_ACC:
			case W_ACC:
			case X_ACC:
				break;
			default:
				return EINVAL;
		}

		if (who == ACC_ALL)
		{
			if (	/* the access is denied the owner */
				(!((snp->sn_mode >> 6) & mode)) ||
				/* the access is denied the group */
				(!((snp->sn_mode >> 3) & mode)) ||
				/* the access is not granted by default */
				(!(snp->sn_mode & mode)))
				return EACCES;
			return 0;
		}

		/* instant success if mode bits for
		 * "others" allows access
		 */
		if (snp->sn_mode & mode)
			return 0;

		if (who == ACC_ANY)
		{
			if (((snp->sn_mode >> 6) & mode) == mode)
				return 0;
		}

		if (((snp->sn_mode >> 3) & mode) == mode)
			return 0;
		return EACCES;
	}
	else
		return EINVAL;

	return rc;
}

int
specaccess(
	struct vnode *	vp,		/* spec vnode for access check	*/
	int		flags,		/* open style flags		*/
	struct ucred *	crp)		/* credentials			*/
{
	struct specnode *snp;		/* specnode for this vnode	*/
	int		amode = 0;	/* access mode needed		*/


	/* 
	 * We must have the specnode locked upon call to this routine
	 */
	ASSERT(lock_mine(&VTOSP(vp)->sn_lock));

	if (flags & FREAD)
		amode |= R_ACC;

	/* only FWRITE and FTRUNC (not FAPPEND) imply writing */
	if (flags & (FWRITE|FTRUNC))
		amode |= W_ACC;

	/* Let the PFS handle the access checking for everything but
	 * mpx channels and pipes.
	 */
	if (vp->v_pfsvnode && !MPX_CHANNEL(VTOGP(vp)))
		return VNOP_ACCESS(vp->v_pfsvnode, amode, ACC_SELF, crp);

	return specacc(VTOSP(vp), amode, crp);
}

int
specacc(
	struct specnode *snp,		/* specnode for access check	*/
	int		amode,		/* access mode to check		*/
	struct ucred	*crp)		/* ptr to credentials		*/
{
	int		deny;		/* prohibited access types	*/
	int		perm;		/* permitted access modes	*/
	int		restr;		/* restricted access modes	*/

	if (GETEUID() == snp->sn_uid)
	{
		perm = (snp->sn_mode >> 6) & 07;
		/* This is also a restriction! */
		restr = ~perm & 07;
	}
	else if (groupmember_cr(snp->sn_gid, crp))
	{
		perm = (snp->sn_mode >> 3) & 07;
		restr = 0;
	}
	else	/* other */
	{
		perm = snp->sn_mode & 07;
		restr = ~perm & 07;
	}

	/* check for access modes which are permitted and
	 * not restricted.
	 */
	deny = (amode & ~(perm & ~restr));

	if (deny == 0)
		return 0;

	if (privcheck_cr(BYPASS_DAC, crp) == 0)
		return 0;

	if (deny & R_ACC)
		if (privcheck_cr(BYPASS_DAC_READ, crp))
			return EACCES;

	if (deny & W_ACC)
		if (privcheck_cr(BYPASS_DAC_WRITE, crp))
			return EACCES;

	if (deny & X_ACC)
		if (privcheck_cr(BYPASS_DAC_EXEC, crp))
			return EACCES;

	/* priveleges allow access */
	return 0;
}

#define ESTUPID EBUSY

/*
 * NAME:	spec_clone(svpp, flags, ext, vinfop, crp)
 *
 * FUNCTION:	clone a special file
 *
 * PARAMETERS:	svpp	- old specnode on way in
 *			  shiny new cloned specnode on way out
 *		flags	- open flags
 *		ext	- whatever was sent via openx (extra? external?)
 *		vinfop	- not used (XXX so why is it here?)
 *		crp	- credential
 *
 * RETURNS:	0	- success
 *		!0	- errors from lower level routines
 *
 * We have previously called the dd open entry point and it has
 * returned ECLONEME, an indication that it wishes to do some
 * finagling with the devno before we start using it.  The vnode, etc.
 * we eventually set up will be inaccessible via the file system.
 * It may, however, share a devnode with a special file opened the
 * normal way.
 *
 * for example:
 *
 * suppose dev(28,-) is the clone dd (the minor number doesn't matter)
 * someone opens dev(28,30)
 * this is really a request to clone dev(30,x) (the minor number is
 *   assigned by the dd)
 * the original VNOP_OPEN() (i.e., spec_open()) in openpnp() fails ECLONEME
 * openpnp() recognizes the failure and calls spec_clone
 * we allocate a new specnode and a new specfs vnode
 * we then re-call the dev(28,30) dd open entry point, requesting cloning
 * the dd (one of them) assigns a new minor number for device major 30
 *   and sends the information back to us (it also actually does the open)
 * we get a devnode for this new (to us) device and finish filling in and
 *   connecting the nodes
 * we release the old (dev(28,30)) nodes and return the new one to
 *   openpnp()
 */

int
spec_clone(
	   struct vnode **svpp, /* uncloned spec vnode in, cloned out */
	   int		  flags,
	   int		  ext,
	   caddr_t	  vinfop,
	   struct ucred   *crp)
{
	struct vfs	*oldvfsp;	/* PFS vnode's vfs		*/
	struct vnode    *osvp;		/* old spec vnode		*/
	struct gnode    *osgp;		/* old spec gnode for special 	*/
	struct specnode *snp = NULL;	/* specnode for device special	*/
	struct devnode  *dp = NULL;	/* devnode for device		*/
	struct gnode    *dgp;		/* devnode's gnode		*/
	struct gnode    *sgp;		/* spec gnode for special file	*/
	struct vnode    *nsvp = NULL;	/* new specfs vnode		*/
	int		 rc;		/* return codes			*/
	dev_t		 olddev;	/* the device the user opened	*/
	dev_t		 newdev;	/* the new device the dd wants	*/
	caddr_t		 pdata;		/* the driver returns something
					   here we'd rather be ignorant
					   of */
	int		noctl;		/* no controlling tty?		*/
	struct timestruc_t t;

	/* remember the old spec vnode and gnode */
	osvp = *svpp;
	osgp = VTOGP(osvp);

	olddev = osvp->v_pfsvnode->v_rdev;
	oldvfsp = osvp->v_pfsvnode->v_vfsp;

	/*
	 * now re-call the device open, asking to be cloned
	 * Much of the following code is reproduced from
	 * cdev_open(cdev_subr.c) and devcopen(devsubs.c).
	 */

	/* We pass only the allowed open flags and whether the caller
	 * was in kernel mode.
	 */
	flags &= (FMASK | FKERNEL);

	/* Hang onto the state of controlling terminal to see if
	 * it is established by the device open.
	 * XXX Is this possible for a clone open?
	 */
	noctl = (u.u_ttyp == NULL);

	/* I can't tell if it's busy yet. */
	if (flags & DMOUNT)
		return ESTUPID;		/* XXX */

	/*
	 * We overload the ext parameter.  On the way in it's
	 * the address of a variable containing the original
	 * user-supplied ext.  The driver must use extra
	 * indirection to get to the real ext.  On the way out
	 * the device driver uses it to tell us the new device
	 * it has picked.  This subtrefuge is only used at the
	 * second open call for drivers that have returned
	 * ECLONEME, i.e., only for clone opens.
	 */
	*(int *)(&newdev) = ext;
	DD_ENT(rc = ,
	       (*devsw[major(olddev)].d_open)(olddev,
					      (flags & DMASK) | DDOCLONE,
					      &pdata,
					      (int *)&newdev),
	       IPRI_NOPREMPT, major(olddev));

	if (rc)
		return(rc);

	/*
	 * allocate and initialize a new specnode
	 */

	if ((snp = (struct specnode *)gpai_alloc(snpool)) == NULL)
	{
		rc = ENOMEM;
		goto out;
	}
	snp->sn_gen = fetch_and_add(&spec_generation, 1);

	/* remember where the gnode is */
	sgp = STOSGP(snp);

	/* fill in the gnode */
	sgp->gn_type =		VCHR;
	sgp->gn_flags =		0;
	sgp->gn_seg =		INVLSID;
	sgp->gn_mwrcnt =	0;
	sgp->gn_mrdcnt =	0;
	sgp->gn_rdcnt =		0;
	sgp->gn_wrcnt =		0;
	sgp->gn_excnt =		0;
	sgp->gn_rshcnt =	0;
	sgp->gn_ops =		&spec_vnops;
	sgp->gn_vnode =		NULL;
	sgp->gn_rdev =		olddev;		/* this will change, natch */
	sgp->gn_chan =		0;
	sgp->gn_reclk_event =	EVENT_NULL;
	sgp->gn_filocks =	NULL;
	sgp->gn_data =		(caddr_t)snp;
	lock_alloc(&sgp->gn_reclk_lock, LOCK_ALLOC_PAGED,
		   RECLK_LOCK_CLASS, gn_reclk_count++);
	simple_lock_init(&sgp->gn_reclk_lock);

	/* fill in the specnode */
	snp->sn_vnode =		NULL;		/* added later, in vn_get */
	snp->sn_count =		1;
	snp->sn_pfsgnode =	NULL;		/* it's anonymous */
	snp->sn_attr =		NULL;

	/* there's no correct devnode to connect to yet */
	snp->sn_devnode = NULL;

	if ((snp->sn_attr = malloc(sizeof *snp->sn_attr)) == NULL)
	{
		rc = ENOMEM;
		goto out;
	}

	curtime(&t);

	bzero(snp->sn_attr, sizeof *snp->sn_attr);	/* XXX */
	snp->sn_mode =		IFCHR;
	snp->sn_uid =		crp->cr_uid;
	snp->sn_gid =		crp->cr_gid;
	snp->sn_atime =		t;
	snp->sn_mtime =		t;
	snp->sn_ctime =		t;
	snp->sn_acl =		NULL;

	/*
	 * now allocate and initialize a new spec vnode
	 */
	
	/* Allocate the specfs vnode from the PFS vnode's vfs.
	 * This ensures that those in the LFS that use the vfs
	 * obtained from the vnode we return will get the right vfs.
	 */
	if (rc = vn_get(oldvfsp, sgp, &nsvp))
		goto out;

	nsvp->v_pfsvnode = NULL;


	/* stuff the new device into the specnode's gnode */
	sgp->gn_rdev = newdev;

	/* at last we can get a devnode */
	if (rc = devget(sgp->gn_rdev, NULL, VCHR, &dp))
		goto out;

	/* and tuck it's private data away safely */
	dp->dv_pdata = pdata;

	dgp = DTODGP(dp);

	/* adjust open counts */
	if (flags & DREAD)
		dgp->gn_rdcnt += 1;
	if (flags & DWRITE)
		dgp->gn_wrcnt += 1;
	if (flags & DMOUNT)
		dgp->gn_mntcnt += 1;

	/* connect specnode to devnode */
	snp->sn_devnode = dp;
	snp->sn_next = dp->dv_specnodes;
	dp->dv_specnodes = snp;

	VNOP_RELE(osvp);

	/* pass back the new specfs vnode */
	*svpp = nsvp;

	/* Save the controlling terminal device number if the device
	 * open established the controlling terminal.
	 */
	if (noctl && u.u_ttyp)
		u.u_ttyd = newdev;

	/* Unlock the devnode */
	DEVNODE_UNLOCK(dp);
	/* return success */
	return 0;

out:
	DD_ENT( (void), (*devsw[major(newdev)].d_close) 
	       (newdev, 0, 0),IPRI_NOPREMPT, major(newdev));

	if (snp->sn_attr)
		free(snp->sn_attr);
	if (snp)
		gpai_free(snpool, snp);
	if (nsvp)
		vn_free(nsvp);
	if (dp)
		devput(dp);

	return rc;
}

/* devtosth -- convert a device (maj,min pair) to that private data
 *  		we're ignorant of
 * XXX this isn't the only place I make assumptions about channels
 *     and vtypes.  can I ignore mpx devices?
 */
caddr_t
devtosth(dev_t dev)
{
	struct devnode *dp;
	int lockt;
	caddr_t rv;

	/* protect against finding fifos */
	if (dev == NODEVICE)
		return NULL;

	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get the devnode for the device. */
	if (devget(dev, NULL, VCHR, &dp))
		rv = NULL;
	else {
		rv = dp->dv_pdata;
		devput(dp);
	}
	if (lockt != LOCK_NEST)
		FS_KUNLOCK(&kernel_lock);

	return rv;
}

extern int fp_getdevno(struct file *fp, dev_t *devp, chan_t *chanp);

/* fdtosth -- convert a fd (presumably for an open device)
 *  		to that private data we're ignorant of
 * XXX this isn't the only place I make assumptions about channels
 *     and vtypes
 */
caddr_t
fdtosth(int fd)
{
	struct file *fp;
	struct vnode *vp;
	struct devnode *dp;
	dev_t dev;
	caddr_t rv = NULL;
	int lockt;

	if ((lockt = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	/* get the file pointer ... */
	if (getft(fd, &fp, DTYPE_VNODE))
		goto out2;

	/* XXX I'm calling this partly because I'm unsure about
	       the significance of the f_type field */
	if (fp_getdevno(fp, &dev, (chan_t *)0))	/* dgb added third arg */
		goto out;

	/* XXX as noted above, I may be overachieving here */
	if (fp->f_vnode->v_vntype != VCHR)	/* dgb uses fp to find vnode */
		goto out;

	/* and then the devnode */
	if (devget(dev, NULL, VCHR, &dp))
		goto out;
	else {
		rv = dp->dv_pdata;
		devput(dp);
	}

      out:

	/* Decrement the use count on the file descriptor */
	ufdrele(fd);

      out2:

	if (lockt)
		lockl(&kernel_lock, LOCK_SHORT);
	return rv;
}

/* devsetsth -- stuff something we're ignorant of into the private
 *		data for a devnode corresponding to a device (maj,min pair)
 *  		
 * XXX this isn't the only place I make assumptions about channels
 *     and vtypes
 */
int
devsetsth(dev_t dev, caddr_t pdata)
{
	int rc = 0;
	struct devnode *dp;
	int lockt;
	
	if ((lockt = IS_LOCKED(&kernel_lock)) != 0 )
		unlockl(&kernel_lock);

	/* Get the devnode for the device. */
	if (!(rc = devget(dev, NULL, VCHR, &dp))) {
		dp->dv_pdata = pdata;
		devput(dp);
	}
	if (lockt)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}
