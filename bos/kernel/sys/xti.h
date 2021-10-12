/* @(#)46     1.4.2.1  src/bos/kernel/sys/xti.h, sysxpse, bos411, 9428A410j 11/9/93 18:55:20 */
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 18,27,63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 *   All Rights Reserved
 *
 *   Copyright (c) 1989  Mentat Inc.
 *
 */

#ifndef _XTI_H
#define _XTI_H

/* Error values */
#define	TBADADDR	1
#define	TBADOPT		2
#define	TACCES		3
#define	TBADF		4
#define	TNOADDR		5
#define	TOUTSTATE	6
#define	TBADSEQ		7
#define TSYSERR		8
#define	TLOOK		9
#define	TBADDATA	10
#define	TBUFOVFLW	11
#define	TFLOW		12
#define	TNODATA		13
#define	TNODIS		14
#define	TNOUDERR	15
#define	TBADFLAG	16
#define	TNOREL		17
#define	TNOTSUPPORT	18
#define	TSTATECHNG	19
#define	TNOSTRUCTYPE	20
#define TBADNAME	21
#define	TBADQLEN	22
#define	TADDRBUSY	23
#define TINDOUT		24
#define TPROVMISMATCH	25
#define TRESQLEN	26
#define TRESADDR	27
#define TQFULL		28
#define TPROTO		29

/* t_look events */
#define	T_LISTEN	0x0001
#define	T_CONNECT	0x0002
#define	T_DATA		0x0004
#define	T_EXDATA	0x0008
#define	T_DISCONNECT	0x0010
#define	T_UDERR		0x0040
#define	T_ORDREL	0x0080
#define	T_GODATA	0x0100
#define	T_GOEXDATA	0x0200
#define	T_EVENTS	0x0400
/*#define	T_ERROR	*/

/* Flag definitions */
#define	T_MORE		0x01
#define	T_EXPEDITED	0x02
#define	T_NEGOTIATE	0x04
#define	T_CHECK		0x08
#define	T_DEFAULT	0x10
#define T_SUCCESS	0x20
#define	T_FAILURE	0x40
#define T_CURRENT	0x0080
#define T_PARTSUCCESS	0x0100
#define T_READONLY	0x0200
#define T_NOTSUPPORT	0x0400


struct t_info {
	long	addr;
	long	options;
	long	tsdu;
	long	etsdu;
	long	connect;
	long	discon;
	long	servtype;
	long	flags;
};

/* Flags defines (other info about the transport provider) */
#define T_SENDZERO	0x001

/* Service types */
#define T_COTS		1	/* Connection-mode service */
#define	T_COTS_ORD	2	/* Connection service with orderly release */
#define	T_CLTS		3	/* Connectionless-mode service */

struct netbuf {
	unsigned int	maxlen;
	unsigned int	len;
	char *		buf;
};

struct t_opthdr { 
	unsigned long len; 	/* total option length- sizeof(struct t_opthdr)
				   + length of option value in bytes */
	unsigned long level;	/* protocol affected */ 
	unsigned long name;	/* option name */
	unsigned long status;	/* status value */
	/* followed by the option value */
};

struct t_bind {
	struct netbuf	addr;
	unsigned	qlen;
};

struct t_optmgmt {
	struct netbuf	opt;
	long		flags;
};

struct t_discon {
	struct netbuf	udata;
	int		reason;
	int		sequence;
};

struct t_call {
	struct netbuf	addr;
	struct netbuf	opt;
	struct netbuf	udata;
	int		sequence;
};

struct t_unitdata {
	struct netbuf	addr;
	struct netbuf	opt;
	struct netbuf	udata;
};

struct t_uderr {
	struct netbuf	addr;
	struct netbuf	opt;
	long		error;
};

/* t_alloc structure types */
#define	T_BIND		1
#define T_OPTMGMT	2
#define	T_CALL		3
#define	T_DIS		4
#define	T_UNITDATA	5
#define	T_UDERROR	6
#define	T_INFO		7

/* t_alloc field identifiers */
#define	T_ADDR		0x01
#define	T_OPT		0x02
#define	T_UDATA		0x04
#define	T_ALL		0xffff

/* State values */
#define	T_UNBND		1	/* unbound */
#define	T_IDLE		2	/* idle */
#define	T_OUTCON	3	/* outgoing connection pending */
#define	T_INCON		4	/* incoming connection pending */
#define	T_DATAXFER	5	/* data transfer */
#define	T_OUTREL	6	/* outgoing orderly release */
#define	T_INREL		7	/* incoming orderly release */

/* general purpose defines */
#define	T_YES		1
#define	T_NO		0
#define	T_UNUSED	(-1)
#define	T_NULL		0
#define	T_ABSREQ	0x8000
#define T_INFINITE	(-1)
#define T_INVALID	(-2)

#define T_UNSPEC	(~0 - 2)
#define T_ALLOPT	0
#define T_ALIGN(p)	(((unsigned long) (p) + (sizeof(long) - 1)) &~ (sizeof(long) - 1))

#define OPT_NEXTHDR(pbuf,buflen,popt)\
		   (((char *)(popt) + T_ALIGN((popt)->len) <\
		   (char *)(pbuf) + buflen) ?\
		   (struct t_opthdr *)((char*)(popt) + T_ALIGN((popt)->len)) :\
		   (struct t_opthdr *)0)

#define XTI_GENERIC	0xffff
#define XTI_DEBUG	0x0001
#define XTI_LINGER	0x0080
#define XTI_RCVBUF	0x1002
#define XTI_RCVLOWAT	0x1004
#define XTI_SNDBUF	0x1001
#define XTI_SNDLOWAT	0x1003

struct t_linger { 
	long  l_onoff; 
	long   l_linger; 
};

/* ISO definitions */
#define	T_CLASS0	0
#define	T_CLASS1	1
#define	T_CLASS2	2
#define	T_CLASS3	3
#define	T_CLASS4	4

/* priorities */
#define	T_PRITOP	0
#define	T_PRIHIGH	1
#define	T_PRIMID	2
#define	T_PRILOW	3
#define	T_PRIDFLT	4

/* protection levels */
#define	T_NOPROTECT		1
#define	T_PASSIVEPROTECT	2
#define	T_ACTIVEPROTECT		4

/* default value for the length of TPDU's */
#define	T_LTPDUDFLT	128

/* rate structure */
struct rate {
	long	targetvalue;
	long	minacceptvalue;
};

/* reqvalue structure */
struct reqvalue {
	struct rate	called;
	struct rate	calling;
};

/* throughput structure */
struct thrpt {
	struct reqvalue	maxthrpt;
	struct reqvalue	avgthrpt;
};

struct transdel { 
	struct reqvalue maxdel; 
	struct reqvalue avgdel; 
};

#define ISO_TP  0x0100
#define TCO_THROUGHPUT		0x0001
#define TCO_TRANSDEL		0x0002
#define TCO_RESERRORRATE	0x0003
#define TCO_TRANSFFAILPROB	0x0004
#define TCO_ESTFAILPROB		0x0005
#define TCO_RELFAILPROB		0x0006
#define TCO_ESTDELAY		0x0007
#define TCO_RELDELAY		0x0008
#define TCO_CONNRESIL		0x0009
#define TCO_PROTECTION		0x000a
#define TCO_PRIORITY		0x000b
#define TCO_EXPD		0x000c

#define TCL_TRANSDEL		0x000d
#define TCL_RESERRORRATE	TCO_RESERRORRATE
#define TCL_PROTECTION		TCO_PROTECTION
#define TCL_PRIORITY		TCO_PRIORITY

#define TCO_LTPDU		0x0100
#define TCO_ACKTIME		0x0200
#define TCO_REASTIME		0x0300
#define TCO_EXTFORM		0x0400
#define TCO_FLOWCTRL		0x0500
#define TCO_CHECKSUM		0x0600
#define TCO_NETEXP		0x0700
#define TCO_NETRECPTCF		0x0800
#define TCO_PREFCLASS		0x0900
#define TCO_ALTCLASS1		0x0a00
#define TCO_ALTCLASS2		0x0b00
#define TCO_ALTCLASS3		0x0c00
#define TCO_ALTCLASS4		0x0d00

#define TCL_CHECKSUM		TCO_CHECKSUM

#define INET_TCP	0x6

#define TCP_NODELAY	0x1
#define TCP_MAXSEG	0x2
#define TCP_KEEPALIVE	0x8

struct t_kpalive { 
	long kp_onoff; 
	long  kp_timeout; 
};
#define T_GARBAGE	0x02

#define INET_UDP	0x11

#define UDP_CHECKSUM	TCO_CHECKSUM

#define INET_IP		0x0

#define IP_OPTIONS	1
#define IP_TOS		3
#define IP_TTL		4
#define IP_REUSEADDR	0x8
#define IP_DONTROUTE	0x10
#define IP_BROADCAST	0x20

/* IP_TOS Precedence Levels */
#define	T_ROUTINE	0
#define	T_PRIORITY	1
#define	T_IMMEDIATE	2
#define	T_FLASH		3
#define	T_OVERRIDEFLASH	4
#define	T_CRITIC_ECP	5
#define	T_INETCONTROL	6
#define	T_NETCONTROL	7

/* IP_TOS type of service */
#define T_NOTOS		0
#define T_LDELAY	(1<<4)
#define T_HITHRPT	(1<<3)
#define T_HIREL		(1<<2)

#define SET_TOS(prec,tos)	((0x7 & (prec)) << 5 | (0x1c & (tos)))

#ifndef	_KERNEL
extern	int	t_errno;
extern	char *	t_errlist[];
extern	int	t_nerr;

#ifdef	_NO_PROTO
extern	int	t_accept();
extern	char *	t_alloc();
extern	int	t_bind();
extern	int	t_blocking();
extern	int	t_close();
extern	int	t_connect();
extern	int	t_error();
extern	int	t_free();
extern	int	t_getinfo();
extern	int	t_getstate();
extern	int	t_listen();
extern	int	t_look();
extern	int	t_nonblocking();
extern	int	t_open();
extern	int	t_optmgmt();
extern	int	t_rcv();
extern	int	t_rcvconnect();
extern	int	t_rcvdis();
extern	int	t_rcvrel();
extern	int	t_rcvudata();
extern	int	t_rcvuderr();
extern	int	t_snd();
extern	int	t_snddis();
extern	int	t_sndrel();
extern	int	t_sndudata();
extern	int	t_sync();
extern	int	t_unbind();
#else
extern	int	t_accept(int fd, int resfd, struct t_call *call);
extern	char *	t_alloc(int fd, int struct_type, int fields);
extern	int	t_bind(int fd, struct t_bind *req, struct t_bind *ret);
extern	int	t_blocking(int fd);
extern	int	t_close(int fd);
extern	int	t_connect(int fd,struct t_call *sndcall,struct t_call *rcvcall);
extern	int	t_error(char *errmsg);
extern	int	t_free(char *ptr, int struct_type);
extern	int	t_getinfo(int fd, struct t_info *info);
extern	int	t_getstate(int fd);
extern	int	t_listen(int fd, struct t_call *call);
extern	int	t_look(int fd);
extern	int	t_nonblocking(int fd);
extern	int	t_open(char *path, int oflag, struct t_info *info);
extern	int	t_optmgmt(int fd, struct t_optmgmt *req, struct t_optmgmt *ret);
extern	int	t_rcv(int fd, char *buf, unsigned nbytes, int *flags);
extern	int	t_rcvconnect(int fd, struct t_call *call);
extern	int	t_rcvdis(int fd, struct t_discon *discon);
extern	int	t_rcvrel(int fd);
extern	int	t_rcvudata(int fd, struct t_unitdata *unitdata, int *flags);
extern	int	t_rcvuderr(int fd, struct t_uderr *uderr);
extern	int	t_snd(int fd, char *buf, unsigned nbytes, int flags);
extern	int	t_snddis(int fd, struct t_call *call);
extern	int	t_sndrel(int fd);
extern	int	t_sndudata(int fd, struct t_unitdata *unitdata);
extern	int	t_sync(int fd);
extern	int	t_unbind(int fd);
#endif	/* _NO_PROTO */
#endif	/* KERNEL */
#endif	/* _XTI_H */
