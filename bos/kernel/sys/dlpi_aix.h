/* @(#)88	1.1  src/bos/kernel/sys/dlpi_aix.h, sysxdlpi, bos41J, 9514A_all 3/31/95 17:51:27  */
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: DLIOC
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SYS_DLPI_AIX_H
#define _SYS_DLPI_AIX_H

/*
 * Implementation constants
 */

#define	PHYSLEN		6	/* IEEE 802.x 48-bit address length */
#define	MAXADDR_LEN	16	/* max length of a DLSAP address */
#define	MAXROUTE_LEN	30	/* max length of TR/FI source routes */
#define	MAXCONIND	10	/* maximum number of pending connections */

/*
 * Implementation-specific ioctls
 */

#define	DLIOC(v)		(('D' << 8) | v)

#define	DL_PKT_FORMAT		DLIOC(1)	/* alter address format */
#define	DL_OUTPUT_RESOLVE	DLIOC(2)	/* tx address resolution */
#define	DL_INPUT_RESOLVE	DLIOC(3)	/* rx address resolution */
#define	DL_ROUTE		DLIOC(4)	/* alter source route */
#define	DL_TUNE_LLC		DLIOC(5)	/* alter LLC tunables */
#define	DL_TUNE_TBL		DLIOC(6)	/* replace LLC tune table */
#define	DL_ZERO_STATS		DLIOC(7)	/* zero statistics counters */
#define	DL_SET_REMADDR		DLIOC(8)	/* set remote address */

/*
 * DLPI-specific packet formats, for use with DL_PKT_FORMAT ioctl
 *
 * The packet formats select the style of address presentation.
 * These are not ioctls, but need unique values, so we use DLIOC.
 */

#define	NS_PROTO_DL_COMPAT	DLIOC(128)	/* AIX 3.2.5 address format */
#define	NS_PROTO_DL_DONTCARE	DLIOC(129)	/* no addresses provided */

/*
 * llctune_t - what the LLC2 tunables are
 */

typedef struct llctune_s {
	int	flags;		/* on ioctl requests, which fields are valid */
	int	t1;		/* ack timeout */
	int	t2;		/* reply timeout (see N3) */
	int	ti;		/* inactivity timeout */
	int	n1;		/* mtu */
	int	n2;		/* retry */
	int	n3;		/* reply threshold (see T2) */
	int	k;		/* window size */
} llctune_t;

/* llctune.flags values */
#define	F_LLC_T1	0x0001
#define	F_LLC_T2	0x0002
#define	F_LLC_TI	0x0004
#define	F_LLC_N1	0x0008
#define	F_LLC_N2	0x0010
#define	F_LLC_N3	0x0020
#define	F_LLC_K		0x0040
#define	F_LLC_ALL	0x007f
#define	F_LLC_SET	0x8000	/* set these values as the new defaults */

#endif /* _SYS_DLPI_AIX_H */
