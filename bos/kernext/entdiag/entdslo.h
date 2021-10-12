/* @(#)72 1.13.1.1 10/10/91 14:05:32 */
#ifndef _H_entDSLO
#define _H_entDSLO

/*****************************************************************************/
/*                                                                           */
/* COMPONENT_NAME: sysxent -- Ethernet Communications Code Device Driver     */
/*                                                                           */
/* FUNCTIONS: entdslo.h                                                      */
/*                                                                           */
/* ORIGINS: 27                                                               */
/*                                                                           */
/* (C) COPYRIGHT International Business Machines Corp. 1990                  */
/* All Rights Reserved                                                       */
/* Licensed Materials - Property of IBM                                      */
/*                                                                           */
/* US Government Users Restricted Rights - Use, duplication or               */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.         */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*
 * NAME: entdslo.h
 *
 * FUNCTION: see discussion below
 *
 * NOTES:
 * The include files that make up the communications device drivers which
 * use the ciodd.c common code have the following heirarchy (where XXX is
 * the device-specific name such as tok or ent):
 *    comio.h   -- standard communications i/o subsystem declarations (Vol3Ch5)
 *                 needed by ciodd.c and XXXds.c, also needed by users
 *    XXXuser.h -- device-specific declarations (Vol3Ch6)
 *                 needed by ciodd.c and XXXds.c, also needed by users
 *    XXXddi.h  -- device specific part of DDI (defines used by cioddi.h)
 *    cioddi.h  -- common part of DDI definition that is part of DDS
 *                 needed by ciodd.c, XXXds.c, XXXconf.c, not needed by users
 *    cioddhi.h -- high-level independent common declarations
 *                 needed by ciodd.c, XXXds.c, ciodds.h, not needed by users
 *    XXXdshi.h -- high-level independent device-specific declarations
 *                 needed by ciodds.h, ciodd.c, XXXds.c, not needed by users
 *    ciodds.h  -- common part of DDS which depends on all preceding includes
 *                 needed by ciodd.c and XXXds.c, not needed by users
 *    cioddlo.h -- low-level common declarations that may depend on the DDS
 *                 needed by ciodd.c and XXXds.c, not needed by users
 *    XXXdslo.h -- low-level device-specific declarations
 *                 needed by XXXds.c ONLY (not needed by ciodd.c or by users)
 */

/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/

typedef struct error_log_def {
      struct err_rec0   errhead;          /* from com/inc/sys/err_rec.h      */
      ulong errnum;                       /* Error returned to user          */
      uchar cmd;                          /* Command register if applicable  */
      uchar status;                       /* Status  register if applicable  */
      uchar pos_reg[8];                   /* Adapter POS Registers           */
      uchar ent_addr[ent_NADR_LENGTH];    /* actual net address in use       */
      uchar ent_vpd_addr[ent_NADR_LENGTH]; /* actual net address from VPD    */
      uchar ent_vpd_rosl[ROS_LEVEL_SIZE]; /* actual ROS Level from VPD       */
      ushort ent_vpd_ros_length;          /* Actual Length of VPD entry      */
      ushort  version_num;                /* Firmware Version number         */
      ulong      exec_cmd_in_progres;     /* Command in progress in adapter  */
      START_STATE adpt_start_state;       /* state variable for start up     */
      int   slot;                         /* slot for this adapter           */
      ushort type_field_off;              /* Adapter Type field Displacement */
      ushort net_id_offset;               /* IEEE 802.3 one byte netid offset*/
};

#define DELAYMS(ms) delay ((int)(((ms)*HZ+999)/1000))


/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter Buffer Descriptor Common Defines                  */
/*                                                                           */
/*****************************************************************************/
#define EL_BIT_MASK    (0x40) /* End of list bit mask                        */
#define EOP_BIT_MASK   (0x80) /* End of packet bit mask                      */
#define CMPLT_BIT_MASK (0x80) /* Transfer complete bit mask                  */
#define OK_BIT_MASK    (0x40) /* Transfer completed OK bit mask              */
#define BUSY_BIT_MASK  (0x20) /* Transfer busy bit mask                      */
#define BD_ERR_CD_MASK (0x0F) /* Buffer Descriptor Error Code Mask           */
#define MIN_DES_LIST   (0x0A) /* Minumun Descriptor List                     */
#define BUF_DES_SIZE   (0x0A) /* Bytes in an adapter buffer descriptor       */


/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter Execute Mailbox Common Defines                    */
/*                                                                           */
/*****************************************************************************/
#define ent_VPD_LENGTH   (121)                    /* maximum VPD bytes       */
#if ent_VPD_LENGTH > MAX_VPD_LENGTH
#   error ent_VPD_LENGTH > MAX_VPD_LENGTH
#endif

#define DONE_MSK      (0x80) /* Mailbox command Done mask                    */
#define BUSY_MSK      (0x40) /* Mailbox command Done mask                    */
#define ERROR_MSK     (0x20) /* Mailbox command Done mask                    */

                             /* Execute Mailbox Commands                     */
#define CONFIGURE     (0x00) /* Set Receive Packet Filter          0x0000    */
#define SET_ADDRESS   (0x01) /* Set Individual Address             0x0001    */
#define SET_MULTICAST (0x02) /* Set Multicast address              0x0002    */
#define SET_TYPE      (0x03) /* Set Receive "Type Field" Filter    0x0003    */
#define SET_TYPE_BAD  (0x13) /* Set Receive "Type Field" Bogus ID  0x0003    */
#define SET_TYPE_NULL (0x23) /* Set Receive "Type Field" null entry0x0003    */
#define INDICAT_EN    (0x14) /* Indication Enable                  0x0004    */
#define INDICAT_DS    (0x04) /* Indication Disable                 0x0004    */
#define REPORT_CONFIG (0x06) /* Report Adapter Configuration       0x0006    */
#define CONFIG_LIST   (0x08) /* Configure Lists                    0x0008    */
#define AL_LOC_ON     (0x0D) /* 586-AL-LOC Command                 0x000D    */


/*****************************************************************************/
/*                                                                           */
/*   3COM Ethernet Adapter I/O Register Common Defines                       */
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
/*   3COM Ethernet Adapter POS Register Common Defines                       */
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


/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1   */
/*  is returned (hence all return an int value).  In the cases of         */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

enum pio_func{
	GETC, GETS, GETSR, GETL, GETLR,
	PUTC, PUTS, PUTSR, PUTL, PUTLR
};

#define PIO_GETCX(addr, c)						\
{									\
	int rc;								\
	if (rc = BUS_GETCX(addr, c))					\
		pio_retry(dds_ptr, rc, GETC, addr, (ulong)(c));		\
}
#define PIO_GETSX(addr, s)						\
{									\
	int rc;								\
	if (rc = BUS_GETSX(addr, s))					\
		pio_retry(dds_ptr, rc, GETS, addr, (ulong)(s));		\
}
#define PIO_GETSRX(addr, sr)						\
{									\
	int rc;								\
	if (rc = BUS_GETSRX(addr, sr))					\
		pio_retry(dds_ptr, rc, GETSR, addr, (ulong)(sr));	\
}
#define PIO_GETLX(addr, l)						\
{									\
	int rc;								\
	if (rc = BUS_GETLX(addr, l))					\
		pio_retry(dds_ptr, rc, GETL, addr, (ulong)(l));		\
}
#define PIO_GETLRX(addr, lr)						\
{									\
	int rc;								\
	if (rc = BUS_GETLRX(addr, lr))					\
		pio_retry(dds_ptr, rc, GETLR, addr, (ulong)(lr));	\
}
#define PIO_PUTCX(addr, c)						\
{									\
	int rc;								\
	if (rc = BUS_PUTCX(addr, c))					\
		pio_retry(dds_ptr, rc, PUTC, addr, (ulong)(c));		\
}
#define PIO_PUTSX(addr, s)						\
{									\
	int rc;								\
	if (rc = BUS_PUTSX(addr, s))					\
		pio_retry(dds_ptr, rc, PUTS, addr, (ulong)(s));		\
}
#define PIO_PUTSRX(addr, sr)						\
{									\
	int rc;								\
	if (rc = BUS_PUTSRX(addr, sr))					\
		pio_retry(dds_ptr, rc, PUTSR, addr, (ulong)(sr));	\
}
#define PIO_PUTLX(addr, l)						\
{									\
	int rc;								\
	if (rc = BUS_PUTLX(addr, l))					\
		pio_retry(dds_ptr, rc, PUTL, addr, (ulong)(l));		\
}
#define PIO_PUTLRX(addr, lr)						\
{									\
	int rc;								\
	if (rc = BUS_PUTLRX(addr, lr))					\
		pio_retry(dds_ptr, rc, PUTLR, addr, (ulong)(lr));	\
}

/*
 * For pio's that are not in performance critical paths use these macros
 * that will resove to a routine.
 */
#define PIO_PUTC(a, c) pio_putc(dds_ptr, (char *)(a), (char)(c))
#define PIO_GETC(a) pio_getc(dds_ptr, (char *)(a))
#define PIO_PUTSR(a, s) pio_putsr(dds_ptr, (short *)(a), (short)(s))
#define PIO_GETSR(a) pio_getsr(dds_ptr, (short *)(a))

# define PIO_RETRY_COUNT        3

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  ENT Receive definitions and data structures.                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
                                                /* Control Register:         */
# define EOP                    (0x01 <<  7)    /* End of Packet             */
# define EL                     (0x01 <<  6)    /* End of List               */

                                                /* Status Register:          */
# define COMPLETE               (0x01 <<  7)    /* Command Complete          */
# define OK                     (0x01 <<  6)    /* No errors                 */
# define BUSY                   (0x01 <<  5)    /* Busy                      */
# define ERRCODE_MASK           (0x0F <<  0)    /* Mask for error codes:     */

# define CRC_ERROR              1               /* Recv CRC Error            */
# define FIFO_OVERRUN           2               /* Recv Fifo Overrun         */
# define ALIGN_ERROR            3               /* Recv Alignment Error      */
# define NO_RESOURCES           4               /* Recv No resources error   */
# define TOO_SHORT              5               /* Recv pkt too short        */
# define TOO_LARGE              6               /* Recv pkt too large        */


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*  Macros used by receive code:                                             */
/*                                                                           */
/*      LO_SHORT(x)     Returns the low 16 bits of x.                        */
/*      HI_SHORT(x)     Returns the high 16 bits of x.                       */
/*      LO_BYTE(x)      Returns the low 8 bits of x.                         */
/*      HI_BYTE(x)      Returns the high 8 bits of x.                        */
/*      BYTESWAP(x)     Swaps the high and low bytes of x.                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/

# define LO_SHORT(x)    ((int)(x) & 0xFFFF)             /* Low 16 bits       */
# define HI_SHORT(x)    (((int)(x) >> 16) & 0xFFFF)     /* High 16 bits      */
# define LO_BYTE(x)     ((short)(x) & 0xFF)             /* Low 8 bits        */
# define HI_BYTE(x)     (((short)(x) >> 8) & 0xFF)      /* High 8 bits       */
# define BYTESWAP(x)    (LO_BYTE(x) << 8 | HI_BYTE(x))  /* swap hi/lo bytes  */

# define GET_NETID(frame,offset)                                        \
                *((unsigned short *)((int)frame                         \
                                + offset))

#endif /* ! _H_entDSLO */
