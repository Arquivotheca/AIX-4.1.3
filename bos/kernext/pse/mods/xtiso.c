static char sccsid[] = "@(#)49        1.9.1.18  src/bos/kernext/pse/mods/xtiso.c, sysxpse, bos41J, 9515A_all 3/24/95 16:53:30";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS: DumpIOCBLK
 *		DumpMBLK
 *		DumpPROTO
 *		DumpSO
 *		Dumpbytes
 *		NEXTSTATE
 *		TNEXTSTATE
 *		copy_to_mbuf
 *		dbclose
 *		dbopen
 *		dbwput
 *		m_freeb
 *		m_leadingspace
 *		m_trailingspace
 *		mblk_to_mbuf
 *		mbufdsize
 *		mbuf_to_mblk
 *		new_tp_ind
 *		prec_insq
 *		xti_addr_req
 *		xti_bind_req
 *		xti_bufcall
 *		xti_canput
 *		xti_chk_protocol
 *		xti_cleanup
 *		xti_conn_con
 *		xti_conn_ind
 *		xti_conn_req
 *		xti_conn_res
 *		xti_data_ind
 *		xti_data_req
 *		xti_discon_ind
 *		xti_discon_req
 *		xti_entry_init
 *		xti_exdata_req
 *		xti_finished
 *		xti_info_req
 *		xti_init
 *		xti_init_default_options
 *		xti_init_socket
 *		xti_input
 *		xti_ip_optmgmt
 *		xti_listen_req
 *		xti_opt_flags
 *		xti_optmgmt_req
 *		xti_ordrel_ind
 *		xti_ordrel_req
 *		xti_output
 *		xti_panic
 *		xti_putnext
 *		xti_rcv
 *		xti_rqenable
 *		xti_send
 *		xti_setopt
 *		xti_snd_error_ack
 *		xti_snd_flushrw
 *		xti_snd_ok_ack
 *		xti_startup
 *		xti_strlog
 *		xti_tcp_optmgmt
 *		xti_term
 *		xti_uderror_ind
 *		xti_udp_optmgmt
 *		xti_unbind_req
 *		xti_unbufcall
 *		xti_unitdata_ind
 *		xti_unitdata_req
 *		xti_wqenable
 *		xti_xti_optmgmt
 *		xticlose
 *		xtiopen
 *		xtirsrv
 *		xtiso_config
 *		xtiwput
 *		xtiwsrv
 *
 *   ORIGINS: 18,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.2
 */

/*
 * TODO:
 *
 * . TS_UNINIT or T_UNINIT state - should these be defined to be truly
 *   XTI conformant?  The XTI documentation references the user space
 *   T_UNINITed state
 * . If an M_IOCTL request to NDELAY on STREAM, seems as though
 *   we should ACK it and then send a M_SETOPTS / SO_NDELOFF request
 *   back up to STREAM head to handle O_NDELAY processing up there...
 *
 */

/*
 * XTI-over-SOcket Pseudo-Device Driver
 *  (xtiso)
 *
 * Notes:
 *
 * XXX WARNING: 
 *	STREAMS code is not supposed to block (no u area), but...
 *
 *	AIX MPS is scheduled via a kproc, which directly runs
 *	the service routine, so a short sleep is possible.
 *	This is "good" for us because the following routines
 *	are called from a service proc:
 *		. xti_send() may sleep in sosend:sblock()
 *		. xti_rcv() may sleep in soreceive:m_get() and
 *			soreceive:sblock()
 *
 * Compilation flags:
 *	-DXNS		include NS support
 *	-DISO		include ISO support
 *	-DCOMPAT_43	socket 4.3 compatability
 *	-DSTREAMS_DEBUG	include XTI debugging support (#defines XTIDEBUG)
 *
 * TODO:	XXX
 *	Add XNS support
 *	Add ISO support
 *	Numerous memory leaks.
 *	Fix anything associated with an XXX.
 */

#undef	STREAMS_DEBUG

#include <sys/param.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/proc.h>

/* Socket include files */
#include <sys/mbuf.h>		/* Pulls in <net/net_globals.h> for locks */
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>	/* Pulls in SOCKET_LOCK definitions */
#include <sys/syspest.h>

/* Protocol include files */
#include <sys/un.h>
#include <netinet/in.h>
#ifdef XNS
#include <netns/ns.h>			/* XXX add xns support */
#endif
#ifdef ISO
#include <netiso/headers.h>		/* XXX add iso support */
#endif
#include <net/netopt.h>

/* AIX configuration include files */
#include <sys/device.h>
#include <sys/strconf.h>

/* STREAMS include files */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/strlog.h>

/* TLI/XTI include files */
#include <sys/tihdr.h>
#undef T_IDLE				/* XXX removes timer.h definition */
#include <sys/xti.h>
#include "./xtiso.h"

/* #define	printf	mi_printf */	/* XXX how to handle printf */

/* forward declarations */

/* Socket mbufs<->STREAMS mblk data copy support */
extern mblk_t *mbuf_to_mblk(struct mbuf *, int);
extern struct mbuf *mblk_to_mbuf(mblk_t *, int);

/* stream data structure definitions */
int xtiopen  (queue_t *q, dev_t *dev, int flag, int sflag, cred_t *credp);
int xticlose (queue_t *q, int flag, cred_t *credp);
int xtiwput  (queue_t *q, mblk_t *mp);
int xtirsrv  (queue_t *q);
int xtiwsrv  (queue_t *q);

/* callback from sockets */
void xti_rqenable  (caddr_t q, int state);
void xti_wqenable  (caddr_t q, int state);

/* Socket glue */
#ifdef	COMPAT_43
#define SOCKNAME_FORMAT	1
#else
#define SOCKNAME_FORMAT	0
#endif

#define DGRAM_OPTS              512
#define STREAM_OPTS             512

/*
 * Supported protocols
 */

static xtiproto_t udp_proto = /* UDP */
	  { 0, AF_INET, SOCK_DGRAM, 0, T_CLTS,
	    8192, -2, -2, -2, sizeof(struct sockaddr_in), DGRAM_OPTS, 8192 };

static xtiproto_t tcp_proto = /* TCP */
	  { 0, AF_INET, SOCK_STREAM, 0, T_COTS_ORD,
	    0, -1, -2, -2, sizeof(struct sockaddr_in), STREAM_OPTS, 4096 };

static xtiproto_t udg_proto = /* Unix DGRAM */
	  { 0, AF_UNIX, SOCK_DGRAM, 0, T_CLTS,
	    8192, -2, -2, -2, sizeof(struct sockaddr_un), DGRAM_OPTS, 8192 };

static xtiproto_t ustr_proto = /* Unix STREAM */
	  { 0, AF_UNIX, SOCK_STREAM, 0, T_COTS_ORD,
	    0, -1, -2, -2, sizeof(struct sockaddr_un), STREAM_OPTS, 4096 };

#ifdef XNS
static xtiproto_t idp_proto = /* IDP */
	  { 0, AF_NS, SOCK_DGRAM, 0, T_CLTS,
	    8192, -2, -2, -2, sizeof(struct sockaddr_ns), -2, 4096 };

static xtiproto_t spp_proto = /* SPP */
	  { 0, AF_NS, SOCK_STREAM, 0, T_COTS_ORD,
	    0, -1, -2, -2, sizeof(struct sockaddr_ns), 0, 4096 };
#endif	/* XNS */

#ifdef ISO
add iso protocol structs here
#endif	/* ISO */

static lock_t xtisolock = LOCK_AVAIL;	/* config struct chain lock */
#define CFGLOCK (void)lockl(&xtisolock,LOCK_SHORT)	/* lock */
#define CFGUNLOCK (void)unlockl(&xtisolock)		/* unlock */
/* Locks for each xtisocfg structure */
#define XTILOCK(xc) lockl(&(xc)->xti_cfglock,LOCK_SHORT)
#define XTIUNLOCK(xc) unlockl(&(xc)->xti_cfglock)

static  IDP     xtiso_g_head = nil(IDP);

/*
 * Head of configured "invocations".
 */
struct xtisocfg *xtisocfg;

static struct module_info xti_info =
{
	XTI_INFO_ID, 	/* module ID number */
	"xtiso",	/* module name 	*/
	0,		/* min pkt size accepted */
	INFPSZ, 	/* max pkt size accepted */
	4096,		/* hi-water mark, flow control */
	1024		/* lo-water mark, flow control */
};

/* Read queue init structure */
static struct qinit xtirinit =
{
	nil(pfi_t), 			/* put procedure */
	xtirsrv, 			/* service procedure */
	xtiopen, 			/* called every open or push */
	xticlose, 			/* called every close or pop */
	nil(pfi_t), 			/* admin - reserved */
	&xti_info,			/* information structure */
	(struct module_stat *)0		/* stats structure - unused */
};

/* Write queue init structure */
static struct qinit xtiwinit =
{
	xtiwput,   			/* put procedure */
	xtiwsrv, 			/* service procedure */
	nil(pfi_t),			/* open procedure - unused */
	nil(pfi_t), 			/* close procedure - unused */
	nil(pfi_t), 			/* admin - reserved */
	&xti_info,			/* info structure */
	(struct module_stat *)0		/* stats structure - unused */
};

/* Basic STREAMS data structure */
struct streamtab xtisoinfo =
{
	&xtirinit, 		/* read queue init */
	&xtiwinit,		/* write queue init */
	(struct qinit *)0,	/* read queue init - mux driver (unused) */
	(struct qinit *)0	/* write queue init - mux driver (unused) */
};

typedef struct iocblk *IOCP;

/*
 * TPI Finite State Machine - State Transition Table
 *
 * Beware: states and events match symbolic definitions in tihdr.h.
 * Table is rooted at 0, states, events at 1.
 * Is state 18 (TS_WACK_ORDREL) useful?
 */

#if	(TE_NOEVENTS != 29) || (TS_NOSTATES != 19)
/*#error XXX */ XTI state table out of sync - TE_NOEVENTS,TS_NOSTATES != 29,19
#endif

#define TS_BAD_STATE	-1

#ifdef	XTIDEBUG

int xtiDEBUG = 0;

#define strlog \
	if (xtiDEBUG & XTIF_STRLOG) xti_strlog /* (args follow) */

#define TNEXTSTATE(x,e) \
	(((x)->xti_state && (unsigned)((x)->xti_state) < TS_NOSTATES) ? \
		xti_fsm[e][(x)->xti_state] : TS_BAD_STATE)

#define NEXTSTATE(x,e,s) \
	(((x)->xti_state && (unsigned)((x)->xti_state) < TS_NOSTATES) ? \
		((x)->xti_state = xti_fsm[e][(x)->xti_state]) : \
		(xti_panic(s), ((x)->xti_state = TS_BAD_STATE)))

#else	/* !XTIDEBUG */

#define TNEXTSTATE(x,e)		xti_fsm[e][(x)->xti_state]
#define NEXTSTATE(x,e,s)	(x)->xti_state = xti_fsm[e][(x)->xti_state]
#define xti_panic(s)		panic(s)

#endif

static short xti_fsm[TE_NOEVENTS][TS_NOSTATES] = {
#define __ -1
/* TE_...   | TS_...  N/A 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18*/
/* 0  N/A         */ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 1  BIND_REQ    */ {__, 2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 2  UNBIND_REQ  */ {__,__,__,__, 3,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 3  OPTMGMT_REQ */ {__, 1,__,__, 5,__,__, 7, 8,__,10,11,12,__,__,__,__,__,__},
/* 4  BIND_ACK    */ {__,__, 4,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 5  OPTMGMT_ACK */ {__, 1,__,__,__, 4,__, 7, 8,__,10,11,12,__,__,__,__,__,__},
/* 6  CONN_REQ    */ {__,__,__,__, 6,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 7  CONN_RES    */ {__,__,__,__,__,__,__,__, 9,__,__,__,__,__,__,__,__,__,__},
/* 8  DISCON_REQ  */ {__,__,__,__,__,__,__,13,14,__,15,16,17,__,__,__,__,__,__},
/* 9  DATA_REQ    */ {__,__,__,__,__,__,__,__,__,__,10,__,12,__,__,__,__,__,__},
/* 10 EXDATA_REQ  */ {__,__,__,__,__,__,__,__,__,__,10,__,12,__,__,__,__,__,__},
/* 11 ORDREL_REQ  */ {__,__,__,__,__,__,__,__,__,__,11,__, 4,__,__,__,__,__,__},
/* 12 CONN_IND    */ {__,__,__,__, 8,__,__,__, 8,__,__,__,__,__,__,__,__,__,__},
/* 13 CONN_CON    */ {__,__,__,__,__,__,__,10,__,__,__,__,__,__,__,__,__,__,__},
/* 14 DATA_IND    */ {__,__,__,__,__,__,__,__,__,__,10,11,__,__,__,__,__,__,__},
/* 15 EXDATA_IND  */ {__,__,__,__,__,__,__,__,__,__,10,11,__,__,__,__,__,__,__},
/* 16 ORDREL_IND  */ {__,__,__,__,__,__,__,__,__,__,12, 4,__,__,__,__,__,__,__},
/* 17 DISCON_IND1 */ {__,__,__,__,__,__,__, 4,__,__, 4, 4, 4,__,__,__,__,__,__},
/* 18 DISCON_IND2 */ {__,__,__,__,__,__,__,__, 4,__,__,__,__,__,__,__,__,__,__},
/* 19 DISCON_IND3 */ {__,__,__,__,__,__,__,__, 8,__,__,__,__,__,__,__,__,__,__},
/* 20 ERROR_ACK   */ {__,__, 1, 4,__, 4, 4,__,__, 8,__,__,__, 7, 8,10,11,12,__},
/* 21 OK_ACK1     */ {__,__,__, 1,__,__, 7,__,__,__,__,__,__, 4,__, 4, 4, 4,__},
/* 22 OK_ACK2     */ {__,__,__,__,__,__,__,__,__,10,__,__,__,__, 4,__,__,__,__},
/* 23 OK_ACK3     */ {__,__,__,__,__,__,__,__,__, 4,__,__,__,__, 4,__,__,__,__},
/* 24 OK_ACK4     */ {__,__,__,__,__,__,__,__,__, 8,__,__,__,__, 8,__,__,__,__},
/* 25 PASS_CONN   */ {__,__,__,__,10,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 26 UNITDATA_REQ*/ {__,__,__,__, 4,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 27 UNITDATA_IND*/ {__,__,__,__, 4,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/* 28 UDERROR_IND */ {__,__,__,__, 4,__,__,__,__,__,__,__,__,__,__,__,__,__,__}
#undef __
};

/* MP declarations */
#define XTI_LOCK_DECL()      int _xtil;
#define XTI_LOCK(l)          _xtil = disable_lock(PL_IMP, l)
#define XTI_UNLOCK(l)        unlock_enable(_xtil, l)
Simple_lock  xtiso_lock;


#ifdef XTIDEBUG
/*
 * ======================
 * XTI Debugging Support.
 * ======================
 */

static int dbinuse = 0;

static int
dbopen(q, dev, flag, sflag)
	queue_t *q;
	dev_t dev;
	int flag, sflag;
{
	if (dbinuse)
		return EBUSY;
	dbinuse = 1;
	return 0;
}

static int
dbclose(q, flag)
	queue_t *q;
	int flag;
{
	dbinuse = 0;
	return 0;
}

static int
dbwput(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	if (mp->b_datap->db_type == M_DATA)
		xtiDEBUG = atoi(mp->b_rptr);
	freemsg(mp);
	return 0;
}

static xtiproto_t db_proto = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static struct module_info db_info = { 0, "dbxti", 0, INFPSZ, 0, 0 };
static struct qinit dbrinit = { 0, 0, dbopen, dbclose, 0, &db_info, 0 };
static struct qinit dbwinit = { dbwput, 0, 0, 0, 0, &db_info, 0 };
static struct streamtab dbinfo = { &dbrinit, &dbwinit };

#endif	/* XTIDEBUG */

/*
 * =========================
 * XTI Driver configuration.
 * =========================
 */

/*
 * xtiso_config - configure xtiso, once per protocol
 *
 * This routine is called multiple times, once for each protocol
 * supported.  The determination of which protocol to configure
 * is a match of the alias in uiop with an internal hardcoded name.
 */

int
xtiso_config(devno, cmd, uiop)
	dev_t devno;
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf;
	static struct {
		char *alias;
		xtiproto_t *proto;
		struct streamtab *str;
	} *p, p_list[] = {
		{ "unixdg",	&udg_proto,	&xtisoinfo, },
		{ "unixst",	&ustr_proto,	&xtisoinfo, },
		{ "udp",	&udp_proto,	&xtisoinfo, },
		{ "tcp",	&tcp_proto,	&xtisoinfo, },
#ifdef	XNS
		{ "idp",	&idp_proto,	&xtisoinfo, },
		{ "spp",	&spp_proto,	&xtisoinfo, },
#endif	/* XNS */
#ifdef	ISO
		{ "esis",	&esis_proto,	&xtisoinfo, },
		{ "clnp",	&clnp_proto,	&xtisoinfo, },
#endif	/* ISO */
#ifdef	XTIDEBUG
		{ "debug",	&db_proto,	&dbinfo,    },
#endif	/* XTIDEBUG */
		{ 0, },
	};
	char alias[FMNAMESZ+1];

	/* no protocol specified */
	if (!uiop)
		return EPROTONOSUPPORT;
	
	if (uiomove(alias, sizeof alias, UIO_WRITE, uiop))
		return EFAULT;

	/* search alias list for supported protocols */
	for (p = p_list; p->alias; p++) {
		if (!strcmp(alias, p->alias))
			break;
	}
	if (!p->alias)
		return EPROTONOSUPPORT;

	conf.sc_open_style = STR_NEW_OPEN;
	conf.sc_name = p->alias;
	conf.sc_str = p->str;
	conf.sc_major = major(devno);
	conf.sc_sqlevel = SQLVL_QUEUEPAIR;
	conf.sc_flags |= STR_MPSAFE;
	conf.sc_flags |= STR_Q_NOTTOSPEC;

	switch (cmd) {
	case CFG_INIT:	return xti_init(&conf, p->proto);
	case CFG_TERM:	return xti_term(&conf, p->proto);
	default:	return EINVAL;
	}
}

/*
 * xti_init - initialize a specific protocol
 *
 * returns 0 or errno
 */

static int
xti_init(sc, pr)
	strconf_t *sc;
	xtiproto_t *pr;
{
	struct xtisocfg *xc;
	int err, rc = 0;

	CFGLOCK;

	if (pr->xp_configured) {
		rc = EEXIST;
		goto out;
	}

	/* allocate & initialize a protocol configure block */
	if (!(xc = (struct xtisocfg *)xmalloc(sizeof *xc, 3, pinned_heap))) {
		rc = ENOMEM;
		goto out;
	}

	bzero((caddr_t)xc, sizeof *xc);
	xc->xti_cfgmajor = sc->sc_major;
	xc->xti_cfglock = LOCK_AVAIL;
	bcopy((caddr_t)pr, (caddr_t)&xc->xti_cfgproto, sizeof xc->xti_cfgproto);

	/* install the driver into streams */
	if (err = str_install(STR_LOAD_DEV, sc)) {
		(void)xmfree(xc, pinned_heap);
		rc = err;
		goto out;
	}

	/* link into list of supported protocols */
	xc->xti_cfgnext = xtisocfg;
	xtisocfg = xc;

	pr->xp_configured = 1;

out:
	CFGUNLOCK;
	return rc;
}

/*
 * xti_term - terminate support for a protocol
 *
 * returns 0 or errno
 */

static int
xti_term(sc, pr)
	strconf_t *sc;
	xtiproto_t *pr;
{
	struct xtisocfg *xc, *oxc;
	int err, rc = 0;

	CFGLOCK;

	if (!pr->xp_configured) {
		CFGUNLOCK;
		return ENXIO;
	}

	/* find the config control block for this proto */
	oxc = nilp(struct xtisocfg);
	for (xc = xtisocfg; xc; xc = xc->xti_cfgnext) {
		if (xc->xti_cfgproto.xp_dom == pr->xp_dom &&
		    xc->xti_cfgproto.xp_type == pr->xp_type &&
		    xc->xti_cfgproto.xp_proto == pr->xp_proto)
			break;
		oxc = xc;
	}
	if (!xc) {	/* impossible! */
		CFGUNLOCK;
		return ESRCH;
	}

	if (xc->xti_cfgnopen) {
		CFGUNLOCK;
		return EBUSY;
	}

	if (err = str_install(STR_UNLOAD_DEV, sc)) {
		CFGUNLOCK;
		return err;
	}

	/* unlink from supported protos list, mark unconfig, unload */
	if (oxc)
		oxc->xti_cfgnext = xc->xti_cfgnext;
	else
		xtisocfg = xc->xti_cfgnext;

	pr->xp_configured = 0;

	CFGUNLOCK;
	(void)xmfree(xc, pinned_heap);

	return rc;
}


/*
 * ==========================
 * XTI Open/close procedures.
 * ==========================
 */

/*
 * xtiopen - Driver open put procedure.
 *
 * Inputs:
 *	q	= read queue pointer
 *	dev	= major/minor device number
 *	flag	= file open flag
 *	sflag	= STREAMS open flag (ie. CLONEOPEN)
 *	credp   = STREAMS open credentials
 *
 * Outputs:
 *	None
 *
 * Return:
 *	If success, >= 0 minor device number to be used for context
 *	 for subsequent STREAMS I/O for this STREAM,
 *	Else, error.
 */
xtiopen(q, devp, flag, sflag, credp)
        queue_t *q;		/* read queue pointer */
        dev_t   *devp;
        int     flag;
        int     sflag;
	cred_t	*credp;
{
	register struct xticb    *xtip;
	register struct xtisocfg *xc;
	int error;
	int majorDevice = major(*devp);
	int minorDevice = minor(*devp);
	XTI_LOCK_DECL()

	CHECKPOINT("xtiopen");

	XTITRACE(XTIF_OPEN,
		printf(" xtiso: xtiopen() - q=%x OTHERQ(q)=%x, dev=%d flag=%x sflag=%x\n",
			q, OTHERQ(q), *devp, flag, sflag););

	if (sflag & CLONEOPEN) {
		XTITRACE(XTIF_OPEN,
			printf(" xtiso: this is a CLONEOPEN of %d\n",
				majorDevice););
	} else {
		XTITRACE(XTIF_OPEN, printf(" xtiso: this is an OPEN of %d/%d\n",
			majorDevice, minorDevice););
		xtip = (struct xticb *)q->q_ptr;
		if (xtip) {			/* Already open. */
			XTITRACE(XTIF_OPEN,
				printf(" xtiso: found xticb %x %d/%d\n",
					xtip, xtip->xti_cfg->xti_cfgmajor,
					xtip->xti_minor););
			if (xtip->xti_cfg->xti_cfgmajor != majorDevice)
				xti_panic("xtiopen major botch");
			if (xtip->xti_minor != minorDevice)
				xti_panic("xtiopen minor botch");
			return 0;
		}
	}

	/* Find config list head. Find better way? */
	CFGLOCK;
	for (xc = xtisocfg; xc; xc = xc->xti_cfgnext)
		if (majorDevice == xc->xti_cfgmajor) {
			XTILOCK(xc);		/* serialize access */
			xc->xti_cfgnopen++;	/* Take reference */
			break;
		}
	CFGUNLOCK;

	if (xc == 0) 
		return ENOPROTOOPT;

	/*
	 * No locks are used here since the xtiso_g_head is protected
	 * via the streamhead mult_sqh.
	 */
	if (error = mi_open_comm(&xtiso_g_head, sizeof *xtip, q, devp, flag, sflag, credp)) {
		XTITRACE(XTIF_OPEN,
			printf("xtiso: xtiopen() cdevsw_open_comm() failed %d\n", error););
		--xc->xti_cfgnopen;
		XTIUNLOCK(xc);
		return error;
	}
	xtip = (struct xticb *)q->q_ptr;
	XTITRACE(XTIF_OPEN,
		printf("xtiso: xtiopen() xtip 0x%x minor %d\n", xtip, minor(*devp)););
	bzero((caddr_t)xtip, sizeof *xtip);
	xtip->xti_cfg = xc;
	xtip->xti_minor = minor(*devp);

	if (error = xti_entry_init(xtip, q, XTI_NEWSOCK)) {
		XTITRACE(XTIF_OPEN,
			printf(" xtiso: xtiopen() failed to create socket\n"););
		--xc->xti_cfgnopen;
		XTIUNLOCK(xc);

		/*
		 * No locks are used here since the xtiso_g_head is protected
	 	 * via the streamhead mult_sqh.
	 	 */
		(void)mi_close_comm(&xtiso_g_head, q);
		return error;
	}

	XTIUNLOCK(xc);

	lock_alloc(&xtip->xtiso_lock, LOCK_ALLOC_PIN, XTISO_LOCK_FAMILY, -1);
	simple_lock_init(&xtip->xtiso_lock);

	/*
	 * Global sequence # initialized only once per STREAM/Socket open
	 */
	xtip->xti_seqcnt = 1;
	xtip->xti_nam = 0;      /* initialize mbuf to save bind address */

	/*
	 * Set SS_PRIV if suser
	 * It will be turned off after the first bind.
	 */
	if (drv_priv(credp) == 0) {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags |= XTI_PRIV;
		XTI_UNLOCK(&xtip->xtiso_lock);
		SOCKET_LOCK(xtip->xti_so);
		xtip->xti_so->so_state |= SS_PRIV;
		SOCKET_UNLOCK(xtip->xti_so);
	}

	return 0;
}

/*
 * xticlose - Driver close put procedure.
 */
xticlose(q, flag, credp)
	register queue_t *q;
	int flag;
	cred_t *credp;
{
	register struct xticb *xtip;
	int error = 0;
	XTI_LOCK_DECL()

	CHECKPOINT("xticlose");
	XTITRACE(XTIF_CLOSE,
		printf(" xtiso: xticlose() of q=%08x, OTHERQ=%08x, so=%08x\n",
						q, OTHERQ(q), xtip->xti_so););

	/*
	 * Flush data/etc out of our queues
	 */

	flushq(q, FLUSHALL);
	flushq(OTHERQ(q), FLUSHALL);

	/*
	 * If xti context block pointer is NULL, then
	 * no more cleanup
	 */
	if (xtip = (struct xticb *)q->q_ptr) {

		if (xtip->xti_bufcallid)
			xti_unbufcall(q);
		/*
		 * avoid a "race" condition where xti_finished causes a 
		 * socket upcall to xti_[rw]qenable, which calls qenable(), 
		 * which puts our queue on the synch queue, and a subsequent 
		 * panic in qrun after streams frees everything associated 
		 * with this q.
		 */
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags	&= ~(XTI_ACTIVE);
		XTI_UNLOCK(&xtip->xtiso_lock);

		if (xtip->xti_nam)
			(void) m_free(xtip->xti_nam);
		/*
	 	 * Initiate general cleanup
	 	 */
		xti_finished(q, XTI_CLOSESOCK);

		XTILOCK(xtip->xti_cfg);
		--xtip->xti_cfg->xti_cfgnopen;
		XTIUNLOCK(xtip->xti_cfg);
	}

	lock_free(&xtip->xtiso_lock);

	/*
	 * Unlink and free structure.
	 * No locks are used here since the xtiso_g_head is protected
	 * via the streamhead mult_sqh.
	 */
	error = mi_close_comm(&xtiso_g_head, q);

	return error;
}


/*
 * ============================
 * XTI Write-side put routines.
 * ============================
 */

/*
 * xtiwput - Driver write-side put procedure.
 *           It validates the message type, perform flush operations,
 *           handles ioctl commands, and puts the message into the
 *           the write-side queue of this stream.
 */
xtiwput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register xtiproto_t *xp;
	XTI_LOCK_DECL()

	CHECKPOINT("xtiwput");

	XTITRACE
	(
		XTIF_WPUT,
		printf(" xtiso: xtiwput() entry...q=%x, OTHERQ(q)=%x, mp=%x\n",
			q, OTHERQ(q), mp);
	);

	pp   = (union T_primitives *)mp->b_rptr;
	xtip = (struct xticb *)q->q_ptr;
	xp   = &(xtip->xti_proto);

	XTITRACE(XTIF_WPUT,
		printf(" xtiso: xtiwput() - M_message type=%d\n",
			(int)mp->b_datap->db_type););

	switch((int)mp->b_datap->db_type) {
	case M_DATA:

		XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - M_DATA\n"););
		prec_insq(q, mp);
		return;

	case M_PROTO:
	case M_PCPROTO:

		/* use smallest primitive for comparison */
		if (mp->b_wptr - mp->b_rptr < sizeof(struct T_info_req)) {
			if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
				    SL_TRACE|SL_ERROR, 
				    "xti_wput: incorrect message size\n");
			return;
		}

		switch (pp->type) {
		case T_UNITDATA_REQ:

			XTITRACE(XTIF_WPUT,
			    printf(" xtiso: xtiwput() - T_UNITDATA_REQ\n"););
			if (xp->xp_servtype != T_CLTS) {
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
					SL_TRACE|SL_ERROR,
				"xtiwput: unitdata_req on non-CLTS provider\n");
				xti_cleanup(q, mp, EPROTO);
			} else
				prec_insq(q, mp);
			return;

		case T_ORDREL_REQ:

			XTITRACE(XTIF_WPUT,
			    printf(" xtiso: xtiwput() - T_ORDREL_REQ\n"););
			if (xp->xp_servtype != T_COTS_ORD) {
				strlog(XTI_INFO_ID, xtip->xti_minor,0, 
					SL_TRACE|SL_ERROR,
				"xtiwput: ordrel_req on non-COTS_ORD provider\n");
				xti_cleanup(q, mp, EPROTO);
			} else
				prec_insq(q, mp);
			return;

		case T_DATA_REQ:
		case T_EXDATA_REQ:

			XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - %s\n",
			    pp->type == T_DATA_REQ ?
				"T_DATA_REQ" : "T_EXDATA_REQ"););

			if ((xp->xp_servtype != T_COTS_ORD) &&
			    (xp->xp_servtype != T_COTS)) {
				strlog(XTI_INFO_ID, xtip->xti_minor,0, 
					SL_TRACE|SL_ERROR,
				"xtiwput: data_req on non-COTS provider\n");
				xti_cleanup(q, mp, EPROTO);
			} else
				prec_insq(q, mp);
			return;

		case T_CONN_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_CONN_REQ\n");
				goto next1;);
		case T_CONN_RES:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_CONN_RES\n");
				goto next1;);
		case T_DISCON_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_DISCON_REQ\n");
				goto next1;);
		next1:
			/* Provider will test for COTS/COTS_ORD */
			prec_insq(q, mp);
			return;

		case T_BIND_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_BIND_REQ\n");
				goto next2;);
		case T_OPTMGMT_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_OPTMGMT_REQ\n");
				goto next2;);
		case T_UNBIND_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_UNBIND_REQ\n");
				goto next2;);
		case T_INFO_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_INFO_REQ\n");
				goto next2;);
		case T_ADDR_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_BOUND_REQ\n");
				goto next2;);
		next2:
			prec_insq(q, mp);
			return;

		default:
			strlog(XTI_INFO_ID,xtip->xti_minor,0, SL_TRACE|SL_ERROR,
				"xtiwput: bad TPI message type %d\n", pp->type);
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	case M_IOCTL:

		XTITRACE(XTIF_WPUT, printf(" xtiso: xtiwput() - M_IOCTL\n"););
		XTITRACE(XTIF_WPUT, DumpIOCBLK((IOCP)mp->b_rptr););

		switch (((IOCP)mp->b_rptr)->ioc_cmd) {
		/* No ioctl's handled! */
		default:
			XTITRACE(XTIF_WPUT,
			    printf(" xtiso: xtiwput() - M_IOCNAK for cmd=%d\n",
				((IOCP)mp->b_rptr)->ioc_cmd););
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return;
		}

 	case M_FLUSH:

		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE,
			"xtiwput: M_FLUSH type %d received mp = %x\n",
	       		mp->b_datap->db_type, mp);

		XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - M_FLUSH\n"););

		if (*mp->b_rptr & FLUSHW) {
			if (xtip->xti_wdata) {
				freemsg(xtip->xti_wdata);
				xtip->xti_wdata = 0;
				xtip->xti_tsdu = 0;
				if (xtip->xti_wnam) {
					(void)m_free(xtip->xti_wnam);
					xtip->xti_wnam = 0;
				}
				XTI_LOCK(&xtip->xtiso_lock);
				xtip->xti_flags &= ~(XTI_FLOW | XTI_MOREDATA);
				XTI_UNLOCK(&xtip->xtiso_lock);
			}
			if (xtip->xti_wexdata) {
				freemsg(xtip->xti_wexdata);
				xtip->xti_wexdata = 0;
				xtip->xti_etsdu = 0;
				XTI_LOCK(&xtip->xtiso_lock);
				xtip->xti_flags &= ~(XTI_FLOW | XTI_MOREEXDATA);
				XTI_UNLOCK(&xtip->xtiso_lock);
			}
			flushq(q, FLUSHALL);
		}

		if (*mp->b_rptr & FLUSHR) {
			if (xtip->xti_rdata) {
				(void)m_freem(xtip->xti_rdata);
				xtip->xti_rdata = 0;
				xtip->xti_rflags = 0;
			}
			flushq(xtip->xti_rq, FLUSHALL);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
		} else
			freemsg(mp);

		return;

	default:
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xtiwput: Non understood msg type %d received mp= %x\n",
			mp->b_datap->db_type, mp);
		XTITRACE(XTIF_WPUT, printf(" xtiso: xtiwput() - M_? (%d)\n",
			(int)mp->b_datap->db_type););
		freemsg(mp);
		return;
	}
}

/*
 * prec_insq - 	insert a message to the write-side queue based on the
 *		the precedence orders defined in the TPI Specification
 */
prec_insq(q, mp)
	register queue_t *q;
	register mblk_t  *mp;
{
	register union  T_primitives *pp, *pp1;
	register mblk_t *mp1;
	register struct xticb *xtip;

	CHECKPOINT("prec_insq");

	XTITRACE(XTIF_MISC,
	    printf(" xtiso: prec_insq() - entry...q=%x, OTHERQ(q)=%x, mp=%x\n",
			q, OTHERQ(q), mp););

	xtip = (struct xticb *)q->q_ptr;
	pp   = (union T_primitives *)mp->b_rptr;

	if ((mp->b_datap->db_type != M_PROTO) &&
	    (mp->b_datap->db_type != M_PCPROTO) ||
	    (pp->type != T_EXDATA_REQ) &&
 	    (pp->type != T_DISCON_REQ)) {
		putq(q, mp);
		return;
	}
	for (mp1 = q->q_first; mp1 != nilp(mblk_t); mp1 = mp1->b_next) {
		pp1 = (union T_primitives *)mp1->b_rptr;
		switch (pp1->type) {
		case T_DATA_REQ:
			if (pp->type == T_EXDATA_REQ) {
				insq(q, mp1, mp);
				return;
			}
			break;
		case T_CONN_REQ:
			if (pp->type == T_DISCON_REQ) {
				/*
				 * Remove previously sent connection
				 * request and the current discon request.
				 */
				rmvq(q, mp1);
				freemsg(mp1);
				XTITRACE(XTIF_CONNECT,
				    printf(" xtiso: prec_insq() - xti_snd_ok_ack(q=%x mp=%x)\n",
					q, mp););
				if (!xti_snd_ok_ack(q, mp))
					xtip->xti_pendcall = xti_snd_ok_ack;
				return;
		 	}
			break;
		case T_CONN_RES:
			if (pp->type == T_DISCON_REQ) {
				rmvq(q, mp1);
				freemsg(mp1);
			}
			break;
        	}
	}
	putq(q, mp);
}

/*
 * ================================
 * XTI Write-side service routines.
 * ================================
 */

/*
 * xtiwsrv - xti write-side service procedure
 */
xtiwsrv(q)
	register queue_t *q;
{
	register mblk_t *mp;
	register struct xticb *xtip;
	int id;
	XTI_LOCK_DECL()

	CHECKPOINT("xtiwsrv");

	XTITRACE(XTIF_WSRV,
		printf(" xtiso: xtiwsrv() - entry...q=%x, OTHERQ(q)=%x\n",
			q, OTHERQ(q)););

	if (!(xtip = (struct xticb *)q->q_ptr))
		return;

	if (id = xtip->xti_bufcallid)
		xti_unbufcall(q);

	if (xtip->xti_pendcall) {
		if ((*xtip->xti_pendcall)(q, (mblk_t *)0)) {
			xtip->xti_pendcall = 0;
			xtip->xti_errtype = xtip->xti_tlierr = 0;
			xtip->xti_unixerr = 0;
		} else
			return;
	}

	/*
	 * If we previously completed the connection via ordrel's and the
	 * socket is disconnected, then close the socket and open a new one. 
	 * When the FINs and ACKs completes, then we will get upcall and if 
	 * the flag is set and socket is disconnected, then call xti_startup().
	 */

	if ((xtip->xti_flags & XTI_NEEDSTARTUP) && (xtip->xti_so) && 
			(!(xtip->xti_so->so_state & SS_ISCONNECTED))) {
		int ostate = xtip->xti_state;
		XTITRACE(XTIF_WSRV,
		printf("xtiso: xtiwsrv() - doing startup on xtip %x\n", xtip););
		if (xti_startup(q, 0, xtip->xti_qlen)) {
			xti_cleanup(q, 0, EPROTO);
			return;
		}
		xtip->xti_state = ostate;
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags &= ~XTI_NEEDSTARTUP;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}

	for (;;) {
		/*
		 * The immediate next send is built in wdata and/or
		 * wexdata, and marked complete with the MORE flag.
		 * If there, send it in case awakened via socket
		 * upcall, then go look at next message. If send
		 * fails, XTI_FLOW will be set TRUE and sends queued.
		 */
		if (xtip->xti_wexdata && !(xtip->xti_flags & XTI_MOREEXDATA)) {

			XTITRACE(XTIF_WSRV,
			printf(" xtiso: xtiwsrv() sending exdata\n"););

			/* Sockets give an extra chunk to exdata */
			if (xti_canput(q, msgdsize(xtip->xti_wexdata)-1024) < 0
			    || xti_send(xtip, T_EXDATA_REQ) != 0)
				break;
		}

		if (xtip->xti_wdata && !(xtip->xti_flags & XTI_MOREDATA)) {

			XTITRACE(XTIF_WSRV,
			printf(" xtiso: xtiwsrv() sending data\n"););

			if (xti_canput(q, msgdsize(xtip->xti_wdata)) < 0 ||
			    xti_send(xtip, T_DATA_REQ) != 0)
				break;
		}
doqueue:
		/*
		 * Go feed stream. Somewhat suboptimal if currently flow
		 * controlled and next message is data send, but...
		 */
		mp = getq(q);
		XTITRACE(XTIF_WSRV,
		   printf(" xtiso: xtiwsrv() - getq() returned: mp=%x\n", mp););
		if (mp == nilp(mblk_t))
			break;

		if (xtip->xti_flags & XTI_FATAL) {
			freemsg(mp);
			break;
		}

		/*
		 * Try to move the message downstream...
		 */
		xti_output(q, mp);

		/*
		 * . If buffer shortage occurs, "xti_bufcall" has been issued
		 *   to callback sometime soon.
		 * . If flow-controlled, then get out of service routine
		 *   and wait to get rescheduled...
		 */
		if ((xtip->xti_flags & XTI_FLOW) || xtip->xti_bufcallid)  {
			XTITRACE(XTIF_WSRV|XTIF_SEND_FLOW,
			    printf(" xtiso: xtiwsrv() - early return flow=%d\n",
				(xtip->xti_flags & XTI_FLOW) != 0););
			break;
		}
		if (id && !xtip->xti_bufcallid) {
			id = 0;
			qenable(OTHERQ(q));
		}
	}
}

/*
 * xti_output - process TPI messages with proper calls
 */
xti_output(q, mp)
	queue_t  *q;
	register mblk_t *mp;
{
	register union  T_primitives *pp;
	register struct xticb *xtip;

	CHECKPOINT("xti_output");

	XTITRACE(XTIF_OUTPUT,
	    printf(" xtiso: xti_output() entry...q=%x, OTHERQ(q)=%x, mp=%x\n",
		q, OTHERQ(q), mp););

	xtip = (struct xticb *)q->q_ptr;
	pp   = (union T_primitives *)mp->b_rptr;
	switch (mp->b_datap->db_type) {
	case M_DATA:
		if (xtip->xti_flags & XTI_FLOW)
			putbq(q, mp);
		else
			xti_data_req(q, mp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		switch (pp->type) {
		case T_UNITDATA_REQ:
			if (xtip->xti_flags & XTI_FLOW)
				putbq(q, mp);
			else
				xti_unitdata_req(q, mp);
			break;

		case T_ORDREL_REQ:
			xti_ordrel_req(q, mp);
			break;

		case T_DATA_REQ:
			if (xtip->xti_flags & XTI_FLOW)
				putbq(q, mp);
			else
				xti_data_req(q, mp);
			break;

		case T_EXDATA_REQ:
			if (xtip->xti_flags & XTI_FLOW)
				putbq(q, mp);
			else
				xti_exdata_req(q, mp);
			break;

		case T_CONN_REQ:
			xti_conn_req(q, mp);
			break;

		case T_CONN_RES:
			xti_conn_res(q, mp);
			break;

		case T_DISCON_REQ:
			xti_discon_req(q, mp);
			break;

		case T_INFO_REQ:
			xti_info_req(q, mp);
			break;

		case T_BIND_REQ:
			xti_bind_req(q, mp);
			break;

		case T_OPTMGMT_REQ:
			xti_optmgmt_req(q, mp);
			break;

		case T_UNBIND_REQ:
			xti_unbind_req(q, mp);
			break;

		case T_ADDR_REQ:
			xti_addr_req(q, mp);
			break;

		default:
			xti_panic("xti_output 1");
			break;
		}

		break;

	case M_IOCTL:

		XTITRACE(XTIF_OUTPUT,
			printf(" xtiso: xti_output() - M_IOCTL\n");
			DumpIOCBLK((IOCP)mp->b_rptr););

		/* Huh? */
		switch (((IOCP)mp->b_rptr)->ioc_cmd) {
		default:
			xti_panic("xti_output 2");
			break;
		}
		break;

	default:
		xti_panic("xti_output 3");
		break;
	}
}

/*
 * ===============================
 * XTI Read-side service routines.
 * ===============================
 */

/*
 * xtirsrv - xti read-side service procedure
 */
xtirsrv(q)
	register queue_t *q;
{
	register struct xticb *xtip;
	int id = 0;

	CHECKPOINT("xtirsrv");

	if (!(xtip = (struct xticb *)q->q_ptr))
		return;

	if (id = xtip->xti_bufcallid)
		xti_unbufcall(q);

	for (;;) {
		if (xtip->xti_flags & XTI_FATAL)
			break;
		if (!canput(q->q_next))
			break;
		if (!xti_input(xtip))
			break;
		if (xtip->xti_bufcallid)
			break;
		if (id && !xtip->xti_bufcallid) {
			id = 0;
			qenable(OTHERQ(q));
		}
	}
}

/*
 *  xti_input - input data from the socket and convert them to TPI msgs
 *
 *  return values:
 *	 non-0 		- receives data/information from the socket.
 *	 0		- receives nothing from the socket.
 */
xti_input(xtip)
	register struct xticb *xtip;
{
	int indication, status = 1;

	CHECKPOINT("xti_input");

	if (xtip->xti_pendind) {
		indication = xtip->xti_pendind;
		xtip->xti_pendind = 0;
	} else
		indication = new_tp_ind(xtip);

	switch (indication) {
	case T_CONN_CON:
		xti_conn_con (xtip);
		break;

	case T_CONN_IND:
		xti_conn_ind (xtip);
		break;

	case T_DISCON_IND:
		if (xtip->xti_flags & XTI_DISCONMAIN) 
			while (xtip->xti_cindno > 0)
				xti_discon_ind (xtip);
		xti_discon_ind (xtip);
		break;

	case T_DATA_IND:
	case T_EXDATA_IND:
		xti_data_ind (indication, xtip);
		break;

	case T_ORDREL_IND:
		xti_ordrel_ind (xtip);
		break;

	case T_UNITDATA_IND:
		xti_unitdata_ind (xtip);
		break;

	case T_UDERROR_IND:
		xti_uderror_ind (xtip, nilp(mblk_t), (struct mbuf *)0, 
				xtip->xti_soerror);
		break;

	case T_ERROR_ACK:
		if (xti_snd_error_ack(xtip->xti_rq, nilp(mblk_t), 0, 0)) {
			xtip->xti_tlierr	= 0;
			xtip->xti_unixerr	= 0;
			xtip->xti_errtype	= 0;
		}
		break;

	default: /* either nothing to be received or error occurred */

		XTITRACE(XTIF_INPUT,
		  printf(" xtiso: xti_input() - indication=%d\n", indication););
		status = 0;
		break;
	}

	/* Buffer alloc failed - bufcall issued */
	if (xtip->xti_bufcallid)
		status = 0;

	return (status);
}

/*
 * new_tp_ind - return indication from socket layer.
 */
new_tp_ind(xtip)
	register struct xticb *xtip;
{
	register struct  socket *so;
	register struct  xtiseq *seq;
	register int 	 connected;
		 int     state;
		 int     lastState;
		 int 	 indication = 0;
		 int	 error = 0;
	XTI_LOCK_DECL()
	/*
	 * The following bit mask indicates that a socket is connected.
	 */
#	define connected (SE_CONNOUT | SE_SENDCONN | SE_RECVCONN)

	CHECKPOINT("new_tp_ind");

 	so = xtip->xti_so;
	if (!so)
		return 0;
	/*
	 * Save asynch socket state bits
	 */
	(void) sbpoll(so, &(so->so_rcv));
	lastState = xtip->xti_sostate;
	XTI_LOCK(&xtip->xtiso_lock);
	xtip->xti_sostate = 0;			/* Clear for new bits */
	XTI_UNLOCK(&xtip->xtiso_lock);

	/*
	 * Update socket state bits and synch socket resources.
	 * Need to fold data bits into state bits.
	 */
	state  = sbpoll(so, &(so->so_rcv));
	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: lastState %04x state %04x xti_sostate %04x\n",
					lastState, state, xtip->xti_sostate););
	state |= xtip->xti_sostate;		/* After sbpoll */

	switch (xtip->xti_proto.xp_servtype) {
	case T_CLTS:

		/*
		 * look for a unitdata indication
		 */
		if (xtip->xti_state == TS_IDLE) {
			if (state & SE_ERROR) {
				XTITRACE(XTIF_EVENTS, printf(" xtiso: new_tp_ind() - T_UDERROR_IND\n"););
				return(T_UDERROR_IND);
			}

			if (state & SE_HAVEDATA) {
				XTITRACE(XTIF_EVENTS,  printf(" xtiso: new_tp_ind() - SE_HAVEDATA, T_UNITDATA_IND\n"););
				return (T_UNITDATA_IND);
			}
		}
		break;

	case T_COTS_ORD:
	case T_COTS:

		/*
		 * look for a discon/ordrel indication
		 */
		switch (xtip->xti_state) {
		case TS_WCON_CREQ:
			if ((state & connected) != connected) {
				indication = T_DISCON_IND;
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=TS_WCON_CREQ, T_DISCON_IND\n", xtip););
			}
                	break;

		case TS_WREQ_ORDREL:
			if (((state & SE_ERROR) && 
				xtip->xti_soerror == ECONNRESET) || 
				!(state & SE_CONNOUT)) {
					indication = T_DISCON_IND;
					XTITRACE(XTIF_EVENTS,
						printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=TS_WREQ_ORDREL, T_DISCON_IND\n", xtip););
			}
                	break;

		case TS_DATA_XFER:	/* When data consumed */
			if (!((state & (SE_HAVEDATA|SE_HAVEOOB)) ||
				xtip->xti_rdata) &&
			    (state & connected) != connected) {
				indication = T_DISCON_IND;
				if (!(state & SE_RECVCONN) &&
				     (state & SE_SENDCONN) &&
				    xtip->xti_proto.xp_servtype == T_COTS_ORD)
					indication = T_ORDREL_IND;
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=TS_DATA_XFER, %s\n",
						xtip, (indication == T_DISCON_IND ? "T_DISCON_IND" : "T_ORDREL_IND")););
			}
			break;

		case TS_WIND_ORDREL:
			/* In T_OUTREL case, after t_sndrel() if it gets
			 * more data, read until SS_CANTRCVMORE flag is set.
			 * If this flag is set, then shutdown the socket and
			 * set the XTI_ORDREL flag and so in the next upcall
			 * we will call xti_ordrel_ind() to send event to
			 * user, this way it will delay the indication.
			 */
			if (!((state & (SE_HAVEDATA|SE_HAVEOOB)) ||
				xtip->xti_rdata) && (!(state & SE_RECVCONN))) {
				if (xtip->xti_flags & XTI_ORDREL)
					indication = T_ORDREL_IND;
				else {
					if (soshutdown(xtip->xti_so, 0)) 
						indication = T_ORDREL_IND;
					else {
					       XTI_LOCK(&xtip->xtiso_lock);
					       xtip->xti_flags |= XTI_ORDREL;
					       XTI_UNLOCK(&xtip->xtiso_lock);
					       return 0;
					}
				}
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=TS_WIND_ORDREL, T_ORDREL_IND\n", xtip););
			}
                	break;

		case TS_WRES_CIND:
#ifdef XXX
			/* XXX
			   Looks like this will never hit this condition !!
			   At this point, if the disconnect indication comes,
			   then it's for accepted connections and so no need
			   to disconnect the Server.
			*/
			if (((state & connected) != connected) &&
				((lastState & connected) == connected)) {
				/*
				 * The main connection for the Server
			 	 * is being disconnected.  We need
			 	 * to disconnect all of the pending
			 	 * connection indications first.  This
			 	 * is why we set the flag here to ensure
			 	 * proper indications will be passed up
				 * later from xtirsrv().
			 	 */
				XTI_LOCK(&xtip->xtiso_lock);
				xtip->xti_flags |= XTI_DISCONMAIN;
				XTI_UNLOCK(&xtip->xtiso_lock);
				indication = T_DISCON_IND;
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=TS_WRES_CIND, T_DISCON_IND\n", xtip););
			}
#endif
			/* Check for lost unaccepted connections */
			seq = xtip->xti_seq;
			do {
			    if (seq->seq_used == XTIS_ACTIVE && seq->seq_so) {
				int unaccState =
				    sbpoll(seq->seq_so, &seq->seq_so->so_rcv);
				if (!(unaccState & SE_CONNOUT)) {
					(void) soclose(seq->seq_so);
					seq->seq_so = 0;
					seq->seq_used = XTIS_LOST;
					indication = T_DISCON_IND;
				}
			    }
			} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);
			break;

		default:
			indication = 0;
			XTITRACE(XTIF_EVENTS,
				printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=%d indication=%d\n",
				xtip, xtip->xti_state, indication););
			break;

		} /* switch */

		/*
		 * Return indication if any.
		 */
		if (indication)
			return (indication);

		/*
		 * look for a conn indication
		 */
		if ((state & SE_CONNIN) &&
		   ((xtip->xti_state == TS_IDLE) || 
		    (xtip->xti_state == TS_WRES_CIND)))  {
			if (!xtip->xti_lso)
				xti_panic("new_tp_ind: socket not listening");
			seq = xtip->xti_seq;
			do {
				if (seq->seq_used != XTIS_AVAILABLE)
					continue;
				seq->seq_used = XTIS_AWAITING;
				error = sodequeue(xtip->xti_lso, &seq->seq_so,
					  (struct mbuf **)0, SOCKNAME_FORMAT);
				XTITRACE(XTIF_EVENTS, if (error)
					printf(" xtiso: new_tp_ind(xtip=%x) - xti_state=%s, SE_CONNIN, sodequeue error=%d\n",
					xtip, xtip->xti_state == TS_IDLE ?
					    "TS_IDLE" : "TS_WRES_CIND",
					error););
				/*
				 * Tally an outstanding connect indication,
				 * even if it failed. Pass result later.
				 */
				xtip->xti_cindno++;
				return (T_CONN_IND);

			} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);
			/* We will be qenabled when a slot opens. */
		}

		/*
		 * look for a conn confirmation
		 */
		if ((state & SE_CONNOUT) && xtip->xti_state == TS_WCON_CREQ)
			return (T_CONN_CON);

		/*
		 * look for a data/exdata indication
		 */
		if ((xtip->xti_state == TS_DATA_XFER) ||
		    (xtip->xti_state == TS_WIND_ORDREL)) {
			if ((state & SE_HAVEOOB) || xtip->xti_rdata) {
				XTITRACE(XTIF_EVENTS, printf(" xtiso: new_tp_ind() - SE_HAVEOOB, T_EXDATA_IND\n"););
				return (T_EXDATA_IND);
			}

			if ((state & SE_HAVEDATA) || xtip->xti_rdata) {
				XTITRACE(XTIF_EVENTS, printf(" xtiso: new_tp_ind() - SE_HAVEDATA, T_DATA_IND\n"););
				return (T_DATA_IND);
			}
		}
		break;

	default:
		break;

	} /* switch */

	return (0);
#undef connected
}

/*
 * ==========================================
 * XTI service routines for TP user requests.
 * ==========================================
 *
 * q 	= Pointer to write queue
 * mp 	= Pointer to message block
 */

xti_info_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register xtiproto_t *xp;
	mblk_t *ack;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_info_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);

	XTITRACE(XTIF_INFO,
		printf(" xtiso: xti_info_req() - dump of xti_proto...\n");
		DumpPROTO(xp);
		printf("     current state: %d\n", xtip->xti_state););

	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < 
			sizeof(struct T_info_ack)) {
		if ((ack = allocb(sizeof(struct T_info_ack), BPRI_HI)) == 
			nilp(mblk_t)) {
			xti_bufcall(q, sizeof(struct T_info_ack), BPRI_HI, 0);
			putbq(q, mp);
			return;
		}
		freemsg(mp);
		mp = ack;
	}
	mp->b_datap->db_type	= M_PCPROTO;
	mp->b_rptr		= mp->b_datap->db_base;
	mp->b_wptr		= mp->b_rptr + sizeof(struct T_info_ack);
	pp			= (union T_primitives *)mp->b_rptr;

	pp->tinfoack.PRIM_type          = T_INFO_ACK;
	pp->tinfoack.TSDU_size          = xp->xp_tsdulen;
	pp->tinfoack.ETSDU_size         = xp->xp_etsdulen;
	pp->tinfoack.CDATA_size         = xp->xp_connectlen;
	pp->tinfoack.DDATA_size         = xp->xp_disconlen;
	pp->tinfoack.ADDR_size          = xp->xp_addrlen;
	pp->tinfoack.OPT_size           = xp->xp_optlen;
	pp->tinfoack.TIDU_size          = xp->xp_tidulen;
	pp->tinfoack.SERV_type          = xp->xp_servtype;
	pp->tinfoack.CURRENT_state      = xtip->xti_state;
	pp->tinfoack.PROVIDER_flag      = XPG4_1 | SENDZERO;

	qreply(q, mp);
}

xti_addr_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	register mblk_t   *new;
	struct   mbuf *peernam = 0;
	struct   mbuf *bndnam = 0;
	int	 error = 0, len;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_info_req");

	xtip = (struct xticb *)q->q_ptr;

	if (xtip->xti_state != TS_UNINIT && xtip->xti_state != TS_UNBND) {
		if (error = sogetaddr(xtip->xti_so,&bndnam,0,SOCKNAME_FORMAT)) {
			if (xti_snd_error_ack(q, mp, TSYSERR, error))
				strlog(XTI_INFO_ID, xtip->xti_minor, 0, 
				SL_TRACE|SL_ERROR,
				"xti_addr_req: couldn't get bound address\n");
			goto bad;
		}
	}
	if (xtip->xti_state == TS_DATA_XFER) {
		if (error =sogetaddr(xtip->xti_so,&peernam,1,SOCKNAME_FORMAT)) {
			if (xti_snd_error_ack(q, mp, TSYSERR, error))
				strlog(XTI_INFO_ID, xtip->xti_minor, 0, 
				SL_TRACE|SL_ERROR,
				"xti_addr_req: couldn't get peer address\n");
			goto bad;
		}
	}

	len = sizeof(struct T_addr_ack) + (bndnam ? bndnam->m_len : 0) +
		(peernam ? peernam->m_len : 0);
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < len) {
		if ((new = allocb(len, BPRI_HI)) == nilp(mblk_t)) {
			xti_bufcall(q, len, BPRI_HI, 0);
			putbq(q, mp);
			error = 1;
			goto bad;
		}
		freemsg(mp);
		mp = new;
	}

	mp->b_datap->db_type  = M_PCPROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->taddrack.PRIM_type = T_ADDR_ACK;
	if (bndnam) {
		pp->taddrack.LOCADDR_length = bndnam->m_len;
		pp->taddrack.LOCADDR_offset = sizeof(struct T_addr_ack);
		bcopy(mtod(bndnam, caddr_t), 
			(caddr_t)(mp->b_rptr + pp->taddrack.LOCADDR_offset), 
			(unsigned)bndnam->m_len);
	} else {
		pp->taddrack.LOCADDR_length = 0;
		pp->taddrack.LOCADDR_offset = 0;
	}
	if (peernam) {
		pp->taddrack.REMADDR_length = peernam->m_len;
		pp->taddrack.REMADDR_offset = sizeof(struct T_addr_ack) + 
						bndnam->m_len;
		bcopy(mtod(peernam, caddr_t), 
			(caddr_t)(mp->b_rptr + pp->taddrack.REMADDR_offset), 
			(unsigned)peernam->m_len);
	} else {
		pp->taddrack.REMADDR_length = 0;
		pp->taddrack.REMADDR_offset = 0;
	}
bad:
	if (bndnam)
		(void) m_free(bndnam);
	if (peernam)
		(void) m_free(peernam);
	if (error == 0)
		qreply(q, mp);
	return;
}

xti_bind_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	struct   mbuf *nam = 0;
	int	 newState;
	int      error = 0, len;

	CHECKPOINT("xti_bind_req");

	xtip = (struct xticb *)q->q_ptr;

	if (xtip->xti_wnam) 
		goto pendcall;

	pp  = (union T_primitives *)mp->b_rptr;

	if (xtip->xti_so == 0) {
		if (error = xti_entry_init(xtip, RD(q), XTI_NEWSOCK)) {
			if (xti_snd_error_ack(q, mp, TSYSERR, error))
				strlog(XTI_INFO_ID,xtip->xti_minor,0, 
					SL_TRACE|SL_ERROR,
				"xti_bind_req: failed to create socket\n");
			return;
		}
	}

	if ((newState = TNEXTSTATE(xtip, TE_BIND_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_bind_req: would place interface out of state\n");
		return;
	}

	XTITRACE(XTIF_BINDING,
		printf(" xtiso: original state=%d, next state=%d\n",
				xtip->xti_state, newState););

	if (((mp->b_wptr - mp->b_rptr) < (sizeof(struct T_bind_req))) ||
	    ((mp->b_wptr - mp->b_rptr) < (pp->tbindreq.ADDR_offset +
					 pp->tbindreq.ADDR_length))) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_bind_req: incorrect message size\n");
		return;
	}
		
	/* Automatic Address generator
	 *	 If we have an address, bind with that address. Otherwise,
	 *	 system will generate one.
	*/

	if (pp->tbindreq.ADDR_length != 0) {
		if (pp->tbindreq.ADDR_offset < sizeof(struct T_bind_req)) {
			if (xti_snd_error_ack(q, mp, TBADADDR, 0))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
					SL_TRACE|SL_ERROR,
				"xti_bind_req: incorrect address size\n");
			return;
		}
		if (pp->tbindreq.ADDR_length < 0 ||
	    	   (xtip->xti_proto.xp_addrlen == -2) ||
	    	   (xtip->xti_proto.xp_addrlen >= 0 &&
	     	    pp->tbindreq.ADDR_length != xtip->xti_proto.xp_addrlen)) {
			if (xti_snd_error_ack(q, mp, TBADADDR, 0))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
					SL_TRACE|SL_ERROR,
					"xti_bind_req: invalid address size\n");
			return;
		}

		XTITRACE(XTIF_BINDING,
			printf(" xtiso: pp->tbindreq.ADDR_length=%d\n",
					pp->tbindreq.ADDR_length););

		error = copy_to_mbuf(mp->b_rptr + pp->tbindreq.ADDR_offset,
			     pp->tbindreq.ADDR_length, MT_SONAME, &nam);
		switch(error) {
		case ENOBUFS:
			xti_bufcall(q, pp->tbindreq.ADDR_length, BPRI_HI, 
				XTI_TIMEOUT);
			putbq(q, mp);
			return;
		case EINVAL:
			if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
					SL_TRACE|SL_ERROR,
					"xti_bind_req: copy_to_mbuf failed\n");
			return;
		}
	}

	XTITRACE(XTIF_BINDING,
		printf(" xtiso: sobind(%x, %x)\n", xtip->xti_so, nam);
		DumpSO(xtip->xti_so););

	error = sobind(xtip->xti_so, nam);

	XTITRACE(XTIF_BINDING,
		if (error) printf(" xtiso: sobind error=%d\n", error););

	if (nam)
		(void) m_free(nam);
	nam = 0;

	switch (error) {
	case 0:
		/*
	 	 * "getsockname",
		 * save the bound address in xtip->xti_nam for future use
	 	 */
		if (error = sogetaddr(xtip->xti_so, &xtip->xti_nam, 0, 
				SOCKNAME_FORMAT)) {
			if (xtip->xti_nam)
				(void)m_free(xtip->xti_nam);
			xtip->xti_nam = 0;
			xti_finished(q, XTI_NOSOCK);
			xti_snd_error_ack(q, mp, TBADADDR, 0);
			return;
		}
		xtip->xti_state = newState;
		SOCKET_LOCK(xtip->xti_so);
		xtip->xti_so->so_state &= ~SS_PRIV;
		SOCKET_UNLOCK(xtip->xti_so);
		break;
	case EADDRNOTAVAIL:
		xti_snd_error_ack(q, mp, TBADADDR, 0);
		return;
	case EADDRINUSE:
		xti_snd_error_ack(q, mp, TADDRBUSY, 0);
		return;
	case EACCES:
		xti_snd_error_ack(q, mp, TACCES, 0);
		return;
	default:
		if (pp->tbindreq.ADDR_length == 0)
			xti_snd_error_ack(q, mp, TNOADDR, 0);
		else
			xti_snd_error_ack(q, mp, TSYSERR, error);
		return;
	}

	xtip->xti_qlen  = (pp->tbindreq.CONIND_number > XTI_MAXQLEN) ?
			 XTI_MAXQLEN : pp->tbindreq.CONIND_number;
pendcall:
	len = sizeof(struct T_bind_ack) + xtip->xti_nam->m_len;
	if (!mp || (mp->b_datap->db_lim - mp->b_datap->db_base) < len) {
		mblk_t	*mpp;
		if (!(mpp = allocb(len, BPRI_HI))) {
			xti_bufcall(q, len, BPRI_HI, 0);
			xtip->xti_wnam = xtip->xti_nam;
			putbq(q, mp);
			return;
		}
		if (mp)
			freemsg(mp);
		mp = mpp;
	}

	/*
	 * If qlen is greater than zero, enable listens on this socket
	 */
	if (xtip->xti_qlen > 0 && xti_listen_req(q, mp) != 0) {
		if (xtip->xti_nam)
			(void) m_free(xtip->xti_nam);
		xtip->xti_nam = xtip->xti_wnam = 0;
		return;
	}

	mp->b_datap->db_type  = M_PCPROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr+sizeof(struct T_bind_ack)+ xtip->xti_nam->m_len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->tbindack.PRIM_type     = T_BIND_ACK;
	pp->tbindack.ADDR_length   = xtip->xti_nam->m_len;
	pp->tbindack.ADDR_offset   = sizeof (struct T_bind_ack);
	pp->tbindack.CONIND_number = xtip->xti_qlen;
	bcopy(mtod(xtip->xti_nam, caddr_t),
		(caddr_t)(mp->b_rptr + pp->tbindack.ADDR_offset),
		(unsigned)xtip->xti_nam->m_len);
	XTITRACE(XTIF_BINDING,
		printf("xti_bind_req() tbindack fields:\n");
		printf("   PRIM_type: 0x%08x\n", pp->tbindack.PRIM_type);
		printf("   ADDR_leng: 0x%08x\n", pp->tbindack.ADDR_length);
		printf("   ADDR_offs: 0x%08x\n", pp->tbindack.ADDR_offset);
		printf("   CONIND_nu: 0x%08x\n", pp->tbindack.CONIND_number);
	);

	xtip->xti_wnam = 0;
	NEXTSTATE(xtip, TE_BIND_ACK, "xti_bind_req");
	qreply(q, mp);
	return;
}

xti_unbind_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	int newState;

	CHECKPOINT("xti_unbind_req");

	xtip = (struct xticb *)q->q_ptr;
	if (!mp)
		goto pendcall;

	if ((newState = TNEXTSTATE(xtip, TE_UNBIND_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_unbind_req: would place interface out of state\n");
		return 0;
	}
	xtip->xti_state = newState;

	if (!xti_snd_flushrw(q, FLUSHRW)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EPROTO))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_unbind_req: xti_snd_flushrw failed\n");
		return 0;
	}

pendcall:
	XTITRACE(XTIF_BINDING,
	    printf(" xtiso: xti_unbind_req() - xti_snd_ok_ack(q=%x mp=%x)\n",
		q, mp););

	if (!xti_snd_ok_ack(q, mp)) {
		xtip->xti_pendcall = xti_unbind_req;
		return 0;
	}

	if (xtip->xti_nam)
		(void) m_free(xtip->xti_nam);
	xtip->xti_nam = 0;

	NEXTSTATE(xtip, TE_OK_ACK1, "xti_unbind_req");
	if (xti_finished(q, XTI_NOSOCK) == 0)
		xti_snd_error_ack(q, mp, TSYSERR, EPROTO);
	return 1;
}

/*
	Option Management Support:

	XPG4 says:

	1. Options are read-only in TS_UNBND state.
	2. If multiple options are specified on input and any option level is
	   different than first option level, then fail with TBADOPT.
	3. If the option level is unknown or unsupported, fail with TBADOPT.
	4. If the option name is unknown or unsupported, return T_NOTSUPPORT
	   and if the negotiation fails return T_FAILURE in status field and 
	   does not cause failure.
	5. If t_opthdr.len exceeds the remaining size of buffer, fail TBADOPT.
	6. If the option value is illegal or if the multiple options are
	   specified and one has illegal value, fail with TBADOPT.
	   what is illegal value ?
	7. If the flags is T_NEGOTIATE and option name is T_ALLOPT, then
	   use the default value to negotiate.
	8. If the provider doesn't capable of supporting T_NEGOTIATE/T_CHECK
	   functionalities, return with TNOTSUPPORT error.

	TPI says:

	9. If the flags is T_NEGOTIATE and OPT_length is 0, then set and
	   return the default options associated with the Stream.

	XXX -- 

	10.If the flags is T_NEGOTIATE and no value to negotiate, then
	   fail with TBADOPT.
	11.The return value of MGMT_flags contains the original value of
	   the MGMT_flags OR'ed with the worst single result of the option
	   status.

*/

xti_optmgmt_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	int	newState;
	int	optlen, origlen, reqlen, newlen = 0;
	int	error  = 0;
	long	flags, level, iflags;
	char	*stopt, *rtopt;
	register struct t_opthdr *optreq, *optack;
	register mblk_t *new;

	CHECKPOINT("xti_optmgmt_req");

	pp  = (union T_primitives *)mp->b_rptr;
	xtip = (struct xticb *)q->q_ptr;

	if ((newState = TNEXTSTATE(xtip, TE_OPTMGMT_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_optmgmt_req:would place interface out of state\n");
		return;
	}

	if (((mp->b_wptr - mp->b_rptr) < (sizeof(struct T_optmgmt_req))) ||
	    ((mp->b_wptr - mp->b_rptr) < (pp->toptmgmtreq.OPT_offset +
					 pp->toptmgmtreq.OPT_length))) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_optmgmt_req: incorrect message size\n");
		return;
	}
	if (xtip->xti_proto.xp_optlen == -2) {
		if (xti_snd_error_ack(q, mp, TNOTSUPPORT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_optmgmt_req: options not supported\n");
		return;
	}
	if ((xtip->xti_proto.xp_optlen >= 0 &&
	     pp->toptmgmtreq.OPT_length > xtip->xti_proto.xp_optlen)  ||
	     pp->toptmgmtreq.OPT_offset < sizeof(struct T_optmgmt_req)) { 
		if (xti_snd_error_ack(q, mp, TBADOPT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_optmgmt_req: invalid options size\n");
		return;
	}

	flags = pp->optmgmt_req.MGMT_flags;
	if (flags != T_NEGOTIATE && flags != T_DEFAULT && flags != T_CURRENT &&
		flags != T_CHECK) {
		if (xti_snd_error_ack(q, mp, TBADFLAG, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_optmgmt_req: invalid flag specified\n");
		return;
	}
		
	if (xtip->xti_state == TS_UNBND)
		flags = T_DEFAULT;

	optlen = xtip->xti_proto.xp_optlen > 0 ? xtip->xti_proto.xp_optlen : 0;
	stopt = (char *)mp->b_rptr + sizeof(struct T_optmgmt_req);

	/*
	 * if optlen == 0 do we support no options ??
	 * XXX - is this possible to happen ??
	 */
	if (optlen == 0) {
		if (xti_snd_error_ack(q, mp, TNOTSUPPORT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_optmgmt_req: options not supported\n");
		return;
	}

	origlen = pp->toptmgmtreq.OPT_length;
	if (origlen && (optreq = (struct t_opthdr *)stopt)) {
		level = optreq->level;
		/* If there is no value for negotiation, fail */
		if (flags == T_NEGOTIATE && optreq->name != T_ALLOPT && 
			optreq->len < (sizeof(struct t_opthdr) + sizeof(long)))
			goto badopt;
	}

	if ((new = allocb(sizeof(struct T_optmgmt_ack) + optlen, BPRI_HI)) 
		== nilp(mblk_t)) {
		xti_bufcall(q, sizeof(struct T_optmgmt_ack)+optlen, BPRI_HI, 0);
		putbq(q, mp);
		return;
	}

	new->b_datap->db_type = M_PCPROTO;
	new->b_rptr = new->b_datap->db_base;
	new->b_wptr = new->b_rptr + sizeof(struct T_optmgmt_ack) + optlen;
	pp = (union T_primitives *)new->b_rptr;
	pp->toptmgmtack.PRIM_type  = T_OPTMGMT_ACK;
	pp->toptmgmtack.OPT_length = origlen;
	pp->toptmgmtack.OPT_offset = sizeof(struct T_optmgmt_ack);
	pp->toptmgmtack.MGMT_flags = iflags = flags;
	optack = (struct t_opthdr *)(new->b_rptr + pp->toptmgmtack.OPT_offset);
	rtopt = (char *)new->b_rptr + sizeof(struct T_optmgmt_ack);

	if (origlen == 0) {		/* always T_DEFAULT and T_ALLOPT */
		flags = T_DEFAULT;
		if (!(xtip->xti_flags & XTI_OPTINIT))
			xti_init_default_options(xtip);

		/* get the XTI-LEVEL options */
		error = xti_xti_optmgmt(0, optack, xtip, &flags, 0, optlen);
		iflags = flags;
		flags = T_DEFAULT;
		newlen += -error;
		optack = (struct t_opthdr *)((char *)optack + (-error));

		if (xtip->xti_proto.xp_dom == AF_INET) {
			/* get the IP-LEVEL options */
			error = xti_ip_optmgmt(0, optack, xtip,&flags,0,optlen);
			iflags = xti_opt_flags(flags, iflags);
			flags = T_DEFAULT;
			newlen += -error;
			optack = (struct t_opthdr *)((char *)optack + (-error));

			/* get the TCP/UDP LEVEL options */
			if (xtip->xti_proto.xp_type == SOCK_STREAM)
				error = xti_tcp_optmgmt(0, optack, xtip, &flags,
								0, optlen);
			else
				error = xti_udp_optmgmt(0, optack, xtip,&flags);
			iflags = xti_opt_flags(flags, iflags);
			newlen += -error;
		}
	} else {
		reqlen = origlen;	/* reuse the optlen */
		for (optreq = (struct t_opthdr *)stopt; optreq != NULL;
			optreq = OPT_NEXTHDR(stopt, origlen, optreq)) {

			if (optreq->level != level || optreq->len > reqlen) 
				goto badopt;

			reqlen -= optreq->len;	/* remaining size of buffer */

			/* XPG4 says, T_ALLOPT cannot be used with T_CHECK */
			if (flags == T_CHECK && optreq->name == T_ALLOPT)
				goto badopt;

			/* If no value to negotiate, then return error */
			if (flags == T_NEGOTIATE && optreq->name != T_ALLOPT &&
				(optreq->len < (sizeof(struct t_opthdr) + 
					sizeof(long))))
				goto badopt;

			bcopy(optreq, optack, optreq->len);

			error = xti_chk_protocol(optreq, optack, xtip, &flags,
							optlen);

			if (error > 0)		/* actual error */
				goto badopt;
			if (error < 0) {	/* T_ALLOPT, exit loop */
				newlen += -error;
				iflags = flags; /* single worst status value */
				break;
			}
			iflags = xti_opt_flags(optack->status, iflags);
			newlen += optack->len;
			optack = OPT_NEXTHDR(rtopt, optlen, optack);
			if (!optack)
				break;
		}
	}
	/* single worst result of option status OR'ed with original flag */
	pp->optmgmt_ack.MGMT_flags = iflags;

	if (newlen) {
		pp->toptmgmtack.OPT_length = newlen;
		new->b_wptr = new->b_rptr + pp->toptmgmtack.OPT_offset + newlen;
	}
	if (xtip->xti_state == TS_UNBND)
		pp->optmgmt_ack.MGMT_flags = T_READONLY;

	xtip->xti_state = newState;
	NEXTSTATE(xtip, TE_OPTMGMT_ACK, "xti_optmgmt_req");
	if (mp)
		freemsg(mp);
	qreply(q, new);
	return 0;
badopt:
	if (new)
		freemsg(new);
	if (xti_snd_error_ack(q, mp, TBADOPT, 0))
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_optmgmt_req: invalid option\n");
	return;
}

xti_listen_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	int error = 0;

	CHECKPOINT("xti_listen_req");

	xtip = (struct xticb *)q->q_ptr;
	if ((xtip->xti_proto.xp_servtype != T_COTS_ORD) &&
	    (xtip->xti_proto.xp_servtype != T_COTS))
		/*
		 * Not a connection-oriented TP so ignore listen request
		 */
		return(0);

	if (xtip->xti_state != TS_WACK_BREQ) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_listen_req: would place interface out of state\n");
		return (TOUTSTATE);
	}

	if (xtip->xti_qlen <= 0) {
		if (xti_snd_error_ack(q, mp, TBADQLEN, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_listen_req: qlen must be greater than zero\n");
		return(TBADQLEN);
	}

	XTITRACE(XTIF_BINDING,
		printf(" xtiso: xti_listen_req() - solisten(so=%x qlen=%d)\n",
			xtip->xti_so, xtip->xti_qlen););

	/* Use the xti qlen, solisten will fix if too big. */
	if (error = solisten(xtip->xti_so, xtip->xti_qlen)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, error))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_listen_req: solisten error status %d\n", error);
		return(error);
	}

	/*
	 * Preserve listening socket
	 */
	xtip->xti_lso = xtip->xti_so;

	return(0);
}

xti_conn_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	struct   mbuf *nam = 0;
	int size, error = 0;
	int newState;
	register struct t_opthdr *opt, *opta;
	char	*stopt;
	long	flags = T_NEGOTIATE;

	CHECKPOINT("xti_conn_req");

	xtip    = (struct xticb *)q->q_ptr;
	if (!mp)
		goto pendcall;
	pp     = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_CONN_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: would place interface out of state\n");
		return 0;
	}
	if (xtip->xti_so == 0) {
		if (error = xti_entry_init(xtip, RD(q), XTI_NEWSOCK)) {
			if (xti_snd_error_ack(q, mp, TSYSERR, error))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
				SL_TRACE|SL_ERROR,
				"xti_conn_req: failed to create socket\n");
			return 0;
		}
	}
	xtip->xti_state = newState;

	/* 	If qlen > 0, Turn off the listening option and socket.
		No need to remove accepted socket (seq->seq_so). if we have
		that, above state checking test will fail !
		xti_so and xti_lso has to be always same !!!
	*/
	if (xtip->xti_lso != 0) {
		SOCKET_LOCK(xtip->xti_so);
		xtip->xti_so->so_options &= ~SO_ACCEPTCONN;
		xtip->xti_so->so_qlimit = 0;
		xtip->xti_lso = 0;
		SOCKET_UNLOCK(xtip->xti_so);
	}

	if ((xtip->xti_proto.xp_servtype != T_COTS_ORD) &&
	    (xtip->xti_proto.xp_servtype != T_COTS)) {
		if (xti_snd_error_ack(q, mp, TNOTSUPPORT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: primitive not supported by provider\n");
		return 0;
	}

	if ((size = mp->b_wptr - mp->b_rptr) < sizeof(struct T_conn_req) ||
	    size < (pp->tconnreq.DEST_length + pp->tconnreq.DEST_offset) ||
	    size < (pp->tconnreq.OPT_length + pp->tconnreq.OPT_offset)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: incorrect message size\n");
		return 0;
	}
	if (pp->tconnreq.DEST_length < 0 ||
	    (pp->tconnreq.DEST_length && xtip->xti_proto.xp_addrlen == -2) ||
	    (xtip->xti_proto.xp_addrlen >= 0 &&
	     pp->tconnreq.DEST_length != xtip->xti_proto.xp_addrlen)) {
		if (xti_snd_error_ack(q, mp, TBADADDR, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid dest address\n");
		return 0;
	}

	if (pp->tconnreq.OPT_length < 0 ||
	    (pp->tconnreq.OPT_length && xtip->xti_proto.xp_optlen == -2) ||
	    (xtip->xti_proto.xp_optlen >= 0 && 
	     pp->tconnreq.OPT_length > xtip->xti_proto.xp_optlen)) {
		if (xti_snd_error_ack(q, mp, TBADOPT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid option size\n");
		return 0;
	}

	size = msgdsize(mp);
	if ((size && xtip->xti_proto.xp_connectlen == -2) ||
	    (xtip->xti_proto.xp_connectlen >= 0 &&
	     size > xtip->xti_proto.xp_connectlen)) {
		if (xti_snd_error_ack(q, mp, TBADDATA, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid connection data size\n");
		return 0;
	}
	error = copy_to_mbuf(mp->b_rptr + pp->tconnreq.DEST_offset,
			     pp->tconnreq.DEST_length, MT_SONAME, &nam);
	switch (error) {
	case ENOBUFS:
		xti_bufcall(q, pp->tbindreq.ADDR_length, BPRI_HI, XTI_TIMEOUT);
		putbq(q, mp);
		return 0;
	case EINVAL:
		if (xti_snd_error_ack(q, mp, TSYSERR, ENOMEM))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_req: copy_to_mbuf failed\n");
		return 0;
	}

	if (pp->tconnreq.OPT_length > 0) {
		/* save the option, xti_conn_con() will return that */
		if (!(xtip->xti_opt = 
			xmalloc(pp->tconnreq.OPT_length, 3, pinned_heap))) {
			if (xti_snd_error_ack(q, mp, TSYSERR, ENOMEM))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
					SL_TRACE|SL_ERROR, 
				    "xti_conn_req: option allocation failed\n");
			return 0;
		}
			
		xtip->xti_optlen = pp->tconnreq.OPT_length;
		stopt = (char *)mp->b_rptr + pp->tconnreq.OPT_offset;
		opta = (struct t_opthdr *)xtip->xti_opt;

		for (opt = (struct t_opthdr *)stopt; opt != NULL;
		     opt = OPT_NEXTHDR(stopt, pp->tconnreq.OPT_length, opt)) {

			if (opt->len > pp->tconnreq.OPT_length ||
				opt->len == sizeof(struct t_opthdr)) 
				goto badopt;

			if (opt->name == T_ALLOPT || opt->level == INET_UDP)
				continue;

			bcopy(opt, opta, opt->len);

			if (opt->level == XTI_GENERIC)
				error = xti_xti_optmgmt(opt, opta, xtip,
							&flags, 0, 0);
			if (opt->level == INET_IP)
				error = xti_ip_optmgmt(opt, opta, xtip, 
							&flags, 0, 0);
			if (opt->level == INET_TCP)
				error = xti_tcp_optmgmt(opt, opta, xtip,
							&flags, 0, 0);
			if (error)
				goto badopt;

		     	opta = OPT_NEXTHDR(xtip->xti_opt,xtip->xti_optlen,opta);
			if (!opta)
				break;
		}
		/* if no option processing, then free the buffer */
		if (xtip->xti_opt && (char *)opta == xtip->xti_opt) {
			(void)xmfree(xtip->xti_opt, pinned_heap);
			xtip->xti_opt = xtip->xti_optlen = 0;
		}
	}
	XTITRACE(XTIF_CONNECT,
		printf(" xtiso: xti_conn_req() - about to soconnect(), so=%x, nam=%x\n",
			xtip->xti_so, nam););

	error = soconnect(xtip->xti_so, nam);

	XTITRACE(XTIF_CONNECT,
		printf(" xtiso: xti_conn_req() - soconnect error=%d. (%x)\n",
			error, error););

	(void) m_free(nam);
	switch (error) {
	case 0:
		SOCKET_LOCK(xtip->xti_so);	
		xtip->xti_so->so_state &= ~SS_PRIV;
		SOCKET_UNLOCK(xtip->xti_so);
		break;

	case EADDRNOTAVAIL:
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TBADADDR 0)\n", q, mp););
		xti_snd_error_ack(q, mp, TBADADDR, 0);
		return 0;

	case EADDRINUSE:
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TADDRBUSY 0)\n", q, mp););
		xti_snd_error_ack(q, mp, TADDRBUSY, 0);
		return 0;

	case EACCES:
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TACCES 0)\n", q, mp););
		xti_snd_error_ack(q, mp, TACCES, 0);
		return 0;

	case EISCONN:
	case ENOTCONN:
	case ESHUTDOWN:
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TOUTSTATE 0)\n", q, mp););
		xti_snd_error_ack(q, mp, TOUTSTATE, 0);
		return 0;

	default:
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TSYSERR error=%d)\n", q, mp, error););
		xti_snd_error_ack(q, mp, TSYSERR, error);
		return 0;
	}
	NEXTSTATE(xtip, TE_OK_ACK1, "xti_conn_req");

pendcall:
	XTITRACE(XTIF_CONNECT,
		printf(" xtiso: xti_conn_req() - xti_snd_ok_ack(q=%x mp=%x)\n",
			q, mp););
	if (!xti_snd_ok_ack(q, mp)) {
		xtip->xti_pendcall = xti_conn_req;
		return 0;
	}
	return 1;
badopt:
	if (xti_snd_error_ack(q, mp, TBADOPT, 0))
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid option\n");
	return 0;
}

xti_conn_res(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register struct xticb *tmp = nilp(struct xticb);
	struct xtiseq *seq;
	queue_t *connq;
	int size, error = 0;
	int newState;
	register struct t_opthdr *opt;
	char *stopt;
	long flags = T_NEGOTIATE;

	CHECKPOINT("xti_conn_res");

	xtip   = (struct xticb *)q->q_ptr;
	if (!mp)
		goto pendcall;

	pp     = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_CONN_RES)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_res: would place interface out of state\n");
		return 0;
	}

	if ((xtip->xti_proto.xp_servtype != T_COTS_ORD) &&
	    (xtip->xti_proto.xp_servtype != T_COTS)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EOPNOTSUPP))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_conn_res: primitive not supported by provider\n");
		return 0;
	}

	if ((size = mp->b_wptr - mp->b_rptr) < sizeof(struct T_conn_res) ||
	    size < (pp->tconnres.OPT_length + pp->tconnres.OPT_offset)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: incorrect message size\n");
		return 0;
	}
	if (pp->tconnres.OPT_length < 0 ||
	    (pp->tconnres.OPT_length && xtip->xti_proto.xp_optlen == -2) ||
	    (xtip->xti_proto.xp_optlen >= 0 &&
	     pp->tconnres.OPT_length > xtip->xti_proto.xp_optlen)) {
		if (xti_snd_error_ack(q, mp, TBADOPT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: incorrect option size\n");
		return 0;
	}

	size = msgdsize(mp);
	if ((size && xtip->xti_proto.xp_connectlen == -2) ||
	    (xtip->xti_proto.xp_connectlen >= 0 &&
	     size > xtip->xti_proto.xp_connectlen)) {
		if (xti_snd_error_ack(q, mp, TBADDATA, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: invalid connect data size\n");
		return 0;
	}

	seq = xtip->xti_seq;
	do {
		if (seq->seq_used == XTIS_ACTIVE &&
		    pp->tconnres.SEQ_number == seq->seq_no)
			    break;
	} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);

	if (seq >= &xtip->xti_seq[XTI_MAXQLEN]) {
		if (xti_snd_error_ack(q, mp, TBADSEQ, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: invalid sequence number\n");
		return 0;
	}

#if	XTIDEBUG
	if (seq->seq_so == 0)
		xti_panic("xti_conn_res seq_so");
	if (xtip->xti_cindno <= 0)
		xti_panic("xti_conn_res cindno");
#endif

	/* set options */
	if (pp->tconnres.OPT_length > 0) {
		stopt = (char *)mp->b_rptr + pp->tconnres.OPT_offset;
		for (opt = (struct t_opthdr *)stopt; opt != NULL;
			opt = OPT_NEXTHDR(stopt, pp->tconnres.OPT_length,opt)) {

			if (opt->len > pp->tconnreq.OPT_length ||
				opt->len == sizeof(struct t_opthdr))
				goto badopt;
			if (opt->name == T_ALLOPT)
				continue;

			if (opt->level == XTI_GENERIC)
				error = xti_xti_optmgmt(opt, opt, xtip,
						&flags, seq->seq_so, 0);
			if (opt->level == INET_IP)
				error = xti_ip_optmgmt(opt, opt, xtip, 
						&flags, seq->seq_so, 0);
			if (opt->level == INET_TCP)
				error = xti_tcp_optmgmt(opt, opt, xtip,
						&flags, seq->seq_so, 0);

			if (error) {
badopt:
				xti_snd_error_ack(q, mp, TBADOPT, 0);
				return 0;
			}
		}
	}

	xtip->xti_state = newState;

	connq = (queue_t *)(pp->tconnres.QUEUE_ptr);
	if (connq == RD(q)) {
		if (xtip->xti_cindno > 1) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: must respond to other rcvd conn indications first\n");
			xti_snd_error_ack(q, mp, TINDOUT, 0);
			return 0;
		}
		/*
		 * Connection response on same stream
		 * as connection request
		 *	. do something with listening socket
		 *	. make socket carrying connect request the
		 *	  active socket for now
		 * XXX
		 * Saving the listening socket would be nice but if we do,
		 * it will continue to accept connections. We need to be
		 * sure we can continue to accept on it... is this true?
		 * Otherwise, close the thing. Would an application do this
		 * accept on a connection with qlen > 1 anyway?
		 */
		xtip->xti_lso = xtip->xti_so;
		xtip->xti_so  = seq->seq_so;
		NEXTSTATE(xtip, TE_OK_ACK2, "xti_conn_res 1");

		/*
		 * Setup socket to make things work between sockets/xtiso
		 */
		xti_init_socket(xtip->xti_so, connq);
	} else {
		tmp = (struct xticb *)connq->q_ptr;
		/*
		 * Grab pointer to "other queue's" xti context block
		 */
		if (tmp == nilp(struct xticb) ||
				OTHERQ(connq)->q_qinfo->qi_putp != xtiwput) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: invalid resfd\n");
			xti_snd_error_ack(q, mp, TBADF, 0);
			return 0;
		}

		if (xtip->xti_proto.xp_dom != tmp->xti_proto.xp_dom || 
		    xtip->xti_proto.xp_type != tmp->xti_proto.xp_type) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: invalid transport provider\n");
			xti_snd_error_ack(q, mp, TPROVMISMATCH, 0);
			return 0;
		}

		/* XPG4 says, no need to bind new socket for accept */
		if (tmp->xti_state == TS_UNBND)
			tmp->xti_state = TS_IDLE;

		if (TNEXTSTATE(tmp, TE_PASS_CONN) == TS_BAD_STATE) {
			xti_snd_error_ack(q, mp, TOUTSTATE, 0);
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			     "xti_conn_res: would place resfd out of state\n");
			return 0;
		}

		/*
		 * Connection response on new stream;
		 * ie. different stream from that on which
		 * connection request arrived
		 */
		SOCKET_LOCK(tmp->xti_so);
		tmp->xti_so->so_snd.sb_wakeup  = 0;
		tmp->xti_so->so_rcv.sb_wakeup  = 0;
		SOCKET_UNLOCK(tmp->xti_so);
		(void) soclose (tmp->xti_so);
		tmp->xti_lso = 0;
		tmp->xti_so = seq->seq_so;
		NEXTSTATE(tmp, TE_PASS_CONN, "xti_conn_res 2");

		if (xtip->xti_cindno == 1)
			NEXTSTATE(xtip, TE_OK_ACK3, "xti_conn_res 3");
		else
			NEXTSTATE(xtip, TE_OK_ACK4, "xti_conn_res 4");

		/*
		 * Setup socket to make things work between sockets/xtiso
		 */
		xti_init_socket(tmp->xti_so, connq);
	}
	if (xtip->xti_cindno-- == XTI_MAXQLEN)
		qenable(xtip->xti_rq);
	seq->seq_used  = XTIS_AVAILABLE;
	seq->seq_so    = 0;
pendcall:
	XTITRACE(XTIF_CONNECT,
	  printf(" xtiso: xti_conn_res() - xti_snd_ok_ack(q=%x mp=%x)\n", q, mp););

	/* save the tmp, if we come thru' pendcall, will use it */
	xtip->xti_opt = (char *)tmp;
	if (!xti_snd_ok_ack(q, mp)) {
		xtip->xti_pendcall = xti_conn_res;
		return 0;
	}
	if (!mp)
		tmp = (struct xticb *)xtip->xti_opt;

	/* XXX - check whether any indication is already queued or not */
	if (tmp == nilp(struct xticb)) 
		tmp = xtip;
	if ((tmp->xti_so->so_state & SS_RCVATMARK)
		|| (tmp->xti_so->so_rcv.sb_cc)
		|| (!(tmp->xti_so->so_state & SS_ISCONNECTED)) 
		|| (tmp->xti_so->so_state & SS_CANTRCVMORE))
		qenable(connq);

	xtip->xti_opt = 0;
	return 1;
}

xti_discon_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	struct   xtiseq *seq;
	int      size, error = 0;
	int 	 savedState;
	int 	 serviceType;
	int	 newState;

	CHECKPOINT("xti_discon_req");

	pp = (union T_primitives *)mp->b_rptr;
	xtip = (struct xticb *)q->q_ptr;

	savedState = xtip->xti_state;
	serviceType = xtip->xti_proto.xp_servtype;
	if ((newState = TNEXTSTATE(xtip, TE_DISCON_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: would place interface out of state\n");
		xti_snd_error_ack(q, mp, TOUTSTATE, 0);
		return;
	}

	if ((serviceType != T_COTS_ORD) && (serviceType != T_COTS)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req:primitive not supported by provider\n");
		xti_snd_error_ack(q, mp, TNOTSUPPORT, 0);
		return;
	}

	if (mp->b_wptr - mp->b_rptr < sizeof(struct T_discon_req)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: incorrect message size\n");
		xti_snd_error_ack(q, mp, TSYSERR, EINVAL);
		return;
	}

	size = msgdsize(mp);
	if ((size && xtip->xti_proto.xp_disconlen == -2) ||
	    (xtip->xti_proto.xp_disconlen >= 0 &&
	     size > xtip->xti_proto.xp_disconlen)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: invalid discon data size\n");
		xti_snd_error_ack(q, mp, TBADDATA, 0);
		return;
	}

	if (xtip->xti_cindno) {
		seq = xtip->xti_seq;
		do {
			if (seq->seq_used == XTIS_ACTIVE &&
			    pp->tdisconreq.SEQ_number == seq->seq_no)
				break;
		} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);

		if (seq >= &xtip->xti_seq[XTI_MAXQLEN]) {
			strlog(XTI_INFO_ID,xtip->xti_minor,0, SL_TRACE|SL_ERROR,
				"xti_discon_req: invalid sequence number\n");
			xti_snd_error_ack(q, mp, TBADSEQ, 0);
			return;
		}
	} else if (pp->tdisconreq.SEQ_number != -1) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req:invalid sequence number:no conn ind\n");
		xti_snd_error_ack(q, mp, TBADSEQ, 0);
		return;
	}

	if (pp->tdisconreq.SEQ_number != -1) {
		/*
		 * "reject" pending connect request
		 */
		if (seq->seq_so) {
			(void) soabort (seq->seq_so);
			(void) soclose (seq->seq_so);
		}
		seq->seq_so = 0;
		seq->seq_used = XTIS_AVAILABLE;
	} else {
		/* After t_snddis(), the state will be T_IDLE and so
		   t_connect() and t_listen() both should work.
		*/
		if (error = xti_startup(q, 0, xtip->xti_qlen))
			goto bad;
	}
	xtip->xti_state = newState;

	if (xtip->xti_cindno == 0)
		NEXTSTATE(xtip, TE_OK_ACK1, "xti_discon_req 1");
	else {
		if (xtip->xti_cindno == 1)
			NEXTSTATE(xtip, TE_OK_ACK2, "xti_discon_req 2");
		else
			NEXTSTATE(xtip, TE_OK_ACK4, "xti_discon_req 3");
		if (xtip->xti_cindno-- == XTI_MAXQLEN)
			qenable(xtip->xti_rq);
	}

	if ((savedState == TS_DATA_XFER || savedState == TS_WIND_ORDREL)  &&
	    (serviceType == T_COTS_ORD  || serviceType == T_COTS))
		if (!xti_snd_flushrw(q, FLUSHRW)) {
			xti_snd_error_ack(q, mp, TSYSERR, EPROTO);
			return;
		}

	XTITRACE(XTIF_CONNECT,
	 printf(" xtiso: xti_discon_req() - xti_snd_ok_ack(q=%x mp=%x)\n", q, mp););
	if (!xti_snd_ok_ack(q, mp))
		xtip->xti_pendcall = xti_snd_ok_ack;
	return;

bad:	
	xti_snd_error_ack(q, mp, TSYSERR, error);
}

xti_data_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register xtiproto_t *xp;
	mblk_t   *mp1;
	int      more;
	int	 newState;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_data_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);
	pp  = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_DATA_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_data_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if (mp->b_datap->db_type != M_DATA) {
		if ((mp->b_wptr - mp->b_rptr) < sizeof(struct T_data_req)) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_data_req: incorrect message size\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	}
	if (xp->xp_tsdulen == -2) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_data_req: cannot send normal data\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	/*
	 * If this TE supports TSDU, then check the "MORE data"
	 * user flag and either link data block or pass it
	 * on, accordingly.
	 */
	if (xp->xp_tsdulen != 0) {
		more = pp->tdatareq.MORE_flag;
		xtip->xti_tsdu += msgdsize(mp);
		if (xp->xp_tsdulen > 0 && xtip->xti_tsdu > xp->xp_tsdulen) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_data_req: TSDU size exceeded\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	} else
		more = 0;

	xtip->xti_state = newState;

	if (mp->b_datap->db_type != M_DATA) {
		mp1 = unlinkb(mp);
		freemsg(mp);
	} else
		mp1 = mp;

	if (xtip->xti_wdata)
		linkb (xtip->xti_wdata, mp1);
	else
		xtip->xti_wdata = mp1;

	/*
	 * If user specified more data to come for
	 * this tsdu, then don't send it out yet, link
	 * it onto the end of any queued data blocks
	 * for this TE.
	 */
	if (!more) {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags &= ~XTI_MOREDATA;
		XTI_UNLOCK(&xtip->xtiso_lock);
		xtip->xti_tsdu = 0;
		if (xti_canput(xtip->xti_wq , msgdsize(xtip->xti_wdata)) >= 0)
			(void) xti_send(xtip, T_DATA_REQ);
	} else {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags |= XTI_MOREDATA;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}
}

xti_exdata_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register xtiproto_t *xp;
	mblk_t   *mp1;
	int      more;
	int	 newState;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_exdata_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);
	pp  = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_EXDATA_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_exdata_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if (mp->b_datap->db_type != M_DATA) {
		if ((mp->b_wptr - mp->b_rptr) < sizeof(struct T_exdata_req)) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_exdata_req: incorrect message size\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	}
	if (xp->xp_etsdulen == -2) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_exdata_req: cannot send expedited data\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if (xp->xp_etsdulen != 0) {
		more = pp->texdatareq.MORE_flag;
		xtip->xti_etsdu += msgdsize(mp);
		if (xp->xp_etsdulen > 0 && xtip->xti_etsdu > xp->xp_etsdulen) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_exdata_req: ETSDU size exceeded\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	} else
		more = 0;

	xtip->xti_state = newState;

	if (mp->b_datap->db_type != M_DATA) {
		mp1 = unlinkb(mp);
		freemsg(mp);
	} else
		mp1 = mp;

	if (xtip->xti_wexdata)
		linkb (xtip->xti_wexdata, mp1);
	else
		xtip->xti_wexdata = mp1;

	if (!more) {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags &= ~XTI_MOREEXDATA;
		XTI_UNLOCK(&xtip->xtiso_lock);
		xtip->xti_etsdu = 0;
		if (xti_canput(xtip->xti_wq,
					msgdsize(xtip->xti_wexdata)-1024) >= 0)
			(void) xti_send (xtip, T_EXDATA_REQ);
	} else {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags |= XTI_MOREEXDATA;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}
}

xti_ordrel_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	int newState;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_ordrel_req");

	xtip = (struct xticb *)q->q_ptr;
	pp  = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_ORDREL_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_ordrel_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if ((mp->b_wptr - mp->b_rptr) < sizeof(struct T_ordrel_req)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_ordrel_req: incorrect message size\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if (xtip->xti_wdata)
		freemsg(xtip->xti_wdata);
	if (xtip->xti_wexdata)
		freemsg(xtip->xti_wexdata);
	if (xtip->xti_wnam)
		(void) m_free(xtip->xti_wnam);

	xtip->xti_wdata   = 0;
	xtip->xti_wexdata = 0;
	xtip->xti_wnam    = 0;
	xtip->xti_tsdu    = 0;
	xtip->xti_etsdu   = 0;

	if (soshutdown(xtip->xti_so, 1)) {
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	/* 
	 *  If shutdown completes, mark xtip so anyone attempting reuse
	 *  will create a new socket to work from.
	 *
	 *  cannot just close socket because of FIN/RST races.
	*/
	if (xtip->xti_so->so_state & SS_CANTSENDMORE &&
	    xtip->xti_so->so_state & SS_CANTRCVMORE) {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags |= XTI_NEEDSTARTUP;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}
	xtip->xti_state = newState;
	if (mp)
		freemsg(mp);
}

xti_unitdata_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register xtiproto_t *xp;
	register mblk_t *mp1;
	int size, error = 0;
	int newState;
	struct mbuf *nam = 0;
	char *stopt;
	struct t_opthdr *opt;
	long flags = T_NEGOTIATE;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_unitdata_req");

	xtip    = (struct xticb *)q->q_ptr;
	xp     = &(xtip->xti_proto);
	pp     = (union T_primitives *)mp->b_rptr;

	/*
	 * For TPI allowable fatal errors, call xti_cleanup
	 * otherwise, call xti_uderror_ind
	 */
	if ((newState = TNEXTSTATE(xtip, TE_UNITDATA_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
		     "xti_unitdata_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if (xp->xp_tsdulen == -2) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: TSDU not supported\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}
	if (xp->xp_tsdulen > 0 && msgdsize(mp) > xp->xp_tsdulen) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: TSDU size exceeded\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}
	if (mp->b_wptr - mp->b_rptr < sizeof(struct T_unitdata_req)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_unitdata_req: incorrect message size\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	error = copy_to_mbuf(mp->b_rptr + pp->tunitdatareq.DEST_offset,
			 pp->tunitdatareq.DEST_length, MT_SONAME, &nam);
	switch(error) {
	case ENOBUFS:
		xti_bufcall(q,pp->tunitdatareq.DEST_length,BPRI_HI,XTI_TIMEOUT);
	       	putbq(q, mp);
		return;
	case EINVAL:
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_unitdata_req: copy_to_mbuf failed\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	size = (mp->b_wptr - mp->b_rptr);

	if ((size < sizeof(struct T_unitdata_req)) ||
	    (size < (pp->tunitdatareq.DEST_length +
			pp->tunitdatareq.DEST_offset)) ||
	    (size < (pp->tunitdatareq.OPT_length +
			pp->tunitdatareq.OPT_offset))){
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: incorrect message size\n");
		xti_uderror_ind(q, mp, nam, XTIU_BADMSGSZ);
		return;
	}

	if (pp->tunitdatareq.OPT_length < 0 ||
	    (pp->tunitdatareq.OPT_length && xtip->xti_proto.xp_optlen == -2) ||
	    (xtip->xti_proto.xp_optlen >= 0 &&
	     pp->tunitdatareq.OPT_length > xtip->xti_proto.xp_optlen)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: incorrect option size\n");
		xti_uderror_ind(q, mp, nam, XTIU_BADOPTSZ);
		return;
	}

	/* set options */
	if (pp->tunitdatareq.OPT_length > 0) {
		xtip->xti_optlen = pp->tunitdatareq.OPT_length;
		stopt = (char *)mp->b_rptr + pp->tunitdatareq.OPT_offset;
		for (opt = (struct t_opthdr *)stopt; opt != NULL;
			opt = OPT_NEXTHDR(stopt, xtip->xti_optlen ,opt)) {

			if (opt->len > pp->tunitdatareq.OPT_length ||
				opt->len == sizeof(struct t_opthdr))
				goto badopt;
			if (opt->name == T_ALLOPT)
				continue;

			if (opt->level == XTI_GENERIC)
				error = xti_xti_optmgmt(opt, opt, xtip, 
							&flags, 0, 0);
			if (opt->level == INET_IP)
				error = xti_ip_optmgmt(opt, opt, xtip, 
							&flags, 0, 0);
			if (opt->level == INET_UDP)
				error = xti_udp_optmgmt(opt, opt, xtip, &flags);

			if (error) {
badopt:
				xti_uderror_ind(q, mp, nam, TBADOPT);
				return;
			}
		}
		if (!error)
			xtip->xti_optlen = 0;
	}

	xtip->xti_state = newState;

	if (xtip->xti_wdata)	/* XTI_FLOW protects */
		xti_panic("xti_unidata_req wdata");

	if (xtip->xti_wdata = unlinkb(mp)) {
		xtip->xti_wnam = nam;
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags &= ~XTI_MOREDATA;
		XTI_UNLOCK(&xtip->xtiso_lock);
		XTITRACE(XTIF_DATA,
		   printf(" xtiso: xti_unitdata_req() - msgdsize(mp=%x) = %d\n",
			  xtip->xti_wdata, msgdsize(xtip->xti_wdata)););
		if (xti_canput(xtip->xti_wq, msgdsize(xtip->xti_wdata)) >= 0) {
			error = xti_send (xtip, T_UNITDATA_REQ);
			if (error) {
				xti_uderror_ind(q, mp, nam, error);
				return;
			}
		} else {
			freemsg(xtip->xti_wdata);
			xtip->xti_wdata = 0;
			(void) m_free(nam);
			xtip->xti_wnam = 0;
		}
	} else
		(void) m_free(nam);

	if (mp)
		freemsg(mp);
}

/*
 * =======================================
 * XTI service routines for socket events.
 * =======================================
 */

xti_send(xtip, type)
	register struct xticb *xtip;
	int type;
{
	struct mbuf *data, *nam = 0;
	mblk_t **datap;
	int error, flag = 0;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_send");

	switch (type) {
	case T_UNITDATA_REQ:
		nam = xtip->xti_wnam;
		/* fall through */
	case T_DATA_REQ:
		datap = &xtip->xti_wdata;
		break;
	case T_EXDATA_REQ:
		flag |= MSG_OOB;
		datap = &xtip->xti_wexdata;
		break;
	default:
		xti_panic("xti_send");
		return (ENXIO);
	}

	XTITRACE(XTIF_SEND,
		printf(" xtiso: xti_send(xtip=%x type=%d) msgdsize=%d nam=%x\n",
			xtip, type, msgdsize(*datap), nam););

	data = mblk_to_mbuf(*datap, M_DONTWAIT);
	if (data == 0) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_send: mblk_to_mbuf failed\n");
		if (type != T_UNITDATA_REQ) {
			xti_bufcall(xtip->xti_wq, msgdsize(*datap), 0,
				XTI_TIMEOUT);
		} else {
			freemsg(*datap);
			*datap = 0;
			xtip->xti_wnam = 0;
			/* preserve nam for uderror_ind */
		}
		return (ENOBUFS);
	}
	*datap = 0;		/* mbufs now own the chain */
	if (nam)
		xtip->xti_wnam = 0;
	XTI_LOCK(&xtip->xtiso_lock);
	xtip->xti_flags &= ~XTI_FLOW;
	XTI_UNLOCK(&xtip->xtiso_lock);

	XTITRACE(XTIF_SEND,
		printf(" xtiso: xti_send() - sosend(%x)\n", xtip->xti_so););

	/* XXX blockage */
	error = sosend(xtip->xti_so, nam, (struct uio *)0, data,
			(struct mbuf *)0, flag);
	/*
	 * If the remote side closes, our socket pcb disappears(!).
	 * Due to flow control and xti_input() logic, we might not have
	 * yet discovered this disconnect, so we enforce it here.
	 */
	if (error == EPIPE)
		(void)xti_discon_ind(xtip);

#if	XTIDEBUG
	/*
	 * XXX Any error here means data lost. Effectively, the printf
	 * asserts we should never get here, though I'm sure we might.
	 * Current problem is exceeding the "TIDU". For datagrams the
	 * tidu/tsdu protects us, but for TCP byte stream connections
	 * (where tsdu == 0) this is an issue. The fix may be to copy
	 * a max length from xti_w[ex]data and send in pieces, or to
	 * pay attention to the tidu on send even if the tsdu is 0. Note
	 * xti_canput keeps us from ever getting here at the moment.
	 */
	if (error)
		printf(" xtiso: xticb %x sosend error %d: data lost\n",
			xtip, error);
#endif

	if (type != T_UNITDATA_REQ) {
		if (nam)
			(void) m_free(nam);
		return 0;
	}
	/* If error, preserve nam for uderror_ind */
	if (error == 0 && nam)
		(void) m_free(nam);

	return(error);
}

xti_conn_ind(xtip)
	register struct xticb *xtip;
{
	struct  xtiseq *seq;
	struct   mbuf *nam = 0;
	mblk_t  *mp = nilp(mblk_t);
	union   T_primitives *pp;
	queue_t *q;

	CHECKPOINT("xti_conn_ind");

	q = xtip->xti_rq;
	seq = xtip->xti_seq;
	do {
		if (seq->seq_used == XTIS_AWAITING)
			break;
	} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);
	if (seq >= &xtip->xti_seq[XTI_MAXQLEN] || seq->seq_so == 0 ||
	    TNEXTSTATE(xtip, TE_CONN_IND) == TS_BAD_STATE)
		goto bad;

	/*
	 * "getpeername"
	 */
	if (sogetaddr(seq->seq_so, &nam, 1, SOCKNAME_FORMAT)) 
		goto bad;
	if ((mp = allocb(sizeof(struct T_conn_ind) + nam->m_len, BPRI_MED)) 
		== nilp(mblk_t)) {
		xti_bufcall(q,sizeof(struct T_conn_ind)+nam->m_len,BPRI_MED, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_conn_ind) + nam->m_len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->type = T_CONN_IND;
	seq->seq_no = xtip->xti_seqcnt++;
	seq->seq_used = XTIS_ACTIVE;
	pp->tconnind.SRC_offset = sizeof(struct T_conn_ind);
	pp->tconnind.SRC_length = nam->m_len;
	pp->tconnind.SEQ_number = seq->seq_no;
	bcopy(mtod(nam, caddr_t),
		(caddr_t)(mp->b_rptr + pp->tconnind.SRC_offset),
		(unsigned)nam->m_len);
	pp->tconnind.OPT_offset = 0;
	pp->tconnind.OPT_length = 0;
	(void) m_free(nam);
	XTITRACE(XTIF_CONNECT,
	    printf(" xtiso: xti_conn_ind() -> putnext(q=%x mp=%x)\n", q, mp););

	/* 
	   set the queues for new socket(accepted) to get disconnect or any
	   other event.
	*/
	xti_init_socket(seq->seq_so, q);

	NEXTSTATE(xtip, TE_CONN_IND, "xti_conn_ind");
	PUTNEXT (q, mp);
	return;

pend:   if (nam)
		(void) m_free(nam);
	xtip->xti_pendind = T_CONN_IND;
	return;

bad:	if (nam)
		(void) m_free(nam);
	xti_cleanup(q, mp, EPROTO);
}

xti_conn_con(xtip)
	register struct xticb *xtip;
{
	struct  xtiseq *seq;
	struct   mbuf *nam = 0;
	mblk_t  *mp = nilp(mblk_t);
	union   T_primitives *pp;
	queue_t *q;
	int     len;

	CHECKPOINT("xti_conn_con");

	q = xtip->xti_rq;
	if (TNEXTSTATE(xtip, TE_CONN_CON) == TS_BAD_STATE)
		goto bad;

	/*
	 * "getpeername"
	 */
	if (sogetaddr(xtip->xti_so, &nam, 1, SOCKNAME_FORMAT))
		goto bad;

	len = sizeof(struct T_conn_con) + nam->m_len + xtip->xti_optlen;
	if ((mp = allocb(len, BPRI_MED)) == nilp(mblk_t)) {
		xti_bufcall(q, len, BPRI_MED, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->type = T_CONN_CON;
	pp->tconncon.RES_offset = sizeof(struct T_conn_con);
	pp->tconncon.RES_length = nam->m_len;
	bcopy(mtod(nam, caddr_t),
		(caddr_t)(mp->b_rptr + pp->tconncon.RES_offset),
		(unsigned)nam->m_len);
	(void) m_free(nam);

	/* Retrieve Options */
	if (xtip->xti_optlen > 0 && xtip->xti_opt) {
		pp->tconncon.OPT_offset = pp->tconncon.RES_offset + 
						pp->tconncon.RES_length;
		pp->tconncon.OPT_length = xtip->xti_optlen;
		bcopy(xtip->xti_opt, mp->b_rptr + pp->tconncon.OPT_offset,
			xtip->xti_optlen);

		(void)xmfree(xtip->xti_opt, pinned_heap);
		xtip->xti_opt = xtip->xti_optlen = 0;
	} else {
		pp->tconncon.OPT_offset = 0;
		pp->tconncon.OPT_length = 0;
	}
	XTITRACE(XTIF_CONNECT,
	    printf(" xtiso: xti_conn_con() -> putnext(q=%x mp=%x)\n", q, mp););
	NEXTSTATE(xtip, TE_CONN_CON, "xti_conn_con");
	PUTNEXT (q, mp);
	return;


pend:   if (nam)
		(void) m_free(nam);
	xtip->xti_pendind = T_CONN_CON;
	return;

bad:	if (nam)
		(void) m_free(nam);
	xti_cleanup(q, mp, EPROTO);
}

xti_discon_ind(xtip)
	register struct xticb *xtip;
{
	struct xtiseq *seq;
	mblk_t *mp = nilp(mblk_t);
	union T_primitives *pp;
	queue_t *q;
	int discon_event;
	int error = 0;
	int savedState;

	CHECKPOINT("xti_discon_ind");
	q = xtip->xti_rq;

	switch(xtip->xti_state) {
	case TS_DATA_XFER:
	case TS_WIND_ORDREL:
		if ((xtip->xti_proto.xp_servtype == T_COTS_ORD) ||
		    (xtip->xti_proto.xp_servtype == T_COTS))
			if (xti_snd_flushrw(q, FLUSHRW) == 0) {
				xti_snd_error_ack(q, mp, TSYSERR, EPROTO);
				return;
			}
	}

	savedState = xtip->xti_state;
	if (xtip->xti_cindno == 0) {
		discon_event = TE_DISCON_IND1;
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_discon_ind() - TE_DISCON_IND1\n"););
	} else if (xtip->xti_cindno == 1) {
		discon_event = TE_DISCON_IND2;
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_discon_ind() - TE_DISCON_IND2\n"););
	} else if (xtip->xti_cindno > 1)  {
		discon_event = TE_DISCON_IND3;
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_discon_ind() - TE_DISCON_IND3\n"););
	} else {
		/* should not occur */
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_ind:invalid connection indication count\n");
		goto bad;
	}

	if (TNEXTSTATE(xtip, discon_event) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_ind: would place interface out of state\n");
		goto bad;
	}

	if ((mp = allocb(sizeof(struct T_discon_ind),BPRI_HI)) ==nilp(mblk_t)) {
		xti_bufcall(q, sizeof(struct T_discon_ind), BPRI_HI, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_discon_ind);
	pp = (union T_primitives *)mp->b_rptr;
	pp->tdisconind.PRIM_type = T_DISCON_IND;

	switch (discon_event) {
	case TE_DISCON_IND1:
		if (xtip->xti_state == TS_WCON_CREQ)
			pp->tdisconind.DISCON_reason = XTID_REMREJECT;
		else
			pp->tdisconind.DISCON_reason = XTID_REMINIT;
		pp->tdisconind.SEQ_number = -1;
		break;

	case TE_DISCON_IND2:
	case TE_DISCON_IND3:
		seq = xtip->xti_seq;
		do {
			if (seq->seq_used == XTIS_LOST) {
				seq->seq_used = XTIS_AVAILABLE;
				pp->tdisconind.SEQ_number = seq->seq_no;
				if (xtip->xti_cindno-- == XTI_MAXQLEN)
					qenable(xtip->xti_rq);
				break;
			}
		} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);
		pp->tdisconind.DISCON_reason = XTID_REMWITHDRAW;
		break;
	}

	/* The close from other side removes our "pcb", so close the
	   current socket, and open, bind and listen again on new socket,
	   if qlen > 0, otherwise open new one.
	*/
	if (discon_event == TE_DISCON_IND1 && (xtip->xti_so->so_pcb == 0 ||
		(xtip->xti_so->so_state & SS_CANTSENDMORE &&
			    xtip->xti_so->so_state & SS_CANTRCVMORE))) {
		if (error = xti_startup(q, 1, xtip->xti_qlen))
			goto bad;
	}
	xtip->xti_state = savedState;
	XTITRACE(XTIF_CONNECT,
	   printf(" xtiso: xti_discon_ind() - putnext(q=%x, mp=%x)\n", q, mp););
	NEXTSTATE(xtip, discon_event, "xti_discon_ind");
	PUTNEXT(q, mp);
	return;

pend:
	xtip->xti_pendind = T_DISCON_IND;
	return;

bad:	xti_cleanup(q, mp, EPROTO);
}

xti_data_ind(indication, xtip)
	int	indication;
	register struct xticb *xtip;
{
	union   T_primitives *pp;
	queue_t *q;
	mblk_t  *mp;

	CHECKPOINT("xti_data_ind");

    	mp = nilp(mblk_t);
	q = xtip->xti_rq;
       	if (TNEXTSTATE(xtip, TE_DATA_IND) == TS_BAD_STATE || 
       		TNEXTSTATE(xtip, TE_EXDATA_IND) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_data_ind: would place interface out of state\n");
		goto bad;
	}
	if ((mp = allocb(sizeof(struct T_data_ind), BPRI_MED)) ==nilp(mblk_t)) {
		xti_bufcall(q, sizeof(struct T_data_ind), BPRI_MED, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_data_ind);
	pp = (union T_primitives *)mp->b_rptr;
	if (xti_rcv(xtip, &mp->b_cont, &pp->tdataind.MORE_flag, indication, 
			(struct mbuf *)0) == 0) {
		if (pp->tdataind.MORE_flag & T_EXPEDITED) {
			pp->type = T_EXDATA_IND;
			NEXTSTATE(xtip, TE_EXDATA_IND, "xti_data_ind");
		} else {
			pp->type = T_DATA_IND;
			NEXTSTATE(xtip, TE_DATA_IND, "xti_data_ind");
		}
		XTITRACE(XTIF_DATA,
		    printf(" xtiso: xti_data_ind() -> putnext(q=%x mp=%x)\n",
			q, mp););
		PUTNEXT (q, mp);
		return;
	}
	if (xtip->xti_bufcallid)
		goto pend;

bad:	xti_cleanup(q, mp, EPROTO);
	return;

pend:   xtip->xti_pendind = indication;
	if (mp) 
		freemsg(mp);
}

xti_ordrel_ind(xtip)
	register struct xticb *xtip;
{
	union   T_primitives *pp;
	queue_t *q;
	mblk_t  *mp;
	int err;
	int savedState;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_ordrel_ind");

    	mp = nilp(mblk_t);
	q = xtip->xti_rq;
       	if (TNEXTSTATE(xtip, TE_ORDREL_IND) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_ordrel_ind: would place interface out of state\n");
		goto bad;
	}

	savedState = xtip->xti_state;
	/*
	 * XXX shutdown here, even if cannot tell user new state.
	 * XXX if soshutdown succeeds, and allocb fails we blew it:
	 * XXX move to after allocb()?
	 */
	if (!(xtip->xti_flags & XTI_ORDREL)) {
		if (err = soshutdown(xtip->xti_so, 0)) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_ordrel_ind: soshutdown(%08x, 0) = %d\n",
							xtip->xti_so, err);
			goto bad;
		}
	} else {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_flags &= ~XTI_ORDREL;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}

	if ((mp = allocb(sizeof(struct T_ordrel_ind),BPRI_MED))==nilp(mblk_t)) {
		xti_bufcall(q, sizeof(struct T_ordrel_ind), BPRI_MED, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_ordrel_ind);
	pp            = (union T_primitives *)mp->b_rptr;
	pp->type      = T_ORDREL_IND;
	XTITRACE(XTIF_CONNECT,
	    printf(" xtiso: xti_ordrel_ind() -> putnext(q=%x mp=%x)\n",
		q, mp););

	/* 
	 * If shutdown completes, mark xtip so anyone attempting reuse
	 * will create a new socket to work from.
	 *
	 * cannot just close socket because of FIN/RST races.
	*/
	if (xtip->xti_so->so_state & SS_CANTSENDMORE &&
	    xtip->xti_so->so_state & SS_CANTRCVMORE) {
		if (xtip->xti_qlen > 0) {
			if (xti_startup(q, 1, xtip->xti_qlen)) 
				goto bad;
		} else {
			XTI_LOCK(&xtip->xtiso_lock);
			xtip->xti_flags |= XTI_NEEDSTARTUP;
			XTI_UNLOCK(&xtip->xtiso_lock);
		}
	}
	xtip->xti_state = savedState;
	NEXTSTATE(xtip, TE_ORDREL_IND, "xti_ordrel_ind");
	PUTNEXT (q, mp);
	return;

bad:	xti_cleanup(q, mp, EPROTO);
	return;

pend:   xtip->xti_pendind = T_ORDREL_IND;
}

xti_unitdata_ind(xtip)
	register struct xticb *xtip;
{
	int flags = 0;
	union T_primitives *pp;
	queue_t *q;
	mblk_t *mp;
	struct mbuf *nam = 0;

	CHECKPOINT("xti_unitdata_ind");

    	mp = nilp(mblk_t);
	q = xtip->xti_rq;

       	if (TNEXTSTATE(xtip, TE_UNITDATA_IND) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_ind: would place interface out of state\n");
		goto bad;
	}

	if ((mp = allocb(sizeof(struct T_unitdata_ind) + 
		xtip->xti_proto.xp_addrlen, BPRI_MED)) == nilp(mblk_t)) {
		xti_bufcall(q, sizeof(struct T_unitdata_ind) +
			xtip->xti_proto.xp_addrlen, BPRI_MED, 0); 
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_unitdata_ind);
	pp = (union T_primitives *)mp->b_rptr;
	pp->type = T_UNITDATA_IND;
	pp->tunitdataind.OPT_offset = 0;
	pp->tunitdataind.OPT_length = 0;
	pp->tunitdataind.SRC_offset = sizeof(struct T_unitdata_ind);
	if (xti_rcv(xtip, &mp->b_cont, &flags, T_UNITDATA_IND, &nam) == 0) {
		NEXTSTATE(xtip, TE_UNITDATA_IND, "xti_unitdata_ind");
		if (nam) {
			pp->tunitdataind.SRC_length = nam->m_len;
			bcopy(mtod(nam, caddr_t),
			     (caddr_t)(mp->b_rptr+pp->tunitdataind.SRC_offset),
			     (unsigned)nam->m_len);
			mp->b_wptr += nam->m_len;
			(void) m_free(nam);
		} else
			pp->tunitdataind.SRC_length = 0;
		XTITRACE(XTIF_DATA,
		   printf(" xtiso: xti_unitdata_ind() -> putnext(q=%x mp=%x)\n",
			q, mp););
		PUTNEXT (q, mp);
		return;
	} else
		goto bad;

	if (xtip->xti_bufcallid)
		goto pend;
bad:
	if (nam)
		(void) m_free(nam);
	xti_cleanup(q, mp, EPROTO);
	return;

pend:
	xtip->xti_pendind = T_UNITDATA_IND;
	if (nam)
		(void) m_free(nam);
	if (mp != nilp(mblk_t))
		freemsg(mp);
}

xti_uderror_ind(q, mp, nam, error)
	register queue_t *q;
	register mblk_t *mp;
	struct mbuf *nam;
	int error;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	register mblk_t *mp1;
	int len;

	CHECKPOINT("xti_uderror_ind");

	XTITRACE(XTIF_DATA,
		printf(" xtiso: xti_uderror_ind() error=%d\n", error););

	xtip = (struct xticb *)q->q_ptr;
	pp = (union T_primitives *)mp->b_rptr;

	if (xtip->xti_optlen > 0) {
		xtip->xti_opt = xmalloc(xtip->xti_optlen, 3, pinned_heap);
		if (xtip->xti_opt) 
			bcopy((mp->b_rptr + pp->tunitdatareq.OPT_offset),
				xtip->xti_opt, xtip->xti_optlen);
		else
			xtip->xti_optlen = 0;
	}

	len = sizeof(struct T_uderror_ind) + (nam ? nam->m_len : 0) + 
		xtip->xti_optlen;
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < len) {
		if ((mp1 = allocb(len, BPRI_HI)) == nilp (mblk_t)) {
			xti_snd_error_ack(q, mp, TSYSERR, EPROTO);
			if (nam)
				(void)m_free(nam);
			return;
		}
		freemsg(mp);
		mp = mp1;
	}	
	mp->b_datap->db_type = M_PROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->tuderrorind.PRIM_type	= T_UDERROR_IND;
	pp->tuderrorind.DEST_length	= (nam ? nam->m_len : 0);
	pp->tuderrorind.DEST_offset	= sizeof(struct T_uderror_ind);
	pp->tuderrorind.OPT_length	= xtip->xti_optlen;
	pp->tuderrorind.OPT_offset	= pp->tuderrorind.DEST_offset + 
						pp->tuderrorind.DEST_length;
	pp->tuderrorind.ERROR_type	= error;

	if (nam) {
		bcopy(mtod(nam, caddr_t),
			(caddr_t)(mp->b_rptr + pp->tuderrorind.DEST_offset),
			(unsigned)pp->tuderrorind.DEST_length);
		(void)m_free(nam);
	}
	if (xtip->xti_optlen > 0 && xtip->xti_opt) {
		bcopy(xtip->xti_opt, mp->b_rptr + pp->tuderrorind.OPT_offset,
			(unsigned)pp->tuderrorind.OPT_length);
		(void)xmfree(xtip->xti_opt, pinned_heap);
		xtip->xti_opt = xtip->xti_optlen = 0;
	}

	XTITRACE(XTIF_DATA,
	    printf(" xtiso: xti_uderror_ind() -> putnext(q=%x mp=%x)\n",
		q, mp););
	NEXTSTATE(xtip, TE_UDERROR_IND, "xti_uderror_ind");
	PUTNEXT(xtip->xti_rq, mp);
	return;
}

xti_rcv(xtip, dp, moreflagp, type, anam)
	register struct xticb *xtip;
	mblk_t	**dp;
	int	*moreflagp;
	int	type;
	struct	mbuf **anam;
{
	int error, size;
	register xtiproto_t *xp;

	CHECKPOINT("xti_rcv");

	XTITRACE(XTIF_RECV,
		printf(" xtiso: xti_rcv(xtip=%x dp=%x type=%x anam=%x)\n",
			xtip, dp, type, anam););

	xp = &(xtip->xti_proto);
	*moreflagp = 0;
	*dp = 0;

	if (xtip->xti_rdata == 0) {		/* No pending read data */
		struct uio uio;

		size = xp->xp_tidulen;
		if (type == T_EXDATA_IND) {
			if (xp->xp_etsdulen > 0 && size > xp->xp_etsdulen)
				size = xp->xp_etsdulen;
		} else {
			if (xp->xp_tsdulen > 0 && size > xp->xp_tsdulen)
				size = xp->xp_tsdulen;
		}

		uio.uio_resid = size;	/* max tsdu size */
		/* XXX blockage */
		error = soreceive(xtip->xti_so, anam, &uio, &xtip->xti_rdata,
				  (struct mbuf *)0, &xtip->xti_rflags);
		if (error && uio.uio_resid != size) switch (error) {
			case EINTR: case ERESTART: case EWOULDBLOCK:
				error = 0;
				break;
			}
		size -= uio.uio_resid;

		XTITRACE(XTIF_RECV,
			printf(" xtiso: soreceive (%d) read %d\n",error,size););

		if ((error == 0) && (xtip->xti_rdata == NULL))
			return 0;

		if (error)
			return error;
	} else {
		struct mbuf *mp;
		for (size = 0, mp = xtip->xti_rdata; mp; mp = mp->m_next)
			size += mp->m_len;
		XTITRACE(XTIF_RECV,
			printf(" xtiso: previous soreceive read %d\n", size););
	}

	XTITRACE(XTIF_RECV,
		printf(" xtiso: xti_rcv() - mbufdsize(%x)=%d\n",
			xtip->xti_rdata, mbufdsize(xtip->xti_rdata)););

	if ((*dp = mbuf_to_mblk(xtip->xti_rdata, BPRI_MED)) == 0) {
		xti_bufcall(xtip->xti_rq, size, BPRI_MED, 0);
		return ENOBUFS;
	}
	xtip->xti_rdata = 0;		/* mblks now own the chain */

	/*
	 * Handle T_MORE, depending on type. Note that the TSDU "option"
	 * is closely tied to the transport behavior here, since EOR will
	 * never be set for non-record oriented transports (e.g. TCP).
	 * However, XTI spec says flag is meaningless in this case.
	 * T_MORE means something additional for exdata.
	 */
	if (type == T_EXDATA_IND) {
		*moreflagp |= T_EXPEDITED;
		if (sbpoll(xtip->xti_so, &xtip->xti_so->so_rcv) & SE_HAVEOOB)
			*moreflagp |= T_MORE;
	} else if ((xp->xp_tsdulen > 0 || xp->xp_tsdulen == -1) &&
		   !(xtip->xti_rflags & MSG_EOR)) {
		*moreflagp |= T_MORE;
	}

	XTITRACE(XTIF_RECV,
		printf(" xtiso: xti_rcv() - size=%d, flags=%x, msgdsize=%d more=%d\n",
		size, xtip->xti_rflags, msgdsize(*dp), *moreflagp););

	xtip->xti_rflags = 0;

	return 0;
}

/*
 * ==================
 * XTI miscellaneous.
 * ==================
 */

/*
 * xti_cleanup()
 *		Called to clean up the xti connection whenever
 *		an unexpected fatal error occurs.  It will mark
 *		the xticb as "fatal" and reset all fields in the
 *		xticb, send a M_ERROR message to the STREAM head
 *		to inform it the fatal error. From this point on,
 *		all input and output messages will be tossed away.
 *		The only meaningful action is to wait for the
 *		driver xticlose routine being called to close the
 *		current stream.
 */
xti_cleanup(q, mp, error)
	register queue_t *q;
	register mblk_t *mp;
	int error;
{
	register struct xticb *xtip;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_cleanup");

	xtip = (struct xticb *)q->q_ptr;

	XTITRACE(XTIF_CLOSE, if (mp) {
		register union T_primitives *pp =
				(union T_primitives *)mp->b_rptr;
		printf(" xtiso: xti_cleanup(q=%x mp=%x error=%d)\n",
			q, mp, error);
		printf(" xtiso: xti_cleanup() - T_primitive type=%x (%d.)\n",
			pp->type, pp->type);
		});

	/*
	 * Flush data/etc out of our queues
	 */
	flushq(q, FLUSHALL);
	flushq(OTHERQ(q), FLUSHALL);

	XTI_LOCK(&xtip->xtiso_lock);
	xtip->xti_flags |= XTI_FATAL;
	XTI_UNLOCK(&xtip->xtiso_lock);

	/*
	 * Deinitialize this queue context
	 */
	xti_finished(q, XTI_NOSOCK);

	if (mp)
		freemsg(mp);

	if ((mp = allocb(sizeof(char), BPRI_HI)) == nilp(mblk_t)) 
		return;

	mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
	mp->b_datap->db_type = M_ERROR;
	/* *mp->b_wptr++ = EPROTO; */
	*mp->b_wptr++ = error;

	/*
	 * Send error message up Read queue.
	 * This should cause STREAM head to generate
	 * an M_FLUSH with FLUSHRW.
	 */
	if (xtip->xti_wq == q)
		qreply(q, mp);
	else
		PUTNEXT(q, mp);
}


xti_startup(q, which, qlen)
	register queue_t *q;
	int which;
	int qlen;
{
	int error = 0;
	register struct xticb *xtip;

	xtip = (struct xticb *)q->q_ptr;

	SOCKET_LOCK(xtip->xti_so);
	xtip->xti_so->so_options |= SO_LINGER;
	xtip->xti_so->so_linger = 0;
	SOCKET_UNLOCK(xtip->xti_so);

	/* close and open the socket */
	if (xti_finished(q, XTI_NOSOCK) == 0) {
		error = EPROTO;
		strlog(XTI_INFO_ID,xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_startup: failed to close the socket\n");
		goto bad;
	}

	if (which)
		error = xti_entry_init(xtip, q, XTI_NEWSOCK);
	else
		error = xti_entry_init(xtip, RD(q), XTI_NEWSOCK);
	if (error) {
		strlog(XTI_INFO_ID,xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_startup:failed to create socket=%d",error);
		goto bad;
	}
	/* bind and listen on new socket, if qlen > 0 */
	if (qlen > 0) {
		if (error = sobind(xtip->xti_so, xtip->xti_nam)) {
			strlog(XTI_INFO_ID,xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_startup: sobind() failed=%d\n", error);
			goto bad;
		}

		xtip->xti_qlen = qlen;
		if (error = solisten(xtip->xti_so, xtip->xti_qlen)) {
			strlog(XTI_INFO_ID,xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_startup: solisten() failed=%d\n", error);
			goto bad;
		}
		xtip->xti_lso = xtip->xti_so;
	}
bad:
	return error;
}


xti_finished(q, flags)
	register queue_t *q;
	int flags;
{
	register struct xticb *xtip;
	register struct xtiseq *seq;

	CHECKPOINT("xti_finished");

	xtip = (struct xticb *)q->q_ptr;
	if (xtip == 0)
		return 0;

	if (xtip->xti_lso)
		(void) soclose(xtip->xti_lso);

	if (xtip->xti_so && xtip->xti_so != xtip->xti_lso) {
		(void) soabort(xtip->xti_so);
		(void) soclose(xtip->xti_so);
	}

	seq = xtip->xti_seq;
	do {
		if (seq->seq_so) {
			(void) soabort(seq->seq_so);
			(void) soclose(seq->seq_so);
		}
	} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);

	if (xtip->xti_def_opts.ip_options)
		(void) m_free(xtip->xti_def_opts.ip_options);

	if (xtip->xti_wdata != nilp(mblk_t))
		freemsg (xtip->xti_wdata);

	if (xtip->xti_wexdata != nilp(mblk_t))
		freemsg (xtip->xti_wexdata);

	if (xtip->xti_wnam)
		(void) m_free(xtip->xti_wnam);

	if (xtip->xti_rdata)
		m_freem(xtip->xti_rdata);

	if (xtip->xti_optlen > 0 && xtip->xti_opt) {
		(void)xmfree(xtip->xti_opt, pinned_heap);
		xtip->xti_opt = xtip->xti_optlen = 0;
	}

	if (xti_entry_init(xtip, q, flags)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_finished: xti_entry_init failed\n");
		return 0;
	}

	return 1;
}

xti_snd_error_ack(q, mp, tli_error, unix_error)
	queue_t *q;
	register mblk_t *mp;
	long tli_error;
	long unix_error;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	long type;

	CHECKPOINT("xti_snd_error_ack");

	xtip = (struct xticb *)q->q_ptr;
	if (mp) {
		pp   = (union T_primitives *)mp->b_rptr;
		type = pp->type;
		freemsg(mp); 
	} else {
		type = xtip->xti_errtype;
		tli_error = tli_error ? tli_error : xtip->xti_tlierr;
		unix_error = unix_error ? unix_error : xtip->xti_unixerr;
	}

	XTITRACE(XTIF_ERRORS,
		printf(" xtiso: xti_snd_error_ack(q=%x mp=%x tli_error=%d unix_error=%d)\n",
			q, mp, tli_error, unix_error););

	if ((mp = allocb(sizeof(struct T_error_ack), BPRI_HI))== nilp(mblk_t)) {
		xtip->xti_tlierr = tli_error;
		xtip->xti_unixerr = unix_error;
		xtip->xti_errtype = type;
		xti_bufcall(xtip->xti_rq, sizeof(struct T_error_ack),BPRI_HI,0);
		xtip->xti_pendind = T_ERROR_ACK;
		return 0;
	}

	mp->b_rptr = mp->b_datap->db_base;
	mp->b_datap->db_type     = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_error_ack);
	pp = (union T_primitives *)mp->b_rptr;
	pp->terrorack.ERROR_prim = type;
	pp->terrorack.TLI_error  = tli_error;
	pp->terrorack.UNIX_error = unix_error;
	pp->terrorack.PRIM_type  = T_ERROR_ACK;

	if (TNEXTSTATE(xtip, TE_ERROR_ACK) != TS_BAD_STATE) 
		NEXTSTATE(xtip, TE_ERROR_ACK, "xti_snd_error_ack");

	/* if xti_conn_req() fails, free the memory */
	if (xtip->xti_optlen > 0 && xtip->xti_opt) {
		(void)xmfree(xtip->xti_opt, pinned_heap);
		xtip->xti_opt = xtip->xti_optlen = 0;
	}
	/*
	 * Send error message back up Read queue
	 */
	if (xtip->xti_wq == q)
		qreply(q, mp);
	else
		PUTNEXT(q, mp);
	return 1;
}

xti_snd_flushrw(q, dir)
	queue_t *q;
	int dir;
{
	register struct xticb *xtip;
	register mblk_t *mp;

	CHECKPOINT("xti_snd_flushrw");

	/*
	 * Generate a FLUSH queues message type,
	 * pass it up to STREAM head which should
	 * flush read queues and then pass M_FLUSH
	 * downstream to flush write queues
	 */
	if (mp = allocb(sizeof(*mp->b_wptr), BPRI_HI)) {
		mp->b_datap->db_type = M_FLUSH;
		mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;
		*mp->b_wptr++ = dir;
		xtip = (struct xticb *)q->q_ptr;
		if (xtip->xti_wq == q)
			qreply(q, mp);
		else
			PUTNEXT(q, mp);
		return 1;
	}
	return 0;
}

/*
 * xti_snd_ok_ack - postively acknowledge the current message
 */
xti_snd_ok_ack(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	long type;

	CHECKPOINT("xti_snd_ok_ack");

	xtip = (struct xticb *)q->q_ptr;
	if (mp) {
		pp = (union T_primitives *)mp->b_rptr;
		type = pp->type;
		freemsg(mp);
	} else
		type = xtip->xti_errtype;

	/*
	 * Allocate new storage for the 'ok' ack and free up the 'mp'.
         */
	if ((mp = allocb(sizeof(struct T_ok_ack), BPRI_HI)) == nilp(mblk_t)) {
		xtip->xti_errtype = type;
		xti_bufcall(xtip->xti_rq, sizeof(struct T_ok_ack), BPRI_HI, 0);
		return 0;
	}
	mp->b_datap->db_type    = M_PCPROTO;
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_ok_ack);
	pp                      = (union T_primitives *)mp->b_rptr;
	pp->tokack.CORRECT_prim = type;
	pp->tokack.PRIM_type    = T_OK_ACK;

	/*
	 * Send error message back up Read queue
	 */
	if (xtip->xti_wq == q)
		qreply(q, mp);
	else
		PUTNEXT(q, mp);
	return 1;
}

/* Buffer Management Routines */
copy_to_mbuf(addr, len, type, nam)
	char *addr;
	int len;
	int type;
	struct mbuf **nam;
{
	register struct mbuf *m = 0;
	int error = 0;

	CHECKPOINT("copy_to_mbuf");

	if ((u_int)len > MLEN)
		return(EINVAL);

	XTITRACE(XTIF_SOCKET,
		printf(" xtiso: addr=%x, len=%d, type=%d, nam=%x\n",
				addr, len, type, nam););

	/*
	 * Check input pointer to location to store mbuf pointer
	 */
	if (nam == (struct mbuf **)0)
		return(EINVAL);

	/*
	 * Attempt to allocate an mbuf
	 */
	*nam = m = m_get(M_DONTWAIT, type);
	if (m == 0)
		return(ENOBUFS);

	XTITRACE(XTIF_SOCKET,
		printf(" xtiso: m_get returns %x\n", m););

	m->m_len = len;
	bcopy((caddr_t)addr, mtod(m, caddr_t), (unsigned)len);

	/*
	 * Setup pointer to sockaddr structure
	 */
	sockaddr_new(m);		/* convert to kernel representation */

	XTITRACE
	(
		XTIF_SOCKET,
		{
			struct sockaddr *sa = mtod(m, struct sockaddr *);
			int i;

			printf(" xtiso:  family: %d\n", sa->sa_family);
			printf("            len: %d\n", sa->sa_len);
			printf("           data: ");
			for (i = 0; i < sizeof(sa->sa_data); i++)
				printf("[%d]=%d ", i, sa->sa_data[i]);
			printf("\n");
		}
	);

	return(error);
}

xti_bufcall(q, size, pri, type)
	queue_t *q;
	int size, pri, type;
{	
	register struct xticb *xtip = (struct xticb *)q->q_ptr;
	XTI_LOCK_DECL()

	XTI_LOCK(&xtip->xtiso_lock);
	xtip->xti_flags &= ~XTI_TIMEOUT;
	if (type == XTI_TIMEOUT) {
		xtip->xti_flags |= XTI_TIMEOUT;
		xtip->xti_bufcallid = timeout(qenable, (caddr_t)q, hz/2);
	} else {
		if ((xtip->xti_bufcallid = 
			bufcall(size, pri, qenable, (long) q)) == 0) {
			xtip->xti_flags |= XTI_TIMEOUT;
			xtip->xti_bufcallid = timeout(qenable,(caddr_t)q, hz/2);
		}
	}
	XTI_UNLOCK(&xtip->xtiso_lock);
}

xti_unbufcall(q)
	queue_t *q;
{
	register struct xticb *xtip = (struct xticb *)q->q_ptr;
	register int id;

	id = xtip->xti_bufcallid;
	if (!(xtip->xti_flags & XTI_TIMEOUT) && id)
		unbufcall(id);
	else if (id)
		untimeout(id);

	xtip->xti_bufcallid = 0;
	qenable(OTHERQ(q));
}


xti_entry_init(xtip, q, flags)
	register struct xticb *xtip;
	queue_t *q;
	int flags;
{
	register xtiproto_t *xp;
	register struct xtiseq *seq;
	int status = 0;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_entry_init");

#if	XTIDEBUG
	if (xtip == nilp(struct xticb)) {
		xti_panic("xti_entry_init");
		return(1);
	}
#endif

	XTITRACE((XTIF_OPEN | XTIF_CLOSE),
		printf(" xtiso: xti_entry_init() - %d\n", flags););

	/*
	 * (Re-)Init the xti context block
	 */
	XTI_LOCK(&xtip->xtiso_lock);
	xtip->xti_flags 	&= ~(XTI_ACTIVE);
	XTI_UNLOCK(&xtip->xtiso_lock);
	xtip->xti_state 	= TS_UNBND;
	xtip->xti_so 		= 0;
	xtip->xti_lso 		= 0;
	seq = xtip->xti_seq;
	do {
		seq->seq_used = XTIS_AVAILABLE;
		seq->seq_so   = 0;
		seq->seq_no   = 0;
	} while (++seq < &xtip->xti_seq[XTI_MAXQLEN]);
	xtip->xti_qlen		= 0;
	xtip->xti_cindno	= 0;
	xtip->xti_pendind 	= 0;
	xtip->xti_tsdu		= 0;
	xtip->xti_etsdu		= 0;
	xtip->xti_wdata		= 0;
	xtip->xti_wexdata	= 0;
	xtip->xti_rdata		= 0;
	xtip->xti_rflags	= 0;
	xtip->xti_wnam		= 0;
	xtip->xti_bufcallid     = 0;
	xtip->xti_pendcall      = 0;
	xtip->xti_tlierr        = 0;
	xtip->xti_unixerr       = 0;
	xtip->xti_errtype       = 0;
	xtip->xti_opt		= 0;
	xtip->xti_optlen	= 0;
	if (!(xtip->xti_flags & XTI_OPTINIT))
		bzero((caddr_t)&xtip->xti_def_opts, sizeof xtip->xti_def_opts);

	if (flags == XTI_NEWSOCK) {

		/*
	 	 * For new opens, grab some socket resources and initialize them
	 	 */
		xp = &(xtip->xti_proto);
		status = socreate(xp->xp_dom, &(xtip->xti_so),
				  xp->xp_type, xp->xp_proto);
		if (status == 0) {
			/*
			 * Setup "glue" in socket structure
			 * to make sockets/xtiso work
			 */
			xti_init_socket(xtip->xti_so, q);

			xtip->xti_wq = WR(q);
			xtip->xti_rq = q;
			XTI_LOCK(&xtip->xtiso_lock);
			xtip->xti_flags |= XTI_ACTIVE;
			xtip->xti_flags &= ~XTI_FLOW;
			XTI_UNLOCK(&xtip->xtiso_lock);

			XTITRACE(XTIF_OPEN,
				printf("\n xtiso: so-> after init...\n");
				DumpSO(xtip->xti_so););

		} else {
			XTITRACE(XTIF_OPEN | XTIF_CLOSE,
				printf(" xtiso: xti_entry_init() - socreate error = %d\n", status););
		}
	}

	return(status);
}


/*
 * xti_[rw]qenable()
 *
 *	Asynchronous upcalls from the socket layer come here. There are
 *	problems with synchronizing the two, especially regarding locking
 *	hierarchy. The solution is to place an "unsafe" state word in the
 *	xticb which is manipulated by these upcalls, and to use the
 *	sbpoll() mechanism to move it to a local copy, along with the
 *	current space or count. The old state is always or'd together
 *	with new events, and cleared on each poll.
 */

#define DATABITS	(SE_HAVEDATA|SE_HAVEOOB|SE_DATAFULL)

void
xti_wqenable(Q, state)
	caddr_t	Q;
	int state;
{
	register queue_t *q = (queue_t *)Q;
	struct xticb *xtip;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_wqenable");

	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: xti_wqenable(so=%x, q=%x)\n", so, q););

	if (!q || !(xtip = (struct xticb *)q->q_ptr))
		return;
	if (state & SE_STATUS) {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_sosnap |= state;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}
	if (state & SE_POLL) {
		/*
		 * The sbpoll() call results in this. Synchronously snap
		 * the current state and space, clear the state for later.
		 */
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_sostate |= (xtip->xti_sosnap & ~DATABITS);
		xtip->xti_sowspace = sbspace(&xtip->xti_so->so_snd);
		if (state & SE_ERROR)
			xtip->xti_soerror = xtip->xti_so->so_error;
		xtip->xti_sosnap = 0;
		XTI_UNLOCK(&xtip->xtiso_lock);
	} else {
		/*
		 * An asynchronous socket event results in this.
		 * If this socket is active, then enable queue,
		 * Otherwise, ignore wakeup call from socket layer
		 */
		XTI_LOCK(&xtip->xtiso_lock);
		if (xtip->xti_flags & XTI_ACTIVE) {
			XTI_UNLOCK(&xtip->xtiso_lock);
			qenable(q);
		}
		else
			XTI_UNLOCK(&xtip->xtiso_lock);
	}

	/* XXX - When this receives T_ORDREL_IND and T_DISCONNECT, all values
	   in so_rcv becomes zero and so the read queue is not getting enabled,
	   this way it will force !
	*/
	if ((xtip->xti_sosnap & SE_ERROR) && 
		(xtip->xti_so->so_error == ECONNRESET) && 
		(!xtip->xti_so->so_rcv.sb_wakearg || !xtip->xti_so->so_pcb)) {
		XTI_LOCK(&xtip->xtiso_lock);
		if (xtip->xti_flags & XTI_ACTIVE) {
			XTI_UNLOCK(&xtip->xtiso_lock);
			xti_discon_ind(xtip);
		}
		else
			XTI_UNLOCK(&xtip->xtiso_lock);
	}
}

void
xti_rqenable(Q, state)
	caddr_t	Q;
	int state;
{
	register queue_t *q = (queue_t *)Q;
	struct xticb *xtip;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_rqenable");

	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: xti_rqenable(so=%x, q=%x)\n", so, q););


	if (!q || !(xtip = (struct xticb *)q->q_ptr))
		return;
	if (state & SE_STATUS) {
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_sosnap |= state;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}
	if (state & SE_POLL) {
		/*
		 * The sbpoll() call results in this. Synchronously snap
		 * the current state and space, clear the state for later.
		 */
		XTI_LOCK(&xtip->xtiso_lock);
		xtip->xti_sostate |= (xtip->xti_sosnap & ~DATABITS);
		xtip->xti_sorcount = xtip->xti_so->so_rcv.sb_cc;
		if (state & SE_ERROR)
			xtip->xti_soerror = xtip->xti_so->so_error;
		xtip->xti_sosnap = 0;
		XTI_UNLOCK(&xtip->xtiso_lock);
	} else {
		/*
		 * An asynchronous socket event results in this.
		 * If this socket is active, then enable queue,
		 * Otherwise, ignore wakeup call from socket layer
		 */
		XTI_LOCK(&xtip->xtiso_lock);
		if (xtip->xti_flags & XTI_ACTIVE) {
			XTI_UNLOCK(&xtip->xtiso_lock);
			qenable(q);
		}
		else
			XTI_UNLOCK(&xtip->xtiso_lock);
	}
}

/*
 * xti_canput()
 *
 *	Flow control checking for socket sends. Returns additional
 *	space available:
 *		>= 0	data length acceptable
 *		<  0	too big by this much
 *	When returning < 0, sets XTI_FLOW. Note, expedited data
 *	may pass in a negative need, to adjust to socket's special
 *	handling of same.
 *
 *	XXX Problem: if xti_canput request is greater than max

 *	re-awaken it. Caller needs to examine xti_sowspace and
 *	return error, or strictly adhere to tsdulen, which must
 *	then be accurate.
 */
xti_canput(q, need)
	register queue_t *q;
	int need;
{
	register struct xticb *xtip;
	int space;
	XTI_LOCK_DECL()

	CHECKPOINT("xti_canput");

	xtip = (struct xticb *)q->q_ptr;

#if	XTIDEBUG
	if (xtip->xti_wq != q)
		xti_panic("xti_canput q");
	if (!xtip->xti_so)
		xti_panic("xti_canput so");
#endif

	/*
	 * Check available space
	 */
	(void) sbpoll(xtip->xti_so, &(xtip->xti_so->so_snd));
	/* Not interested in sostate here (???), so leave it alone */
	space = xtip->xti_sowspace;

	if (space < need) {
		XTITRACE(XTIF_SEND_FLOW, if (!(xtip->xti_flags & XTI_FLOW))
			printf(" xtiso: xti_canput() - setting FLOW CONTROL, space=%d, need=%d\n",
				space, need););
		XTI_LOCK(&xtip->xtiso_lock);	
		xtip->xti_flags |= XTI_FLOW;
		XTI_UNLOCK(&xtip->xtiso_lock);
	} else {
		XTITRACE(XTIF_SEND_FLOW, if (xtip->xti_flags & XTI_FLOW)
			printf(" xtiso: xti_canput() - clearing FLOW CONTROL, space=%d, need=%d\n",
				space, need););
		XTI_LOCK(&xtip->xtiso_lock);	
		xtip->xti_flags &= ~XTI_FLOW;
		XTI_UNLOCK(&xtip->xtiso_lock);
	}
	return (space - need);
}

xti_init_socket(so, q)
	register struct socket *so;
	queue_t *q;
{
	int x;

	CHECKPOINT("xti_init_socket");

	if (!so)
		return 0;

	SOCKET_LOCK(so);

	/*
	 * Don't do blocking operations down in Socket layer.
	 * No associated u-area with XTI Sockets.
	 */
	so->so_state   |= SS_NBIO;
	so->so_state   &= ~SS_PRIV;
	so->so_special |= (SP_NOUAREA|SP_EXTPRIV);

	/*
	 * Always want oob (exdata) as stream, not byte.
	 */
	so->so_options |= SO_OOBINLINE;

	/*
	 * Set SB_SEL to ensure upcalls.
	 * Make send/recv socket I/O non-interrupted
	 */
	so->so_rcv.sb_flags |= (SB_SEL|SB_NOINTR);
	so->so_snd.sb_flags |= (SB_SEL|SB_NOINTR);

	/*
	 * Assume the `q' points to read queue
	 */
	so->so_snd.sb_wakeup  = xti_wqenable;
	so->so_snd.sb_wakearg = (caddr_t)WR(q);
	so->so_rcv.sb_wakeup  = xti_rqenable;
	so->so_rcv.sb_wakearg = (caddr_t)q;

	SOCKET_UNLOCK(so);

	return 1;
}


/*
 *    XTI OPTION MANAGEMENT
 *
 *	Note: most all of the options below default to off or 0,
 *	except for the buffer sizes, which in the case of TCP may
 *	change after connection. We fetch them all anyway, but only
 *	when first asked for one.
 */

xti_init_default_options(xtip)
	struct xticb *xtip;
{
	struct socket *so = xtip->xti_so;
	struct mbuf   *m = 0;
	XTI_LOCK_DECL()

	/* XTI options */

	xtip->xti_def_opts.xti_debug = so->so_options & SO_DEBUG ? T_YES : T_NO;
	xtip->xti_def_opts.xti_linger.l_onoff = 
				so->so_options & SO_LINGER ? T_YES : T_NO;
	xtip->xti_def_opts.xti_linger.l_linger = so->so_linger;
	xtip->xti_def_opts.xti_sndbuf = so->so_snd.sb_hiwat;
	xtip->xti_def_opts.xti_rcvbuf = so->so_rcv.sb_hiwat;
	xtip->xti_def_opts.xti_sndlowat = so->so_snd.sb_lowat;
	xtip->xti_def_opts.xti_rcvlowat = so->so_rcv.sb_lowat;

	/* IP options */
	if (xtip->xti_proto.xp_dom == AF_INET) {

		xtip->xti_def_opts.ip_broadcast = 
			so->so_options & SO_BROADCAST ? T_YES : T_NO;
		xtip->xti_def_opts.ip_dontroute =
			so->so_options & SO_DONTROUTE ? T_YES : T_NO;
		xtip->xti_def_opts.ip_reuseaddr =
			so->so_options & SO_REUSEADDR ? T_YES : T_NO;

		if (!sogetopt(so, IPPROTO_IP, IP_OPTIONS, &m)) {
			if (m->m_len)
				xtip->xti_def_opts.ip_options = m;
			else
				(void) m_free(m);
			m = 0;
		}

		if (!sogetopt(so, IPPROTO_IP, IP_TOS, &m)) {
			xtip->xti_def_opts.ip_tos = *mtod(m, int *);
			(void) m_free(m);
			m = 0;
		}
		if (!sogetopt(so, IPPROTO_IP, IP_TTL, &m)) {
			xtip->xti_def_opts.ip_ttl = *mtod(m, int *);
			(void) m_free(m);
			m = 0;
		}

		/* TCP options */
		if (so->so_type == SOCK_STREAM) {

			xtip->xti_def_opts.tcp_keepalive.kp_onoff =
				so->so_options & SO_KEEPALIVE ? T_YES : T_NO;
			xtip->xti_def_opts.tcp_keepalive.kp_timeout = T_UNSPEC;

			if (!sogetopt(so, IPPROTO_TCP, TCP_NODELAY, &m)) {
				xtip->xti_def_opts.tcp_nodelay = 
					*mtod(m, int *) ? T_YES : T_NO;
				(void) m_free(m);
				m = 0;
			}
			if (!sogetopt(so, IPPROTO_TCP, TCP_MAXSEG, &m)) {
				xtip->xti_def_opts.tcp_maxseg = *mtod(m, int *);
				(void) m_free(m);
				m = 0;
			}
		/*
		 * UDP options.
		 * none at present can not be modified...
		 */
		} else
			xtip->xti_def_opts.udp_checksum = T_YES;
	}
	XTI_LOCK(&xtip->xtiso_lock);
	xtip->xti_flags |= XTI_OPTINIT;
	XTI_UNLOCK(&xtip->xtiso_lock);
}

/*
	optr is requested option pointer
	opta is returned option pointer
*/
xti_chk_protocol(optr, opta, xtip, flags, optlen)
	struct	t_opthdr *optr;
	struct	t_opthdr *opta;
	struct	xticb 	*xtip;
	long	*flags;
	int	optlen;
{
	if (!(xtip->xti_flags & XTI_OPTINIT))
		xti_init_default_options(xtip);

	switch (optr->level) {

	case XTI_GENERIC:
		return xti_xti_optmgmt(optr, opta, xtip, flags, 0, optlen);

	case INET_IP:
		if (xtip->xti_proto.xp_dom != AF_INET)
			break;
		return xti_ip_optmgmt(optr, opta, xtip, flags, 0, optlen);

	case INET_TCP:
		if (xtip->xti_proto.xp_dom != AF_INET)
			break;
		if (xtip->xti_so->so_type != SOCK_STREAM)
			break;
		return xti_tcp_optmgmt(optr, opta, xtip, flags, 0, optlen);

	case INET_UDP:
		if (xtip->xti_proto.xp_dom != AF_INET)
			break;
		if (xtip->xti_so->so_type != SOCK_DGRAM)
			break;
		return xti_udp_optmgmt(optr, opta, xtip, flags);

	/* Support more protocols here */

	default:
		/* unsupported/unknown level */
		return TBADOPT;
	}
	return opta->status = T_FAILURE;
}

/*	Get the worst single result of options from the status */
long
xti_opt_flags(status, iflags)
	long	status;
	long 	iflags;
{

	if (status == T_NOTSUPPORT)
		iflags = T_NOTSUPPORT;
	else if (status == T_READONLY && !(iflags & T_NOTSUPPORT))
		iflags = T_READONLY;
	else if (status == T_FAILURE && !(iflags & T_NOTSUPPORT) && 
			!(iflags & T_READONLY))
		iflags = T_FAILURE;
	else if (!(iflags & T_NOTSUPPORT) && !(iflags & T_READONLY) && 
			!(iflags & T_FAILURE))
		iflags = status;

	return iflags;
}

/*	Set the options */
static int
xti_setopt(so, level, name, addr)
	struct socket *so;
	long    level;
	long    name;
	long    *addr;
{
	struct mbuf *m = 0;
	struct t_linger *ling;
	struct t_kpalive *kp;
	int error;

	if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
		return 1;
	m->m_len = sizeof (int);
	*mtod(m, int *) = *addr;
	if (name == SO_LINGER) {
		m->m_len = sizeof (struct linger);
		ling = (struct t_linger *)addr;
		mtod(m, struct linger *)->l_onoff = ling->l_onoff;
		mtod(m, struct linger *)->l_linger = ling->l_linger;
	}
	if (name == SO_KEEPALIVE) {
		kp = (struct t_kpalive *)addr;
		*mtod(m, int *) = kp->kp_onoff;
	}
	if (error = sosetopt(so, level, name, m)) {
		strlog(XTI_INFO_ID, 0 ,0, SL_TRACE|SL_ERROR, 
			"xti_setopt: sosetopt failed = %d\n", error);
		return 1;
	}
	return 0;
}

xti_xti_optmgmt(optr, opta, xtip, flag, sock, optlen)
	struct t_opthdr *optr;
	struct t_opthdr *opta;
	struct xticb  *xtip;
	long *flag;
	struct	socket	*sock;
	int optlen;
{
	struct socket *so = xtip->xti_so;
	long *mpbuf = (long *)(opta + 1);
	long *mrbuf = (long *)(optr + 1);
	struct mbuf *m = 0;
	struct t_linger *ling;
	int all = 0, len = 0;
	long name, iflags;
	char *startopt;

	if (sock)
		so = sock;

	if (optr)
		name = optr->name;
	else
		name = T_ALLOPT;

	switch (name) {
	case T_ALLOPT:
		startopt = (char *)opta;
		all = 1;

	case XTI_DEBUG:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_debug;
			break;
		case T_CURRENT:
			*mpbuf = so->so_options & SO_DEBUG;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.xti_debug;
				if (xti_setopt(so, SOL_SOCKET, SO_DEBUG, mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (xti_setopt(so, SOL_SOCKET, SO_DEBUG, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = XTI_DEBUG;
		opta->level = XTI_GENERIC;
		len += opta->len;
		iflags = opta->status;
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case XTI_LINGER:
		ling = (struct t_linger *)mpbuf;
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			ling->l_onoff = xtip->xti_def_opts.xti_linger.l_onoff;
			ling->l_linger = xtip->xti_def_opts.xti_linger.l_linger;
			break;
		case T_CURRENT:
			ling->l_onoff = so->so_options & SO_LINGER ? T_YES:T_NO;
			ling->l_linger = so->so_linger;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) {
				ling = (struct t_linger *)mrbuf;
				if (ling->l_onoff != T_YES && 
						ling->l_onoff != T_NO)
					opta->status = T_FAILURE;
			}
			break;
		case T_NEGOTIATE:
			if (all) {
				ling->l_onoff = 
					xtip->xti_def_opts.xti_linger.l_onoff;
				ling->l_linger = 
					xtip->xti_def_opts.xti_linger.l_linger;
				if (xti_setopt(so,SOL_SOCKET, SO_LINGER, mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			ling = (struct t_linger *)mrbuf;
			if (ling->l_onoff != T_YES && ling->l_onoff != T_NO)
				return TBADOPT;
			if (ling->l_linger == T_UNSPEC)
				ling->l_linger = 
					xtip->xti_def_opts.xti_linger.l_linger;
			if (xti_setopt(so, SOL_SOCKET, SO_LINGER, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(struct t_linger) + 
						sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = XTI_LINGER;
		opta->level = XTI_GENERIC;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case XTI_RCVBUF:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_rcvbuf;
			break;
		case T_CURRENT:
			*mpbuf = so->so_rcv.sb_hiwat;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf < 0)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.xti_rcvbuf;
				if (xti_setopt(so,SOL_SOCKET, SO_RCVBUF, mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf < 0)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_RCVBUF, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = XTI_RCVBUF;
		opta->level = XTI_GENERIC;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case XTI_SNDBUF:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_sndbuf;
			break;
		case T_CURRENT:
			*mpbuf = so->so_snd.sb_hiwat;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf < 0)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.xti_sndbuf;
				if (xti_setopt(so,SOL_SOCKET, SO_SNDBUF, mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf < 0)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_SNDBUF, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = XTI_SNDBUF;
		opta->level = XTI_GENERIC;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case XTI_RCVLOWAT:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_rcvlowat;
			break;
		case T_CURRENT:
			*mpbuf = so->so_rcv.sb_lowat;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf < 0)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.xti_rcvlowat;
				if (xti_setopt(so,SOL_SOCKET,SO_RCVLOWAT,mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf < 0)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_RCVLOWAT, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = XTI_RCVLOWAT;
		opta->level = XTI_GENERIC;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case XTI_SNDLOWAT:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_sndlowat;
			break;
		case T_CURRENT:
			*mpbuf = so->so_snd.sb_lowat;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf < 0)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.xti_sndlowat;
				if (xti_setopt(so,SOL_SOCKET,SO_SNDLOWAT,mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf < 0)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_SNDLOWAT, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = XTI_RCVLOWAT;
		opta->level = XTI_GENERIC;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
done:
		*flag = iflags;
		return -len;	/* completed T_ALLOPT, notify caller */

	default:
		opta->status = T_NOTSUPPORT;
	}
	return 0;
}

xti_ip_optmgmt(optr, opta, xtip, flag, sock, optlen)
	struct	t_opthdr *optr;
	struct	t_opthdr *opta;
	struct	xticb	*xtip;
	int	*flag;
	struct	socket	*sock;
	int	optlen;
{
	int all = 0, len = 0, mlen = 0;
	struct socket *so = xtip->xti_so;
	long *mpbuf = (long *)(opta + 1);
	long *mrbuf = (long *)(optr + 1);
	struct mbuf *m = 0;
	long name, iflags;
	char *startopt;

	if (sock)
		so = sock;

	if (optr)
		name = optr->name;
	else
		name = T_ALLOPT;

	switch (name) {
	case T_ALLOPT:
		startopt = (char *)opta;
		all = 1;

	case IP_REUSEADDR:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_reuseaddr;
			break;
		case T_CURRENT:
			*mpbuf = so->so_options & SO_REUSEADDR ? T_YES : T_NO;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf != T_YES && *mrbuf != T_NO)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.ip_reuseaddr;
				if (xti_setopt(so, SOL_SOCKET, SO_REUSEADDR, 
						mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf != T_YES && *mrbuf != T_NO)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_REUSEADDR, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = IP_REUSEADDR;
		opta->level = INET_IP;
		len += opta->len;
		iflags = opta->status;
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case IP_DONTROUTE:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_dontroute;
			break;
		case T_CURRENT:
			*mpbuf = so->so_options & SO_DONTROUTE ? T_YES : T_NO;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf != T_YES && *mrbuf != T_NO)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.ip_dontroute;
				if (xti_setopt(so, SOL_SOCKET, SO_DONTROUTE, 
						mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf != T_YES && *mrbuf != T_NO)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_DONTROUTE, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = IP_DONTROUTE;
		opta->level = INET_IP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case IP_BROADCAST:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_broadcast;
			break;
		case T_CURRENT:
			*mpbuf = so->so_options & SO_BROADCAST ? T_YES : T_NO;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf != T_YES && *mrbuf != T_NO)
					opta->status = T_FAILURE;
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.ip_broadcast;
				if (xti_setopt(so, SOL_SOCKET, SO_BROADCAST,
						mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf != T_YES && *mrbuf != T_NO)
				return TBADOPT;
			if (xti_setopt(so, SOL_SOCKET, SO_BROADCAST, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = IP_BROADCAST;
		opta->level = INET_IP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case IP_TOS:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_tos;
			break;
		case T_CURRENT:
			if (!sogetopt(so, IPPROTO_IP, IP_TOS, &m)) {
				*mpbuf = *mtod(m, int *);
				(void) m_free(m);
				m = 0;
			} else
				opta->status = T_FAILURE;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.ip_tos;
				if (xti_setopt(so, IPPROTO_IP, IP_TOS, mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (xti_setopt(so, IPPROTO_IP, IP_TOS, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = IP_TOS;
		opta->level = INET_IP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case IP_TTL:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_ttl;
			break;
		case T_CURRENT:
			if (!sogetopt(so, IPPROTO_IP, IP_TTL, &m)) {
				*mpbuf = *mtod(m, int *);
				(void) m_free(m);
				m = 0;
			} else
				opta->status = T_FAILURE;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.ip_ttl;
				if (xti_setopt(so, IPPROTO_IP, IP_TTL, mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (xti_setopt(so, IPPROTO_IP, IP_TTL, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = IP_TTL;
		opta->level = INET_IP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case IP_OPTIONS:
		if (xtip->xti_def_opts.ip_options)
			mlen = xtip->xti_def_opts.ip_options->m_len;
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			bzero((caddr_t)mpbuf, mlen);
			if (xtip->xti_def_opts.ip_options) {
				if (xtip->xti_def_opts.ip_options->m_len > mlen)
					opta->status = T_FAILURE;
				else
					bcopy(mtod(xtip->xti_def_opts.ip_options, caddr_t),
					     (caddr_t)mpbuf, mlen);
			}
			break;
		case T_CURRENT:
			if (!sogetopt(so, IPPROTO_IP, IP_OPTIONS, &m)) {
				if (m->m_len) {
					mlen = m->m_len;
					bzero((caddr_t)mpbuf, mlen);
					bcopy(mtod(m, caddr_t), mpbuf,m->m_len);
				}
				m_free(m);
			} else
				opta->status = T_FAILURE;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (all) {
				bzero((caddr_t)mpbuf, mlen);
				if (xtip->xti_def_opts.ip_options) {
					if (xtip->xti_def_opts.ip_options->m_len > mlen)
						opta->status = T_FAILURE;
					else {
						bcopy(mtod(xtip->xti_def_opts.ip_options, caddr_t), (caddr_t)mpbuf, mlen);
						if (xti_setopt(so, IPPROTO_IP, 
							IP_OPTIONS, xtip->xti_def_opts.ip_options))
							opta->status =T_FAILURE;
					}
				}
				break;
			}
			if (mlen > MLEN)
				opta->status = T_FAILURE;
			else {
				if (!(m = m_get(M_DONTWAIT, MT_SOOPTS))) {
					opta->status = T_FAILURE;
					break;
				}
				m->m_len = optr->len - sizeof(struct t_opthdr);
				bcopy((caddr_t)mrbuf,mtod(m, caddr_t),m->m_len);
				if (sosetopt(so, IPPROTO_IP, IP_OPTIONS, m))
					opta->status = T_FAILURE;
			}
			break;
		}
		if (*flag != T_CHECK)
			opta->len = mlen + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = IP_OPTIONS;
		opta->level = INET_IP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
done:
		*flag = iflags;
		return -len;	/* completed T_ALLOPT, notify caller */

	default:
		opta->status = T_NOTSUPPORT;
	}
	return 0;
}

xti_udp_optmgmt(optr, opta, xtip, flag)
	struct t_opthdr *optr;
	struct t_opthdr *opta;
	struct xticb  *xtip;
	long *flag;
{
	long name;
	long *mpbuf = (long *)(opta + 1);
	int  all = 0;

	if (optr)
		name = optr->name;
	else
		name = T_ALLOPT;

	switch (name) {
	case T_ALLOPT:
		all = 1;
		opta->level = INET_UDP;
		opta->name = UDP_CHECKSUM;
	case UDP_CHECKSUM:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
		case T_CURRENT:
			*mpbuf = T_YES;
			opta->len = sizeof(unsigned long) + 
					sizeof(struct t_opthdr);
			break;
		case T_CHECK:
		case T_NEGOTIATE:
			opta->status = T_NOTSUPPORT;
			opta->len = sizeof(struct t_opthdr);
			break;
		}
		if (all) {
			*flag = opta->status;
			return -(opta->len);	/* completed T_ALLOPT */
		}
		break;
	default:
		opta->status = T_FAILURE;
	}
	return 0;
}

xti_tcp_optmgmt(optr, opta, xtip, flag, sock, optlen)
	struct	t_opthdr *optr;
	struct	t_opthdr *opta;
	struct	xticb	*xtip;
	long	*flag;
	struct	socket	*sock;
	int	optlen;
{
	struct mbuf *m = 0;
	struct socket *so = xtip->xti_so;
	long *mpbuf = (long *)(opta + 1);
	long *mrbuf = (long *)(optr + 1);
	int all = 0;
	struct t_kpalive *kp;
	int mlen, len = 0;
	long name, iflags;
	char *startopt;

	if (sock)
		so = sock;
	if (optr)
		name = optr->name;
	else
		name = T_ALLOPT;

	switch (name) {
	case T_ALLOPT:
		startopt = (char *)opta;
		all = 1;

	case TCP_NODELAY:
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.tcp_nodelay;
			break;

		case T_CURRENT:
			if (!sogetopt(so, IPPROTO_TCP, TCP_NODELAY, &m)) {
				*mpbuf = *mtod(m, int *) ? T_YES : T_NO;
				m_free(m);
			} else
				opta->status = T_FAILURE;
			break;

		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) 
				if (*mrbuf != T_YES && *mrbuf != T_NO)
					opta->status = T_FAILURE;
			break;

		case T_NEGOTIATE:
			if (all) {
				*mpbuf = xtip->xti_def_opts.tcp_nodelay;
				if (xti_setopt(so, IPPROTO_TCP, TCP_NODELAY,
						mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			if (*mrbuf != T_YES && *mrbuf != T_NO)
				return TBADOPT;
			if (xti_setopt(so, IPPROTO_TCP, TCP_NODELAY, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(unsigned long) + 
				sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = TCP_NODELAY;
		opta->level = INET_TCP;
		len += opta->len;
		iflags = opta->status;
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case TCP_MAXSEG:
		opta->status = T_READONLY;
		switch (*flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.tcp_maxseg;
			break;
		case T_CURRENT:
			if (!sogetopt(so, IPPROTO_TCP, TCP_MAXSEG, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opta->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (all)
				*mpbuf = xtip->xti_def_opts.tcp_maxseg;
			opta->status = T_READONLY;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(unsigned long) +
				sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = TCP_MAXSEG;
		opta->level = INET_TCP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
		if (!opta)
			goto done;
		mpbuf = (long *)(opta + 1);
		/* fall through */

	case TCP_KEEPALIVE:
		kp = (struct t_kpalive *)mpbuf;
		opta->status = T_SUCCESS;
		switch (*flag) {
		case T_DEFAULT:
			kp->kp_onoff =xtip->xti_def_opts.tcp_keepalive.kp_onoff;
			kp->kp_timeout = 
				xtip->xti_def_opts.tcp_keepalive.kp_timeout;
			break;
		case T_CURRENT:
			kp->kp_onoff = 
				so->so_options & SO_KEEPALIVE ? T_YES : T_NO;
			kp->kp_timeout = 
				xtip->xti_def_opts.tcp_keepalive.kp_timeout;
			break;
		case T_CHECK:
			if (optr->len > sizeof(struct t_opthdr)) {
				kp = (struct t_kpalive *)mrbuf;
				if (kp->kp_onoff != T_YES && 
							kp->kp_onoff != T_NO)
					opta->status = T_FAILURE;
				if (kp->kp_timeout > 0) 
					opta->status = T_NOTSUPPORT;
			}
			break;
		case T_NEGOTIATE:
			if (all) {
				kp->kp_onoff =
					xtip->xti_def_opts.tcp_keepalive.kp_onoff;
				kp->kp_timeout = 
					xtip->xti_def_opts.tcp_keepalive.kp_timeout;
				if (xti_setopt(so, SOL_SOCKET, SO_KEEPALIVE, 
						mpbuf))
					opta->status = T_FAILURE;
				break;
			}
			kp = (struct t_kpalive *)mrbuf;
			if (kp->kp_onoff != T_YES && kp->kp_onoff != T_NO)
				return TBADOPT;
			if (kp->kp_timeout > 0) {
				opta->status = T_NOTSUPPORT;
				break;
			}
			if (xti_setopt(so, SOL_SOCKET, SO_KEEPALIVE, mrbuf))
				opta->status = T_FAILURE;
			break;
		}
		if (*flag != T_CHECK)
			opta->len = sizeof(struct t_kpalive) + 
					sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opta->name = TCP_KEEPALIVE;
		opta->level = INET_TCP;
		len += opta->len;
		iflags = xti_opt_flags(opta->status, iflags);
		opta = OPT_NEXTHDR(startopt, optlen, opta);
done:
		*flag = iflags;
		return -len;	/* completed T_ALLOPT, notify caller */

	default:
		opta->status = T_NOTSUPPORT;
	}
	return 0;
}

#if	XTIDEBUG

/*
 * Routines for debugging.
 */

xti_putnext(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	CHECKPOINT("xti_putnext");

	XTITRACE(XTIF_MISC,
		printf(" xtiso: xti_putnext(q=%x mp=%x)\n", q, mp);
		DumpMBLK(mp);
	);

	putnext(q, mp);
}

xti_panic(s)
	char *s;
{
	if (xtiDEBUG & XTIF_PANIC)
		panic(s);
	printf(" xtiso: xti_panic \"%s\"\n", s);
}

#include <varargs.h>

xti_strlog(va_alist)
	va_dcl
{
	va_list ap;
	int a, b, c, d, e, f, g;
	char *fmt;

	va_start(ap);
	a =  va_arg(ap, int);
	b =  va_arg(ap, int);
	c =  va_arg(ap, int);
	d =  va_arg(ap, int);
	fmt = va_arg(ap, char *);
	e =  va_arg(ap, int);
	f =  va_arg(ap, int);
	g =  va_arg(ap, int);

	printf(" xtiso: xti_strlog() - (%d %d %d %d) ", a, b, c, d);
	printf(fmt, e, f, g);
}

Dumpbytes(hdr, start, end)
	char *hdr;
	char *start;
	char *end;
{
	int count = 0;
	char *cp;

	printf(hdr);
	for (cp = start; cp < end; cp++) {
		printf("%02x ", *cp);
		if (++count % 16 == 0)
			printf("\n");
	}
	printf("\n");
}

DumpIOCBLK(p)
	register struct iocblk *p;
{
	register status = 1;

	if (p) {
		printf("\n");
		printf(" xtiso: iocblk = %x...\n", p);
		printf("	  cmd: %d (%x)\n", p->ioc_cmd, p->ioc_cmd);
		printf("	  uid: %d\n", p->ioc_uid);
		printf("	  gid: %d\n", p->ioc_gid);
		printf("	   id: %d\n", p->ioc_id);
		printf("	count: %d\n", p->ioc_count);
		printf("	error: %d\n", p->ioc_error);
		printf("	 rval: %d\n", p->ioc_rval);
		printf("\n");

		status = 0;
	}

	return(status);
}

DumpPROTO(p)
	register xtiproto_t *p;
{
	register status = 1;

	if (p) {
		printf("\n");
		printf(" xtiso: xtiproto = %x...\n", p);
		printf("	   tsdulen: %d\n", p->xp_tsdulen);
		printf("	  etsdulen: %d\n", p->xp_etsdulen);
		printf("	connectlen: %d\n", p->xp_connectlen);
		printf("  	 disconlen: %d\n", p->xp_disconlen);
		printf("	   addrlen: %d\n", p->xp_addrlen);
		printf("	    optlen: %d\n", p->xp_optlen);
		printf(" 	   tidulen: %d\n", p->xp_tidulen);
		printf("	  servtype: %d\n", p->xp_servtype);
		printf("\n");

		status = 0;
	}

	return(status);
}

DumpSO(p)
	register struct socket *p;
{
	register status = 1;

	if (p) {
		printf("\n");
		printf(" xtiso: socket = %x...\n", p);
		printf("           type: %d\n", p->so_type);
		printf("        options: %x\n", p->so_options);
		printf("         linger: %d\n", p->so_linger);
		printf("          state: %x\n", p->so_state);
		printf("            pcb: %x\n", p->so_pcb);
		printf("          proto: %x\n", p->so_proto);
		printf("         iodone: %x\n", p->so_rcv.sb_iodone);
		printf("          ioarg: %x\n", p->so_rcv.sb_ioarg);
		printf("\n");

		status = 0;
	}

	return(status);
}

mbufdsize(m)
	register struct mbuf *m;
{
	int count = 0;

	while (m) {
		if (m->m_type == MT_DATA || m->m_type == MT_HEADER)
			count += m->m_len;
		m = m->m_next;
       }

       return(count);
}

DumpMBLK(p)
	mblk_t *p;
{
	mblk_t *mp = p;
	int status = 0;

	if (mp) {
		int cnt = 0;

		do {
			cnt++;
			printf("\n");
			printf(" %d -> mblk_t = %x ", cnt, mp);
			if (cnt == 1)
				printf(" (msgdsize = %d)", msgdsize(mp));
			printf(" ...\n");
			printf("  msgb:\n");
			printf("      b_next: %x\n", mp->b_next);
			printf("      b_prev: %x\n", mp->b_prev);
			printf("      b_cont: %x\n", mp->b_cont);
			printf("      b_rptr: %x\n", mp->b_rptr);
			printf("      b_wptr: %x\n", mp->b_wptr);
			printf("     b_datap: %x\n", mp->b_datap);
			printf("  datab:\n");
			printf("        db_freep: %x\n", mp->b_datap->db_freep);
			printf("         db_base: %x\n", mp->b_datap->db_base);
			printf("          db_lim: %x\n", mp->b_datap->db_lim);
			printf("          db_ref: %d\n", mp->b_datap->db_ref);
			printf("         db_type: %d\n", mp->b_datap->db_type);
			printf("\n");

		} while (mp = mp->b_cont);

		status = 0;
	}

	return(status);
}
#endif	/* XTIDEBUG */

/*
 * Excerpted from kernel/bsd/uipc_mbuf.c
 */

/* Mbuf <-> Mblk conversion routines */
/*
 * Copy mbufs to and from mblks. Note these structures are very
 * similar but do not _quite_ line up.
 * A semi-complete list:
 *	MBUF		MBLK
 *	m_data		b_rptr
 *	m_len		b_wptr-b_rptr
 *	m_data+m_len	b_wptr
 *	m_next		b_cont
 *	m_nextpkt	b_next
 *	m_ext		mh_dblk
 *
 *	m_get()		allocb()/allocbi()
 *	m_freem()	freeb()
 *
 * When copying, for now we simply wrap each buffer in the chain
 * inside a buffer of the destination. This is to 1) avoid bcopies,
 * 2) simplify, 3) support only the necessary stuff, and could well
 * be changed.
 */

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 */
m_leadingspace(m)
register struct	mbuf *m;
{
	if (m->m_flags & M_EXT)	{
		if (MCLREFERENCED(m))
			return 0;
		return (m->m_data - m->m_ext.ext_buf);
	}
	if (m->m_flags & M_PKTHDR)
		return (m->m_data - m->m_pktdat);
	return (m->m_data - m->m_dat);
}

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 */
m_trailingspace(m)
register struct	mbuf *m;
{
	if (m->m_flags & M_EXT)	{
		if (MCLREFERENCED(m))
			return 0;
		return (m->m_ext.ext_buf + m->m_ext.ext_size -
			(m->m_data + m->m_len));
	}
	return (&m->m_dat[MLEN]	- (m->m_data + m->m_len));
}

/*ARGSUSED*/
static void
m_freeb(p, size, bp)
	caddr_t p, bp;
	int size;
{
	(void) freeb((mblk_t *)bp);
}

mblk_t *
mbuf_to_mblk(m, pri)
	struct mbuf *m;
{
	mblk_t *bp, *top = 0, **bpp = &top;
	struct mbuf *nm;

	while (m) {
		int front = m_leadingspace(m);
		int back  = m_trailingspace(m);
		/* XXX switch to esballoc() - the "official" way to do this */
		bp = allocbi(front + m->m_len + back, pri,
				m_free, m, m->m_data - front);
		if (bp == 0) {
			while (top) {
				++top->b_datap->db_ref;
				bp = top->b_cont;
				(void) freeb(top);
				top = bp;
			}
			break;
		}
		bp->b_rptr = (unsigned char *)m->m_data;
		bp->b_wptr = (unsigned char *)(m->m_data + m->m_len);
		*bpp = bp;
		bpp = &bp->b_cont;
		nm = m->m_next;
		/*m->m_next = 0;*/
		m = nm;
	}
	return top;
}

struct mbuf *
mblk_to_mbuf(bp, canwait)
	mblk_t *bp;
{
	struct mbuf *m, *top = 0;
	struct mbuf **mp = &top;
	mblk_t *nbp;
	int len = 0;

	while (bp) {
		if (top == 0)
			m = m_gethdr(canwait, MT_DATA);
		else
			m = m_get(canwait, MT_DATA);
		if (m == 0) {
			while (top) {
				top->m_flags &= ~M_EXT;
				top = m_free(top);
			}
			break;
		}
		m->m_data = (caddr_t)bp->b_rptr;
		m->m_len = bp->b_wptr - bp->b_rptr;
		m->m_flags |= M_EXT;
		m->m_hasxm = 0;
		if (bp->b_datap->db_ref > 1) {		/* referenced */
			m->m_ext.ext_buf = m->m_data;
			m->m_ext.ext_size = m->m_len;
		} else {				/* available */
			m->m_ext.ext_buf = (caddr_t)bp->b_datap->db_base;
			m->m_ext.ext_size =
				bp->b_datap->db_lim - bp->b_datap->db_base;
		}
		m->m_ext.ext_free = m_freeb;
		m->m_ext.ext_arg = (caddr_t)bp;
		m->m_ext.ext_ref.forw = m->m_ext.ext_ref.back = 
			&m->m_ext.ext_ref;
		*mp = m;
		mp = &m->m_next;
		len += m->m_len;
		nbp = bp->b_cont;
		/*bp->b_cont = 0;*/
		bp = nbp;
	}
	if (top) {
		top->m_pkthdr.len = len;
		top->m_pkthdr.rcvif = 0;
	}
	return top;
}
