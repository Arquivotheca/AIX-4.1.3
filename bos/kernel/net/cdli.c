static char sccsid[] = "@(#)06	1.11  src/bos/kernel/net/cdli.c, sysnet, bos41J, 9517B_all 4/28/95 12:03:10";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: ns_add_demux
 *		ns_add_filter
 *		ns_add_status
 *		ns_alloc
 *		ns_attach
 *		ns_del_demux
 *		ns_del_filter
 *		ns_del_status
 *		ns_detach
 *		ns_free
 *		ns_get_demuxer
 *		ns_init
 *		ns_locate
 *		
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <net/net_globals.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <net/spl.h>
#include <sys/socket.h>
#include <sys/kinfo.h>
#include <net/if.h>


struct ndd		*ndd = NULL;
lock_data_t		ndd_lock; 
struct ns_demuxer	*demuxers = NULL;
lock_data_t		ns_demuxer_lock;

struct ndd * ns_locate();

/***************************************************************************
*
*	Notes:	
*		Lock hierarchy	
*			ndd_lock
*			ns_demuxer_lock 
*			per ndd lock
*	
***************************************************************************/

/***************************************************************************
*
*	ns_init() -  CDLI initialization
*	
***************************************************************************/
ns_init()
{
	nddsock_init();		/* Init AF_NDD */
	lock_alloc(&ndd_lock, LOCK_ALLOC_PIN, NDDCHAIN_LOCK_FAMILY, -1);
	lock_init(&ndd_lock, TRUE);
	lock_alloc(&ns_demuxer_lock, LOCK_ALLOC_PIN, DMXCHAIN_LOCK_FAMILY, -1);
	lock_init(&ns_demuxer_lock, TRUE);
	return(0);
}

/***************************************************************************
*
*	ns_get_demuxer() -  Search the demuxer chain for a demuxer of the
*			    given type. Returns a pointer to that demuxer.
*	
***************************************************************************/
struct ns_demuxer *
ns_get_demuxer(type)
	u_short		type;
{
	struct ns_demuxer *nd;

	assert(lock_islocked(&ns_demuxer_lock));

	for (nd = demuxers; nd; nd = nd->nd_next) {
		if (nd->nd_type == type)
			break;
	}
	return(nd);
}

/***************************************************************************
*
*	ns_add_demux() - Add a demuxer to the list of available demuxers.
*	
***************************************************************************/
ns_add_demux(ndd_type, demuxer)
u_long			ndd_type;
struct ns_demuxer	*demuxer;
{
	struct ns_demuxer 	**p = &demuxers;

	lock_write(&ns_demuxer_lock);
	if (ns_get_demuxer(ndd_type)) {
		lock_done(&ns_demuxer_lock);
		return(EEXIST);
	}

	demuxer->nd_next = NULL;
	while (*p)
		p = &((*p)->nd_next);
	*p = demuxer;
	demuxer->nd_type = ndd_type;
	lock_done(&ns_demuxer_lock);
	return(0);	
}

/***************************************************************************
*
*	ns_del_demux() - Remove a demuxer from the list of available demuxers.
*	
***************************************************************************/
ns_del_demux(ndd_type)
u_int	ndd_type;
{
	struct ns_demuxer 	**p = &demuxers;
	int			error = 0;

	lock_write(&ns_demuxer_lock);
	while (*p && (*p)->nd_type != ndd_type) 
		p = &((*p)->nd_next);

	if (*p == (struct ns_demuxer *) NULL) 
		error = ENOENT;
	else {
		if ((*p)->nd_inuse)
			error = EBUSY;
		else
			*p = (*p)->nd_next;
	}

	lock_done(&ns_demuxer_lock);
	return(error);	
}

/***************************************************************************
*
*	ns_add_filter() -  Pass "add filter" request on to demuxer
*	
***************************************************************************/
ns_add_filter(nddp, filter, len, ns_user)
	struct ndd		*nddp;	/* specific interface	  */
	caddr_t			filter;
	int			len;
	struct ns_user		*ns_user;	/* the details		  */
{
	return((*(nddp->ndd_demuxer->nd_add_filter))
		(nddp, filter, len, ns_user));
}

/***************************************************************************
*
*	ns_del_filter() -  Pass "delete filter" request to demuxer
*	
***************************************************************************/
ns_del_filter(nddp, filter, len)
	struct ndd	*nddp;	/* specific interface */
	caddr_t		filter;
	int		len;
{
	return((*(nddp->ndd_demuxer->nd_del_filter))(nddp, filter, len));
}

/***************************************************************************
*
*	ns_add_status() -  Pass add status filter request to demuxer. 
*	
***************************************************************************/
ns_add_status(nddp, filter, len, ns_statuser)
	struct ndd		*nddp;	/* specific interface	  */
	caddr_t			filter;
	int			len;
	struct ns_statuser	*ns_statuser;	/* the details		  */
{
	return((*(nddp->ndd_demuxer->nd_add_status))
		(nddp, filter, len, ns_statuser));
}

/***************************************************************************
*
*	ns_del_status() -  Pass add status filter request to demuxer.
*	
***************************************************************************/
ns_del_status(nddp, filter, len)
	struct ndd	*nddp;	/* specific interface */
	caddr_t		filter;
	int		len;
{
	return((*(nddp->ndd_demuxer->nd_del_status))(nddp, filter, len));
}

/***************************************************************************
*
*	ns_attach() -  Attach a NDD to the list of available drivers.
*	
***************************************************************************/
ns_attach(nddp)
	struct ndd *nddp;
{
	register struct ndd **p = &ndd;

	lock_write(&ndd_lock);
	if (ns_locate(nddp->ndd_name) || 
	    (nddp->ndd_alias && ns_locate(nddp->ndd_alias)) ) {
		lock_done(&ndd_lock);
		return(EEXIST);
	}

	nddp->ndd_next = NULL;
	while (*p)
		p = &((*p)->ndd_next);
	*p = nddp;
	nddp->ndd_refcnt = 0;
	lock_alloc(&nddp->ndd_lock, LOCK_ALLOC_PIN, NDD_LOCK_FAMILY, nddp);
	lock_init(&nddp->ndd_lock, TRUE);
	DEMUXER_LOCKINIT(&nddp->ndd_demux_lock);
	lock_done(&ndd_lock);
	return(0);
}

/***************************************************************************
*
*	ns_detach() -  Detach a NDD from the list of available drivers.
*	
***************************************************************************/
ns_detach(nddp)
register struct ndd 	*nddp;
{
	register struct ndd 	**p = &ndd;
	int 			error = 0;

	lock_write(&ndd_lock);
	NDD_LOCK(nddp);

	if (nddp->ndd_refcnt != 0) {
		NDD_UNLOCK(nddp);
		lock_done(&ndd_lock);
		return(EBUSY);
	}	

	while (*p && *p != nddp) 
		p = &((*p)->ndd_next);

	if (*p == (struct ndd *) NULL) 
		error = ENOENT;
	else 
		*p = (*p)->ndd_next;

	NDD_UNLOCK(nddp);
	lock_free(&nddp->ndd_lock);
	lock_free(&nddp->ndd_demux_lock);
	lock_done(&ndd_lock);
	return(error);
}

/***************************************************************************
*
* 	ns_locate() - Locates NDD based on NDD name. Returns NDD
*		      pointer. Must be called with ndd_lock.
*
***************************************************************************/
struct ndd *
ns_locate(name)
	register char *name;
{
	register struct ndd *nddp;

	assert(lock_islocked(&ndd_lock));

	for (nddp = ndd; nddp; nddp = nddp->ndd_next) {
		if (!strncmp(nddp->ndd_name, name, NDD_MAXNAMELEN))
			break;
		if (nddp->ndd_alias && 
		    !strncmp(nddp->ndd_alias, name, NDD_MAXNAMELEN))
			break;
	}
	return (nddp);
}

/*
 * Wait up to 60 seconds for the NDD_RUNNING flag to come on, checking
 * it every 1/2 second.
 */
void
ns_wait(ndd_t *nddp)
{
	int	count = 0;

	while (!(nddp->ndd_flags & NDD_RUNNING) && (count++ < 120))  {
		delay(HZ/2);
	}
}

/***************************************************************************
*
*	ns_alloc() -  Allocate use of a NDD.
*		      This function takes a NDD name as input and returns
*		      a struct ndd pointer if the NDD is succesfully
*		      allocated. If this is the first alloc for a particular
*		      NDD, an associated demuxer must be located and the NDD
*		      must be opened.
*	
***************************************************************************/
ns_alloc(nddname, nddpp)
	register char	*nddname;
	struct ndd 	**nddpp;
{
	register struct ndd 	*nddp;
	int			error = 0;
	struct ns_demuxer	*demuxer;

#define	reterr(errno, out)	{error = errno; goto out/**/_unlock;}

	/*
	 * First see if we can find the requested NDD.
	 */
	lock_write(&ndd_lock);
	lock_set_recursive(&ndd_lock);
	nddp = ns_locate(nddname);
	if (nddp == (struct ndd *) NULL) 
		reterr(ENODEV, ndd);

	lock_write(&ns_demuxer_lock);
	lock_set_recursive(&ns_demuxer_lock);
	/* 
	 * Locate demuxer for the NDD. If there is already an associated
	 * demuxer, a demuxer pointer will be in the ndd struct. If not
	 * look for a demuxer based on NDD type.
	 */
	demuxer = nddp->ndd_demuxer;
	if (demuxer == (struct ns_demuxer *) NULL) {
		demuxer = ns_get_demuxer(nddp->ndd_type);
		if (demuxer == NULL)
			reterr(ENOENT, demux);
	}

	/* 
	 * If this is the first alloc for a NDD the reference count
	 * will be zero and the NDD must be opened.
	 */
	lock_write(&nddp->ndd_lock);
	lock_set_recursive(&nddp->ndd_lock);
	if (nddp->ndd_refcnt == 0) {
		if (nddp->ndd_demuxer != (struct ns_demuxer *) NULL)
			nddp->ndd_demuxsource = 1;
		else {
			nddp->ndd_demuxer = demuxer;
			nddp->ndd_demuxsource = 0;
		}
		/*
		 * Initialize demuxer if it's using common services.
		 */
		if (demuxer->nd_use_nsdmx)
			error = dmx_init(nddp);

		if (!error) {
			nddp->nd_receive = demuxer->nd_receive;
			nddp->nd_status = demuxer->nd_status;
			error = (*(nddp->ndd_open))(nddp);
			if (error == 0) {
				nddp->ndd_demuxer->nd_inuse++;
			}
		}
	}
	if (error == 0) {
		nddp->ndd_refcnt++;
		*nddpp = nddp;
	} else {
		dmx_term(nddp);
	}
	lock_clear_recursive(&nddp->ndd_lock);
	lock_done(&nddp->ndd_lock);
demux_unlock:
	lock_clear_recursive(&ns_demuxer_lock);
	lock_done(&ns_demuxer_lock);
ndd_unlock:
	lock_clear_recursive(&ndd_lock);
	lock_done(&ndd_lock);
	if (error == 0)
		ns_wait(nddp);
	return(error);
}

/***************************************************************************
*
*	ns_free() -  Relinquish use of a NDD.
*	
***************************************************************************/
void
ns_free(nddp)
	struct ndd *nddp;
{

	lock_write(&nddp->ndd_lock);
	lock_set_recursive(&nddp->ndd_lock);
	nddp->ndd_refcnt--;
	assert(nddp->ndd_refcnt >= 0);

	/* 
	 * If reference count drops to zero, close NDD.
	 */
	if (nddp->ndd_refcnt == 0) {
		(*(nddp->ndd_close))(nddp);
		dmx_term(nddp);
		nddp->ndd_demuxer->nd_inuse--;
		/* 
		 * If the demuxer was not provided by the NDD need to NULL 
		 * it out in case it changes before the next alloc.
		 */
		if (nddp->ndd_demuxsource = 1)
			nddp->ndd_demuxsource = 0;
		else
			nddp->ndd_demuxer = (struct ns_demuxer *) NULL;
	}
	lock_clear_recursive(&nddp->ndd_lock);
	lock_done(&nddp->ndd_lock);
	
}

kinfo_ndd(op, where, acopysize, arg, aneeded)
	int	op, arg;
	caddr_t	where;
	int	*acopysize, *aneeded;
{
	struct ndd		*nddp;
	int			needed = 0;
	int			buflen;
	struct kinfo_ndd 	*dp = (struct kinfo_ndd *)where;
	char			name[NDD_MAXNAMELEN];

	if (where != NULL)
		buflen = *acopysize;

	if (arg && copyin(arg, name, sizeof(name)))
		return(EFAULT);

	lock_write(&ndd_lock);

	for (nddp = ndd; nddp ; nddp = nddp->ndd_next) {
		if (arg) {
			if (strncmp(nddp->ndd_name, name, NDD_MAXNAMELEN)) {
				if (nddp->ndd_alias == NULL) 
					continue;
				if (strncmp(nddp->ndd_alias, name, 
			             NDD_MAXNAMELEN))
					continue;
			}
		}
		if (where != NULL && buflen >= sizeof (struct kinfo_ndd)) {
			if (kinfo_ndd_copy(nddp, dp)) {
				lock_done(&ndd_lock);
				return(EFAULT);
			}
			dp++;
			buflen -= sizeof(struct kinfo_ndd);
		}
		needed += sizeof(struct kinfo_ndd);
	}
	lock_done(&ndd_lock);
	if (where != NULL)
		*acopysize = (caddr_t)dp - where;
	*aneeded = needed;
	return (0);
}

kinfo_ndd_copy(nddp, where)
struct ndd	*nddp;
caddr_t		where;
{
	struct kinfo_ndd	kinfo;

	strncpy(kinfo.ndd_name, nddp->ndd_name, NDD_MAXNAMELEN);
	strncpy(kinfo.ndd_alias, nddp->ndd_alias, NDD_MAXNAMELEN);
	kinfo.ndd_flags = nddp->ndd_flags;
	kinfo.ndd_mtu = nddp->ndd_mtu;
	kinfo.ndd_mintu = nddp->ndd_mintu;
	kinfo.ndd_type = nddp->ndd_type;
	kinfo.ndd_addrlen = nddp->ndd_addrlen;
	bcopy(nddp->ndd_physaddr, kinfo.ndd_addr, 
		MIN(sizeof(kinfo.ndd_addr), nddp->ndd_addrlen));
	return(copyout((caddr_t)&kinfo, where, sizeof(struct kinfo_ndd)));
}
