static char sccsid[] = "@(#)84	1.10  src/bos/kernext/bpf/bpf.c, sysxbpf, bos41B, 412_41B_sync 11/28/94 11:14:55";
/*
 *   COMPONENT_NAME: SYSXBPF
 *
 *   FUNCTIONS: BPF_SLEEP
 *		D_ISFREE
 *		D_MARKFREE
 *		D_MARKUSED
 *		ROTATE_BUFFERS
 *		UIOMOVE
 *		num_bpf_d
 *		bpf_aix_tap
 *		bpf_aix_mtap
 *		bpf_alloc
 *		bpf_allocbufs
 *		bpf_attachd
 *		bpf_cdli_tap
 *		bpf_detachd
 *		bpf_freed
 *		bpf_ifname
 *		make_ifname
 *		bpf_mcopy
 *		bpf_movein
 *		bpf_aix_mtap
 *		bpf_select
 *		bpf_setf
 *		bpf_setif
 *		bpf_sleep
 *		bpf_tap
 *		bpf_timeout
 *		bpf_wakeup
 *		bpfattach
 *		bpfclose
 *		bpfioctl
 *		bpfopen
 *		bpfread
 *		bpfselect
 *		bpfwrite
 *		catchpacket
 *		free
 *		ifpromisc
 *		malloc
 *		reset_d
 *
 *   ORIGINS: 26, 27
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
/*-
 * Copyright (c) 1990-1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from the Stanford/CMU enet packet filter,
 * (net/enet.c) distributed as part of 4.3BSD, and code contributed
 * to Berkeley by Steven McCanne and Van Jacobson both of Lawrence
 * Berkeley Laboratory.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	bpf.c	7.5 (Berkeley) 7/15/91
 *
 */


#ifndef __GNUC__
#define inline
#else
#define inline __inline__
#endif

#ifdef	_AIX
#define	KERNEL
#define BSD	199203
#endif	/* _AIX */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/dir.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/poll.h>
#ifndef	_AIX
#include <sys/map.h>
#endif	/* _AIX */

#include <sys/file.h>
#if defined(sparc) && BSD < 199103
#include <sys/stream.h>
#endif
#include <sys/tty.h>
#include <sys/uio.h>

#include <sys/protosw.h>
#include <sys/socket.h>
#include <net/if_types.h>
#include <net/if.h>

#include <net/bpf.h>
#include <net/bpfdesc.h>

#include <sys/errno.h>

#ifdef _AIX
#include <sys/cdli.h>
#include <net/nd_lan.h>
#endif /* _AIX */

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>
#include <netinet/if_fddi.h>

#ifndef	_AIX
#include <sys/kernel.h>
#endif	/* _AIX */

/*
 * Older BSDs don't have kernel malloc.
 */
#if BSD < 199103
extern bcopy();
static caddr_t bpf_alloc();
#define BPF_BUFSIZE (MCLBYTES-8)
#define UIOMOVE(cp, len, code, uio) uiomove(cp, len, code, uio)
#else
#define BPF_BUFSIZE 4096
#ifndef _AIX
#define UIOMOVE(cp, len, code, uio) uiomove(cp, len, uio)
#endif	/* _AIX */
#endif
#define BUFSIZE 512

#ifdef _AIX
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/entuser.h>
#define UIOMOVE(cp, len, code, uio) uiomove(cp, len, code, uio)
#define malloc(size, type, wait) net_malloc(size, type, wait)
#define free(addr, type) net_free(addr, type)
extern bcopy();
int tick = 1000;	/* XXX */
#else
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#endif	/* _AIX */

#ifndef _AIX
#define PRINET  26			/* interruptible */
#endif

/*
 * The default read buffer size is patchable.
 */
int bpf_bufsize = BPF_BUFSIZE;

/*
 *  bpf_iflist is the list of interfaces; each corresponds to an ifnet
 *  bpf_dtab holds the descriptors, indexed by minor device #
 */
struct bpf_if	*bpf_iflist;
struct bpf_d	bpf_dtab[NBPFILTER];

static void	bpf_ifname();
static void	catchpacket();
static void	bpf_freed();
static int	bpf_setif();
static int	bpf_initd();
static int	bpf_allocbufs();

#ifdef _AIX
void bpf_aix_tap();
void bpf_aix_mtap();
void make_ifname();
#endif  /* _AIX */

/*
 * Mark a descriptor free by making it point to itself.
 * This is probably cheaper than marking with a constant since
 * the address should be in a register anyway.
 */
#define D_ISFREE(d) ((d) == (d)->bd_next)
#define D_MARKFREE(d) ((d)->bd_next = (d))
#define D_MARKUSED(d) ((d)->bd_next = 0)

/*
 * return the number of listeners for a particular ifname in bpf_dtab[]
 */
int
num_bpf_d(ifname)
	char	*ifname;
{
	int		i, count;
	char		str[IFNAMSIZ];

	for (i=0, count=0; i<NBPFILTER; i++) {
		if(!(D_ISFREE(&bpf_dtab[i]))) {
			make_ifname(bpf_dtab[i].bd_bif->bif_ifp->if_name,
				    bpf_dtab[i].bd_bif->bif_ifp->if_unit,
				    str);
			if(strcmp(ifname, str) == 0)
				count++;
		}
	}
	return(count);
}

#ifndef _AIX
static int
bpf_movein(uio, linktype, mp, sockp)
	register struct uio *uio;
	int linktype;
	register struct mbuf **mp;
	register struct sockaddr *sockp;
{
	struct mbuf *m;
	int error;
	int len;
	int hlen;

	/*
	 * Build a sockaddr based on the data link layer type.
	 * We do this at this level because the ethernet header
	 * is copied directly into the data field of the sockaddr.
	 * In the case of SLIP, there is no header and the packet
	 * is forwarded as is.
	 * Also, we are careful to leave room at the front of the mbuf
	 * for the link level header.
	 */
	switch (linktype) {

	case IFT_SLIP:
		sockp->sa_family = AF_INET;
		hlen = 0;
		break;

	case IFT_ETHER:
		sockp->sa_family = AF_UNSPEC;
		/* XXX Would MAXLINKHDR be better? */
		hlen = sizeof(struct ether_header);
		break;

	case IFT_ISO88025:
		sockp->sa_family = AF_UNSPEC;
		hlen = 40;
		break;

	case IFT_FDDI:
		sockp->sa_family = AF_UNSPEC;
		/* XXX 4(FORMAC)+6(dst)+6(src)+3(LLC)+5(SNAP) */
		hlen = 24;
		break;

	default:
		return (EIO);
	}

	len = uio->uio_resid;
	if ((unsigned)len > MCLBYTES)
		return (EIO);

	MGET(m, M_WAIT, MT_DATA);
	if (m == 0)
		return (ENOBUFS);
	if (len > MLEN) {
#if BSD >= 199103
		MCLGET(m, M_WAIT);
		if ((m->m_flags & M_EXT) == 0) {
#else
		MCLGET(m);
		if (m->m_len != MCLBYTES) {
#endif
			error = ENOBUFS;
			goto bad;
		}
	}
	m->m_len = len;
	*mp = m;
	/*
	 * Make room for link header.
	 */
	if (hlen != 0) {
		m->m_len -= hlen;
#if BSD >= 199103
		m->m_data += hlen; /* XXX */
#else
		m->m_off += hlen;
#endif
		error = UIOMOVE((caddr_t)sockp->sa_data, hlen, UIO_WRITE, uio);
		if (error)
			goto bad;
	}
	error = UIOMOVE(mtod(m, caddr_t), len - hlen, UIO_WRITE, uio);
	if (!error)
		return (0);
 bad:
	m_freem(m);
	return (error);
}
#endif /* not _AIX */

/*
 * Attach file to the bpf interface, i.e. make d listen on bp.
 */
static void
bpf_attachd(d, bp)
	struct bpf_d *d;
	struct bpf_if *bp;
{
	BPF_LOCK_DECL()
	/*
	 * Point d at bp, and add d to the interface's list of listeners.
	 * Finally, point the driver's bpf cookie at the interface so
	 * it will divert packets to bpf.
	 */
	BPF_LOCK(d);
	d->bd_bif = bp;
	d->bd_next = bp->bif_dlist;
	bp->bif_dlist = d;

	*bp->bif_driverp = bp;
	BPF_UNLOCK(d);
#ifdef  _AIX
	bpf_aix_tap(1, bp->bif_ifp, bp);
#endif  /* _AIX */
}

/*
 * Detach a file from its interface.
 */
static void
bpf_detachd(d)
	struct bpf_d *d;
{
	struct bpf_d **p;
	struct bpf_if *bp;
	struct ifnet *bpifp;
	int s;

	IFQ_LOCK_DECL()
	BPF_LOCK_DECL()

	bp = d->bd_bif;
	bpifp = bp->bif_ifp;
	/*
	 * Check if this descriptor had requested promiscuous mode.
	 * If so, turn it off.
	 */
	if (d->bd_promisc) {
		d->bd_promisc = 0;
		if (ifpromisc(bp->bif_ifp, 0))
			/*
			 * Something is really wrong if we were able to put
			 * the driver into promiscuous mode, but can't
			 * take it out.
			 */
#ifndef _AIX
			panic("bpf: ifpromisc failed");
#else
			bsdlog("bpf: ifpromisc failed (disabling)");
#endif
	}
	/* Remove d from the interface's descriptor list. */
	p = &bp->bif_dlist;
	while (*p != d) {
		p = &(*p)->bd_next;
		if (*p == 0)
			panic("bpf_detachd: descriptor not in list");
	}
	*p = (*p)->bd_next;
	if (bp->bif_dlist == 0) {
		/*
		 * Let the driver know that there are no more listeners.
		 */
		BPF_LOCK(d);
		*d->bd_bif->bif_driverp = -1;
		BPF_UNLOCK(d);
#ifdef	_AIX
		(void)bpf_aix_tap(0, bp->bif_ifp, bp);
#endif	/* _AIX */
	}
	BPF_LOCK(d);
	d->bd_bif = 0;
	BPF_UNLOCK(d);
}


/*
 * Open device.  Returns ENXIO for illegal minor device number,
 * EBUSY if file is open by another process.
 */
/* ARGSUSED */
int
bpfopen(dev, flag)
	dev_t dev;
	int flag;
{
	register struct bpf_d *d;
	BPF_LOCK_DECL()

	if (minor(dev) >= NBPFILTER)
		return (ENXIO);
	/*
	 * Each minor can be opened by only one process.  If the requested
	 * minor is in use, return EBUSY.
	 */
	d = &bpf_dtab[minor(dev)];
	BPF_LOCK(d);
	if (!D_ISFREE(d)) {
		BPF_UNLOCK(d);
		return (EBUSY);
	}

	/*
	 * Mark "free" and do most initialization. 
	 * We don't want to zero out the lock element, this assumes 
	 * that the lock element is at the end of the bpf_d structure. 
	 */
	bzero((char *)d, sizeof(*d) - sizeof(simple_lock_data_t));
	d->bd_bufsize = bpf_bufsize;
	d->bd_devno = dev;
	BPF_UNLOCK(d);

	return (0);
}

/*
 * Close the descriptor by detaching it from its interface,
 * deallocating its buffers, and marking it free.
 */
/* ARGSUSED */
int
bpfclose(dev, flag)
	dev_t dev;
	int flag;
{
	BPF_LOCK_DECL()

	register struct bpf_d *d = &bpf_dtab[minor(dev)];
	register int s;

	if (d->bd_bif)
		bpf_detachd(d);
	BPF_LOCK(d);
	bpf_freed(d);
	BPF_UNLOCK(d);

	return (0);
}

/*
 * Support for SunOS, which does not have tsleep.
 */
#if BSD < 199103
#define BPF_SLEEP tsleep
#else
static
bpf_timeout(arg)
	caddr_t arg;
{
	struct bpf_d *d = (struct bpf_d *)arg;
	d->bd_timedout = 1;
#ifndef _AIX
	wakeup(arg);
#else
	net_wakeup(arg);
#endif
}

#define BPF_SLEEP(chan, pri, s, t) bpf_sleep((struct bpf_d *)chan)

int
bpf_sleep(d)
	register struct bpf_d *d;
{
	register int rto = d->bd_rtout;
	register int st;

	if (rto != 0) {
		d->bd_timedout = 0;
/*
		timeout(bpf_timeout, (caddr_t)d, rto);
*/
	}
#ifndef _AIX
	st = sleep((caddr_t)d, PRINET|PCATCH);
else

	st = net_sleep((caddr_t)d, PZERO+1);
#endif /* _AIX */
	if (rto != 0) {
		if (d->bd_timedout == 0) {
/*
			untimeout(bpf_timeout, (caddr_t)d);
*/
		}
		else if (st == 0)
			return EWOULDBLOCK;
	}
	return (st != 0) ? EINTR : 0;
}
#endif

/*
 * Rotate the packet buffers in descriptor d.  Move the store buffer
 * into the hold slot, and the free buffer into the store slot.
 * Zero the length of the new store buffer.
 */
#define ROTATE_BUFFERS(d) \
	(d)->bd_hbuf = (d)->bd_sbuf; \
	(d)->bd_hlen = (d)->bd_slen; \
	(d)->bd_sbuf = (d)->bd_fbuf; \
	(d)->bd_slen = 0; \
	(d)->bd_fbuf = 0;
/*
 *  bpfread - read next chunk of packets from buffers
 */
int
bpfread(dev, uio)
	dev_t dev;
	register struct uio *uio;
{
	register struct bpf_d *d = &bpf_dtab[minor(dev)];
	int error;
	BPF_LOCK_DECL()

	/*
	 * Restrict application to use a buffer the same size as
	 * as kernel buffers.
	 */
	if (uio->uio_resid != d->bd_bufsize)
		return (EINVAL);

	BPF_LOCK(d);
	/*
	 * If the hold buffer is empty, then do a timed sleep, which
	 * ends when the timeout expires or when enough packets
	 * have arrived to fill the store buffer.
	 */
	while (d->bd_hbuf == 0) {
		if (d->bd_immediate && d->bd_slen != 0) {
			/*
			 * A packet(s) either arrived since the previous
			 * read or arrived while we were asleep.
			 * Rotate the buffers and return what's here.
			 */
			ROTATE_BUFFERS(d);
			break;
		}

#ifndef	_AIX
		error = BPF_SLEEP((caddr_t)d, PRINET|PCATCH, "bpf",
				  d->bd_rtout);
#else
		BPF_UNLOCK(d);
		error = net_sleep((caddr_t)d, PZERO+1);
		if (error)
			error = EINTR;
		BPF_LOCK(d);
#endif
		if (error == EINTR || error == ERESTART) {
			BPF_UNLOCK(d);
			return (error);
		}
		if (error == EWOULDBLOCK) {
			/*
			 * On a timeout, return what's in the buffer,
			 * which may be nothing.  If there is something
			 * in the store buffer, we can rotate the buffers.
			 */
			if (d->bd_hbuf)
				/*
				 * We filled up the buffer in between
				 * getting the timeout and arriving
				 * here, so we don't need to rotate.
				 */
				break;

			if (d->bd_slen == 0) {
				BPF_UNLOCK(d);
				return (0);
			}
			ROTATE_BUFFERS(d);
			break;
		}
	}
	/*
	 * At this point, we know we have something in the hold slot.
	 */

	/*
	 * Move data from hold buffer into user space.
	 * We know the entire buffer is transferred since
	 * we checked above that the read buffer is bpf_bufsize bytes.
	 */
	error = UIOMOVE(d->bd_hbuf, d->bd_hlen, UIO_READ, uio);

	d->bd_fbuf = d->bd_hbuf;
	d->bd_hbuf = 0;
	d->bd_hlen = 0;
	BPF_UNLOCK(d);

	return (error);
}


/*
 * If there are processes sleeping on this descriptor, wake them up.
 */
static inline void
bpf_wakeup(d)
	register struct bpf_d *d;
{
	net_wakeup((caddr_t)d);
	if (d->bd_selreq & POLLIN) {
		d->bd_selreq &= ~POLLIN;
		selnotify(d->bd_devno, 0, POLLIN);
	}
}

#ifndef _AIX
int
bpfwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register struct bpf_d *d = &bpf_dtab[minor(dev)];
	struct ifnet *ifp;
	struct mbuf *m;
	int error, s;
	static struct sockaddr dst;

	if (d->bd_bif == 0)
		return (ENXIO);

	ifp = d->bd_bif->bif_ifp;

	if (uio->uio_resid == 0)
		return (0);
	if (uio->uio_resid > ifp->if_mtu)
		return (EMSGSIZE);

	error = bpf_movein(uio, (int)d->bd_bif->bif_dlt, &m, &dst);
	if (error)
		return (error);

	s = splnet();
#if BSD >= 199103
	error = (*ifp->if_output)(ifp, m, &dst, (struct rtentry *)0);
#else
	error = (*ifp->if_output)(ifp, m, &dst);
#endif
	splx(s);
	/*
	 * The driver frees the mbuf.
	 */
	return (error);
}
#endif

/*
 * Reset a descriptor by flushing its packet buffer and clearing the
 * receive and drop counts.
 */
static void
reset_d(d)
	struct bpf_d *d;
{
	BPF_LOCK_DECL()

	BPF_LOCK(d);
	if (d->bd_hbuf) {
		/* Free the hold buffer. */
		d->bd_fbuf = d->bd_hbuf;
		d->bd_hbuf = 0;
	}
	d->bd_slen = 0;
	d->bd_hlen = 0;
	d->bd_rcount = 0;
	d->bd_dcount = 0;
	BPF_UNLOCK(d);
}

/*
 *  FIONREAD		Check for read packet available.
 *  SIOCGIFADDR		Get interface address - convenient hook to driver.
 *  BIOCGBLEN		Get buffer len [for read()].
 *  BIOCSETF		Set ethernet read filter.
 *  BIOCFLUSH		Flush read packet buffer.
 *  BIOCPROMISC		Put interface into promiscuous mode.
 *  BIOCGDLT		Get link layer type.
 *  BIOCGETIF		Get interface name.
 *  BIOCSETIF		Set interface.
 *  BIOCSRTIMEOUT	Set read timeout.
 *  BIOCGRTIMEOUT	Get read timeout.
 *  BIOCGSTATS		Get packet stats.
 *  BIOCIMMEDIATE	Set immediate mode.
 *  BIOCVERSION		Get filter language version.
 */
/* ARGSUSED */
int
bpfioctl(dev, cmd, addr, flag)
	dev_t dev;
	int cmd;
	caddr_t addr;
	int flag;
{
	register struct bpf_d *d = &bpf_dtab[minor(dev)];
	int s, error = 0;
	BPF_LOCK_DECL()

	switch (cmd) {

	default:
		error = EINVAL;
		break;

	/*
	 * Check for read packet available.
	 */
	case FIONREAD:
		{
			int n;

			BPF_LOCK(d);
			n = d->bd_slen;
			if (d->bd_hbuf)
				n += d->bd_hlen;
			BPF_UNLOCK(d);

			*(int *)addr = n;
			break;
		}

	case SIOCGIFADDR:
		{
			struct ifnet *ifp;

			if (d->bd_bif == 0)
				error = EINVAL;
			else {
				ifp = d->bd_bif->bif_ifp;
				error = (*ifp->if_ioctl)(ifp, cmd, addr);
			}
			break;
		}

	/*
	 * Get buffer len [for read()].
	 */
	case BIOCGBLEN:
		*(u_int *)addr = d->bd_bufsize;
		break;

	/*
	 * Set buffer length.
	 */
	case BIOCSBLEN:
#if BSD < 199103
		error = EINVAL;
#else
		if (d->bd_bif != 0)
			error = EINVAL;
		else {
			register u_int size = *(u_int *)addr;

			if (size > BPF_MAXBUFSIZE)
				*(u_int *)addr = size = BPF_MAXBUFSIZE;
			else if (size < BPF_MINBUFSIZE)
				*(u_int *)addr = size = BPF_MINBUFSIZE;
			BPF_LOCK(d);
			d->bd_bufsize = size;
			BPF_UNLOCK(d);
		}
#endif
		break;

	/*
	 * Set link layer read filter.
	 */
	case BIOCSETF:
		error = bpf_setf(d, (struct bpf_program *)addr);
		break;

	/*
	 * Flush read packet buffer.
	 */
	case BIOCFLUSH:
		reset_d(d);
		break;

	/*
	 * Put interface into promiscuous mode.
	 */
	case BIOCPROMISC:
		if (d->bd_bif == 0) {
			/*
			 * No interface attached yet.
			 */
			error = EINVAL;
			break;
		}
		if (d->bd_promisc == 0) {
			error = ifpromisc(d->bd_bif->bif_ifp, 1);
			if (error == 0)
				d->bd_promisc = 1;
		}
		break;

	/*
	 * Get device parameters.
	 */
	case BIOCGDLT:
		if (d->bd_bif == 0)
			error = EINVAL;
		else
			*(u_int *)addr = d->bd_bif->bif_dlt;
		break;

	/*
	 * Set interface name.
	 */
	case BIOCGETIF:
		if (d->bd_bif == 0)
			error = EINVAL;
		else
			bpf_ifname(d->bd_bif->bif_ifp, (struct ifreq *)addr);
		break;

	/*
	 * Set interface.
	 */
	case BIOCSETIF:
		error = bpf_setif(d, (struct ifreq *)addr);
		break;

	/*
	 * Set read timeout.
	 */
	case BIOCSRTIMEOUT:
		{
			struct timeval *tv = (struct timeval *)addr;
			u_long msec;

			/* Compute number of milliseconds. */
			msec = tv->tv_sec * 1000 + tv->tv_usec / 1000;
			/* Scale milliseconds to ticks.  Assume hard
			   clock has millisecond or greater resolution
			   (i.e. tick >= 1000).  For 10ms hardclock,
			   tick/1000 = 10, so rtout<-msec/10. */
			BPF_LOCK(d);
			d->bd_rtout = msec / (tick / 1000);
			BPF_UNLOCK(d);
			break;
		}

	/*
	 * Get read timeout.
	 */
	case BIOCGRTIMEOUT:
		{
			struct timeval *tv = (struct timeval *)addr;
			u_long msec = d->bd_rtout;

			msec *= tick / 1000;
			tv->tv_sec = msec / 1000;
			tv->tv_usec = msec % 1000;
			break;
		}

	/*
	 * Get packet stats.
	 */
	case BIOCGSTATS:
		{
			struct bpf_stat *bs = (struct bpf_stat *)addr;

			bs->bs_recv = d->bd_rcount;
			bs->bs_drop = d->bd_dcount;
			break;
		}

	/*
	 * Set immediate mode.
	 */
	case BIOCIMMEDIATE:
		BPF_LOCK(d);
		d->bd_immediate = 1;
		BPF_UNLOCK(d);
		break;

	case BIOCVERSION:
		{
			struct bpf_version *bv = (struct bpf_version *)addr;

			bv->bv_major = BPF_MAJOR_VERSION;
			bv->bv_minor = BPF_MINOR_VERSION;
			break;
		}
	}
	return (error);
}

/*
 * Set d's packet filter program to fp.  If this file already has a filter,
 * free it and replace it.  Returns EINVAL for bogus requests.
 */
int
bpf_setf(d, fp)
	struct bpf_d *d;
	struct bpf_program *fp;
{
	struct bpf_insn *fcode, *old;
	u_int flen, size;
	int s;
	BPF_LOCK_DECL()

	old = d->bd_filter;
	if (fp->bf_insns == 0) {
		if (fp->bf_len != 0)
			return (EINVAL);
		BPF_LOCK(d);
		d->bd_filter = 0;
		BPF_UNLOCK(d);
		reset_d(d);
		if (old != 0)
			free((caddr_t)old, M_TEMP);
		return (0);
	}
	flen = fp->bf_len;
	if (flen > BPF_MAXINSNS)
		return (EINVAL);

	size = flen * sizeof(*fp->bf_insns);
	fcode = (struct bpf_insn *)malloc(size, M_TEMP, M_WAITOK);
	if (copyin((caddr_t)fp->bf_insns, (caddr_t)fcode, size) == 0 &&
	    bpf_validate(fcode, (int)flen)) {
		BPF_LOCK(d);
		d->bd_filter = fcode;
		BPF_UNLOCK(d);
		reset_d(d);
		if (old != 0)
			free((caddr_t)old, M_TEMP);

		return (0);
	}
	free((caddr_t)fcode, M_TEMP);
	return (EINVAL);
}

/*
 * Detach a file from its current interface (if attached at all) and attach
 * to the interface indicated by the name stored in ifr.
 * Return an errno or 0.
 */
static int
bpf_setif(d, ifr)
	struct bpf_d *d;
	struct ifreq *ifr;
{
	struct bpf_if *bp;
	char *cp;
	int unit, s, error;
	IFQ_LOCK_DECL()
	BPF_LOCK_DECL()

	/*
	 * Separate string into name part and unit number.  Put a null
	 * byte at the end of the name part, and compute the number.
	 * If the a unit number is unspecified, the default is 0,
	 * as initialized above.  XXX This should be common code.
	 */
	unit = 0;
	cp = ifr->ifr_name;
	cp[sizeof(ifr->ifr_name) - 1] = '\0';
	while (*cp++) {
		if (*cp >= '0' && *cp <= '9') {
			unit = *cp - '0';
			*cp++ = '\0';
			while (*cp)
				unit = 10 * unit + *cp++ - '0';
			break;
		}
	}
	/*
	 * Look through attached interfaces for the named one.
	 */
	for (bp = bpf_iflist; bp != 0; bp = bp->bif_next) {
		struct ifnet *ifp = bp->bif_ifp;

		if (ifp == 0 || unit != ifp->if_unit
		    || strcmp(ifp->if_name, ifr->ifr_name) != 0)
			continue;
		/*
		 * We found the requested interface.
		 * If it's not up, return an error.
		 * Allocate the packet buffers if we need to.
		 * If we're already attached to requested interface,
		 * just flush the buffer.
		 */
		if ((ifp->if_flags & IFF_UP) == 0)
			return (ENETDOWN);

		BPF_LOCK(d);
		if (d->bd_sbuf == 0) {
			error = bpf_allocbufs(d);
			if (error != 0) {
				BPF_UNLOCK(d);
				return (error);
			}
		}
		BPF_UNLOCK(d);
		if (bp != d->bd_bif) {
			if (d->bd_bif)
				/*
				 * Detach if attached to something else.
				 */
				bpf_detachd(d);

			bpf_attachd(d, bp);
		}
		reset_d(d);
		return (0);
	}
	/* Not found. */
	return (ENXIO);
}

/*
 * Convert an interface name plus unit number of an ifp to a single
 * name which is returned in the ifr.
 */
static void
bpf_ifname(ifp, ifr)
	struct ifnet *ifp;
	struct ifreq *ifr;
{
	char *s = ifp->if_name;
	char *d = ifr->ifr_name;

	while (*d++ = *s++)
		continue;
	/* XXX Assume that unit number is less than 10. */
	*d++ = ifp->if_unit + '0';
	*d = '\0';
}


/*
 * Support for select() system call
 *
 * Return true iff the specific operation will not block indefinitely.
 * Otherwise, return false but make a note that a selwakeup() must be done.
 */
bpfselect(dev, reqevents, rtneventsp, chan)
dev_t dev;
ushort reqevents;
ushort *rtneventsp;
int chan;
{
	BPF_LOCK_DECL()
	register struct bpf_d *d;

	d = &bpf_dtab[minor(dev)];

	BPF_LOCK(d);

	*rtneventsp = 0;
	if (reqevents & POLLOUT)
		*rtneventsp |= POLLOUT;

	if (reqevents & POLLIN) {
		if (d->bd_hlen != 0 || (d->bd_immediate && d->bd_slen != 0)) {
			/*
			 * There is data waiting.
			 */
			*rtneventsp |= POLLIN;
		}
	}
	if (*rtneventsp == 0) {
		d->bd_selreq |= reqevents;
	}
	BPF_UNLOCK(d);
	return (0);
}

/*
 * Incoming linkage from device drivers.  Process the packet pkt, of length
 * pktlen, which is stored in a contiguous buffer.  The packet is parsed
 * by each process' filter, and if accepted, stashed into the corresponding
 * buffer.
 */
void
bpf_tap(arg, pkt, pktlen)
	caddr_t arg;
	register u_char *pkt;
	register u_int pktlen;
{
	struct bpf_if *bp;
	register struct bpf_d *d;
	register u_int slen;
	/*
	 * Note that the ipl does not have to be raised at this point.
	 * The only problem that could arise here is that if two different
	 * interfaces shared any data.  This is not the case.
	 */
	bp = (struct bpf_if *)arg;
	for (d = bp->bif_dlist; d != 0; d = d->bd_next) {
		++d->bd_rcount;
		slen = bpf_filter(d->bd_filter, pkt, pktlen, pktlen);
		if (slen != 0)
			catchpacket(d, pkt, pktlen, slen, bcopy);
	}
}

/*
 * Copy data from an mbuf chain into a buffer.  This code is derived
 * from m_copydata in sys/uipc_mbuf.c.
 */
static void
bpf_mcopy(src, dst, len)
	u_char *src;
	u_char *dst;
	register int len;
{
	register struct mbuf *m;
	register unsigned count;

	m = (struct mbuf *)src;
	while (len > 0) {
		if (m == 0)
			panic("bpf_mcopy");
		count = MIN(m->m_len, len);
		bcopy(mtod(m, caddr_t), (caddr_t)dst, count);
		m = m->m_next;
		dst += count;
		len -= count;
	}
}

/*
 * Incoming linkage from device drivers, when packet is in an mbuf chain.
 */
void
bpf_aix_mtap(arg, m)
	caddr_t arg;
	struct mbuf *m;
{
	struct bpf_if *bp = (struct bpf_if *)arg;
	struct bpf_d *d;
	u_int pktlen, slen;
	struct mbuf *m0, *m1;
	register struct ie5_mac_hdr *ie5_macp;
	register struct fddi_mac_hdr *macf;
	int mac_len;
	BPF_LOCK_DECL()

	pktlen = 0;

	for (d = bp->bif_dlist; d != 0; d = d->bd_next) {
		BPF_LOCK(d);
		++d->bd_rcount;
		BPF_UNLOCK(d);
		switch (d->bd_bif->bif_dlt) {
		
		case IFT_ISO88025:
			if ((m1 = m_gethdr(M_DONTWAIT, MT_HEADER)) == NULL) {
				BPF_LOCK(d);
				++d->bd_dcount;
				BPF_UNLOCK(d);
				return;
			}
			ie5_macp = mtod((struct mbuf *)m, struct ie5_mac_hdr *);
			mac_len = mac_size(ie5_macp); 
			m1->m_next = m;
			bcopy(mtod(m,caddr_t), mtod(m1,caddr_t), mac_len);
			m->m_data += mac_len;
			m->m_len -= mac_len;
			m1->m_len = 32; /* maximum possible mac size */
			pktlen = m1->m_pkthdr.len = 
				(m->m_pkthdr.len - mac_len) + 32;
			slen = bpf_filter(d->bd_filter, (u_char *)m1, pktlen, 0);
			m1->m_next = NULL;
			m_freem(m1);
			m->m_len += mac_len;
			m->m_data -= mac_len;
			break;

		case IFT_FDDI:
			if ((m1 = m_gethdr(M_DONTWAIT, MT_HEADER)) == NULL) {
				BPF_LOCK(d);
				++d->bd_dcount;
				BPF_UNLOCK(d);
				return;
			}
			macf = mtod((struct mbuf *)m, struct fddi_mac_hdr *);
			mac_len = mac_size_f(macf); 
			m1->m_next = m;
			bcopy(mtod(m,caddr_t), mtod(m1,caddr_t), mac_len);
			m->m_data += mac_len;
			m->m_len -= mac_len;
			m1->m_len = 46; /* maximum possible mac size */
			pktlen = m1->m_pkthdr.len = 
				(m->m_pkthdr.len - mac_len) + 46;
			slen = bpf_filter(d->bd_filter, (u_char *)m1, pktlen, 0);
			m1->m_next = NULL;
			m_freem(m1);
			m->m_len += mac_len;
			m->m_data -= mac_len;
			break;

		default:
			pktlen = m->m_pkthdr.len;
			slen = bpf_filter(d->bd_filter, (u_char *)m, pktlen, 0);
		}

		pktlen = m->m_pkthdr.len;
		if (slen != 0)
			catchpacket(d, (u_char *)m, pktlen, slen, bpf_mcopy);
	}
}

/*
 * Move the packet data from interface memory (pkt) into the
 * store buffer. 
 */
static void
catchpacket(d, pkt, pktlen, snaplen, cpfn)
	register struct bpf_d *d;
	register u_char *pkt;
	register u_int pktlen, snaplen;
	register void (*cpfn)();
{
	register struct bpf_hdr *hp;
	register int totlen, curlen, hdrlen;
	register struct ie5_mac_hdr *ie5_macp;
	register struct fddi_mac_hdr *macf;
	BPF_LOCK_DECL()

	switch (d->bd_bif->bif_dlt) {
	
	case IFT_ISO88025:
		ie5_macp = mtod((struct mbuf *)pkt, struct ie5_mac_hdr *);
		hdrlen = (mac_size(ie5_macp) + sizeof(struct ie2_llc_snaphdr));
		break;
	case IFT_FDDI:
		macf = mtod((struct mbuf *)pkt, struct fddi_mac_hdr *);
		hdrlen = (mac_size_f(macf) + sizeof(struct ie2_llc_snaphdr));
		break;
	default:
		hdrlen = d->bd_bif->bif_hdrlen; 
	}
	/*
	 * Figure out how many bytes to move.  If the packet is
	 * greater or equal to the snapshot length, transfer that
	 * much.  Otherwise, transfer the whole packet (unless
	 * we hit the buffer size limit).
	 */
	BPF_LOCK(d);
	totlen = hdrlen + MIN(snaplen, pktlen);
	if (totlen > d->bd_bufsize)
		totlen = d->bd_bufsize;
	/*
	 * Round up the end of the previous packet to the next longword.
	 */
	curlen = BPF_WORDALIGN(d->bd_slen);
	if (d->bd_immediate) {
		/*
		 * Immediate mode is set.  A packet arrived so any
		 * reads should be woken up.
		 */
		bpf_wakeup(d);
	} 
	if (curlen + totlen > d->bd_bufsize) {
		/*
		 * This packet will overflow the storage buffer.
		 * Rotate the buffers if we can, then wakeup any
		 * pending reads.
		 */
		if (d->bd_fbuf == 0) {
			/*
			 * We haven't completed the previous read yet,
			 * so drop the packet.
			 */
			++d->bd_dcount;
			BPF_UNLOCK(d);
			return;
		}
		ROTATE_BUFFERS(d);
		bpf_wakeup(d);
		curlen = 0;
	}

	/*
	 * Append the bpf header.
	 */
	hp = (struct bpf_hdr *)(d->bd_sbuf + curlen);
#ifdef  _AIX
        curtime(&hp->bh_tstamp);
#else
#if BSD >= 199103
	microtime(&hp->bh_tstamp);
#elif defined(sun)
	uniqtime(&hp->bh_tstamp);
#else
	hp->bh_tstamp = time;
#endif
#endif	/* _AIX */
	hp->bh_datalen = pktlen;
	hp->bh_hdrlen = hdrlen;
	/*
	 * Copy the packet data into the store buffer and update its length.
	 */

	(*cpfn)(pkt, (u_char *)hp + hdrlen, (hp->bh_caplen = totlen - hdrlen));
	d->bd_slen = (curlen + BPF_WORDALIGN(totlen));
	BPF_UNLOCK(d);
}

/*
 * Initialize all nonzero fields of a descriptor.
 */
static int
bpf_allocbufs(d)
	register struct bpf_d *d;
{
	d->bd_fbuf = (caddr_t)malloc(d->bd_bufsize, M_TEMP, M_WAITOK);
	if (d->bd_fbuf == 0)
		return (ENOBUFS);

	d->bd_sbuf = (caddr_t)malloc(d->bd_bufsize, M_TEMP, M_WAITOK);
	if (d->bd_sbuf == 0) {
		free(d->bd_fbuf, M_TEMP);
		return (ENOBUFS);
	}
	d->bd_slen = 0;
	d->bd_hlen = 0;
	return (0);
}

/*
 * Free buffers currently in use by a descriptor.
 * Called on close.
 */
static void
bpf_freed(d)
	register struct bpf_d *d;
{
	/*
	 * We don't need to lock out interrupts since this descriptor has
	 * been detached from its interface and it yet hasn't been marked
	 * free.
	 */
	if (d->bd_sbuf != 0) {
		free(d->bd_sbuf, M_TEMP);
		if (d->bd_hbuf != 0)
			free(d->bd_hbuf, M_TEMP);
		if (d->bd_fbuf != 0)
			free(d->bd_fbuf, M_TEMP);
	}
	if (d->bd_filter)
		free((caddr_t)d->bd_filter, M_TEMP);

	D_MARKFREE(d);
}

/*
 * Attach an interface to bpf.  driverp is a pointer to a (struct bpf_if *)
 * in the driver's softc; dlt is the link layer type; hdrlen is the fixed
 * size of the link header (which is of little value for variable length headers).
 */
void
bpfattach(driverp, ifp, dlt, hdrlen)
	caddr_t *driverp;
	struct ifnet *ifp;
	u_int dlt, hdrlen;
{
	struct bpf_if *bp;
	struct mbuf *mbp;
	int i;
#if BSD < 199103
	static struct bpf_if bpf_ifs[NBPFILTER];
	static int bpfifno;

	bp = (bpfifno < NBPFILTER) ? &bpf_ifs[bpfifno++] : 0;
#else
	bp = (struct bpf_if *)malloc(sizeof(*bp), M_TEMP, M_WAIT);
#endif
	if (bp == 0)
		panic("bpfattach");

	bp->bif_dlist = 0;
	bp->bif_driverp = (struct bpf_if **)driverp;
	bp->bif_ifp = ifp;
	bp->bif_dlt = dlt;

	bp->bif_next = bpf_iflist;
	bpf_iflist = bp;

	*bp->bif_driverp = -1;

	/*
	 * Compute the length of the bpf header.  This is not necessarily
	 * equal to SIZEOF_BPF_HDR because we want to insert spacing such
	 * that the network layer header begins on a longword boundary (for
	 * performance reasons and to alleviate alignment restrictions).
	 */
	bp->bif_hdrlen = BPF_WORDALIGN(hdrlen + SIZEOF_BPF_HDR) - hdrlen;

	/*
	 * Mark all the descriptors free if this hasn't been done.
	 */
	if (!D_ISFREE(&bpf_dtab[0]))
		for (i = 0; i < NBPFILTER; ++i)
			D_MARKFREE(&bpf_dtab[i]);

	bsdlog(0, "bpf: %s%d attached\n", ifp->if_name, ifp->if_unit);
}

#if BSD >= 199103
/*
 * Set/clear promiscuous mode on interface ifp based on the truth value
 * of pswitch.  The calls are reference counted so that only the first
 * "on" request actually has an effect, as does the final "off" request.
 * Results are undefined if the "off" and "on" requests are not matched.
 * 
 * For AIX, promiscuous mode set/clear are done via CDLI Network Services.
 */
int
ifpromisc(ifp, pswitch)
	struct ifnet *ifp;
	int pswitch;
{
	int error;
	struct ifreq ifr;
	struct ndd *nddp;
	char	str[BUFSIZE];
	IFQ_LOCK_DECL()

	/*
	 * If the device is not configured up, we cannot put it in
	 * promiscuous mode.
	 */
	if ((ifp->if_flags & IFF_UP) == 0)
		return (ENETDOWN);
	if (ifp->if_type == IFT_LOOP)
		return(0);

#ifndef	_AIX
	if (pswitch) {
		if (ifp->if_pcount++ != 0)
			return (0);
		ifp->if_flags |= IFF_PROMISC;
	} else {
		if (--ifp->if_pcount > 0)
			return (0);
		ifp->if_flags &= ~IFF_PROMISC;
	}
	ifr.ifr_flags = ifp->if_flags;
	return ((*ifp->if_ioctl)(ifp, SIOCSIFFLAGS, (caddr_t)&ifr));
#else
	(void) make_ifname(ifp->if_name, ifp->if_unit, str);
	if (ns_alloc(str, &nddp))
		return(ENODEV);
	if (pswitch) {
		if(error = ((*nddp->ndd_ctl)(nddp, NDD_PROMISCUOUS_ON, 0,0))) {
			IFQ_LOCK(&(ifp->if_snd));
			ifp->if_flags |= IFF_PROMISC;
			IFQ_UNLOCK(&(ifp->if_snd));
		}
	} else {
		error = ((*nddp->ndd_ctl)(nddp, NDD_PROMISCUOUS_OFF, 0,0));
		if (!(nddp->ndd_flags & NDD_PROMISC)) {
			IFQ_LOCK(&(ifp->if_snd));
			ifp->if_flags &= ~IFF_PROMISC;
			IFQ_UNLOCK(&(ifp->if_snd));
		}
	}
	ns_free(nddp);
	return(error);
#endif	/* _AIX */
}
#endif

#if BSD < 199103
/*
 * Allocate some memory for bpf.  This is temporary SunOS support, and
 * is admittedly a hack.
 * If resources unavaiable, return 0.
 */
static caddr_t
bpf_alloc(size, canwait)
	register int size;
	register int canwait;
{
	register struct mbuf *m;

	if ((unsigned)size > (MCLBYTES-8))
		return 0;

	MGET(m, canwait, MT_DATA);
	if (m == 0)
		return 0;
	if ((unsigned)size > (MLEN-8)) {
#if BSD >= 199103
                MCLGET(m, M_WAIT);
                if ((m->m_flags & M_EXT) == 0) {
#else
                MCLGET(m);
                if (m->m_len != MCLBYTES) {
#endif
			m_freem(m);
			return 0;
		}
	}
	*mtod(m, struct mbuf **) = m;
	return mtod(m, caddr_t) + 8;
}
#endif

#ifdef _AIX
void
bpf_cdli_tap(nddp, m, hp, arg)
struct	ndd	*nddp;
struct	mbuf	*m;
caddr_t		hp;
caddr_t		arg;
{
	bpf_aix_mtap(arg, m);
}

void
bpf_aix_tap(op, ifp, bp)
int		op;
struct ifnet	*ifp;
struct bpf_if	*bp;
{
	struct	ns_user	ns_user;
	struct	ns_8022	filter;
	struct 	ndd	*nddp;
	char		ifname[20];
	int		error;
	char		str[BUFSIZE];
	IFQ_LOCK_DECL()

	if (ifp->if_type == IFT_LOOP) {
		if (op) {
			IFQ_LOCK(&(ifp->if_snd));
			ifp->if_tapctl = (caddr_t)bp;
			ifp->if_tap = (void *)bpf_aix_mtap;
			IFQ_UNLOCK(&(ifp->if_snd));
		}
		else {
			IFQ_LOCK(&(ifp->if_snd));
			ifp->if_tap = NULL;
			ifp->if_tapctl = NULL;
			IFQ_UNLOCK(&(ifp->if_snd));
		}
		return;
	}

	make_ifname(ifp->if_name, ifp->if_unit, str); 
	if(num_bpf_d(str) > 1)
		return;

	if (str[0] == 'e' && str[1] == 't')
		str[1] = 'n'; /* boy o'boy */

	if (error = ns_alloc(str, &nddp)) {
		bsdlog(0, "bpf: ns_alloc() failed! [0x%x] on %s.\n", error, str);
		return;
	}


	bzero(&filter, sizeof(filter));
	bzero(&ns_user, sizeof(ns_user));
	filter.filtertype = NS_TAP; 
	ns_user.isr_data = (caddr_t)bp;
	ns_user.isr = bpf_cdli_tap;
	ns_user.protoq = (struct ifqueue *)NULL;
	ns_user.netisr = 0;
	ns_user.pkt_format = NS_INCLUDE_MAC;
	if (op) {
		if(!(error = ns_add_filter(nddp, &filter, sizeof(filter), 
					   &ns_user))) {
			NDD_LOCK(nddp);
			nddp->ndd_trace_arg = (caddr_t)bp;
			nddp->ndd_trace = bpf_cdli_tap;
			NDD_UNLOCK(nddp);
		}
	}
	else {
		if(!(error = ns_del_filter(nddp, &filter, sizeof(filter), 
					   &ns_user))) {
			NDD_LOCK(nddp);
			nddp->ndd_trace = NULL;
			nddp->ndd_trace_arg = NULL;
			NDD_UNLOCK(nddp);
		}
	}
	if (error)
		bsdlog(0, "bpf: ns_%s_filter failed on %s", 
			(op) ? "add" : "delete", str);
	ns_free(nddp);
}


void
make_ifname(name, unit, str)
char	*name;
int	unit;
char	*str;
{
	char 	*cp1, *cp2;
	int 	i = 0;
	int	j,k;
 
	if (!name[0] || (!strcpy(str,name)))
		str[0] = '\0';
	else {
		cp1 = str;
		while (*cp1) cp1++;
		cp2 = cp1;
		do {
			cp1[i++] = unit % 10 + '0';
		} while ((unit /= 10) > 0);
		cp1[i] = '\0';
		for (i = 0, j = strlen(cp2)-1; i < j; i++, j--) {
			k = cp2[i];
			cp2[i] = cp2[j];
			cp2[j] = k;
		}
	}
}
#endif /* _AIX */
