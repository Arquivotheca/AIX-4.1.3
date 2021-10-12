static char sccsid[] = "@(#)07	1.6  src/bos/kernext/aixif/slip.c, sysxaixif, bos411, 9435D411a 9/2/94 14:45:00";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: config_slip
 *		funit
 *		if_str_output
 *		if_str_detach
 *		sl_close
 *		sl_open
 *		sl_rput
 *		sl_rsrv
 *		sl_wput
 *		sl_wsrv
 *
 *   ORIGINS: 26,27,85
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/* slip (Serial Line Internet Protocol) streams module (line discipline) */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/ioctl.h>
#include <pse/str_lock.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/netisr.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/str_tty.h>
#include <net/slcompress.h>
#include <net/if_slvar.h>

int sl_rput();
int sl_wput();
int sl_rsrv();
int sl_wsrv();
int sl_open();
int sl_close();

static struct module_info minfo = 
	{ 0, "slip", 0, INFPSZ, 4096, 512
};

static struct qinit rinit = {
	sl_rput, sl_rsrv, sl_open, sl_close, NULL, &minfo
};

static struct qinit winit = {
	sl_wput, sl_wsrv, NULL, NULL, NULL, &minfo
};

struct streamtab slipinfo = { &rinit, &winit };

extern struct sl_softc *sl_softc;
static int if_str_output();
static int if_str_detach();

#include <sys/device.h>
#include <sys/uio.h>
#include <sys/strconf.h>

int
config_slip(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"slip", &slipinfo, STR_NEW_OPEN,
	};

	switch (cmd) {
		case CFG_INIT:  return str_install(STR_LOAD_MOD, &conf);
		case CFG_TERM:  return str_install(STR_UNLOAD_MOD, &conf);
		default:        return EINVAL;
	}

}

int
sl_open(q, dev, flag, sflag, credp)
	queue_t *q;
	dev_t *dev;
	int flag;
	int sflag;
	cred_t *credp;
{
	return 0;
}

int 
sl_close(q,flag, credp)
	queue_t *q;
	int flag;
	cred_t *credp;
{
	struct sl_softc *sc = (struct sl_softc *)q->q_ptr;

	if (sc != NULL)
		sldinit(sc);
	q->q_ptr = NULL;
	return 0;
}

int
sl_wput(q,mp)
	queue_t *q;
	mblk_t *mp;
{
	struct iocblk *iop;
	struct sl_softc *sc;
	int unit, error, *uptr;
	SLIP_LOCK_DECL()

	switch (mp->b_datap->db_type) {
	   case M_FLUSH:
		/* pass this message down-stream, so do not free here */
		if (*mp->b_rptr & FLUSHW)
 			flushq(q, FLUSHDATA);
		break;
	   case M_IOCTL:
		iop = (struct iocblk *)mp->b_rptr;
		switch(iop->ioc_cmd) {
		   case SLIOCGUNIT:
		   case SLIOCGFLAGS:
			if((sc = (struct sl_softc *)q->q_ptr) == NULL) {
				error = ENETDOWN;
				goto err;
			}
		    	if(!mp->b_cont &&
			   !(mp->b_cont = allocb(sizeof(int), BPRI_MED))) {
				error = ENOSR;
				goto err;
			}
			mp->b_datap->db_type = M_IOCACK;
			*((int *)(mp->b_cont->b_rptr)) =
				(iop->ioc_cmd == SLIOCGUNIT) ?
					sc->sc_if.if_unit :
					sc->sc_flags;
			mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
			iop->ioc_count = sizeof(int);
			iop->ioc_error = 0;
			break;
		   case SLIOCSFLAGS:
			if (!mp->b_cont ||
			    mp->b_cont->b_wptr - mp->b_cont->b_rptr != sizeof (int)) {
				error = EINVAL;
				goto err;
			}
			if((sc = (struct sl_softc *)q->q_ptr) == NULL) {
				error = ENETDOWN;
				goto err;
			}
			SLIP_LOCK(sc);
			sc->sc_flags = (*(int *)mp->b_cont->b_rptr & SC_CANSET)|
				(sc->sc_flags & ~SC_CANSET);
			SLIP_UNLOCK(sc);
			mp->b_datap->db_type = M_IOCACK;
			iop->ioc_rval = iop->ioc_count = iop->ioc_error = 0;
			break;
		   case SLIOCSATTACH:
			if (!mp->b_cont ||
			    mp->b_cont->b_wptr - mp->b_cont->b_rptr != sizeof (int)) {
				error = EINVAL;
				goto err;
			}
			uptr = *(int *)mp->b_cont->b_rptr;
			unit = *uptr;
			if ((error = funit( &unit)) == SC_FOUND) {
				for (sc = sl_softc; sc && \
				     sc->sc_if.if_unit != unit;
				     sc = sc->slip_next) 
					;
				if (error = slinit(sc)) {
					SLIP_LOCK(sc);
					sc->sc_flags &= ~SC_MASK;
					SLIP_UNLOCK(sc);
					goto err;
				}
				SLIP_LOCK(sc);
				sc->sc_qptr = (void *)q;
				RD(q)->q_ptr = q->q_ptr = (caddr_t)sc;
				sc->sc_output = if_str_output;
				sc->sc_detach = if_str_detach;
				/* sc->sc_flags |= SC_COMPRESS;	/% default(s) */
				sc->sc_if.if_flags |= (IFF_UP | IFF_RUNNING);
				SLIP_UNLOCK(sc);
				mp->b_datap->db_type = M_IOCACK;
				*(int *)mp->b_cont->b_rptr = unit;
				iop->ioc_rval = iop->ioc_error = 0;
				iop->ioc_count = sizeof(int);
			} else {
err:
				iop->ioc_count = 0;
				iop->ioc_error = error;
				mp->b_datap->db_type = M_IOCNAK;
			}
			break;

		   default:
			putnext(q, mp);
			return;
		}
		qreply(q, mp);
		return;
	   case M_DATA:
		freemsg(mp);
		return;
	   default:
		break;
	}
	putnext(q, mp);
}

int
sl_wsrv(q)
	register queue_t *q;
{
	register mblk_t *mp;

	while ((mp = getq(q)) != NULL) {
		if (!bcanput(q->q_next, mp->b_band)) {
			putbq(q, mp);
			return;
		}
		putnext(q, mp);
	}
}

int
sl_rput(q,mp)
	queue_t *q;
	mblk_t *mp;
{
	int *ctl;
	switch (mp->b_datap->db_type) {
		case M_DATA:
			if (canput(q))
				putq(q, mp);
			else
				freemsg(mp);
			return;
		case M_CTL:
			ctl = (int *)mp->b_rptr;
			switch (*ctl) {
			case cd_off:
				/* flush rs driver's read-queue, 
					re-use the message */
				mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
				mp->b_datap->db_type = M_FLUSH;
				*mp->b_wptr++ = FLUSHR;
				putnext(WR(q), mp);

				/* XXX - Mark hangup on this stream */
				putctl(q, M_HANGUP);
				/* 
				 * send M_ERROR to stream head and it will send
				 * M_FLUSH down stream to flush everything.
				 */
				putctl2(q, M_ERROR, 0, EIO);
				break;
			case cd_on:
				/* we are back, so clear the errors !! */
				putctl2(q, M_ERROR, 0, 0);
				freemsg(mp);
				break;
			default:
				freemsg(mp);
				break;
			}
                	break;
		case M_FLUSH:
			if (*mp->b_rptr & FLUSHR)
				flushq(q, FLUSHDATA);
			break;
		default:
			if (!canput(q->q_next)) {
				putbq(q, mp);
				return;
			}
			putnext(q, mp);
			break;
	}
}

sl_rsrv(q)
	register queue_t *q;
{
	register mblk_t *mp,*bp;
	register u_char c, *cp;
	struct sl_softc *sc;
	struct mbuf *m;
	int len;
	SLIP_LOCK_DECL()

	if((sc = (struct sl_softc *)q->q_ptr) == 0) {
		flushq(q, FLUSHALL);
		return;
	}
	while ((mp = getq(q)) != NULL) {
	    for (bp = mp; bp != 0; bp = bp->b_cont) {
               	cp = bp->b_rptr;
               	while (cp < bp->b_wptr) {
                       	c = *cp++;
	 		switch (c) {

		   	case TRANS_FRAME_ESCAPE:
				if (sc->sc_escape)
					c = FRAME_ESCAPE;
				break;

		   	case TRANS_FRAME_END:
				if (sc->sc_escape)
					c = FRAME_END;
				break;

		   	case FRAME_ESCAPE:
				sc->sc_escape = 1;
				continue;

		   	case FRAME_END:
				if (sc->sc_mp >= sc->sc_ep) {	/* overrun */
					goto error1;
				}
				len = sc->sc_mp - sc->sc_buf;
				if (len < 3) {
					goto newpack;
				}
				SLIP_LOCK(sc);
				if ((c = (*sc->sc_buf & 0xf0)) != (IPVERSION << 4)) {
					if (c & 0x80) {
						c = TYPE_COMPRESSED_TCP;
					}
					else if (c == TYPE_UNCOMPRESSED_TCP) {
						*sc->sc_buf &= 0x4f; /* XXX */
					}
			/*
 			 * We've got something that's not an IP packet.
 			 * If compression is enabled, try to decompress it.
 			 * Otherwise, if `auto-enable' compression is on and
 			 * it's a reasonable packet, decompress it and then
 			 * enable compression.  Otherwise, drop it.
 			 */
					if (sc->sc_flags & SC_COMPRESS) {
						len = sl_uncompress_tcp(
							&sc->sc_buf,
							len, (int)c,
							&sc->sc_comp);
						if (len <= 0)
							goto error;
					} else if ((sc->sc_flags & SC_AUTOCOMP) &&
					    c == TYPE_UNCOMPRESSED_TCP && 
					    len >= 40) {
						len = sl_uncompress_tcp(
							&sc->sc_buf,
							len, (int)c,
							&sc->sc_comp);
						if (len <= 0)
							goto error;
						sc->sc_flags |= SC_COMPRESS;
					} else {
						goto error;
					}
				}
				if ((m = sl_btom(sc, len)) == NULL) {
					goto error;
				}
				SLIP_UNLOCK(sc);
				sc->sc_if.if_ipackets++;
				microtime(&sc->sc_if.if_lastchange);
#ifndef _AIX
				if (netisr_input(NETISR_IP, m, (caddr_t)0, 0))
					goto error1;
#else

				/* IP only for now */
				find_input_type(0x0800, m, 
					(struct arpcom *)&sc->sc_if, 0);
#endif /* _AIX */

				goto newpack;
			}
			if (sc->sc_mp < sc->sc_ep)
				*sc->sc_mp++ = c;
			sc->sc_escape = 0;
			continue;
error:
 			SLIP_UNLOCK(sc);
error1:
			sc->sc_if.if_ierrors++;
newpack:
			sc->sc_mp = sc->sc_buf = sc->sc_ep - SLMAX;
			sc->sc_escape = 0;
		}
	    }
	    freemsg(mp);
	}
}

static int
if_str_output(inter, m, sc)
	int inter;
	struct	mbuf *m;
	struct	sl_softc *sc;
{
	mblk_t	*mp;
	struct	mbuf	*bp, *n;
	u_char	*cp;
	int	cnt, ch, len, i;
	queue_t	*wrtq;

	if(! (wrtq = (queue_t *)sc->sc_qptr)) {
		m_freem(m);
		return ENETDOWN;
	}
	if(! bcanput(wrtq, inter)) {
		m_freem(m);
		++sc->sc_if.if_collisions;
		return 0;
	}
	len = 2;
	for (n = m; n != 0 ; n = n->m_next ) {
		cp = mtod(n, u_char *);
		cnt = n->m_len;
		len += cnt;
		for(i = 0; i<cnt; i++) {
			switch (*cp++) {
			case FRAME_ESCAPE:
			case FRAME_END:
				len++;
			}
		}
	}
	if ((mp = allocb(len, BPRI_MED)) == NULL) {
		m_freem(m);
 		return ENOSR;
	}
	*mp->b_wptr++ = FRAME_END;
	for (bp = m; bp != NULL; bp = bp->m_next) {
		cp = mtod(bp, u_char *);
		for (cnt = 0; cnt < bp->m_len; cnt++) {
			switch (ch = *cp++) {
			   case FRAME_END:
				*mp->b_wptr++ = FRAME_ESCAPE;
				*mp->b_wptr++ = TRANS_FRAME_END;
				break;
			   case FRAME_ESCAPE:
				*mp->b_wptr++ = FRAME_ESCAPE;
				*mp->b_wptr++ = TRANS_FRAME_ESCAPE;
				break;
			   default:
				*mp->b_wptr++ = ch;
			}
		}
	}
	m_freem(m);
	*mp->b_wptr++ = FRAME_END;
	mp->b_band = inter;
	putq(wrtq, mp);
	return 0;
}

static int
if_str_detach(q)
	register queue_t *q;
{
	putctl1(RD(q), M_PCSIG, SIGTERM);
	return(0);
}

int
funit(i)
	int *i;
{
	struct sl_softc *sc;
	struct ifnet *ifp;
	int mode;
	char s[64];
	SLIP_LOCK_DECL()

	mode = 1;
	if (*i < 0)
		*i = mode = 0;
	while (1) {
		sprintf(s, "%s%d", SLIFNAME, *i);
		
		for (sc = sl_softc; sc && sc->sc_if.if_unit != *i;
			sc = sc->slip_next)
			    ;
		if ( !(ifp = ifunit(s)) || (ifp != &sc->sc_if)) 
			return mode ? ENXIO : EBUSY;
		SLIP_LOCK(sc);
		if (sc->sc_flags & SC_INUSE) {
			SLIP_UNLOCK(sc);
			if (mode)
				return (EALREADY);
		} else {
			sc->sc_flags = SC_INUSE;
			SLIP_UNLOCK(sc);
			return SC_FOUND;
		}
		*i++;
	}
}
