static char sccsid[] = "@(#)05	1.22  src/bos/kernel/pfs/xix_close.c, syspfs, bos411, 9428A410j 7/7/94 16:53:11";
/*
 * COMPONENT_NAME: (SYSPFS) journaled files system close vnop
 *
 * FUNCTIONS: jfs_close
 *
 * ORIGINS: 3, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/errno.h"
#include "sys/syspest.h"
#include "sys/sleep.h"
#include "sys/file.h"

BUGVDEF(xclosedbg,0);
BUGVDEF(pfsdbg,0);

extern struct vfs *rootvfs;
static void cwakeup();

/*
 * NAME:	jfs_close (vp, flag, vinfo, crp)
 *
 * FUNCTION:	Adjust conts that are kept, per open
 *
 * PARAMETERS:	Ignored
 *
 * RETURN VALUE: Always zero
 */

jfs_close (vp, flag, vinfo, crp)
struct vnode	*vp;		/* vnode pointer		*/
int		flag;		/* flags from the file pointer	*/
caddr_t		vinfo;		/* for remote */
struct ucred	*crp;		/* pointer to credential structure */
{
	struct gnode *gp;	/* gnode pointer		*/
	struct inode *ip;
	int	wakeup = 0;	/* wakeup open_sleep()er ? */

	BUGLPR(xclosedbg+pfsdbg,					\
		BUGACT,("jfs_close:vp=%X flag=%X ip=%X gp=%X\n",	\
		vp, flag, VTOGP(vp), VTOIP(vp)));

	gp = VTOGP(vp);
	ip = GTOIP(gp);
	IWRITE_LOCK(ip);

	if (flag & FEXEC) {
		ASSERT(gp->gn_excnt > 0);
		(gp->gn_excnt)--;
	}

	if (flag & FREAD) {
		ASSERT(gp->gn_rdcnt > 0);
		if (--(gp->gn_rdcnt) == 0)
			wakeup = 1;
	}

	if (flag & FWRITE) {
		ASSERT(gp->gn_wrcnt > 0);
		if (--(gp->gn_wrcnt) == 0)
			wakeup = 1;
	}

	if (flag & (FRSHARE|FEXEC)) {
		ASSERT(gp->gn_rshcnt > 0);
		if (--(gp->gn_rshcnt) == 0)
			wakeup = 1;
	}

 	if (flag & FNSHARE) {
 		ASSERT(gp->gn_flags & GNF_NSHARE);
 		gp->gn_flags &= ~GNF_NSHARE;
		wakeup = 1;
 	}

	if (wakeup && ip->i_openevent != EVENT_NULL)
		e_wakeupx(&ip->i_openevent, E_WKX_NO_PREEMPT);

	/* if the file was opened deferred update.. backout 
	 * changes to file if there are no more accessors
	 * this has to be done here because the file may not
	 * get iput as a part of this close (cwakeup).
	 */

	if (ip->i_flag & IDEFER)
	{
		if ((gp->gn_rdcnt + gp->gn_wrcnt + gp->gn_excnt +
		     gp->gn_mrdcnt + gp->gn_mwrcnt) == 0)
		{
			ifreeseg(ip);
			ip->i_flag &= ~(IDEFER | IACC | ICHG | IFSYNC);
			ip->i_cflag = 0;
		}
	}

	IWRITE_UNLOCK(ip);
	return 0;
}
