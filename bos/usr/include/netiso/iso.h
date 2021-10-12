/* @(#)50	1.6  src/bos/usr/include/netiso/iso.h, sockinc, bos411, 9428A410j 3/5/94 12:41:08 */
/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/* $Header: iso.h,v 4.9 88/09/11 18:06:38 hagens Exp $ */
/* $Source: /usr/argo/sys/netiso/RCS/iso.h,v $ */
/*	 (#)iso.h	7.4 (Berkeley) 6/22/90 */

#ifndef __ISO__
#define __ISO__

/*
 * Protocols
 */
#define	ISOPROTO_TP_CONS	24	/* connection oriented transport protocol */
					/* 				over CONS */
#define	ISOPROTO_TP0	25		/* connection oriented transport protocol */
#define	ISOPROTO_TP1	26		/* not implemented */
#define	ISOPROTO_TP2	27		/* not implemented */
#define	ISOPROTO_TP3	28		/* not implemented */
#define	ISOPROTO_TP4	29		/* connection oriented transport protocol */
#define	ISOPROTO_TP		ISOPROTO_TP4	 /* tp-4 with negotiation */
#define	ISOPROTO_CLTP	30		/* connectionless transport (not yet impl.) */
#define	ISOPROTO_CLNP	31		/* connectionless internetworking protocol */
#define	ISOPROTO_X25	32		/* cons */
#define	ISOPROTO_INACT_NL	33	/* inactive network layer! */
#define	ISOPROTO_ESIS	34		/* ES-IS protocol */
#define	ISOPROTO_INTRAISIS	35		/* IS-IS protocol */

#define	ISOPROTO_RAW	255		/* raw clnp */
#define	ISOPROTO_MAX	256

#define	ISO_PORT_RESERVED		1024
#define	ISO_PORT_USERRESERVED	5000
/*
 * Port/socket numbers: standard network functions
 * NOT PRESENTLY USED
 */
#define	ISO_PORT_MAINT		501
#define	ISO_PORT_ECHO		507
#define	ISO_PORT_DISCARD	509
#define	ISO_PORT_SYSTAT		511
#define	ISO_PORT_NETSTAT	515
/*
 * Port/socket numbers: non-standard application functions
 */
#define ISO_PORT_LOGIN		513
/*
 * Port/socket numbers: public use
 */
#define ISO_PORT_PUBLIC		1024		/* high bit set --> public */

/*
 *	Network layer protocol identifiers
 */
#define ISO8473_CLNP	0x81
#define	ISO9542_ESIS	0x82
#define ISO9542X25_ESIS	0x8a
#define ISO10589_ISIS		0x83


#ifndef IN_CLASSA_NET
#include <netinet/in.h>
#endif /* IN_CLASSA_NET */



/* The following looks like a sockaddr
 * to facilitate using tree lookup routines */
struct iso_addr {
	u_char	isoa_len;						/* length (in bytes) */
	char	isoa_genaddr[20];				/* general opaque address */
};

struct sockaddr_iso {
	u_char	 			siso_len;			/* length */
	u_char	 			siso_family;		/* family */
	u_char				siso_plen;			/* presentation selector length */
	u_char				siso_slen;			/* session selector length */
	u_char				siso_tlen;			/* transport selector length */
	struct 	iso_addr	siso_addr;			/* network address */
	u_char				siso_pad[6];		/* space for gosip v2 sels */
											/* makes struct 32 bytes long */
};
#define siso_nlen siso_addr.isoa_len
#define siso_data siso_addr.isoa_genaddr

#define TSEL(s) ((caddr_t)((s)->siso_data + (s)->siso_nlen))
#define TSEL_SHORT(s)	( *((short *)(TSEL(s))) )

#define SAME_ISOADDR(a, b) \
	(bcmp((a)->siso_data, (b)->siso_data, (unsigned)(a)->siso_nlen)==0)
/*
 * The following are specific values for siso->siso_data[0],
 * otherwise known as the AFI( Authority and Format Identfier):
 */
#define	AFI_37		0x37	/* bcd of "37" */
#define AFI_OSINET	0x47	/* bcd of "47" */
#define AFI_RFC986	0x47	/* bcd of "47" */
#define	AFI_SNA		0x00	/* SubNetwork Address; invalid really...*/

				/* XXX */
#define ADDR37_IDI_LEN		7
#define ADDR37_DSP_LEN		0

#define IDI_RFC986		6
#define ADDRRFC986_IDI_LEN	2

#define IDI_OSINET 		4
#define ADDROSINET_IDI_LEN	2
#define OVLOSINET_ORGID_LEN	2
#define OVLOSINET_SNETID_LEN	2
#define	MAX_SNPALEN		8	/* curiously equal to sizeof x.121 (
					plus 1 for nibble len) addr */
#define DLSAP_LEN 		1
#define NSEL_LEN 		1
#define MAX_DTE_DIGITS		14

struct iso_addr_f {
		u_char isoa_afi;	/* authority & format id*/
		union	{
			struct addr_37_f {
				u_char x121[ADDR37_IDI_LEN];
			} addr_37;
			struct addr_osinet_f {
				u_char osinet_idi[2];
				u_char orgid[2];
				u_char snetid[2];
				u_char snpa[MAX_SNPALEN + DLSAP_LEN
							+ NSEL_LEN];
			}addr_osinet;
			struct addr_rfc986_f {
				u_char rfc986_idi[2];
				u_char ver;
				u_char ip_addr[4];
				
			}	addr_rfc986;
			u_char addr_local[19];
		} u;
};
#define SOCKADDR_OSI_NSAP_LEN(so) \
			((so)->siso_nlen)
#define SOCKADDR_OSI_AFI(so)	\
			(((struct iso_addr_f *) \
				((so)->siso_data))->isoa_afi)
#define SOCKADDR_TO_X121(so)	\
			(((struct iso_addr_f *) \
				((so)->siso_data))->u.addr_37.x121)

/*  FUNCTIONS imported from if_cons kernel extensions 
 *  when if_cons is loaded
 */

struct cons_fn_t {
	int (*cons_output)();
	int (*cons_openvc)();
	int (*cons_netcmd)();
	int (*cons_send_on_vc)();
	int (*cons_chan_to_pcb)();
};

#define CONS_LOADED 	(cons_fn.cons_output)
							/* XXX */
#ifdef _KERNEL

extern int iso_netmatch();
extern int iso_hash(); 
extern int iso_addrmatch();
extern struct iso_ifaddr *iso_iaonnetof();
extern	struct domain isodomain;
extern	struct protosw isosw[];

#else
/* user utilities definitions from the iso library */

char *iso_ntoa();
struct hostent *iso_gethostbyname(), *iso_gethostbyaddr();

#endif /* _KERNEL */

#define _offsetof(t, m) ((int)((caddr_t)&((t *)0)->m))
#endif /* __ISO__ */
