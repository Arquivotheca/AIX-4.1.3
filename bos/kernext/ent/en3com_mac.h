/* @(#)35  1.11  src/bos/kernext/ent/en3com_mac.h, sysxent, bos411, 9431A411a 8/2/94 12:47:34 */
/*
 * COMPONENT_NAME: sysxent --  High Performance Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/
#ifndef _H_EN3COM_MAC
#define _H_EN3COM_MAC

#include <sys/time.h>
#include <sys/trcmacros.h>


#define EN3COM_MAX_ADAPTERS	16	/* max. adapters supported by driver */
#define EN3COM_DD_NAME		"en3com"	/* driver name for dump */
#define ROS_LEVEL_SIZE  	4       /* bytes in ROS Level                */
#define PN_SIZE          	8       /* bytes in Part number              */
#define EC_SIZE          	6       /* bytes in Engineering Change no.   */
#define DD_SIZE          	2       /* bytes in Device Driver Level      */
#define MAX_MULTI        	10      /* max multicast addresses support
					   by adapter         */
#define MULTI_TABLE_SLOT 	16	/* no. of addr. in each multicast 
					   table  */
#define MIN_TXD       		10      /* Min Transmit buffer descriptors */
#define MIN_RVD			10      /* Min Receive buffer descriptors */
#define MAX_TXD       		64      /* Max Transmit buffer descriptors */
#define MAX_RVD			64      /* Max Receive buffer descriptors */
#define RAM_SIZE         	0x4000  /* Adapter RAM size - 16K            */
#define MEM_ALIGN		2
#define EN3COM_TRACE_END	0x43434343	/* 'CCCC' is end of trace */

#define ENT_HDR_LEN		14	/* Ethernet header length */
#define MULTI_BIT_MASK		0x01	/* multicast bit in first addr. byte */

#define CRR_DELAY		1	/* only wait 1 ms for command reg.
						to be ready		     */
/*
 * Internal trace table size and trace macros
 */

#ifdef DEBUG
#define EN3COM_TRACE_SIZE	1024		/* 4096 bytes, 256 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	en3com_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	\
	en3com_trace(hook, tag, arg1, arg2, arg3)
#else
#define EN3COM_TRACE_SIZE	128             /* 512 bytes, 32 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	TRCHKGT((hook << 20) | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	
#endif 

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3)	\
	en3com_trace(hook, tag, arg1, arg2, arg3)
/*
 * Trace points defined for the netpmon performance tool
 */
#define TRC_WQUE 	"WQUE" 	/* write data has just been queued */
#define TRC_WEND 	"WEND" 	/* write is complete              */
#define TRC_RDAT 	"RDAT" 	/* a packet was received    */
#define TRC_RNOT 	"RNOT" 	/* a packet was passed up        */
#define TRC_REND 	"REND" 	/* receive is complete      */


/* 
 * The component dump table will have the dd_ctl table,  and one dev_ctl 
 * for each adapter that is opened. Leave one extra entry always empty for
 * the table management.
 */
#define EN3COM_CDT_SIZE		(2 + EN3COM_MAX_ADAPTERS)

/*
 * delay mili-seconds
 */
#define DELAYMS(ms) 		delay ((int)(((ms)*HZ+999)/1000))

#define I_DELAYMS(p, ms)	{\
	en3com_loopdelay(p, (int)(((ms)*HZ+999)/1000)); \
				}

/*
 * Set and start system timer for the mili-second specified
 */
#define STIMER_MS(ms) { \
	p_dev_ctl->systimer->timeout.it_value.tv_sec = ms / 1000; \
        p_dev_ctl->systimer->timeout.it_value.tv_nsec = (ms % 1000) * 1000000; \
	tstart(p_dev_ctl->systimer); \
		      }


/*
 * compare two ethernet network address
 */
#define SAME_NADR(a, b)	( \
	*((ulong *)(a)) == *((ulong *)(b)) && \
	*((ushort *)((char *)(a) + 4)) == *((ushort *)((char *)(b) + 4)) \
			)
/*
 * copy a ethernet network address from a to b
 */
#define COPY_NADR(a, b)	{ \
	*((ulong *)(b)) = *((ulong *)(a)); \
	*((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \
			}

/*
 * calculate the elapsed time by subtract the start tick counter (s) from
 * the current tick counter (lbolt) and convert it to seconds
 */
#define ELAPSED_TIME(s)     ((lbolt - s) / HZ)


#endif /* _H_EN3COM_MAC */
