/* @(#)13	1.2  src/bos/diag/tu/tok/toktst.h, tu_tok, bos411, 9428A410j 9/18/90 16:12:29 */
/* @(#)HTX toktst.h	1.13  4/23/90 15:16:40 */
/*
 * COMPONENT_NAME: (TOKENTU) Token Test Unit
 *
 * FUNCTIONS: Token Test Unit Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Header HTX/Mfg. Token Ring Definitions File

Module Name :  toktst.h
SCCS ID     :  1.13

Current Date:  5/14/90, 08:14:17
Newest Delta:  4/23/90, 15:16:40

Header file contains basic definitions needed by three applications for
testing the Token Ring Adapter:

	1)  Hardware exerciser invoked by HTX,
	2)  Manufacturing application, and
	3)  Diagnostic application.


*****************************************************************************/
/*
 * We include the HTX header file hxihtx.h because we have a need
 * to access this structure within test units in case the invoker
 * is an HTX Hardware Exerciser application (i.e., NOT the
 * manufacturing diagnostic application).  Since, the main driver
 * function, exectu(), has already been defined for use by both
 * types of applications, we will "sneak" in this structure 
 * inside the TUTYPE one that we are allowed to define and pass in.
 */
#include "hxihtx.h"

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define INVOKED_BY_HTX   99


/*
 * standard structure used by manufacturing diagnostics.
 */
struct tucb_t
   {
	long tu,	/* test unit number   */
	     loop,	/* loop test of tu    */
	     mfg;	/* flag = 1 if running mfg. diagnostics, else 0  */

	long r1,	/* reserved */
	     r2;	/* reserved */
   };

/*
 * error types and error codes for token ring test units.
 */
#define SYS_ERR     0x00
#define TOK_ERR     0x01
#define LOG_ERR     0x02

/*
 * LOG_ERR (logic errors)
 */
#define OPEN_DD_ERR   0x8000	/* failed to open device driver */
#define MEM_CMP_ERR   0x9400	/* transmit/rec'v buffers didn't match */
#define READ_ERR      0x9401	/* read timed-out with no explanation */

#define MEM_ERR       0x0001	/* malloc failed */
#define WR_TRUNC_ERR  0x0010	/* write was truncated */
#define RD_TRUNC_ERR  0x0011	/* read was truncated */
#define NETID_ERR     0x0020	/* illegal or bad network id */
#define CHAIN_LEN_ERR 0x0030	/* less than 1 frame in TX chain */

#define PAT_OPEN_ERR  0x0040	/* couldn't open spec. pattern file */
#define PAT_EMP_ERR   0x0041	/* empty pattern file */
#define OPEN_OPT_ERR  0x0050	/* couldn't specify open options */
#define TOK_ST1_ERR   0x0051	/* couldn't start (open) adapter */
#define ST1_DONE_ERR  0x0052	/* no start done - no status */
#define ST2_DONE_ERR  0x0053	/* start done, but unknown bad status */
#define ST3_DONE_ERR  0x0054	/* start not done, get stat failed */
#define NOT_DIAG_ERR  0x0055	/* adapter not in diagnostics mode */

#define TOK_WR1_ERR   0x0060	/* write failed - no status */
#define TOK_WR2_ERR   0x0061	/* write chain failed - no status */
#define TOK_TX1_ERR   0x0062	/* no tx done - no status */
#define TOK_TX2_ERR   0x0063	/* no tx done on chain - no status */
#define TOK_RD1_ERR   0x0064	/* read failed - no status */
#define TOK_RD2_ERR   0x0065	/* read failed - max attempts, no stat */

#define TOK_HA1_ERR   0x0070	/* couldn't halt - no status */
#define HA1_DONE_ERR  0x0071	/* no halt done - no status */

#define POS_RD_ERR    0x0080	/* couldn't read POS register */
#define POS0_RD_ERR   (POS_RD_ERR+0)
#define POS1_RD_ERR   (POS_RD_ERR+1)
#define POS2_RD_ERR   (POS_RD_ERR+2)
#define POS3_RD_ERR   (POS_RD_ERR+3)
#define POS4_RD_ERR   (POS_RD_ERR+4)
#define POS5_RD_ERR   (POS_RD_ERR+5)
#define POS6_RD_ERR   (POS_RD_ERR+6)
#define POS7_RD_ERR   (POS_RD_ERR+7)

#define POS_WR_ERR    0x0090	/* couldn't write POS register */
#define POS2_WR_ERR   (POS_WR_ERR+2)
#define POS3_WR_ERR   (POS_WR_ERR+3)
#define POS4_WR_ERR   (POS_WR_ERR+4)
#define POS5_WR_ERR   (POS_WR_ERR+5)
#define POS6_WR_ERR   (POS_WR_ERR+6)
#define POS7_WR_ERR   (POS_WR_ERR+7)

#define POS_CMP_ERR   0x0100	/* POS register r/w didn't compare */
#define POS0_CMP_ERR   (POS_CMP_ERR+0)
#define POS1_CMP_ERR   (POS_CMP_ERR+1)
#define POS2_CMP_ERR   (POS_CMP_ERR+2)
#define POS3_CMP_ERR   (POS_CMP_ERR+3)
#define POS4_CMP_ERR   (POS_CMP_ERR+4)
#define POS5_CMP_ERR   (POS_CMP_ERR+5)
#define POS6_CMP_ERR   (POS_CMP_ERR+6)
#define POS7_CMP_ERR   (POS_CMP_ERR+7)

#define TOK_SUS_ERR   0x0200	/* suspend TX failed */
#define TOK_RES_ERR   0x0201	/* resume failed */

#define QVPD_ERR      0x0300	/* vital product data error */

#define LOST_BLK_ERR  0x0FFF

/*
 * TOK_ERR (error codes returned from the adapter, itself)
 */
#define PASSED        0
#define	FAILED       -1

#define INIT_TST_ERR	0x0030 /* Initial test error. */
#define MC_CRC_ERR	0x0031 /* Microcode not loaded or
					microcode CRC error. */

#define RAM_ERR		0x0032 /* RAM error. */
#define INSTR_TST_ERR	0x0033 /* Instruction test error. */
#define XOP_INTR_ERR	0x0034 /* XOP test error, interrupt test error. */
#define PH_HDW_ERR	0x0035 /* PH hardware error. */
#define SIF_REG_ERR	0x0036 /* SIF register error. */
#define PARMS1_ERR	0x0011 /* Parameters written by system
					incomplete/invalid. */
#define PARMS2_ERR	0x0012 /* Parameters written by system
					incomplete/invalid. */
#define PARMS3_ERR	0x0013 /* Parameters written by system
					incomplete/invalid. */
#define PARMS4_ERR	0x0014 /* Parameters written by system
					incomplete/invalid. */
#define PARMS5_ERR	0x0015 /* Parameters written by system
					incomplete/invalid. */
#define PARMS6_ERR	0x0016 /* Parameters written by system
					incomplete/invalid. */
#define PARMS7_ERR	0x0017 /* Parameters written by system
					incomplete/invalid. */
#define MMIO_PAR_ERR	0x0018 /* MMIO parity error. */
#define DMA_TIM_ERR	0x0019 /* DMA timeout error. */
#define DMA_PAR_ERR	0x001A /* DMA parity error. */
#define DMA_BUS_ERR	0x001B /* DMA bus error. */
#define DMA_CMP_ERR	0x001C /* DMA data compare error. */
#define ADAP_CHK_ERR	0x001D /* Adapter check. */
#define NONS_INIT_ERR	0x8820 /* Nonsensical initialization results. */
#define INIT_CMP_ERR	0x8830 /* Initialization parms. in adapter don't
					compare. */
#define INIT_TIME_ERR	0x8832 /* Initialization timed out - no status. */
#define OPEN_TIME_ERR	0x8842 /* Time out during adapter open. */
#define OPN_PARM_ERR	0x8C00 /* Open parameters invalid. */
#define LM_WRAP_ERR	0x0211 /* Lobe media wrap test failed. */
#define LM_WRAP_PAS	0x0212 /* Lobe media wrap test passed. */

#define OPN_PINS_ERR	0x8D20 /* Open failed - physical insertion. */
#define OPN_ADDR_ERR	0x8D30 /* Open failed - address verification. */
#define OPN_RPOLL_ERR	0x8D40 /* Open failed - ring poll. */
#define OPN_RPARM_ERR	0x8D50 /* Open failed - request parameters. */
#define XMIT_PARM_ERR	0x9000 /* Transmit list parameter error. */
#define XMIT_FSTR_ERR	0x9100 /* Error during transmit or frame strip. */
#define XR_CMP_ERR	0x9400 /* Receive and transmit data did not match. */
#define WRAP_TIM_ERR	0x9401 /* Wrap time-out. */
#define SCB_BUSY0_ERR	0x9402 /* System Command Block busy. */
#define SCB_BUSY1_ERR	0x9403 /* System Command Block busy. */
#define UNX_SFT_ERR	0x9C00 /* Unexpected software error. */
#define RSTAT_OFF_ERR	0xA000 /* Ring status causing adapter to get off. */
#define UNX_HDW_ERR	0xA400 /* Unexpected hardware error. */
#define SCB_BUSY2_ERR	0xA408 /* SCB was busy not allowing CLOSE. */
#define ASY_CC_ERR	0xCCCC /* Async channel check by system - frame
								discarded. */
#define AC_XDMA_R_ERR	0xFFDA /* Adapter check, all except DMA abort - read.*/
#define OPN_TIM_ERR	0xFFDB /* OPEN timeout. */
#define AC_DMA_R_ERR	0xFFDC /* Adapter check, DMA abort - read. */
#define ADAP_NOPEN_ERR	0xFFF5 /* Adapter not OPENed. */
#define ADAP_OPN_ERR	0xFFF6 /* Adapter is already OPENed. */

/*
 * adapter specific definitions for token ring test units.
 */
#define RULE_LEN      8
#define RETRIES_LEN   3
#define PATTERN_LEN  80
#define NETADD_LEN    6
#define FAIR_LEN      3
#define STREAM_LEN    3
#define PRE_LEN       5
#define NET_OFFSET 0x0B		/* offset into VPD for network address */
#define TU_NET_ID  0xAA

struct _token_htx
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	char pattern_id[PATTERN_LEN+1];
	long frame_size;
	unsigned char network_address[NETADD_LEN];
	char fairness[FAIR_LEN+1];
	char streaming[STREAM_LEN+1];
	char preempt[PRE_LEN+1];
	struct htx_data *htx_sp;
   };

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */
#define TUTYPE struct _token_tu

struct _token_tu
   {
	struct tucb_t header;
	struct _token_htx token_s;
   };

