/* @(#)40  1.14  src/bos/kernext/ient/i_enthw.h, sysxient, bos41B, 412_41B_sync 12/8/94 14:24:31 */
/****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS: BUSGETC
 *              BUSPUTC
 *              BUSPUTL
 *              CHANNEL_ATTENTION
 *              COMMAND_QUIESCE
 *              DELAYMS
 *              DISABLE_DEVICE
 *              GET_NETID
 *              IOADDRTOSM
 *              IOCC_HANDLE
 *              NEXT_CLINE
 *              READ_CHAR
 *              READ_LONG
 *              READ_LONG_REV
 *              READ_SHORT
 *              READ_SHORT_REV
 *              SMTOIOADDR
 *              START_CU
 *              SUSPEND_RU
 *              WRITE_CHAR
 *              WRITE_LONG
 *              WRITE_LONG_REV
 *              WRITE_SHORT
 *              WRITE_SHORT_REV
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
#ifndef _H_IENT_HW
#define _H_IENT_HW

/*
**                      82596 Ethernet data structures
**
**      The following structures make up the shared memory
**      structure that the 82596DX and the CPU use to communicate
**      with one another. The 82596DX is configured for the linear mode
**      using the Little Endian format.
*/



/*
**      System Configuration Pointer
*/
struct scp
{
    ulong   sysbus;                 /* configuration parameters  */
    ulong   reserved;               /* reserved */
    ulong   iscp_addr;              /* address of the ISCP */
};


/*
**      Intermediate System Configuration Pointer
*/
struct iscp
{
    ulong   busy;                   /* 82596 being initialized */
    ulong   scb_addr;               /* address of the SCB */
};


/*
**      System Control Block
*/
struct scb
{
    ushort   status;                /* status word */
    volatile ushort   command;      /* command word */
    ulong    cbl_addr;              /* address of command block */
    ulong    rfa_addr;              /* address of the receive frame area */
    ulong    crc_errs;              /* count of CRC errors */
    ulong    align_errs;            /* count of alignment errors */
    ulong    resource_errs;         /* no resources available errors */
    ulong    overrun_errs;          /* count of overrun errors */
    ulong    rcvcdt_errs;           /* collisions detected */
    ulong    frame_errs;            /* short frame errors */
    ushort   off_timer;             /* t-off timer */
    ushort   on_timer;              /* t_on timer */
};

/*
**      Byte addressing between the 82596 and system memory.
**
**      The 82596 will operate in Little Endian mode ( ie. low
**      address contains the least significant byte. ).
**      The byte reversal performed by the IOCC will convert all DMA and PIO
**      operations of 16-bits or greater into Little Endian format
**      when writing out to the bus
**      1-byte PIO's or string operations are not reversed
**
**      -------------------------------------------
**      |ACK 0|CUC R|RUC 0000|STAT|0 CUS|RUS|T 000|     IBM
**      |___ _ ___ _ ___ ____|____ _ ___ ___ _ ___|
*/



/*
**   The following defines refer to the System Control Block (SCB).
*/

/*
** Command word defines:
**   The ACK field. This field specifies the action to be
**   be performed as a result of a CA ( Channel Attention ). This
**   field is set by the CPU and cleared by the 82596.
*/
#define CMD_ACK_CX              0X0080         /* CU completed action cmd */
#define CMD_ACK_FR              0X0040         /* RU received a frame */
#define CMD_ACK_CNA             0X0020         /* CU became not active */
#define CMD_ACK_RNR             0X0010         /* RU became not ready */
#define CMD_ACK_MASK            0x00F0         /* ACK field mask */

/*
**  The command unit field. This field contains the command to the
**  command unit.
*/
#define CMD_CUC_NOP             0x0000         /* NOP */
#define CMD_CUC_START           0x0001         /* start execution of 1st cmd */
#define CMD_CUC_RES             0x0002         /* resume operation of CU */
#define CMD_CUC_SUSPEND         0x0003         /* suspend execution of cmds */
#define CMD_CUC_ABORT           0x0004         /* abort current command */
#define CMD_CUC_TIMER_LOAD      0x0005         /* loads bus throttle timer */
#define CMD_CUC_TIMER_START     0x0006         /* loads & starts bus timer */

/*
**  The receive unit field. This field contains the command to the
**  receive unit.
*/
#define CMD_RUC_NOP             0x0000         /* NOP */
#define CMD_RUC_START           0x1000         /* start reception of frames */
#define CMD_RUC_RES             0x2000         /* resume reception of frames */
#define CMD_RUC_SUSPEND         0x3000         /* suspend frame reception */
#define CMD_RUC_ABORT           0x4000         /* abort receiver operation */

/*
**  Sofware reset.  Resets the 82596 (logically the same as hardware RESET).
*/
#define CMD_RESET               0x8000         /* Soft Reset */

/*
**   Status word defines.
**   The STAT field. This field indicates the status of the
**   82596. This word is modified only by the 82596.
*/
#define STAT_CX                 0x0080         /* CU executed cmd,I bit set */
#define STAT_FR                 0x0040         /* RU finished recv frame */
#define STAT_CNA                0x0020         /* CU left active state */
#define STAT_RNR                0x0010         /* RU left ready state */

/*
**  The command unit status field. This field contains the status of the
**  command unit.
*/
#define STAT_CUS_IDLE           0x0000         /* idle */
#define STAT_CUS_SUSPEND        0x0001         /* suspended */
#define STAT_CUS_ACTIVE         0x0002         /* active */
#define STAT_CUS_TIMEOUT        0x0004         /* Our timeout condition */

/*
**  The receive unit status field. This field contains the status of the
**  receive unit.
*/
#define STAT_RUS_IDLE           0x0000         /* idle */
#define STAT_RUS_SUSPEND        0x1000         /* suspended */
#define STAT_RUS_NO_RESOURCE    0x2000         /* no resources */
#define STAT_RUS_READY          0x4000         /* ready */
#define STAT_RUS_NO_RSC_RBD     0xA000         /* no RBD's -> no resources */
#define STAT_RUS_NO_RBD         0xC000         /* no more RBD's */

/*
**  Ethernet registers on the native planar
*/
#define ENT_MEMBASED_CA_REG     0x4     /* Channel attention offset.         */
#define ENT_MEMBASED_STATUS_REG 0x8     /* Status register offset.           */
#define ENT_PORT_IO_BASED       0x115F  /* IObased ethernet port reg address */
#define ENT_CA_IO_BASED         0x115E  /* IObased ethernet CA register      */
#define ENT_STATUS_IO_BASED     0x115C  /* IObased ethernet status register  */
#define INTERRUPT               0x01    /* ether chip int bit in status reg  */
#define ABORTED_CYCLE           0x10    /* Status Reg. aborted cycle         */
#define RESET_STATUS_REG        0x00000001      /* Status Reg. reset value   */
#define CHAN_CHECK              0x80    /* micro channel error               */
#define MICRO_CHAN_ERR          0xf0    /* leon/chaps,channel check,parity,. */
#define CHK_OR_NOT_INTR         0x07    /* intr, chan chk, or any error 2-6  */
#define ANY_MC_CHECK            0x02    /* any uchan error bit 2-6 active    */
#define RAM_ADDR_LOW            0x1158  /* ASIC RAM address low byte         */
#define RAM_ADDR_HIGH           0x1159  /* ASIC RAM address high byte        */
#define PORT_DATA_LOW           0x115A  /* port command data low byte        */
#define PORT_DATA_HIGH          0x115B  /* port command data high byte       */
#define PORT_CONTROL            0x115F  /* port command control register     */

/*****************************************************************************/

/*
**      Structures and defines for all of the action commands.
*/


/*
**  Bit defines for the command block words.
*/
#define CB_COMP                 0x00a00000      /* CU complete and OK        */
#define CB_NC                   0x00001000      /* CRC insertion for xmit    */
#define CB_SF                   0x00000800      /* operating mode for xmit   */
#define CB_C                    0x00800000      /* execution complete        */
#define CB_B                    0x00400000      /* 82596 currently executing */
#define CB_OK                   0x00200000      /* cmd executed without error*/
#define CB_ABORT                0x00100000      /* cmd abnormally terminated */
#define CB_EL                   0x00000080      /* last command blk om CBL   */
#define CB_SUS                  0x00000040      /* suspend CU on completion  */
#define CB_INT                  0x00000020      /* interrupt after execution */
#define CB_STATUS_MASK          0x001f0000      /* mask to get statis from CB*/

#define CMD_QUEUED              0x00008000

/*
** The csc viewed as a ulong and as two ushorts.
*/
typedef union {
    ulong _csc;
    struct {
        ushort  _csc_cb_short;  /* contains c and b bits */
        ushort  _csc_el_short;  /* contains el bit */
    } s;
} csc_t;

#define csc             c._csc
#define csc_cb_short    c.s._csc_cb_short
#define csc_el_short    c.s._csc_el_short

/*****************************************************************************/
/*
**      The NOP command.
*/
struct nop {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block */
};

/*
**  NOP command block defines.
*/
#define NOP_CMD                 0x00000000      /* the NOP command */

/*****************************************************************************/

/*
**      The Individual Address Setup command.
*/
volatile struct ias {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block */
    uchar iaddr[6];                         /* individual address */
};

/*
**  Individual Address Setup command block defines.
*/
#define IAS_CMD                 0x00000100      /* the IAS command */

/*****************************************************************************/

/*
**      The Configure command.
*/
volatile struct cfg {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block */
    uchar byte_count;                       /* # of CB bytes to configure*/
    uchar fifo_limit;                       /* pt in FIFO to request bus */
    uchar save_bf;                          /* save bad frames */
    uchar loopback;                         /* loopback and addr length */
    uchar linear_pri;                       /* linear priority */
    uchar spacing;                          /* interframe spacing */
    uchar slot_time_low;                    /* slot time for the network */
    uchar slot_time_up;                     /* upper nibble of slot time */
    uchar promiscuous;                      /* accept all frames */
    uchar carrier_sense;                    /* carrier sense filter */
    uchar frame_len;                        /* minimum frame length */
    uchar preamble;                         /* preamble until carrier sns*/
    uchar dcr_slot;                         /* DCR slot number */
    uchar dcr_num;                          /* # of stations in DCR mode */
};

/*
**  Configure command block defines.
*/
#define CFG_CMD                 0x00000200      /* the configure command */
/* The internal cfg command is used internally by the driver. */
#define INTERNAL_CFG_CMD        0x00000201      /* Should not equal any */
                                                /* other command. */

/*
**  Initial configuration values
*/
#define CFG_BYTE_COUNT          0x0E    /* 14 bytes in this cmd, prefetch off*/
#define CFG_FIFO_LIMIT          0xCF    /* fifo = F, monitor mode disabled */
#define CFG_BAD_FRAMES          0x40    /* do not save bad frames */
#define CFG_LOOPBACK            0x2E    /* loopbk off, addr len, preamble len*/
#define CFG_LINEAR              0x00    /* linear and exponential priority */
#define CFG_SPACING             0x60    /* interframe spacing, 96 xmit clocks*/
#define CFG_SLOT_LOW            0x00    /* lower byte of slot time */
#define CFG_SLOT_HIGH           0xF2    /* max retry and upper nibble of slot*/
#define CFG_PROMISCUOUS         0x00    /* promiscuous mod not supported */
#define CFG_CSF                 0x00    /* external carrier sense */
#define CFG_FRAME_LEN           0x40    /* Default min. frame length. */
#define CFG_PREAMBLE            0xFF    /* A number of different bits. */
#define CFG_RESERVED            0x00    /* all other fields are reserved */

#define MC_ALL_BIT              0x20    /* for switching all multicast */

/*
**      The Multicast-Setup command.
*/
volatile struct mc {
    csc_t c;                                /* cmd,status,control fields  */
    ulong next_cb;                          /* link to next cmd block     */
    ushort byte_count;                      /* # of CB bytes to configure */
    uchar addr_list[MAX_MULTI * ENT_NADR_LENGTH];  /* list of multicast  */
    /* addresses by byte  */
};

/*
**  Multicast-setup command block defines.
*/
#define MCS_CMD                 0x00000300      /* the mcast-setup command */

/*****************************************************************************/

/*
**      The Transmit command.
*/
volatile struct xmit {
    csc_t  c;                               /* cmd,status,control fields */
    struct xcbl     *next_cb;               /* link to next cmd block */
    ulong  *tbd;                            /* addr of 1st xmit buffer */
    ushort tcb_cnt;                         /* number of bytes to xmit */
    ushort reserved;                        /* set to zeros */
    ulong  daddr_high;                      /* destination address */
    ushort daddr_low;                       /* destination address */
    ushort length;                          /* length of data field */
};

/*
 *  Transmit command block defines.
 */
#define XMIT_CMD                0x00000400  /* the transmit command */
#define XMIT_EOF                0x00800000  /* the transmit command */
#define XMIT_STAT_LC            0x00080000  /* late collision detected */
#define XMIT_STAT_NCS           0x00040000  /* No carrier sense signal */
#define XMIT_STAT_CSS           0x00020000  /* Xmit stop due to no clear */
#define XMIT_STAT_DU            0x00010000  /* to send sig/DMA underrun */
#define XMIT_STAT_TD            0x80000000  /* transmission deferred */
#define XMIT_STAT_HB            0x40000000  /* heartbeat indicator */
#define XMIT_STAT_STOP          0x20000000  /* xmit, too many collisions */
#define XMIT_STAT_MAXCOLL       0x0F000000  /* # of collisions/this frame */
#define XMIT_STAT_MASK          0xA00F0000  /* Xmit status bit mask */
#define STAT_FIELD_MASK         0xFFFF0000  /* mask of status bits */

/*****************************************************************************/

/*
**      The Time Domain Reflectionary (TDR) command.
*/
struct tdr {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block */
    ulong time;                             /* status and elapsed time */
};


/*
**  Dump command block defines.
*/
#define TDR_CMD                 0x00000500      /* the TDR command */

/*****************************************************************************/

/*
**      The Dump command.
*/
struct dump {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block */
    ulong buffer;                           /* addr dump buffer, 304 byte */
};


/*
**  Dump command block defines.
*/
#define DUMP_CMD                0x00000600      /* the dump command */

/*****************************************************************************/

/*
**      The Diagnose command.
*/
struct diag {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block */
};

/*
**  Diagnostic command block defines.
*/
#define DIAG_CMD                0x00000700      /* the diag command */
#define DIAG_FAIL               0x00000800      /* self-test failed */

/*****************************************************************************/


/*
**   Memory layout of Shared Memory Structure ( SMS - ethernet control block ).
**   This contains the ISCP, SCB, CBL ( command block list)
**   The location of the SMS is platform dependent. It may reside either
**   in system memory or in the bus memory space.
*/

/*
         --------------------------------------------------------
       0| SCP                                                    |
        |--------------------------------------------------------|
    0x0C| ISCP                                                   |
        |--------------------------------------------------------|
    0x14| SCB                                                    |
        |                                                        |
        |                                                        |
        |                                                        |
        |--------------------------------------------------------|
    0x3C| XMIT CBL's                                             |
        |                                                        |
        |                                                        |
        |                                                        |
        |                                                        |
        |--------------------------------------------------------|
    0xAC| MISC CBL's                                             |
        |                                                        |
        |                                                        |
        |                                                        |
        |                                                        |
        |                                                        |
         --------------------------------------------------------

******************************************************************************/


#define PACKET_SIZE     2048  /* !!!1536  multiple of cache line, >= 1518 */
#define MAX_XMIT_QUE    32
#define MAX_RECV_QUE    100
#define MAX_GATHER       6
#define ADP_RCV_QSIZE   60

/*********************************************************************/

/*
**      Transmit Queue
**      Note: Will do a software gather so only one
**      TBD and xmit element needed per transmit.
*/

/*
**      The Transmit Buffer Descriptor.
*/
volatile struct tbd {
    ulong   control;                        /* control info, size */
    uchar   *next_tbd;                      /* next xmit buffer descptr */
    ulong   tb_addr;                        /* xmit buffer address  */
};

/*
**      Transmit Queue ( Command Block List )
*/
struct xcbl {
    volatile struct  xmit            xmit;   /* Transmit command block */
};

/*
**      Transmit buffers and buffer descriptors
*/
volatile struct xmit_buffer {
    char    buf[PACKET_SIZE];
};

/****************************************************************************

Uses an array of structures

                ----------
           [0] |          |
               | xmit cmd |
               |          |
                ----------
           [1] |          |
               |          |
               |          |
                ----------
           [2] |          |
               |          |
               |          |
                ----------
           [3] |          |
               |          |
               |          |
                ----------
                    .
                    .
                    .
                    .
******************************************************************************/


/*
**  this define indicates the number of 4k pages
**  that will be allocated as transmit buffers.
**  note: yield two buffers per page
*/
#define XMIT_BUFFERS            8


/******************************************************************************
**
**      The Receive Frame Area
**
******************************************************************************/

#define NBR_DATA_BYTES_IN_RFD 2

/*
**      The Receive Frame Descriptor.
*/
volatile struct rfd {
    csc_t c;                                /* status, control fields */
    ulong next_rfd;                         /* link to next RFD */
    ulong rbd;                              /* addr of recv buffer desc */
    ushort count;                           /* actual count */
    ushort size;                            /* size of buffer */
    /* DATA from now on */
    uchar d_addr[6];                        /* destination address */
    uchar s_addr[6];                        /* source address */
    ushort length;                          /* length of data field */
    uchar data[NBR_DATA_BYTES_IN_RFD];      /* First nbr bytes in frame */
};

/*
**      The Receive Buffer Descriptor.
*/
volatile struct rbd {
    ushort count;                           /* actual count */
    ushort unused;                          /* unused field */
    ulong next_rbd;                         /* link to next RBD */
    ulong rb_addr;                          /* receive buffer address */
    ulong size;                             /* control and length info */
};

/*
**      The Receive Frame Area ( array of structures )
*/
struct recv_buffer {
    char    buf[PACKET_SIZE]; /* receive buffer */
};

/*
**  defines for the status field in the RFD
*/
#define RECV_EOF                0x00C0          /* EOF flag for recv command */
#define RECV_STAT_MASK          0xFFFF0000      /* status bits mask value    */
#define RECV_STAT_ERR_MASK      0xC01F0000      /* status error bits mask    */
#define RECV_COLLISION          0x01000000      /* collision during receive  */
#define RECV_IA_MISMATCH        0x02000000      /* Broadcast or Multicast in */
#define RECV_SBF_TRUNC          0x20000000      /* Save Bad Frames? Trunc'd  */
#define RECV_EOP                0x40000000      /* no EOP flag, bit stuffing */
#define RECV_FRAME_SHORT        0x80000000      /* Frame to short            */
#define RECV_DMA_OVERRUN        0x00010000      /* DMA overrun failure       */
#define RECV_NO_RESOURCE        0x00020000      /* ran out of buffer space   */
#define RECV_ALIGN              0x00040000      /* alignment error           */
#define RECV_CRC                0x00080000      /* CRC error                 */
#define RECV_LENGTH             0x00100000      /* length error              */

/*
**  defines for the receive buffer descriptor
*/
#define RBD_EOF         0x80            /* last buffer for this frame */
#define RBD_F           0x40            /* buffer used */
#define RBD_EL          0x00008000      /* Size field */

/*****************************************************************************/

/*
**      The Action Command Queue
*/
volatile struct acbl {
    csc_t c;                                /* cmd,status,control fields */
    ulong next_cb;                          /* link to next cmd block    */
    uchar cmd_data[128];                    /* generic area used for     */
                                            /* all action commands       */
                                            /* ie. for MC setup command, */
                                            /* used for MC addresses     */
};



/*
**      Ethernet Adapter POS Register Defines
*/

#define POS_REG_OFFSET   0x00400000     /* Offset to MicroChannel POS regs. */

/*
**      POS Register 0   (ethernet POS ID register )
*/
#define POS_0               0           /* POS Register 0 Base offset */
#define PID_LSB_MEM_BASED   0xF3        /* Membased ethernet POS ID LSB 0xF3 */
#define PID_LSB_IO_BASED    0xF2        /* IObased ethernet POS ID LSB 0xF2  */
#define PID_LSB_RAINBOW     0x98        /* Membased ethernet POS ID LSD 0x98 */

/*
**      POS Register 1   (ethernet POS ID register )
*/
#define POS_1               1           /* POS register 1 base offset */
#define PID_MSB             0x8E        /* ethernet POS ID MSB 0x8E */
#define PID_MSB_RAINBOW     0x8F        /* ethernet POS ID MSB 0x8F (rainbow)*/


/*
**      POS Register 2   (ethernet control register )
*/

/*
** Note:  bit numbering is MCA bit numbering
**
** membased format:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
** -IRQ 5  |              RESERVED             |  +CSF  |+PARITY | +CARD  *
** +IRQ 7  |                                   | ENABLE | ENABLE | ENABLE *
***************************************************************************
**
** IObased format:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
**     INTERRUPT    |  +INT  |  +CSF  |  +i596 | +A PAR | +D PAR | +CARD  *
**    LEVEL SELECT  | ENABLE | ENABLE | ENABLE | ENABLE | ENABLE | ENABLE *
***************************************************************************
*/
#define POS_2                   2      /* POS register 2 base offset         */
#define POS2_CARD_DISABLE       0x00   /* ethernet card enable               */
#define POS2_CARD_ENABLE        0x01   /* ethernet card enable               */
#define POS2_PARITY_MEM_BASED   0x02   /* membased enable ethernet parity    */
#define POS2_PARITY_IO_BASED    0x06   /* IObased enable ethernet parity     */
#define POS2_82596_IO_BASED     0x08   /* IObased enable 82596 access        */
#define POS2_CSF_MEM_BASED      0x04   /* membased enable card select feedbk */
#define POS2_CSF_IO_BASED       0x10   /* IObased enable card select feedbk  */
#define POS2_IRQMASK_MEM_BASED  0x80   /* membased interrupt level mask      */
#define POS2_IRQMASK_IO_BASED   0xC0   /* IObased interrupt level mask       */
#define POS2_INT_ENABLE         0x20   /* enable chip interrupts             */
#define POS2_IRQ5               0x00   /* membased interrupt level 5         */
#define POS2_IRQ7               0x80   /* membased interrupt level 7         */
#define POS2_IRQ9               0x00   /* IObased interrup level 9           */
#define POS2_IRQ10              0x80   /* IObased interrup level 10          */
#define POS2_IRQ11              0x40   /* IObased interrup level 11          */
#define POS2_IRQ12              0xC0   /* IObased interrup level 12          */


/*
**      POS Register 3   ( VPD data register )
**
** membased:
**      When the VPD address register at POS 6 contains a non-zero
**      value, a read of the POS 3 will return a VPD data stored in
**      the IPL ROM VPD area at the offset which was pre-written
**      to POS 6.
**
** IObased:
**      not used
*/

#define POS_3           3              /* POS register 3 base offset */


/*
**      POS Register 4   ( ethernet DMA control register )
**
** Note:  bit numbering is MCA bit numbering
**
** membased:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
**          RESERVED         |+ENABLE |        ARBITRATION LEVEL          *
**                           |FAIRNESS|                                   *
***************************************************************************
**
** membased (Rainbow):
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
**      RESERVED    |-7.5uS  |+ENABLE |        ARBITRATION LEVEL          *
**                  |TIMEOUT |FAIRNESS|                                   *
***************************************************************************
**
** IObased:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
**        ARBITRATION LEVEL           |+ENABLE |         RESERVED         *
**                                    |FAIRNESS|                          *
***************************************************************************
*/
#define POS_4                   4     /* POS register 4 base offset */
#define POS4_FAIR_MEM_BASED     0x10  /* membased enable Ethernet fairness */
#define POS4_FAIR_IO_BASED      0x08  /* IObased enable Ethernet fairness  */
#define POS4_ARBLVL_MEM_BASED   0x0F  /* membased mask for DMA arbitration */
#define POS4_ARBLVL_IO_BASED    0xF0  /* IObased mask for DMA arbitration  */
#define POS4_ARBSHIFT_IO_BASED  4     /* bit shift for Stilwell arbitration*/


/*
**      POS Register 5   ( ethernet status register )
**
** Note:  bit numbering is MCA bit numbering
**
** membased:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
** CHANNEL |                            RESERVED                          *
** CHECK   |                                                              *
***************************************************************************
**
** membased (Rainbow):
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
** CHANNEL | ALWAYS |                   RESERVED                          *
** CHECK   |   1    |                                                     *
***************************************************************************
**
**
** IObased:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
** -CHECK  | -CHECK |                   RESERVED                          *
** ACTIVE  | STATUS |                                                     *
***************************************************************************
*/
#define POS_5           5               /* POS register 5 base offset */
#define POS5_CC         0x01            /* channel check */
#define POS5_CHKACT     0x80            /* check status indication */
#define POS5_CHKSTATUS  0x40            /* check status indication */


/*
**      POS Register 6   ( VPD address register )
**
** membased:
**      See POS 3 for description.
**
** IObased:
**
***************************************************************************
**    7    |   6    |    5   |   4    |    3   |    2   |    1   |    0   *
**                   RESERVED                  |BUS SIZE| A PAR  | D PAR  *
**                                             | ERROR  | ERROR  | ERROR  *
***************************************************************************
*/

#define POS_6           6                /* POS register 6 base offset */



/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/

#define IENT_FNAME_LEN        32

typedef struct error_log_def {
    struct err_rec0   errhead;        /* from com/inc/sys/err_rec.h      */
    uchar fname[IENT_FNAME_LEN];      /* filename and line number        */
    uchar pos_reg[8];                 /* Adapter POS Registers           */
    uchar ent_addr[ENT_NADR_LENGTH];  /* actual net address in use       */
    ulong parm1;                      /* log data 1                      */
    ulong parm2;                      /* log data 2                      */
    ulong parm3;                      /* log data 3                      */
};

#define uS_PER_MS      1000      /* Number of microseconds per millisecond */
#define NSEC_PER_MSEC  1000000   /* Number of nanoseconds per millisecond  */

#define MSEC_PER_SEC   (NS_PER_SEC / NSEC_PER_MSEC)


/*
** This routine will delay for ms number of milliseconds.
*/

#define DELAYMS(ms)                                                      \
{                                                                        \
    ulong limit;                                                         \
                                                                         \
    limit = ms * uS_PER_MS;                                              \
                                                                         \
    if (limit > (uS_PER_SECOND - 1))                                     \
    {                                                                    \
        limit = uS_PER_SECOND - 1;                                       \
    }                                                                    \
                                                                         \
    (void) io_delay(limit);                                              \
}

/*
**  Miscellaneous defines.
*/
#define BUS_TTL                 0x0008          /* bus throttle timer loaded */
#define CMD_COMMAND             0xFFFF0000      /* 0 off the status portion  */
#define CMD_STATUS              0x0000FFFF      /* 0 off the command portion */
#define BUSY                    0x01000000      /* 82596 in initialization   */
#define SCP_CFG_MEM_BASED       0x00007400      /* config field in SCP       */
#define SCP_CFG_IO_BASED        0x00005400      /* config field in SCP       */
#define MACH_MEM_BASED          0               /* platform identifier       */
#define MACH_IO_BASED           1               /* platform identifier       */
#define IOCC_SELECT             0x00000080      /* IOCC select bit on        */
#define BUSMEM_IO_SELECT        0x000C0020      /* BUS Mem/IO select bit on  */
#define NO_ADDR_CHECK_MASK      0xFFF3FFFF      /* no addr incr. or chk.     */
#define MFG_TEST_DATA           0x0000007F      /* IOCTL for Manufacturing   */

/* Bit-pattern constants for kludge for the Rainbow-3 systems. */
#define IS_PPC_BASED          0x01    /* Running on a Rainbow-3. */
#define DONE_CA_ALREADY       0x02    /* Channel_attention done. */

/*
void    retry_put(ulong, ulong, ulong);
void    retry_get(ulong, ulong, void *);
*/

/*
**  Disables the device by clearing bit 0 in POS register 2.
*/
#define DISABLE_DEVICE()                                                 \
{                                                                        \
    ulong ioccaddr;                                                      \
    ulong ioacc;                                                         \
    volatile ulong *crr;                                                 \
    char pos2val;                                                        \
                                                                         \
    ioacc = (ulong) io_att((DDS.bus_id | IOCC_SELECT), 0);               \
    ioccaddr = ioacc + (POS_REG_OFFSET + (DDS.slot << 16));              \
    BUS_GETCX((caddr_t) ioccaddr, &pos2val);                             \
    BUS_PUTCX((caddr_t) ioccaddr, (pos2val & 0xFE));                     \
                                                                         \
    io_det(ioacc);                                                       \
}
        
/*
**  Enables the device by setting bit 0 in POS register 2.
*/
#define ENABLE_DEVICE()                                                  \
{                                                                        \
    caddr_t ioacc;                                                       \
    char pos2val;                                                        \
                                                                         \
    ioacc = (caddr_t) io_att((DDS.bus_id | IOCC_SELECT),                 \
                            (POS_REG_OFFSET + (DDS.slot << 16)));        \
    BUS_GETCX(ioacc, &pos2val);                                          \
    BUS_PUTCX(ioacc, (pos2val | 0x01));                                  \
    io_det(ioacc);                                                       \
}
        
/*
**      Generate a channel attention signal to the 82596DX.
*/
#define CHANNEL_ATTENTION()                                             \
{                                                                       \
    ulong channel_att;                                                  \
                                                                        \
    if (WRK.machine == MACH_MEM_BASED)                                  \
    {                                                                   \
        if (__power_pc()) WRK.do_ca_on_intr = FALSE;  /* kludge */      \
                                                                        \
        __iospace_sync();                                               \
                                                                        \
        channel_att = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT),   \
                              (DDS.io_port + ENT_MEMBASED_CA_REG));     \
                                                                        \
        if (BUS_PUTLX((long *) channel_att, 0x01000000))                \
                retry_put(channel_att, 4, 0x01000000);                  \
                                                                        \
        io_det(channel_att);                                            \
                                                                        \
    }                                                                   \
    else                                                                \
    {                                                                   \
        if (BUS_PUTCX((caddr_t)(ioa + ENT_CA_IO_BASED), 0x00))          \
            retry_put(ioa + ENT_CA_IO_BASED, 1, 0x00);                  \
    }                                                                   \
}

#define GET_NETID(frame, offset)                                        \
        *((unsigned short *)((int)frame + offset))

#define ARAM_MASK       (WRK.aram_mask)

/*
**      address -       structure pointer
**      field -         field in structure
**      value -         value to write (type matches field)
*/
#define WRITE_LONG(location, value)                                     \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        (location) = (value);                                           \
    else                                                                \
        if (BUS_PUTLX((long *)((ulong)&(location) + ioa), value))       \
            retry_put((ulong)&location + ioa, 4, value);                \
}

/*
**      location -      value to read
**      value -         address to receive read value (type matches field)
*/
#define READ_LONG(location, vptr)                                       \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        *(vptr) = (location);                                           \
    else                                                                \
        if (BUS_GETLX((long *)((ulong)&(location) + ioa), (long *)vptr)) \
            retry_get((ulong) &location + ioa, 4, vptr);                \
}


/*
**      address -       structure pointer
**      field -         field in structure
**      value -         value to write
*/
#define WRITE_SHORT(location, value)                                    \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        (location) = (value);                                           \
    else                                                                \
        if (BUS_PUTSX((short *)((ulong)&(location) + ioa), value))      \
            retry_put((ulong)&location + ioa, 2, value);                \
}

/*
**      location -      value to read
**      value -         address to receive read value
*/
#define READ_SHORT(location, vptr)                                      \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        *(vptr) = (location);                                           \
    else                                                                \
        if (BUS_GETSX((short *)((ulong)&(location) + ioa), (short *)vptr)) \
            retry_get((ulong) &location + ioa, 2, vptr);                \
}


/*
**      address -       structure pointer
**      field -         field in structure
**      value -         value to write
*/
#define WRITE_CHAR(location, value)                                     \
{                                                                       \
        if (WRK.machine == MACH_MEM_BASED)                              \
                (location) = (value);                                   \
        else                                                            \
                if (BUS_PUTCX((char *)&(location) + ioa, value))        \
                        retry_put(&location + ioa, 1, value);           \
}

/*
**      location -      value to read
**      value -         address to receive read value
*/
#define READ_CHAR(location, vptr)                                       \
{                                                                       \
        if (WRK.machine == MACH_MEM_BASED)                              \
                *(vptr) = (*location);                                  \
        else                                                            \
                if (BUS_GETCX((char *)&(location) + ioa, (char *)vptr)) \
                        retry_get(&location + ioa, 1, vptr);            \
}


/*
**      address -       structure pointer
**      value -         value to write (type matches field)
*/
#define WRITE_LONG_REV(location, value)                                 \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        qrev_writel(value, &location);                                  \
    else                                                                \
        if (BUS_PUTLRX((long *)((ulong)&(location) + ioa), (long)value)) \
            retry_put((ulong)&location + ioa, 4, value);                \
}

/*
**      location -      value to read
**      value -         address to receive read value (type matches field)
*/
#define READ_LONG_REV(location, vptr)                                   \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        *(vptr) = qrev_readl(&(location));                              \
    else                                                                \
        if (BUS_GETLRX((long *)((ulong)&(location) + ioa), (long *) vptr)) \
            retry_get((ulong)&location + ioa, 4, vptr);                 \
}


/*
**      address -       structure pointer
**      field -         field in structure
**      value -         value to write
*/
#define WRITE_SHORT_REV(location, value)                                \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        qrev_writes(value, &(location));                                \
    else                                                                \
        if (BUS_PUTSRX((short *)((ulong)&(location) + ioa), (short)value)) \
            retry_put((ulong)&location + ioa, 2, value);                \
}

/*
**      location -      value to read
**      value -         address to receive read value
*/
#define READ_SHORT_REV(location, vptr)                                  \
{                                                                       \
    if (WRK.machine == MACH_MEM_BASED)                                  \
        *(vptr) = qrev_reads(&(location));                              \
    else                                                                \
        if (BUS_GETSRX((short *)((ulong)&(location) + ioa), (short *)vptr)) \
            retry_get((ulong)&location + ioa, 2, vptr);                 \ 
}




/*
**      MACROS will retry PIO's
*/
#define BUSPUTL(addr, value)                                            \
        if (BUS_PUTLX((long *)(ioa + (ulong)(addr)), value))            \
                retry_put(ioa + (ulong)(addr), 4, value)

#define BUSPUTC(addr, value)                                            \
        if (BUS_PUTCX((caddr_t)(ioa + (ulong)(addr)), value))           \
                retry_put(ioa + (ulong)(addr), 1, value)

#define BUSGETC(addr, vptr)                                             \
        if (BUS_GETCX((caddr_t)(ioa + (ulong)(addr)), vptr))            \
                retry_get(ioa + (ulong)(addr), 1, vptr)


#define IOCC_HANDLE(bid)                                                \
                ((0x0FF00000 & bid) | 0x800C00E0)

#define CLINESIZE       256     /* cache line size (must be power of 2) */
#define NEXT_CLINE(x)   (((ulong)(x) + (CLINESIZE - 1)) & ~(CLINESIZE - 1))
#define SMTOIOADDR(x)   ((ulong)(x) - WRK.sysmem + WRK.dma_base)
#define IOADDRTOSM(x) ((ulong)(x) + WRK.sysmem - WRK.dma_base)

#define ALLONES 0xFFFFFFFF

enum {
    PORT_RESET =    0x0,
    PORT_SELF_TEST =0x1,
    PORT_SCP =      0x2,
    PORT_DUMP =     0x3
};

/*
**      Self_test structure
*/
struct selftst
{
    ulong   signature;              /* sig of ROM content */
    ulong   result;                 /* results of test */
};


/*
**  Miscellaneous macros
*/

#define START_CU()                                                      \
{                                                                       \
        short   stat;                                                   \
        READ_SHORT(WRK.scb_ptr->status, &stat);                         \
        TRACE_DBG(HKWD_IENT_OTHER, "ScuC", stat, 0, 0);                 \
        if ( ! (stat & STAT_CUS_ACTIVE ))                               \
            WRITE_SHORT(WRK.scb_ptr->command, (ushort)CMD_CUC_START);   \
        CHANNEL_ATTENTION();                                            \
}

#define SUSPEND_RU()                                                    \
{                                                                       \
        READ_SHORT(WRK.scb_ptr->status, &status);                       \
        if ( ! (status & STAT_RUS_READY) )                              \
        {                                                               \
                WRITE_SHORT(WRK.scb_ptr->command, CMD_RUC_SUSPEND);     \
                CHANNEL_ATTENTION();                                    \
        }                                                               \
}

/*
**  COMMAND_QUIESCE
**  ensure the CU is not running.  When the command field is all zeros
**  then the CU has finished processing the command.
*/

#define COMMAND_QUIESCE(rc)                                             \
{                                                                       \
        int  k = 0, rc = 0;                                             \
        ushort cmd_value;                                               \
        while (k < 10) {                                                \
                READ_SHORT(WRK.scb_ptr->command, &cmd_value);           \
                if (!cmd_value)                                         \
                    break;                                              \
                k++;                                                    \
                                                                        \
                io_delay(100);                                          \
        }                                                               \
        if (k >= 10)                                                    \
            rc = -1;                                                    \
}

/*
** WAIT_FOR_COMMAND
** This is for the dump utility.  Since we are not interrupt driven we
** must poll the device to determine when it is done processing a commnand.
*/

#define WAIT_FOR_COMMAND(rc)                                            \
{                                                                       \
    int pass = 0;                                                       \
    ushort cmd_value;                                                   \
    ulong i;                                                            \
    char ch;                                                            \
                                                                        \
    rc = 0;                                                             \
    while (pass++ < 10)                                                 \
    {                                                                   \
        READ_LONG(WRK.acbl_ptr[0].csc, &value);                         \
        if (value & CB_OK) break;                                       \
        DELAYMS(10);                                                    \
        if (pass == 2) CHANNEL_ATTENTION();                             \
    }                                                                   \
    if (pass > 10) rc = EIO;                                            \
}

/*
** ABORT_CU
** This macro will abort the execution of the CU.
*/

#define ABORT_CU()                                                      \
{                                                                       \
    short stat;                                                         \
    READ_SHORT(WRK.scb_ptr->status, &stat);                             \
    if (!( stat & STAT_CUS_SUSPEND )) {                                 \
        WRITE_SHORT(WRK.scb_ptr->command, CMD_CUC_ABORT);               \
        CHANNEL_ATTENTION();                                            \
    }                                                                   \
}

#endif /* _H_IENT_HW */
