/* static char sccsid[] = "@(#)25  1.4  src/bos/diag/tu/tablet/tu_type.h, tu_tab, bos411, 9428A410j 5/12/94 15:13:44"; */
/*
 *   COMPONENT_NAME: tu_tab
 *
 *   FUNCTIONS:
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

/***********************************************************************

Header HTX/Mfg. Tablet Definitions File

Module Name :  tu_type.h

Header file contains basic definitions needed by three applications for
tablettesting the  Device:

        1)  Hardware exerciser invoked by HTX,
        2)  Manufacturing application, and
        3)  Diagnostic application.

***********************************************************************/

 /**********************************************************************
 *
 * We include the HTX header file hxihtx.h because we have a need
 * to access this structure within test units in case the invoker
 * is an HTX Hardware Exerciser application (i.e., NOT the
 * manufacturing diagnostic application).  Since, the main driver
 * function, exectu(), has already been defined for use by both
 * types of applications, we will "sneak" in this structure
 * inside the TUTYPE one that we are allowed to define and pass in.
 **********************************************************************/

#ifdef nodiag
#include <hxihtx.h>
#endif

#include <nl_types.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/intr.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/termio.h>

#ifdef DIAGNOSTICS
#include <sys/inputdd.h>
#include "dtablet_msg.h"
#include "diag/diago.h"
#define YES     1
#define NO      2
#define CANCEL_KEY_ENTERED     7
#define EXIT_KEY_ENTERED       8
#endif

 /*
  * definition of constants used by all Test Units
  */

#define REPEAT_COUNT  300
#define WAIT_FOR_ONE    1           /* This is the wait_time in sem.c */
#define POS_SEMKEY      0x3141593   /* This is an arbitrary value and
                                     * denotes the value of pi.+1 */
#define MAX_SEM_RETRIES 3

 /*
  * Standard Structure used by Manufacturing Diagnostics
  */

struct tucb_hdr
   {
        long tu;        /* test unit number   */
        long mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */
        long loop;      /* loop test of tu    */
        long r1;        /* reserved */
        long r2;        /* reserved */
   };

 /*
  * Erros types and error codes for Tablet Device Test Units
  */
#define SYS_ERR     0x00
#define TAB_ERR     0x01
#define LOG_ERR     0x02

 /*
  * Tablet Adapter Registers values
  */
#define IOW_PA_8255     0x0050    /* Write 8255 Port A Output Buffer */
#define IOR_PA_8255     0x0054    /* Read 8255 Port A Input Buffer */
#define IOR_PB_8255     0x0055    /* Read 8255 Port B */
#define IOR_PC_8255     0x0056    /* Read 8255 Port C */
#define IOW_CFG_8255    0x0057    /* Write 8255 Configure
                                     Enable/Disable Keyboard and UART IRQ */
/*
 * Interrupt ID Number
 */

#define ID_INFO_INT     0x08      /* Informational interrupt */
#define ID_BAT_INT      0x06      /* 8051 self-test performed */
#define ID_ERR_INT      0x07      /* 8051 detected an error condition */
#define ID_BYTE_INT     0x01      /* Byte received from keyboard */

 /*
  * POS Register and SRAM address
  */
#define POS_SLOT        15        /* MC device dedicated slot */
#define RESET_REG        2        /* POS register */
#define RST_KBD_8051  0x04        /* bit pattern to reset keyboard */
#define REL_8051_RST  0xFB        /* Initial POS reg. value  */

 /*
  * RETURN CODES
  * Adapter general
  */
#define ADP_COMP_CODE   0xAE   /* Successful adapter completion code */
#define KBD_CMP_CDE     0xAA   /* Successful completion of BAT */
#define KBD_ACK         0xA0   /* Keyboard acknowlegde */

 /*
  * UART Query/Control Command return codes
  */

#define UART_CMD_ACC     0x00       /* command accepted */
#define UART_Q_TIMEOUT   0xEA       /* time out in 25 msecs */

 /*
  * Commands sent to register
  */

#define KBD_ACK_BYT_SET_CMD     0x3000    /* Set SRAM 10 mode bit 0 */
#define INVALID_CMD_ACK         0x7F      /* Ack from an invalid command */
#define CFG_8255_CMD            0xC3      /* configure 8255 command */
#define UART_BLK_INACT          0x2500    /* Reset SRAM 10 mode bit 5 */
#define UART_BLK_ACT            0x3500    /* Set SRAM 10 mode bit 5 */
#define ENBL_UART_IF            0x3C00    /* Set SRAM 11 mode bit 12 */
#define INIT_UART_FRM_O         0x8606    /* Initialize UART framing to ODD 
                                           *    and six bytes report block */
#define TAB_DSBL_CMD            0x0903    /* Disable tablet cmd */
#define TAB_ENBL_CMD            0x0803    /* Enable tablet cmd */
#define TAB_RESET_CMD           0x0103    /* Reset tablet */
#define TAB_RD_CFG_CMD          0x0604    /* Read configuration cmd */
#define TAB_RD_STAT_CMD         0x0B03    /* Read current status/coordinate */
#define TAB_SET_WRAP_MODE_CMD   0x0E03    /* Set wrap mode cmd */
#define TAB_RSET_WRAP_MODE_CMD  0x0F03    /* Reset wrap mode cmd */
#define TAB_SET_CONVER_CMD      0x8303    /* Set convertion mode command */
#define TAB_SET_METRIC_CMD      0x0103    /* Set conversion to metric */
#define SET_RESOLN_CMD84        0x8403    /* Set resolution cmd xx=84 */
#define SET_RESOLN_MSB_CMD65    0x6503    /* MSByte byte of scale factor */
#define SET_RESOLN_CMD85        0x8503    /* Set resolution cmd xx=85 */
#define SET_RESOLN_LSB          0x0003    /* LSByte byte of scale factor */
#define SET_RESOLN_LSB_CMD99    0x9903    /* LSByte byte of scale factor */
#define SET_SAMP_SPEED_CMD      0x8A03    /* Set sampling speed command */
#define SET_DATA_MODE_CMD       0x8D03    /* Set incremental data mode cmd */
#define TAB_WRAP_MODE_TEST_DATA 0x1004    /* Data sent to test wrap mode */
#define TAB_WRAP_TEST_DATA      0x10
#define SAMP_50_SPEED_CMD       0x3203    /* Set sampling speed 50 crd.prs/s */
#define SET_RESOLN_02_MSB       0x0203    /* Set resolution to 10 lines/in */

 /*
  * Read Configuration command return codes
  */

#define KNOWN_TABLET            0x14
#define KNOWN_CURSOR_PAD        0x15

 /* TO CHECK BIT 5 READ PORT C REGISTER */
#define INPUT_BUFF_FULL         0x20

/* TO CHECK BIT 7 OF READ PORT C REGISTER */
#define OUTPUT_BUFF_EMPTY       0x80

/* TO CHECK BIT 3 OF READ PORT B REGISTER FOR STROBE */
#define STROBE_RECD             0x08

/*
 * TABLET ERROR CODES
 */

#define SUCCESS         0
#define FAILURE         -1
#define BAD_TU_NO	1

/*
 * Common error codes from Tablet
 */

#define ADP_HDW_RSET_ERR        0xF004  /* Hardware Reset Error */
#define ADP_FRM_O_ERR           0xF008  /* Error in initializing UART frm odd */
#define DEV_DSBL_ERR            0xF009  /* Error during disabling tablet */

#define DEV_RESL_CMD84_ERR      0xF00E  /* Error in cmd84 of set resoln */
#define DEV_RESL_MSB_ERR        0xF00F  /* Error in MSB of set resoln */
#define DEV_RESL_CMD85_ERR      0xF010  /* Error in cmd85 of set resoln */
#define DEV_RESL_LSB_ERR        0xF011  /* Error in LSB of set resoln */
#define DEV_SAMP_SPD_ERR        0xF012  /* Error in set samp speed */
#define DEV_SMP_SPD_YY_ERR      0xF013  /* Error in yy byte of set samp speed */

#define DEV_ENBL_ERR            0xF018  /* Error in enabling tablet */
#define DEV_RD_ST_ERR           0xF01B  /* Error during read status cmd. */

#ifdef DIAGNOSTICS
#define DEV_DAT_YY_ERR          0xF017  /* Error in yy byte of set data mode */
#define DEV_INCR_YY_ERR         0xF015  /* Error in yy byte of set incr. val */
#define ADP_DSBL_ERR            0xF001  /* Disable kbd and UART IRQ error */
#define DEV_FAIL_STATUS         0xF002  /* Failed status block */
#define DEV_FL_STAT_BCOUNT      0xF003  /* Failed status byte count */
#define ADP_EZ_BLK_ERR          0xF019  /* Error in UART frame to odd and */
                                        /* block to 4 */
#define ADP_BLK_ACT_ERR         0xF01A  /* Error setting UART BLK to active */
#define ADP_HDW_REL_ERR         0xF005  /* Hardware Release Error */
#define ADP_BLK_INACT_ERR       0xF006  /* Error setting UART BLK to inactive */
#define ADP_ENBL_IF_ERR         0xF007  /* Error in enabling UART interf. */
#define DEV_CONV_ERR            0xF00C  /* Error in set conv. */
#define DEV_CONV_YY_ERR         0xF00D  /* Error in yy byte of set conversion */
#define DEV_INCR_ERR            0xF014  /* Error in set increment value */
#define DEV_INCR_YY_ERR         0xF015  /* Error in yy byte of set incr. val */
#define DEV_DAT_ERR             0xF016  /* Error in set data mode */
#define DEV_DAT_YY_ERR          0xF017  /* Error in yy byte of set data mode */
#endif

/* SEMAPHORE ERROR CODES */

#define SYS_SEMID_NEXST          0xF02C    /* Semaphore ID does not exist */
#define SYS_SEMID_NVALID         0xF02D    /* Semaphore ID not valid */
#define SYS_SEMVAL_NVALID        0xF02E    /* Semaphore value not valid */
#define SYS_SPND_SEM_OP          0xF02F    /* Suspend semaphore operation */
#define SYS_PID_NVALID           0xF02B    /* Process ID not valid */


/* TU10 ERRORS */

#define DEV_RESET_ERR           0x1001     /* Unable to reset tablet */
#define DEV_RD_CFG_ERR          0x1002     /* Unable to configure */
#define DEV_UNKNOWN_ERR         0x1005     /* Unknown Dev. Error */

#ifdef DIAGNOSTICS
#define DEV_UNKNOWN_CFG_ERR     0x1006     /* Unknown Dev. Cfg.*/
#define ADP_PA_CFG_ERR          0x1004     /* Error in port A during rd cfg */
#define ADP_PC_CFG_ERR          0x1003     /* Error in port C during rd cfg */
#endif

/* TU20 ERRORS */

#define DEV_SET_WRAP_MDE_ERR    0x2001   /* Unable to set wrap mode */
#define DEV_WRP_MDE_DATA_ERR    0x2002   /* Set wrap mode data error */
#define DEV_WRP_MDE_ERR         0x2005   /* Wrap mode error */
#define DEV_RST_WRP_MDE_ERR     0x2006   /* Unable to reset wrap mode */
#define DEV_RST_WRP_MDE_DTA_ERR 0x2007   /* Reset wrap mode data error */

#ifdef DIAGNOSTICS
#define ADP_PC_RSET_WRAP_ERR    0x2009   /* Error in port C during reset */
					 /* wrap mode */
#define ADP_PA_RSET_WRAP_ERR    0x2008   /* Error in port A during reset */
                                         /* wrap mode */
#define ADP_PC_WRAP_ERR         0x2004   /* Error in port C during wrap mode */
#define ADP_PA_WRAP_ERR         0x2003   /* Error in port A during wrap mode */
#endif

/* TU30 ERRORS */

#define DEV_SET_CONVER_ERR      0x3001   /* Set conversion error */
#define DEV_SET_METRIC_ERR      0x3002   /* Error in setting conv. to metric */
#define DEV_RESLN_BYTE1_ERR     0x3003   /* Error in byte 1 of set resoln. */
#define DEV_RESLN_MSB_ERR       0x3004   /* Error in MSB of set resoln. */
#define DEV_RESLN_BYTE2_ERR     0x3005   /* Error in byte 2 of set resoln. */
#define DEV_RESLN_LSB_ERR       0x3006   /* Error in LSB of set resoln. */

/* TU40 ERRORS */

#define DEV_FAIL_LED_TEST       0x4001  /* Failed LED test */

/* TU50 ERRORS */

#define DEV_FAIL_ACT            0x5001  /* Failed LED active area test */
#define DEV_FAIL_INACT          0x5002  /* Failed LED inactive area test */

/* TU60 ERRORS */

#define DEV_FL_PRES_SWTCH       0x6001  /* Failed when switch pressed */
#define DEV_FL_REL_SWTCH        0x6002  /* Failed when switch released */

/* TU70 ERRORS */

#define DEV_INCR_MDE_ERR        0x7001  /* Error setting tab to incr. mode */
#define DEV_FL_ENBL_TEST        0x7002  /* Failed Enable Test */
#define DEV_FL_DSBL_TEST        0x7003  /* Failed Disable Test */


/**********************************************************************
 
 During a data transfer, the tablet device transmits two status bytes to
 the adapter according to the bit diagram below.  (Cf. the hardware spec
 for more detailed information.)

 +--------+--------+----------+-----------------+-----------+----------+------+
 |  MSB   |    1   |     2    |    3      4     |     5     |     6    |  7   |
 +--------+--------+----------+-----------------+-----------+----------+------+
 |Presence|Dev. ID.|Dev Attch.|Input Dev. Attch.|Convertion.|Resolution|Always|
 +--------+--------+----------+-----------------+-----------+----------+------+
 | 0=out  |   1    |  0=M21   |00 = None        |0 = English|1 = Set   |      |
 | 1=in   | Tablet |  1=M22   |01 = Pen         |1 = Metric |resolution|   1  |
 |        |        |          |10 = 4 but. curso|           |command   |      |
 |        |        |          |11=16 but. cursor|           |received  |      |
 +--------+--------+----------+-----------------+-----------+----------+------+

 NOTE: Presence (in) indicates that the Stylus or cursor is proximate to
       the Tablet work area.
**********************************************************************/

#define VALID_BYTES             0x41      /* bit 1 must be [1] indicating 
                                             that a Tablet device is present 
                                             and bit 7 must be [1] "always" */


int kbdtufd;          /* keyboard device driver file descriptor */

#ifdef DIAGNOSTICS

typedef struct TUCB_SYS
{
    nl_catd  catd; 
    long     ad_mode;
    int      tabtype;
    char     kbd_fd[20];
};

long  msgtab[8];

long   menunums[][8] =
        {
                {
                        0x926401,
                        0x926402,
                        0x926601,
                        0x926602,
                        0x926701,
                        0x926702,
                        0x926121,
                        0x926122
                },
                {
                        0x927401,
                        0x927402,
                        0x927601,
                        0x927602,
                        0x927701,
                        0x927702,
                        0x927121,
                        0x927122
                }
        };


#endif

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */

#define TUTYPE struct _tablet_tu

struct _tablet_tu
{
        struct tucb_hdr header;
	struct TUCB_SYS tuenv;
};
