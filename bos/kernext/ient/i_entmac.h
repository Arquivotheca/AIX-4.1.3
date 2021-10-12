/* @(#)43  1.7  src/bos/kernext/ient/i_entmac.h, sysxient, bos411, 9428A410j 7/1/94 16:31:40 */
/****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS: COPY_NADR
 *              SAME_NADR
 *              TRACE_BOTH
 *              TRACE_DBG
 *              TRACE_SYS
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/
#ifndef _H_IENT_MAC
#define _H_IENT_MAC

#define IENT_DD_NAME      "ient_dd"
#define ENT_HDR_LEN       14           /* Ethernet header length */
#define MULTI_BIT_MASK    0x01         /* multicast bit in first addr. byte */

#define TRACE_HOOKWORD   HKWD_DD_ETHDD /* in <sys/trchkid.h>  */
#define MAX_MULTI        20            /* max multicast addresses */
#define MEM_ALIGN        2
#define IENT_TRACE_END   0x2A2A2A2A

/*
** Internal trace table size and trace macros
*/
#ifdef DEBUG
#define TRACE_TABLE_SIZE        (2048*4)         /* SAVETRACE table (ulongs) */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)  \
        ient_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)  \
        ient_trace(hook, tag, arg1, arg2, arg3)
#else
#define TRACE_TABLE_SIZE        (250*4)         /* SAVETRACE table (ulongs) */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)  \
        TRCHKGT((hook << 20) | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)
#endif

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3) \
        ient_trace(hook, tag, arg1, arg2, arg3)

/*
 * compare two ethernet network address
 */
#define SAME_NADR(a, b) ( \
        *((ulong *)(a)) == *((ulong *)(b)) && \
        *((ushort *)((char *)(a) + 4)) == *((ushort *)((char *)(b) + 4)) \
                        )
/*
 * copy a ethernet network address from a to b
 */
#define COPY_NADR(a, b) { \
        *((ulong *)(b)) = *((ulong *)(a)); \
        *((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \
                        }

/*
 * calculate the elapsed time by subtract the start tick counter (s) from
 * the current tick counter (lbolt) and convert it to seconds
 */
#define ELAPSED_TIME(s)     ((lbolt - s) / HZ)

#endif /* _H_IENT_MAC */
