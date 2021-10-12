static char sccsid[] = "@(#)03	1.8  src/bos/kernel/net/aix_misc.c, sysnet, bos412, 9443A412c 10/20/94 15:51:48";
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: add_input_type
 *		add_netisr
 *		add_netoption
 *		aix_netinit
 *		del_input_type
 *		del_netisr
 *		delete_netopt
 *		find_input_type
 *		net_sleep
 *		net_tap
 *		net_wakeup
 *		netsleepinit
 *		netsqhash
 *		splhi
 *		splimp
 *		splnet
 *		splx
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "net/net_globals.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/sleep.h"
#include "sys/errno.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/intr.h"
#include "sys/pri.h"
#include "sys/ioctl.h"
#include "sys/nettrace.h"

#include <sys/mbuf.h>
#include <net/netisr.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/spl.h>
#include <net/netopt.h>

#include <aixif/net_if.h>
#include <netinet/if_ether.h>

void	netsleepinit();			/* initialize network sleep stuff  */

/* sb_max option settable by no cmd limits the socketbuffer size. */
struct netopt sb_max_opt;
extern u_long sb_max;
extern u_long sb_max_dflt;

/* tcp retransmission exponential backoff options settable by no cmd */
int rto_low = RTO_DFLT_LOW;
int rto_low_dflt = RTO_DFLT_LOW;
int rto_high = RTO_DFLT_HIGH;
int rto_high_dflt = RTO_DFLT_HIGH;
int rto_limit = RTO_DFLT_LIMIT;
int rto_limit_dflt = RTO_DFLT_LIMIT;
int rto_length = RTO_DFLT_LENGTH;
int rto_length_dflt = RTO_DFLT_LENGTH;

/* confurable arp table size options settable by no cmd */
int arptab_bsiz	= ARPTAB_BSIZ;
int arptab_bsiz_dflt = ARPTAB_BSIZ;
int arptab_nb = ARPTAB_NB;
int arptab_nb_dflt = ARPTAB_NB;
/* non-configurable arp table size variables used internaly. */
/* these had to be put here because of io/dumpdd_pg.c	     */
int arptabbsiz	= ARPTAB_BSIZ;
int arptabnb = ARPTAB_NB;
int arptabsize = (ARPTAB_BSIZ * ARPTAB_NB);
struct arptab *arptabp;
/* configurable tcp debug table size option settable by no cmd */
int tcp_ndebug = TCP_NDEBUG;
int tcp_ndebug_dflt = TCP_NDEBUG;
/* configurable ifnet softc's option settable by the no cmd */
int ifsize = IF_SIZE;
int ifsize_dflt = IF_SIZE;

int             wildcard_in_table = 0;  /* wildcard present flag           */
struct nit_ent	nit_table[MAX_NITS];	/* Network Input Table (NIT)	   */

simple_lock_data_t	nit_lock;
#define 		NIT_LOCK_DECL() 	int	_nits;
#define 		NIT_LOCK(l)		_nits = disable_lock(PL_IMP, l);
#define 		NIT_UNLOCK(l)		unlock_enable(_nits, l);

#define THEMAX (2 * AF_MAX)
struct af_ent	af_table[THEMAX];
extern int net_malloc_police_station();

NETOPTINT(dog_ticks);
NETOPTINT(thewall);			/* see mbuf.c */

NETOPTINT(rto_low);			/* see tcp_timer.c */
NETOPTINT(rto_high);
NETOPTINT(rto_limit);
NETOPTINT(rto_length);

NETOPTINT(arptab_bsiz);
NETOPTINT(arptab_nb);

NETOPTINT(tcp_ndebug);

NETOPTINT(ifsize);

NETOPTINT(net_malloc_police);

#define	DEFAULT_HOSTNAME	"localhost"
extern int hostnamelen;
extern char hostname[];

/***************************************************************************
*
* 	aix_netinit() - init the network kproc  
*	
*	RETURNS : none	
*
***************************************************************************/
void
aix_netinit()
{
	extern int net_queued_write();

	lock_alloc(&nit_lock, LOCK_ALLOC_PIN, NETISR_LOCK_FAMILY, -1);
	simple_lock_init(&nit_lock);

	netsleepinit();
	ns_init();
	arptabp = (struct arptab *)NULL;
	hostnamelen = strlen(DEFAULT_HOSTNAME);
	bcopy(DEFAULT_HOSTNAME, hostname, hostnamelen);

	/* Add user modifiable kernel options here */
	ADD_NETOPT(sb_max, "%d");

	ADD_NETOPT(net_malloc_police, "%d");
	net_malloc_police_opt.init = net_malloc_police_station;

	ADD_NETOPT(rto_low, "%d");
	ADD_NETOPT(rto_high, "%d");
	ADD_NETOPT(rto_limit, "%d");
	ADD_NETOPT(rto_length, "%d");

	ADD_NETOPT(arptab_bsiz, "%d");
	ADD_NETOPT(arptab_nb, "%d");

	ADD_NETOPT(tcp_ndebug, "%d");

	ADD_NETOPT(ifsize, "%d");

	(void) netisr_add(NETISR_WRITE, net_queued_write,
				(struct ifqueue *)0, (struct domain *)0);
}

#ifdef	_MOVE_TO_INTF
/***************************************************************************
*
*	
*	RETURNS :	
*			0      - no errors 
*
***************************************************************************/
void
net_tap(nddp, m, macp, ifp)
	struct ndd	*nddp;		/* network interface	*/
	struct mbuf	*m;		/* input packet		*/
	caddr_t		macp;
	u_char		dir_flag;	/* transmit or receive pkt */
{
	struct	mbuf	*n;		/* copy of m. It's passed up	*/
	struct	mbuf	*n0;		/* control header		*/
	struct	packet_trace_header	*pth;
	static	struct  sockaddr rawdst = { sizeof(rawdst), AF_INTF };
	static	struct  sockaddr rawsrc = { sizeof(rawsrc), AF_INTF };
	static	struct  sockproto intfproto = { PF_INTF };

	n0 = m_gethdr(M_DONTWAIT, MT_HEADER);
 	if (!n0)
		goto out;

	pth = mtod(n0, struct packet_trace_header *);

	n = m_copy...
	if (!n) {
		m_freem(n0);
		return;
	}

	n0->m_next = n;
	n0->m_len = sizeof(struct packet_trace_header);
	n0->m_pkthdr.rcvif = ifp;
	n0->m_pkthdr.len = n0->m_len + n->m_pkthdr.len;
	n->m_flags &= ~M_PKTHDR;

	bcopy((caddr_t)nddp->ndd_name, (caddr_t)pth->ifname, 
		sizeof(pth->ifname));
	pth->unit = (u_char)nddp->ndd_unit;
	pth->iftype = nddp->ndd_type;
	pth->xmit_pkt = (u_char)xmit_flag;

	n0 = m_pullup(n0, MIN(max_hdr, n->m_pkthdr.len));
	if (!n0)
		return;
		
	raw_input(n0, &intfproto, (struct sockaddr *)&rawsrc,
	  (struct sockaddr *)&rawdst);

	return;
}
#endif

int			nethsque[NETNHSQUE];

/*****************************************************************************
 * NAME:  netsleepinit
 *
 * FUNCTION:  Initialize nethsque
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * NOTES:  This routine initializes each element of the event-list anchor
 *         array to EVENT_NULL.
 *****************************************************************************/
void
netsleepinit()
{
	register int i;

	for (i = 0; i < NETNHSQUE; i++)
		nethsque[i] = EVENT_NULL;
}

/***************************************************************************
*
* 	net_sleep() - sleep (the network way!)
*		      liberally adapted from sleep.c
*
* EXECUTION ENVIRONMENT:
*      This procedure can only be called by a process.
*
* RETURNS: 1 if signalled, 0 otherwise
*
***************************************************************************/
net_sleep(chan, flags)
int chan;		/* wait channel                */
int flags;		/* signal control flags        */
{
	register	rc;		/* e_sleepl return code        	*/
	register	sflags;		/* e_sleepl flags              	*/
	
	if (flags & PCATCH | ((flags & PMASK) > PZERO) )
		sflags = EVENT_SIGRET;
	else
	        sflags = EVENT_SHORT;

	rc = e_sleep(netsqhash(chan), sflags);
	if (rc == EVENT_SIG)
		return(EINTR);
	else 
		return(0);
}

/***************************************************************************
*
* 	net_wakeup() -  wakeup sleeping network processes
*			wakes everyone on hash
*
* EXECUTION ENVIRONMENT:
*      This procedure can be called by a process or interrupt handler.
*
***************************************************************************/
net_wakeup(chan)
int		chan;
{
	e_wakeup(netsqhash(chan));
}

/*
 * NAME: add_netoption
 *
 * FUNCTION:  to add a netopt struct to end of list of 'netopts'
 *
 * RETURNS:  nothing.
 */
add_netoption(newopt)
	struct netopt *newopt;
{
	struct netopt *np = netopts;

	if (np) {
		while (np->next)  np = np->next;
		np->next = newopt;
	} else
		netopts = newopt;
}
/*
 *
 * FUNCTION:  removes a network option from the linked list of options.
 *
 * RETURNS:  nothing.
 */
void
delete_netopt(optaddr)
	struct netopt *optaddr;
{
	struct netopt **prevp = &netopts, *np = netopts;
	
	/* look for option in chain of network options */
	while (np && np != optaddr) {
		prevp = &np->next;
		np = np->next;
	}

	/* and if option was found, then remove it */
	if (np)
		*prevp = np->next;
}

#undef	splnet
#undef	splimp
#undef	splhi
#undef	splhigh
#undef	splx

/***************************************************************************
*
* 	splnet() -  
*
* EXECUTION ENVIRONMENT:
*      This procedure can be called by a process or interrupt handler.
*
***************************************************************************/
splnet()
{
	return(i_disable(PL_NET));
}

/***************************************************************************
*
* 	splimp() -  
*
* EXECUTION ENVIRONMENT:
*      This procedure can be called by a process or interrupt handler.
*
***************************************************************************/
splimp()
{
	return(i_disable(PL_IMP));
}

/***************************************************************************
*
* 	splhi() -  
*
* EXECUTION ENVIRONMENT:
*      This procedure can be called by a process or interrupt handler.
*
***************************************************************************/
splhi()
{
	return(i_disable(PL_HI));
}

splhigh()
{
	return(i_disable(PL_HI));
}

/***************************************************************************
*
* 	splx() -  
*
* EXECUTION ENVIRONMENT:
*      This procedure can be called by a process or interrupt handler.
*
***************************************************************************/
splx(x)
int	x;
{
	i_enable(x);
}

/***************************************************************************
*
*	find_input_type() - Searches for a packet type in the Network
*			    Input Table (NIT). If the type is found
*			    the mbuf is either ENQUEUEd or a function
*			    is called directly with the mbuf.
*	
*	RETURNS :	
*			0      - no errors 
*			ENOENT - type not found
*
***************************************************************************/
find_input_type(type, m, ac, hp)
	u_short		type;		/* type from packet	*/
	struct mbuf	*m;		/* input packet		*/
	struct arpcom	*ac;		/* interface common portion */
	caddr_t 	*hp;		/* pointer to link-level header */
{
	int		i = 0;
	IFQ_LOCK_DECL()
	NIT_LOCK_DECL()

        if (wildcard_in_table)
                wild_type_dispatch(type, m, &ac->ac_if, hp, 0);

	NIT_LOCK(&nit_lock);
	while ( i != MAX_NITS && !(nit_table[i].type == type && 
                nit_table[i].used) ) {
		i++;
	    }

	if (i >= MAX_NITS) {		/* type nowhere to be found	*/
		m_freem(m);
		ac->ac_if.if_noproto++;
		NIT_UNLOCK(&nit_lock);
		return(ENOENT);
	}
 	
	if (nit_table[i].ifq_addr == NULL) {
		(*(nit_table[i].handler))(ac, m, hp);
	}
	else {
		IFQ_LOCK(nit_table[i].ifq_addr);
		/* if not full, enqueue and schednetisr	*/
		if (IF_QFULL(nit_table[i].ifq_addr)) {
			IF_DROP(nit_table[i].ifq_addr);
			m_freem(m);
		} else {
			IF_ENQUEUE_NOLOCK(nit_table[i].ifq_addr, m);
		}
		IFQ_UNLOCK(nit_table[i].ifq_addr);
		schednetisr(nit_table[i].af);
	}
	NIT_UNLOCK(&nit_lock);
	return(0);
}

/***************************************************************************
*
*	add_input_type() - Adds a packet type to the Network Input Table(NIT).
*	
*	RETURNS :	
*			0      - no errors 
*			EEXIST - type already in table with different isr
*			ENOSPC - no room in NIT table
*
***************************************************************************/
add_input_type(type, service_level, isr, ifq_addr, af)
	u_short		type;		/* type of packet, e.g. IP, arp...  */
	u_short		service_level;	/* call direct or schedule	    */
	int		(*isr)();	/* input handler for this type 	    */
	struct ifqueue	*ifq_addr;	/* ^queue, if pkt is to be enqueued */
	u_short		af;		/* address family number of caller  */
{
	int		rc, i, x;
	int		free = -1;	/* first free slot found in table   */
	int		error;
	struct ifnet	*ifp;
	NIT_LOCK_DECL()

	if ((ifq_addr == NULL) && (service_level == NET_KPROC))
		return(EINVAL);
	if ((type == NET_WILD_TYPE) && (service_level != NET_KPROC))
		return(EINVAL);
	if ((service_level != NET_OFF_LEVEL) && (service_level != NET_KPROC))
		return(EINVAL);
	if (isr == NULL)
		return(EINVAL);
	if ((af < 0) || (af >= NETISR_MAX))
		return(EINVAL);

	NIT_LOCK(&nit_lock);
	for ( i = 0 ; i < MAX_NITS ; i++ ) {
		if ((nit_table[i].used == 0) && (free == -1))
			free = i;
	
		if (nit_table[i].type == type) {  /* already in */
			if ( (nit_table[i].ifq_addr == NULL) &&
			     (nit_table[i].handler == isr) ) {
				NIT_UNLOCK(&nit_lock);
				return(EEXIST);
			}
		
			if ( (error = add_netisr(af, service_level, isr)) ) {
				NIT_UNLOCK(&nit_lock);
				return(error);
			}
			
			nit_table[i].ref_cnt++;
			NIT_UNLOCK(&nit_lock);
			return(0);
		}
	}

	if (free == -1) {		/* won't fit	*/
		NIT_UNLOCK(&nit_lock);
		return(ENOSPC);
	}

	/* fill out the slot	*/
	if (ifq_addr != NULL) {
		error = netisr_add(af, isr, NULL, NULL);
		if (error) {
			NIT_UNLOCK(&nit_lock);
			return(error);
		}
	}

	nit_table[free].handler  = isr;
	nit_table[free].used = 1;
	nit_table[free].ref_cnt = 1;
	nit_table[free].type = type;
	nit_table[free].ifq_addr = ifq_addr;
	nit_table[free].af = af;

        if (type == NET_WILD_TYPE) {
                wildcard_in_table = 1;
                NIT_UNLOCK(&nit_lock);
                return(0);
        }
	
	NIT_UNLOCK(&nit_lock);
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_ioctl)
			if (rc = (*ifp->if_ioctl)(ifp, SIOCADDNETID, &type))
				if (rc != EINVAL)
					return (rc);
	}

	return(0);
}

/***************************************************************************
*
*	del_input_type() - Deletes a type from the Network Input Table(NIT).
*	
*	RETURNS :	
*			0      - no errors 
*			ENOENT - the type was not found
*
***************************************************************************/
del_input_type(type)
	u_short		type;		/* input packet type */
{
	int		i = 0, x;
	NIT_LOCK_DECL()

	NIT_LOCK(&nit_lock);
	while ( (i < MAX_NITS) && (nit_table[i].type != type) )
		i++;

	if (i >= MAX_NITS) {
		NIT_UNLOCK(&nit_lock);
		return(ENOENT);		/* type not found */
	}

	/* Since more than one protocol may register the same type
	 * (as long as the input handler is the same), a reference
	 * count is kept to keep track of the number of users. The
	 * entry is removed only when the reference count goes to
	 * zero.
	 */
	nit_table[i].ref_cnt--;
	if (nit_table[i].ref_cnt <= 0) {
		nit_table[i].used = 0;
		nit_table[i].ref_cnt = 0;
		nit_table[i].type = 0;
		nit_table[i].handler = NULL;
		if (nit_table[i].ifq_addr != NULL)
			netisr_del(nit_table[i].af);
			
		nit_table[i].ifq_addr = NULL;
		if (type == NET_WILD_TYPE)
			wildcard_in_table = 0;

	}

	NIT_UNLOCK(&nit_lock);
	return(0);
}

wild_type_dispatch(type, m, ifp, hp, xmit_flag)
	u_short		type;		/* type from packet	*/
	struct mbuf	*m;		/* input packet		*/
	struct ifnet	*ifp;		/* network interface	*/
	caddr_t 	*hp;		/* physical header	*/
	u_char		xmit_flag;	/* transmit or receive pkt */
{
	int		i = 0, x;
	struct	mbuf	*n;		/* copy of m. It's passed up	*/
	struct	mbuf	*n0;		/* control header		*/
	struct	packet_trace_header	*pth;
	IFQ_LOCK_DECL()
	NIT_LOCK_DECL()

	NIT_LOCK(&nit_lock);
	while ((i < MAX_NITS) && (nit_table[i].type != NET_WILD_TYPE))
		i++;

	if (i >= MAX_NITS) {
		NIT_UNLOCK(&nit_lock);
		return(0);
	}

	n0 = m_gethdr(M_DONTWAIT, MT_HEADER);
 	if (!n0) {
		NIT_UNLOCK(&nit_lock);
		return(0);
	}

	pth = mtod(n0, struct packet_trace_header *);
	pth->type = type;
	if (hp != NULL)
		pth->hlen = (int) (mtod(m, caddr_t) - hp);
	else
		pth->hlen = 0;

	/* XXX - fake out. make sure headers are copied */
	m->m_data -= pth->hlen;
	m->m_len += pth->hlen;

	n = m_copym(m, 0, M_COPYALL, M_DONTWAIT);

	/* XXX - now put them back where they should be */
	m->m_data += pth->hlen;
	m->m_len -= pth->hlen;

 	if (!n) {
		NIT_UNLOCK(&nit_lock);
		m_freem(n0);
		return(0);
	}

	n0->m_next = n;
	n0->m_len = sizeof(struct packet_trace_header);
	n0->m_pkthdr.rcvif = ifp;
	n0->m_pkthdr.len = n0->m_len + n->m_pkthdr.len;
	n->m_flags &= ~M_PKTHDR;

	bcopy((caddr_t)ifp->if_name, (caddr_t)pth->ifname, IFNAMSIZ);
	pth->unit = (u_char)ifp->if_unit;
	if ((ifp->if_type == IFT_ETHER) || (ifp->if_type == IFT_ISO88023))
		/* boy oh'boy */
		if (mtod(m, struct ether_header *)->ether_type <= ETHERMTU) {
			pth->iftype = IFT_ISO88023;
			pth->ifname[1] = 't'; 
		} else {
			pth->iftype = IFT_ETHER;
			pth->ifname[1] = 'n'; 
		}
	else
		pth->iftype = ifp->if_type;
	pth->xmit_pkt = (u_char)xmit_flag;

	n0 = m_pullup(n0, MIN(max_hdr, n->m_pkthdr.len));
	if (!n0) {
		NIT_UNLOCK(&nit_lock);
		return(0);
	}
		
	/* if not full, enqueue and schednetisr	*/
	IFQ_LOCK(nit_table[i].ifq_addr);
	if (IF_QFULL(nit_table[i].ifq_addr)) {
		IF_DROP(nit_table[i].ifq_addr);
		m_freem(n0);
	} else {
		IF_ENQUEUE_NOLOCK(nit_table[i].ifq_addr, n0);
		schednetisr(nit_table[i].af);
	}
	IFQ_UNLOCK(nit_table[i].ifq_addr);
	NIT_UNLOCK(&nit_lock);
}

/***************************************************************************
*
*	add_netisr() -  Adds a network software interrupt service routine to
*			the Network Software Interrupt Table.
*	
*	RETURNS :	
*			0      - no errors 
*			EEXIST - interrupt level already in table
*			EINVAL - parameters error
*
*
***************************************************************************/
add_netisr(soft_intr_level, service_level, isr)
	u_short		soft_intr_level;	/* software interrupt number */
	u_short		service_level;		/* call direct or schedule   */
	int		(*isr)();		/* interrupt service routine */
{

	if ((service_level != NET_OFF_LEVEL) && (service_level != NET_KPROC))
		return(EINVAL);
	return(netisr_add(soft_intr_level, isr, NULL, NULL));
}

/***************************************************************************
*
*	del_netisr() -  Deletes a network software interrupt service routine 
*			from the Network Software Interrupt Table.
*	
*	RETURNS :	
*			0      - no errors 
*			EINVAL - soft_intr_level out of range
*
*
***************************************************************************/
del_netisr(soft_intr_level)
	u_short		soft_intr_level;	/* software interrupt number */
{
	return(netisr_del(soft_intr_level));
}
