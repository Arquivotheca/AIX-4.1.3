static char sccsid[] = "@(#)36	1.2  src/bos/kernext/xns/spp_debug.c, sysxxns, bos411, 9428A410j 7/24/93 14:01:37";
/*
 *   COMPONENT_NAME: SYSXXNS
 *
 *   FUNCTIONS: spp_trace
 *		
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
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
/*
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *      Base:   spp_debug.c     7.7 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/errno.h>

#include <net/route.h>
#include <net/if.h>
#include <netinet/tcp_fsm.h>

#include <netns/ns.h>
#include <netns/ns_pcb.h>
#include <netns/idp.h>
#include <netns/idp_var.h>
#include <netns/sp.h>
#include <netns/spidp.h>
#define SPPTIMERS
#include <netns/spp_timer.h>
#include <netns/spp_var.h>
#define	SANAMES
#include <netns/spp_debug.h>

int	sppconsdebug = 0;
struct  spp_debug spp_debug[SPP_NDEBUG];
int     spp_debx;
/*
 * spp debug routines
 */
spp_trace(
	int act,
	u_char ostate,
	struct sppcb *sp,
	struct spidp *si,
	int req)
{
#ifndef	INET
#include <inet.h>
#endif
#if	INET > 0
#if	INETPRINTFS
	u_short seq, ack, len, alo;
	unsigned long iptime();
	int flags;
	struct spp_debug *sd = &spp_debug[spp_debx++];
	extern char *prurequests[];
	extern char *sanames[];
	extern char *tcpstates[];
	extern char *spptimers[];

	if (spp_debx == SPP_NDEBUG)
		spp_debx = 0;
	sd->sd_time = iptime();
	sd->sd_act = act;
	sd->sd_ostate = ostate;
	sd->sd_cb = (caddr_t)sp;
	if (sp)
		sd->sd_sp = *sp;
	else
		bzero((caddr_t)&sd->sd_sp, sizeof (*sp));
	if (si)
		sd->sd_si = *si;
	else
		bzero((caddr_t)&sd->sd_si, sizeof (*si));
	sd->sd_req = req;
	if (sppconsdebug == 0)
		return;
	if (ostate >= TCP_NSTATES) ostate = 0;
	if (act >= SA_DROP) act = SA_DROP;
	if (sp)
		printf("%x %s:", sp, tcpstates[ostate]);
	else
		printf("???????? ");
	printf("%s ", sanames[act]);
	switch (act) {

	case SA_RESPOND:
	case SA_INPUT:
	case SA_OUTPUT:
	case SA_DROP:
		if (si == 0)
			break;
		seq = si->si_seq;
		ack = si->si_ack;
		alo = si->si_alo;
		len = si->si_len;
		if (act == SA_OUTPUT) {
			seq = ntohs(seq);
			ack = ntohs(ack);
			alo = ntohs(alo);
			len = ntohs(len);
		}
#ifndef lint
#ifdef	__STDC__
#define p1(f)  { printf("%s = %x, ", #f, f); }
#else
#define p1(f)  { printf("%s = %x, ", "f", f); }
#endif
		p1(seq); p1(ack); p1(alo); p1(len);
#endif
		flags = si->si_cc;
		if (flags) {
			char *cp = "<";
#ifndef lint
#ifdef	__STDC__
#define pf(f) { if (flags&SP_##f) { printf("%s%s", cp, #f); cp = ","; } }
#else
#define pf(f) { if (flags&SP_/**/f) { printf("%s%s", cp, "f"); cp = ","; } }
#endif
			pf(SP); pf(SA); pf(OB); pf(EM);
#else
			cp = cp;
#endif
			printf(">");
		}
#ifndef lint
#ifdef	__STDC__
#define p2(f)  { printf("%s = %x, ", #f, si->si_##f); }
#else
#define p2(f)  { printf("%s = %x, ", "f", si->si_/**/f); }
#endif
		p2(sid);p2(did);p2(dt);p2(pt);
#endif
		ns_printhost(&si->si_sna);
		ns_printhost(&si->si_dna);

		if (act==SA_RESPOND) {
			printf("idp_len = %x, ",
				((struct idp *)si)->idp_len);
		}
		break;

	case SA_USER:
		printf("%s", prurequests[req&0xff]);
		if ((req & 0xff) == PRU_SLOWTIMO)
			printf("<%s>", spptimers[req>>8]);
		break;
	}
	if (sp)
		printf(" -> %s", tcpstates[sp->s_state]);
	/* print out internal state of sp !?! */
	printf("\n");
	if (sp == 0)
		return;
#ifndef lint
#ifdef	__STDC__
#define p3(f)  { printf("%s = %x, ", #f, sp->s_##f); }
#else
#define p3(f)  { printf("%s = %x, ", "f", sp->s_/**/f); }
#endif
	printf("\t"); p3(rack);p3(ralo);p3(smax);p3(flags); printf("\n");
#endif
#endif
#endif
}
