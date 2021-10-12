/* @(#)98	1.11  src/bos/kernext/tok/tokmacros.h, sysxtok, bos411, 9428A410j 5/26/94 16:22:48 */
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: ADDR_HI
 *		ADDR_LO
 *		GET_TX_INDEX
 *		MCOPY_TO_BUF
 *		NEXT_D_TX_AVAIL
 *		NEXT_TX_AVAIL
 *		REG_OFFSET
 *		TOK_GETCX
 *		TOK_GETLX
 *		TOK_GETPOS
 *		TOK_GETSRX
 *		TOK_PUTCX
 *		TOK_PUTPOS
 *		TOK_PUTSRX
 *		TRACE_BOTH
 *		TRACE_DBG
 *		TRACE_SYS
 *		XMITQ_INC
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

#ifndef _H_TOKMACROS
#define _H_TOKMACROS

/*
 *	trace macros
 */
#define MON_TRACE_NEXT	0x4E455854	/* "NEXT" marks next entry in table */
#define MON_XMIT	((HKWD_CDLI_MON_XMIT << 20) | HKTY_GT | 4)
#define MON_RECV	((HKWD_CDLI_MON_RECV << 20) | HKTY_GT | 4)
#define MON_OTHER	((HKWD_CDLI_MON_OTHER << 20) | HKTY_GT | 4)

#ifdef TOKDEBUG_TRACE
#define TRACE_TABLE_SIZE	1024		/* 4096 bytes, 256 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	save_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	\
	save_trace(hook, tag, arg1, arg2, arg3)
#else
#define TRACE_TABLE_SIZE	256		/* 1024 bytes, 64 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	TRCHKGT(hook, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	
#endif 

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3)	\
	save_trace(hook, tag, arg1, arg2, arg3)

#define TRC_WQUE "WQUE" /* tok_output -- write data has just been queued */
#define TRC_WEND "WEND" /* xmit_done -- write is complete              */
#define TRC_RDAT "RDAT" /* read_recv_chain -- a packet was received    */
#define TRC_RNOT "RNOT" /* proc_recv -- demuxer passed data	       */
#define TRC_REND "REND" /* proc_recv -- return from demuxer	       */

/*-------------------------------------------------------------------------*/
/*                              MACROS                                     */
/*-------------------------------------------------------------------------*/

#define	MCOPY_TO_BUF(mp, buf)					\
{								\
	struct	mbuf	*m = mp;				\
	caddr_t		cp = buf;				\
								\
	while (m) {						\
		bcopy(mtod(m, caddr_t), cp, m->m_len);		\
		cp += m->m_len;					\
		m = m->m_next;					\
	}							\
}

/*
 *  The NEXT_TX_AVAIL and NEXT_D_TX_AVAIL macros return the pointer
 *  (virtual and BUS address) of the next location in the Transmit
 *  List Chain.
 */
#define NEXT_TX_AVAIL(p_tx, p_dds)             \
   ( (p_tx == p_dds->wrk.p_tx_tail) ?          \
       (p_tx = p_dds->wrk.p_tx_head) : (++p_tx) )

#define NEXT_D_TX_AVAIL(p_d_tx, p_dds)         \
   ( (p_d_tx == p_dds->wrk.p_d_tx_tail) ?      \
       ( p_d_tx = p_dds->wrk.p_d_tx_head) : (++p_d_tx) )

/*
 * The log base 2 of the size of a transmit list element
 */
#define LOG2_TX_LIST_SIZE	6

/*
 * This macro will take a transmit list element's address (one that resides
 * in the TX chain) and returns the array index for the p_d_tx_fwds table
 */

#define GET_TX_INDEX( p_addr, p_head )	\
	( ( (unsigned int)(p_addr) - (unsigned int)(p_head) ) >> (LOG2_TX_LIST_SIZE) )	

# define ADDR_LO(addr)   ((int)(addr) & 0xFFFF)         /* Low 16 bits */
# define ADDR_HI(addr)  (((int)(addr) >> 16) & 0xFFFF)  /* High 16 bits */
                                                        /* Swap lo and hi */

#define NDD_ELAPSED_TIME(s)	((lbolt -s) / HZ)

/*
 * copy a token-ring network address from a to b
 */
#define COPY_NADR(a, b)	{ \
	*((ulong *)(b)) = *((ulong *)(a)); \
	*((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \
			}

#define XMITQ_INC(x) {(x)++; if ((x)==(short)DDI.xmt_que_size) (x) = 0;}
#define XMITQ_FULL 	( \
 ((WRK.tx_que_next_buf+1 == DDI.xmt_que_size) ? 0:WRK.tx_que_next_buf+1)\
			 == WRK.tx_que_next_out)

/*-------------------------------------------------------------------------*/
/*                            PIO ACCESSORS                                */
/*-------------------------------------------------------------------------*/

/* PIO register definitions */

# define DATA_REG       offsetof(pass2_regs_t, data)
# define AUTOINCR_REG   offsetof(pass2_regs_t, data_autoincr)
# define ADDRESS_REG    offsetof(pass2_regs_t, addr)
# define COMMAND_REG    offsetof(pass2_regs_t, cmd)
# define ENABLE_REG     offsetof(pass2_regs_t, enable)
# define RESET_REG      offsetof(pass2_regs_t, reset)
# define IMASK_ENABLE   offsetof(pass2_regs_t, imask_enable)
# define IMASK_DISABLE  offsetof(pass2_regs_t, imask_disable)
# define TIMER_REG      offsetof(pass2_regs_t, timer)
# define STATUS_REG     offsetof(pass2_regs_t, status)

/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur.					  */
/*									  */
/*  PIO_GETPOS and PIO_PUTPOS were added to implement the pio operations  */
/*  for the pos registers.  The pos registers differ from other pio  	  */
/*  operations as there isn't any checking done.  To check the values on a*/
/*  get operation, the driver will get the value twice and compare what   */
/*  was received.  On a put the driver will write it and then get the     */
/*  register's contents to compare with.				  */
/*------------------------------------------------------------------------*/

enum pio_func
{
	GETC, GETSR, GETL, PUTC, PUTSR
};

#define PIO_RETRY_COUNT		3

#define TOK_GETPOS(addr, c)						\
{									\
	uchar pos_c;							\
	uint pos_i;							\
									\
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) {			\
		BUS_GETCX((char *)(addr), (char *)(c));			\
		BUS_GETCX((char *)(addr), &pos_c);			\
		if (*(char *)(c) == pos_c)				\
			break;						\
	}								\
	if (pos_i >= PIO_RETRY_COUNT) {					\
		WRK.pio_rc = TRUE;					\
		WRK.pio_addr = addr;					\
		logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,		\
			 __FILE__);					\
	}								\
}
	
#define TOK_PUTPOS(addr, c)						\
{									\
	uchar pos_c;							\
	uint pos_i;							\
									\
	for (pos_i=0; pos_i<PIO_RETRY_COUNT; pos_i++) {			\
		BUS_PUTCX((char *)(addr), (char)(c));			\
		BUS_GETCX((char *)(addr), &pos_c);			\
		if ((char)(c) == pos_c)					\
			break;						\
	}								\
	if (pos_i >= PIO_RETRY_COUNT) {					\
		WRK.pio_rc = TRUE;					\
		WRK.pio_addr = addr;					\
		logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,		\
			 __FILE__);					\
	}								\
}

#define TOK_GETCX(addr, c)						\
{									\
	if (WRK.piox = BUS_GETCX((char *)(addr), (char *)(c))) {	\
		if (pio_retry(p_dds, GETC, (int)(addr),	(long)(c))) {	\
			logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,	\
				__FILE__);				\
		}							\
	}								\
}

#define TOK_GETSRX(addr, sr)						\
{									\
	if (WRK.piox = BUS_GETSRX((short *)(addr), (short *)(sr))) {	\
		if (pio_retry(p_dds, GETSR, (int)(addr), (long)(sr))) {	\
			logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,	\
				__FILE__);				\
		}							\
	}								\
}

#define TOK_GETLX(addr, lr)						\
{									\
	if (WRK.piox = BUS_GETLX((long *)(addr), (long *)(lr))) {	\
		if (pio_retry(p_dds, GETL, (int)(addr), (long)(lr))) {	\
			logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,	\
				__FILE__);				\
		}							\
	}								\
}

#define TOK_PUTCX(addr, c)						\
{									\
	if (WRK.piox = BUS_PUTCX((char *)(addr), (char)(c))) {		\
		if (pio_retry(p_dds, PUTC, (int)(addr), (long)(c))) {	\
			logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,	\
				__FILE__);				\
		}							\
	}								\
}

#define TOK_PUTSRX(addr, sr)						\
{									\
	if (WRK.piox = BUS_PUTSRX((short *)(addr), (short)(sr))) {	\
		if (pio_retry(p_dds, PUTSR, (int)(addr), (long)(sr))) {	\
			logerr(p_dds, ERRID_CTOK_DEVICE_ERR, __LINE__,	\
				__FILE__);				\
		}							\
	}								\
}

#endif     /* ! _H_TOKMACROS */
