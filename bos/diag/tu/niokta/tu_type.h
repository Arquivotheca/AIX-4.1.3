/* static char sccsid[] = "@(#)40  1.2  src/bos/diag/tu/niokta/tu_type.h, tu_adpkbd, bos411, 9431A411a 7/19/94 15:35:08"; */
/*
 *   COMPONENT_NAME: tu_adpkbd
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/***********************************************************************

Module Name :  tu_type.h

Header file contains basic definitions needed by three applications for
testing the  Device:

        1)  Manufacturing application, and
        2)  Diagnostic application.

***********************************************************************/

/*
 * definition of constants used by Test Units 
 */
 
#define REPEAT_COUNT       10000
#define WAIT_FOR_ONE       1      /* This is the value of wait_time in sem.c */
#define POS_SEMKEY         0x3141592  /* This value is an arbitrary value
                                       * and denotes the value of pi. */
#define MAX_SEM_RETRIES    3


/*
 * Standard Structure used by Manufacturing Diagnostics 
 */

struct tucb_t
   {
        long tu;        /* test unit number   */
        long mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */
        long loop;      /* loop test of tu    */
        long r1;        /* reserved */
        long r2;        /* reserved */
   };

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

#define IOW_PA_8255     0x004E    /* Write 8255 Port A Output Buffer */ 
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

/*
 * Keyboard/Speaker Command return codes 
 */ 

#define TAB_FUSE_GOOD    0x04         /* Tablet/mouse fuse good */
#define KBD_FUSE_GOOD    0x02         /* Keyboard fuse good */   
#define RD_SRAM_0D_ACK   0x11         /* Ack from reading SRAM 0x0D */
#define RD_SRAM_00_ACK   0xFA         /* Ack from reading SRAM 0x00 */
#define INVALID_CMD_ACK  0x7F         /* Ack from an invalid command */
#define INVALID_SCN_ACK  0x50         /* Ack from an invalid set scan cnt cmd */
#define INVALID_MDE_ACK  0x51         /* Illegal mode */ 
#define KBD_UART_PIN_ACK 0x21         /* Diag Sense Kbd & UART Port Pins ack */
#define WRAP_DATA_55     0x55  
#define WRAP_DATA_9A     0x9A  
 
/*
 * Commands sent to register  
 */

#define KBD_ACK_BYT_SET_CMD     0x3000       /* Set SRAM 10 mode bit 0 */
#define	WRITE_2_TABLET		0x0003	  /* Write to tablet uart */ 	
#define SET_BAUD_CMD		0x0005	  /* Set Baud rate command */
#define DSBL_KBD_UART_IRQ       0x08      /* Disable kbd and IRQ */ 
#define CFG_8255_CMD            0xC3      /* configure 8255 command */
#define NO_OP_CMD               0xF000    /* No op command */
#define DSBL_KBD_ITF            0x2B00    /* Disable kbd interface */
#define ENBL_KBD_ITF            0x3B00    /* Enable kbd interface */
#define SET_DIAG_MODE_ON        0x3A00    /* Set diagnostic mode */
#define SET_DIAG_MDE_OFF        0x2A00    /* Reset diagnostic mode */
#define DIAG_SNSE_KBD_UART      0x7000    /* Diag Sense Kbd/UART pins */
#define DIAG_WRITE_40           0x40C0    /* Write data byte bit 6
                                           * to kbd data line */
#define DIAG_WRITE_C0           0xC00C    /* Write data byte bit 7 to
                                           * kbd clock line and data
                                           * byte bit 6 to kbd data line */
#define RD_SRAM_0D              0x0D00    /* Read Shared RAM address 0x0D */
#define RD_SRAM_00              0x0000    /* Read Shared RAM address 0x00 */
#define INVALID_CMD             0x000A    /* Invalid command */
#define INVALID_SCN_CNT_CMD     0x5500    /* Invalid scan count cmd */
#define INVALID_MDE_CMD         0x6600    /* Invalid mode */
#define INVALID_EXT_CMD         0x8500    /* Invalid extended byte cmd */
#define UART_BLK_INACT          0x2500    /* Set SRAM 10 bit 5(=0) */
#define UART_BLK_ACT            0x3500    /* Set SRAM 10 bit 5(=1) */
#define DIAG_WRAP_DATA_55       0x5523    /* Diagnostic wrap data cmd */
#define DIAG_WRAP_DATA_9A       0x9A23    /* Diagnostic wrap data cmd */
#define ENBL_UART_ITF           0x3C00    /* Enable UART interface */
					  /* Set SRAM 11 mode bit 12 */
#define INIT_UART_FRM_E		0x0606	  /* Initialize UART framing to Even */
#define DSBL_UART_ITF           0x2C00    /* Disable UART interface */

/* TO CHECK BIT 5 READ PORT C REGISTER */

#define INPUT_BUFF_FULL         0x20 

/* TO CHECK BIT 7 OF READ PORT C REGISTER */ 

#define OUTPUT_BUFF_EMPTY       0x80

/*
 *                     KEYBOARD ERROR CODES 
 */

#define SUCCESS         0
#define FAILURE         -1
#define BAD_TU_NO       1

/*
 * Common error codes from Keyboard 
 */

#define ADP_HDW_RSET_ERR        0xF001  /* Hardware Reset Error */
#define ADP_ENBL_IF_ERR         0xF002  /* Enable UART interface */
#define ADP_BLK_INACT_ERR       0xF003  /* setting UART blk to inactive */

/* SEMAPHORE ERROR CODES */

#define SEMID_NEXST             0xF004  /* Semaphore ID does not exist */
#define SEMID_NVALID            0xF005  /* Semaphore ID not valid */
#define SEMVAL_NVALID           0xF006  /* Semaphore value not valid */
#define SPND_SEM_OP             0xF007  /* Suspend semaphore operation */
#define PID_NVALID              0xF008  /* Process ID not valid */
 
/* TU 10 ERRORS */

#define ADP_TAB_FUSE_FAIL       0x1001  /* Keyboard fuse not good */
#define ADP_KEY_FUSE_FAIL       0x1002  /* Keyboard fuse not good */
#define ADP_NO_OP_ERR           0x1003  /* during no op command */
#define ADP_DIAG_ON_ERR         0x1004  /* setting diagnostic mode */
#define ADP_DIAG_OFF_ERR        0x1005  /* resetting diagnostic mode */
#define ADP_DSBL_KBD_ERR        0x1006  /* disabling kbd interface */  
#define ADP_DIAG_SNSE_ERR       0x1007  /* sensing the kbd and UART pins */
#define ADP_DIAG_WRT40_ERR      0x1008  /* writing 0x40 to the kbd port pins*/
#define ADP_DIAG_WRTC0_ERR      0x1009  /* writing 0xC0 to the kbd port pins*/
#define ADP_ENBL_KBD_ERR        0x100A  /* disabling kbd interface */  

/* TU 20 ERRORS */

#define ADP_RD_SRAM_0D_ERR     0x2001  /* reading SRAM 0x0D */
#define ADP_RD_SRAM_00_ERR     0x2002  /* reading SRAM 0x00 */
#define ADP_INVALID_CMD_ERR    0x2003  /* due to invalid cmd */
#define ADP_INVL_SCN_CNT_ERR   0x2004  /* due to invalid scan count cmd */
#define ADP_INVALID_MDE_ERR    0x2005  /* due to invalid mode */

/* TU 30 ERRORS */

#define ADP_BLK_ACT_ERR        0x3001  /* setting UART blk to active */
#define ADP_WRP_55_ERR         0x3002  /* wrap data 55 */
#define ADP_WRP_9A_ERR         0x3003  /* wrap data 9A */
#define ADP_DSBL_IF_ERR        0x3004  /* Disable UART interface */

/* TU 40 ERRORS */

#define ADP_FRM_O_ERR           0x4001  /* initializing UART frame to even */
#define ADP_WRP_MDE_ERR         0x4002  /* Wrap mode */
#define	ADP_WRAP_ERR		0x4003	/* Wrap */
#define ADP_EZ_BLK_ERR          0x4004  /* UART frame to even and block to 4 */

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */

#define TUTYPE struct _keybd_tu 

struct _keybd_tu
   {
        struct tucb_t header;
   };
