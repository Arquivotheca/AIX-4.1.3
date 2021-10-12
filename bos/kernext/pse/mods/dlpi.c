static char sccsid[] = "@(#)36  1.17.1.22  src/bos/kernext/pse/mods/dlpi.c, sysxpse, bos41J, 9515A_all 4/5/95 11:36:48";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS: ATTACHED
 *		BCOPY
 *		BOUND
 *		GETDSAP
 *		GETSSAP
 *		GETTYPE
 *		cleanup_multi
 *		cleanup_promisc
 *		dlb_add_type
 *		dlb_attach
 *		dlb_bind
 *		dlb_bind_sap
 *		dlb_close
 *		dlb_config
 *		dlb_del_type
 *		dlb_detach
 *		dlb_disabmulti
 *		dlb_enabmulti
 *		dlb_error
 *		dlb_error_ack
 *		dlb_get_statistics
 *		dlb_info
 *		dlb_init
 *		dlb_intr
 *		dlb_ok_ack
 *		dlb_open
 *		dlb_phys_addr
 *		dlb_prim
 *		dlb_promiscoff
 *		dlb_promiscon
 *		dlb_rsrv
 *		dlb_subs_bind
 *		dlb_subs_unbind
 *		dlb_tap_intr
 *		dlb_term
 *		dlb_uderror
 *		dlb_unbind
 *		dlb_unbind_sap
 *		dlb_wput
 *		getbind
 *		findndd
 *		freebind
 *		isnetware
 *		m_freeb
 *		mk_tapuser
 *		mknddname
 *		ndd_to_dlb_mactype
 *		remove_link
 *		samesnap
 *		snap_type
 *
 *   ORIGINS: 27,63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** dlaix.c 1.2
 **/


/* STREAMS DLPI to network interface module for AIX */


#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/syspest.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <net/nd_lan.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>
#include <netinet/if_fddi.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/dlpi.h>
#include <pse/mi.h>
#include <sys/dlpistats.h>

#ifndef staticf
#define staticf static
#endif

#define	GETTYPE(x)	(((union sapu *)&x)->su_dst.type)
#define	GETDSAP(x)	(((union sapu *)&x)->su_dst.dsap)
#define	GETSSAP(x)	(((union sapu *)&x)->su_dst.ssap)

#define	ATTACHED(dlb)	((dlb)->dlb_ndd)
#define	BOUND(dlb)	((dlb)->dlb_sap)

#define BCOPY(f,t,l) {\
        typedef struct { char x[l]; } a_t;\
        *(a_t*)(t) = *(a_t*)(f);\
}

#define SIZEOF_LLC      3
#define	MAC_LEN		6
#define	SNAP_LEN	5
#define	MACSAP_LEN	(MAC_LEN + 1)
#define	MACSNAP_LEN	(MACSAP_LEN + SNAP_LEN)
#define	MAX_ROUTE_LEN	(sizeof(ushort) + sizeof(ushort) * 14) /*fddi_mac_hdr*/
#define	MAX_ADDR_LEN	(MACSNAP_LEN + MAX_ROUTE_LEN)

typedef	struct ndd	NDD, * NDDP;
typedef struct mbuf	MBUF, * MBUFP;
typedef struct msgb	* MBLKP;

typedef struct {
	uchar	org_id[3];
	uchar	proto_id[2];
} snap_t;
#define	snap_type(p)	(*(ushort *)(((snap_t *)p)->proto_id))

typedef union sapu {
	struct {
		uchar	dsap;
		uchar	ssap;
		ushort	type;
	} su_dst;
	ulong	su_sap;
} sapu_t;

typedef struct address {
	char  address[6];
	struct address  *next;
};

/*
 * bind_t - N:1 with a stream, these are bound to types
 */
typedef struct bind_s {
	struct bind_s *next;            /* other records bound on this type */
	struct bind_s *dlbnext;         /* other records on this stream */
	struct dlb_s  *dlb;             /* used to discover stream */
	struct ie2_llc_snaphdr llc;     /* used to check snaps */
	ushort type;                    /* used for unbind */
	ushort class;                   /* used for unbind */
} bind_t;

static bind_t	*bindlist;		/* Head of bind_t linked list */

typedef struct dlb_s {
	queue_t			*dlb_rq;
	int			dlb_addr_length;
	char    		dlb_addr[MACSNAP_LEN];
	struct	ie2_llc_snaphdr	dlb_llc;
	NDDP 			dlb_ndd;
	bind_t  	       *dlb_bound;  /* linked list of bound types */
	sapu_t			dlb_sapu;
#define	dlb_sap 	dlb_sapu.su_sap
#define	dlb_dsap	dlb_sapu.su_dst.dsap
#define	dlb_ssap	dlb_sapu.su_dst.ssap
#define	dlb_type	dlb_sapu.su_dst.type
	long			dlb_saplen;
	int			dlb_netware; /* are we NetWare special? */
	char			dlb_nddname[FMNAMESZ+1]; /* ndd specific data */
	ulong			dlb_pkt_format;
	struct	address		*dlb_addr_list;/* multicast-enabled addresses */
	ushort			dlb_promisc_flags;/*promiscuous mode per level*/
#define DL_PROMISC_MULTI_FLAG   0x04    	/* for bitmask checking only */
#define DL_PROMISC_PRIV		0x08	/* has privilege to be promoisous */
	struct	statistics	dlb_stats;
	int			(*dlb_output)();/* output resolve routine */
	void			(*dlb_input)();	/* intput resolve routine */
} DLB, * DLBP, ** DLBPP;

staticf	MBLKP	dlb_attach(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_bind(   DLBP dlb, MBLKP mp   );
staticf	int	dlb_close(   queue_t * q   );
staticf	MBLKP	dlb_detach(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_error_ack(   MBLKP mp, ulong prim, int unix_error, int dl_error, DLBP dlb   );
staticf	MBLKP	dlb_ok_ack(   MBLKP mp, ulong prim, DLBP dlb   );
staticf	int	dlb_open(  queue_t * q, dev_t * devp, int flag, int sflag, cred_t * credp   );
staticf	void	dlb_rsrv(   queue_t * q   );
staticf	MBLKP	dlb_unbind(   DLBP dlb, MBLKP mp   );
staticf	int	dlb_wput(   queue_t * q, MBLKP mp   );

staticf	MBLKP	dlb_error();
staticf	MBLKP	dlb_subs_bind();
staticf	MBLKP	dlb_subs_unbind();
staticf	MBLKP	dlb_enabmulti();
staticf	MBLKP	dlb_disabmulti();
staticf	MBLKP	dlb_promiscon();
staticf	MBLKP	dlb_promiscoff();
staticf MBLKP   dlb_phys_addr();
staticf MBLKP   dlb_get_statistics();
staticf	int	dlb_bind_sap();
staticf	int	dlb_unbind_sap();

void		remove_link();
void		cleanup_multi();
void		cleanup_promisc();

staticf	int	dlb_intr(NDDP ndd, MBUFP m, caddr_t llhdr,
						struct isr_data_ext *isr);
staticf	int	dlb_tap_intr(NDDP ndd, MBUFP m, caddr_t llhdr,
						struct isr_data_ext *isr);
extern	int	ns_alloc(   char * nddname, struct ndd	**nddpp);

#define	MODNUM	5010
static struct module_info minfo =  {
	MODNUM, "dlpi", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	nil(pfi_t), (pfi_t)dlb_rsrv, dlb_open, dlb_close, nil(pfi_t), &minfo
};

static struct qinit winit = {
	dlb_wput, nil(pfi_t), nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

struct streamtab dlbinfo = { &rinit, &winit };

static	IDP	dlb_g_head = nil(IDP);
struct	statistics	all_stats;  /* No MP locks - global stats are "vague" */

/*
 * this table is used to:
 *      a. map between a major number and the ndd name
 *      b. the table is dynamically extended by calls to dlb_config.
 */

struct dl_data {
	char    ndd_name[FMNAMESZ+1];   /* name used by ndd */
	dev_t   maj;                    /* major number for this ndd */
	struct  dl_data *next;         /* linked-list of ndd's */
};

struct dl_data *dl_data = (struct dl_data *) 0;

static unsigned char broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/* MP declarations */
int  dlpi_lock = LOCK_AVAIL;
#define DLPILOCK        (void)lockl(&dlpi_lock,LOCK_SHORT)
#define DLPIUNLOCK      (void)unlockl(&dlpi_lock)
int  bind_lock = LOCK_AVAIL;
#define BINDLOCK        (void)lockl(&bind_lock,LOCK_SHORT)
#define BINDUNLOCK      (void)unlockl(&bind_lock)

/*
 * findndd - find interface by major number
 */

static struct dl_data *
findndd(maj)
	int maj;
{
	struct dl_data *p;
	for (p = dl_data; p; p = p->next) {
		if (maj == p->maj)
			return p;
	}
	return 0;
}

/*
 * mknddname - return a pointer to an ndd name suitable for ns_alloc()
 *
 * ppa is assumed to be 0 <= ppa < 99
 * base is assumed to be short, such that strlen(base) < sizeof buf
 */

char *
mknddname(base, ppa)
	char *base;
	int ppa;

{
	static char buf[100];
	char *cp = buf;

	while (*cp++ = *base++)
		;
	if (ppa < 10) {                 /* one digit ppa (0-9) */
		cp[-1] = ppa + '0';	/* cheap ascii conversion */
		*cp = 0;
	} else { 			/* two digit ppa (10-99) */
		cp[-1] = (ppa / 10) % 10 + '0';
		*cp = ppa % 10 + '0';
		*++cp = 0;
	}
	return buf;
}

/*
 * isnetware - For the padding purpose check whether the sap is for NetWare 
 *	       or not. Token-Ring and FDDI does not need this.
 */

#define isnetware(dlb, s)				\
	((dlb)->dlb_ndd->ndd_type == NDD_ISO88023 &&	\
	 (s == SSAP_NETWARE || s == ETHERTYPE_NETWARE))


/* ndd_to_dlb_mactype - convert ndd mactype to dlpi mactypes */

ulong
ndd_to_dlb_mactype(dlb)
	DLBP	dlb;
{

	switch(dlb->dlb_ndd->ndd_type) {
	case NDD_ISO88023:
		if (!strcmp(dlb->dlb_nddname, "en"))
			return DL_ETHER;
		else
			return DL_CSMACD;
	case NDD_ISO88025:
		return DL_TPR;
	case NDD_ISO88024:
		return DL_TPB;
	case NDD_ISO88026:
		return DL_METRO;
	case NDD_FDDI:
		return DL_FDDI;
	default:
		return DL_OTHER;
	}
}

/*
 * dlb_attach - attach interface to this driver
 *
 * the attachment is accomplished by setting dlb_ndd in the DLB struct
 */

staticf MBLKP
dlb_attach(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	int len;
	int ppa;
	char *name;
	NDDP ndd;

	if (ATTACHED(dlb))
		return dlb_error_ack(mp, DL_ATTACH_REQ, 0, DL_OUTSTATE, dlb);

	ppa = ((dl_attach_req_t *)(mp->b_rptr))->dl_ppa;

	/* "en" and "et" both uses same ndd, so to open "et", open "en" */
	if (!strcmp(dlb->dlb_nddname, "et"))
		name = mknddname("en", ppa);
	else
		name = mknddname(dlb->dlb_nddname, ppa);
	if (ns_alloc(name, &ndd)) {
		char msg[100];	/* big enough for error message */
		strcpy(msg, "could not find interface ");
		strcat(msg, name);	/* strlog cannot do %s conversions */
		strlog(MODNUM, 0, 0, SL_ERROR, msg);
		return dlb_error_ack(mp, DL_ATTACH_REQ, 0, DL_BADPPA, dlb);
	}

	len = ndd->ndd_addrlen;
	if (len > sizeof dlb->dlb_addr) {
		strlog(MODNUM, 0, 0, SL_ERROR, "ndd_addr too long for driver");
		return dlb_error_ack(mp, DL_ATTACH_REQ, 0, DL_BADADDR, dlb);
	}
	bcopy(ndd->ndd_physaddr, dlb->dlb_addr, len);
	dlb->dlb_ndd = ndd;
	dlb->dlb_addr_length = len;

	/* assign the output and input resolve routines */
	if (ndd->ndd_demuxer->nd_address_resolve)
		dlb->dlb_output = ndd->ndd_demuxer->nd_address_resolve;
	if (ndd->ndd_demuxer->nd_address_input)
		dlb->dlb_input = ndd->ndd_demuxer->nd_address_input;

	return dlb_ok_ack(mp, DL_ATTACH_REQ, dlb);
}

/*
 * dlb_detach - unhook us from dlb_ndd
 */

staticf MBLKP
dlb_detach (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	/* The board may only be detached if
	 * the stream is attached but unbound
	 */
	if (!ATTACHED(dlb) || BOUND(dlb))
		return mp ? dlb_error_ack(mp, DL_DETACH_REQ, 0, DL_OUTSTATE,
			dlb) : nil(MBLKP);

	if (dlb->dlb_promisc_flags)
		cleanup_promisc(dlb);
	if (dlb->dlb_addr_list)
		cleanup_multi(dlb);

	ns_free(dlb->dlb_ndd);
	dlb->dlb_ndd = nilp(NDD);

	return mp ? dlb_ok_ack(mp, DL_DETACH_REQ, dlb) : nil(MBLKP);
}

/*
 * dlb_bind - register sap with interface
 */

staticf MBLKP
dlb_bind(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_bind_req_t	* dlbr;
	dl_bind_ack_t	* dlba;
	ulong		sap;
	int		len, err;
	mblk_t		*mp1;

	/* if not attached, or already bound, then out of state */
	if (!ATTACHED(dlb) || BOUND(dlb))
	 	return dlb_error_ack(mp, DL_BIND_REQ, 0, DL_OUTSTATE, dlb);

	dlbr = (dl_bind_req_t *)mp->b_rptr;
	if (dlbr->dl_xidtest_flg & ~(DL_AUTO_XID | DL_AUTO_TEST))
		return dlb_error_ack(mp, DL_BIND_REQ, EINVAL, DL_SYSERR, dlb);

	/* DLPI only supports DL_CLDLS */
	if (dlbr->dl_service_mode != DL_CLDLS)
		return dlb_error_ack(mp, DL_BIND_REQ, 0, DL_UNSUPPORTED, dlb);

	/* this keeps us from binding to the 802.2 null sap */
	sap = dlbr->dl_sap;
	if (!sap)
		return dlb_error_ack(mp, DL_BIND_REQ, 0, DL_BADADDR, dlb);

	strlog(MODNUM,0,0,SL_TRACE,"dlb_bind: dlb %x sap %x", dlb, sap);

	/* obtain an mblk big enough for a response */
	len = sizeof(dl_bind_ack_t) +dlb->dlb_addr_length +sizeof(dlb->dlb_llc);
	if ( (mp->b_datap->db_lim - mp->b_datap->db_base) < len ) {
		if (!(mp1 = allocb(len, BPRI_HI))) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
			return dlb_error_ack(mp, DL_BIND_REQ, ENOMEM,
				DL_SYSERR, dlb);
		}
		freemsg(mp);
		mp = mp1;
	} else
		mp->b_rptr = mp->b_datap->db_base;

	/* register type with system */
	if (err = dlb_bind_sap(dlb, sap))
		return dlb_error(mp, DL_BIND_REQ, err, dlb);
	dlb->dlb_sap = sap; 

	/* format response */
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + len;
	dlba = (dl_bind_ack_t *)mp->b_rptr;
	dlba->dl_primitive = DL_BIND_ACK;
	dlba->dl_sap = dlb->dlb_sap;
	dlba->dl_addr_length = dlb->dlb_addr_length;
	dlba->dl_addr_offset = sizeof(dl_bind_ack_t);
	dlba->dl_max_conind = 0;
	dlba->dl_xidtest_flg = DL_AUTO_XID | DL_AUTO_TEST;
	bcopy(dlb->dlb_addr, (char *)&dlba[1], dlb->dlb_addr_length);

	dlb->dlb_stats.binds++;
	all_stats.binds++;
	
	return mp;
}

/*
 * dlb_unbind - unbind a SAP
 *
 * must unbind ALL bound information; snaps included (DLPI 4.1.8)
 */

staticf MBLKP
dlb_unbind (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	int	err;

	if (!BOUND(dlb))
		return mp ? dlb_error_ack(mp, DL_UNBIND_REQ, 0, DL_OUTSTATE,
			dlb) : nil(MBLKP);

	if (err = dlb_unbind_sap(dlb))
		return mp ? dlb_error(mp, DL_UNBIND_REQ, err, dlb) : nil(MBLKP);

	dlb->dlb_sap = 0;
	dlb->dlb_stats.binds--;
	all_stats.binds--;
	return mp ? dlb_ok_ack(mp, DL_UNBIND_REQ, dlb) : nil(MBLKP);
}

/*
 * dlb_subs_bind - bind a SNAP
 *               - hierarchical and peer binds
 *
 * if bind->type != 0xAA, then either
 *	- we bound to some non-snap sap
 *	- we have already done a subs_bind
 */

staticf MBLKP
dlb_subs_bind(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_subs_bind_req_t *dlsbr = (dl_subs_bind_req_t *)mp->b_rptr;
	int err = 0;
	struct	ns_8022	dl;
	struct ie2_llc_snaphdr llc;
	uchar *sap = mp->b_rptr + dlsbr->dl_subs_sap_offset;
	int len = dlsbr->dl_subs_sap_length;
	int class = dlsbr->dl_subs_bind_class;
	ushort  type;
	bind_t *wasbound;
	uchar  *snap;

	bzero(&llc, sizeof(llc));
	bzero(&dl, sizeof(dl));
	wasbound = dlb->dlb_bound;
	snap = NULL;

	strlog(MODNUM,0,0,SL_TRACE,"dlb_subs_bind: dlb %x class=%d", dlb,
		class);

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (class != DL_HIERARCHICAL_BIND && class != DL_PEER_BIND)
		err = DL_NOTSUPPORTED;
	else if (sap + len > mp->b_wptr)
		err = DL_BADADDR;
	else if (ndd_to_dlb_mactype(dlb) == DL_ETHER) {
		if (class != DL_PEER_BIND)
			err = DL_UNSUPPORTED;
		else if (len != sizeof(ushort))
			err = DL_BADADDR;
		else {
			type = *(ushort *)sap;
			if (type < 0x600)
				err = DL_BADADDR;
			dl.filtertype = NS_ETHERTYPE;
			dl.ethertype = type;
		}
	} else {
		if (len == sizeof(uchar)) {
			type = *sap;
			dl.filtertype = NS_8022_LLC_DSAP;
			dl.dsap = type;
			llc.ssap = llc.dsap = type;
			llc.ctrl = CTRL_UI;
			if (type == SSAP_INET)
				err = DL_BADADDR;
		} else if (len == sizeof(snap_t)) {
			dl.filtertype = NS_8022_LLC_DSAP_SNAP;
			snap = sap;
			type = snap_type(sap);
			llc.ssap = llc.dsap = SSAP_INET;
			llc.ctrl = CTRL_UI;
			llc.type = type;
			dl.dsap = SSAP_INET;
			dl.ethertype = type;
			bcopy(sap, dl.orgcode, sizeof(dl.orgcode));
			bcopy(sap, llc.prot_id, len);
		} else
			err = DL_BADADDR;
	}
	
	if (err)
		return dlb_error_ack(mp, DL_SUBS_BIND_REQ, 0, err, dlb);

        if (err = dlb_add_type(dlb, type, &llc, 1, &dl))
		return dlb_error(mp, DL_SUBS_BIND_REQ, err);

	/*
	 * If no bind records existed before this bind, then the current
	 * address is incomplete, because no binds where completed;
	 * this will only occur when the DL_BIND_REQ was for 0xaa.
	 * In this case, this is the first subs bind, and it will
	 * contain the rest of the DLSAP.
	 *
	 * As a special case, not really specified by the DLPI spec,
	 * if the DL_BIND_REQ was for 0xaa, but this _first_ subs bind
	 * contains only a sap (not a snap), then the DL_BIND_REQ is
	 * ignored, and this subs bind will replace it.  Other interpretations
	 * can cause confusion or inconsistant treatment.  In particular,
	 * consider the case:
	 *	dl_bind_req(0xaa); dl_subs_bind(peer, 0xfe);
	 * The first bind is, by definition, incomplete, thus unusable,
	 * but enough to transit the state machine to DL_IDLE.  The
	 * subs bind does contain complete info, although it is not
	 * a continuation of the snap sap.  This implementation will
	 * prefer the 0xfe to the 0xaa when reporting the current address
	 * and when sending datagrams using the default address.
	 */

	if (!wasbound) {
		dlb->dlb_llc = llc;
		if (!snap) {
			/* replace 0xaa with new sap */
			dlb->dlb_addr[dlb->dlb_addr_length] = *sap;
		} else {
			/* append snap to the current address */
			bcopy(sap, dlb->dlb_addr+dlb->dlb_addr_length,
				sizeof(snap_t));
			dlb->dlb_addr_length += sizeof(snap_t);
			dlb->dlb_saplen += sizeof(snap_t);
		}
	}

	dlsbr->dl_primitive = DL_SUBS_BIND_ACK;
	return mp;
}

/*
 * dlb_subs_unbind - unbind a SNAP
 *
 * if dlb_sap != 0xAA, then we bound to a non-snap sap
 * if bind->type == 0xAA, then we have already done the subs_unbind
 */

staticf MBLKP
dlb_subs_unbind(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_subs_unbind_req_t *dlsur = (dl_subs_unbind_req_t *)mp->b_rptr;
	int err = 0;
	struct	ns_8022	dl;
	uchar *sap = mp->b_rptr + dlsur->dl_subs_sap_offset; /* unbind this */
	int len = dlsur->dl_subs_sap_length;
	ushort curtype;
	ushort type;

	if (!BOUND(dlb))
		err = DL_OUTSTATE;
	else if (sap + len > mp->b_wptr)
		err = DL_BADADDR;
	else if (ndd_to_dlb_mactype(dlb) == DL_ETHER) {
		if (len == sizeof(ushort)) {
			type = *(ushort *)sap;
			if (type < 0x600)
				err = DL_BADADDR;
			dl.filtertype = NS_ETHERTYPE;
			dl.ethertype = type;
		}
		else
			err = DL_BADADDR;
	} else {
		if (len == sizeof(uchar)) {
			type = *sap;
			dl.filtertype = NS_8022_LLC_DSAP;
			dl.dsap = type;
			if (type == SSAP_INET)
				err = DL_BADADDR;
		}
		else if (len == sizeof(snap_t)) {
			type = snap_type(sap);
			dl.filtertype = NS_8022_LLC_DSAP_SNAP;
			dl.dsap = SSAP_INET;
			dl.ethertype = type;
			bcopy(sap, dl.orgcode, sizeof(dl.orgcode));
		}
		else
			err = DL_BADADDR;
	}

	if (err)
		return dlb_error_ack(mp, DL_SUBS_UNBIND_REQ, 0, err, dlb);

	/* remember what the first bound type is; see below */
	curtype = dlb->dlb_bound->type;

	err = dlb_del_type(dlb, type, 1);
	if (err < 0)
		return dlb_error(mp, DL_SUBS_UNBIND_REQ, err);

	if (err = ns_del_filter(dlb->dlb_ndd, &dl, sizeof(dl))) 
		return dlb_error(mp, DL_SUBS_UNBIND_REQ, err, dlb);

	/*
	 * See the note in dlb_subs_bind.
	 *
	 * If the first DLSAP bound was just unbound, then there will
	 * be confusion for current address reporting and the default
	 * address when sending datagrams.  Unfortunately, this scenario
	 * is not discussed in the DLPI spec.  This implementation will
	 * simply replace the first bound DLSAP with the next bound DLSAP.
	 *
	 * If there are no further bind records, then the first bound
	 * address MUST have been 0xaa, since that is the only way
	 * to have the last bound address removed by a subs unbind.
	 * In this case, the address is recreated as in dlb_bind_sap.
	 */

	if (type == curtype) {
		bind_t *bind = dlb->dlb_bound;

		/* will be at least an incomplete snap (0xaa only) */
		dlb->dlb_addr_length = dlb->dlb_ndd->ndd_addrlen + 1;
		dlb->dlb_saplen = 1;

		if (!bind) {
			bzero(&dlb->dlb_llc, sizeof(dlb->dlb_llc));
			dlb->dlb_llc.ssap = SSAP_INET;
			dlb->dlb_llc.dsap = SSAP_INET;
			dlb->dlb_llc.ctrl = CTRL_UI;
		} else {
			dlb->dlb_llc = bind->llc;
			if (dlb->dlb_llc.ssap == SSAP_INET) {
				bcopy(dlb->dlb_llc.prot_id,
					dlb->dlb_addr + dlb->dlb_addr_length,
					sizeof(snap_t));
				dlb->dlb_addr_length += sizeof(snap_t);
				dlb->dlb_saplen += sizeof(snap_t);
			}
		}
	}

	return dlb_ok_ack(mp, DL_SUBS_UNBIND_REQ, dlb);
}

/*
 * dlb_enabmulti - enable specific multicast addresses on a per Streams basis
 *
 */

staticf MBLKP
dlb_enabmulti(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	ulong  len, offset;
	char   address[6];
	struct address  *link, *tail;
	int    rc, i=0;

	/* If not attached then out of state. */
	if (!ATTACHED(dlb))
	 	return dlb_error_ack(mp, DL_ENABMULTI_REQ, 0, DL_OUTSTATE, dlb);

	/*
	 * Make sure that this thread is not running in the interrupt
	 * environment.  dlpi must be running in the process environment in
	 * order to issue the ndd_ctl().  If dlpi is running in the
	 * interrupt environment, return DL_SYSERR.
	 */
	if (getpid() == -1)
	 	return dlb_error_ack(mp, DL_ENABMULTI_REQ, 0, DL_SYSERR, dlb);

	/* Get the address the user is requesting to be enabled. */
	len = ((dl_enabmulti_req_t *)(mp->b_rptr))->dl_addr_length;
	if (len <= 0 || len != MAC_LEN)
		return dlb_error_ack(mp, DL_ENABMULTI_REQ, 0, DL_BADADDR, dlb);

	offset = ((dl_enabmulti_req_t *)(mp->b_rptr))->dl_addr_offset;
	if (mp->b_rptr + offset + len > mp->b_wptr)
		return dlb_error_ack(mp, DL_ENABMULTI_REQ, 0, DL_BADADDR, dlb);

	bcopy(mp->b_rptr + offset, address, len);

	/* 
	 * Look in the linked list of addresses and see if this address has
	 * already been enabled.  If already enabled, do nothing.  Keep track
	 * of the tail so that a new link can be added in quickly.
	 */

	link = tail = dlb->dlb_addr_list;
	while (link) {
		if (!bcmp(address, link->address, len)) 
			return dlb_ok_ack(mp, DL_ENABMULTI_REQ, dlb);
		link = link->next;
		if (i)
			tail = tail->next;
		i++;
	}
	
	/* Enable this new address.  */
	rc = dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd, NDD_ENABLE_ADDRESS,
			(caddr_t)(mp->b_rptr + offset), len);

	/*
	 * If there are too many multicast address enable attempts, the
	 * device driver will return ENOSPC.
	 */
	if (rc == ENOSPC)
		return dlb_error_ack(mp, DL_ENABMULTI_REQ, 0, DL_TOOMANY, dlb);
	if (rc)
		return dlb_error_ack(mp, DL_ENABMULTI_REQ, rc, DL_SYSERR, dlb);

	/*
	 * Add this new address to the end of the list now that it has been
	 * successfully enabled.  If no space, no big deal, don't add it.
	 * The disadvantage of not keeping track of an address is that it will
	 * never be disabled if the stream is closed before the
	 * DL_DISABMULTI_REQ primitive is issued.  The space will get freed
	 * when the address is disabled, either by dlb_disabmulti() or
	 * cleanup_multi().
	 */
	if (link = (struct address *)xmalloc(sizeof(struct address), 3,
			pinned_heap)) {
		bcopy (mp->b_rptr + offset, link->address, len);
		link->next = NULL;
		if (!dlb->dlb_addr_list)
			dlb->dlb_addr_list = link;
		else
			tail->next = link;
		dlb->dlb_stats.multicast_addrs++;
		all_stats.multicast_addrs++;
	} else {
		dlb->dlb_stats.no_bufs++;
		all_stats.no_bufs++;
	}

	return dlb_ok_ack(mp, DL_ENABMULTI_REQ, dlb);
}

/*
 * dlb_disabmulti - disable specific multicast addresses on a per Streams basis
 */

staticf MBLKP
dlb_disabmulti(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	ulong  len, offset;
	char   address[6];
	struct address  *link_ptr, *old_link;
	int    rc, i=0;

	/* if not attached then out of state */
	if (!ATTACHED(dlb))
	 	return dlb_error_ack(mp, DL_DISABMULTI_REQ, 0, DL_OUTSTATE,
			dlb);

	/* 
	 * Make sure that this thread is not running in the interrupt
	 * environment.  dlpi must be running in the process environment in
	 * order to issue the ndd_ctl().  If dlpi is running in the
	 * interrupt environment, return DL_SYSERR.
	 */
	if (getpid() == -1)
	 	return dlb_error_ack(mp, DL_DISABMULTI_REQ, 0, DL_SYSERR, dlb);

	/* Get the address the user is requesting to be disabled. */
	len = ((dl_disabmulti_req_t *)(mp->b_rptr))->dl_addr_length;
	if (len <= 0 || len != MAC_LEN)
		return dlb_error_ack(mp, DL_DISABMULTI_REQ, 0, DL_BADADDR, dlb);

	offset = ((dl_disabmulti_req_t *)(mp->b_rptr))->dl_addr_offset;
	bcopy(mp->b_rptr + offset, address, len);

	/* 
	 * Look in the linked list of addresses and see if this address has
	 * already been enabled.  If it is not enabled, then return a
	 * DL_NOTENAB.
	 */
	link_ptr = old_link = dlb->dlb_addr_list;
	while (old_link) {
		if (!bcmp(address, old_link->address, len))
			break;
		old_link = old_link->next;
		if (i)
			link_ptr = link_ptr->next;
		i++;
	}
	if (!old_link) 
		return dlb_error_ack(mp, DL_DISABMULTI_REQ, 0, DL_NOTENAB, dlb);

	/* Disable this address.  */
	rc = dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd, NDD_DISABLE_ADDRESS, 
					address, len);

	/* 
	 * If the device driver has not enabled this address, the device
	 * driver will return EINVAL.  Remove the link from the list.
	 */
	if (rc == EINVAL) {
		remove_link(dlb, old_link, link_ptr);
		return dlb_error_ack(mp, DL_DISABMULTI_REQ, 0, DL_NOTENAB, dlb);
	}

	/*
	 * If the device driver returns any other error, leave the link in
	 * the list, and either the user can issue the primitive again, or
	 * else DLPI will attempt to disable the address before the stream
	 * is closed.
	 */
	if (rc)
		return dlb_error_ack(mp, DL_DISABMULTI_REQ, rc, DL_SYSERR, dlb);

	/* 
	 * Successful disablement.  Remove the link from the list.
	 */
	remove_link(dlb, old_link, link_ptr);
	return dlb_ok_ack(mp, DL_DISABMULTI_REQ, dlb);
}

/*
 * remove_link - remove and free a link in the list of multicast enabled
 *               addresses.
 * NOTE:  Currently, only dlb_disabmulti() calls remove_link().
 *        dlb_disabmulti() will *NOT* call remove_link() if the thread
 *        is running with interrupts disabled.  If the dlpi code is
 *        modified in the future so that any additional functions call
 *        remove_link(), the calling routines must check to make sure that
 *        the thread is running without interrupts disabled before calling
 *        remove_link().  This is all because remove_link() calls
 *        xmfree(). 
 */

void
remove_link(dlb, old_link, link_ptr)
	DLBP	dlb;
	struct address  *old_link;
	struct address  *link_ptr;
{
	if (old_link == dlb->dlb_addr_list)
		dlb->dlb_addr_list = old_link->next;
	link_ptr->next = old_link->next;
	old_link->next = NULL;
	xmfree(old_link, pinned_heap);
	dlb->dlb_stats.multicast_addrs--;
	all_stats.multicast_addrs--;
}

/*
 * cleanup_multi - disable multicast addresses on a per Streams basis if
 *                 the Stream is being closed.  The generic LAN device
 *		   drivers may keep reference counts of enablements, so we
 *                 need to clean up after ourselves to keep the reference
 *                 counts in synch.
 */

void
cleanup_multi(dlb)
	DLBP	dlb;
{
	struct address  *link, *head;

	/* 
	 * Make sure that this thread is not running in the interrupt
	 * environment.  dlpi must be running in the process environment in
	 * order to issue the ndd_ctl().  If dlpi is running in the
	 * interrupt environment, just return.
	 */
	if (getpid() == -1)
	 	return;

	/*
	 * March down the linked list of enabled addresses.  Issue a
	 * request to disable the address, then free the link.  Don't
	 * wait around for the return code.  If it works, great.  If it fails,
	 * well, we tried.
	 */
	head = dlb->dlb_addr_list;
	while (head) {
		(void) dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd, NDD_DISABLE_ADDRESS,
			head->address, sizeof(head->address));
		link = head->next;
		head->next = NULL;
		xmfree(head, pinned_heap);
		head = link;
		dlb->dlb_stats.multicast_addrs--;
		all_stats.multicast_addrs--;
	}
}

/*
 * dlb_promiscon - enable promiscuous mode on a per Streams basis
 *
 */

staticf MBLKP
dlb_promiscon(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	int    rc;
	ulong  level;
	ushort level_flag;

	/* If not attached then out of state. */
	if (!ATTACHED(dlb))
	 	return dlb_error_ack(mp, DL_PROMISCON_REQ, 0, DL_OUTSTATE, dlb);

	/* 
	 * Make sure that this thread is not running in the interrupt
	 * environment.  dlpi must be running in the process environment in
	 * order to issue the ndd_ctl().  If dlpi is running in the
	 * interrupt environment, return DL_SYSERR.
	 */
	if (getpid() == -1)
	 	return dlb_error_ack(mp, DL_PROMISCON_REQ, 0, DL_SYSERR, dlb);

	/*
	 * The calling application must be a trusted user (i.e., have root
	 * authority) to issue this primitive.  If the calling application
	 * does not have root authority, return a dlb_error_ack() with
	 * DL_ACCESS.
	 */
	if (!(dlb->dlb_promisc_flags & DL_PROMISC_PRIV))
		return dlb_error_ack(mp, DL_PROMISCON_REQ, 0, DL_ACCESS, dlb);

	/* 
	 * Look at the requested SAP level.  The ndd_ctl() command parameter
	 * is dictated by the level specified in the dl_promiscuous_req_t
	 * structure.
	 */
	level = ((dl_promiscon_req_t *)(mp->b_rptr))->dl_level;
	level_flag = (ushort)level;

	switch (level) {
		case DL_PROMISC_PHYS:
		case DL_PROMISC_SAP:
			/* If already enabled, do nothing. */
			if (dlb->dlb_promisc_flags & level_flag)
				break;
			/*
	 		 * The 'arg' and 'len' parameters of ndd_ctl() are
			 * not used for NDD_PROMISCUOUS_ON.
	 		 */
			if (rc = dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd, 
					NDD_PROMISCUOUS_ON, (caddr_t)NULL, 0))
				break;

			/*
			 * This level is handled by the network demuxer.
			 * Increment the global stats here because it is here
			 * where we know if we have successfully turned on this
			 * level of promiscuity.
			 */
			rc = mk_tapuser(dlb, mp);
			if (!rc) {
				if (dlb->dlb_promisc_flags & DL_PROMISC_PHYS)
					all_stats.promisc_phys++;
				else
					all_stats.promisc_sap++;
			}
			break;

		case DL_PROMISC_MULTI:
			/* 
			 * Reset level_flag because DL_PROMISC_MULTI is not set
			 * up for bitmask checking.  If promiscuous mode is
			 * already enabled, do nothing. 
			 */
			level_flag = DL_PROMISC_MULTI_FLAG;
			if (dlb->dlb_promisc_flags & level_flag)
				break;
			/*
	 		 * The 'arg' and 'len' parameters of ndd_ctl() are
			 * not used for NDD_ENABLE_MULTICAST.
			 * Increment the global stats here because it is here
			 * where we know if we have successfully turned on this
			 * level of promiscuity.
	 		 */
			if (!(rc = dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd,
				NDD_ENABLE_MULTICAST, (caddr_t)NULL, 0)))
				all_stats.promisc_multi++;
			break;

		default:
	 		return dlb_error_ack(mp, DL_PROMISCON_REQ, 0,
				     DL_UNSUPPORTED, dlb);
	}

	if (rc) 
		return dlb_error_ack(mp, DL_PROMISCON_REQ, rc, DL_SYSERR, dlb);

	dlb->dlb_promisc_flags |= level_flag;
	if (level_flag & DL_PROMISC_PHYS)
		dlb->dlb_stats.promisc_phys = PROMISCUOUS_ON;
	if (level_flag & DL_PROMISC_SAP)
		dlb->dlb_stats.promisc_sap = PROMISCUOUS_ON;
	if (level_flag & DL_PROMISC_MULTI_FLAG)
		dlb->dlb_stats.promisc_multi = PROMISCUOUS_ON;

	return dlb_ok_ack(mp, DL_PROMISCON_REQ, dlb);
}

/*
 * dlb_promiscoff - disable promiscuous mode on a per Streams basis
 *
 */

staticf MBLKP
dlb_promiscoff(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	int    rc;
	ulong  level;
	ushort level_flag;
	struct	ns_8022	dl;

	/* If not attached then out of state error. */
	if (!ATTACHED(dlb))
	 	return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0, DL_OUTSTATE,
			dlb);

	/*
	 * Make sure that this thread is not running in the interrupt
	 * environment.  dlpi must be running in the process environment in
	 * order to issue the ndd_ctl().  If dlpi is running in the
	 * interrupt environment, return DL_SYSERR.
	 */
	if (getpid() == -1)
	 	return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0, DL_SYSERR, dlb);

	/*
	 * The calling application must be a trusted user (i.e., have root
	 * authority) to issue this primitive.  If the calling application
	 * does not have root authority, return a dlb_error_ack() with
	 * DL_ACCESS. 
	 */
	if (!(dlb->dlb_promisc_flags & DL_PROMISC_PRIV))
		return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0, DL_ACCESS, dlb);

	/* 
	 * Look at the requested SAP level.  This level determines the degree 
	 * of promiscuity requested by the user.  The ndd_ctl() command
	 * parameter is dictated by the level specified in the 
	 * dl_promiscoff_req_t structure.  The ndd_ctl() 'arg' and 'len'
	 * parameters are not used for NDD_PROMISCUOUS_OFF and
	 * NDD_DISABLE_MULTICAST.
	 */
	level = ((dl_promiscoff_req_t *)(mp->b_rptr))->dl_level;
	level_flag = level;

	switch (level) {
		case DL_PROMISC_PHYS:
		case DL_PROMISC_SAP:
			/* If DL_PROMISC_PHYS is not turned on, error ack. */
			if (!(dlb->dlb_promisc_flags & level_flag))
				return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0,
					DL_NOTENAB, dlb);

			if (rc = dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd,
				  	NDD_PROMISCUOUS_OFF, (caddr_t)NULL, 0)) 
				  break;

			/*
			 * This level of promiscuity is handled by the network
			 * demuxer.  ns_del_filter() ignores dsap, orgcode,
			 * ethertype.  EINVAL is returned for reasons other
			 * than DL_NOTENAB.
			 */
			bzero(&dl, sizeof(dl));
			dl.filtertype = NS_TAP;
			rc = ns_del_filter(dlb->dlb_ndd, &dl,sizeof(dl));

			/*
			 * Decrement the global stats here because it is here
			 * where we know if we have successfully turned off
			 * this level of promiscuity.
			 */
			if (!rc) {
				if (dlb->dlb_promisc_flags & DL_PROMISC_PHYS)
					all_stats.promisc_phys--;
				else
					all_stats.promisc_sap--;
			}
			break;

		case DL_PROMISC_MULTI:
			/* 
			 * Reset level_flag because the value of
			 * DL_PROMISC_MULTI is not compatible with bitmasking.
			 * (See the value defined in dlpi.h.)  A local value,
			 * DL_PROMISC_MULTI_FLAG, is defined for bitmasking.
			 */
			level_flag = DL_PROMISC_MULTI_FLAG;

			/* If DL_PROMISC_MULTI is not turned on, error ack. */
			if (!(dlb->dlb_promisc_flags & DL_PROMISC_MULTI_FLAG))
				return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0,
					DL_NOTENAB, dlb);

			/*
			 * Decrement the global stats here because it is here
			 * where we know if we have successfully turned off
			 * this level of promiscuity.
			 */
			if (!(rc = dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd,
				  NDD_DISABLE_MULTICAST, (caddr_t)NULL, 0)))
				all_stats.promisc_multi--;
			break;

		default:
	 		return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0,
				     DL_UNSUPPORTED, dlb);
	}

	if (rc) {
		if (rc == EINVAL)
			return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0, 
						DL_NOTENAB, dlb);
		else
			return dlb_error_ack(mp, DL_PROMISCOFF_REQ, rc, 
						DL_SYSERR, dlb);
	}

	dlb->dlb_promisc_flags &= ~level_flag;
	if (level_flag & DL_PROMISC_PHYS)
		dlb->dlb_stats.promisc_phys = PROMISCUOUS_OFF;
	if (level_flag & DL_PROMISC_SAP)
		dlb->dlb_stats.promisc_sap = PROMISCUOUS_OFF;
	if (level_flag & DL_PROMISC_MULTI_FLAG)
		dlb->dlb_stats.promisc_multi = PROMISCUOUS_OFF;

	return dlb_ok_ack(mp, DL_PROMISCOFF_REQ, dlb);
}

/*
 * cleanup_promisc- disable promiscuous mode on a per Streams basis if
 *                  the Stream is being closed.  The generic LAN device
 *		    drivers keep reference counts of enablements, so we
 *                  need to clean up after ourselves to keep the reference
 *                  counts in synch.
 */

void
cleanup_promisc(dlb)
	DLBP	dlb;
{
	struct ns_8022  dl;
	
	/* Make sure that this thread is not running in the interrupt
	 * environment.  dlpi must be running in the process environment in
	 * order to issue the ndd_ctl().  If dlpi is running in the
	 * interrupt environment, just return.
	 */
	if (getpid() == -1)
	 	return;

	/* 
	 * For each bit set in the dlb_promisc_flags variable, turn off
	 * promiscuous mode.  Don't wait around for return codes.  If it
	 * works, great.  If it fails, well, we tried.
	 */
	if ((dlb->dlb_promisc_flags & DL_PROMISC_PHYS) || 
			(dlb->dlb_promisc_flags & DL_PROMISC_SAP)) {
		(void) dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd, NDD_PROMISCUOUS_OFF,
				(caddr_t)NULL, 0);
		bzero(&dl, sizeof(dl));
		dl.filtertype = NS_TAP;
		(void) ns_del_filter(dlb->dlb_ndd, &dl, sizeof(dl));

		if (dlb->dlb_promisc_flags & DL_PROMISC_PHYS) {
			dlb->dlb_promisc_flags &= ~DL_PROMISC_PHYS;
			dlb->dlb_stats.promisc_phys = PROMISCUOUS_OFF;
			all_stats.promisc_phys--;
		} else {
			dlb->dlb_promisc_flags &= ~DL_PROMISC_SAP;
			dlb->dlb_stats.promisc_sap = PROMISCUOUS_OFF;
			all_stats.promisc_sap--;
		}
	}
	if (dlb->dlb_promisc_flags & DL_PROMISC_MULTI_FLAG) {
		(void) dlb->dlb_ndd->ndd_ctl(dlb->dlb_ndd,
			NDD_DISABLE_MULTICAST, (caddr_t)NULL, 0);
		dlb->dlb_promisc_flags &= ~DL_PROMISC_MULTI_FLAG;
		dlb->dlb_stats.promisc_multi = PROMISCUOUS_OFF;
		all_stats.promisc_multi--;
	}
}

/*
 * mk_tapuser - Set up a tap user for the Network Demuxer to get
 *              no packet filtering.
 */

int
mk_tapuser(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	struct ns_8022  dl;
	struct ns_user  ns_user;
	int  err;

	/* 
	 * CDLI ignores dsap, orgcode, and ethertype for filtertype = NS_TAP.
	 */
	bzero(&dl, sizeof(dl));
	dl.filtertype = NS_TAP;

	ns_user.isr = (int)dlb_tap_intr;
	ns_user.isr_data = (caddr_t)dlb;
	ns_user.protoq = nilp(struct ifqueue);
	ns_user.netisr = NULL;
	ns_user.ifp = nilp(struct ifnet);
	ns_user.pkt_format = dlb->dlb_pkt_format;

	return ns_add_filter(dlb->dlb_ndd, &dl, sizeof(dl), &ns_user);
}

/*
 * dlb_phys_addr -  returns the current value of the physical
 *		    address associated with the Stream.
 */
staticf MBLKP
dlb_phys_addr(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_phys_addr_ack_t	*dlpaa;
	mblk_t			*mp1;

	/* If not attached then out of state error. */
	if (!ATTACHED(dlb))
	 	return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, 0, DL_OUTSTATE, dlb);

	/* 
	 * Look at the requested address type.  DLPI *ONLY* supports
	 * DL_CURR_PHYS_ADDR.
	 */
	if ((((dl_phys_addr_req_t *)(mp->b_rptr))->dl_addr_type) != 
			DL_CURR_PHYS_ADDR)
		return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, 0, DL_UNSUPPORTED, 
					dlb);

	/*
	 * Make sure that the ndd structure was filled in by Network
	 * Services during ns_alloc() execution.
	 */
	if (!dlb->dlb_ndd->ndd_addrlen)
	 	return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, 0, DL_SYSERR, dlb);
	
	/*
	 * Success!  Respond with a DL_PHYS_ADDR_ACK.
	 */
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) <
			(DL_PHYS_ADDR_ACK_SIZE + dlb->dlb_ndd->ndd_addrlen)) {
		mp1 = allocb(DL_PHYS_ADDR_ACK_SIZE + dlb->dlb_ndd->ndd_addrlen,
			BPRI_HI);
		if (!mp1) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
	 		return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, ENOMEM, 
						DL_SYSERR, dlb);
		}
		freemsg(mp);
		mp = mp1;
	}
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + DL_PHYS_ADDR_ACK_SIZE +
		dlb->dlb_ndd->ndd_addrlen;
	mp->b_datap->db_type = M_PCPROTO;
	dlpaa = (dl_phys_addr_ack_t *)mp->b_rptr;
	dlpaa->dl_primitive = DL_PHYS_ADDR_ACK;
	dlpaa->dl_addr_length = dlb->dlb_ndd->ndd_addrlen;
	dlpaa->dl_addr_offset = DL_PHYS_ADDR_ACK_SIZE;
	bcopy(dlb->dlb_ndd->ndd_physaddr, mp->b_rptr + DL_PHYS_ADDR_ACK_SIZE,
		dlb->dlb_ndd->ndd_addrlen);
	return mp;
}

/*
 * dlb_get_statistics - returns the collected statistics on both the global
 *                      level and on a per Stream basis.
 */
staticf MBLKP
dlb_get_statistics(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_get_statistics_ack_t  *dlgsa;
	mblk_t			 *mp1;

	/*
	 * Always success!  Respond with a DL_GET_STATISTICS_ACK.
	 */
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) <
		(DL_GET_STATISTICS_ACK_SIZE + 2 * sizeof(struct statistics))) {
		mp1 = allocb(DL_GET_STATISTICS_ACK_SIZE + 
			2 * sizeof(struct statistics), BPRI_HI);
		if (!mp1) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
	 		return dlb_error_ack(mp, DL_GET_STATISTICS_REQ, ENOMEM, 
						DL_SYSERR, dlb);
		}
		freemsg(mp);
		mp = mp1;
	}

	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + DL_GET_STATISTICS_ACK_SIZE +
		2*sizeof(struct statistics);
	mp->b_datap->db_type = M_PCPROTO;
	dlgsa = (dl_get_statistics_ack_t *)mp->b_rptr;
	dlgsa->dl_primitive = DL_GET_STATISTICS_ACK;
	dlgsa->dl_stat_length = 2*sizeof(struct statistics);
	dlgsa->dl_stat_offset = DL_GET_STATISTICS_ACK_SIZE;
	bcopy(&dlb->dlb_stats, (char *)&dlgsa[1], sizeof(struct statistics));
	bcopy(&all_stats, mp->b_rptr + dlgsa->dl_stat_offset +
		sizeof(struct statistics), sizeof(struct statistics));
	return mp;
}

/*
 * dlb_bind_sap - bind sap to interface
 *
 * returns 0 on success, < 0 dlpi error, > 0 system error
 */

staticf int
dlb_bind_sap(dlb, sap)
	DLBP dlb;
	ulong sap;
{
	ushort	type = GETTYPE(sap);
	uchar	ssap = GETSSAP(sap);
	uchar	sapbuf[6];
	int	saplen = 0;
	int	err;
	struct	ns_8022	dl;
	struct	ns_user	ns_user;
	int	mactype = dlb->dlb_ndd->ndd_type;

	/*
	 * validate types according to interface
	 */

	bzero(&dl, sizeof(dl));
	dl.filtertype = NS_8022_LLC_DSAP;

	/* XXXX - NDD is now common for ethernet and 802.3, use the
	 * 	interface name to differentiate.
	 */
	if (!strcmp(dlb->dlb_nddname, "en")) {
		if (type < 0x600)
			return -DL_BADADDR;
		saplen = 2;
		*(ushort *)sapbuf = type;
		dl.filtertype = NS_ETHERTYPE;
		dl.ethertype = type;
	} else {
		saplen = 1;
		sapbuf[0] = type = ssap;
		dl.dsap = type;
		if (sap < 0x100) {
			ssap = sap;
			sapbuf[0] = type = ssap;
			dl.dsap = ssap;
		} else if (ssap == SSAP_INET) { /* old style snap bind */
			type = GETTYPE(sap);
			if ((mactype == NDD_ISO88023 && type == _802_3_TYPE_AP)
				|| (mactype ==NDD_ISO88025 && 
					type ==_802_5_TYPE_AP)) {
				sapbuf[1] = dl.orgcode[0] = 0x08;
				sapbuf[2] = dl.orgcode[1] = 0x00;
				sapbuf[3] = dl.orgcode[2] = 0x07;
			} else {
				sapbuf[1] = dl.orgcode[0] = 0;
				sapbuf[2] = dl.orgcode[1] = 0;
				sapbuf[3] = dl.orgcode[2] = 0;
			}
			*((ushort *)&sapbuf[4]) = type;
			saplen += 5;
			BCOPY(&sapbuf[1], dlb->dlb_llc.prot_id, 5);
			dl.filtertype = NS_8022_LLC_DSAP_SNAP;
			dl.dsap = ssap;
			dl.ethertype = type;

			/* 
			   Default pkt_format is NS_PROTO, but for SNAP it 
			   needs to be NS_PROTO_SNAP
			*/

			if ((dlb->dlb_pkt_format & NS_PROTO) == NS_PROTO) {
				dlb->dlb_pkt_format &= ~NS_PROTO;
				dlb->dlb_pkt_format |= NS_PROTO_SNAP;
			}
		}
		dlb->dlb_llc.ssap = ssap;
		dlb->dlb_llc.dsap = ssap;
		dlb->dlb_llc.ctrl = CTRL_UI;
	}

	/* dlpi implementation restriction: type cannot be zero */
	if (type == 0)
		return -DL_BADADDR;

	/* bind only if have complete information */
	if (type != SSAP_INET) {
		if (err = dlb_add_type(dlb, type, &dlb->dlb_llc, 0, &dl))
			return err;
	}

	bcopy(sapbuf, dlb->dlb_addr + dlb->dlb_ndd->ndd_addrlen, saplen);
	dlb->dlb_addr_length = dlb->dlb_ndd->ndd_addrlen + saplen;
	dlb->dlb_saplen = saplen;
	dlb->dlb_sap = sap;

	return 0;
}

/*
 * dlb_unbind_sap - unbind all SAPs from interface
 */

staticf int
dlb_unbind_sap(dlb)
	DLBP dlb;
{
	int err;
	struct	ns_8022	dl;
	bind_t *bind, *next;

	/* if bind->type == 0xAA, haven't bound to interface */
	bind = dlb->dlb_bound;
	while (bind) {
		if (bind->type != SSAP_INET) {

			if (!strcmp(dlb->dlb_nddname, "en")) {
				dl.filtertype = NS_ETHERTYPE;
				dl.ethertype = bind->type;
			} else {
				dl.filtertype = NS_8022_LLC_DSAP;
				dl.dsap = bind->type;
			}
			/* if we have snap */
			if (bind->llc.type) {
				dl.filtertype = NS_8022_LLC_DSAP_SNAP;
				dl.dsap = bind->llc.dsap;
				bcopy(bind->llc.prot_id, dl.orgcode,
					sizeof(dl.orgcode)); 
				dl.ethertype = bind->type;
			}

			/* 
			 * Unbind all saps, but save off the bind->dlbnext
			 * pointer first since this bind link may get
			 * removed from the list.  If dlb_del_type returns
			 * a 0, then no bind_t structure was freed.  If
			 * dlb_del_type returns a 1, then a bind_t structure
			 * was freed, and we need to use the link saved in next.
			 */
			next = bind->dlbnext;
			err = dlb_del_type(dlb, bind->type, bind->class, dl);
			if (err < 0)
				return err;
			if (err)
				bind = next;
			else
				bind = bind->dlbnext;

			if (err = ns_del_filter(dlb->dlb_ndd, &dl, sizeof(dl)))
				return err;
		}
		else
			bind = bind->dlbnext;
	}

	dlb->dlb_addr_length = dlb->dlb_ndd->ndd_addrlen;
	dlb->dlb_saplen = 0;
	dlb->dlb_sap = 0;
	return 0;
}

/*
 * getbind - allocate a bind record
 */

static bind_t *
getbind(dlb)
DLB *dlb;
{
	bind_t *bind;

	if (bind = (bind_t *)xmalloc(sizeof(bind_t), 3, pinned_heap)) {
		bzero(bind, sizeof(bind_t));
		bind->dlb = dlb;
		return bind;
	}
	else {
		dlb->dlb_stats.no_bufs++;
		all_stats.no_bufs++;
		return NULL;
	}
}

/*
 * freebind - remove a bind record from the linked list and free it.
 */

static void
freebind(dlb, bind, prev_dlb_bind)
DLBP    dlb;
bind_t *bind;		/* bind link to free */
bind_t *prev_dlb_bind;	/* link whose dlbnext pointer points to bind */
{
	bind_t  *prev;

	/* Sanity check. */
	if (!bind)
		return;

	/* 
	 * If bind is the first link in bindlist, reset bindlist.  If it is not
	 * the first link, march down the next pointers in the linked list 
	 * pointed to by bindlist until we reach bind, keeping track of the
	 * link just visited.  There are two previous links we have to keep
	 * track of:  the link whose next pointer points to bind and the
	 * link whose dlbnext pointer points to bind.  The link whose dlbnext
	 * pointer points to bind is passed in.  We have to figure out which
	 * link's next pointer points to bind.  Reset it when found.
	 */
	if (bind == bindlist)
		bindlist = bind->next;
	else {
		prev=bindlist;
		while (prev) {
			if (prev->next == bind) {
				prev->next = bind->next;
				break;
			}
			prev = prev->next;
		}
	}

	/*
	 * Reset the prev_dlb_bind pointer.  If this is the first bind
	 * in the dlbnext list, then we have to reset the dlb->dlb_bound
	 * pointer to bind's dlbnext pointer.
	 */
	if (prev_dlb_bind)
		prev_dlb_bind->dlbnext = bind->dlbnext;
	else
		dlb->dlb_bound = bind->dlbnext;	

	bind->next = NULL;
	bind->dlbnext = NULL;
	bind->dlb = NULL;
	xmfree(bind, pinned_heap);
}


/*
 * dlb_add_type - register type
 */

staticf int
dlb_add_type(dlb, type, llc, class, dl)
	DLBP   dlb;
	ushort type;
	struct ie2_llc_snaphdr *llc;
	int    class;		/* 0 = not a subsbind, 1 = subsbind */
	struct ns_8022	*dl;
{
	struct	ns_user	ns_user;
	bind_t **pp, *bind, *bp;
	int err;

	strlog(MODNUM,0,0,SL_TRACE,"dlb_add_type: dlb %x type %x class %d",
		dlb, type, class);

	/*
	 * ns_add_filter() will not allow the same type to be registered
	 * more than once.  ns_add_filter() will return an error, EEXIST, if
	 * the type is already registered. 
	 */
	ns_user.isr = (int)dlb_intr;
	ns_user.isr_data = (caddr_t)dlb;
	ns_user.protoq = nilp(struct ifqueue);
	ns_user.netisr = NULL;
	ns_user.ifp = nilp(struct ifnet);
	ns_user.pkt_format = dlb->dlb_pkt_format;

	if (err = ns_add_filter(dlb->dlb_ndd, dl, sizeof(struct ns_8022),
			&ns_user))
		return err;

	/* allocate a bind record */
	if (!(bind = getbind(dlb)))
		return ENOMEM;

	bind->llc = *llc;
	bind->type = type;
	bind->class = (u_short)class;

	/*
	 * New binds are linked on the tail of the bind->next list
	 * and the tail of the bind->dlbnext list, so unbinds can
	 * intelligently update info_ack addresses.
	 */
	BINDLOCK;
	if (!bindlist)
		bindlist = bind;
	else {
		for (bp=bindlist; bp->next; bp=bp->next)
			;
		bp->next = bind;
	}
	for (pp = &dlb->dlb_bound; *pp; pp = &(*pp)->dlbnext)
		;
	*pp = bind;
	BINDUNLOCK;

	/* if *any* stream binds a netware sap, entire stream is netware */
	if (isnetware(dlb, type))
		dlb->dlb_netware = 1;

	return 0;
}

/*
 * dlb_del_type - unregister bind type
 * Returns 0 if no bind_t is freed.  Returns 1 if a bind_t is freed.
 */

staticf int
dlb_del_type(dlb, type, class)
	DLBP dlb;
	ushort type;
	int class;		/* 0 = not a subsbind, 1 = subsbind */
{
	bind_t **pp, *bind, *prev_dlb_bind;
	int x, rc = 0;			/* rc acts as sanity check */

	/*
	 * Locate and remove the bind record for this type.
	 */
	prev_dlb_bind = NULL;
	for (pp = &dlb->dlb_bound; *pp; pp = &(*pp)->dlbnext) {
		if (type == (*pp)->type) {
			bind = *pp;
			/* catch attempts to unbind via incorrect primitive */
			if (class != bind->class) {
				return -DL_BADADDR;
			}
			BINDLOCK;
			freebind(dlb, bind, prev_dlb_bind);
			BINDUNLOCK;
			rc = 1;  /* We freed something. */
			break;
		}
		prev_dlb_bind = *pp;
	}
	
	return rc;
}


/*
 * dlb_error - generic error return for both DLPI and system errors
 */

staticf MBLKP
dlb_error(mp, prim, err, dlb)
	MBLKP	mp;
	ulong	prim;
	DLBP    dlb;
	int	err;
{
	int dlerr = DL_SYSERR;

	if (err < 0) {
		dlerr = -err;
		err = 0;
	}
	return dlb_error_ack(mp, prim, err, dlerr, dlb);
}

/*
 * dlb_error_ack - create a DLPI error response message
 *
 * returns null if no mblk available, else formatted mblk
 */

staticf MBLKP
dlb_error_ack (mp, prim, unix_error, dl_error, dlb)
	MBLKP	mp;
	ulong	prim;
	int	unix_error;
	int	dl_error;
	DLBP	dlb;
{
	dl_error_ack_t	* dlea;

	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < sizeof(dl_error_ack_t)) {
		freemsg(mp);
		mp = allocb(sizeof(dl_error_ack_t), BPRI_HI);
		if (!mp) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
			return nil(MBLKP);
		}
	}
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(dl_error_ack_t);
	mp->b_datap->db_type = M_PCPROTO;
	dlea = (dl_error_ack_t *)mp->b_rptr;
	dlea->dl_primitive = DL_ERROR_ACK;
	dlea->dl_error_primitive = prim;
	dlea->dl_errno = dl_error;
	dlea->dl_unix_errno = unix_error;
	return mp;
}

/*
 * dlb_ok_ack - create a DLPI okay response message
 */

staticf MBLKP
dlb_ok_ack (mp, prim, dlb)
	MBLKP	mp;
	ulong	prim;
	DLBP	dlb;
{
	dl_ok_ack_t	* dloa;

	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < sizeof(dl_ok_ack_t)) {
		freemsg(mp);
		mp = allocb(sizeof(dl_ok_ack_t), BPRI_HI);
		if (!mp) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
			return nil(MBLKP);
		}
	}
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(dl_ok_ack_t);
	mp->b_datap->db_type = M_PCPROTO;
	dloa = (dl_ok_ack_t *)mp->b_rptr;
	dloa->dl_primitive = DL_OK_ACK;
	dloa->dl_correct_primitive = prim;
	return mp;
}

/*
 * dlb_open
 */

staticf int
dlb_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	DLBP	dlb;
	int	err;
	struct	dl_data *dldata;

	DLPILOCK;
	if (!(dldata = findndd(major(*devp)))) {
		DLPIUNLOCK;
		return ENXIO;
	}
	DLPIUNLOCK;

	/*
	 * No locks are used here since the dlb_g_head is protected
	 * via the streamhead mult_sqh.
	 */
	err = mi_open_comm(&dlb_g_head, sizeof(DLB),q,devp, flag, sflag, credp);

	if (err == 0) {
		dlb = (DLBP)q->q_ptr;
		bzero(dlb, sizeof(DLB));
		dlb->dlb_rq = q;		/* for intr routines */
		bcopy(dldata->ndd_name, dlb->dlb_nddname, FMNAMESZ+1);
		dlb->dlb_pkt_format = NS_PROTO | NS_HANDLE_NON_UI | 
					NS_HANDLE_HEADERS;
		if (!drv_priv(credp))
		    dlb->dlb_promisc_flags |= DL_PROMISC_PRIV;
	}
	return err;
}

/*
 * dlb_close - closes channel to dlpi
 */

staticf int
dlb_close (q)
	queue_t	* q;
{
	DLBP	dlb;
	int     err;

	dlb = (DLBP)q->q_ptr;

	(void)dlb_unbind(dlb, nil(MBLKP));
	(void)dlb_detach(dlb, nil(MBLKP));
	err = mi_close_comm(&dlb_g_head, q);
	return err;
}

/*
 * dlb_info - handle DLPI info requests
 */

staticf MBLKP
dlb_info (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_info_ack_t	* dlia;
	mblk_t *mp1;
	int len;

	/* variable length reply - alloc max size */
	len = sizeof(dl_info_ack_t) + MACSNAP_LEN * 2;
	if (mp->b_datap->db_lim - mp->b_datap->db_base < len) {
		if (!(mp1 = allocb(len, BPRI_HI))) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
			return dlb_error_ack(mp, DL_INFO_REQ, ENOMEM, DL_SYSERR,
				dlb);
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
	dlia = (dl_info_ack_t *)mp->b_rptr;

	/* constants for any state */
	dlia->dl_primitive = DL_INFO_ACK;
	dlia->dl_version = DL_VERSION_2;
	dlia->dl_service_mode = DL_CLDLS;
	dlia->dl_provider_style = DL_STYLE2;
	dlia->dl_min_sdu = 1;
	dlia->dl_max_sdu = 1;
	dlia->dl_mac_type = ndd_to_dlb_mactype(dlb);
	dlia->dl_current_state = DL_UNATTACHED;

	if (ATTACHED(dlb)) {
		dlia->dl_current_state = DL_UNBOUND;
		dlia->dl_max_sdu = dlb->dlb_ndd->ndd_mtu;
		dlia->dl_brdcst_addr_length = sizeof(broadcast);
		dlia->dl_brdcst_addr_offset = len;
		bcopy(broadcast, mp->b_rptr + len, sizeof(broadcast));
		len += dlia->dl_brdcst_addr_length;
	}

	if (BOUND(dlb)) {
		dlia->dl_current_state = DL_IDLE;
		dlia->dl_sap_length = -dlb->dlb_saplen;
		dlia->dl_addr_length = dlb->dlb_addr_length;
		dlia->dl_addr_offset = len;
		bcopy(dlb->dlb_addr, mp->b_rptr + len, dlia->dl_addr_length);
		len += dlia->dl_addr_length;
	}
	mp->b_wptr += len;
	return mp;
}

/*
 * dlb_intr - interrupt handler: receive incoming frames
 */

staticf int
dlb_intr (nddp, m, llhdr, isr)
	NDDP			nddp;
	MBUFP			m;
	caddr_t			llhdr;
	struct	isr_data_ext	*isr;
{
reg	MBLKP			first_mp, prev_mp, mp;
reg	struct ie2_llc_snaphdr	*llc;
reg	dl_unitdata_ind_t 	*udip;
	uchar			*da, *sa;
	DLBP			dlb;
	int			size = 0;
	bind_t  *bind;

	dlb = (DLBP)isr->isr_data;

	if ( !dlb ) {
		ASSERT(0);
		m_freem(m);
		return;
	}

	/* call the input address resolution routine */
	if (dlb->dlb_input)
		(*(dlb->dlb_input))(llhdr,m);

	/*
	 * If the DLPI user has enabled multicasting, DLPI must perform
	 * some address filtering since the CDLI demuxer will be not be doing
	 * all the filtering for these requests.  If the user has requested
	 * any level of promiscuous mode, service this mbuf.  If the user has
	 * only requested multicasting, then only service mbufs for addresses
	 * in the dlb linked address list.
	 */
	if (m->m_hdr.mh_flags & M_MCAST) {
		if ((dlb->dlb_promisc_flags & ~(DL_PROMISC_PHYS | 
				DL_PROMISC_SAP | DL_PROMISC_MULTI_FLAG)) &&
				dlb->dlb_addr_list) {
			struct address *link;
	 
			link = dlb->dlb_addr_list;
			while (link) {
				if (!bcmp(isr->dstp, link->address, 
						nddp->ndd_addrlen))
					break;
				link = link->next;
			}
			if (!link) {
				m_freem(m);
				return;
			}
		}
	}

	/*
	 * If the DLPI user has turned on the sap level of promiscuous mode,
	 * the demuxer will give us all the packets on wire, so some filtering
	 * is required. The user is only interested in the packets which 
	 * are destined to him for all saps, all multicast addresses he has
	 * registered, and all broadcast messages.  Otherwise free them.
	 */
	 /* XXXX */
	 if ((dlb->dlb_promisc_flags & DL_PROMISC_SAP) && isr->llclen) {
		/*
		 * The user's address is stored in dlb->dlb_ndd->ndd_physaddr.
		 * This is a sanity check.
		 */
		if (!dlb->dlb_ndd->ndd_addrlen) {
			m_freem(m);
			return;
		}
		
		/*
		 * Make sure that this packet is destined for this user.
		 */
		if (!bcmp(isr->dstp, dlb->dlb_ndd->ndd_physaddr, isr->dstlen))
			goto out;

		/* We want any multicast packets that were registered. */
		if (m->m_hdr.mh_flags & M_MCAST) {
			struct address *link;
	 
			link = dlb->dlb_addr_list;
			while (link) {
				if (!bcmp(isr->dstp, link->address, 
						nddp->ndd_addrlen))
					goto out;
				link = link->next;
			}
			if (!link) {
				m_freem(m);
				return;
			}
		}
		else {
			/* We want all broadcast packets. */
			if (m->m_hdr.mh_flags & M_BCAST)
				goto out;
		}

		/* This packet is not for us. */
		m_freem(m);
		return;
	}

out:
	dlb->dlb_stats.rx_pkts++;
	all_stats.rx_pkts++;

	if (!canput(dlb->dlb_rq)) {
		m_freem(m);
		dlb->dlb_stats.rx_discards++;
		all_stats.rx_discards++;
		return;
	}

	/* 
	   calculate the size for the DLPI DL_UNITDATA_IND header. 
	   
	   For NS_PROTO and NS_PROTO_SNAP, there won't be any MAC header or
	   LLC information in data, so copy that into the DLPI header.

	   For NS_INCLUDE_LLC and NS_INCLUDE_MAC, copy only destination and
	   source addresses into the DLPI header.
	 */

	if ((dlb->dlb_pkt_format & NS_PROTO) || 
			(dlb->dlb_pkt_format & NS_PROTO_SNAP)) 
		size = isr->dstlen + isr->srclen + isr->llclen + isr->seglen;
	else if ((dlb->dlb_pkt_format & NS_INCLUDE_MAC) ||
			(dlb->dlb_pkt_format & NS_INCLUDE_LLC)) 
		size = isr->dstlen + isr->srclen;
	else {
		m_freem(m);
		return;
	}
	if (!(first_mp = allocb((sizeof(dl_unitdata_ind_t) + size), BPRI_HI))) {
		dlb->dlb_stats.no_bufs++;
		all_stats.no_bufs++;
		dlb->dlb_stats.rx_discards++;
		all_stats.rx_discards++;
		m_freem(m);
		return;
	}
	first_mp->b_wptr += sizeof(dl_unitdata_ind_t) + size;
	first_mp->b_datap->db_type = M_PROTO;

	udip = (dl_unitdata_ind_t *)first_mp->b_rptr;
	udip->dl_primitive = DL_UNITDATA_IND;
	udip->dl_dest_addr_offset = sizeof(dl_unitdata_ind_t);
	udip->dl_src_addr_offset  = sizeof(dl_unitdata_ind_t) + isr->dstlen;

	da = first_mp->b_rptr + udip->dl_dest_addr_offset;
	sa = first_mp->b_rptr + udip->dl_src_addr_offset;

	/* For NS_INCLUDE_MAC and NS_INCLUDE_LLC, copy destination and source
	 * addresses in the DLPI header.
	 * For NS_PROTO and NS_PROTO_SNAP, copy the destination and source
	 * address, llc (SAP or SNAP) and source routing information.
	 */
	bcopy(isr->dstp, da, isr->dstlen);
	bcopy(isr->srcp, sa, isr->srclen);
	if ((dlb->dlb_pkt_format & NS_PROTO) || 
			(dlb->dlb_pkt_format & NS_PROTO_SNAP)) {
		bcopy(isr->llcp, &sa[isr->srclen], isr->llclen);
		if (isr->segp)
			bcopy(isr->segp, &sa[isr->srclen + isr->llclen], 
					isr->seglen);
	}

	/* XXXX - correct ? */
	udip->dl_group_address = (m->m_hdr.mh_flags & M_MCAST);
	udip->dl_dest_addr_length = isr->dstlen;
	udip->dl_src_addr_length = size - isr->dstlen;

	/*
	 * construct data portion from mbuf chain
	 */
	prev_mp = first_mp;
	do {
		mp = allocbi((int)m->m_len, BPRI_LO,
				(pfi_t)m_free, (char *)m, mtod(m, char *));
		if (!mp) {
			dlb->dlb_stats.no_bufs++;
			all_stats.no_bufs++;
			dlb->dlb_stats.rx_discards++;
			all_stats.rx_discards++;
			m_freem(m);
			freemsg(first_mp);
			return;
		}
		mp->b_wptr = mp->b_rptr + m->m_len;
		dlb->dlb_stats.rx_bytes += m->m_len;
		all_stats.rx_bytes += m->m_len;
		prev_mp->b_cont = mp;
		prev_mp = mp;
	} while (m = m->m_next);

	putq(dlb->dlb_rq, first_mp);
}

/*
 * dlb_tap_intr - copies the mbuf for the tap user case and passes control
 *                to  dlb_intr().
 */

staticf int
dlb_tap_intr (nddp, m, llhdr, isr)
	NDDP			nddp;
	MBUFP			m;
	caddr_t			llhdr;
	struct	isr_data_ext	*isr;
{
	MBUFP	mcopy;
	DLBP	dlb;

	dlb = (DLBP)isr->isr_data;
	/*
	 * Copy the mbuf since the tap user only has a pointer to it.
	 */
	if (!(mcopy = m_copym(m, 0, (int)m->m_len, M_DONTWAIT))) {
		dlb->dlb_stats.no_bufs++;
		all_stats.no_bufs++;
		dlb->dlb_stats.rx_discards++;
		all_stats.rx_discards++;
		return;
	}

	/*
	 * For the tap user, the demuxer can't give us the length of the LLC,
	 * so we just copy the bind time length.
	 */
	if (isr->llclen == 0)
		isr->llclen = dlb->dlb_saplen;

	dlb_intr (nddp, mcopy, llhdr, isr);
}

/*
 * dlb_rsrv - handle data from interrupt routine
 */

staticf void
dlb_rsrv (q)
	queue_t	*q;
{
	reg mblk_t *mp;

	/* flow control!!! */
	while (mp = getq(q)) {
		if (canput(q->q_next))
			putnext(q, mp);
		else {
			putbq(q, mp);
			return;
		}
	}
}

/* 
 * m_freeb - glue to allow m_free() to also free an mblk
 */

/* ARGSUSED */
staticf int
m_freeb(dat, len, mp)
	caddr_t dat;
	int len;
	MBLKP mp;
{
	freeb(mp);
	return 0;
}

static mblk_t *
dlb_uderror(mp, unix_error, error)
	mblk_t *mp;
	int unix_error;
	int error;
{
	dl_uderror_ind_t *dlui;

	dlui = (dl_uderror_ind_t *)mp->b_rptr;
	dlui->dl_primitive = DL_UDERROR_IND;
	dlui->dl_unix_errno = unix_error;
	dlui->dl_errno = error;
	return mp;
}

/*
 * dlb_pad - "free" the pad byte used for netware; see dlb_wput()
 */

static int
dlb_pad(b,s,a)
	caddr_t b;
	int s,a;
{
	return 0;
}

/*
 * dlb_wput - send data to the interface
 */

staticf int
dlb_wput (q, orig_mp)
	queue_t	* q;
	MBLKP	orig_mp;
{
	DLBP			dlb;
	dl_unitdata_req_t	* dlur;
	MBUFP			first_m, prev_m, m;
reg	MBLKP			mp1, lastmp, mp = orig_mp;
	int			len, pktlen;
	int			netware = 0, err = 0;
	int			dlen, daddrlen;
	unsigned char  		*daddr;
	struct iocblk 		*iocp;
        int			tx_bytes = 0;
	struct output_bundle	obundle;
	struct ie2_llc_snaphdr	*llc;
#define NWBUG() {	\
	if (isnetware(dlb,dlb->dlb_llc.dsap) && (pktlen & 1)) {			\
		if (lastmp->b_wptr == lastmp->b_datap->db_lim) {	\
			static char pad = 0;				\
			if (prev_m->m_next = m_clattach(&pad,dlb_pad, 1, 0 ,M_DONTWAIT)) \
				m->m_pkthdr.len++;			\
		} else {						\
			prev_m->m_len++;				\
			m->m_pkthdr.len++;				\
		}							\
	}								\
}

	dlb = (DLBP)q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_PROTO:
	case M_PCPROTO:
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHALL);
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 0;
		}
		freemsg(mp);
		return 0;
	case M_IOCTL:
		/* user will pass the pkt_format through I_STR ioctl */
		iocp = (struct iocblk *)mp->b_rptr;
		if (!mp->b_cont || mp->b_cont->b_cont)
			goto iocnak;
		switch (iocp->ioc_cmd) {
		case DL_PKT_FORMAT:
			dlb->dlb_pkt_format = *(ulong *)mp->b_cont->b_rptr;

			/* if already bound, then can't change pkt_format */
			if (BOUND(dlb))
				goto iocnak;

			/* check for valid pkt_format */
			if (dlb->dlb_pkt_format & ~(NS_PROTO | NS_PROTO_SNAP |
			    NS_INCLUDE_LLC | NS_INCLUDE_MAC | 
			    NS_HANDLE_NON_UI | NS_HANDLE_HEADERS))
				goto iocnak;

			if (dlb->dlb_pkt_format == NS_HANDLE_NON_UI)
				dlb->dlb_pkt_format = NS_PROTO |
					NS_HANDLE_HEADERS | NS_HANDLE_NON_UI;

			/* always add this internal pkt_format for demuxer */
			dlb->dlb_pkt_format |= NS_HANDLE_HEADERS;
			break;
		case DL_OUTPUT_RESOLVE:
			dlb->dlb_output = (int (*) ())(mp->b_cont->b_rptr);
			break;
		case DL_INPUT_RESOLVE:
			dlb->dlb_input = (void (*) ())(mp->b_cont->b_rptr);
			break;
		default:
iocnak:
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			qreply(q,mp);
			return 0;
		}
		iocp->ioc_count = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return 0;
	default:
		/* drop it on the floor */
		freemsg(mp);
		dlb->dlb_stats.unknown_msgs++;
		all_stats.unknown_msgs++;
		return 0;
	}

	dlur = (dl_unitdata_req_t *)mp->b_rptr;

	if (dlur->dl_primitive != DL_UNITDATA_REQ)
		return dlb_prim(q, dlb, mp);

	/*
	 * unitdata_req verification:
	 *	- must be bound
	 *	- priorities not supported
	 *	- must contain at least one byte of data
	 */
	if (!BOUND(dlb)) {
		qreply(q, dlb_uderror(mp, 0, DL_OUTSTATE));
		return 0;
	}
	if (dlur->dl_priority.dl_min) {
		qreply(q, dlb_uderror(mp, 0, DL_UNSUPPORTED));
		return 0;
	}
	if ((dlen = msgdsize(mp)) < 1 || dlen > dlb->dlb_ndd->ndd_mtu) {
		qreply(q, dlb_uderror(mp, 0, DL_BADDATA));
		return 0;
	}

	/*
	 * verify address length
	 */
	daddrlen = dlur->dl_dest_addr_length;
	daddr = mp->b_rptr + dlur->dl_dest_addr_offset;
	if (daddr + daddrlen > mp->b_wptr) {
		qreply(q, dlb_uderror(mp, 0, DL_BADADDR));
		return 0;
	}

	/* XXXX - Is this ok for NS_INCLUDE_MAC ? in that case M_DATA type
		message contains the Mbuf header plus the data !
	*/
	/* link mblks into mbufs */
	prev_m = nil(MBUFP);
	first_m = nil(MBUFP);
	pktlen = 0;
	for ( mp = mp->b_cont; mp; mp = mp1) {
		len = mp->b_wptr - mp->b_rptr;
		/* no zero length mbufs, please */
		if ( len == 0 ) {
			mp1 = mp->b_cont;
			freeb(mp);
			dlb->dlb_stats.tx_discards++;
			all_stats.tx_discards++;
			continue;
		}
		pktlen += len;
		m = m_clattach((caddr_t)mp->b_rptr, m_freeb, len, (int)mp, M_DONTWAIT);
		/*
		 * if cannot create mbuf cluster,
		 * forget the whole thing
		 */
		if ( !m ) {
			if ( first_m )
				m_freem(first_m);
			freemsg(mp);
			freeb(orig_mp);
			dlb->dlb_stats.tx_discards++;
			all_stats.tx_discards++;
			return 0;
		}
		if ( first_m )
			prev_m->m_next = m;
		else
			first_m = m;
		prev_m = m;
		m->m_len = len;
		tx_bytes += len;
		dlb->dlb_stats.tx_pkts++;
		all_stats.tx_pkts++;
		mp1 = mp->b_cont;
		lastmp = mp;
	}
	/* must have been a zero length message - drop it */
	if ( !first_m ) {
		freeb(orig_mp);
		dlb->dlb_stats.tx_discards++;
		all_stats.tx_discards++;
		return 0;
	}

	/* put a packet header at the front of the chain */
	MGETHDR(m, M_DONTWAIT, MT_HEADER);
	if (!m) {
		freeb(orig_mp);
		m_freem(first_m);
		dlb->dlb_stats.tx_discards++;
		all_stats.tx_discards++;
		return 0;
	}
	m->m_next = first_m;
	m->m_pkthdr.len = pktlen;
	m->m_data += MHLEN;
	m->m_len = 0;

	bzero(&obundle, sizeof(struct output_bundle));
	if ((dlb->dlb_pkt_format & NS_PROTO) || 
		(dlb->dlb_pkt_format & NS_PROTO_SNAP)) {

		/* assign the destination address into key_to_find,
		 * copy the bind time llc and pkt_format into helpers
		 * if the user has provided different dsap/type than bind time,
		 * then pass that in helpers.
		 * Source Routing is not allowed in this case !!!
		 */
		len = dlb->dlb_ndd->ndd_addrlen;
		obundle.key_to_find = (caddr_t)daddr;
		obundle.helpers.pkt_format = dlb->dlb_pkt_format;
		netware = dlb->dlb_netware;
		if (!strcmp(dlb->dlb_nddname, "en")) {
			obundle.helpers.ethertype = dlb->dlb_type;
			if (daddrlen > len && daddr[len]) { 
				ushort	type;
				type = *(ushort *)(daddr + len);
				obundle.helpers.ethertype = type;
				netware = isnetware(dlb, type);
			}
		} else {
			obundle.helpers.ethertype = 0;
			llc = &obundle.helpers.sapu.llcsnap;
			*llc = dlb->dlb_llc;

			/* if DLPI header contains more than addrlen, then
			 * update the llc. If it is one byte, it's a DSAP
			 * else it is SNAP. 
			 */

			if (daddrlen > len && daddr[len]) {
				llc->dsap = daddr[len];
				netware = isnetware(dlb, llc->dsap);
				if (daddrlen > (len + sizeof(uchar)))
					BCOPY(daddr + (len + sizeof(uchar)), 
						llc->prot_id, SNAP_LEN);
			}
		}

		/*
		 * Due to a bug in Mess-DOS Novell Ethernet/802.3 drivers,
		 * packets with an odd number of bytes are discarded!
		 * Therefore, we pad outbound Novell packets (only)
		 * to avoid this, if we can; else just send it unpadded.
		 */
		if (netware && (pktlen & 1)) {
			if (lastmp->b_wptr == lastmp->b_datap->db_lim) {
				static char pad = 0;
				if (prev_m->m_next = m_clattach(&pad,dlb_pad, 1, 0 ,M_DONTWAIT))
					m->m_pkthdr.len++;
			} else {
				prev_m->m_len++;
				m->m_pkthdr.len++;
			}
		}

		/* XXX - always call the output resolve routine */
		err = (*(dlb->dlb_output))(&obundle, m, dlb->dlb_ndd); 
		m = 0;

	} else if (dlb->dlb_pkt_format & NS_INCLUDE_LLC) {
		/* In this case the user will fill the LLC in data portion,
		 * assign the address into key_to_find and pkt_format
		 */
		obundle.key_to_find = daddr;
		obundle.helpers.pkt_format = dlb->dlb_pkt_format;
		NWBUG()
		/* XXX - always call the output resolve routine */
		err = (*(dlb->dlb_output))(&obundle, m, dlb->dlb_ndd); 
		m = 0;

	} else if (dlb->dlb_pkt_format & NS_INCLUDE_MAC) {
		/* call the ndd_output(), because the user has included full
		 * (MAC + LLC) header in data portion
		 */
		NWBUG() /* XXX - Does this make sense? Does not happen in NW4AIX code. */
		err = (*(dlb->dlb_ndd->ndd_output))(dlb->dlb_ndd, m);
	} else 
		err = EINVAL;	/* XXXX */

	if (err != 0) {
		if (m)
			m_freem(m);
		strlog(MODNUM, 0, 0, SL_ERROR,"ndd_output err 0x%x\n", err);
	} else {
		dlb->dlb_stats.tx_pkts++;
		all_stats.tx_pkts++;
		dlb->dlb_stats.tx_bytes += tx_bytes;
		all_stats.tx_bytes += tx_bytes;
	}
	freeb(orig_mp);
	return 0;
}

dlb_prim(q, dlb, mp)
	queue_t *q;
	DLBP dlb;
	mblk_t *mp;
{
	dl_unitdata_req_t *dlur = (dl_unitdata_req_t *)mp->b_rptr;

	switch (dlur->dl_primitive) {
	case DL_ATTACH_REQ:
		mp = dlb_attach(dlb, mp);
		break;
	case DL_DETACH_REQ:
		mp = dlb_detach(dlb, mp);
		break;
	case DL_BIND_REQ:
		mp = dlb_bind(dlb, mp);
		break;
	case DL_SUBS_BIND_REQ:
		mp = dlb_subs_bind(dlb, mp);
		break;
	case DL_SUBS_UNBIND_REQ:
		mp = dlb_subs_unbind(dlb, mp);
		break;
	case DL_UNBIND_REQ:
		mp = dlb_unbind(dlb, mp);
		break;
	case DL_INFO_REQ:
		mp = dlb_info(dlb, mp);
		break;
	case DL_ENABMULTI_REQ:
		mp = dlb_enabmulti(dlb, mp);
		break;
	case DL_DISABMULTI_REQ:
		mp = dlb_disabmulti(dlb, mp);
		break;
	case DL_PROMISCON_REQ:
		mp = dlb_promiscon(dlb, mp);
		break;
	case DL_PROMISCOFF_REQ:
		mp = dlb_promiscoff(dlb, mp);
		break;
        case DL_PHYS_ADDR_REQ:
		mp = dlb_phys_addr(dlb, mp);
		break;
	case DL_GET_STATISTICS_REQ:
		mp = dlb_get_statistics(dlb, mp);
		break;

	/*
	 * since XID and TEST are automatically supported,
	 * any explicit request needs to be nak'd, and
	 * any response must be in error since no indication generated
	 */

	case DL_XID_REQ:
		mp = dlb_error_ack(mp, dlur->dl_primitive, 0, DL_XIDAUTO, dlb);
		break;
	case DL_TEST_REQ:
		mp = dlb_error_ack(mp, dlur->dl_primitive, 0, DL_TESTAUTO, dlb);
		break;
	case DL_XID_RES:
	case DL_TEST_RES:
		mp = dlb_error_ack(mp, dlur->dl_primitive, 0, DL_NOTSUPPORTED,
			dlb);
		break;

	/*
	 * see dlpi.h, error codes and descriptive text
	 *
	 * known, but not supported requests -
	 * if we get a primitive from the header file,
	 * but we don't support it: DL_NOTSUPPORTED
	 *
	 * only requests are listed here; unsupported indications
	 * and confirmations are not generated.
	 */
	
	/*
	 * things we don't support, but probably should:
	 * (in no particular order)
	 */

	case DL_UDQOS_REQ:
	case DL_SET_PHYS_ADDR_REQ:

	/*
	 * things we could support (on token ring/fddi only), but why?
	 * (aka DL_ACLDLS support)
	 */

	case DL_DATA_ACK_REQ:
	case DL_REPLY_REQ:
	case DL_REPLY_UPDATE_REQ:

	/*
	 * things we won't support, because 802.x LANs are CL only
	 */

	case DL_CONNECT_REQ:
	case DL_CONNECT_RES:
	case DL_TOKEN_REQ:
	case DL_DISCONNECT_REQ:
	case DL_RESET_REQ:
	case DL_RESET_RES:
		mp = dlb_error_ack(mp, dlur->dl_primitive, 0, DL_NOTSUPPORTED,
			dlb);
		break;

	/*
	 * unknown requests
	 */

	default:
		mp = dlb_error_ack(mp, dlur->dl_primitive, 0, DL_BADPRIM, dlb);
		break;
	}
	qreply(q, mp);
	return 0;
}

#include <sys/device.h>
#include <sys/uio.h>
#include <sys/strconf.h>

int
dlb_config(dev, cmd, uiop)
	dev_t dev;
	int cmd;
	struct uio *uiop;
{
	char buf[FMNAMESZ+1];
	static strconf_t conf = {
		"dlpi", &dlbinfo, STR_NEW_OPEN,
	};

	if (uiomove(buf, sizeof buf, UIO_WRITE, uiop))
		return EFAULT;
	buf[FMNAMESZ] = 0;

	conf.sc_name = buf;
	conf.sc_major = major(dev);
	conf.sc_flags |= STR_MPSAFE;
	conf.sc_sqlevel = SQLVL_QUEUEPAIR;

	switch (cmd) {
	case CFG_INIT:
		return dlb_init(&conf);
	case CFG_TERM:
		/* what about inuse protocols? cf xtiso */
		return dlb_term(&conf);
	default:
		return EINVAL;
	}
}

static int
dlb_init(sc)
	strconf_t *sc;
{
	int err;
	struct  dl_data *dl;

	DLPILOCK;
	dl = (struct dl_data *)xmalloc(sizeof *dl, 3, pinned_heap);
	if (!dl) {
		DLPIUNLOCK;
		return ENOMEM;
	}

	bzero(dl, sizeof(struct dl_data));
	dl->maj = sc->sc_major;
	bcopy(sc->sc_name, dl->ndd_name, FMNAMESZ+1);
	if (err = str_install(STR_LOAD_DEV, sc)) {
		(void)xmfree(dl, pinned_heap);
		DLPIUNLOCK;
		return err;
	}

	dl->next = dl_data;
	dl_data = dl;
	DLPIUNLOCK;

	/*
	 * There are no MP locks around all_stats due to performance
	 * degradation reasons.  In the docs, global statisitics are
	 * described as "vague" in an MP environment since sometimes
	 * the counters can be touched by more than one cpu at a time.
	 */
	bzero(&all_stats, sizeof(struct statistics));

	return 0;
}

static int
dlb_term(sc)
strconf_t *sc;
{
	struct  dl_data *dl, *odl = nilp(struct dl_data);
	int	err, rc = 0;

	DLPILOCK;

	for (dl = dl_data; dl; dl = dl->next) {
		if (dl->maj == sc->sc_major && 
			(!strcmp(dl->ndd_name, sc->sc_name)))
			break;
		odl = dl;
	}
	if (!dl) {	/* impossible */
		rc = ESRCH;
		goto out;
	}

	if (err = str_install(STR_UNLOAD_DEV, sc)) {
		rc = err;
		goto out;
	}

	if (odl)
		odl->next = dl->next;
	else
		dl_data = dl->next;
	(void)xmfree(dl, pinned_heap);

out:
	DLPIUNLOCK;
	return rc;
}
