/* static char sccsid[] = "@(#)04  1.5  src/bos/diag/tu/kbd/tu_type.h, tu_kbd, bos411, 9433A411a 7/13/94 09:37:40"; */
/*
 *   COMPONENT_NAME: tu_kbd
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
#include <cur00.h>
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

#ifdef DIAGNOSTICS
#include <sys/inputdd.h>
#include "diag/atu.h"
#include "diag/diago.h"
#include "kbd_msg.h"
#endif
/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define RULE_LEN      8
#define RETRIES_LEN   3

/*
 * definition of constants used by Test Units 
 */
 
#define RETRY		   3	
#define REPEAT_COUNT 	   30
#define SCN_CDE_CNT  	   3
#define KEY_POS_CNT  	   133
#define ESC_KEY	     	   110
#define ESC_KEY_SCN_SET3   0x08
#define WAIT_FOR_ONE       1   /* This is the value of wait_time in sem.c */
#define POS_SEMKEY         0x3141592  /* This value is an arbitrary value
				       * and denotes the value of pi. */
#define DIAG_ERR_MASK	   0x00FF0000
#define MAX_SEM_RETRIES    3


/*
 * Erros types and error codes for Keyboard Device Test Units  
 */

#define SYS_ERR     0x00
#define KBD_ERR     0x01
#define LOG_ERR     0x02

/*
 * Keyboard Adapter Registers values 
 */

/* NOTE: THOUGH THE ADDRESS OF WR_PORTA IS 0x0050, THERE IS NO HALF
	 WORD OR TWO BYTE DEFINITION IN THE MACHINE DEVICE DRIVER
	 ROUTINE. AS A RESULT THE ADDRESS FOR WR_PORTA STARTS AT
	 0x004E */

#define IOW_PA_8255	0x004E    /* Write 8255 Port A Output Buffer */ 
#define IOR_PA_8255	0x0054    /* Read 8255 Port A Input Buffer */
#define IOR_PB_8255	0x0055    /* Read 8255 Port B */ 
#define IOR_PC_8255	0x0056    /* Read 8255 Port C */
#define	IOW_CFG_8255    0x0057    /* Write 8255 Configure  */

/*
 * Interrupt ID Number
 */

#define ID_INFO_INT     0x08      /* Informational interrupt */
#define ID_BAT_INT	0x06	  /* 8051 self-test performed */	
#define ID_ERR_INT	0x07	  /* 8051 detected an error condition */
#define ID_BYTE_INT	0x01	  /* Byte received from keyboard */

/*
 * POS Register and SRAM address
 */

#define POS_SLOT	15        /* MC device dedicated slot */
#define RESET_REG 	 2        /* POS register */
#define RST_KBD_8051  0x04        /* bit pattern to reset keyboard */  
#define REL_8051_RST  0xFB        /* Initial POS reg. value  */  

/*
 * RETURN CODES 
 * Adapter general 
 */

#define ADP_COMP_CODE	0xAE   /* Successful adapter completion code */
#define KBD_CMP_CDE     0xAA   /* Successful completion of BAT */
#define KBD_ACK         0xA0   /* Keyboard acknowlegde */

/*
 * Keyboard/Speaker Command return codes 
 */ 

#define	KBD_CMD_ACCP	 0x00         /* keyboard command accepted */
#define SPK_CMD_ACCP	 0x01	      /* Speaker command accepted */
#define SPK_INACT        0x02         /* Speaker already inactive */
#define KBD_1_101_ID     0xAB         /* First byte of Kbd 101/102 ID */
#define KBD_1_106_ID	 0xBF	      /* First byte of Kbd 106 ID */
#define KBD_2_101_ID     0x83         /* Sec. byte of 101/102 kbd ID */
#define KBD_2_106_ID     0xB2         /* Sec. byte of 106 Kbd ID */
#define KBD_LAY_101      0xB0         /* First byte of 101 Kbd Layout ID */ 
#define KBD_LAY_102      0xB1         /* First byte of 102 Kbd Layout ID */
#define KBD_2ND_LAY_ID   0xBF         /* Sec. byte of 101/102 Kbd Layout ID */
#define ECHO_DATA	 0xEE         /* Echoed data verification */
#define SCAN_SET1	 0x01         /* Scan code set 1 */
#define INVALID_CMD_ACK  0x7F	      /* Ack from an invalid command */

/*
 * Commands sent to register  
 */

#define CFG_8255_CMD	       	0xC3         /* configure 8255 command */
#define KBD_ACK_BYT_SET_CMD     0x3000       /* Set SRAM 10 mode bit 0 */
#define RST_MDE_IND_CMD		0xED01	     /* Reset keyboard indicators */
#define SET_IND_OFF_CMD		0x0001       /* Set all indicators off */
#define SET_IND_ON_CMD		0x0701       /* Set all indicators on */
#define KBD_RSET_CMD		0xFF01       /* Reset keyboard */ 
#define SPK_HVOL_CMD		0x4300       /* Set speaker to high volume */
#define SPK_LVOL_CMD		0x4100       /* Set speaker to high volume */
#define SPK_30HZ_HBYT_CMD       0x4008       /* High byte of set speaker    
					      * to 30Hz frequency */
#define SPK_30HZ_LBYT_CMD       0xBF09       /* Low byte of set speaker    
					      * to 30Hz frequency */
#define SPK_12000HZ_HBYT_CMD    0x0108       /* High byte of set speaker    
					      * to 12000Hz frequency */
#define SPK_12000HZ_LBYT_CMD    0x1309       /* Low byte of set speaker    
					      * to 12000Hz frequency */
#define SPK_2SEC_DUR_HBYT_CMD   0x0007       /* High byte of set speaker    
					      * to 2 secs duration */
#define SPK_2SEC_DUR_LBYT_CMD   0xFF02       /* Low byte of set speaker    
					      * to 2 secs duration */
#define SPK_4SEC_DUR_HBYT_CMD   0x0107       /* High byte of set speaker    
					      * to 4 secs duration */
#define SPK_4SEC_DUR_LBYT_CMD   0xFF02       /* Low byte of set speaker    
					      * to 4 secs duration */
#define SPK_6SEC_DUR_HBYT_CMD   0x0207       /* High byte of set speaker    
					      * to 6 secs duration */
#define SPK_6SEC_DUR_LBYT_CMD   0xFF02       /* Low byte of set speaker    
					      * to 6 secs duration */
#define KBD_ID_CMD		0xF201       /* Keyboard ID */
#define KBD_LAY_ID_CMD		0xEF01       /* kbd layout ID */
#define KBD_ECHO_CMD		0xEE01	     /* Kbd Echo cmd */
#define KBD_SCAN_CDE_CMD        0xF001       /* Kbd scan code */
#define KBD_SCAN_SET1_CMD       0x0101       /* Kbd scan code set 1 */
#define KBD_SCAN_SET3_CMD       0x0301       /* Kbd scan code set 3 */
#define KBD_SCAN_QRY_CMD        0x0001       /* Kbd scan query */
#define INVALID_CMD	        0x000A	     /* Invalid command */

/* TO CHECK BIT 5 READ PORT C REGISTER */
#define INPUT_BUFF_FULL		0x20 

/* TO CHECK BIT 7 OF READ PORT C REGISTER */ 
#define OUTPUT_BUFF_EMPTY	0x80


/* TO CHECK BIT 3 OF READ PORT B REGISTER FOR STROBE */
#define STROBE_RECD		0x08

/*
 *                     KEYBOARD ERROR CODES 
 */

#define SUCCESS		0
#define	FAILURE		-1
#define BAD_TU_NO	1

/*
 * Common error codes from Keyboard 
 */

#define ADP_HDW_RSET_ERR	0xF002	/* Hardware Reset Error */

/* SEMAPHORE ERROR CODES */

#define SEMID_NEXST         	0xF00B  /* Semaphore ID does not exist */
#define SEMID_NVALID        	0xF00C  /* Semaphore ID not valid */
#define SEMVAL_NVALID       	0xF00D  /* Semaphore value not valid */
#define SPND_SEM_OP         	0xF00E  /* Suspend semaphore operation */
#define PID_NVALID          	0xF00F  /* Process ID not valid */
 
/* TU 10 ERRORS */

#define DEV_RESET_ERR		0x1001 	/* Unable to reset keyboard */
#define ADP_PA_RST_ERR          0x1002  /* Error in BAT completion */

/* TU 20 ERRORS */
   
#define DEV_ID_ERR		0x2001  /* Error in keyboard ID */
#define ADP_PA_ID_ERR           0x2002  /* Error in port A of keyboard ID */
#define DEV_ID_BYT1_ERR         0x2004  /* Error in byte 1 of kbd ID */
#define DEV_ID_BYT2_ERR         0x2005  /* Error in byte 2 of kbd ID */
#define DEV_LAY_ID_ERR          0x2007  /* Error in kbd layout ID command */
#define ADP_PA_LID_ERR          0x2008  /* Error in port A of keyboard layout ID */
#define DEV_LID_BYT1_ERR        0x200A  /* Error in byte 1 of kbd layout ID */
#define DEV_LID_BYT2_ERR        0x200B  /* Error in byte 2 of kbd layout ID */
#define DEV_ECHO_ERR		0x200C  /* Error during ECHOED data */
#define ADP_PA_ECH_ERR          0x200D  /* Error in port A during echoed data  */
#define DEV_SCAN_ERR            0x200F  /* Error during scan cmd */
#define DEV_SCAN_SET1_ERR       0x2010  /* Error during select scan code 1 */
#define DEV_SCN_QRY_ERR         0x2011  /* Error during scan query */
#define ADP_PA_QRY_ERR          0x2012  /* Error in port A during scan query  */
#define DEV_SCAN_SET3_ERR       0x2014  /* Error during select scan code 3 */

/* TU 30 ERRORS */

#define DEV_IND_ON_ERR		0x3001  /* Error in turning ON the LEDs */
#define DEV_FAIL_IND_TEST       0x3002  /* Failed Indicator Test */

/* TU 40 ERRORS */

#define DEV_FAIL_PAD_TEST       0x4001  /* Failed Key Pad/Type Test */


/* TU 50 ERRORS */

#define ADP_CLICK_ON_ERR        0x5006  /* Error in activating auto-click */
#define ADP_CLICK_OFF_ERR       0x5008  /* Error in deactivating auto-click */

/* TU 60 ERRORS */

#define ADP_FAIL_VOL_TEST       0x6003  /* Failed Speaker freq, volume test */
#define DEV_SPK_HVOL_ERR        0x6004  /* Error in setting spk to high vol. */
#define DEV_30_HBYT_ERR         0x6005  /* Error in high byte of set
                                         * frequency */
#define DEV_30_LBYT_ERR         0x6006  /* Error in low byte of set
                                         * frequency */


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
   the '106' Kanji keyboard.  The fourth set is for new Brazilian Karok
   104 key '102' keyboard.  The fifth set is for new German Kaska 100 key '102'
   keyboard.
*/

static char indexes[5][133] = {
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
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0}};


/*
 * Standard Structure used by Manufacturing Diagnostics 
 */

int kbdtufd;    /* keyboard device driver file descriptor */

#define RINGSIZE 200   /* Size of kbd input ring - */

struct tucb_hdr
   {
	long tu;	/* test unit number   */
	long mfg;	/* flag = 1 if running mfg. diagnostics, else 0  */
	long loop;	/* loop test of tu    */
	long r1;	/* reserved */
	long r2;	/* reserved */
   };

struct _keybd_htx 
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	struct htx_data *htx_sp;
   };

#ifdef DIAGNOSTICS
typedef struct TUCB_SYS
{
    nl_catd	catd;
    long	ad_mode;
    int	kbtype;
    char kbd_fd[20];
};
#endif

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */

#define TUTYPE struct _keybd_tu 

struct _keybd_tu
   {
	struct tucb_hdr header;
#ifdef nodiag
	struct _keybd_htx keybd_s;
#endif

#ifdef DIAGNOSTICS
        struct TUCB_SYS tuenv;
#endif
    };

