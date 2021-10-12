/* @(#)83	1.2  src/bos/kernel/sys/ndd_var.h, sysnet, bos411, 9428A410j 10/30/93 13:24:10 */
/*
 *   COMPONENT_NAME: SOCKINC
 *
 *   FUNCTIONS: sotorawnddpcb
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_NDD_VAR
#define	_H_NDD_VAR
#include <sys/ndd.h>
#include <sys/cdli.h>

#define	SNDD_MAXFILTER	64

/* 
 * generic ndd sockaddr
 */
struct sockaddr_ndd {
	u_char	sndd_len;
	u_char	sndd_family;
	u_char	sndd_nddname[NDD_MAXNAMELEN];
	u_int	sndd_filterlen;
	u_char	sndd_data[SNDD_MAXFILTER];
};

struct sockaddr_ndd_8022 {
	u_char			sndd_8022_len;
	u_char			sndd_8022_family;
	u_char			sndd_8022_nddname[NDD_MAXNAMELEN];
	u_int			sndd_8022_filterlen;
	struct ns_8022 		sndd_8022_ns;
};

#define	sndd_8022_filtertype	sndd_8022_ns.filtertype
#define	sndd_8022_dsap		sndd_8022_ns.dsap
#define	sndd_8022_orgcode	sndd_8022_ns.orgcode
#define	sndd_8022_ethertype	sndd_8022_ns.ethertype

struct nddctl {
	u_int		nddctl_buflen;
	caddr_t		nddctl_buf;
};

#define	NDD_CTL_MAXBUFLEN	4096


#ifdef sotorawcb
/*
 * Common structure pcb for raw ndd protocol access.
 * Here are ndd specific extensions to the raw control block,
 * and space is allocated to the necessary sockaddrs.
 */
struct raw_nddpcb {
	struct rawcb 		rndd_rcb; /* common control block prefix */
	struct ndd 		*rndd_sendnddp;
	struct ndd 		*rndd_recvnddp;
	struct sockaddr_ndd 	rndd_faddr;
	struct sockaddr_ndd 	rndd_laddr;
};
#endif

#define	sotorawnddpcb(so)	((struct raw_nddpcb *)(so)->so_pcb)

#endif	/* _H_NDD_VAR */
