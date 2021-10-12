/* @(#)85       1.1  src/bos/kernext/fddidiag/fddimacro.h, diagddfddi, bos411, 9428A410j 11/1/93 11:00:28 */
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: ADDR_HI
 *		ADDR_LO
 *		CHK_FRAME_SIZ
 *		DESC_TQ
 *		FDDI_DBTRACE
 *		FDDI_GET_LINK_STATS
 *		FDDI_PTRACE
 *		FDDI_TRACE
 *		FDDI_TTRACE
 *		FDDI_X_BOUND
 *		GET_NETID_OPEN_PTR
 *		INCREMENT
 *		INCRE_TQ
 *		INCR_STATQ_INDEX
 *		MOVEIN
 *		MOVEOUT
 *		PIO_GETCX
 *		PIO_GETLRX
 *		PIO_GETSRX
 *		PIO_GETSTRX
 *		PIO_GETSX
 *		PIO_PUTCX
 *		PIO_PUTLRX
 *		PIO_PUTSRX
 *		PIO_PUTSTRX
 *		QUE_FULL
 *		SWAPLONG
 *		SWAPSHORT
 *		TQ_FULL
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>

#ifndef _H_FDDIMACRO
#define _H_FDDIMACRO


/*
 * This macro will index into the netid table
 * for the open ptr that has the passed in netid value.
 * If the netid boundary checks fail, NULL will be returned.
 *
 * Boundary checks of the passed in netid that are done are:
 *	netid < 0
 *	netid > FDDI_MAX_NETID_VAL
 *	netid is odd
 *
 * The netid table is indexed by dividing the netid by 2 
 * (netid >> 1) and using the result for the index value.
 *
 * PARAMETERS:
 *	fddi_acs_t *p_acs
 *	netid_t	netid
 */

#define GET_NETID_OPEN_PTR(p_acs, netid)				\
{									\
	( (netid < 0) || (netid > FDDI_MAX_NETID_VAL) ||		\
	  (netid & 0x01) ) ? (NULL) : 					\
			(p_acs->wrk.p_netids[(netid >> 1)])		\
}					

/* 
 * Macros to get/put the caller's parameter block when only an 
 * address is supplied. 
 *
 * INPUTS:
 *	dvf	user's devflag
 *	usa	user's source address 
 *	dda	local data destination address 
 *	size	size of local data
 *
 * ROUTINES CALLED:
 *	bcopy(), copyin(), copyout()
 *
 * CALLING SEQUENCE:
 *
 *	rc = MOVEIN(dvf, usa, dda, siz)
 *
 * RETURNS:
 *	0	- successful
 *	EFAULT	- copyin(),copyout() failed
 */
#define MOVEIN(dvf,usa,dda,siz)                               \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( copyin(usa,dda,siz) != 0) ) )

#define MOVEOUT(dvf,dda,usa,siz)                              \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( copyout(dda,usa,siz) != 0)  ) )

/*
 * This macro will increment a status queue index to the next 
 * status queue element.
 * We decrement the status queue size by one because our indexes 
 * are zero based.
 *
 * INPUTS:
 *	p_acs	ptr to the ACS
 *	index	a status queue index
 *
 * Example Calling sequence: 
 *
 *		INCR_STATQ_INDEX(p_acs, (p_open->stat_in) );
 */
#define INCR_STATQ_INDEX(p_acs, index)			\
	( (index == (p_acs->dds.stat_que_size-1)) ?	\
			(index=0) : (++index) )

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

/*
 * INCRE_TQ, TQ_FULL and DESC_TQ: Operate on the driver's circular transmit que.
 * This que handles the over flow from the adapter's transmit que.
 */

#define INCRE_TQ(i)						\
	{							\
		(i) = (((i) + 1) % (p_acs->dds.tx_que_sz));	\
	}							\

/* 
 * The variable 'i' indexes the array of structures comprising the 
 * software transmit queue.  The addition of 'i' to the base pointer
 * should increment the base pointer by the sizeof the structure through
 * the malloc'd memory.
 */
#define DESC_TQ(p,i)						\
	{							\
		(p) = p_acs->tx.p_tx_que + (i); 		\
	}							\

#define TQ_FULL(q,n)	(((q) + (n)) > p_acs->dds.tx_que_sz)	

/* 
 * FDDI_X_BOUND checks if data portion of an mbuf crosses a PAGE Boundary
 *	d - pointer to the data area of an mbuf
 *	l - the length of the data
 */
#define FDDI_X_BOUND(d,l) \
	((((uint)(d) & ~(PAGESIZE - 1)) + PAGESIZE)  <  ((uint)(d) + (l)))

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

#define PIO_GETCX(addr, c)						\
{									\
	int rc;								\
	if (rc = BUS_GETCX(addr, c))					\
		fddi_pio_retry(p_acs, rc, GETC, addr, (ulong)(c), NULL);\
}
#define PIO_GETSX(addr, s)						\
{									\
	int rc;								\
	if (rc = BUS_GETSX(addr, s))					\
		fddi_pio_retry(p_acs, rc, GETS, addr, (ulong)(s), NULL);\
}
#define PIO_GETSRX(addr, sr)						\
{									\
	int rc;								\
	if (rc = BUS_GETSRX(addr, sr))					\
		fddi_pio_retry(p_acs, rc, GETSR, addr, (ulong)(sr), NULL);\
}
#define PIO_GETLRX(addr, lr)						\
{									\
	int rc;								\
	if (rc = BUS_GETLRX(addr, lr))					\
		fddi_pio_retry(p_acs, rc, GETLR, addr, (ulong)(lr), NULL);\
}
#define PIO_PUTCX(addr, c)						\
{									\
	int rc;								\
	if (rc = BUS_PUTCX(addr, c))					\
		fddi_pio_retry(p_acs, rc, PUTC, addr, (ulong)(c), NULL);\
}
#define PIO_PUTSRX(addr, sr)						\
{									\
	int rc;								\
	if (rc = BUS_PUTSRX(addr, sr))					\
		fddi_pio_retry(p_acs, rc, PUTSR, addr, (ulong)(sr), NULL);\
}
#define PIO_PUTLRX(addr, lr)						\
{									\
	int rc;								\
	if (rc = BUS_PUTLRX(addr, lr))					\
		fddi_pio_retry(p_acs, rc, PUTLR, addr, (ulong)(lr), NULL);\
}
#define PIO_PUTSTRX(addr, src, c)					\
{									\
	int rc;								\
	if (rc = BUS_PUTSTRX(addr, src, c))				\
		fddi_pio_retry(p_acs, rc, PUTSTR, addr, src, c);	\
}
#define PIO_GETSTRX(addr, src, c)					\
{									\
	int rc;								\
	if (rc = BUS_GETSTRX(addr, src, c))				\
		fddi_pio_retry(p_acs, rc, GETSTR, addr, src, c);	\
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
	PIO_GETSTRX (bus + FDDI_LS_SHARED_RAM, &ls, sizeof (ls));	\
	ls.smt_error_lo = SWAPSHORT(ls.smt_error_lo);		\
	ls.smt_error_hi = SWAPSHORT(ls.smt_error_hi);		\
	ls.smt_event_lo = SWAPSHORT(ls.smt_event_lo);		\
	ls.smt_event_hi = SWAPSHORT(ls.smt_event_hi);		\
	ls.cpv 	   	= SWAPSHORT(ls.cpv);			\
	ls.port_event   = SWAPSHORT(ls.port_event);		\
	ls.setcount_lo  = ls.setcount_lo;			\
	ls.setcount_hi  = ls.setcount_hi;			\
	ls.aci_code 	= SWAPSHORT(ls.aci_code);		\
	ls.pframe_cnt 	= SWAPSHORT(ls.pframe_cnt);		\
	ls.ecm_sm 	= SWAPSHORT(ls.ecm_sm);			\
	ls.pcm_a_sm 	= SWAPSHORT(ls.pcm_a_sm);		\
	ls.pcm_b_sm 	= SWAPSHORT(ls.pcm_b_sm);		\
	ls.cfm_a_sm 	= SWAPSHORT(ls.cfm_a_sm);		\
	ls.cfm_b_sm 	= SWAPSHORT(ls.cfm_b_sm);		\
	ls.cf_sm 	= SWAPSHORT(ls.cf_sm);			\
	ls.mac_cfm_sm 	= SWAPSHORT(ls.mac_cfm_sm);		\
	ls.rmt_sm 	= SWAPSHORT(ls.rmt_sm);			\
	ls.sba_alloc_lo = SWAPSHORT(ls.sba_alloc_lo);		\
	ls.sba_alloc_hi = SWAPSHORT(ls.sba_alloc_hi);		\
	ls.tneg_lo 	= SWAPSHORT(ls.tneg_lo);		\
	ls.tneg_hi 	= SWAPSHORT(ls.tneg_hi);		\
	ls.payload_lo 	= SWAPSHORT(ls.payload_lo);		\
	ls.payload_hi 	= SWAPSHORT(ls.payload_hi);		\
	ls.overhead_lo 	= SWAPSHORT(ls.overhead_lo);		\
	ls.overhead_hi 	= SWAPSHORT(ls.overhead_hi);		\
	ls.ucode_ver 	= SWAPSHORT(ls.ucode_ver);		\
}


/* 
 * Macros for debug and error tracing.
 */

#define FDDI_TTRACE(a1,a2,a3,a4)	
#define FDDI_TRACE(a1,a2,a3,a4)	fddi_trace(a1,a2,a3,a4)

#ifdef PERF
#define FDDI_TRACE(a1,a2,a3,a4)		fddi_trace(a1,a2,a3,a4)
#define FDDI_PTRACE(a1)		fddi_trace(a1,0,0,0)
#else
#define FDDI_PTRACE(a1)	
#endif

#ifdef	FDDI_DEBUG
#define FDDI_DBTRACE(a1,a2,a3,a4)	fddi_trace(a1,a2,a3,a4)
#else
#define FDDI_DBTRACE(a1,a2,a3,a4)	
#endif

#endif /* endif ! _H_FDDIMACRO */
