/* @(#)32       1.7  src/bos/kernext/mps_tok/mps_mac.h, sysxmps, bos41J, 9511A_all 3/7/95 15:28:15 */
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: COPY_NADR
 *		NDD_ELAPSED_TIME
 *		SAME_NADR
 *		TRACE_BOTH
 *		TRACE_DBG
 *		TRACE_SYS
 *		XMITQ_DEC
 *		XMITQ_INC
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MPS_MAC
#define _H_MPS_MAC

#define MPS_DD_NAME		"trmps"	/* driver name for dump             */
#define MEM_ALIGN		2
#define NUM_POS_REG		8       /* number of POS register            */
#define MAX_MULTI               256     /* max group addr support by adapter */
#define MULTI_ENTRY             16      /* len of multi_address table entry  */
#define MPS_HDR_LEN		14	/* Wildwood header length            */
#define MULTI_BIT_MASK		0x80	/* multicast bit in first addr. byte */
#define CARD_ENABLE		0x01	/* card enable bit in pos2 register  */
#define TOKEN_ACCESS_PRIORITY   0x06    /* priority Tx token access priority */

#define TOK_MAX_ADAPTERS   	(32)    /* maximum adapters in system        */
#define MAX_TX_LIST      	(32)    /* Maximum Transmit frame list       */
#define TX_LIST_SIZE      	(0x20)  /* length of TX element              */
#define MAX_RX_LIST      	(32)    /* Maximum Receive  buffer list      */
#define RX_LIST_SIZE     	(16)    /* size of receive elem              */
#define H_PAGE     		(2048)  /* Haft page size                    */


/*----------------------------------------------------------------------------*/
/*			DMA arae for TX & RX			              */
/*	Rx_DESCRIPTOR		0x0000   .. 0x1FFF		              */
/*	Tx1_DESCRIPTOR		0x2000   .. 0x3FFF		              */
/*	Tx2_DESCRIPTOR		0x4000   .. 0x5FFF		              */
/*	Rx_BUFF			0x6000   .. 0x25FFF		              */
/*	Tx1_BUF			0x26000  .. 0x45FFF		              */
/*	Tx2_BUF			0x46000  .. 0x65FFF		              */
/*	Tx1_DMA			0x66000  .. 0x105FFF		              */
/*	Tx2_DMA			0x106000 .. 0x1a5FFF		              */
/*----------------------------------------------------------------------------*/
#define RX_DESCRIPTOR_SIZE     	PAGESIZE * 2  /* size of receive  descriptor */
#define TX1_DESCRIPTOR_SIZE     PAGESIZE * 2  /* size of transmit descriptor */
#define TX2_DESCRIPTOR_SIZE     PAGESIZE * 2  /* size of transmit descriptor */
#define RX_BUF_OFFSET     	PAGESIZE * 6  /* Rx buffer offset            */
#define TX1_BUF_OFFSET     	RX_BUF_OFFSET  + (MAX_RX_LIST * PAGESIZE)
#define TX2_BUF_OFFSET          TX1_BUF_OFFSET + (MAX_TX_LIST * PAGESIZE)
#define TX1_DMA_ADDR_OFFSET     TX2_BUF_OFFSET + (MAX_TX_LIST * PAGESIZE)
#define TX2_DMA_ADDR_OFFSET     TX1_DMA_ADDR_OFFSET+(MAX_TX_LIST*(PAGESIZE *5))

/*----------------------------------------------------------------------------*/
/*                                RETURN CODES                                */
/* These are the possible error codes return from the adapter.  Some of them  */
/*			     are not currently use.                           */
/*----------------------------------------------------------------------------*/
#define OPERATION_SUCCESSFULLY  0x00    /* Operation Complete successfully    */
#define UNRECOGNIZED_CODE       0x01    /* Unrecognized command code          */
#define ADAPTER_OPEN            0x03    /* Adapter Open, should be close      */
#define ADAPTER_CLOSE           0x04    /* Apapter close, should be open      */
#define PARAMETER_NEED          0x05    /* Required parameter not provided    */
#define COMMAND_CANCELLED       0x07    /* Command cancelled                  */
#define UNAUTHORIZED_ACCESS     0x08    /* Unauthorized access priority       */
#define LOST_DATA               0x20    /* Lost data on receive               */
#define ERROR_FRAME             0x22    /* Error on frame transmission        */
#define ERROR_DATA              0x23    /* Error in frame transmit or strip   */
#define UNAUTHORIZED_MAC        0x24    /* Unauthorized MAC frame             */
#define UNRECOGNIZED_CORRELATOR 0x26    /* Unrecognized command correlator    */
#define INVALID_LEN             0x28    /* Invalid frame length for transmit  */
#define UNAUTHORIZED_LLC        0x29    /* Unauthorized LLC frame             */
#define INVALID_ROUTE           0x2A    /* Invalid routing information        */
#define ROUTE_NOT_EXPECT        0x2B    /* Routing information not expected   */
#define UNAUTHORIZE_ENT         0x2E    /* Unauthorized Ethernet frame        */
#define INVALID_NODE_ADDR       0x32    /* Invalid node address               */
#define INVALID_BUF_LEN         0x34    /* Invalid adapter transmit buf_len   */
#define GROUP_FULL              0x35    /* Group address register full        */
#define RING_OUT_OF_RANGE       0x36    /* Ring number out of range           */
#define CHANNEL_INUSER          0x37    /* Priority channel already in use    */
#define BRIDGE_NOT_ISSUED       0x38    /* Set bridge parameter not issued    */
#define NO_GROUP_ADDRESS        0x39    /* Group address not found            */
#define INVALID_SOURCE_ADDRESS  0x3A    /* Invalid source address             */
#define BRIDGE_OUT_RANGE        0x3B    /* Bridge number out of range         */
#define NOT_SET_GROUP           0x3C    /* Group/functional address indicator */
                                        /* bit not set correctly              */
#define NOT_MULTI_PORT          0x3D    /* Multi_port bridge option not select*/
#define INDEX_GROUP_FULL        0x3E    /* Indexed group register full        */
#define GEN_GROUP_FULL          0x3F    /* General group register full        */
#define NOT_MULTI_ADDR          0x50    /* Multiple address not expected      */
#define LOCAL_ADDR_LARGE        0x51    /* Number local address too large     */
#define NO_MULTI_BRIDGE         0x52    /* Target ring not expected for multi */
                                        /* port bridge                        */
#define RING_NOT_FOUND          0x53    /* Target ring not found              */
#define INVALID_GROUP_TYPE      0x54    /* Invalid group type                 */
#define ALREADY_SET_GROUP       0x55    /* Group address already set          */
#define NO_BRIDGE_CONFIG        0x56    /* Config bridge channel not issued   */
#define GROUP_ADDRESS_SET       0x5A    /* Group address set                  */
#define ADDRESS_NOT_FOUND       0x5B    /* address no found                   */
#define INVALID_CONFIGURATION   0x62    /* invalid configguration             */
#define RESPONSE_VALID          0xFF    /* Response valid, ASB available      */
#define PROMIS_ON          	0x41    /* promiscuous mode on                */
#define PROMIS_OFF          	0x00    /* promiscuous mode off               */


/******************************************************************************/
/*                             LAN STATUS CODES                               */
/******************************************************************************/
#define  SIGNAL_LOSS                 0x8000
#define  HARD_ERROR                  0x4000
#define  SOFT_ERROR                  0x2000
#define  TRANSMIT_BEACON             0x1000
#define  LOBE_WIRE_FAULT             0x0800
#define  AUTO_REMOVAL_ERROR          0x0400
#define  REMOVE_RECEIVE              0x0100
#define  COUNTER_OVERFLOW            0x0080
#define  SINGLE_STATION              0x0040
#define  RING_RECOVERY               0x0020
#define  SR_BRIDGE_COUNTER_OVERFLOW  0x0010
#define  CABLE_NOT_CONNECTED         0x0008
#define  LAN_OPEN_RESERVE            0x0207
#define  ERR_LOG                     0x2000 | 0x0080 | 0x0010

/*
 *      trace macros
 */
#define MPS_TRACE_TOP	"mpstraceTOP!!!!!"   /* top of trace table */
#define MPS_TRACE_BOT	"mpstraceBOT!!!!!"   /* bottom of trace table */
#define MPS_TRACE_CUR	"mpstraceCUR!!!!!"   /* Current trace table entry */

#define MPS_TX        ((HKWD_CDLI_MPS_XMIT << 20)  | HKTY_GT | 4)
#define MPS_RV        ((HKWD_CDLI_MPS_RECV << 20)  | HKTY_GT | 4)
#define MPS_ERR       ((HKWD_CDLI_MPS_ERR << 20) | HKTY_GT | 4)
#define MPS_OTHER     ((HKWD_CDLI_MPS_OTHER << 20) | HKTY_GT | 4)

/*
 * Trace points defined for the netpmon performance tool
 */
#define TRC_WQUE        "WQUE"  /* write data has just been queued */
#define TRC_WEND        "WEND"  /* write is complete              */
#define TRC_RDAT        "RDAT"  /* a packet was received    */
#define TRC_RNOT        "RNOT"  /* a packet was passed up        */
#define TRC_REND        "REND"  /* receive is complete      */

/*
 * Internal trace table size and trace macros
 */

#ifdef TRACE_DEBUG
#define MPS_TRACE_SIZE	1024		/* 4096 bytes, 256 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	mps_trace(hook, tag, arg1, arg2, arg3)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	\
	mps_trace(hook, tag, arg1, arg2, arg3)
#else
#define MPS_TRACE_SIZE	128             /* 512 bytes, 32 traces */
#define TRACE_SYS(hook, tag, arg1, arg2, arg3)	\
	TRCHKGT(hook, *(ulong *)tag, arg1, arg2, arg3, 0)
#define TRACE_DBG(hook, tag, arg1, arg2, arg3)	
#endif 

#define TRACE_BOTH(hook, tag, arg1, arg2, arg3)	\
	mps_trace(hook, tag, arg1, arg2, arg3)

/* 
 * The component dump table will have the dd_ctl table,  and one dev_ctl 
 * for each adapter that is opened. Leave one extra entry always empty for
 * the table management.
 */
#define MPS_CDT_SIZE		(2 + TOK_MAX_ADAPTERS)

/*
 * Compares two network address
 */
#define SAME_NADR(a, b) ( \
        *((ulong *)(a)) == *((ulong *)(b)) && \
        *((ushort *)((char *)(a) + 4)) == *((ushort *)((char *)(b) + 4)) \
                        )

/*
 * Copys a network address from a to b
 */
#define COPY_NADR(a, b)	{ \
	*((ulong *)(b)) = *((ulong *)(a)); \
	*((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \
			}
/*
 * Calculates the elapsed time by subtract the start tick counter (s) from
 * the current tick counter (lbolt) and convert it to seconds
 */
#define NDD_ELAPSED_TIME(s)     ((lbolt - s) / HZ)


#define XMITQ_DEC(x) {(x)--; if ((x) < 0) (x) = MAX_TX_LIST - 1;};
#define XMITQ_INC(x) {(x)++; if ((x) == MAX_TX_LIST) (x) = 0;};
#define XMITQ1_FULL      ( \
 (((WRK.tx1_elem_next_in + 1) == MAX_TX_LIST) ? 0 : WRK.tx1_elem_next_in + 1) \
 == WRK.tx1_elem_next_out)
#define XMITQ2_FULL      ( \
 (((WRK.tx2_elem_next_in + 1) == MAX_TX_LIST) ? 0 : WRK.tx2_elem_next_in + 1) \
 == WRK.tx2_elem_next_out)

#endif /* _H_MPS_MAC */
