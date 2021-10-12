/* @(#)52       1.1  src/bos/kernel/sys/POWER/dmpauser.h, sysxdmpa, bos411, 9428A410j 4/30/93 12:34:50 */
/*
 *   COMPONENT_NAME: (MPAINC) MP/A HEADER FILES
 *
 *   FUNCTIONS: 
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

#include <sys/termio.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/types.h>
#include <sys/err_rec.h>


#define MAX_FRAME_SIZE   4096    /* Maximum SDLC frame size */
#define MAX_CHAN         1       /* Maximum number of channels */
				 /* this is not multipled device driver */

typedef struct errmsg {
	struct  err_rec0 err;
	char    file[32];
	char    data1[80];   /* use data1 and data2 to show detail  */
	int     data2;       /* data in the errlog report. Define   */
			     /* these fields in the errlog template */
			     /* These fields may not be used in all */
			     /* cases.                              */
} ;


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

/* In buffered mode two more result bytes will be sent.. Addr and control  */
/* bytes will follow the RIC, R0 and R1 .                                  */





/**************************************************************************
 *   MPA IOCTL COMMANDS                                                  *
 **************************************************************************/
#define MPA                    ('m'<<8)
#define MPA_RW_PORT             (MPA|0xA1)
#define MPA_CMD_8273            (MPA|0xA2)
#define MPA_RW_POS              (MPA|0xA3)
#define MPA_GET_ACB             (MPA|0xA4)

/**************************************************************************
 *   MPA codes returned in option[0] as ASYNC status                     *
 **************************************************************************/



/**************************************************************************
 *   MPA IOCTL DIAGNOSTIC/SETUP COMMANDS                                 *
 **************************************************************************/


/******* these defines are for trace hooks   *******************/

#define ADAPT_TOO_BIG           0x000000f0   /* adapter number is too big */
#define NO_ACB                  0x000000f1   /* acb is NULL               */
#define NO_OFFL_INTR            0x000000f2   /* there is no offlevel intr
						  structure                */
#define NO_INTR_REG             0x000000f3   /* no interrupt registration */
#define NO_PORT_DDS             0x000000f4   /* no dds for requested port */
#define CHAN_TOO_BIG            0x000000f5   /* channel > CHAN_MAX        */
#define CHAN_BUSY               0x000000f6   /* channed is used by kernel */
#define NO_MBUF_AVAIL           0x000000f7   /* no mbuf available         */
#define NO_XMIT_CHAIN           0x000000f8   /* no transmit chain         */
#define ADAPT_ALRDY_OPEN        0x000000f9   /* adapter already opened    */
#define UIO_MOVE_ERR            0x000000fd   /* error in uiomove          */
#define PHYS_LINK_INV           0x000000e0   /* physical link is invalid  */
#define DATA_PROTO_INV          0x000000e1   /* data protocol is invalid  */
#define BAUD_RATE_INV           0x000000e2   /* baud rate is invalid      */
#define NO_ERROR                0x000000e3
#define PARM1                   0x00000001   /* parameter # 1    */
#define PARM2                   0x00000002   /* parameter # 2    */
#define PARM3                   0x00000003   /* parameter # 3    */
#define PARM4                   0x00000004   /* parameter # 4    */


/*
 *  Multi Protocol Single Port start device data structure Definition
 */
typedef struct
{
	char    retry_cnt;
	char    rsv      ;
	ushort  retry_delay;
	ushort  cps_group_0;
	ushort  cps_group_1;
	ushort  cps_group_2;
	ushort  cps_group_3;
	ushort  cps_group_4;
	ushort  cps_group_5;
	ushort  cps_group_6;
	ushort  cps_group_7;
	ushort  cps_group_8;
	ushort  cps_group_9;
	ushort  cps_netlog_0;
	ushort  cps_netlog_1;
	ushort  cps_netlog_2;
	ushort  cps_netlog_3;
	ushort  cps_netlog_4;
	ushort  cps_netlog_5;
	ushort  cps_netlog_6;
	ushort  cps_netlog_7;
	ushort  cps_netlog_8;
	ushort  cps_netlog_9;
	char    gr0_thresh;
	char    gr1_thresh;
	char    gr2_thresh;
	char    gr3_thresh;
	char    gr4_thresh;
	char    gr5_thresh;
	char    gr6_thresh;
	char    gr7_thresh;
	char    gr8_thresh;
	char    gr9_thresh;
	ushort  select_sig_len;
	char    select_sig[256];
} t_x21_data;
typedef struct T_ERR_THRESH
{
	ulong   tx_err_thresh;
	ulong   rx_err_thresh;
	ulong   tx_err_percent;
	ulong   rx_err_percent;
	ulong   tx_underrun_thresh;
	ulong   tx_cts_drop_thresh;
	ulong   tx_cts_timeout_thresh;
	ulong   tx_fs_timeout_thresh;
	ulong   rx_overrun_err_thresh;
	ulong   rx_abort_err_thresh;
	ulong   rx_frame_err_thresh;
	ulong   rx_par_err_thresh;
	ulong   rx_bad_sync_thresh;
	ulong   rx_dma_bfr_err_thresh;
} t_err_threshold;

typedef struct MPA_START
{
    cio_sess_blk_t      sb;
    unsigned char       phys_link;        /* physical link        */
    unsigned char       station_type;     /* specify type of station */
#define  PRIMARY        0x01              /* if set use general recv */
#define  SECONDARY      0x02              /* if set use selective recv */
    unsigned char       dial_proto;       /* dial protocol        */
    unsigned char       dial_flags;       /* dial protocol        */
#define  ENABLE_V.25  0x10                /* enable autodial */
#define  SET_DTR      0x04                /* set DTR         */
#define  SET_RTS      0x01                /* set RTS         */

    unsigned char       data_proto;       /* protocol in data transfer */
#define  SDLC     0x01
#define  BI_SYNC  0x02
#define  ASYNC    0x04

    unsigned char       data_flags;       /* protocol flags for data */
#define SET_NRZI_DATA      0x01      /* Parameter to set NZRI encoded data */
#define SET_CLOCK_LOOPBACK 0x02      /* Parameter to set clock loopback    */
#define SET_DATA_LOOPBACK  0x04      /* Parameter to set data loopback     */
					  /* serial i/o mode reg      */

    unsigned char       modem_flags;      /* modem flags sets 8255 Port C */
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

    unsigned char       port_b_8255;    /* modem flags sets 8255 Port B */
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


    unsigned char       poll_addr;        /* poll address         */
    unsigned char       select_addr;      /* select address       */
    unsigned char       select_cntl;      /* select control       */
    unsigned char       modem_intr_mask;  /* currently unused     */
    unsigned short      baud_rate;        /* baud rate for internal clk */
    unsigned short      rcv_timeout;      /* receive time out     */
    unsigned short      rcv_data_offset;  /* receive data offset  */
    unsigned short      dial_data_length; /* length of dial data  */
    unsigned char       mode_reg_flags;   /* 8273 mode reg flags  */
#define SET_FLAG_STREAM    0x01      /* Parameter to set flag stream mode  */
#define SET_PRE_FRAME_MODE 0x02      /* Parameter to set pre_frame sync mode */
#define SET_BUFFERED_MODE  0x04      /* Parameter to set buffered mode.    */
#define SET_EARLY_TX_ON    0x08      /* Parameter to set early xmit irpt on */
#define SET_EOP_IRPT_ON    0x10      /* Parameter to set end of poll irpt on */
#define SET_HDLC_ABORT     0x20      /* Parameter to set HDLC abort  */
    unsigned char       xfer_mode;   /* 8273 xfer mode reg   */
#define SET_NO_DMA         0x01      /* Parameter to set irpt xfer mode    */
    unsigned char       bit_delay;   /* 8273 one bit delay mode setting */
#define SET_1_BIT_DELAY    0x80      /* Parameter to set 1 bit delay       */
    unsigned char       diag_flags;  /* flags for various diagnostics   */
#define XMIT_ONLY          0x01      /* when set no recv command will be issued */

    union
    {
	t_x21_data      *p_x21_data;
	char            (*p_dial_data) [256];
    } t_dial_data;
    t_err_threshold     *p_err_threshold;
} mpa_start_t ;

typedef struct T_ERR_LOG
{
	unsigned        error_id;
	char            resource_name[8];
	char            detail_data[64];        /* detail data of error */
} t_error_log;

typedef struct mp_write_extension
{
    struct write_extension      cio_ext;   /* COMIO write extension */
    uchar                       transparent; /* bisync transparent mode flag */
    uchar                       adr;
    uchar                       cntl;
} t_write_ext;

/******************************************************************************
 *   MSQP card register state structure.                                   *
 ***************************************************************************/
typedef struct Q_STATE
{
    uchar     port_a_8255;
    uchar     port_b_8255;
    uchar     port_c_8255;
    uchar     mode_sel_8255;
    int       cnt_0_8254;           /* not used */
    int       cnt_1_8254;           /* not used */
    int       cnt_2_8254;           /* not used */
    uchar     statreg_8273;
    uchar     resreg_8273;
    uchar     tx_ir_8273;
    uchar     rx_ir_8273;
    uchar     port_a_8273;
    uchar     port_b_8273;
    uchar     oper_mode_8273;    /* these last fout regs are internal to */
    uchar     serial_io_8273;    /* 8273 and can't be read, so I set up  */
    uchar     one_bit_8273;      /* these values as I set up these regs  */
    uchar     data_xfer_8273;
} card_state_t;


/*
** Device-specific statistics
** Returned in driver statistics structure.
*/
typedef struct mpa_stats {
	ulong sta_que_overflow;         /* status lost, full status que */
	ulong recv_lost_data;           /* receive packet lost*/
	ulong total_intr;               /* total interrupts */
	ulong recv_not_handled;         /* interrupts not handled, */
	ulong xmit_not_handled;         /* interrupts not handled, */
	ulong recv_irpt_error;          /* interrupts with no results */
	ulong xmit_irpt_error;          /* interrupts with no results */
	ulong recv_intr_cnt;            /* number of receive interrupts */
	ulong xmit_intr_cnt;            /* number of transmit interrupts */
	ulong rec_no_mbuf;              /* no mbuf available */
	ulong xmit_sent;                /* transmit commands send to MPA */
	ulong xmit_dma_completes;       /* transmit dma transfers completed */
	ulong recv_sent;                /* receive dma transfers completed */
	ulong recv_dma_completes;       /* receive dma transfers completed */
	ulong recv_crc_errors;
	ulong recv_aborts;
	ulong recv_idle_detects;
	ulong recv_eop_detects;
	ulong recv_frame_to_small;
	ulong recv_dma_overruns;
	ulong recv_buf_overflow;
	ulong recv_cd_failure;
	ulong recv_irpt_overruns;
	ulong xmit_early_irpts;
	ulong xmit_completes;
	ulong xmit_dma_underrun;
	ulong xmit_cts_errors;
	ulong xmit_aborts;
	ulong recv_completes;
	ulong io_irpt_error;
	int   bps_rate;
	ulong recv_pio_byte;
	ulong xmit_pio_byte;
	ulong irpt_fail;
	ulong irpt_succ;
} mpa_stats_t;

/*
** Driver statistics structure.
** Returned by CIO_QUERY ioctl operation
*/
typedef struct QUERY {
	cio_stats_t cc;                 /* General COMIO statistics */
	mpa_stats_t ds;                 /* Device specific statistics */
} mpa_query_t;

typedef struct rw_port
{
    int            port_addr;   /* i/o addr index to the desired port    */
    uchar          rw_flag;     /* in - destination buffer pointer */
#define MPA_WRITE   0x01
#define MPA_READ    0x02
    uchar          value;       /* value to write or value read */
} rw_port_t;

typedef struct COMMAND {
      uchar cmd;
      uchar parm[3];
      int parm_count;
      uchar flag;
#define RETURN_RESULT        0x01    /* Set if result expected. */
      uchar result;
} cmd_phase_t;

typedef struct FRAME {
      uchar  addr;
      uchar  cntl;
      uchar  data[MAX_FRAME_SIZE - 2];
} frame_t;

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
 *   When checking bit values, if 0 indicates the condition, negate the  *
 *   if statement. example  if( !(reg_val & RING_ON) ) { ring is on }    *
 *   This is a read only port.                                           *
 *************************************************************************/
#define RING_ON         0x01            /* 0 = Ring indicator on         */
#define CARRIER_DET     0x02            /* 0 = Data carrier detect on    */
#define XMIT_CLK_ON     0x04            /* 1 = on , 0 = off              */
#define CLEAR_TO_SEND   0x08            /* 0 = Clear to send on          */
#define RECV_CLK_ON     0x10            /* 1 = on , 0 = off              */
#define MODEM_STAT_CHG  0x20            /* 1 = Modem status changed      */
#define TIMER2_ACTIVE   0x40            /* 1 = Timer 2 output active     */
#define TIMER1_ACTIVE   0x80            /* 1 = Timer 1 output active     */
#define TIMER_ACTIVE    0xC0            /* Either timer active           */


#define SET_8255_MODE   0x98            /* Set up mode for 8255         */

/***************************************************************************
*           Currently no defines for the 8254 counter hardware             *
***************************************************************************/


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

#define RESET_OPER_MODE_CMD  0x51    /* cmd to reset operating mode reg    */
#define RESET_FLAG_STREAM    0xFE    /* Parameter to reset flag stream mode  */
#define RESET_PRE_FRAME_MODE 0xFD    /* Parameter to reset pre_frame sync mode */
#define RESET_BUFFERED_MODE  0xFB    /* Parameter to reset buffered mode.    */
#define RESET_EARLY_TX_ON    0xF7    /* Parameter to reset early xmit irpt on */
#define RESET_EOP_IRPT_ON    0xEF    /* Parameter to reset end of poll irpt on */
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


