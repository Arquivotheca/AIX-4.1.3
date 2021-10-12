static char sccsid[] = "@(#)07  1.4  src/bos/usr/ccs/lib/libc/authdes_subr.c, libcrpc, bos41J, 9511A_all 3/3/95 09:51:05";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: atoa
 *		debug
 *		getnetname
 *		rtime
 *		rtime_wakeup
 *		sitoa
 *		
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)authdes_subr.c	1.6 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.8 88/02/08
 */

#ifdef _KERNEL

/*
 * Miscellaneous support routines for kernel implentation of AUTH_DES
 */

/*
 *  rtime - get time from remote machine
 *
 *  sets time, obtaining value from host
 *  on the udp/time socket.  Since timeserver returns
 *  with time of day in seconds since Jan 1, 1900,  must
 *  subtract 86400(365*70 + 17) to get time
 *  since Jan 1, 1970, which is what get/settimeofday
 *  uses.
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mbuf.h>
#include <sys/socketvar.h>
#include <sys/intr.h>
#include <errno.h>
#include <sys/user.h>
#include <sys/cred.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <nfs/nfs_trc.h>

#define debug(msg)			/* turn off debugging */

#define TOFFSET (86400*(365*70 + (70/4)))
#define WRITTEN (86400*(365*86 + (86/4)))

rtime(addrp, timep, wait)
	struct sockaddr_in *addrp;
	struct timeval *timep;
	struct timeval *wait;
{
	struct socket *so;
	struct mbuf *m;
	int error;
	u_long thetime;
	struct sockaddr_in from;
	int s;
	struct mbuf *ku_recvfrom();

	error = socreate(AF_INET, &so, SOCK_DGRAM, IPPROTO_UDP);
	if (error) {
		return(-1);
	}
	addrp->sin_family = AF_INET;
	addrp->sin_port = htons(IPPORT_TIMESERVER);

	m = m_get(M_WAIT, MT_DATA);
	if (m == NULL) {
		(void) soclose(so);
		return(-1);
	}
	m->m_len = 0;
	
	error = ku_sendto_mbuf(so, m, addrp, (struct socket *)NULL);
	/* m is now free */
	if (error) {
		TRCHKL1(HKWD_NFS_RPC_DEBUG|hkwd_KDES_RTIME_FAIL_SEND, error);
		(void) soclose(so);
		return(-1);
	}

	SOCKET_LOCK(so);
	so->so_error = 0;
	if (error = sosblock(&so->so_rcv, so)) {
		SOCKET_UNLOCK(so);
		soclose(so);
		return(-1);
	}
	so->so_rcv.sb_flags |= SB_WAIT | SB_WAKEONE;
	so->so_rcv.sb_timeo = (wait->tv_sec * uS_PER_SECOND) + wait->tv_usec;
	
	error = 0;
	while (so->so_rcv.sb_cc == 0 && !error)
		error = sosbwait(&so->so_rcv, so);

	so->so_rcv.sb_flags &= ~SB_WAKEONE;

	if (error) {
		so->so_error = error;
		sbunlock(&so->so_rcv);
		SOCKET_UNLOCK(so);
		TRCHKL1(HKWD_NFS_RPC_DEBUG|hkwd_KDES_RTIME_FAIL_RECV, 
			so->so_error);
		(void) soclose(so);
		return(-1);	
	}

	sbunlock(&so->so_rcv);
	SOCKET_UNLOCK(so);

	/* ku_recvfrom will take care of its own serialization */
	m = ku_recvfrom(so, &from);

	(void) soclose(so);

	if (m == NULL) {
		debug("recvfrom");
		return(-1);
	}
	if (m->m_len != sizeof(u_long)) {
		debug("invalid receipt time");
		m_freem(m);	
		return(-1);
	}
	thetime = ntohl(*mtod(m, u_long *));
	m_freem(m);
	if (thetime < WRITTEN) {
		debug("time returned is too far in past");
		return(-1);
	}
	timep->tv_sec = thetime - TOFFSET;
	timep->tv_usec = 0;
	return(0);
}


/*
 * Short to ascii conversion
 */
static char *
sitoa(s, i)
	char *s;
	short i;
{
	char *p;
	char *end;
	char c;

	if (i < 0) {
		*s++ = '-';		
		i = -i;
	} else if (i == 0) {
		*s++ = '0';
	}

	/*
	 * format in reverse order
	 */
	for (p = s; i > 0; i /= 10) {	
		*p++ = (i % 10) + '0';
	}
	*(end = p) = 0; 

	/*
	 * reverse
	 */
	while (p > s) {
		c = *--p;
		*p = *s;
		*s++ = c;
	}
	return(end);
}

static char *
atoa(dst, src)
	char *dst;	
	char *src;
{
	while (*dst++ = *src++)
		;
	return(dst-1);
}

/*
 * What is my network name?
 * WARNING: this gets the network name in sun unix format. 
 * Other operating systems (non-unix) are free to put something else
 * here.
 */
#define MAXDNAME 64

getnetname(netname,tcred)
	char *netname;
	struct ucred *tcred;
{
        char *p;
        char hostname[MAXHOSTNAMELEN+1];
        char domainname[MAXDNAME+1];
        int  len;

        p = atoa(netname, "unix.");
        if (tcred->cr_uid == 0) {
                len = MAXHOSTNAMELEN;
                kgethostname(hostname, &len);
                p = atoa(p, hostname);
        } else {
                p = sitoa(p, tcred->cr_uid);
        }
        *p++ = '@';
        len = MAXDNAME;
        kgetdomainname(domainname, &len);
        p = atoa(p, domainname);

}
#endif /* _KERNEL */


