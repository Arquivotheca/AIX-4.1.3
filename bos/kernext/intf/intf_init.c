static char sccsid[] = "@(#)23	1.8  src/bos/kernext/intf/intf_init.c, sysxintf, bos411, 9428A410j 4/21/94 06:48:08";
/*
 * COMPONENT_NAME: (SYSXINTF) raw interface services 
 *
 * FUNCTIONS: config_intf intf_promisc nstap cdli_compat make_ifname
 *
 * ORIGINS: 26, 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/types.h>
#include <sys/uio.h>		/* for uio struct */
#include <sys/device.h>
#include <sys/domain.h>		/* for struct domain */
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/socket.h>

#include <net/if.h>		/* include befure devinfo.h */
#include <net/netisr.h>
#include <net/netopt.h>

#include <sys/devinfo.h>	/* for DD_EN */

#include <sys/cdli.h>
#include <sys/ndd.h>
#include <net/nd_lan.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>

int netintf_kmid;		/* kernel module id, must be "mod name"_kmid */
int netintf_count = 0;		/* # of times CFG_INIT has been done */
int netintf_promisc = 0;	/* promiscous on */
lock_t cfg_lock = LOCK_AVAIL;

extern	struct domain intfdomain;
extern	struct ifqueue intfintrq;
extern  intfintr();
extern	net_xmit_trace_cdli();
extern	net_recv_trace_cdli();
extern	net_xmit_trace();

/* these functions will be imported from kernel socket stuff */
extern  add_input_type(), add_domain_af();

/* CDLI driven interfaces that netintf supports (interface name) */
static char	*cdlisup[] = {
	"en",
	"et",
	"tr",
	"fi",
	0 
};

/*
 * cdli_compat
 *
 * Given an if_name (eg, "en"), determine if it is a netintf supported
 * CDLI driven Interface.
 *
 * RETURNS:
 *	0 - if_name is NOT a netintf CDLI driven Interface.
 *	1 - if_name is a netintf CDLI driven Interface.
 */
int
cdli_compat(if_name)
char	*if_name;
{
	char	**cdlip = cdlisup;
	int	i;

	i=0;
	do {
		if (!strcmp(if_name, cdlip[i])) {
			return(1);
		}
		i++;
	} while (cdlip[i]);

	return(0);
}

/*
 * make_ifname
 *
 * Given an if_name and unit, this function creates an ifname.
 *
 * RETURNS:
 *	nothing: ifname string stored in str.
 *	         errors recored via bsdlog.
 */
void
make_ifname(name, unit, str)
char	*name;
int	unit;
char	*str;
{
char	*cp1, *cp2;
int	i = 0;
int	j,k;

	if (str == NULL) {
		bsdlog(0, "netintf: make_ifname called with str==NULL");
		return;
	}

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

/*
 * intf_promisc
 *
 * Turns ON/OFF promiscuous mode for a CDLI driven Interface.
 *
 * RETURNS:
 *	nothing: errors recorded via bsdlog.
 */
void
intf_promisc(op, ifname, ifp)
int		op;
char		*ifname;
struct ifnet	*ifp;
{
	struct	ndd	*nddp;
	int		error=0;

	IFQ_LOCK_DECL()

	if (ifname[0] == 'e' && ifname[1] == 't')
		ifname[1] = 'n'; /* boy o'boy */

	if (error = ns_alloc(ifname, &nddp)) {
		bsdlog(0, "netintf: ns_alloc() failed on [%s] error[%d]", 
			ifname, error);
		return;
	}

	if (op) {
		if (!(*nddp->ndd_ctl)(nddp, NDD_PROMISCUOUS_ON, 0,0)) {
			IFQ_LOCK(&(ifp->if_snd));
			ifp->if_flags |= IFF_PROMISC;
			IFQ_UNLOCK(&(ifp->if_snd));
		}
		else
			bsdlog(0, "netintf: promisc ON failed for [%s]", ifname);
	}
	else {
		if ((*nddp->ndd_ctl)(nddp, NDD_PROMISCUOUS_OFF, 0,0))
			bsdlog(0, "netintf: promisc OFF failed for [%s]", ifname);
		if (!(nddp->ndd_flags & NDD_PROMISC)) {
			IFQ_LOCK(&(ifp->if_snd));
			ifp->if_flags &= ~IFF_PROMISC;
			IFQ_UNLOCK(&(ifp->if_snd));
		}
	}
	ns_free(nddp);

	if (error) {
		if (op)
		  bsdlog(0, "netintf: promiscuous on failed for [%s] error[%d]",
			ifname, error);
		else
		  bsdlog(0, "netintf: promiscuous off failed for [%s] error[%d]",
			ifname, error);
	}
}

/*
 * nstap
 *
 * Add/Delete Network TAP user for a CDLI driven Interface.
 * Also, adds/deletes a trace filter.
 *
 * RETURNS:
 *	nothing: errors recorded via bsdlog.
 */
void
nstap(op, ifname, ifp)
int		op;
char		*ifname;
struct ifnet	*ifp;
{
	struct	ns_user	ns_user;
	struct	ns_8022	filter;
	struct	ndd	*nddp;
	int		error=0;

	if (ifname[0] == 'e' && ifname[1] == 't')
		ifname[1] = 'n'; /* boy o'boy */

	if (error = ns_alloc(ifname, &nddp)) {
		bsdlog(0, "netintf: ns_alloc() failed on [%s] error[%d]",
			ifname, error);
		return;
	}

	bzero(&filter, sizeof(filter));
	bzero(&ns_user, sizeof(ns_user));
	filter.filtertype = NS_TAP;
	ns_user.isr_data = (caddr_t)ifp;
	ns_user.isr = net_recv_trace_cdli;
	ns_user.protoq = (struct ifqueue *)NULL;
	ns_user.netisr = 0;
	if (op) {
		if(!(error = ns_add_filter(nddp, &filter, sizeof(filter),
				&ns_user))) {
			NDD_LOCK(nddp);
			nddp->ndd_trace_arg = (caddr_t)ifp;
			nddp->ndd_trace = net_xmit_trace_cdli;
			NDD_UNLOCK(nddp);
		}
	}
	else {
		if (!(error = ns_del_filter(nddp, &filter, sizeof(filter),
				&ns_user))) {
			NDD_LOCK(nddp);
			nddp->ndd_trace = NULL;
			nddp->ndd_trace_arg = (caddr_t)NULL;
			NDD_UNLOCK(nddp);

		}
	}
	ns_free(nddp);

	if (error) {
		if (op)
			bsdlog(0, 
			    "netintf: ns_add_filter() failed on [%s] error[%d]",
				ifname, error);
		else
			bsdlog(0, 
			    "netintf: ns_del_filter() failed on [%s] error[%d]",
				ifname, error);
	}
}

/*
 * config_intf
 *
 * entry point for netintf kernel extension.
 *

 * RETURNS:
 *	0	no problemo.
 *	EINVAL	invalid parameter of some sort (go figure).
 */
config_intf(cmd, uio)
	int cmd;
	struct uio *uio;
{
	int		err;
	struct iftrace	info;
	struct ifnet	*ifp;
	char		str[IFNAMSIZ];

	IFQ_LOCK_DECL()

	if (uio)
		uiomove((caddr_t) &info, sizeof(info), 
			UIO_WRITE, uio);

	netintf_kmid = info.kmid;

	switch (cmd) {
	case CFG_INIT:
		lockl(&cfg_lock, LOCK_SHORT);
		netintf_count++;
		if (netintf_count == 1) {

			/* pin the code	*/
			if (err = pincode(config_intf))
				return(err);

			add_domain_af(&intfdomain);
			
			/* Add intf entries to the NIT */
			add_input_type(NET_WILD_TYPE, NET_KPROC,
					intfintr, &intfintrq, AF_INTF);
			for (ifp=ifnet; ifp; ifp = (struct ifnet *)ifp->if_next) {
				if (cdli_compat(ifp->if_name)) {
					make_ifname(ifp->if_name, ifp->if_unit,
							str);
					nstap(1, str, ifp);
					if (info.promisc) {
						netintf_promisc = 1;
						intf_promisc(1, str, ifp);
					}
				}
				else {
					IFQ_LOCK(&(ifp->if_snd));
					ifp->if_tapctl = ifp;
					ifp->if_tap = net_xmit_trace;
					IFQ_UNLOCK(&(ifp->if_snd));
				}
			}
		}
		/* Turn on promiscuous upon request if it isn't already on. */
		/* The side effect here is that if more than one process is */
		/* running, it only takes one process to enable promiscuous */
		/* mode for ALL processes.  				    */
		else {
			if ((!netintf_promisc) && info.promisc) {
			    for (ifp=ifnet;ifp;ifp=(struct ifnet *)ifp->if_next) {
				if (cdli_compat(ifp->if_name)) {
				    make_ifname(ifp->if_name, ifp->if_unit, str);
					netintf_promisc = 1;
					intf_promisc(1, str, ifp);
				}
			    }
			}
		}
		unlockl(&cfg_lock);
		break;
	case CFG_TERM:
		lockl(&cfg_lock, LOCK_SHORT);
		netintf_count--;
		if (netintf_count == 0) {
			for (ifp=ifnet; ifp; ifp = (struct ifnet *)ifp->if_next) {
				if (cdli_compat(ifp->if_name)) {
				    make_ifname(ifp->if_name, ifp->if_unit, str);
				    nstap(0, str, ifp);
				    if (netintf_promisc) 
						intf_promisc(0, str, ifp);
				}
				else {
				    IFQ_LOCK(&(ifp->if_snd));
				    ifp->if_tap = NULL;
				    ifp->if_tapctl = NULL;
				    IFQ_UNLOCK(&(ifp->if_snd));
				}
			}
			netintf_promisc=0;
			del_input_type(NET_WILD_TYPE);
			del_domain_af(&intfdomain);
			if (err = unpincode(config_intf)) {
				unlockl(&cfg_lock);
				return(err);
			}
		}
		unlockl(&cfg_lock);
		break;
	default:
		return(EINVAL);
	}
	
	return(0);
}
