/* @(#)74	1.5  src/bos/kernext/inet/ip_security.h, sockinc, bos411, 9428A410j 6/16/90 00:21:10 */
/*
 * COMPONENT_NAME: (NETCMD) Network commands. 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* For IP Security Option, RFC 1038, January 1988 */

/*
 * This structure is used to generate a buffer which
 * is passed with a raw socket via a
 * setsockopt(fd, 0, IP_OPTION, buf, buf_len) call
 * into the kernels struct mbuf *ip_security_option.
 */
struct ip_security {
	unsigned char type;		/* ip option type */
	unsigned char length;		/* ip option length */
	unsigned char c_p_level;	/* classification protection level */
	unsigned char ip_eol;		/* end of option list */
	unsigned char *p_a_flags;	/* protection authority flags */
};

#define MAX_C_P_LEVEL_LENGTH	16	/* some big enough number */
struct c_p_level {
	unsigned short c_p_level;
	char c_p_string[MAX_C_P_LEVEL_LENGTH];
};
#define MAX_C_P_LEVELS		5	/* (rfc 1038) + 1 */

#define MAX_P_A_FLAGS_LENGTH	16	/* some big enough number */
struct p_a_flags {
	unsigned short p_a_flags;
	char p_a_string[MAX_P_A_FLAGS_LENGTH];
};
#define MAX_P_A_FLAGS		5	/* (rfc 1038) + 1 */

#define P_A_GENSER		0x80
#define P_A_SIOP		0x40
#define P_A_DSCCS_SPINTCOM	0x20
#define P_A_DSCCS_CRITICOM	0x10
#define P_A_NONE		0x00

#define MCLR_IP_DATAGRAM(m)		/* This macro sets the data in */ \
	{				/* an Internet datagram to all 1s */ \
		register short i; \
		register caddr_t p; \
		struct ip *ip; \
		ip = mtod(m, struct ip); \
		p = mtod(m, caddr_t) + ip->ip_hl; \
		for (i = 0; i <= (m)->m_len; i++) \
			*p = 0xff; \
	}

struct c_p_level C_P_LEVEL[MAX_C_P_LEVELS] = {
        { IPOPT_SECUR_NONE, "none" },
        { IPOPT_SECUR_UNCLASS, "unclassified" },
        { IPOPT_SECUR_CONFID, "confidential" },
        { IPOPT_SECUR_SECRET, "secret" },
        { IPOPT_SECUR_TOPSECRET, "top_secret" }
};
struct p_a_flags P_A_FLAGS[MAX_P_A_FLAGS] = {
        { P_A_GENSER, "genser" },
        { P_A_SIOP, "siop" },
        { P_A_DSCCS_SPINTCOM, "dsccs-spintcom" },
        { P_A_DSCCS_CRITICOM, "dsccs-criticom" },
        { P_A_NONE, "none" },
};
#define VTYPE   0
#define VLENGT  1
#define VCLASS  2
#define VPROTE  3
