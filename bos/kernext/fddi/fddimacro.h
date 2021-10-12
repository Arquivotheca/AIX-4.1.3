/* @(#)78       1.8  src/bos/kernext/fddi/fddimacro.h, sysxfddi, bos411, 9436E411a 9/9/94 20:05:51 */
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: ADDR_HI
 *		ADDR_LO
 *		CHK_FRAME_SIZ
 *		FDDI_ASSERT
 *		FDDI_ERTRACE
 *		FDDI_ETRACE
 *		FDDI_ETTRACE
 *		FDDI_FC
 *		FDDI_GET_LINK_STATS
 *		FDDI_IS_BCAST
 *		FDDI_REARM
 *		FDDI_RTRACE
 *		FDDI_TRACE
 *		FDDI_TTRACE
 *		INCREMENT
 *		MBUF_CNT
 *		PIO_GETCX
 *		PIO_GETLRX
 *		PIO_GETPOS
 *		PIO_GETPOS2
 *		PIO_GETSRX
 *		PIO_GETSTRX
 *		PIO_GETSX
 *		PIO_PUTCX
 *		PIO_PUTLRX
 *		PIO_PUTPOS
 *		PIO_PUTPOS2
 *		PIO_PUTSRX
 *		PIO_PUTSTRX
 *		QUE_FULL
 *		SWAPLONG
 *		SWAPSHORT
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stddef.h>

#ifndef _H_FDDIMACRO
#define _H_FDDIMACRO

/*
 * INCREMENT operates on specified indexes by specified value
 * 	Indexes are used to manage the circular adapter queues: 
 * 	tx queue, rcv queue.
 */
#define INCREMENT(i,n,x)					\
	{							\
		(i) = (((i) + (n)) % (x));			\
	}							\

#define QUE_FULL(q,n,x)	(((q) + (n)) > (x))


/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1   */
/*  is returned (hence all return an int value).  In the cases of         */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

enum pio_func
{
	GETC, GETS, GETSR, GETL, GETLR, PUTSTR, GETSTR,
	PUTC, PUTS, PUTSR, PUTL, PUTLR
};

/*------------------------------------------------------------------------*/
/* PIO_GETPOS and PIO_PUTPOS were added to implement the pio operations   */
/* for the pos registers.  The pos registers differ from other pio  	  */
/* operations as there isn't any checking done.  To check the values on a */
/* get operation, the driver will get the value twice and compare what    */
/* was received.  On a put the driver will write it and then get the      */
/* register's contents to compare with.  The PIO_GETPOS2 and PIO_PUTPOS2  */
/* macros were added for pos register 2 which acts differently on the FDDI*/
/* adapters.  The AR (adapter reset) bit is always set on the read.  So   */
/* the checks for the put operation must assume it is set and check so that */
/* when written any other bits that were set are ignored.		  */
/*------------------------------------------------------------------------*/
#define PIO_GETPOS(addr, c)						\
{									\
	uchar pos_c;							\
	uint pos_i;							\
									\
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++)			\
	{								\
		BUS_GETCX(addr, c);					\
		BUS_GETCX(addr, &pos_c);				\
		if (*(c) == pos_c) break;				\
	}								\
	if (pos_i >= PIO_RETRY_COUNT)					\
		p_acs->dev.pio_rc = TRUE;				\
}
	
#define PIO_PUTPOS(addr, c)						\
{									\
	uchar pos_c;							\
	uint pos_i;							\
									\
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++)			\
	{								\
		BUS_PUTCX(addr, c);					\
		BUS_GETCX(addr, &pos_c);				\
		if (c == pos_c) break;					\
	}								\
	if (pos_i >= PIO_RETRY_COUNT)					\
		p_acs->dev.pio_rc = TRUE;				\
}
	
#define PIO_GETPOS2(addr, c)						\
{									\
	uchar pos_c;							\
	uint pos_i;							\
									\
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++)			\
	{								\
		BUS_GETCX(addr, (c));					\
		BUS_GETCX(addr, &pos_c);				\
		if (*(c) == pos_c) break;				\
	}								\
	if (pos_i >= PIO_RETRY_COUNT)					\
	{								\
		p_acs->dev.pio_rc = TRUE;				\
		FDDI_TRACE("PIO ",*(c),pos_c,0);			\
	}								\
}
	
#define PIO_PUTPOS2(addr, c)						\
{									\
	uchar pos_c;							\
	uint pos_i;							\
									\
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++)			\
	{								\
		BUS_PUTCX(addr, (c));					\
		BUS_GETCX(addr, &pos_c);				\
		if (((c) & FDDI_POS2_AR) || 				\
			(((c)|FDDI_POS2_AR) == pos_c)) break;		\
	}								\
	if (pos_i >= PIO_RETRY_COUNT)					\
	{								\
		p_acs->dev.pio_rc = TRUE;				\
		FDDI_TRACE("PIO ",c,pos_c,0);				\
	}								\
}
	
	
	

#define PIO_GETCX(addr, c)						\
{									\
	if (p_acs->dev.iox = BUS_GETCX(addr, c))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, GETC, addr, (ulong)(c), NULL);\
	}								\
}
#define PIO_GETSX(addr, s)						\
{									\
	if (p_acs->dev.iox = BUS_GETSX(addr, s))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, GETS, addr, (ulong)(s), NULL);\
	}								\
}
#define PIO_GETSRX(addr, sr)						\
{									\
	if (p_acs->dev.iox = BUS_GETSRX(addr, sr))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, GETSR, addr, (ulong)(sr), NULL);\
	}								\
}
#define PIO_GETLRX(addr, lr)						\
{									\
	if (p_acs->dev.iox = BUS_GETLRX(addr, lr))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, GETLR, addr, (ulong)(lr), NULL);\
	}								\
}
#define PIO_PUTCX(addr, c)						\
{									\
	if (p_acs->dev.iox = BUS_PUTCX(addr, c))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, PUTC, addr, (ulong)(c), NULL);\
	}								\
}
#define PIO_PUTSRX(addr, sr)						\
{									\
	if (p_acs->dev.iox = BUS_PUTSRX(addr, sr))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, PUTSR, addr, (ulong)(sr), NULL);\
	}								\
}
#define PIO_PUTLRX(addr, lr)						\
{									\
	if (p_acs->dev.iox = BUS_PUTLRX(addr, lr))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, PUTLR, addr, (ulong)(lr), NULL);\
	}								\
}
#define PIO_PUTSTRX(addr, src, c)					\
{									\
	if (p_acs->dev.iox = BUS_PUTSTRX(addr, src, c))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, PUTSTR, addr, src, c);\
	}								\
}
#define PIO_GETSTRX(addr, src, c)					\
{									\
	if (p_acs->dev.iox = BUS_GETSTRX(addr, src, c))			\
	{								\
		fddi_logerr(p_acs,ERRID_CFDDI_PIO, __LINE__, __FILE__, 0,0,0); \
		p_acs->dev.pio_rc = 					\
			fddi_pio_retry(p_acs, GETSTR, addr, src, c);\
	}								\
}


/*
 * For use with the copy string PIO functions
 */
#define SWAPSHORT(x)   ((((x) & 0xFF) << 8) | ((x) >> 8))
#define SWAPLONG(x)    ((((x) & 0xFF)<<24) | (((x) & 0xFF00)<<8) | \
			 (((x) & 0xFF0000)>>8) | (((x) & 0xFF000000)>>24))

#define ADDR_LO(addr)   ((int)(addr) & 0xffff)          /* low 16 bits */
#define ADDR_HI(addr)   (((int)(addr) >> 16) & 0xffff)  /* high 16 bits */

/*
 * FDDI_FC : returns the fc field of an mbuf
 */
#define FDDI_FC(p_mbuf)						\
	(p_mbuf)->m_data[3]

/*
 * FDDI_IS_BCAST(addr) : returns TRUE if the address is a Broadcast address 
 * 			and FALSE otherwise
 */
#define FDDI_IS_BCAST(addr)					\
	((*(long *)(&(addr)[0]) == 0xffffffff) && 		\
		(*(long *)(&(addr)[2]) == 0xffffffff))		\
			? (TRUE) : (FALSE)

/*
 * CHK_FRAME_SIZ macro:
 *
 * 	If any bits in the LLC mask are set then it is not an SMT 
 *	Frame. Accordingly check the length of the frame.
 *	Also, the len is dependent on the address size.
 *
 */
#define CHK_FRAME_SIZ(len, fc)					    \
	(fc & FDDI_FC_MSK) ?					    \
			(len < FDDI_MIN_PACKET || len > FDDI_MAX_LLC_PACKET) : \
			(len < FDDI_MIN_PACKET || len > FDDI_MAX_SMT_PACKET)

/*
 * Get links statistics (in place). 
 *	The setcount does not need swapping
 *	because we just use it to give back to the adapter and we would
 *	just have to swap it back...
 */
#define FDDI_GET_LINK_STATS(ls)					\
{								\
	fddi_adap_links_t	adap;				\
	PIO_GETSTRX (bus + FDDI_LS_SHARED_RAM, &adap, sizeof (adap));	\
	ls.smt_error_lo = SWAPSHORT(adap.smt_error_lo);		\
	ls.smt_error_hi = SWAPSHORT(adap.smt_error_hi);		\
	ls.smt_event_lo = SWAPSHORT(adap.smt_event_lo);		\
	ls.smt_event_hi = SWAPSHORT(adap.smt_event_hi);		\
	ls.cpv 	   	= SWAPSHORT(adap.cpv);			\
	ls.port_event   = SWAPSHORT(adap.port_event);		\
	ls.setcount_lo  = adap.setcount_lo;			\
	ls.setcount_hi  = adap.setcount_hi;			\
	ls.aci_code 	= SWAPSHORT(adap.aci_code);		\
	ls.pframe_cnt 	= SWAPSHORT(adap.pframe_cnt);		\
	ls.ecm_sm 	= SWAPSHORT(adap.ecm_sm);			\
	ls.pcm_a_sm 	= SWAPSHORT(adap.pcm_a_sm);		\
	ls.pcm_b_sm 	= SWAPSHORT(adap.pcm_b_sm);		\
	ls.cfm_a_sm 	= SWAPSHORT(adap.cfm_a_sm);		\
	ls.cfm_b_sm 	= SWAPSHORT(adap.cfm_b_sm);		\
	ls.cf_sm 	= SWAPSHORT(adap.cf_sm);			\
	ls.mac_cfm_sm 	= SWAPSHORT(adap.mac_cfm_sm);		\
	ls.rmt_sm 	= SWAPSHORT(adap.rmt_sm);			\
}


/*
 * This is used to reset a receive descriptor for the adapter to write into
 */
#define FDDI_REARM()							\
{									\
	cache_inval(p_rx->p_buf, len);					\
	PIO_PUTSRX(bus+p_rx->offset+offsetof(fddi_adap_t,cnt), 		\
			p_acs->rx.l_adj_buf);				\
	PIO_PUTSRX(bus+p_rx->offset+offsetof(fddi_adap_t,stat), 0);	\
	PIO_PUTSRX(bus+p_rx->offset+offsetof(fddi_adap_t,ctl), 		\
			(FDDI_RX_CTL_BDV| FDDI_RX_CTL_IFR));		\
	INCREMENT (p_acs->rx.nxt_rx, 1, FDDI_MAX_RX_DESC);		\
}

/*
 * The MBUF_CNT macro will determine if we were passed a packet of
 *	1, 2 or 3 mbufs. These are the only legal values and no
 *	parameter checking is done 
 */
#define MBUF_CNT(m)	\
	((m->m_next == NULL) ? 1 : (m->m_next->m_next == NULL) ? 2 : 3)
	


/*
 * Convert a micro-second to a rtcl_t time value.
 */
#define MS2TIME(msec, tp)	\
    (tp).tv_sec = (msec) / MSEC_PER_SEC; \
    (tp).tv_nsec = ((msec) % MSEC_PER_SEC) * NS_PER_MSEC; 


/*
 * Asserts defined with debug
 */
#ifdef FDDI_DEBUG
#	define FDDI_ASSERT(c)	if (!(c)) brkpoint();
#else
#	define FDDI_ASSERT(c)
#endif

/* 
 * Macros for debug and error tracing.
 */

#define FDDI_XMIT 	((HKWD_CFDDI_SFF_XMIT << 20) | HKTY_GT | 4)
#define FDDI_RECV 	((HKWD_CFDDI_SFF_RECV << 20) | HKTY_GT | 4)
#define FDDI_OTHER 	((HKWD_CFDDI_SFF_OTHER << 20) | HKTY_GT | 4)

#define FDDI_ETRACE(a1,a2,a3,a4)					\
{									\
	fddi_trace(a1,a2,a3,a4);					\
	TRCHKGT(FDDI_OTHER, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}

#ifdef FDDI_DEBUG
#define FDDI_TTRACE(a1,a2,a3,a4)					\
{									\
	fddi_trace(a1,a2,a3,a4);					\
	TRCHKGT(FDDI_XMIT, *(ulong *)(a1), 		\
		(a2), (a3), (a4), 0);					\
}
#else
#define FDDI_TTRACE(a1,a2,a3,a4)					\
{									\
	TRCHKGT(FDDI_XMIT, *(ulong *)(a1), 		\
		(a2), (a3), (a4), 0);					\
}
#endif

#define FDDI_ETTRACE(a1,a2,a3,a4)					\
{									\
	fddi_trace(a1,a2,a3,a4);					\
	TRCHKGT(FDDI_XMIT, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}

#ifdef FDDI_DEBUG
#define FDDI_RTRACE(a1,a2,a3,a4)					\
{									\
	fddi_trace(a1,a2,a3,a4);					\
	TRCHKGT(FDDI_RECV, *(ulong *)(a1), 		\
		(a2), (a3), (a4), 0);					\
}
#else
#define FDDI_RTRACE(a1,a2,a3,a4)					\
{									\
	TRCHKGT(FDDI_RECV, *(ulong *)(a1), 		\
		(a2), (a3), (a4), 0);					\
}
#endif

#define FDDI_ERTRACE(a1,a2,a3,a4)					\
{									\
	fddi_trace(a1,a2,a3,a4);					\
	TRCHKGT(FDDI_RECV, *(ulong *)(a1), 		\
		(ulong)(a2), (ulong)(a3), (ulong)(a4), 0);		\
}

#ifdef FDDI_DEBUG
#define FDDI_TRACE(a1,a2,a3,a4)					\
{									\
	fddi_trace(a1,a2,a3,a4);					\
	TRCHKGT(FDDI_OTHER, *(ulong *)(a1), 		\
		(a2), (a3), (a4), 0);					\
}
#else
#define FDDI_TRACE(a1,a2,a3,a4)					\
{									\
	TRCHKGT(FDDI_OTHER, *(ulong *)(a1), 		\
		(a2), (a3), (a4), 0);					\
}
#endif

#endif /* endif ! _H_FDDIMACRO */
