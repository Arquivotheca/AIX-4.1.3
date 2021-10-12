/* @(#)34  1.2  src/bos/kernext/ent/en3com_hw.h, sysxent, bos411, 9428A410j 11/9/93 10:15:00 */
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

#ifndef _H_EN3COM_HW
#define _H_EN3COM_HW

#define EN3COM_VPD_LENGTH	121	/* maximum VPD size      */
#define IOCC_DELAY_REG		0xE0	/* hardware delay register */


/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter Buffer Descriptor Definitions                     */
/*                                                                           */
/*****************************************************************************/
typedef struct {                                /* BUFFER DESCRIPTOR:        */
        unsigned char   status;                 /* Status Byte               */
        unsigned char   control;                /* Control Byte              */
        unsigned short  next;                   /* Link to next pkt buffer   */
        unsigned short  count;                  /* Byte count                */
        unsigned short  buflo;                  /* Low part of buffer addr   */
        unsigned short  bufhi;                  /* High part of buffer addr  */
} BUFDESC;

#define BUF_DES_SIZE	(sizeof(BUFDESC))

/* bit mask for the status field */
#define CMPLT_BIT_MASK (0x80) /* Transfer complete bit mask                  */
#define OK_BIT_MASK    (0x40) /* Transfer completed OK bit mask              */
#define BUSY_BIT_MASK  (0x20) /* Transfer busy bit mask                      */
#define BD_ERR_MASK    (0x0F) /* Buffer Descriptor Error Code Mask           */

/* bit mask for the control field */
#define EOP_BIT_MASK   (0x80) /* End of packet bit mask                      */
#define EL_BIT_MASK    (0x40) /* End of list bit mask                        */


/*                                                                           
 *  receive error error codes.
 */

# define CRC_ERROR              1               /* Recv CRC Error            */
# define FIFO_OVERRUN           2               /* Recv Fifo Overrun         */
# define ALIGN_ERROR            3               /* Recv Alignment Error      */
# define NO_RESOURCES           4               /* Recv No resources error   */
# define TOO_SHORT              5               /* Recv pkt too short        */
# define TOO_LARGE              6               /* Recv pkt too large        */

/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter Execute Mailbox Definitions                       */
/*                                                                           */
/*****************************************************************************/

#define DONE_MSK      (0x8000) /* Mailbox command Done mask                  */
#define BUSY_MSK      (0x4000) /* Mailbox command Done mask                  */
#define ERROR_MSK     (0x2000) /* Mailbox command Done mask                  */

/*
 * 3com exec mailbox commands 
 */
#define CONFIGURE     (0x00) /* Set Receive options 	           0x0000    */
#define SET_ADDRESS   (0x01) /* Set Individual Address             0x0001    */
#define SET_MULTICAST (0x02) /* Set Multicast address              0x0002    */
#define SET_TYPE      (0x03) /* Set Receive "Type Field" Filter    0x0003    */
#define SET_TYPE_BAD  (0x13) /* Set Receive "Type Field" Bogus ID  0x0003    */
#define SET_TYPE_NULL (0x23) /* Set Receive "Type Field" null entry0x0003    */
#define INDICAT_DS    (0x04) /* Indication Disable                 0x0004    */
#define INDICAT_EN    (0x14) /* Indication Enable                  0x0004    */
#define REPORT_CONFIG (0x06) /* Report Adapter Configuration       0x0006    */
#define CONFIG_LIST   (0x08) /* Configure Lists                    0x0008    */
#define AL_LOC_OFF    (0x0D) /* 586-AL-LOC Command                 0x000D    */
#define PAUSE_ADPT    (0xC3) /* Pause adapter 			   0x00C3    */

/*
 * 3com exec mailbox commands flags and options
 */
#define PROMIS_ON     (0x01) /* CONFIGURE - promiscuous on */
#define BC_DIS	      (0x02) /* CONFIGURE - broadcast disable */
#define SAVE_BP       (0x04) /* CONFIGURE - save bad packet */

#define IND_EN	      (0x01) /* INCICAT - enable indication */
#define CONFIG_TBL_SIZE		28	/* # of short in REPORT_CONFIG table */
			
/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter Receive Mailbox Definitions                       */
/*                                                                           */
/*****************************************************************************/

typedef struct {                                /* RECEIVE MAILBOX:          */
        unsigned short  status;                 /* Status field              */
        unsigned short  rv_list;                /* Offset to receive list    */
} RECVMBOX;

/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter I/O Register Definitions                          */
/*                                                                           */
/*****************************************************************************/

/**************************************************************************/
/* Command Register (I/O Base + 0, Read/Write)                            */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* Overflow|execute | Receive Commands/Status  | Transmit Commands/Status */
/**************************************************************************/

#define COMMAND_REG   (0x0)  /* Command      Register I/O Base offset */
#define OFLOW_MSK     (0x80) /* Over Flow  mask for Command Register  */
#define EXECUTE_MSK   (0x40) /* Execute    mask for Command Register  */
#define SELF_TESTS_OK (0x00) /* Adapter Self Test Completed OK, mask  */

#define RECEIVE_MSK  (0x38) /* Receive    mask for Command Register   */
#define RX_NOP       (0x00) /* Receive - No Operation Mask            */
#define RX_START     (0x08) /* Receive - Start Packet Reception Mask  */
#define RX_P_RCVD    (0x10) /* Receive - Packet Received Mask         */
#define RX_ABORT     (0x20) /* Receive - Abort Reception Mask         */
#define RX_RESTART   (0x28) /* Receive - Restart Reception Mask       */
#define RX_ERROR     (0x30) /* Receive - Out of Receive Resources Mask*/

#define XMIT_MSK     (0x07) /* Transmit   mask for Command Register   */
#define TX_NOP       (0x00) /* Transmit - No Operation Mask           */
#define TX_START     (0x01) /* Transmit - Execute list Mask           */
#define TX_P_SENT    (0x02) /* Transmit - Single Packet xmitted Mask  */
#define TX_ABORT     (0x04) /* Transmit - Abort Transmission Mask     */
#define TX_RESTART   (0x05) /* Transmit - Restart Transmission Mask   */
#define TX_ERROR     (0x06) /* Transmit - Transmission Error Mask     */
#define TE_OK        (0x00) /* Transmit Error - No error              */
#define TE_COLLISION (0x01) /* Transmit Error - Max Collisions Mask   */
#define TE_UNDERRUN  (0x02) /* Transmit Error - FIFO Underrun  Mask   */
#define TE_CARRIER   (0x03) /* Transmit Error - Carrier Sense Lost    */
#define TE_CTS_LOST  (0x04) /* Transmit Error - Clear to Send Lost    */
#define TE_TIMEOUT   (0x05) /* Transmit Error - Transmit Timeout Mask */
#define TE_SIZE      (0x06) /* Transmit Error - Bad Packet Size Mask  */

/**************************************************************************/
/* Host Status Register (I/O Base + 2, RO except for parity)              */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +Parity |  +CRR  |  +CWR  |   *    |    *   |    *   |    *   |    *   */
/**************************************************************************/

#define STATUS_REG   (0x2)  /* Host Status   Register I/O Base offset */
#define PARITY_MSK   (0x80) /* Parity Bit mask for Status Register    */
#define CRR_MSK      (0x40) /* CMD Reg Ready mask for Status Register */
#define CWR_MSK      (0x20) /* CMD Byte Recd mask for Status Register */
#define INTR_MSK     (0xA0) /* Interrupt Recd mask for Status Register*/

/**************************************************************************/
/* Host Control Register (I/O Base + 6, Read/Write)                       */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +ATTN   | +RESET |    *   |   *    |    *   |+CMDINTE|    *   |    *   */
/**************************************************************************/
#define CONTROL_REG  (0x6)  /* Host Control  Register I/O Base offset */
#define HARD_RST_MSK (0xC0) /* Hard Reset   mask for Control Register */
#define EN_INTR_MSK  (0x04) /* Enable Interrupt mask for Control Reg  */
#define CONTROL_VALUE (0x00) /* Host Control Base Value               */

/**************************************************************************/
/* RAM Page Register (I/O Base + 8, Read/Write)                           */
/**************************************************************************/
#define RAM_PAGE_REG (0x8)  /* RAM Page      Register I/O Base offset */
#define RAM_PAGE_VALUE (0x00) /* RAM Page Register Default value      */

/**************************************************************************/
/* Parity Control Register (I/O Base + A, Read/Write) (Version 1 & 2)     */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +RAMERR |   *    |    *   |   *    |    *   |+DATAERR|+ADDRERR| +PAREN */
/**************************************************************************/
/**************************************************************************/
/* Parity Control Register (I/O Base + A, Read/Write) (Version 3 card)    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +RAMERR |   *    |    *   |   *    |+ MCHCK |+DATAERR|+FBRTERR| +PAREN */
/**************************************************************************/
#define PARITY_REG    (0x0A)   /* Parity Enable Register I/O Base offset  */
#define ANY_ERR_MSK   (0x86)   /* Any Parity error Mask for Parity Reg    */
#define RAM_ERR_MSK   (0x80)   /* RAM Parity error Mask for Parity Reg    */
#define DATA_ERR_MSK  (0x04)   /* Data Parity error Mask for Parity Reg   */
#define ADDR_ERR_MSK  (0x02)   /* Address Parity error Msk in Parity Reg  */
#define PAREN_MSK     (0x01)   /* Parity Enable Mask  in the  Parity Reg  */
#define PARDS_MSK     (0x00)   /* Parity Disable Mask in the  Parity Reg  */

/* Parity Control Register (I/O Base + A, Read/Write) (Version 3 card)    */
#define ANY_ERR3_MSK  (0x8E)   /* Any Parity error Mask for Parity Reg    */
#define MCHK_ERR3_MSK (0x08)   /* Channel Check Mask in the Parity Reg    */
#define FBRT_ERR3_MSK (0x02)   /* Feedback Return error Msk in Parity Reg */


/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter POS Register Definitions                          */
/*                                                                           */
/*****************************************************************************/

/**************************************************************************/
/* POS Register 0                                                         */
/**************************************************************************/
#define POS_REG_0     (0x0)     /* POS Register 0 Base offset             */
#define PR0_VALUE     (0xF5)    /* POS Register 0 Adapter ID Code  Hex F5 */

/**************************************************************************/
/* POS Register 1                                                         */
/**************************************************************************/
#define POS_REG_1     (0x1)     /* POS Register 1 Base offset             */
#define PR1_VALUE     (0x8E)    /* POS Register 1 Adapter ID Code  Hex 8E */

/**************************************************************************/
/* POS Register 2                                                         */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* +PAREN  | Memory Base Address      |     I/O Base Address     | +CDEN  */
/**************************************************************************/
#define POS_REG_2     (0x02)    /* POS Register 2 Base offset             */
#define PR2_PEN_MSK   (0x80)    /* POS Register 2 POS Parity Enable Mask  */
#define PR2_RAM_MSK   (0x70)    /* POS Register 2 RAM Base Mask           */
#define PR2_IO_MSK    (0x0E)    /* POS Register 2 I/O Base Mask           */
#define PR2_CDEN_MSK  (0x01)    /* POS Register 2 Card Enable Mask        */

/**************************************************************************/
/* POS Register 3                                                         */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/*          reserved         | +Fair  |          DMA Level                */
/**************************************************************************/
#define POS_REG_3     (0x3)     /* POS Register 3 Base offset             */
#define PR3_DMA_MSK   (0x0F)    /* POS Register 3 DMA Level Mask          */
#define PR3_FAIR_MSK  (0x10)    /* POS Register 3 Fairness Mask           */

/**************************************************************************/
/* POS Register 4  (Version 1 & 2 cards only)                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/*    *    |+STARTEN|    *   |   *    |    *   |    *   |  +BNC  | +WDOEN */
/**************************************************************************/
/**************************************************************************/
/* POS Register 4  (Version 3 card)                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/*+FDBKINTN|+STARTEN|    *   |+ADPAREN|    *   |    *   |  +BNC  | +WDOEN */
/**************************************************************************/
#define POS_REG_4     (0x04)  /* POS Register 4 Base offset               */
#define PR4_STRT_MSK  (0x40)  /* POS Register 4 Start ROM Enable Mask     */
#define PR4_BNC_MSK   (0x02)  /* POS Register 4 BNC Transceiver Select MSK*/
#define PR4_WDEN_MSK  (0x01)  /* POS Register 4 Window Enable Mask        */

/* POS Register 4  (Version 3 card)                                       */
#define PR4_FDBK_MSK  (0x80)  /* POS Reg 4 - Feedback interrupt enable msk*/
#define PR4_PAREN_MSK (0x10)  /* POS Reg 4 - Adapter Parity Enable Mask   */

/**************************************************************************/
/* POS Register 5                                                         */
/**************************************************************************/
/*    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   */
/* -CHCK   |   1    |   Window Size   | Interrupt Level |  Addr Burst Mgm */
/**************************************************************************/
#define POS_REG_5     (0x5)     /* POS Register 5 Base offset             */
#define PR5_CHCK_MSK  (0x80)    /* POS Register 5 Channel Check Mask      */
#define PR5_BIT_6     (0x40)    /* POS Register 5 bit 6 is always 1       */
#define PR5_WDSZ_MSK  (0x30)    /* POS Register 5 Window Size Mask        */
#define PR5_INTR_MSK  (0x0C)    /* POS Register 5 Interupt Channel Mask   */
#define PR5_ABM_MSK   (0x03)    /* POS Register 5 ABM Mode Mask           */

/**************************************************************************/
/* POS Register 6                                                         */
/**************************************************************************/
#define POS_REG_6     (0x6)     /* POS Register 6 Base offset             */
#define PR6_VALUE     (0x00)    /* POS Register 6 default value           */

/**************************************************************************/
/* POS Register 7                                                         */
/**************************************************************************/
#define POS_REG_7     (0x7)     /* POS Register 7 Base offset             */
#define PR7_VALUE     (0x00)    /* POS Register 7 default value           */

#define NUM_POS_REG	POS_REG_7 + 1	/* total number of POS registers */

/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter Statistics Counters offsets			     */
/*                                                                           */
/*****************************************************************************/
#define RV_CRC		0	/* receive CRC error */
#define RV_ALIGN	0x4	/* receive alignment error */
#define RV_OVERRUN	0x8	/* receive overrun error */
#define RV_SHORT	0xC	/* receive short frame error */
#define RV_LONG		0x10	/* receive long frame error */
#define RV_RSC		0x14	/* receive resource error */
#define RV_DISCARD	0x18	/* receive - packets discarded */
#define TX_MAX_COLL	0x1C	/* transmit maximum collision error */
#define TX_NO_CS	0x20	/* transmit no carrier sense error */
#define TX_UNDERRUN	0x24	/* transmit underrun error */
#define TX_CLS		0x28	/* transmit no clear-to-send error */
#define TX_TMOUT	0x2C	/* transmit timeout error */
#define ADPT_RV_EL	0x30	/* times EL bit seen in receive list */		
#define ADPT_RSC_586	0x34	/* no resource error from 82586 */
#define ADPT_RVPKTS_OK	0x38	/* good packets received */
#define ADPT_RVPKTS_UP	0x3C	/* received packets uploaded */
#define ADPT_RV_START	0x40	/* times of "start reception" command */
#define ADPT_DMA_TMOUT	0x44	/* bus master trasnfer timeout */
#define ADPT_3COM_STAT	0x48	/* five 16 bit 3com stat variable */
#define ADPT_TXPKTS_DN	0x56	/* packets downloaded */
#define ADPT_TX_ERR	0x5E	/* transmit errors */
#define ADPT_TX_RETRY	0x6E	/* number of transmit retries */
#define ADPT_TX_FAILED	0x72	/* number of transmit failed */
#define TX_ONE_COLL	0x7A	/* number of packets with single collision */
#define TX_MULTI_COLL	0x7E	/* number of packets with multiple collision */
#define LAST_STATS	0x84	/* the last stats counter on the adapter */

#define EN3COM_USE	5	/* no. of 16 bit stats for 3com internal use */

#endif /* _H_EN3COM_HW */
