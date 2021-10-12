/* @(#)88	1.6  src/bos/usr/include/POWER/ethtst.h, daeth, bos411, 9428A410j 5/20/92 13:31:30 */
/*
 * COMPONENT_NAME: (ETHERTU) Ethernet Test Unit
 *
 * FUNCTIONS: Ethernet Test Unit Header File
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

Header HTX/Mfg. Ethernet Adapter Definitions File

Module Name :  ethtst.h
SCCS ID     :  1.22

Current Date:  12/12/90, 13:10:32
Newest Delta:  12/12/90, 13:10:00

Header file contains basic definitions needed by three applications for
testing the Ethernet Adapter:

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


/*****************************************************/
/*   Ethernet Adapter Board Test Units Header File   */
/*****************************************************/
/*         initial values for pos reg                          */
#define  INTPOS0         0xf5    /* adapter id                 */
#define  INTPOS1         0x8e    /* adapter id                 */
/* #define  INTPOS2      0x41  *//* initial value for pos reg 2*/
#define  INTPOS3         0x4     /* initial value for pos reg 3*/
#define  INTPOS4         0x1     /* initial value for pos reg 4*/
#define  INTPOS5         0x4f    /* initial value for pos reg 5*/
#define  INTPOS6         0x00    /* initial value for pos reg 6*/
#define  INTPOS7         0x00    /* initial value for pos reg 7*/

#define INTERNAL	0x01	/* wrap types */
#define EXTERNAL_BNC    0x03
#define EXTERNAL_DIX    0x05

#define LOCK_DMA	0x01
#define UNLOCK_DMA	0x02


#define ALLOCATE_DMA	0x01	/* DMA commands */
#define WRITE_DMA	0x02
#define READ_DMA	0x03

#define FREE_DMA	0x04

#define AT		0x01	/* Adapter types */
#define AT_PARTNO	"022F9381"
#define AT2_PN_PRO	"PROT0ETH"

#define PS2		0x02
#define PS2_PARTNO	"058F2881"
#define PS2_PN_REV2	"071F0927"
#define PS2_PN_REV3	"031F4073"

#define PS3		0x03

struct mailbox
   {
	unsigned short a_status;	/* address of status value */
	unsigned short a_list_ptr;	/* address of List Pointer value */
   };

struct buffer_descriptor
   {
	unsigned short a_ctrl_status;	/* address of control/status value */
	unsigned short a_next_ptr;	/* address of Next pointer value */
	unsigned short a_count;		/* address of count value */
	unsigned short a_buf_lsw_ptr;	/* address of buf_lsw pointer value */
	unsigned short a_buf_msw_ptr;	/* address of buf_msw pointer value */
   };

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
 * error types and error codes for tca test units.
 */
#define SYS_ERR     0x00
#define LOG_ERR     0x02

/*
 * LOG_ERR (logic errors)
 */

/*
 * adapter specific definitions for test units.
 */
#define RULE_LEN      8
#define RETRIES_LEN   3
#define YES_NO_LEN    3
#define CONFIG_LEN   15
#define NETADD_LEN    6
#define PARTNO_LEN    8

#define DISABLE       0
#define ENABLE        1

struct _eth_htx
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	char show[YES_NO_LEN+1];
	unsigned long rcv_filter;

	unsigned long main_offset;
	unsigned long exec_mbox_offset;
	unsigned long config_table[CONFIG_LEN];
	unsigned char net_addr[NETADD_LEN];
	unsigned short netid;
	int wrap_type;
	int fairness;
	int parity;
	int adapter_type;
	long abm;
	long packet_size;
	long num_packets;
	struct htx_data *htx_sp;
   };

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */
#define TUTYPE struct _eth_tu

struct _eth_tu
   {
	struct tucb_t header;
	struct _eth_htx eth_htx_s;
   };

#define BOOLEAN	unsigned char
#define BYTE	unsigned char
#define HALFWD	unsigned short
#define WORD	unsigned long

/*
 * Error codes
 */

#define POS0_RD_ERR	0x0100	/* Error reading POS register 0 */
#define POS1_RD_ERR	0x0101	/* Error reading POS register 1 */
#define POS2_RD_ERR	0x0102	/* Error reading POS register 2 */
#define POS3_RD_ERR	0x0103	/* Error reading POS register 3 */
#define POS4_RD_ERR	0x0104	/* Error reading POS register 4 */
#define POS5_RD_ERR	0x0105	/* Error reading POS register 5 */
#define POS6_RD_ERR	0x0106	/* Error reading POS register 6 */
#define POS7_RD_ERR	0x0107	/* Error reading POS register 7 */

#define POS0_WR_ERR	0x0110	/* Error writing POS register 0 */
#define POS1_WR_ERR	0x0111	/* Error writing POS register 1 */
#define POS2_WR_ERR	0x0112	/* Error writing POS register 2 */
#define POS3_WR_ERR	0x0113	/* Error writing POS register 3 */
#define POS4_WR_ERR	0x0114	/* Error writing POS register 4 */
#define POS5_WR_ERR	0x0115	/* Error writing POS register 5 */
#define POS6_WR_ERR	0x0116	/* Error writing POS register 6 */
#define POS7_WR_ERR	0x0117	/* Error writing POS register 7 */

#define POS0_CMP_ERR	0x0120	/* POS 0 has invalid value */
#define POS1_CMP_ERR	0x0121	/* POS 1 has invalid value */
#define POS2_CMP_ERR	0x0122	/* POS 2 Write/Read did not compare */
#define POS3_CMP_ERR	0x0123	/* POS 3 Write/Read did not compare */
#define POS4_CMP_ERR	0x0124	/* POS 4 Write/Read did not compare */
#define POS5_CMP_ERR	0x0125	/* POS 5 Write/Read did not compare */
#define POS6_CMP_ERR	0x0126	/* POS 6 Write/Read did not compare */
#define POS7_CMP_ERR	0x0127	/* POS 7 Write/Read did not compare */

#define RPAR_RD_ERR	0x0200	/* Error reading Parity Register */
#define RRAM_RD_ERR	0x0201	/* Error reading RAM Page Register */
#define RCTL_RD_ERR	0x0202	/* Error reading Control Register */
#define RSTA_RD_ERR	0x0203	/* Error reading Status Register */
#define RCMD_RD_ERR	0x0204	/* Error reading Command Register */
#define QCMD_RD_ERR	0x0205	/* Error reading Command Register queue */
#define QSTA_RD_ERR     0x0206  /* Error reading Status Register queue */
#define RSTA_ER_ERR 	0x0207	/* Error value in Status register */

#define RPAR_WR_ERR	0x0210	/* Error writing Parity Register */
#define RRAM_WR_ERR	0x0211	/* Error writing RAM Page Register */
#define RCTL_WR_ERR	0x0212	/* Error writing Control Register */
#define RSTA_WR_ERR	0x0213	/* Error writing Status Register */
#define RCMD_WR_ERR	0x0214	/* Error writing Command Register */

#define RPAR_CMP_ERR	0x0220	/* Parity Register Write/Read did not compare */
#define RRAM_CMP_ERR	0x0221	/* RAM Page Register Write/Read did not compare */
#define RCTL_CMP_ERR	0x0222	/* Control Register Write/Read did not compare */
#define RSTA_CMP_ERR	0x0223	/* Status Register Write/Read did not compare */
#define RCMD_CMP_ERR	0x0224	/* Command Register Write/Read did not compare */
#define RCMD_NOT_ERR	0x0234	/* Cmd in Command Register not read by adapter*/
#define REC_NOT_ERR	0x1234	/* Start reception command not read by adapter*/

#define MEM_RD_ERR	0x0300	/* Error reading shared memory */
#define MEM_WR_ERR	0x0301	/* Error writing shared memory */
#define MEM_CMP_ERR	0x0302	/* Memory Write/Read did not compare */

#define TXMB_RD_ERR	0x0400	/* Error reading Transmit Mailbox */
#define RXMB_RD_ERR	0x0401	/* Error reading Receive Mailbox */
#define EXMB_RD_ERR	0x0402	/* Error reading Execute Mailbox */
#define MB_RD_ERR	0x0409	/* Error reading Mailbox */

#define TXMB_WR_ERR	0x0410	/* Error writing Transmit Mailbox */
#define RXMB_WR_ERR	0x0411	/* Error writing Receive Mailbox */
#define EXMB_WR_ERR	0x0412	/* Error writing Execute Mailbox */
#define MB_WR_ERR	0x0419	/* Error writing Mailbox */

#define TXMB_CMP_ERR	0x0420	/* Write/Read of Transmit Mailbox did not compare */
#define RXMB_CMP_ERR	0x0421	/* Write/Read of Receive Mailbox did not compare */
#define EXMB_CMP_ERR	0x0422	/* Write/Read of Execute Mailbox did not compare */
#define MB_CMP_ERR	0x0429	/* Write/Read of Mailbox did not compare */

#define TXMB_ERR_ERR	0x0430	/* Error value in Transmit Mailbox status */
#define RXMB_ERR_ERR	0x0431	/* Error value in Receive Mailbox status */
#define EXMB_ERR_ERR	0x0432	/* Error value in Execute Mailbox status */
#define MB_ERR_ERR	0x0439	/* Error value in Mailbox status */

#define TXMB_BSY_ERR	0x0440	/* Transmit Mailbox busy */
#define RXMB_BSY_ERR	0x0441	/* Receive Mailbox busy */
#define EXMB_BSY_ERR	0x0442	/* Execute Mailbox busy */
#define MB_BSY_ERR	0x0449	/* Mailbox busy */

#define TXMB_LIS_ERR	0x0450	/* Error reading list pointer of Transmit Mailbox */
#define RXMB_LIS_ERR	0x0451	/* Error reading list pointer of Receive Mailbox */
#define RXMB_LISW_ERR	0x0452	/* Error writing list pointer of Receive Mailbox */

#define TXBD_CT_RD_ERR	0x0500	/* Error reading Transmit Buffer Descriptor Control */
#define TXBD_ST_RD_ERR	0x0501	/* Error reading Transmit Buffer Descriptor Status */
#define TXBD_NX_RD_ERR	0x0502	/* Error reading Transmit Buffer Descriptor Next */
#define TXBD_CO_RD_ERR	0x0503	/* Error reading Transmit Buffer Descriptor Count */
#define TXBD_LS_RD_ERR	0x0504	/* Error reading Transmit Buffer Descriptor LSW */
#define TXBD_MS_RD_ERR	0x0505	/* Error reading Transmit Buffer Descriptor MSW */

#define TXBD_CT_WR_ERR	0x0510	/* Error writing Transmit Buffer Descriptor Control */
#define TXBD_ST_WR_ERR	0x0511	/* Error writing Transmit Buffer Descriptor Status */
#define TXBD_NX_WR_ERR	0x0512	/* Error writing Transmit Buffer Descriptor Next */
#define TXBD_CO_WR_ERR	0x0513	/* Error writing Transmit Buffer Descriptor Count */
#define TXBD_LS_WR_ERR	0x0514	/* Error writing Transmit Buffer Descriptor LSW */
#define TXBD_MS_WR_ERR	0x0515	/* Error writing Transmit Buffer Descriptor MSW */

#define TXBD_TIME_ERR	0x0550	/* Time out - completion for TX Buffer descriptor */
#define TXBD_NOK_ERR	0x0551	/* TX Buffer Descriptor Status - NOT okay */

#define RXBD_CT_RD_ERR	0x0506	/* Error reading Receive Buffer Descriptor Control */
#define RXBD_ST_RD_ERR	0x0507	/* Error reading Receive Buffer Descriptor Status */
#define RXBD_NX_RD_ERR	0x0508	/* Error reading Receive Buffer Descriptor Next */
#define RXBD_CO_RD_ERR	0x0509	/* Error reading Receive Buffer Descriptor Count */
#define RXBD_LS_RD_ERR	0x050A	/* Error reading Receive Buffer Descriptor LSW */
#define RXBD_MS_RD_ERR	0x050B	/* Error reading Receive Buffer Descriptor MSW */

#define RXBD_CT_WR_ERR	0x0516	/* Error writing Receive Buffer Descriptor Control */
#define RXBD_ST_WR_ERR	0x0517	/* Error writing Receive Buffer Descriptor Status */
#define RXBD_NX_WR_ERR	0x0518	/* Error writing Receive Buffer Descriptor Next */
#define RXBD_CO_WR_ERR	0x0519	/* Error writing Receive Buffer Descriptor Count */
#define RXBD_LS_WR_ERR	0x051A	/* Error writing Receive Buffer Descriptor LSW */
#define RXBD_MS_WR_ERR	0x051B	/* Error writing Receive Buffer Descriptor MSW */

#define RXBD_TIME_ERR	0x0556	/* Time out - completion for RX Buffer descriptor */
#define RXBD_NOK_ERR	0x0557	/* RX Buffer Descriptor Status - NOT okay */

#define DMA_NOTA_ERR	0x0600	/* DMA Buffer not yet allocated */
#define DMA_ALOC_ERR	0x0601	/* DMA Buffer allocation failed */
#define DMA_MAL_ERR	0x0602	/* Temporary malloc to clean DMA failed */
#define DMA_RD_ERR	0x0603	/* DMA Buffer read error */
#define DMA_WR_ERR	0x0613	/* DMA Buffer write error */
#define DMA_FREE_ERR	0x0604	/* DMA Buffer free error */
#define DMA_UNK_ERR	0x0605	/* Illegal DMA operation error */

#define DMFIL_C_ERR	0x0710	/* Disable match filter command write failed */
#define DMFIL_P_ERR	0x0711	/* Disable match filter parameter write failed */
#define DMFIL_EX_ERR	0x0712	/* Disable match filter execute failed */
#define DMFIL_NOT_ERR	0x0713 /* Disable match filter cmd not read by adapter*/

#define EXCMD_WR_ERR	0x0810	/* Execute command write to command register failed */
#define EXCMD_TIME_ERR	0x0811	/* Execute command timeout */

#define START_ERR	0x0910	/* START failed */
#define START_STAT_ERR	0x0911	/* START status completion error */
#define START_TIME_ERR	0x0912	/* START status timed out - no completion */
#define START_BSTAT_ERR	0x0914	/* Bad START completion received */
#define HALT_ERR	0x0918	/* HALT failed */

#define SFIL_C_ERR	0x0a10	/* Match filter command write failed */
#define SFIL_P_ERR	0x0a11	/* Match filter parameter write failed */
#define SFIL_EX_ERR	0x0a12	/* Match filter execute failed */
#define SFIL_NOT_ERR	0x0a13	/* Match filter cmd not read by adapter*/

#define IND_C_ERR	0x0b10	/* Indication enable command write failed */
#define IND_P_ERR	0x0b11	/* Indication enable parameter write failed */
#define IND_EX_ERR	0x0b12	/* Indication enable execute failed */
#define IND_NOT_ERR	0x0b13	/* Indication enable cmd not read by adapter*/

#define LOOP_C_ERR	0x0c10	/* Loopback command write failed */
#define LOOP_P_ERR	0x0c11	/* Loopback command parameter write failed */
#define LOOP_EX_ERR	0x0c12	/* Loopback command execute failed */
#define LOOP_NOT_ERR	0x0c13	/* Loopback cmd not read by adapter */

#define SNET_C_ERR	0x0d10	/* Network address command write failed */
#define SNET_P_ERR	0x0d11	/* Network address cmd parameter write failed */
#define SNET_EX_ERR	0x0d12	/* Network address command execute failed */
#define SNET_NOT_ERR	0x0d13	/* Network address cmd not read by adapter*/

#define HARD_RES_ERR	0x0e10	/* Hard Reset Error - status remained at 0xff */
#define HARD_RES1_ERR	0x0e11	/* Hard Reset Error - unknown status */
#define CONF_RD_ERR	0x0e12	/* Error reading config. table after reset */
#define ADAP_RES_ERR    0x0e13  /* Unable to reset adapter */

#define BASE_ADD_ERR	0x0f00	/* Illegal value at subaddress x102 for mem base */
#define BASE_ADD2_ERR	0x0f01	/* Illegal value at subaddress x103 for mem base */

#define REG_WRAP_ERR	0x1000	/* Register Wrap command failed */
#define REG_WRAP2_ERR	0x1001	/* Register Wrap command error */
#define RSTA_TIME_ERR	0x1002	/* Register Wrap status time out */
#define REG_NOT_ERR	0x1003	/* Register Wrap cmd not read by adapter */

#define WRAP_CMP_ERR	0x1100	/* Packet data did not compare */

#define DIX_ON_BNC	0x1111	/* Dix connector on BNC only test */

#define VPD_HDR_ERR	0x1200	/* VPD header data VPD invalid */
#define VPD_CRC_ERR	0x1201	/* VPD CRC's do not match */
#define VPD_ROS_ERR	0x1202	/* VPD ROS level does not match */
#define ROS_NF_ERR	0x1203	/* VPD ROS not found */
#define ROS_LEN_ERR	0x1204	/* VPD ROS maximum length exceeded */

#define SPAU_C_ERR	0x1410	/* Pause command write failed */
#define SPAU_P_ERR	0x1411	/* Pause parameter write failed */
#define SPAU_EX_ERR	0x1412	/* Pause execute failed */
#define SPAU_NOT_ERR	0x1413	/* Pause cmd not read by adapter */
#define SPAU_RD_ERR	0x1414	/* Read of parm0 after execute failed */
#define SPAU_FF_ERR	0x1415	/* Adapt never wrote 0xffff to parm */

#define SRES_P_ERR	0x1511	/* resume parameter write failed */

#define ATYPE_PAR_ERR    0x1600	/* Adapter type doesn't support parity */
#define ADAP_PAR_ERR     0x1601 /* Micro Channel exception or parity error */

#define TRBUF_MAL_ERR   0x1700	/* Memory alloc failed for packet buffer */

#define PART_NO_ERR	0x1800	/* Unrecognizable part number on adapter */
#define PART_NO2_ERR	0x1805	/* Couldn't find part number in VPD */

#define ILLEGAL_TU_ERR	0x9999	/* Illegal TU value - internal error */
#define BAD_DMA_S_ERR	0x999a	/* Bad DMA Buf size - internal error */
#define BAD_PCK_S_ERR	0x999b	/* Bad Packet size - internal error */

#define DATA7_ERR       0x7777  /* RCV Data transfer to host failed (0x07) */

#define SIG_SAV_ERR     0x6001	/* Error saving user's signals. */
#define SIG_RES_ERR     0x6002	/* Error restoring user's signals. */
#define SIG_OP_ERR      0x6003	/* Unknown signal op err - internal. */
#define SIG_REC_ERR     0x6010	/* Received external generated
						signal/interrupt. */

#define QCMD_EMP_ERR	0xac00	/* Cmd. queue could not be cleared. */
#define QCMD_EMP2_ERR	0xac01	/* Cmd. queue empty reading reset completion.*/
#define QCMD_EMP3_ERR	0xac02	/* Cmd. queue empty reading exec mbox offset.*/

#define QSTA_EMP_ERR	0xab01
#define RXCMD_TIME_ERR	0xab02
#define RX_ABORT_ERR	0xab03
#define RXBD_RESC_ERR	0xab04
#define TXCMD_TIME_ERR	0xab05
#define TX_ABORT_ERR	0xab06
#define TXCMD_TX_ERR	0xab07
#define FIRM_PAR_ERR    0xab08

#define POST_01_ERR	0xdd01	/* POST - Processor instruction failure */
#define POST_02_ERR	0xdd02	/* POST - Processor Data Bus 02 failure */
#define POST_03_ERR	0xdd03	/* POST - Processor Data Bus 03 failure */
#define POST_04_ERR	0xdd04	/* POST - Processor Data Bus 04 failure */
#define POST_05_ERR	0xdd05	/* POST - Adapter Data Bus failure */
#define POST_06_ERR	0xdd06	/* POST - ROM Checksum failure */
#define POST_07_ERR	0xdd07	/* POST - Base RAM failure */
#define POST_08_ERR	0xdd08	/* POST - Extended RAM failure */
#define POST_09_ERR	0xdd09	/* POST - 82586 Internal Loopback failure */
#define POST_0A_ERR	0xdd0a	/* POST - 82586 Init/Config failure */
