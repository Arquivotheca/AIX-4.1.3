static char sccsid[] = "@(#)26  1.2  src/bos/kernext/dlpi/multi.c, sysxdlpi, bos41J, 9514A_all 4/4/95 18:37:28";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: addmulti
 *		delmulti
 *		dl_disabmulti
 *		dl_enabmulti
 *		dl_promiscoff
 *		dl_promiscon
 *		mktap
 *		promiscoff
 *		rmtap
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * multi.c - multicast and promiscuous requests
 */

#include "include.h"

/* public routines */
void dl_enabmulti(), dl_disabmulti();
int delmulti();
void dl_promiscon(), dl_promiscoff();
int promiscoff();

/*
 **************************************************
 * DL_ENABMULTI_REQ, DL_DISABMULTI_REQ
 **************************************************
 */

/*
 * dl_enabmulti - enable a multicast address
 */

void
dl_enabmulti(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_enabmulti_req_t *dlem = (dl_enabmulti_req_t *)mp->b_rptr;
	uchar *addr = mp->b_rptr + dlem->dl_addr_offset;
	ulong len = dlem->dl_addr_length;
	int err = 0;

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (dlb->dlb_mode != DL_CLDLS)
		err = DL_NOTSUPPORTED;
	else if ((addr + len > mp->b_wptr) || len != dlb->dlb_ndd->ndd_addrlen)
		err = DL_BADADDR;

	if (err || (err = addmulti(dlb, addr, len))) {
		putq(dlb->dlb_rq, errack(mp, DL_ENABMULTI_REQ, err));
		return;
	}

	putq(dlb->dlb_rq, okack(mp, DL_ENABMULTI_REQ));
}

/*
 * dl_disabmulti - disable a multicast address
 */

void
dl_disabmulti(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_disabmulti_req_t *dldm = (dl_disabmulti_req_t *)mp->b_rptr;
	uchar *addr = mp->b_rptr + dldm->dl_addr_offset;
	ulong len = dldm->dl_addr_length;
	int err = 0;

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (dlb->dlb_mode != DL_CLDLS)
		err = DL_NOTSUPPORTED;
	else if (addr + len > mp->b_wptr || len != dlb->dlb_ndd->ndd_addrlen)	
		err = DL_BADADDR;

	if (err || (err = delmulti(dlb, addr, len))) {
		putq(dlb->dlb_rq, errack(mp, DL_DISABMULTI_REQ, err));
		return;
	}

	putq(dlb->dlb_rq, okack(mp, DL_DISABMULTI_REQ));
}

/*
 * addmulti - the dirty work of adding a multicast address
 */

static int
addmulti(dlb, addr, len)
	DLB *dlb;
	uchar *addr;
	unsigned int len;
{
	multi_t *multi;
	NDD *ndd = dlb->dlb_ndd;
	int err;

	/* if dup, simply ack it */
	for (multi = dlb->dlb_multi; multi; multi = multi->next)
		if (!bcmp(addr, multi->addr, ndd->ndd_addrlen))
			return 0;

	if (!(multi = getmem(multi_t, 1)))
		return -ENOSR;

	/*
	 * register multicast address;
	 * CDLI will return ENOSPC if too many addresses
	 */
	if (err = ndd->ndd_ctl(ndd, NDD_ENABLE_ADDRESS, addr, len))
		return (err == ENOSPC) ? DL_TOOMANY : -err;

	/* remember what we did */
	bcopy(addr, multi->addr, len);
	multi->next = dlb->dlb_multi;
	dlb->dlb_multi = multi;

	incstats(dlb, multicast_addrs);
	return 0;
}

/*
 * delmulti - the dirty work of removing a multicast address
 */

int
delmulti(dlb, addr, len)
	DLB *dlb;
	uchar *addr;
	unsigned int len;
{
	multi_t *multi, **pp;
	NDD *ndd = dlb->dlb_ndd;
	int err;

	/* check if already enabled */
	for (pp = &dlb->dlb_multi; *pp; pp = &(*pp)->next) {
		if (!bcmp(addr, (*pp)->addr, len))
			break;
	}
	if (!(multi = *pp))
		return DL_NOTENAB;

	/*
	 * unregister multicast address;
	 * CDLI will return EINVAL if we're confused (nothing registered)
	 * NB: continue after failure: we must remove the record - see close
	 */
	if (err = ndd->ndd_ctl(ndd, NDD_DISABLE_ADDRESS, addr, len))
		err = (err == EINVAL) ? DL_NOTENAB : -err;

	*pp = multi->next;
	putmem(multi);

	/* decstats(dlb, multicast_addrs);	* XXX really? */
	return err;
}

/*
 **************************************************
 * DL_PROMISCON_REQ, DL_PROMISCOFF_REQ
 **************************************************
 */

/*
 * mktap - create a tap user so CDLI will not filter packets
 */

static int
mktap(dlb)
	DLB *dlb;
{
	struct ns_8022 ns;
	struct ns_user ns_user;

	extern void dlpiraw();

	bzero(&ns, sizeof(ns));
	bzero(&ns_user, sizeof(ns_user));

	/* CDLI ignores everything else if filtertype is NS_TAP. */
	ns.filtertype = NS_TAP;

	ns_user.isr = dlpiraw;
	ns_user.isr_data = (caddr_t)dlb;
	ns_user.pkt_format = NS_INCLUDE_MAC | NS_HANDLE_HEADERS;

	return ns_add_filter(dlb->dlb_ndd, &ns, sizeof(ns), &ns_user);
}

/*
 * rmtap - undo effects of mktap
 */

static int
rmtap(dlb)
	DLB *dlb;
{
	struct ns_8022 ns;

	bzero(&ns, sizeof(ns));
	ns.filtertype = NS_TAP;
	return ns_del_filter(dlb->dlb_ndd, &ns, sizeof(ns));
}

/*
 * dl_promiscon - enable promiscuous mode
 */

void
dl_promiscon(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	NDD *ndd = dlb->dlb_ndd;
	int level = ((dl_promiscon_req_t *)mp->b_rptr)->dl_level;
	int err = 0;

	if (!ATTACHED(dlb))
		err = DL_OUTSTATE;
	else if (!dlb->dlb_priv)
		err = DL_ACCESS;
	else if (dlb->dlb_promisc)
		err = DL_OUTSTATE;
	else
		err = mktap(dlb);

	if (err) {
		putq(dlb->dlb_rq, errack(mp, DL_PROMISCON_REQ, err));
		return;
	}

	/*
	 * only one level of promiscuity is possible at a time;
	 * consider: _MULTI and _SAP are subclasses of _PHYS,
	 * and the DLPI bit definitions are not coded as bitflags.
	 */
	dlb->dlb_promisc = level;

	switch (level) {
	case DL_PROMISC_PHYS:
		if (!(err = ndd->ndd_ctl(ndd, NDD_PROMISCUOUS_ON, 0, 0))) {
			dlb->dlb_stats.promisc_phys = PROMISCUOUS_ON;
			incstats(0, promisc_phys);
		}
		break;
	case DL_PROMISC_SAP:
		if (!(err = ndd->ndd_ctl(ndd, NDD_PROMISCUOUS_ON, 0, 0))) {
			dlb->dlb_stats.promisc_sap = PROMISCUOUS_ON;
			incstats(0, promisc_sap);
		}
		break;
	case DL_PROMISC_MULTI:
		if (!(err = ndd->ndd_ctl(ndd, NDD_ENABLE_MULTICAST, 0, 0))) {
			dlb->dlb_stats.promisc_multi = PROMISCUOUS_ON;
			incstats(0, promisc_multi);
		}
		break;
	default:
		err = EINVAL;
	}

	if (err) {
		(void)rmtap(dlb);
		dlb->dlb_promisc = 0;
		putq(dlb->dlb_rq, errack(mp, DL_PROMISCON_REQ, -err));
		return;
	}

	putq(dlb->dlb_rq, okack(mp, DL_PROMISCON_REQ));
}

/*
 * dl_promiscoff - disable promiscuous mode
 */

void
dl_promiscoff(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	int level = ((dl_promiscoff_req_t *)mp->b_rptr)->dl_level;
	int err = 0;

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (!dlb->dlb_priv)
		err = DL_ACCESS;
	else if (!dlb->dlb_promisc || level != dlb->dlb_promisc)
		err = DL_NOTENAB;

	if (err || (err = promiscoff(dlb, level))) {
		putq(dlb->dlb_rq, errack(mp, DL_PROMISCOFF_REQ, err));
		return;
	}

	dlb->dlb_promisc = 0;
	putq(dlb->dlb_rq, okack(mp, DL_PROMISCON_REQ));
}

/*
 * promiscoff - shared routine to turn off promiscuity
 */

int
promiscoff(dlb, level)
	DLB *dlb;
	int level;
{
	NDD *ndd = dlb->dlb_ndd;
	int err;

	/* stop the deluge */
	if (err = rmtap(dlb))
		return -err;	/* XXX should continue if notenab failure */

	switch (level) {
	case DL_PROMISC_PHYS:
		if (!(err = ndd->ndd_ctl(ndd, NDD_PROMISCUOUS_OFF, 0, 0))) {
			dlb->dlb_stats.promisc_phys = PROMISCUOUS_OFF;
			/* XXX decstats(0, promisc_phys); */
		}
		break;
	case DL_PROMISC_SAP:
		if (!(err = ndd->ndd_ctl(ndd, NDD_PROMISCUOUS_OFF, 0, 0))) {
			dlb->dlb_stats.promisc_sap = PROMISCUOUS_OFF;
			/* XXX decstats(0, promisc_sap); */
		}
		break;
	case DL_PROMISC_MULTI:
		if (!(err = ndd->ndd_ctl(ndd, NDD_DISABLE_MULTICAST, 0, 0))) {
			dlb->dlb_stats.promisc_multi = PROMISCUOUS_OFF;
			/* XXX decstats(0, promisc_multi); */
		}
		break;
	default:
		err = EINVAL;
	}
	return -err;
}
