/* @(#)48	1.6  src/bos/kernel/sys/POWER/mpadefs.h, mpainc, bos411, 9435C411a 8/31/94 19:01:52 */
/*
 *   COMPONENT_NAME: (MPAINC) MP/A HEADER FILES
 *
 *   FUNCTIONS: COPYIN
 *		COPYOUT
 *		DISABLE_INTERRUPTS
 *		ENABLE_INTERRUPTS
 *		KFREE
 *		KMALLOC
 *		MPATRACE1
 *		MPATRACE2
 *		MPATRACE3
 *		MPATRACE4
 *		M_INPAGE
 *		PIO_GETC
 *		PIO_PUTC
 *		SLEEP
 *		WAKEUP
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 */

/*************************************************************************
 *  Multi Protocol Single Port Device Driver Defines                     *
 ************************************************************************/
#define ACMD_ACQ        (unsigned short)0
#define MAX_ADAPTERS    2               /* number of adapters supported */
#define MPA_MAX_OPENS   1
#define Q_CMD_TIMEOUT   100000		/* 100000 microseconds (100 ms) */
#define LOOP_CNTR	300
#define DL_DELAY_REG   0xE0
#define POS0           0x100            /* POS Register 0 IOCC offset   */
#define P0_F            0xFF            /* POS Card ID low, MPA        */
#define POS1           0x101            /* POS Register 1 IOCC offset   */
#define P1_F            0xDE            /* POS1 Card ID high, MPA      */
#define POS2           0x102            /* POS Register 2 IOCC offset   */
#define P2_ENABLE       0x01            /* -sleep/+ENABLE               */
#define P2_SDLC_MODE    0x10            /* Set SDLC mode                */
#define P2_ALT_SDLC     0x12            /* Set alternate SDLC mode      */
#define P2_DMA_ENABLED  0x20            /* Set if dma enabled for SDLC mode*/
#define P2_NOT_V25      0x40            /* 0 if V.25 bis exits*/
#define POS3           0x103            /* POS Register 3 IOCC offset   */
#define P3_ARB_1        0x01            /* arb level for SDLC */
#define P3_ARB_7        0x07            /* arb level for ALT SDLC */

#define MAX_FRAME_SIZE   4096    /* Maximum SDLC frame size */
#define MAX_XMITS	    7    /* Maximum SDLC xmits before poll/final bit */
#define POLL_BIT_OFFSET     1    /* Offset for poll/final bit */
#define POLL_BIT         0x10    /* Poll/Final Bit mask */

#define GENERAL        0x01      /* if set use general recv */
#define SELECTIVE      0x02      /* if set use selective recv */
#define BROADCAST_ADDR 0xFF      /* secondary "broadcast" address */
#define ENABLE_V.25  0x10        /* enable autodial */
#define SET_DTR      0x04        /* set DTR         */
#define SET_RTS      0x01        /* set RTS         */
#define SET_NRZI_DATA      0x01  /* Parameter to set NZRI encoded data */
#define SET_CLOCK_LOOPBACK 0x02  /* Parameter to set clock loopback    */
#define SET_DATA_LOOPBACK  0x04  /* Parameter to set data loopback     */

/**************************************************************************
 *   MPA 8255 PORT_C values. Defined for internal control functions     *
 *   and to monitor receive data.                                        *
 *   When the bit must be set to one or in the defined value.            *
 *   When the bit must be set to 0 and in the defined value.             *
 *   Bits pc0-pc3 are read/write and bits pc4-pc7 are read only.         *
 *   NOTE: using intel notation ordered as pc7-pc0.                      *
 **************************************************************************/
                             /* writeable*/

#define GATE_INT_CLK    0x01            /* 1 = Gate internal clock       */
#define GATE_EXT_CLK    0x02            /* 1 = Gate external clock       */
#define ELECTRONIC_WRAP 0x04            /* 1 = Electronic wrap           */
#define NO_IRPTS 0x08                   /* disable irpts and dma when set */
                             /* read only */
#define RECV_DATA       0x10            /* Oscillating = receiving data  */
#define TIMER0_OUT      0x20            /* Oscillating = timer 0 output  */
#define TEST_ACTIVE     0x40            /* 0 = Test actvie in elec wrap  */
#define SDLC_BSC        0x80            /* 1 = SDLC, 0 = BSC adapter     */

/**************************************************************************
 *   MPA 8255 PORT_B values. Used for modem control.                    *
 *   When the bit must be set to one or in the defined value.            *
 *   When the bit must be set to 0 and in the defined value.             *
 *   This is a read/write port.                                          *
 **************************************************************************/
#define SPEED_SEL_OFF   0x01            /* 0 = Turn on speed select      */
#define SEL_STANBY_OFF  0x02            /* 0 = Turn on Select Standby    */
#define TEST_OFF        0x04            /* 0 = Turn on test              */
#define FREE_STAT_CHG   0x08     /* 0 = Reset modem status changed logic */
#define RESET_8273      0x10            /* 1 = Reset the 8273            */
#define GATE_TIMER2     0x20            /* 1 = Gate timer 2              */
#define GATE_TIMER1     0x40            /* 1 = Gate timer 1              */
#define ENABLE_IRPT4    0x80            /* 1 = Enable irpt level 4       */

/**************************************************************************
 *   MPA OPTION 1 return codes                                           *
 **************************************************************************/
/* Results for all the xmit commands are passed back in the Tx I/R        */
/* (xmit result irpt reg). The TIC (xmit irpt codes) are defined as       */
/* follows:                                                               */
#define XMIT_EARLY_IRPT      12      /* Early Tx irpt                     */
#define XMIT_COMPLETE        13      /* xmit done ok                      */
#define XMIT_DMA_UNDERRUN    14      /* DAM underrun                      */
#define XMIT_CL_TO_SEND_ERR  15      /* clear to send error, no connect   */
#define XMIT_ABORT_DONE      16      /* abort complete.                   */
#define XMIT_PIO_ERROR       17      /* Pio error on xmit processing      */

/* the results for the first three receive commands are as follows:        */
/* in non_buffered mode the first byte in RxI/R reg (recv result irpt reg  */
/* is RIC (receive irpt code) then R0 and R1 (LSB and MSB of actual length */
/* received. The high order 3 bits of the RIC are for partial bytes        */
/* received on the last byte and must be ignored when checking RIC.        */
#define RECV_GEN_OK          0       /* Gen or sel A1 match read ok.       */
#define RECV_SEL_OK          1       /* Sel read ok A2 match.              */
#define RECV_CRC_ERR         3       /* Read crc error detected            */
#define RECV_ABORTED         4       /* Read abort detected                */
#define RECV_IDLE            5       /* Idle detected on read              */
#define RECV_EOP             6       /* EOP detected on read               */
#define RECV_BAD_FRAME       7       /* frame < 32 bits                    */
#define RECV_DMA_OVERRUN     8       /* dma overrun on read                */
#define RECV_MEM_OVERFLOW    9       /* memory buffer too small            */
#define RECV_CARRIER_DOWN    10      /* carrier detect failure             */
#define RECV_IRPT_OVERRUN    11      /* Receiver irpt overrun              */
#define RECV_PIO_ERROR       12      /* Pio error on recv processing      */

/* trace hook for mpa sdlc driver */
#ifndef HKWD_DD_MPADD
#define HKWD_DD_MPADD             0x22c00000   
#endif

#ifndef DD_MPA_HOOK
#define DD_MPA_HOOK         	  0x21          
#endif 

#ifndef MPA_RECV_DATA
#define MPA_RECV_DATA         	  0xC1          
#endif 

#ifndef MPA_XMIT_DATA
#define MPA_XMIT_DATA         	  0xC2          
#endif 
/* end of trace hook for mpa sdlc driver */

/* error ids for mpa sdlc driver */
#ifndef ERRID_MPA_ADPERR
#define ERRID_MPA_ADPERR     0x1a7039df /*  MP/A Adapter Error  */
#endif

#ifndef ERRID_MPA_XFTO
#define ERRID_MPA_XFTO       0x963defb4 /* Failsafe Transmit Timer */
#endif

#ifndef ERRID_MPA_DSRON
#define ERRID_MPA_DSRON      0xc8620de8 /*  DSR Already On for Switched Line  */
#endif

#ifndef ERRID_MPA_DSRDRP
#define ERRID_MPA_DSRDRP     0x5cf8ddba /*  DSR Dropped  */
#endif

#ifndef ERRID_MPA_CTSON
#define ERRID_MPA_CTSON      0x21db13ae /*  CTS Already On at Modem  */
#endif

#ifndef ERRID_MPA_CTSDRP
#define ERRID_MPA_CTSDRP     0x95895e26 /*  CTS Dropped During Transmit  */
#endif

#ifndef ERRID_MPA_RCVERR
#define ERRID_MPA_RCVERR     0xd1d9b302 /*  MPA Error On Receive Data */
#endif

#ifndef ERRID_MPA_RCVOVR
#define ERRID_MPA_RCVOVR     0x0590b818 /*  Receive Overrun    */
#endif

#ifndef ERRID_MPA_BFR
#define ERRID_MPA_BFR        0xfc39cd8c /*  Out of Resources  */
#endif

#ifndef ERRID_MPA_XMTUND
#define ERRID_MPA_XMTUND     0xc5c3e045 /*  Transmit Underrun  */
#endif
/* end of error ids for mpa sdlc driver */
/*****************************************************************************/
/* error threshold and trace size definitions                                */
/*****************************************************************************/
#ifdef MPA_DEBUG
#define RX_ABORT_THRESHOLD     1
#else
#define RX_ABORT_THRESHOLD     5000
#endif
#define MPA_TRACE_SIZE         (500*4) /* max number of trace table entries */

/*****************************************************************************/
/* internal trace routine                                                    */
/*****************************************************************************/
extern void mpa_trace (register uchar  str[],  /* trace data Footprint */
                        register ulong  arg2,   /* trace data */
                        register ulong  arg3,   /* trace data */
                        register ulong  arg4);  /* trace data */


/*****************************************************************************/
/* tracing macros                                                            */
/*****************************************************************************/

#define MPATRACE1(a1) \
        mpa_trace(a1, 0, 0, 0)
#define MPATRACE2(a1,a2) \
        mpa_trace(a1, a2, 0, 0)
#define MPATRACE3(a1,a2,a3) \
        mpa_trace(a1, a2, a3, 0)
#define MPATRACE4(a1,a2,a3,a4) \
        mpa_trace(a1, a2, a3, a4)

/*************************************************************************
  *  I/O Register Offsets from start of I/O Memory base address          *
  *  and the defined values for each register or port                    *
  ***********************************************************************/
#define PORT_A_8255     0x00            /* 8255 Port A Select offset     */
#define PORT_B_8255     0x01            /* 8255 Port B Select offset     */
#define PORT_C_8255     0x02            /* 8255 Port C Select offset      */
#define MODE_OFFSET     0x03            /* 8255 Mode Select offset      */
#define COUNT0_OFFSET   0x04            /* 8254 Counter0 Select */
#define COUNT1_OFFSET   0x05            /* 8254 Counter1 Select */
#define COUNT2_OFFSET   0x06            /* 8254 Counter2 Select */
#define CONTROL_OFFSET  0x07            /* 8254 Control  Select */
#define WR_CMD_OFFSET   0x08            /* 8273 output command reg */
#define RD_STAT_OFFSET  0x08            /* 8273 input status reg   */
#define WR_PARM_OFFSET  0x09            /* 8273 output parameter   */
#define RD_RES_OFFSET   0x09            /* 8273 input  result      */
#define WR_TEST_OFFSET  0x0A            /* 8273 output test mode   */
#define RD_TX_IR_OFFSET 0x0A            /* 8273 input xmit irpt result */
#define RD_RX_IR_OFFSET 0x0B            /* 8273 input recv irpt result */
#define WR_DATA_OFFSET  0x0C            /* 8273 output data reg    */
#define RD_DATA_OFFSET  0x0C            /* 8273 input data reg     */

/*************************************************************************
 *   MPA 8255 PORT_A values. Used for sensing external modem signals    *
 *   and internal signals. The 8273 has commands for reading this reg.   *
 *   This is a read only port.                                           *
 *************************************************************************/
#define RING_OFF        0x01            /* 0 = Ring indicator on         */
#define CARRIER_DET_OFF 0x02            /* 0 = Data carrier detect on    */
#define XMIT_CLK_ON     0x04            /* 1 = on , 0 = off              */
#define CTS_OFF   	0x08            /* 0 = Clear to send on          */
#define RECV_CLK_ON     0x10            /* 1 = on , 0 = off              */
#define MODEM_STAT_CHG  0x20            /* 1 = Modem status changed      */
#define TIMER2_ACTIVE   0x40            /* 1 = Timer 2 output active     */
#define TIMER1_ACTIVE   0x80            /* 1 = Timer 1 output active     */
#define TIMER_ACTIVE    0xC0            /* Either timer active           */
#define SET_8255_MODE   0x98            /* Set up mode for 8255         */

/***************************************************************************
*  This next section contains the definitions of the various commands      *
*  and the parameters that must be passed with the commands  and the       *
*  results passed back during the result phase.                            *
***************************************************************************/

/***************************************************************************
*                 Initialization/Configuration commands and parameters     *
*                 There are no results for these commands.                 *
***************************************************************************/
#define SET_1_BIT_DELAY_CMD  0xA4    /* cmd to set up one bit delay mode   */
#define RESET_1_BIT_DELAY_CMD  0x64  /* cmd to reset up one bit delay mode */
#define RESET_1_BIT_DELAY  0x7F      /* parameter to reset 1 bit delay     */
#define SET_DATA_XFER_CMD    0x97    /* cmd to set data xfer mode          */
/* NOTE: use this reset command with reset mask to set up for dma adapter  */
#define RESET_DATA_XFER_CMD  0x57    /* cmd to reset data xfer mode        */
#define RESET_TO_USE_DMA   0xFE      /* Parameter to set up dma xfer mode  */
#define SET_OPER_MODE_CMD    0x91    /* cmd to set operating mode reg      */
#define SET_FLAG_STREAM    0x01      /* Parameter to set flag stream mode  */
#define SET_PRE_FRAME_MODE 0x02      /* Parameter to set pre_frame sync mode */
#define SET_BUFFERED_MODE  0x04      /* Parameter to set buffered mode.    */
/* In buffered mode two more result bytes will be sent.. Addr and control  */
/* bytes will follow the RIC, R0 and R1 .                                  */
#define SET_EARLY_TX_ON    0x08      /* Parameter to set early xmit irpt on */
#define SET_EOP_IRPT_ON    0x10      /* Parameter to set end of poll irpt on */
#define SET_HDLC_ABORT     0x20      /* Parameter to set HDLC abort  */
#define SET_NO_DMA         0x01      /* Parameter to set irpt xfer mode    */
#define SET_1_BIT_DELAY    0x80      /* Parameter to set 1 bit delay       */
#define RESET_OPER_MODE_CMD  0x51    /* cmd to reset operating mode reg    */
#define RESET_FLAG_STREAM    0xFE    /* Parameter to reset flag stream mode  */
#define RESET_PRE_FRAME_MODE 0xFD    /* Parameter to reset pre_frame sync mode *
#define RESET_BUFFERED_MODE  0xFB    /* Parameter to reset buffered mode.    */
#define RESET_EARLY_TX_ON    0xF7    /* Parameter to reset early xmit irpt on */
#define RESET_EOP_IRPT_ON    0xEF    /* Parameter to reset end of poll irpt on *
#define RESET_HDLC_ABORT     0xDF    /* Parameter to reset HDLC abort  */
#define SET_IO_MODE_CMD      0xA0    /* cmd to set serial io mode reg      */
#define RESET_IO_MODE_CMD    0x60    /* cmd to reset serial io mode reg    */
#define RESET_NZRI_DATA      0xFE    /* Parameter to reset NZRI encoded data */
#define RESET_CLOCK_LOOPBACK 0xFD    /* Parameter to reset clock loopback    */
#define RESET_DATA_LOOPBACK  0xFB    /* Parameter to reset data loopback     */

/***************************************************************************
*  Reset command  no parms and no results. The reset is accomplished       *
*  outside the normal command interface by writing a 0x01 to the address   *
*  WR_TEST_OFFSET then waiting 10 cycles and writing 0x00.                 *
***************************************************************************/
#define START_RESET         0x01     /* start the reset process           */
#define END_RESET           0x00     /* end the reset process             */

/***************************************************************************
*              modem control commands parms and results                    *
*              8273 PORT A is read only, PORT B is read/write              *
***************************************************************************/
#define READ_8273_PORT_A_CMD 0x22    /* Cmd to read from 8273 Port A      */
                                     /* no parms, result in RD_RES_OFFSET */
#define PORT_A_8273_CTS      0x01    /* 1 = clear to send active          */
#define PORT_A_8273_CD       0x02    /* 1 = carrier detect active         */
#define PORT_A_8273_PA2      0x04    /* 1 = data set ready                */
#define PORT_A_8273_PA3      0x08    /* 1 = CTS changed                   */
#define PORT_A_8273_PA4      0x10    /* 1 = DSR changed                   */
#define READ_8273_PORT_B_CMD 0x23    /* Cmd to read from 8273 Port B      */
                                     /* no parms, data in RD_RES_OFFSET   */
#define SET_8273_PORT_B_CMD  0xA3    /* Cmd to set 8273 Port B values     */
                                     /* parameter is set_mask, no results */
#define SET_8273_PORT_B_RTS  0x01    /* mask bit to set req to send bit   */
#define SET_8273_PORT_B_PB1  0x02    /* mask bit to set PB1 bit           */
#define SET_8273_PORT_B_PB2  0x04    /* mask bit to set Data terminal ready */
#define SET_8273_PORT_B_PB3  0x08    /* mask bit to set PB3 bit           */
#define SET_8273_PORT_B_PB4  0x10    /* mask bit to set V.25 enable bit   */
#define SET_8273_PORT_B_PB5  0x20    /* mask bit to set flag detect bit   */
#define RESET_8273_PORT_B_CMD 0x63   /* Cmd to reset 8273 Port B values   */
                                     /* parameter reset_mask, no results  */
#define RESET_8273_PORT_B_RTS  0xFE  /* mask bit to reset req to send bit   */
#define RESET_8273_PORT_B_PB1  0xFD  /* mask bit to reset PB1 bit           */
#define RESET_8273_PORT_B_PB2  0xFB  /* mask bit to reset Data terminal ready */
#define RESET_8273_PORT_B_PB3  0xF7  /* mask bit to reset PB3 bit           */
#define RESET_8273_PORT_B_PB4  0xEF  /* mask bit to reset V.25 enable bit   */
#define RESET_8273_PORT_B_PB5  0xDF  /* mask bit to reset flag detect bit   */

#define MAX_RECV_SIZE   0xFFFF
#define KMALLOC(dtyp)   (dtyp *)xmalloc(sizeof(dtyp), 2, pinned_heap)
#define KFREE(buf)              xmfree((buf), pinned_heap)
#define OPENP           acb->open_struct
#define DISABLE_INTERRUPTS(lvl) lvl=i_disable(INTCLASS2)
#define ENABLE_INTERRUPTS(lvl)  i_enable(lvl)
#define SLEEP(el)       e_sleep (el, EVENT_SIGRET)
#define WAKEUP(el)      e_wakeup (el)
#define DDS             acb->mpaddp
#define SLOT            (DDS.slot_num)
#define ARB             (DDS.dma_lvl)
#define MPA_IOCC_ATT    (ulong)(IOCC_ATT(DDS.bus_id,IO_IOCC+(SLOT << 16)))
#define MPA_CHAN_STAT   (ulong)(IOCC_ATT(DDS.bus_id,0x004F0060))
#define MPA_DMA_STAT    (ulong)(IOCC_ATT(DDS.bus_id,IO_IOCC+(ARB << 16)+0x60))
#define MPA_CCR_ATT     (ulong)(IOCC_ATT(DDS.bus_id,IO_IOCC+0x2C))
#define MPA_BUSIO_ATT   (ulong)(BUSIO_ATT(DDS.bus_id, DDS.io_addr))
#define CBSY_0           2
#define CRBF_1           3
#define CPBF_0           4

/***************************************************************************
*                 Receive commands  parameters and results                 *
****************************************************************************/
#define GEN_RECEIVE_CMD      0xC0    /* cmd to general receive             */
                                     /* parms are LSB then MSB of recv len */
#define SEL_RECEIVE_CMD      0xC1    /* cmd to selective receive           */
                                     /* parms are LSB then MSB of recv len */
                                     /* then match addr 1, match addr 2    */
#define LOOP_RECEIVE_CMD     0xC2    /* cmd to selective loop receive      */
                                     /* parms are LSB then MSB of recv len */
                                     /* then match addr 1, match addr 2    */
#define DISABLE_RECV_CMD     0xC5    /* cmd to disable the receiver        */


#define RIC_MASK_LAST_BYTE   0x1F    /* used to and with RIC to remove the */
                                     /* last byte bit count                */

/***************************************************************************
*                 Transmit commands  parameters and results                *
****************************************************************************/
#define XMIT_CMD             0xC8    /* cmd to xmit                        */
                                     /* parms are LSB then MSB of xmit len */
                                     /* in buffered mode send A, and C too */
#define XMIT_ABORT_CMD       0xCC    /* cmd to abort xmit                  */
                                     /* No parms.                          */
#define LOOP_XMIT_CMD        0xCA    /* cmd to do loop xmit                */
                                     /* parms are LSB then MSB of xmit len */
                                     /* in buffered mode send A, and C too */
#define LOOP_ABORT_CMD       0xCE    /* cmd to abort loop xmit             */
                                     /* No parms.                          */
#define TRANSPARENT_XMIT_CMD 0xC9    /* cmd to do transparent xmit         */
                                     /* parms are LSB then MSB of xmit len */
#define TRANSPARENT_ABORT_CMD 0xCD   /* cmd to do transparent xmit         */
                                     /* No parms.                          */

/***************************************************************************
*              Status reg definitions. Status read from                    *
*                      RD_STAT_OFFSET                                      *
***************************************************************************/
#define TX_RESULT_READY        0x01  /* 1 = Tx result in TxI/R            */
#define RX_RESULT_READY        0x02  /* 1 = Rx result in RxI/R            */
#define TX_IRPT_ACTIVE         0x04  /* 1 = xmit irpt active              */
#define RX_IRPT_ACTIVE         0x08  /* 1 = recv irpt active              */
#define CRBF                   0x10  /* 1 = command result buffer full    */
#define CPBF                   0x20  /* 1 = command parm buffer full      */
#define CBF                    0x40  /* 1 = command buffer full           */
#define CBSY                   0x80  /* 1 = in command phase              */
#define IRPT_PENDING           0x0C  /* to see if there is a TX or RX     */
                                     /* interrupt in the status reg       */

/**************************************************************************
 *   MPA MACRO DEFINITIONS                                               *
 **************************************************************************/

/*----------------------------------------------------------------------*/
/*  M_INPAGE  for checking funky mbufs                                  */
/*  This macro determines if the data portion of an mbuf resides within */
/*  one page -- if TRUE is returned, the data does not cross a page     */
/*  boundary.  If FALSE is returned, the data does cross a page         */
/*  boundary and cannot be d_mastered.                                  */
/*----------------------------------------------------------------------*/

# define M_INPAGE(m)    ((((int)MTOD((m), uchar *)                      \
                                & ~(PAGESIZE - 1)) + PAGESIZE) >        \
                                    ((int)MTOD((m), uchar *) + (m)->m_len))


/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1   */
/*  is returned (hence all return an int value).  In the cases of         */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

# define C              1       /* Character type of PIO access */
# define S              2       /* Short type of PIO access */
# define SR             3       /* Short-reversed type of PIO access */
# define L              4       /* Long type of PIO access */
# define LR             5       /* Long-reverse type of PIO access */

# define PIO_GETC( p, a )          ((int) PioGet( p, a ))

# define PIO_PUTC( p, a, v )       ((int) PioPut( p, a, v ))


/*
** Macros to get/put caller's parameter block
** Useful for "arg" in ioctl and for extended paramters on other dd entries.
** Value is 0 if ok, otherwise EFAULT.
*/
#define COPYIN(dvf,usa,dda,siz)                               \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( copyin(usa,dda,siz) ) ) )

#define COPYOUT(dvf,dda,usa,siz)                              \
( (usa == NULL) ? (EFAULT) :                                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( copyout(dda,usa,siz) ) ) )
