/* @(#)39       1.2  src/bos/diag/tu/mps/mps_regs.h, tu_mps, bos41J, 9523A_all 6/5/95 11:24:20 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS Register Definitions Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/


/****************************************************************************/
/*             I/O Register Definitions                                     */
/****************************************************************************/
#define BCtl       0x60    /* Basic Control Register Low Byte               */
#define LISR       0x10    /* Local Interrupt Status Register Low Byte      */
#define LISR_SUM   0x12    /* Local Interrupt Status Register w/SUM LB      */
#define LISR_RUM   0x14    /* Local Interrupt Status Register w/RUM LB      */
#define SISR       0x16    /* System Interrupt Status Register LB           */
#define SISR_SUM   0x18    /* System Interrupt Status Register w/SUM LB     */
#define SISR_RUM   0x1a    /* System Interrupt Status Register w/RUM LB     */
#define SISRM      0x54    /* System Interrupt Status Mask Register LB      */
#define SISRM_SUM  0x56    /* System Interrupt Status Mask Register w/SUM   */
#define SISRM_RUM  0x58    /* System Interrupt Status Mask Register w/RUM   */
#define MISR       0x5a    /* Bus Master Interrupt Status Register          */
#define MISRM_SUM  0x5c    /* Bus Master Interrupt Status Mask Reg. w/SUM   */
#define MISRM_RUM  0x5e    /* Bus Master Interrupt Status Mask Reg. w/RUM   */
#define LAPA       0x62    /* Local Access Port Address Register            */
#define LAPE       0x64    /* Local Access Port Extended Address Register   */
#define LAPD       0x66    /* Local Access Port Data/DInc Register          */
#define LAPDInc    0x68    /* Local Access Port Data/DInc Reg. w/auto-inc   */
#define LAPWWO     0x6a    /* Local Access Port RAM Write Window Open       */
#define LAPWWC     0x6c    /* Local Access Port RAM Write Window Close      */
#define Timer      0x4e    /* Timer Control Register                        */
#define SCBSig     0x04    /* SCB Signalling and Control Port Register      */
#define SCBStat    0x06    /* SCB Status Port Register                      */
#define BMCtl_SUM  0x50    /* Bus Master Control Register  w/SUM            */
#define BMCtl_RUM  0x52    /* Bus Master Control Register  w/RUM            */
#define RxLBDA_LO  0x90    /* Receive Channel Last Buffer Descriptor Add.   */
#define RxLBDA_HI  0x92    /* Receive Channel Last Buffer Descriptor Add.   */
#define RxBDA_LO   0x94    /* Receive Channel Buffer Descriptor Address     */
#define RxBDA_HI   0x96    /* Receive Channel Buffer Descriptor Address     */
#define Tx1LFDA_LO 0xa0    /* Transmit Channel 1 Last Frame Desc. Addr.     */
#define Tx1LFDA_HI 0xa2    /* Transmit Channel 1 Last Frame Desc. Addr.     */
#define Tx1FDA_LO  0xa4    /* Transmit Channel 1 Frame Descriptor Addr.     */
#define Tx1FDA_HI  0xa6    /* Transmit Channel 1 Frame Descriptor Addr.     */
#define Tx1DBA_LO  0xac    /* Transmit Channel 1 Frame Descriptor Addr.     */
#define Tx2LFDA_LO 0xb0    /* Transmit Channel 2 Last Frame Desc. Addr.     */
#define Tx2LFDA_HI 0xb2    /* Transmit Channel 2 Last Frame Desc. Addr.     */
#define Tx2FDA_LO  0xb4    /* Transmit Channel 2 Frame Descriptor Addr.     */
#define Tx2FDA_HI  0xb6    /* Transmit Channel 2 Frame Descriptor Addr.     */
#define Tx2DBA_LO  0xbc    /* Transmit Channel 1 Frame Descriptor Addr.     */

/*----------------------------------------------------------------------------*/
/*               MPS Host to Adapter Commands                                 */
/*----------------------------------------------------------------------------*/
#define OPERATION_SUCCESSFUL      0x00
#define INITIALIZATION_COMPLETE   0x80
#define ACCESS_MPC_REGISTER       0x12
#define CLOSE_ADAPTER             0x04
#define CONFIGURE_BRIDGE_CHANNEL  0x0c
#define CONFIGURE_HP_CHANNEL      0x13
#define FLASH_UPDATE              0x19
#define MODIFY_BRIDGE_PARAMETERS  0x15
#define MODIFY_OPEN_OPTIONS       0x01
#define MODIFY_RECEIVE_OPTION     0x17
#define NO_OPERATION              0x00
#define OPEN_ADAPTER              0x03
#define READ_LOG                  0x08
#define READ_SR_BRIDGE_COUNTERS   0x16
#define RESET_GROUP_ADDRESS       0x02
#define RESET_TARGET_SEGMENT      0x14
#define SET_BRIDGE_PARAMETERS     0x09
#define SET_TARGET_SEGMENT        0x05
#define SET_FUNCTIONAL_ADDRESS    0x07
#define SET_GROUP_ADDRESS         0x06
#define TB_REJECT_ADDRESS         0x18

/*----------------------------------------------------------------------------*/
/*               OPEN Options                                                 */
/*----------------------------------------------------------------------------*/
#define INTERNAL_WRAP             1
#define EXTERNAL_WRAP             2
#define NETWORK_MODE              3

/*----------------------------------------------------------------------------*/
/*               Adapter to Host Commands                                     */
/*----------------------------------------------------------------------------*/
#define RECEIVE_DATA              0x81
#define LAN_STATUS_CHANGE         0x84
#define SEND_FLASH_DATA           0x89

/*----------------------------------------------------------------------------*/
/*                             LAN STATUS CODES                               */
/*----------------------------------------------------------------------------*/
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


/*----------------------------------------------------------------------------*/
/*                 MPS Interrupt Code definitions                             */
/*----------------------------------------------------------------------------*/
/* Mask interrupt register for LISR */
#define SRB_CMD         0x20     /* Command in SRB                            */
#define ASB_RSP         0x10     /* Response in ASB                           */
#define ASB_REQ         0x04     /* ASB free request                          */
#define ARB_FREE        0x02     /* ARB free                                  */
#define TRB_FRAME       0x01     /* Frame in TRB                              */

/* Mask interrupt register for SISR */
#define MISR_INT        0x8000   /* MISR interrupt indicate                   */
#define SCB_STAT        0x4000   /* SCB status port interrupt                 */
#define SCB_CTL         0x2000   /* SCB control port interrupt                */
#define SCB_SIG         0x1000   /* SCB signalling port interrupt             */
#define TIMER_EXP       0x0800   /* Timer expired interrupt                   */
#define LAP_PRTY        0x0400   /* LAP data parity error interrupt bit       */
#define LAP_ACC         0x0200   /* LAP access violation error interrupt      */
#define MC_PRTY         0x0100   /* Micro channel parity error interrupt      */
#define ADAPT_CHK       0x0040   /* Adapter Check interrupt                   */
#define SRB_RSP         0x0020   /* SRB response  interrupt                   */
#define ASB_FREE        0x0010   /* ASB free interrupt                        */
#define ARB_CMD         0x0008   /* ARB command interrupt                     */
#define TRB_RSP         0x0004   /* TRB response interrupt                    */
#define UNKNOWN         0xEEEE   /* Unknown interrupt                         */

#define SISR_MSK        0x87FF   /* System interrupt status mask register     */
#define MISR_MSK        0x7737   /* Master interrupt status mask register     */
#define SOFT_RST_MSK    0x8000   /* Soft reset mask bit                       */
#define LISR_MSK        0xA200   /* Local Interrupt Status Mask               */

#define RECEIVE_MSK     0x0037   /* Receive interrupt mask register           */
#define Rx_NBA          0x0020   /* Receive channel no buffers available      */
#define Rx_MSK          0x0007   /* Receive channel enf of frame              */
#define Rx_EOB          0x0010   /* Receive channel enf of buffer             */
#define Rx_NOSTA        0x0004   /* Receive Channel No Status Posted          */
#define Rx_HALT         0x0002   /* Receive channel halt                      */
#define Rx_EOF          0x0001   /* Receive channel enf of frame              */

#define XMIT_DONE_MSK_2 0x7000   /* Transmit done interrupt mask register     */
#define XMIT_DONE_MSK_1 0x0700   /* Transmit done interrupt mask register     */
#define Tx2_NOST        0x4000   /* Transmit channel 2 No status posted       */
#define Tx2_HALT        0x2000   /* Transmit channel 2 halt interrupt         */
#define Tx2_EOF         0x1000   /* Transmit channel 2 end of frame           */
#define Tx1_NOST        0x4000   /* Transmit channel 1 No status posted       */
#define Tx1_HALT        0x2000   /* Transmit channel 1 halt interrupt         */
#define Tx1_EOF         0x1000   /* Transmit channel 1 end of frame           */

/*----------------------------------------------------------------------------*/
/*                 Initialization Status Codes                                */
/*----------------------------------------------------------------------------*/
#define INIT_SUCCESSFUL           0x0
#define PROCESSOR_INIT_ERR        0x20
#define ROS_TEST_ERR              0x22
#define RAM_TEST_ERR              0x24
#define INSTRUCTION_TEST_ERR      0x26
#define INTERRUPT_TEST_ERR        0x28
#define MEMORY_INTERFACE_ERR      0x2A
#define TOKEN_RING_HANDLER_ERR    0x2C
#define CHANNEL_TEST_ERR          0x2E
#define ADDRESS_MATCH_RAM_ERR     0x30
#define ADDRESS_MATCH_CAM_ERR     0x32
#define ETHERNET_MAC_ERR          0xE0
#define ETHERNET_ENDEC_ERR        0xE2
#define ETHERNET_TRANSCEIVER_ERR  0xE4
#define ETHERNET_HANDLER_ERR      0xEC

