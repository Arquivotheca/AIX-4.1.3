/* @(#)27       1.1  src/bos/diag/tu/ppckbd/tu_type.h, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:33:23 */
/*
 *   COMPONENT_NAME: tu_ppckbd
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/***********************************************************************

Header HTX/Mfg. Keyboard Definitions File

Module Name :  tu_type.h

Header file contains basic definitions needed by three applications for
testing the  Device:

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
#include "hxihtx.h"
#endif
#include <nl_types.h>
#include <stdio.h>
#include <cur01.h>
#include <cur02.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
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
#include <sys/inputdd.h>

#ifdef DIAGNOSTICS
#include "diag/atu.h"
#include "diag/diago.h"
#include "kbd_msg.h"
#endif

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define INVOKED_BY_HTX   99
#define RULE_LEN      8
#define RETRIES_LEN   3

/*
 * definition of constants used by Test Units 
 */
 
#define REPEAT_COUNT 	    300
#define KEY_POS_CNT  	    133
#define ESC_KEY	     	    110
#define WAIT_FOR_ONE        1           /* This is the wait_time in sem.c */
#define SEMKEY              0x3141593   /* This is an arbitrary value and
                                         * denotes the value of pi. +1 */
#define MAX_SEM_RETRIES 3

/*
 * Erros types and error codes for Keyboard Device Test Units  
 */

#define SYS_ERR     0x00
#define KBD_ERR     0x01
#define LOG_ERR     0x02

/*
 * Keyboard Adapter Registers values 
 */

/*
 * RETURN CODES 
 * Adapter general 
 */

#define KBD_CMP_CDE     0xAA   /* Successful completion of BAT */

/*
 * Keyboard response to Commands
 */ 

#define	KBD_CMD_ACCP	 0x00         /* keyboard command accepted */
#define KBD_1_101_ID     0xAB         /* First byte of Kbd 101/102 ID */
#define KBD_1_106_ID	 0xAB	      /* First byte of Kbd 106 ID */
#define KBD_2_101_ID     0x83         /* Sec. byte of 101/102 kbd ID */
#define KBD_2_106_ID     0x83         /* Sec. byte of 106 Kbd ID */
#define ECHO_DATA	 0xEE         /* Echoed data verification */
#define SCAN_SET3	 0x03         /* Scan code set 3 */

/*
   When bit 5 and bit 0 of Control Status Register are set to 1, then the
   port 0x60 has the data from the mouse. If bit 5 is 0, and bit 0 is 1, then
   port 0x60 has data from keyboard or controller.
*/

#define OUTPUT_BUFF_FULL	0x01 
#define CSR_BIT_0               0x01
#define CSR_BIT_1               0x02
#define CSR_BIT_5               0x20
#define CSR_BIT_6               0x40
#define CSR_BIT_7               0x80

/* The input buffer is writeonly and is at address 0x60 or 0x64.
   The output buffer is readonly and is at address 0x60
*/

#define CMD_PORT        0x64
#define DATA_PORT       0x60
#define KBD_ACK         0xFA

/*
 * Commands sent to keyboard 
 */

#define KBD_RSET_CMD	        0xFF       /* Reset keyboard */ 
#define KBD_ECHO_CMD		0xEE       /* Kbd Echo cmd */
#define RST_MDE_IND_CMD		0xED       /* Reset keyboard indicators */
#define SET_IND_OFF_CMD		0x00       /* Set all indicators off */
#define SET_IND_ON_CMD		0x07       /* Set all indicators on */
#define KBD_ID_CMD		0xF2       /* Keyboard ID */
#define KBD_SCAN_CDE_CMD        0xF0       /* Kbd scan code */
#define KBD_SCAN_SET2_CMD       0x02       /* Kbd scan code set 1 */
#define KBD_SCAN_SET3_CMD       0x03       /* Kbd scan code set 3 */
#define KBD_SCAN_QRY_CMD        0x00       /* Kbd scan query */
#define KBD_ENBL_CMD            0xF4       /* Enable kbd */
#define KEY_TYP_MK_BRK_CMD      0xFA       /* Set keys to
					      * Typamatic Make/Break type */
#define SET_MK_BRK_CMD          0xFC       /* Set key type to make/break */
#define KBD_RESEND_CMD		0xFE	   /* kbd resend command */

/*
 *                     KEYBOARD ERROR CODES 
 */

#define SUCCESS		0
#define	FAILURE		-1
#define BAD_TU_NO       1

/* SEMAPHORE ERROR CODES */

#define SYS_SEMID_NEXST         0xF00B  /* Semaphore ID does not exist */
#define SYS_SEMID_NVALID        0xF00C  /* Semaphore ID not valid */
#define SYS_SEMVAL_NVALID       0xF00D  /* Semaphore value not valid */
#define SYS_SPND_SEM_OP         0xF00E  /* Suspend semaphore operation */
#define SYS_PID_NVALID          0xF00F  /* Process ID not valid */

/* Common Error Codes */

#define FAILED_DD_OPEN          0xF001  /* Failed to open Device Driver in */
                                        /* Diagnostics Mode */
#define KBD_RESET_ERR		0xF002	/* problem in sending reset cmd */
#define KBD_RESTORE_ERR		0xF003	/* unable to resotre kybd */

/* Error codes from TU10 */

#define KBD_LED_ON_ERR		0x1001  /* failed LED on */
#define KBD_LED_OFF_ERR         0x1002  /* failed LED off */

/* Error codes from TU20 */

#define INVALID_ID		0x2001	/* unknown keyboard id */
#define KBD_ECHO_ERR		0x2002	/* problem in echo cmd */
#define SCAN_SET3_ERR		0x2003	/* problem in sending SCAN_SET3 cmd */

/* Error codes from TU30 */

#define FAIL_IND_TEST		0x3001	/* Set/Restore indicator cmd failed */
#define DEV_IND_ON_ERR		0x3001  /* Error in turning ON the LEDs */
#define DEV_FAIL_IND_TEST       0x3002  /* Failed Indicator Test */

/* Error Codes from TU40 */

#define DEV_FAIL_PAD_TEST       0x4001  /* Failed Key Pad/Type Test */
#define KBD_ID_ERR		0x4001  /* Keyboard ID error */ 
#define KEYPAD_ERR		0x4002  /* Failed keypad test */

#define CANCEL_KEY_ENTERED	7 
#define EXIT_KEY_ENTERED	8 

/* Declare following variables and arrays for use in TU 40 */

/* Array of scan codes for all key positions */
static unsigned char scancode[133] = {
	0x0e,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,
	0x45,0x4e,0x55,0x5d,0x66,0x0d,0x15,0x1d,0x24,0x2d,0x2c,0x35,0x3c,0x43,
	0x44,0x4d,0x54,0x5b,0x5c,0x14,0x1c,0x1b,0x23,0x2b,0x34,0x33,0x3b,0x42,
	0x4b,0x4c,0x52,0x53,0x5a,0x12,0x13,0x1a,0x22,0x21,0x2a,0x32,0x31,0x3a,
	0x41,0x49,0x4a,0x51,0x59,0x11,0x00,0x19,0x29,0x39,0x00,0x58,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x64,0x00,0x00,0x61,0x6e,
	0x65,0x00,0x63,0x60,0x6f,0x6d,0x00,0x00,0x6a,0x76,0x6c,0x6b,0x69,0x68,
	0x77,0x75,0x73,0x72,0x70,0x7e,0x7d,0x74,0x7a,0x71,0x84,0x7c,0x7b,0x79,
	0x78,0x08,0x00,0x07,0x0f,0x17,0x1f,0x27,0x2f,0x37,0x3f,0x47,0x4f,0x56,
	0x5e,0x57,0x5f,0x62,0x00,0x00,0x00,0x00,0x20,0x28,0x30};

/* This array defines an actual row and column for each key
   position.  The first set contains all row information and the
   second set contains all column information for the keys.  This
   information is used to display the keyboard picture using curses.
*/

static int key[2][133] = {
        {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,
         4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,
         6,6,6,6,6,6,7,7,7,7,7,7,7,3,3,3,3,3,3,3,3,3,3,3,4,5,6,
         7,3,4,5,6,7,3,4,5,6,7,3,4,5,6,7,3,4,5,6,7,3,4,5,6,7,
         3,4,5,6,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,7,7,7},
        {0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,0,3,5,7,9,11,13,15,
         17,19,21,23,25,28,0,4,6,8,10,12,14,16,18,20,22,24,26,28,0,2,
         5,7,9,11,13,15,17,19,21,23,25,28,0,3,6,13,23,25,28,0,0,0,
         0,0,0,0,0,0,0,31,31,31,31,31,32,32,32,32,32,33,33,33,33,33,
         36,36,36,36,36,37,37,37,37,37,38,38,38,38,38,39,39,39,39,39,
         0,2,4,6,8,10,13,15,17,19,22,24,26,28,31,32,33,0,0,0,0,9,17,
         20}};

/* This array gives information about which key positions are used for each
   type of keyboard.  A '1' is for when that key position is used.  The first 
   set is for the '101' keyboard, the second for the '102', and the third for
   the '106' Kanji keyboard.  The fourth set is extra for any new keyboard.
*/

static char indexes[4][133] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0}};

/*
 * Standard Structure used by Manufacturing Diagnostics
 */

int kbdtufd = FAILURE;    /* keyboard device driver file descriptor */

#define RINGSIZE 200   /* Size of kbd input ring - */

#ifdef nodiag
struct tucb_hdr
   {
        long tu;        /* test unit number   */
        long mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */
        long loop;      /* loop test of tu    */


        long r1;        /* reserved */
        long r2;        /* reserved */
   };

struct _keybd_htx 
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	struct htx_data *htx_sp;
   };
#endif

struct tucb_env
{
    nl_catd	catd;
    long	ad_mode;
    int		kbtype;
    char	kbd_fd[20];
};

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */

#define TUTYPE struct _keybd_tu 

#ifdef nodiag
struct _keybd_tu
   {
	struct tucb_hdr   header;
	struct _keybd_htx keybd_s;
        struct tucb_env   tuenv;
   };
#else
struct _keybd_tu
   {
	struct tucb_t     header;
        struct tucb_env   tuenv;
   };
#endif
