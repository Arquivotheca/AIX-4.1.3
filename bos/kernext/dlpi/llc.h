/* @(#)23	1.1  src/bos/kernext/dlpi/llc.h, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:30  */
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: CRPF
 *		GETNR
 *		GETNS
 *		ISCMD
 *		ISCTL1
 *		ISCTL2
 *		ISPF
 *		ISRSP
 *		PRIM
 *		SETCMD
 *		SETNR
 *		SETNS
 *		SETPF1
 *		SETPF2
 *		SETRSP
 *		incr
 *		snap_type
 *		swapsap
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

/*
 * llc.h - llc specifics
 */

/*
 * reserved sap values
 */

#define SAP_NULL	0x00
#define	SAP_DRD		0x14
#define	SAP_SNAP	0xaa
#define	SAP_GLOBAL	0xff

/*
 * llc control field values
 */

/* command/response bit */
#define	CMD	0x00
#define	RSP	0x01

/* poll/final bit (type1/type2 frames) */
#define	PF1	0x10
#define	PF2	0x01

/* type 1 unnumbered commands/responses */
#define	UI	0x03
#define XID	0xaf
#define	TEST	0xe3

/* type 2 unnumbered commands/responses */
#define	SABME	0x6f
#define	DISC	0x43
#define	UA	0x63
#define	DM	0x0f
#define	FRMR	0x87

/* type 2 supervisory commands/responses */
#define	RR	0x01
#define	RNR	0x05
#define	REJ	0x09

/* LLC variable extraction */
#define	ISRSP(l)	(((l)->ssap & RSP) == RSP)
#define	ISCMD(l)	(!ISRSP(l))
#define	ISCTL1(l)	(((l)->ctl1 & 0x03) == 0x03)
#define	ISCTL2(l)	(!ISCTL1(l))
#define	ISPF(l)		(!!(ISCTL1(l) ? ((l)->ctl1 & PF1) : ((l)->ctl2 & PF2)))
#define	PRIM(l)		(ISCTL1(l) ? ((l)->ctl1 & ~PF1) : (l)->ctl1)
#define	GETNS(l)	((l)->ctl1 >> 1)
#define	GETNR(l)	((l)->ctl2 >> 1)

/* LLC variable insertion */
#define	SETCMD(l)	((l)->ssap &= ~RSP)
#define	SETRSP(l)	((l)->ssap |= RSP)
#define	SETPF1(l)	((l)->ctl1 |= PF1)
#define	SETPF2(l)	((l)->ctl2 |= PF2)
#define	SETNS(l,ns)	((l)->ctl1 = ((ns) << 1))	/* destructive */
#define	SETNR(l,nr)	((l)->ctl2 = ((nr) << 1))	/* destructive */

/* S-frame requests (bit flags) */
#define	S_CMD	0x0000		/* nop; for symmetry */
#define	S_RSP	0x0001		/* set rsp on S-frame */
#define	S_POLL	0x0002		/* set poll on S-frame */
#define	S_FINAL	0x0002		/* same as poll; again, for symmetry */

/* debugging support */
#define	CRPF(l)		(ISCMD(l) ? (ISPF(l)?"CP":"C") : (ISPF(l)?"RF":"R"))

/* change direction of llc */
#define	swapsap(l) do {			\
	uchar t;			\
	t = (l)->ssap & 0xfe;		\
	(l)->ssap = (l)->dsap & 0xfe;	\
	(l)->dsap = t;			\
} while (0)

/* increment counter modulo 128 */
#define	incr(x) ((x) = (((x) + 1) & 0x7f))

/* XID parameters */
#define	XID_FI	0x81		/* XID format identifier: IEEE Basic */
#define	XID_T1	0x01		/* XID type: type 1 */
#define	XID_T2	0x02		/* XID type: type 2 */
#define	XID_T12	0x03		/* XID type: type 1 + type 2 */
#define	XID_WS	0xfe		/* XID window size: max of 127 */

/*
 * llc_t - llc format (u-frames do not have ctl2)
 */

typedef struct llc {
	uchar	dsap;
	uchar	ssap;
	uchar	ctl1;
	uchar	ctl2;
} llc_t;

/*
 * snap_t - anatomy of a SNAP
 */

typedef struct {
	uchar	org_id[3];
	uchar	proto_id[2];
} snap_t;
#define	snap_type(p)	(*(ushort *)(((snap_t *)p)->proto_id))

/*
 * llcsnap_t - llc u-frame format with snap
 *
 * replicated fields because don't have anonymous structs
 */

typedef struct llcsnap {
	uchar	dsap;
	uchar	ssap;
	uchar	ctl1;
	uchar	org_id[3];
	uchar	proto_id[2];
} llcsnap_t;

/*
 * important lengths
 */

#define	LLC_ULEN	3
#define	LLC_SLEN	4
#define	LLC_ILEN	4

#define	SNAP_LEN	sizeof(snap_t)
#define	LLC_SNAP_LEN	sizeof(llcsnap_t)
