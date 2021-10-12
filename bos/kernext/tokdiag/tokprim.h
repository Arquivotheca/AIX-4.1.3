/* @(#)30	1.12  src/bos/kernext/tokdiag/tokprim.h, diagddtok, bos411, 9428A410j 5/6/91 15:52:43 */
#ifndef _H_TOKPRIM
#define _H_TOKPRIM

/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokprim.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define DISABLE_INTERRUPTS(lvl) lvl=i_disable(INTMAX)
#define ENABLE_INTERRUPTS(lvl)  i_enable(lvl)
#define KMALLOC(size)           xmalloc ((uint)(size), (uint)0, pinned_heap)
#define KMFREE(ptr)             xmfree ((void *)ptr, pinned_heap)
#define SLEEP(el)               e_sleep (el, EVENT_SIGRET)
#define WAKEUP(el)              e_wakeup (el)

#define TRACE1(s)               save_trace(1,s)
#define TRACE2(s,a2)            save_trace(2,s,a2)
#define TRACE3(s,a2,a3)         save_trace(3,s,a2,a3)
#define TRACE4(s,a2,a3,a4)      save_trace(4,s,a2,a3,a4)
#define TRACE5(s,a2,a3,a4,a5)   save_trace(5,s,a2,a3,a4,a5)


/* #define TOKDEBUG_TRACE  (1)    */

#ifdef TOKDEBUG_TRACE
#   define DEBUGTRACE1(s)               save_trace(1,s)
#   define DEBUGTRACE2(s,a2)            save_trace(2,s,a2)
#   define DEBUGTRACE3(s,a2,a3)         save_trace(3,s,a2,a3)
#   define DEBUGTRACE4(s,a2,a3,a4)      save_trace(4,s,a2,a3,a4)
#   define DEBUGTRACE5(s,a2,a3,a4,a5)   save_trace(5,s,a2,a3,a4,a5)
#else
#   define DEBUGTRACE1(s)               (0)
#   define DEBUGTRACE2(s,a2)            (0)
#   define DEBUGTRACE3(s,a2,a3)         (0)
#   define DEBUGTRACE4(s,a2,a3,a4)      (0)
#   define DEBUGTRACE5(s,a2,a3,a4,a5)   (0)
#endif /* DEBUG */

/*
    Macro to provide permanent trace points for system activity monitoring.
 */

#define TRC_WQUE (0x77515545) /* tokwrite -- write data has just been queued */
#define TRC_WBEG (0x77424547) /* ds_xmit or tokoflv -- data dequeued to adp*/
#define TRC_WEND (0x77454E44) /* xmit_done -- write is complete          */
#define TRC_RDAT (0x72444154) /* tokoflv -- a packet was received           */
#define TRC_RNOT (0x724E4F54) /* proc_recv -- kernel proc passed data    */
#define TRC_RQUE (0x72515545) /* proc_recv -- read data queued for user  */
#define TRC_ROVR (0x724F5652) /* proc_recv -- user proc read queue ovfl  */
#define TRC_REND (0x72454E44) /* tokread -- data delivered to user proc      */

#define AIXTRACE(tp,mm,ch,mb,ln) {                                      \
   TRCHKGT (TRACE_HOOKWORD | HKTY_GT | 0,                               \
      (ulong)(tp), (ulong)(mm), (ulong)(ch), (ulong)(mb), (ulong)(ln)); \
}

/*
 * Macros to get/put caller's parameter block when only an address is supplied.
 * Useful for "arg" in ioctl and for extended paramters on other dd entries.
 * Value is 0 if ok, otherwise EFAULT.  Typical usage is:
 * if (rc = COPYIN (devflag, arg, &localdata, sizeof(localdata)))
 *    return (rc);
 */
#define COPYIN(dvf,usa,dda,siz)                               \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( (rc = copyin(usa,dda,siz) != 0) ? rc : 0  ) ) )

#define COPYOUT(dvf,dda,usa,siz)                              \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( (rc = copyout(dda,usa,siz) !=0) ? rc : 0 ) ) )


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

# define GET_NETID(frameptr) \
                 *((unsigned char *)((int)frameptr + TOK_NETID_RECV_OFFSET))

# define MAC_FRAME(frameptr) \
                 ((*((unsigned char *)((int)(frameptr) + TOK_FC_OFFSET))) & \
                 FC_TYPE) == MAC_TYPE

/*
 * This macro will schedule off-level if necessary
 * INPUT: pointer to DDS
 */
#define SCHED_OFLV( p_dds )             \
        {       \
            if ( (!p_dds->ofl.scheduled) && (!p_dds->ofl.running) )     \
            {                                                   \
               i_sched ((struct intr *) &(p_dds->ofl) );        \
               p_dds->ofl.scheduled = TRUE;                     \
            }                                                   \
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
 * This macro will take a transmit list element's address
 * (one that resides in the TX chain) and return the array
 * index for the transmit control variables.
 */

#define GET_TX_INDEX( p_addr, p_head )	\
	( ( (unsigned int)(p_addr) - (unsigned int)(p_head) ) >> (LOG2_TX_LIST_SIZE) )	

/*
 * The CROSS_BOUND macros is used to determine if the buffer pointed to
 * by p_data crosses a page boundary.
 */
#define CROSS_BOUND(p_data, len)        \
                ( ( ( ((unsigned int)(p_data)) & ~(PAGESIZE-1) )                \
                         + (PAGESIZE)  )  <             \
                ( ((unsigned int)(p_data)) + ((unsigned int)(len)) ) )

# define ADDR_LO(addr)   ((int)(addr) & 0xFFFF)         /* Low 16 bits */
# define ADDR_HI(addr)  (((int)(addr) >> 16) & 0xFFFF)  /* High 16 bits */
                                                        /* Swap lo and hi */
# define BYTESWAP(x)    ((((unsigned short)(x) & 0x00FF) << 8) | \
                         (((unsigned short)(x) & 0xFF00) >> 8))

# define BITS_SET(word, bitmask) ((word)&(bitmask)) == (bitmask)


/*-------------------------------------------------------------------------*/
/*                            PIO ACCESSORS                                */
/*-------------------------------------------------------------------------*/

/* PIO register definitions */

# define DATA_REG       (sizeof(short) << 8) | offsetof(pass2_regs_t, data)
# define AUTOINCR_REG   (sizeof(short) << 8) | offsetof(pass2_regs_t, \
                                                        data_autoincr)
# define ADDRESS_REG    (sizeof(short) << 8) | offsetof(pass2_regs_t, addr)
# define COMMAND_REG    (sizeof(short) << 8) | offsetof(pass2_regs_t, cmd)
# define ENABLE_REG     (sizeof(char)  << 8) | offsetof(pass2_regs_t, enable)
# define RESET_REG      (sizeof(char)  << 8) | offsetof(pass2_regs_t, reset)
# define IMASK_ENABLE   (sizeof(char)  << 8) | offsetof(pass2_regs_t, \
                                                        imask_enable)
# define IMASK_DISABLE  (sizeof(char)  << 8) | offsetof(pass2_regs_t, \
                                                        imask_disable)
# define TIMER_REG      (sizeof(short) << 8) | offsetof(pass2_regs_t, timer)
# define STATUS_REG     (sizeof(short) << 8) | offsetof(pass2_regs_t, status)

# define REG_SIZE(reg)          (((reg) >> 8) & 0xFF)
# define REG_OFFSET(reg)        ((reg) & 0xFF)

#endif     /* ! _H_TOKPRIM */
