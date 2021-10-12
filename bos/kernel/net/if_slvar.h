/* @(#)08	1.6  src/bos/kernel/net/if_slvar.h, sysnet, bos411, 9435D411a 9/2/94 14:45:33 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: SLIP_LOCK_INIT
 *		SLIP_LOCK
 *		SLIP_UNLOCK
 *		
 *
 *   ORIGINS: 27,85
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
 * Definitions for SLIP interface data structures
 * 
 * (this exists so programs like slstats can get at the definition
 *  of sl_softc.)
 *
 *	Base:	if_slvar.h	7.4 (Berkeley) 1/20/90
 *		from if_slvar.h,v 1.3 89/05/31 02:25:18 van Exp
 */
#ifndef _H_IF_SLVAR
#define _H_IF_SLVAR

struct slipstat {
	u_int		sl_ibytes;	/* total number of data bytes in */
	u_int		sl_ipackets;	/* total number of data packets in */
	u_int		sl_ierrors;	/* total number of input errors */
	u_int		sl_obytes;	/* total number of data bytes out */
	u_int		sl_opackets;	/* total number of data packets out */
	u_int		sl_oerrors;	/* total number of output errors */
};

#ifdef _KERNEL
struct sl_softc {
	struct	sl_softc *slip_next;	/* chain of softc's */
	struct	ifnet sc_if;		/* network-visible interface */
	void	*sc_qptr;
	u_char	*sc_mp;			/* pointer to next available buf char */
	u_char	*sc_ep;			/* pointer to last available buf char */
	u_char	*sc_buf;		/* input buffer */
	caddr_t	sc_cluster;		/* cluster page holding above */
	u_int	sc_flags;		/* see below */
	u_int	sc_escape;		/* last char input was FRAME_ESCAPE */
	struct	slcompress sc_comp;	/* tcp compression data */
	struct	slipstat sc_stat;
	int	(*sc_output)();	
	int	(*sc_detach)();
	int	slip_attached;
	dev_t	devno;
	simple_lock_data_t sc_lock;
};
#define SLIP_LOCK_DECL()	int	_slsc;
#define SLIP_LOCK(sc)		_slsc = disable_lock(PL_IMP, &((sc)->sc_lock))
#define SLIP_UNLOCK(sc)		unlock_enable(_slsc, &(sc)->sc_lock)
#define SLIP_LOCK_INIT(sc)	simple_lock_init(&(sc)->sc_lock)
#endif /* _KERNEL */

#define BUFOFFSET	128
#define SLMAX		(MCLBYTES - BUFOFFSET)
#define SLBUFSIZE	(SLMAX + BUFOFFSET)
#define SLMTU		296
#define SLIP_MODID	518
#define SLIP_IFQMAXLEN	50
#define SLIP_HIWAT	1024
#define SLIP_LOWAT	1024

#define SLIFNAME      "sl"

/* visible flags */
#define	SC_COMPRESS	0x000002	/* compress TCP traffic */
#define	SC_NOICMP	0x000004	/* supress ICMP traffic */
#define	SC_AUTOCOMP	0x000008	/* auto-enable TCP compression */
#define SC_CANSET	(SC_COMPRESS|SC_NOICMP|SC_AUTOCOMP)	

#ifdef _KERNEL
/* internal flags (should be separate) */
#define SC_INUSE	0x040000		
#define SC_ATTACHED	0x080000		
#define SC_MASK		0x0f0000

#define SC_ERROR	-1
#define SC_FOUND	0
#define SC_BUSY		1
#endif /* _KERNEL */

#define FRAME_END		0xc0	/* Frame End */
#define FRAME_ESCAPE		0xdb	/* Frame Esc */
#define TRANS_FRAME_END		0xdc	/* transposed frame end */
#define TRANS_FRAME_ESCAPE	0xdd	/* transposed frame esc */

#define microtime(tv) {						\
	struct timestruc_t		ct;			\
								\
	curtime(&ct);						\
	(tv)->tv_sec = (int) ct.tv_sec;				\
	(tv)->tv_usec = (int) ct.tv_nsec / 1000;		\
}

#endif /* _H_IF_SLVAR */
