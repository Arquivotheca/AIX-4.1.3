static char sccsid[] = "@(#)21  1.1  src/bos/kernext/dlpi/info.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:28";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: dl_getstats
 *		dl_info
 *		dl_physaddr
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
 * info.c - info requests (info, physaddr, getstats)
 */

#include "include.h"

/* public interfaces */
void dl_info();
void dl_physaddr();
void dl_getstats();

/*
 **************************************************
 * DL_INFO_REQ
 **************************************************
 */

/*
 * dl_info - generate DL_INFO_ACK
 */

void
dl_info(dlb, mp)
	DLB	*dlb;
	mblk_t	*mp;
{
	dl_info_ack_t *ack;
	int len;

	/* variable length reply - alloc max size */
	len = sizeof(dl_info_ack_t) + 2 * MAXADDR_LEN;
	if (mp->b_datap->db_lim - mp->b_datap->db_base < len) {
		mblk_t *mp1;
		if (!(mp1 = allocb(len, BPRI_HI))) {
			incstats(dlb, no_bufs);
			putq(dlb->dlb_rq, errack(mp, DL_INFO_REQ, -ENOSR));
			return;
		}
		freemsg(mp);
		mp = mp1;
	} else {
		mp->b_rptr = mp->b_datap->db_base;
		mp->b_wptr = mp->b_rptr;
	}

	mp->b_datap->db_type = M_PCPROTO;

	bzero(mp->b_rptr, len);
	len = sizeof(dl_info_ack_t);
	ack = (dl_info_ack_t *)mp->b_rptr;

	/* constants for any state */
	ack->dl_primitive = DL_INFO_ACK;
	ack->dl_version = DL_VERSION_2;
	ack->dl_service_mode = dlb->dlb_mode;
	ack->dl_provider_style = DL_STYLE2;
	ack->dl_min_sdu = 1;
	ack->dl_max_sdu = dlb->dlb_n1;
	ack->dl_current_state = dlb->dlb_state;

	if (ATTACHED(dlb)) {
		ack->dl_mac_type = dlb->dlb_mactype;
		ack->dl_brdcst_addr_length = sizeof(dl_broadcast);
		ack->dl_brdcst_addr_offset = len;
		bcopy(dl_broadcast, mp->b_rptr + len, sizeof(dl_broadcast));
		len += ack->dl_brdcst_addr_length;
	}

	if (BOUND(dlb)) {
		ack->dl_sap_length = -dlb->dlb_saplen;
		ack->dl_addr_length = dlb->dlb_addrlen;
		ack->dl_addr_offset = len;
		bcopy(dlb->dlb_addr, mp->b_rptr + len, ack->dl_addr_length);
		len += ack->dl_addr_length;
	}

	mp->b_wptr += len;
	putq(dlb->dlb_rq, mp);
}

/*
 **************************************************
 * DL_PHYS_ADDR_REQ
 **************************************************
 */

/*
 * dl_physaddr - get our current physical address
 */

void
dl_physaddr(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_phys_addr_req_t *req = (dl_phys_addr_req_t *)mp->b_rptr;
	dl_phys_addr_ack_t *ack;
	NDD *ndd = dlb->dlb_ndd;
	int err = 0;
	int len;

	if (!ATTACHED(dlb))
		err = DL_OUTSTATE;
	else if (req->dl_addr_type != DL_CURR_PHYS_ADDR)
		err = DL_UNSUPPORTED;

	if (err) {
		putq(dlb->dlb_rq, errack(mp, DL_PHYS_ADDR_REQ, err));
		return;
	}

	len = sizeof(dl_phys_addr_ack_t) + ndd->ndd_addrlen;
	if (mp->b_datap->db_lim - mp->b_datap->db_base < len) {
		mblk_t *mp1;
		if (!(mp1 = allocb(len, BPRI_LO))) {
			incstats(dlb, no_bufs);
	 		putq(dlb->dlb_rq, errack(mp, DL_PHYS_ADDR_REQ, -ENOSR));
			return;
		}
		freemsg(mp);
		mp = mp1;
	} else
		mp->b_rptr = mp->b_datap->db_base;

	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + len;

	ack = (dl_phys_addr_ack_t *)mp->b_rptr;
	ack->dl_primitive = DL_PHYS_ADDR_ACK;
	ack->dl_addr_length = ndd->ndd_addrlen;
	ack->dl_addr_offset = sizeof(dl_phys_addr_ack_t);
	bcopy(ndd->ndd_physaddr, (char *)&ack[1], ndd->ndd_addrlen);
	putq(dlb->dlb_rq, mp);
}

/*
 **************************************************
 * DL_GET_STATISTICS_REQ
 **************************************************
 */

/*
 * dl_getstats - get the statistics for both per-stream and global
 */

#define	acksz	sizeof(dl_get_statistics_ack_t)
#define	statsz	sizeof(struct statistics)

void
dl_getstats(dlb, mp)
	DLB *dlb;
	mblk_t *mp;
{
	dl_get_statistics_ack_t *ack;
	int len = acksz + 2 * statsz;

	if (mp->b_datap->db_lim - mp->b_datap->db_base < len) {
		mblk_t *mp1;
		if (!(mp1 = allocb(len, BPRI_LO))) {
			incstats(dlb, no_bufs);
	 		putq(dlb->dlb_rq, errack(mp, DL_GET_STATISTICS_REQ, -ENOSR));
			return;
		}
		freemsg(mp);
		mp = mp1;
	} else
		mp->b_rptr = mp->b_datap->db_base;

	mp->b_datap->db_type = M_PCPROTO;

	mp->b_wptr = mp->b_rptr + len;

	ack = (dl_get_statistics_ack_t *)mp->b_rptr;
	ack->dl_primitive = DL_GET_STATISTICS_ACK;
	ack->dl_stat_length = 2 * statsz;
	ack->dl_stat_offset = acksz;
	bcopy(&dlb->dlb_stats, (char *)&ack[1], statsz);
	bcopy(&dl_stats, (char *)&ack[1] + statsz, statsz);
	putq(dlb->dlb_rq, mp);
}
