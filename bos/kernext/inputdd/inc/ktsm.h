/* @(#)43	1.5  src/bos/kernext/inputdd/inc/ktsm.h, inputdd, bos411, 9434B411a 8/25/94 05:52:46  */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard/Tablet/Sound/Mouse DD - ktsm.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include  "ktsmtrace.h"                 /* must be first                     */
#include  <sys/types.h>
#include  <sys/sleep.h>                 /* wait structure                    */
#include  <sys/timer.h>
#include  <sys/malloc.h>                /* memory allocation definitions     */
#include  <sys/sysmacros.h>             /* defines the minor() macro         */
#include  <sys/errno.h>
#include  <sys/pin.h>
#include  <sys/ioacc.h>                 /* bus access macros                 */
#include  <sys/iocc.h>
#include  <sys/ioctl.h>
#include  <sys/adspace.h>
#include  <sys/intr.h>
#include  <sys/device.h>
#include  <sys/except.h>
#include  <sys/uio.h>
#include  <sys/xmem.h>
#include  <sys/cfgdb.h>
#include  <sys/signal.h>
#include  <sys/devinfo.h>
#include  <sys/dump.h>
#include  <sys/inputdd.h>
#include  <sys/ktsmdds.h>
#include  <stdarg.h>
#include  <sys/errids.h>
#include  <string.h>
#include  "tabcmd.h"

/*****************************************************************************/
/* Structures for NIO port addresses                                         */
/*****************************************************************************/

struct kts_port                         /* keyboard / tablet port structure  */
{                                       /*  for RS1/RS2                      */
   short w_port_a;                      /* write port a address          50  */
   short pad1;                          /* no register                       */
   char  r_port_a;                      /* read  port a address          54  */
   char  r_port_b;                      /* read  port b address          55  */
   char  r_port_c;                      /* read  port c address          56  */
   char  w_port_ced;                    /* config8255,enable,disable irq 57  */
   char  w_port_3key;                   /* clr 3key int, read 3key stat  58  */
   char  r_cmd_reg;                     /* read command register         59  */
   short pad2;                          /* no register                       */
};

struct kbd_port                         /* keyboard for RSC                  */
{
   char  rd_wd_port;                    /* read data/ write data         50  */
   char  rs_wc_port;                    /* read status/ write command    51  */
   short pad1;                          /* no register                       */
   char  spkr_port_lo;                  /* speaker port: freq bits 7-0   54  */
   char  spkr_port_hi;                  /* speaker port                  55  */
};

struct tablet_port                      /* tablet port structure for RSC     */
{
   char  rw_dll;                        /* read/write/Divisor Latch LSB    70*/
   char  dlm_ie;                        /* Divisor Latch MSB and Int Enable71*/
   char  idd_fifo_alt;                  /* Int Id, FIFO control, Alt Fun   72*/
   char  line_ctl;                      /* Line Control                    73*/
   char  modem_ctl;                     /* Modem Control                   74*/
   char  line_status;                   /* Line Status                     75*/
   char  modem_status;                  /* Modem Status                    76*/
   char  scratch ;                      /* scratch register                77*/
};

struct mouse_port                       /* mouse port                        */
{
   short  w_data_cmd;                   /* write xmit cmd reg            48  */
   char   r_astat_reg;                  /* read adapter status register  4A  */
   char   padding;                      /* no register                       */
   long   r_mdata_regs;                 /* read rcv-status/data regs     4C  */
};

/*****************************************************************************/
/* Keyboard/Tablet Defines - RS1/RS2 Platforms                               */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* POS register 2 bits                                                       */
/*---------------------------------------------------------------------------*/
#define  RESET_KEYBOARD_8051  0x04
#define  NOT_RESET_8255       0x02

/*---------------------------------------------------------------------------*/
/* 8255 register bits (kts_port)                                             */
/*---------------------------------------------------------------------------*/
/* port b (0x55) - sense register                                            */
#define  SVB1                 0x40      /* _1______ spkr vol bit 1           */
#define  SVB0                 0x20      /* __1_____ spkr vol bit 0           */
#define  ACKN                 0x10      /* ___1____ acknowledge              */
#define  STRB                 0x08      /* ____1___ strobe                   */
#define  TFG                  0x04      /* _____1__ tab fuse good            */
#define  KFG                  0x02      /* ______1_ kbd fuse good            */
#define  T_RXD                0x01      /* _______1 tab ready sig            */

/* port c (0x56) - status register                                           */
#define  OBE                  0x80      /* 1_______ out buf empty=1          */
#define  IBF                  0x20      /* __1_____ inp buf full=1           */
#define  IRQ1                 0x08      /* ____1___ intr req line            */

#define  INT_TYPE             0x07      /* _____111 interrupt type           */
#define    INFOBYTE           0x00      /* _____000   informational          */
#define    KYBDBYTE           0x01      /* _____001   byte from kbd          */
#define    UARTBYTE           0x02      /* _____010   byte from tab          */
#define    REQ_BYTE           0x03      /* _____011   req byte               */
#define    BLK_XFER           0x04      /* _____100   block xfer             */
#define    COMPCODE           0x06      /* _____110   complete code          */
#define    ERR_CODE           0x07      /* _____111   error code             */

/* 8255 configuration parameters (port 0x57)                                 */
#define  CNFGDATA             0xC3      /* configure 8255    (cnfg8255)      */
#define  ENABLE_INT           0x09      /* enable interrupts (enable_irq)    */
#define  DISABLE_INT          0x08      /* disable interrupt (disable_irq)   */

/*---------------------------------------------------------------------------*/
/* Adapter Commands                                                          */
/*---------------------------------------------------------------------------*/

#define  CMD_8051             0x00FF    /* 8051 command                      */
/*                            0x??00       Extended cmd set (see below)      */
#define  WRITE_KBD_CMD        0x0001    /* Write to kybd cmd                 */
#define  WRITE_SPK_CMD        0x0002    /* Write to spkr cmd (dur lo 1/128s) */
#define  WRITE_TAB_CMD        0x0003    /* Write to tablet cmd               */
#define  WRITE_TAB_QRY_CMD    0x0004    /* Write to tablet query command     */
#define  SET_TAB_BAUD_RATE    0x0005    /* Set tablet baud rate command      */
#define  SET_TAB_FRAMING      0x0006    /* Set tablet blocking factor        */
#define  SET_SPK_DURATION     0x0007    /* Set spkr duration hi cmd          */
#define  SET_SPK_FREQ_HI      0x0008    /* Set spkr frequency hi cmd         */
#define  SET_SPK_FREQ_LO      0x0009    /* Set spkr frequency lo cmd         */
#define  DIAG_WRITE_KBD_PORT  0x000C    /* Diagnostic write kbd port cmd     */
/*                            0x001?       Write to shared ram addr  ?       */

/* configuation values used by driver                                        */
#define  SET_9600_BAUD_RATE   0xFB05    /* set baud rate of  9600 bits/sec   */
#define  TAB_BLOCK_6          0x8606    /* Set blocking factor to 6          */

/*---------------------------------------------------------------------------*/
/* Extended Adapter Commands                                                 */
/*---------------------------------------------------------------------------*/

/* "Read only" shared ram                                                    */
                                        /* DEFINITION                    INIT*/
#define  RD_SRAM_KBD_ACK      0x0000    /* read keyboard acknowledge     0xFA*/
#define  RD_SRAM_PND_DUR_TK_1 0x0100    /* read pending duration ticks1  0x00*/
#define  RD_SRAM_PND_DUR_TK_2 0x0200    /* read pending duration ticks2  0x00*/
#define  RD_SRAM_KBD_RESP_CMD 0x0300    /* read keyboard response cmd    0xFE*/
#define  RD_SRAM_KBD_BRK_CODE 0x0400    /* read keyboard break code      0xF0*/
#define  RD_SRAM_KBD_MAX_TRY  0x0500    /* read keyboard max re-tries    0x08*/
#define  RD_SRAM_KBD_ECHO     0x0600    /* read keyboard echo            0xEE*/
#define  RD_SRAM_CLICK_DUR    0x0700    /* read click duration           0x36*/
#define  RD_SRAM_CLK_SUP_SC_1 0x0800    /* read click supress scan code1 0x12*/
#define  RD_SRAM_CLK_SUP_SC_2 0x0900    /* read click supress scan code2 0x59*/
#define  RD_SRAM_CLK_SUP_SC_3 0x0A00    /* read click supress scan code3 0x39*/
#define  RD_SRAM_CLK_FRQ_HI   0x0B00    /* read click frequency hi       0x08*/
#define  RD_SRAM_CLK_FRQ_LO   0x0C00    /* read click frequency lo       0x96*/
#define  RD_SRAM_K_SCAN1      0x0D00    /* read sys atn key scan code 1  0x11*/
#define  RD_SRAM_K_SCAN2      0x0E00    /* read sys atn key scan code 2  0x19*/
#define  RD_SRAM_K_SCAN3      0x0F00    /* read sys atn key scan code 3  0xFF*/

#define  DUR_TICK_H_REMAIN    0x1300    /* active spkr dur ticks left (hi)   */
#define  DUR_TICK_L_REMAIN    0x1400    /* active spkr dur ticks left (lo)   */
#define  PEND_SPKR_FREQ_H     0x1500    /* pending spkr freq left     (hi)   */
#define  PEND_SPKR_FREQ_L     0x1600    /* pending spkr freq left     (lo)   */
#define  SCAN_COUNT           0x1700    /* scan count for system attn key    */
#define  SEQUENCE_STATE       0x1800    /* state of system attn key sequence */
#define  TABLET_FRAMING       0x1900    /* b7=parity(1=odd) b012=blk factor  */
#define  SYS_ATN_SCAN_CODE    0x1A00    /* actual 3rd byte recieved in seq   */
#define  RD_TAB_BAUD_RATE     0x1B00    /* counter reload value              */
#define  TEST_COMP_CODE       0x1C00    /* actual value of 8051 self test    */
#define  ABORT_INFO_H         0x1D00    /* byte 1 of abort information       */
#define  ABORT_INFO_L         0x1E00    /* byte 2 of abort information       */
#define  LATEST_ERROR_CODE    0x1F00    /* error code for most recent int#7  */

/* "Reset mode bits"                                                         */
                                        /* DESCRIPTION                 INIT  */
#define  DONT_REP_KBD_ACK     0x2000    /* Report reciept of kbd acks  (off) */
#define  DONT_REP_COMP_TAB    0x2100    /* Report comp of tablet xmit  (off) */
#define  DONT_REP_COMP_TONE   0x2300    /* Report comp of spkr tone    (on)  */
#define  DONT_BLK_TAB_BYTES   0x2500    /* Blocking of recvd tab bytes (on)  */
#define  DONT_SRCH_SYS_ATN    0x2600    /* Search for sys atn key seq  (on)  */
#define  SURPRESS_CLICK       0x2700    /* Surpress click              (off) */
#define  RES_SPKR_VOL_BIT0    0x2800    /* Speaker volume bit 0        (off) */
#define  RES_SPKR_VOL_BIT1    0x2900    /* Speaker volume bit 1        (on)  */
#define  DONT_SET_DIAG_MODE   0x2A00    /* Diagnostic mode in effect   (off) */
#define  DISABLE_KBD          0x2B00    /* Disable keyboard interface  (on)  */
#define  DISABLE_TAB          0x2C00    /* Disable tablet interface    (off) */
#define  AUTO_CLICK_ON        0x2E00    /* Turn on keystroke auto click(on)  */

/* "Set mode bits"                                                           */
                                        /* DESCRIPTION                 INIT  */
#define  DO_REP_KBD_ACK       0x3000    /* Report reciept of kbd acks  (off) */
#define  DO_REP_COMP_TAB      0x3100    /* Report comp of tablet xmit  (off) */
#define  DO_REP_COMP_TONE     0x3300    /* Report comp of spkr tone    (on)  */
#define  DO_BLK_TAB_BYTES     0x3500    /* Blocking of recvd tab bytes (on)  */
#define  DO_SRCH_SYS_ATN      0x3600    /* Search for sys atn key seq  (on)  */
#define  PLAY_CLICK           0x3700    /* Surpress click              (off) */
#define  SET_SPKR_VOL_BIT0    0x3800    /* Speaker volume bit 0        (off) */
#define  SET_SPKR_VOL_BIT1    0x3900    /* Speaker volume bit 1        (on)  */
#define  DO_SET_DIAG_MODE     0x3A00    /* Diagnostic mode in effect   (off) */
#define  ENABLE_KBD           0x3B00    /* Disable keyboard interface  (on)  */
#define  ENABLE_TAB           0x3C00    /* Disable tablet interface    (off) */
#define  AUTO_CLICK_OFF       0x3E00    /* Turn off keystrke auto click(on)  */

#define  SET_SPKR_VOL         0x4000    /* Set speaker volume cmd            */
#define  SET_SPKR_VOL_LOW     0x4100    /* Set speaker volume low            */
#define  SET_SPKR_VOL_MED     0x4200    /* Set speaker volume medium         */
#define  SET_SPKR_VOL_HI      0x4300    /* Set speaker volume high           */

#define  TERMINATE_SPKR       0x4400    /* Terminate speaker and reset dur   */

#define  SET_1_KEY_SEQ        0x5100    /* Set scan code  sys atn 1  key seq */
#define  SET_2_KEY_SEQ        0x5200    /* Set scan code  sys atn 2  key seq */
#define  SET_3_KEY_SEQ        0x5300    /* Set scan code  sys atn 3  key seq */

#define  SOFT_RESET           0x6000    /* Execute 8051 soft reset           */
#define  FORCE_SYS_ATN_INT    0x6200    /* Force system attention interrupt  */

#define  DIAG_KB_TAB_PORT     0x7000    /* bit 0 = tablet RxD pin            */
                                        /* bit 2 = keybd  clock in pin       */
                                        /* bit 5 = keybd  data in pin        */

#define  DUMP_SHRAM_00_0F     0x8000    /* dump shared ram 0x00->0x0F        */
#define  DUMP_SHRAM_10_1F     0x8100    /* dump shared ram 0x10->0x1F        */
#define  DUMP_RAS_RESET       0x8200    /* dump ras logs and reset counters  */
#define  DUMP_RAS             0x8300    /* dump ras logs - do not reset      */

#define  INIT_8051            0x9000    /* restore initial conditions        */

#define  READ_REL_MKR_V1      0xE000    /* version # (1st character)         */
#define  READ_REL_MKR_V2      0xE100    /* version # (2nd character)         */
#define  READ_REL_MKR_M1      0xE200    /* month #   (1st character)         */
#define  READ_REL_MKR_M2      0xE300    /* month #   (2nd character)         */
#define  READ_REL_MKR_D1      0xE400    /* day #     (1st character)         */
#define  READ_REL_MKR_D2      0xE500    /* day #     (2nd character)         */
#define  READ_REL_MKR_Y1      0xE600    /* year #    (1st character)         */
#define  READ_REL_MKR_Y2      0xE700    /* year #    (2nd character)         */
#define  READ_REL_MKR_S1      0xE800    /* unique serial # (1st character)   */
#define  READ_REL_MKR_S2      0xE900    /* unique serial # (2nd character)   */
#define  READ_REL_MKR_S3      0xEA00    /* unique serial # (3rd character)   */
#define  READ_REL_MKR_S4      0xEB00    /* unique serial # (4th character)   */
#define  READ_REL_MKR_K1      0xEC00    /* check sum on sn # (1st character) */
#define  READ_REL_MKR_K2      0xED00    /* check sum on sn # (2nd character) */
#define  READ_REL_MKR_Z1      0xEE00    /* chk sum on ROS (1st character)    */
#define  READ_REL_MKR_Z2      0xEF00    /* chk sum on ROS (2nd character)    */

#define  NO_OP_COMMAND        0xF000    /* no-op command                     */

/* 8255 configuration parameters (port 0x57)                                 */
#define  CNFGDATA             0xC3      /* to configure 8255    (cnfg8255)   */
#define  ENABLE_INT           0x09      /* to enable interrupts (enable_irq) */

/*---------------------------------------------------------------------------*/
/* 8051 BAT return codes                                                     */
/*---------------------------------------------------------------------------*/
/* 8051 BAT return codes                                                     */
#define  GOODMACHINE          0xAE      /* hardware ok return value          */
#define  RAMVERFY_RST         0x2E      /* RAM verify reset value            */
#define  INVRTN               0xFF      /* invalid return value              */
#define  UARTPOI_1            0xFF

/*---------------------------------------------------------------------------*/
/* Information Interrupt - information codes                                 */
/*---------------------------------------------------------------------------*/

/* Acknowledge Information Codes                                             */
#define  HOST_CMD_ACK             0x00  /* host command acknowledgement      */
#define  SPKR_STARTED             0x01  /* speaker started                   */
#define  SPKR_INACTIVE            0x02  /* speaker inactive                  */
#define  SPKR_TERMINATED          0x03  /* speaker terminated                */
#define  SPKR_PARMS_QUEUED        0x04  /* speaker parameters queued         */

/* Command Reject Information Code                                           */
#define  REJ_KBD_TRANS_BUSY       0x41  /* write keyboard operation          */
#define  REJ_KLCK_ACTIVE          0x42  /* write keyboard operation          */
#define  REJ_KBD_DISABLED         0x43  /* write keyboard operation          */
#define  REJ_INV_KBD_DATA         0x44  /* write keyboard operation          */
#define  REJ_INV_SPKR_DUR         0x47  /* write speaker operation           */
#define  REJ_INV_SPKR_FREQ        0x48  /* start or write speaker oper       */
#define  REJ_SPKR_BUSY            0x49  /* start speaker operation           */
#define  REJ_SPKR_QUEUE_FULL      0x4A  /* start speaker operation           */
#define  REJ_UART_DISABLED        0x4B  /* write uart operation              */
#define  REJ_UART_TRANS_BUSY      0x4C  /* write uart operation              */
#define  REJ_INV_BAUD             0x4D  /* set uart baud operation           */
#define  REJ_INV_FRAMING          0x4E  /* init uart framing operation       */
#define  REJ_INV_COUNT            0x50  /* set sequence A length oper        */
#define  REJ_ILLEGAL_MODE         0x51  /* diagnose or diag. sense           */
#define  REJ_RAM_QUEUE_BUSY       0x60  /* dump ram block operation          */
#define  REJ_UNDEFINED_OP         0x7F  /* undefined operation               */

/* Status Report Information Code                                            */
#define  STATUS_RPT_ID_MASK       0x80  /* status report identifier          */
#define  SPKR_TONE_COMP_MASK      0x40  /* tone complete (condition by M3=1) */
#define  KBD_RET_ACK_MASK         0x20  /* kbd retn ACK  (condition by M0=1) */
#define  INV_SPKR_PARAM_MASK      0x08  /* invalid speaker parameter         */
#define  UART_XMIT_COMP_MASK      0x04  /* tab xmit complete (cond by M1=1)  */
#define  RAS_LOG_NEAR_OFLO_MASK   0x02  /* RAS log near overflow             */
#define  RAS_LOG_OFLO_MASK        0x01  /* RAS log overflowed                */


/*---------------------------------------------------------------------------*/
/* Error Condition Interrupt - error codes                                   */
/*---------------------------------------------------------------------------*/

/* abort codes                                                               */
#define  DIAG_SOFT_RESET          0xA0  /* Diagnose initiated 8051 reset     */
#define  WORKQ_LOW_DECODE         0xA1  /* workq low decode                  */
#define  HOST_TRANS_QUEUE_DECODE  0xA3  /* host transmit queue decode        */
#define  INCR_RAS_LOG_DECODE      0xA4  /* increment ras log decode          */
#define  WILD_BRANCH              0xA6  /* wild branch                       */
#define  SYSTEM_RESET_FAILED      0xA7  /* system reset failed               */

/* device error codes                                                        */
#define  KBD_TRANS_TIMEOUT        0xE0  /* keyboard xmit timeout             */
#define  KBD_REC_TIMEOUT          0xE1  /* keyboard recieve timeout          */
#define  KBD_ACK_NOT_REC          0xE2  /* keyboard ack not recieved         */
#define  UNEXPECT_KBD_ACK_REC     0xE3  /* unexpected keyboard ack recieved  */
#define  HARD_ERR_ON_KBD_FR_REC   0xE4  /* hard err on kbd frame recieved    */
#define  HARD_ERR_ON_KBD_FR_TRANS 0xE5  /* hard err on kbd frame transmit    */
#define  KBD_CLCK_PIN_NOT_PLUS    0xE6  /* keyboard clock pin not +          */
#define  KBD_CLCK_PIN_NOT_MINUS   0xE7  /* keyboard clock pin not -          */
#define  UART_INTR_WO_TIRI        0xE8  /* tablet interrupt w/o xmit/rcv id  */
#define  UART_TRANS_TIMEOUT       0xE9  /* tablet transmit timeout           */
#define  UART_ACK_TIMEOUT         0xEA  /* tablet ack timeout                */

/*---------------------------------------------------------------------------*/
/* keyboard commands                                                         */
/*---------------------------------------------------------------------------*/

#define  KBD_RESET_CMD            0xFF01      /* Reset command               */
#define  KBD_RESEND_CMD           0xFE01      /* Resend command              */
#define  SET_MAKE_CMD             0xFD01      /* Set key to make only        */
#define  SET_MAKE_BREAK_CMD       0xFC01      /* Set key to make/break only  */
#define  SET_TYPAMATIC_CMD        0xFB01      /* Set key to typamatic only   */
#define  SET_ALL_TYPA_MK_BRK_CMD  0xFA01      /* Set all keys typa/make/brk  */
#define  SET_DBL_RATE_TMATIC_CMD  0xFA01      /* Set dbl rate (106 only)     */
#define  SET_ALL_MAKE_CMD         0xF901      /* Set all keys make only      */
#define  SET_ALL_MAKE_BRK_CMD     0xF801      /* Set all keys make/brk only  */
#define  SET_ALL_TYPAMATIC_CMD    0xF701      /* Set all keys typamatic only */
#define  SET_DEFAULT_CMD          0xF601      /* Set default conditions      */
#define  DEFAULT_DISABLE_CMD      0xF501      /* Disable default conditions  */
#define  KBD_ENABLE_CMD           0xF401      /* Enable keyboard output      */
#define  SET_RATE_DELAY_CMD       0xF301      /* Set typamtic rates & delay  */
#define  READ_ID_CMD              0xF201      /* Read keyboard id            */
#define  UNSUPORTED_CMD           0xF101      /* NO GOOD ON R2 KEYBOARD      */
#define  SELECT_SCAN_CODE         0xF001      /* Select other scan code set  */
#define  LAYOUT_ID_CMD            0xEF01      /* Read layout id              */
#define  ECHO_CMD                 0xEE01      /* Echo command                */
#define  SET_LED_CMD              0xED01      /* Set LED command             */

#define  SET_ALL_LEDS_OFF         0x0001      /* data to set all leds off    */
#define  SET_500MS_DELAY          0x2B01      /* data to set delay at 500 ms */
#define  SET_300MS_DELAY          0x0B01      /* data to set delay at 300 ms */
#define  SET_400MS_DELAY          0x2B01      /* data to set delay at 400 ms */
#define  SELECT_SCAN_CODE_3       0x0301      /* data to select sc-code set 3*/

#define  SET_ACTION_SCAN_CODE     0x5801      /* data to set action key      */
#define  SET_R_ALT_SCAN_CODE      0x3901      /* data to set right alt key   */
#define  SET_L_ALT_SCAN_CODE      0x1901      /* data to set left alt key    */
#define  SET_L_SHIFT_SCAN_CODE    0x1201      /* data to set left shift key  */
#define  SET_R_SHIFT_SCAN_CODE    0x5901      /* data to set right shift     */
#define  SET_CONTROL_SCAN_CODE    0x1101      /* data to set control key     */
#define  SET_CAPS_LOCK_SCAN_CODE  0x1401      /* data to set caps lock key   */
#define  SET_NUM_LOCK_SCAN_CODE   0x7601      /* data to set num lock key    */
#define  SET_SCRLLOCK_SCAN_CODE   0x5F01      /* data to set scrl lock key   */
#define  SET_106K_1_SCAN_CODE     0x0E01      /* data to set keypos 1 key    */
#define  SET_106K_131_SCAN_CODE   0x2001      /* data to set alpha key       */
#define  SET_106K_132_SCAN_CODE   0x2801      /* data to set keypos 132 key  */
#define  SET_KATA_KEY_SCAN_CODE   0x2801      /* data to set katakana key    */
#define  SET_HIRA_KEY_SCAN_CODE   0x3001      /* data to set hiragana key    */
#define  SET_L_ARROW_SCAN_CODE    0x6101      /* data to set left arrow key  */
#define  SET_R_ARROW_SCAN_CODE    0x6A01      /* data to set right arrow key */
#define  SET_U_ARROW_SCAN_CODE    0x6301      /* data to set up arrow key    */
#define  SET_D_ARROW_SCAN_CODE    0x6001      /* data to set down arrow key  */

/*****************************************************************************/
/* Keyboard Defines - RSC Platform                                           */
/*****************************************************************************/

/* POS register 2 bits                                                       */
#define  RESET_KEYBOARD         0x04

/* status/command port (0x51)                                                */
#define  KEYERROR               0x80    /* 1 = parity or framing error       */
#define  RX_FULL                0x40    /* 1 = recieve buffer full           */
#define  TX_EMPTY               0x20    /* 1 = transmit buffer empty         */
#define  KYBD_DATA              0x10    /* status of device data             */
#define  KYBD_CLOCK             0x08    /* status of device clock            */
#define  LOOP_MODE              0x04    /* 1 = set loop mode on (diagnostic) */
#define  KYBD_RESET             0x02    /* 1 = reset keyboard controller     */
#define  ENABLE_KYBD            0x01    /* 0 = enable, 1 = disable keyboard  */

/* speaker port low  (0x55)                                                  */
#define ENABLE_SPKR             0x80    /* bit    7: 1=enable 0=disable spkr */
#define SPEAKER_OFF             0x00    /* bits 6&5: 00 = off                */
#define SPEAKER_LOW             0x20    /* bits 6&5: 01 = low                */
#define SPEAKER_MED             0x40    /* bits 6&5: 10 = medium             */
#define SPEAKER_HI              0x60    /* bits 6&5: 11 = high               */
                                        /* bits 0-4: freq (low bits)         */

/* keyboard commands                                                         */
#define  K_RESET_CMD            0xFF    /* Reset command                     */
#define  K_RESEND_CMD           0xFE    /* Resend command                    */
#define  MAKE_CMD               0xFD    /* Set key to make only              */
#define  MAKE_BREAK_CMD         0xFC    /* Set key to make/break only        */
#define  TYPAMATIC_CMD          0xFB    /* Set key to typamatic only         */
#define  ALL_TYPA_MK_BRK_CMD    0xFA    /* Set all keys typa/make/brk        */
#define  DBL_RATE_TMATIC_CMD    0xFA    /* Set dbl rate (106 only)           */
#define  ALL_MAKE_CMD           0xF9    /* Set all keys make only            */
#define  ALL_MAKE_BRK_CMD       0xF8    /* Set all keys make/brk only        */
#define  ALL_TYPAMATIC_CMD      0xF7    /* Set all keys typamatic only       */
#define  DEFAULT_CMD            0xF6    /* Set default conditions            */
#define  DEF_DISABLE_CMD        0xF5    /* Disable default conditions        */
#define  K_ENABLE_CMD           0xF4    /* Enable keyboard output            */
#define  RATE_DELAY_CMD         0xF3    /* Set typamtic rates & delay        */
#define  READID_CMD             0xF2    /* Read ID command                   */
#define  NON_CMD                0xF1    /* NO GOOD ON R2 KEYBOARD            */
#define  SELECT_SCANCODE        0xF0    /* Select other scan code set        */
#define  LAY_OUT_ID_CMD         0xEF    /* Read Layout id                    */
#define  K_ECHO_CMD             0xEE    /* Echo command                      */
#define  LED_CMD                0xED    /* Set LED command                   */

/*****************************************************************************/
/* Tablet Defines - RSC Platform                                             */
/*****************************************************************************/

/* POS register 2 bits                                                       */
#define RESET_TABLET            0x20

/* 16550 UART registers                                                      */
/* ...Line Control registers bit definitions  (port 0x73)                    */
#define CHAR_LEN_MASK           0x03    /* char lent bits mask               */
#define SET_5_BITS              0x00    /* char length generators : bits 0,1 */
#define SET_6_BITS              0x01
#define SET_7_BITS              0x02
#define SET_8_BITS              0x03
#define SET_STOP_BIT            0x04    /* stop bit generator :   bit 2      */
#define SET_ENABLE_PARITY       0x08    /* enable parity :        bit 3      */
#define SET_EVEN_PARITY         0x10    /* even parity set :      bit 4      */
#define SET_STICKY_PARITY       0x20    /* stikcy parity :        bit 5      */
#define SET_BREAK_CONTROL       0x40    /* break control :        bit 6      */
#define SET_DIVLAT_ACC          0x80    /* divisor latch access : bit 7      */

/* ...Line Status Regigster bit definitions (port 0x75)                      */
#define IS_DATA_READY           0x01    /* data ready  bit                   */
#define IS_BIT_OVERRUN          0x02    /* bit overrun error                 */
#define IS_PARITY_ERROR         0x04    /* parity error odd or even          */
#define IS_FRAMING_ERROR        0x08    /* no valid stop bit                 */
#define IS_BREAK_INTERRUPT      0x10    /* received data is held in Spacing ?*/
#define IS_TXHOLD_EMPTY         0x20    /* transmit hold register empty.     */
#define IS_TX_EMPTY             0x40    /* transmitter empty                 */
#define IS_FIFO_ERROR           0x80    /* parity/frame/break error in fifo  */

/* ...Interrupt Register Bit definitions (port 0x72 - write)                 */
#define SET_RCV_DATA_AVAIL_INT  0x01    /* receive data available interrupt  */
#define SET_TX_HOLD_REG_EMT_INT 0x02    /* tranmit hold register empty       */
#define SET_RCV_LINE_STAT_INT   0x04    /* recieve line status int           */
#define SET_MODEM_STAT_INT      0x08    /* modem status int                  */

/* ...Interupt Identifier Register definitions (port 0x72 - read)            */
#define INT_ID_MASK             0x0F    /* bits 4,5,6,7                      */
#define IS_INT                  0x01    /* yes/no interrupt identified       */
#define IS_RCV_LINE_STAT_INT    0x06    /* bits 1 & 2 set                    */
#define IS_RCV_DATA_AVAIL_INT   0x04    /* receive data available interrupt  */
#define IS_TX_HOLD_REG_EMT_INT  0x02    /* tranmit hold register empty       */
#define IS_CHAR_TIME_OUT_INT    0x0C    /* character timeout indication      */
#define IS_MODEM_STAT_INT       0x00    /* modem status int                  */

/* Line Control Parameters for 16550                                         */
#define TAB_LINE_CONTROL        0x0B    /* BAUD rate for 16550               */
#define  BAUD_RATE_9600         52      /* set baud rate of  9600 bits/sec   */

/*****************************************************************************/
/* Mouse Defines                                                             */
/*****************************************************************************/

/* POS register 2 bits                                                       */
#define  RESET_MOUSE   0x80

/* adapter command register (port 0x48)                                      */
/*       MOUSE_CMD     0xCC00    mouse command CC=command                    */
#define  ADPTWRAP      0x0001 /* diagnostic wrap DD=test data                */
#define  M_ENABLE_INT  0x0002 /* interrupt enabled after status reg is read  */
#define  M_DISABLE_INT 0x0003 /* interrupt disabled                          */
#define  EN_BLK_MODE   0x0004 /* set adapter in block mode                   */
#define  DIS_BLK_MODE  0x0005 /* exits adapter from block mode               */

/* Adapter status register (port 0x0x4a)                                     */
#define             BLK_DIS_RCV             0x04  /* _____1__ block dis rcvd */
#define             BLK_MODE_EN             0x02  /* ______1_ blk mode enabl */
#define             INT_ENABLE              0x01  /* _______1 interrupt enab */

/* Mouse receive registers (4 byte PIO)                                      */
/* ...byte 0 - receive status register (port 0x4c)                           */
#define             M_DATA_0          0xFF000000

#define             REG_3_FULL        0x20000000  /* __1_____ register 3 full*/
#define             REG_2_FULL        0x10000000  /* ___1____ register 2 full*/
#define             REG_1_FULL        0x08000000  /* ____1___ register 1 full*/
#define             M_FACE_ERR        0x04000000  /* _____1__ interface err  */
#define             M_DATA_ERR        0x02000000  /* ______1_ data error     */
#define             M_INTERRUPT       0x01000000  /* _______1 interrupt      */

/* ...byte 1 - receive data register 1 (port 0x4d)                           */
#define             M_DATA_1          0x00FF0000  /* mouse report frame 0    */

/* ...byte 2 - receive data register 2 (port 0x4e)                           */
#define             M_DATA_2          0x0000FF00  /* mouse report frame 1    */

/* ...byte 3 - receive data register 3 (port 0x4f)                           */
#define             M_DATA_3          0x000000FF  /* mouse report frame 2    */

/*---------------------------------------------------------------------------*/
/* adapter status register masks                                             */
/*---------------------------------------------------------------------------*/

                                    /* Adapter status register to be read    */
                                    /* after issuing one of the following    */
                                    /* commands:                             */
                                    /* ENABLE_INT, DISABLE_INT               */
                                    /* EN_BLK_MODE, DIS_BLK_MODE             */

#define BLOCK_DISABLE_MASK 0x06     /* masks for adapter status register     */
#define   CMD_NOT_RCVD     0x00     /* 0  = command not recieved, resend     */
#define   CMD_RCVD_BUSY    0x02     /* 2  = cmd rcvd/adapter busy            */
#define   CMD_RCVD_BLK_DIS 0x04     /* 4  = command recieved, block disable  */

#define BLOCK_MODE_EN_MASK 0x02     /* 0= cmd not rcvd,resend 2= cmd rcvd    */
#define    BLK_MOD_EN_RCVD 0x02

#define INTERRUPT_EN_MASK  0x01     /* If cmd was ENABLE_INT then:           */
                                    /* 0= command not received, resend       */
                                    /* 1= command recieved, interrupt enable */
                                    /*    after reading status register      */
                                    /* If cmd was DISABLE_INT then:          */
                                    /* 1= command not received, resend       */
                                    /* 0= command recieved, interrupt enable */
                                    /*    after reading status register      */


/*---------------------------------------------------------------------------*/
/* mouse commands                                                            */
/*---------------------------------------------------------------------------*/

#define  M_RESET         0xFF00     /* Reset mouse                           */
#define  M_RESEND        0xFE00     /* Resend previous byte(s) of packet     */
#define  M_SET_DEFAULT   0xF600     /* Set conditions to default state       */
#define  M_DISABLE       0xF500     /* Disable mouse                         */
#define  M_ENABLE        0xF400     /* Enable mouse                          */

#define  M_SET_SAM_RATE  0xF300     /* Set sampling rate (stream mode only)  */
#define     M_RATE_10    0x0A00     /* Sampling rate of 10                   */
#define     M_RATE_20    0x1400     /* Sampling rate of 20                   */
#define     M_RATE_40    0x2800     /* Sampling rate of 40                   */
#define     M_RATE_60    0x3C00     /* Sampling rate of 60                   */
#define     M_RATE_80    0x5000     /* Sampling rate of 80                   */
#define     M_RATE_100   0x6400     /* Sampling rate of 100                  */
#define     M_RATE_200   0xC800     /* Sampling rate of 200                  */

#define  M_READ_DEV_TYP  0xF200     /* Read device type                      */
#define  M_SET_REM_MODE  0xF000     /* Set remote mode                       */
#define  M_SET_WRP_MODE  0xEE00     /* Set wrap mode                         */
#define  M_RES_WRP_MODE  0xEC00     /* Reset wrap mode                       */
#define  M_READ_DATA     0xEB00     /* Read data                             */
#define  M_SET_STM_MODE  0xEA00     /* Set stream mode                       */
#define  M_STATUS_REQ    0xE900     /* Status request                        */

#define  M_SET_RES       0xE800     /* Set resolution                        */
#define     M_RES_1      0x0000     /* 1 count  per mm                       */
#define     M_RES_3      0x0100     /* 3 counts per mm                       */
#define     M_RES_6      0x0200     /* 6 counts per mm                       */
#define     M_RES_12     0x0300     /* 12 counts per mm                      */

#define  M_SET_SCL_2_1   0xE700     /* Set scaling to 2:1 (stream mode only) */
#define  M_RESET_SCALE   0xE600     /* Reset scaling to 1:1                  */

#define  M_NULL          0x0000     /* null                                  */


/*---------------------------------------------------------------------------*/
/* mouse response (4 byte read)                                              */
/*---------------------------------------------------------------------------*/

#define  M_ACK           0x00FA0000 /* response from mouse after valid input */
#define  M_INV_CMD_RC    0x00FE0000 /* response for invalid command          */
#define  M_INV_CMDS_RC   0x00FC0000 /* response for invalid commands         */

#define  M_BAT_CC        0xAA       /* response for succesful BAT test       */
#define  M_IBM_2_BUTTON  0x00       /* id for ibm 2 button mouse             */
#define  M_LOG_2_BUTTON  0x02       /* id for logitech 2 button mouse        */
#define  M_LOG_3_BUTTON  0x03       /* id for logitech 3 button mouse        */
#define  M_DEFAULT_ID    0x00       /* default id send after reset command   */

/* mouse data report                                                         */
#define             M_Y_DATA_OVERFLOW 0x00800000  /* 1= overflow             */
#define             M_X_DATA_OVERFLOW 0x00400000  /* 1= overflow             */
#define             M_Y_DATA_SIGN     0x00200000  /* 1= negative             */
#define             M_X_DATA_SIGN     0x00100000  /* 1= negative             */
#define             M_C_BUTTON_STATUS 0x00040000  /* 1= down; center button  */
#define             M_R_BUTTON_STATUS 0x00020000  /* 1= down; right button   */
#define             M_L_BUTTON_STATUS 0x00010000  /* 1= down; left  button   */

/* mouse status request                                                      */
#define             MSR_STR_REM_MODE  0x00400000  /* stream =0; remote=1     */
#define             MSR_DIS_ENABLE    0x00200000  /* disable=0; enable=1     */
#define             MSR_SCALING       0x00100000  /* 1:1    =0; 2:1   =1     */
#define             MSR_LEFT_BUTTON   0x00040000  /* up     =0; down  =1     */
#define             MSR_CENTER_BUTTON 0x00020000  /* up     =0; down  =1     */

/*****************************************************************************/
/* delay timings                                                             */
/*****************************************************************************/

/* watchdog timers values                                                    */
#define WDOGTIME            20000000  /* 20ms no response timeout            */
#define RSTWD               500000000 /* 500ms reset timeout                 */
#define TABRST              500000000 /* 500ms tablet reset time             */
#define TABFRAME            10000000  /* 10ms tablet frame delay time        */

/* ktsm_sleep timer values                                                   */
#define FRAME_WAIT          200000    /* 200ms I/O wait loop sleep time      */
#define ADPT_RST_WAIT       120000    /* 120ms adapter reset sleep time      */
#define RESETTO             700000    /* 700ms RESET timeout                 */
#define RCVTO               30000     /* 30ms timeout waiting for RCV frames */
#define DMBSY               1500      /* 1.5 ms timeout waiting for mouse    */
                                      /*   adapter to drop busy              */

#define KPOLL_TO            30        /* 30sec keep alive poll ACK timeout   */

/*****************************************************************************/
/* error log ID's                                                            */
/*****************************************************************************/

                                      /* error log resource names            */
#define RES_NAME_KTS        "KTSDD"
#define RES_NAME_TAB        "TABLETDD"
#define RES_NAME_KBD        "KBDDD"
#define RES_NAME_MSE        "MOUSEDD"

#define PIO_ERROR           1         /* permanent PIO error                 */
#define RCV_ERROR           2         /* receive errors have exceeded        */
                                      /*   threshold                         */
#define KBDBAT_ERROR        3         /* invalid keyboard BAT                */
#define KBDID_ERROR         4         /* invalid keyboard ID                 */
#define XMEMCPY_ERROR       5         /* cross memory copy failure           */
#define KIO_ERROR           6         /* keyboard  I/O error                 */

#define MDDBAT_ERROR        7         /* invalid mouse BAT                   */
#define MDDID_ERROR         8         /* invalid mouse ID                    */
#define MDDTYPE_ERROR       9         /* invalid type of mouse               */
#define MIO_ERROR           10        /* mouse I/O error                     */

#define ADPT_ERROR          11        /* invalid 8051 adapter status         */
#define ABAT_ERROR          12        /* 8051 adapter BAT failure            */

#define TAB_CFG_ERROR       13        /* Tablet Read Configuration Failure   */
#define TABTYPE_ERROR       14        /* Tablet Read Configuration Failure   */
#define TIO_ERROR           15        /* tablet I/O error                    */

/*****************************************************************************/
/* Non scan code responses from keyboard                                     */
/*****************************************************************************/

#define  OVERRUN                  0x00  /* overrun condition                 */
#define  OVERRUN_2                0xFF  /* overrun condition                 */
#define  KBD_ACK                  0xFA  /* keyboard acknowledge              */
#define  BREAK_CODE               0xF0  /* break code                        */
#define  KBD_ECHO                 0xEE  /* keyboard echo                     */
#define  KBD_BAT_CC               0xAA  /* keyboard BAT completion code      */
#define  KBD_RESEND               0xFE  /* keyboard resend                   */

#define  KBD_POR_ID_1             0x83  /* keyboard POR id byte #1           */
#define  KBD_POR_ID_1B            0x84  /* keyboard POR id byte #1           */
#define  KBD_POR_ID_1C            0xBF  /* keyboard POR id byte #1           */
#define  KBD_POR_ID_2A            0xB0  /* keyboard POR id byte #2 (101 kbd) */
#define  KBD_POR_ID_2B            0xB1  /* keyboard POR id byte #2 (102 kbd) */
#define  KBD_POR_ID_2C            0xB2  /* keyboard POR id byte #2 (106 kbd) */
#define  KBD_POR_ID_2D            0xAB  /* keyboard POR id byte #2           */


/*****************************************************************************/
/* Defines for misc key scan codes                                           */
/*****************************************************************************/

#define  ACTION_SCAN_CODE         0x58        /* data to set action key      */
#define  R_ALT_SCAN_CODE          0x39        /* data to set right alt key   */
#define  L_ALT_SCAN_CODE          0x19        /* data to set left alt key    */
#define  L_SHIFT_SCAN_CODE        0x12        /* data to set left shift key  */
#define  R_SHIFT_SCAN_CODE        0x59        /* data to set right shift     */
#define  CONTROL_SCAN_CODE        0x11        /* data to set control key     */
#define  CAPS_LOCK_SCAN_CODE      0x14        /* data to set caps lock key   */
#define  NUM_LOCK_SCAN_CODE       0x76        /* data to set num lock key    */
#define  K106_1_SCAN_CODE         0x0E        /* data to set keypos 1 key    */
#define  K106_131_SCAN_CODE       0x20        /* data to set alpha key       */
#define  K106_132_SCAN_CODE       0x28        /* data to set keypos 132 key  */
#define  KATA_KEY_SCAN_CODE       0x28        /* data to set katakana key    */
#define  HIRA_KEY_SCAN_CODE       0x30        /* data to set hiragana key    */
#define  L_ARROW_SCAN_CODE        0x61        /* data to set left arrow key  */
#define  R_ARROW_SCAN_CODE        0x6A        /* data to set right arrow key */
#define  U_ARROW_SCAN_CODE        0x63        /* data to set up arrow key    */
#define  D_ARROW_SCAN_CODE        0x60        /* data to set down arrow key  */

#define  LARGEST_SCAN_CODE        0x84        /* largest possible scan code  */

/*****************************************************************************/
/* Defines for misc position codes                                           */
/*****************************************************************************/

#define  L_SHIFT_POS_CODE         44
#define  R_SHIFT_POS_CODE         57

#define  L_ALT_POS_CODE           60
#define  R_ALT_POS_CODE           62

#define  CNTL_POS_CODE            58

#define  ACTION_POS_CODE          64

#define  CAPS_LOCK_POS_CODE       30
#define  NUM_LOCK_POS_CODE        90

#define  ALPHA_POS_CODE           60
#define  KATAKANA_POS_CODE        30
#define  HIRAGANA_POS_CODE        133

#define  R_CURSOR_POS_CODE        89
#define  L_CURSOR_POS_CODE        79
#define  U_CURSOR_POS_CODE        83
#define  D_CURSOR_POS_CODE        84

#define  NUMPAD0                  99
#define  NUMPAD1                  93
#define  NUMPAD2                  98
#define  NUMPAD3                 103
#define  NUMPAD4                  92
#define  NUMPAD5                  97
#define  NUMPAD6                 102
#define  NUMPAD7                  91
#define  NUMPAD8                  96
#define  NUMPAD9                 101

#define  BACKSPACE                15

#define  SAK_KEY_0                58    /* ctrl key pos code for SAK         */
#define  SAK_KEY_0_SC             0x11  /* ctrl key scan code for SAK        */
#define  SAK_KEY_1                47    /* "x" key pos  code for SAK         */
#define  SAK_KEY_1_SC             0x22  /* "x" key scan code for SAK         */
#define  SAK_KEY_2                20    /* "r" key pos  code for SAK         */

/*****************************************************************************/
/* Miscellaneous definitions                                                 */
/*****************************************************************************/

                                       /* device type for devinfo sturct     */
#define INPUTDEV               DD_INPUT

#define M_RETRY                3       /* max mouse operation retries        */
#define K_RETRY                3       /* max keyboard operation retries     */
#define T_RETRY                3       /* max tablet operation retries       */
#define R_RETRY                3       /* max receive (get_iq()) retries     */

#define RCV_ERR_THRESHOLD      50      /* receive error threshold            */

#define  SCAN_CODE_3           0x03    /* data to select sc-code set 3       */

#define  MIN_TYPA_RATE         2       /* minimum typamatic rate             */
#define  MAX_TYPA_RATE         30      /* maximum typamatic rate             */
#define  DEF_TYPA_RATE         20      /* default typamatic rate             */
#define  DEF_DELAY             500     /* default typamatic delay            */

                                       /* keyboard led cmd parms             */
#define  CAPSLOCK_LED          4       /*   Capslock led                     */
#define  NUMLOCK_LED           2       /*   Numlock led                      */
#define  SCRLLOCK_LED          1       /*   Scroll lock led                  */
#define  CAPSLOCK_LED_106      2       /*   Capslock led - 106 key keyboard  */
#define  NUMLOCK_LED_106       4       /*   Numlock led - 106 key keyboard   */
#define  ALL_LEDS_OFF          0       /*   all leds off                     */

                                       /* keyboard rate/delay cmd parms      */
#define  DELAY_1000MS          0x60    /* data to set delay at 1000 ms       */
#define  DELAY_750MS           0x40    /* data to set delay at 750 ms        */
#define  DELAY_500MS           0x20    /* data to set delay at 500 ms        */
#define  DELAY_250MS           0x00    /* data to set delay at 250 ms        */

#define KOREAN_MAP             1       /* dds map parameter for Korean       */
#define JAPANESE_MAP           2       /* dds map parameter for Japanese     */
#define CHINESE_MAP            3       /* dds map parameter for Chinese      */

#define NO_NUM_PAD             1       /* dds type parameter for non 10key kb*/

#define SOUND_Q_SIZE           64      /* size of sound queue (num elements) */
#define MAX_ALARM_DURATION     32767
#define MAX_ALARM_FREQ         12000
#define MIN_ALARM_FREQ         32

#define NOVOLUME               0xff    /* adapter volume not set             */

#define  MIN_MOUSE_THRESHOLD       0
#define  MAX_MOUSE_THRESHOLD       32767

#define  DEFAULT_THRESHOLD         22
#define  DEFAULT_RESOLUTION        MRES3
#define  DEFAULT_SAMPLE_RATE       MSR100
#define  DEFAULT_SCALING           MSCALE11

#define NO_PARM   0xffff               /* no command parameter               */

#define  TBM21                     5
#define  TBM22                     6

#define  DEFAULT_TAB_RESOLUTION_H  0x6403
#define  DEFAULT_TAB_RESOLUTION_L  0x0003

#define  DEFAULT_TAB_ORIGIN        0x0003
#define  DFT_TAB_ORIGIN            TABORGLL

#define  DFT_TAB_CONVERSION        TABINCH

#define  DEFAULT_TAB_SAMPLE_RATE   0x0103

#define  TABCH                     0      /* tablet channel number           */

/*****************************************************************************/
/* input ring control block structure                                        */
/*****************************************************************************/
struct rcb {
   pid_t   owner_pid;                  /* process ID of ring's owner         */
#define KERNEL_PID   -1                /*   kernel process owns channel      */

   caddr_t ring_ptr;                   /* pointer to input ring              */
                                       /*   (NULL if no ring registered)     */
   int     ring_size;                  /* ring size                          */
   struct  xmem  xmdb;                 /* cross memory descriptor block      */
   uchar   rpt_id;                     /* report source identifier           */
};

