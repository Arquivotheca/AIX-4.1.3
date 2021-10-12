/* static char sccsid[] = "@(#)87  1.7  src/bos/diag/tu/mouse/tu_type.h, tu_mouse, bos41J, 9515A_all 4/5/95 16:03:16"; */
/*
 *   COMPONENT_NAME: TU_MOUSE
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

/*****************************************************************************

Header HTX/Mfg. Mouse Definitions File

Module Name :  tu_type.h

Header file contains basic definitions needed by three applications for
testing the Mouse Device:

	1)  Hardware exerciser invoked by HTX,
	2)  Manufacturing application, and
	3)  Diagnostic application.


 **********************************************************************
 *
 * We include the HTX header file hxihtx.h because we have a need
 * to access this structure within test units in case the invoker
 * is an HTX Hardware Exerciser application (i.e., NOT the
 * manufacturing diagnostic application).  Since, the main driver
 * function, exectu(), has already been defined for use by both
 * types of applications, we will "sneak" in this structure 
 * inside the TUTYPE one that we are allowed to define and pass in.
 **********************************************************************/

#include <hxihtx.h>
#include "diag/modid.h"

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define INVOKED_BY_HTX   99
#define RULE_LEN      8
#define RETRIES_LEN   3

/*
 * definition of constants used by all Test Units 
 */
#define READ_COUNT	4
#define REPEAT_COUNT    100
#define POS_SLOT7	6	    /* POS slot for 7012-G30 */
#define POS_SLOT0	15	    /* POS slot for all others */
#define RESET_REG	2	    /* POS register 2 used to reset mouse */
#define WAIT_FOR_ONE    1           /* This is the wait_time in sem.c */
#define POS_SEMKEY      0x3141593   /* This is an arbitrary value and
                                     * denotes the value of pi. +1 */
#define MAX_SEM_RETRIES 3

/* STANDARD STRUCTURE USED BY MANUFACTURING DIAGNOSTICS */

struct tucb_t
   {
	long tu,	/* test unit number   */
	     loop,	/* loop test of tu    */
	     mfg;	/* flag = 1 if running mfg. diagnostics, else 0  */

	long r1,	/* reserved */
	     r2;	/* reserved */
   };

/* ERROR TYPES AND ERROR CODES FOR MOUSE DEVICE TEST UNITS */
#define SYS_ERR     0x00
#define MOU_ERR     0x01
#define LOG_ERR     0x02

/* VALUES FOR VARIOUS REGISTERS */

#define	MOUSE_DATA_TX_REG      	 0x0048	
#define	MOUSE_ADP_CMD_REG        0x0049
#define MOUSE_ADP_STAT_REG       0x004A
#define MOUSE_RX_STAT_REG        0x004C 
#define MOUSE_RX_1_REG           0x004D 
#define MOUSE_RX_2_REG           0x004E
#define MOUSE_RX_3_REG           0x004F

/* VALUES SET FOR MOUSE COMMAND REGISTER  */

#define	MOUSE_DEV_CMD  		 0x00	

/* VALUES TO BE LOADED IN MOUSE DATA TRANSMIT REGISTER */

#define	ADP_CMD_DIAG_WRAP        0x01   	
#define	ADP_CMD_ENABLE_INT       0x02	
#define	ADP_CMD_DISABLE_INT      0x03	
#define	ADP_CMD_ENABLE_BLK_MODE  0x0004	
#define	ADP_CMD_DISABLE_BLK_MODE 0x0005	
 
/* BITS MAPPED FOR MOUSE STATUS REGISTER  */

#define	STAT_INT_ENABLED         0x01	
#define	STAT_BLK_MODE_ENABLED    0x02	
#define	STAT_BLK_MODE_DSBLD      0x04
#define STAT_BLK_CMD_RECD_WAIT   0x06	
 
/* BITS MAPPED FOR MOUSE RECEIVE STATUS REGISTER */

#define	STAT_RX_INTERRUPT        0x01	
#define	STAT_RX_DATA_ERROR       0x02	
#define	STAT_RX_INTF_ERROR       0x04	
#define	STAT_RX_REG1_FULL        0x08	
#define	STAT_RX_REG2_FULL        0x10	
#define	STAT_RX_REG3_FULL        0x20

/* COMMANDS SENT TO THE REGISTER TO DO VARIOUS FUNCTIONS */

#define SET_MOU_REMOTE_MODE      0xf000
#define READ_DEV_TYP_COMMAND     0xf200
#define SET_SAMPLING_RATE        0xf300
#define ENABLE_MOUSE             0xf400
#define MOUSE_DISABLE_COMMAND    0xf500	
#define SET_DEFAULT              0xf600
#define MOUSE_RESET_COMMAND      0xff00
#define RESET_SCALING            0xe600
#define SET_SCALING_TWO_TO_ONE   0xe700
#define SET_RESOLUTIONS          0xe800
#define STATUS_RQST_COMMAND      0xe900
#define SET_STREAM_MODE          0xea00
#define READ_DATA_COMMAND        0xeb00
#define RESET_WRAP_MODE          0xec
#define SET_WRAP_MODE            0xee00
#define RESEND_COMMAND	         0xfe00		

/* COMMANDS RECEIVED BY THE REGISTER AFTER VARIOUS FUNCTIONS */

#define MOUSE_ACK_COMMAND        0xFA
#define MOUSE_ERROR_CODE         0xFC

/* CHECKING RETURN CODES FROM SELF-TEST PROCEDURE */

#define MOUSE_COMPLETION_CODE    0xaa
#define MOUSE_ID_CODE            0x00

/* LOG_ERR ( error codes returned from mouse adapter) */

#define ADP_DATA_TX_REG_ERR      0x0048
#define ADP_CMD_REG_ERR          0x0049 
 
/* MOU_ERR (error codes returned from the mouse device) */

#define SUCCESS			 0
#define	FAILURE			-1

/* COMMON ERRORS FOR ALL TUs  */	

#define DEV_ADP_ERR              0xA001    /* Unexpected device or adapter err*/
#define ADP_BLOCK_DIS_ERR	 0xA002    /* Unable to disable block mode */
#define DEV_IDCODE_ERR		 0xA003    /* failed I.D. code */
#define ADP_BYTE1_ERR		 0xA004    /* Error reading byte 1 */
#define ADP_BYTE2_ERR            0xA005    /* Error reading byte 2 */
#define ADP_BYTE3_ERR            0xA006    /* Error reading byte 3 */
#define DEV_POLLING_ERR		 0xA007    /* failed repeat count */
#define	DEV_INT_DIS_ERR		 0xA008    /* Error Disabling Mouse */
#define ADP_NONBLK_ERR		 0xA009    /* Error in Setting nonblock mode */
#define ADP_BLK_MOD_ERR	         0xA00A	   /* Error in setting block mode */
#define DEV_INIT_ERR		 0xA00B	   /* Error reseting the mouse */

/* SEMAPHORE ERROR CODES */

#define SYS_SEMID_NEXST          0xA00C    /* Semaphore ID does not exist */
#define SYS_SEMID_NVALID         0xA00D    /* Semaphore ID not valid */
#define SYS_SEMVAL_NVALID        0xA00E    /* Semaphore value not valid */
#define SYS_SPND_SEM_OP          0xA00F    /* Suspend semaphore operation */
#define SYS_PID_NVALID           0xA010    /* Process ID not valid */

/* ERRORS FOR TU 10 */

#define DEV_DISABL_ERR           0x1000    /* Error in disable mouse */
#define DEV_RSET_ERR             0x1100    /* Error in reset test */
#define DEV_RD_STAT_ERR          0x1200    /* Read status error */
#define DEV_ENABL_ERR            0x1300    /* Error in "enable" mouse */
#define DEV_SET_SAMP_ERR         0x1400    /* Setting sampling error */
#define DEV_SET_REMOTE_ERR       0x1500    /* Setting remote mode error */
#define DEV_SET_SCALE_ERR        0x1600    /* Setting scale error */
#define DEV_SET_RESOLN_ERR       0x1700    /* Setting resolution error */
#define UNKNOWN_MOU_ERR          0x1800    /* Unknown mouse */
#define DEV_SET_STREAM_ERR	 0x1900	   /* error setting stream mode */

/* ERRORS FOR TU 20 */	

#define DEV_SET_WRP_ERR          0x2100    /* Error in wrap set mode */
#define DEV_RSET_WRP_ERR         0x2200    /* Error in wrap reset mode */
#define DEV_HANDLING_ERR         0x2300    /* Handling error test */
#define DEV_RESEND_ERR           0x2400    /* Resend test error */
#define DEV_WRAP_ERR             0x2500    /* pattern wrap test */

/* ERRORS IN TU 30 */

#define DEV_SAMP_RATE_ERR        0x3100    /* Setting sampling rate error */
#define DEV_RESOLUTION_RATE_ERR  0x3200    /* Setting resolution error */
#define DEV_SCALING_SET_ERR      0x3300    /* Setting scaling 2:1 error */
#define DEV_SCALING_RSET_ERR     0x3400    /* Resetting scaling error */
#define DEV_RQST_STAT_ERR        0x3500    /* Requesting status error */

/* ERRORS IN TU 40 */

#define DEV_DEFAULT_ERR          0x4100    /* Defaulting Mouse Error */
#define DEV_RQST_STAT_REPORT_ERR 0x4200    /* Requesting Status report error */

/* ERRORS IN TU 50 */

#define DEV_READ_DEV_TYP_ERR     0x5100    /* Reading device type error */
#define DEV_RD_DATA_REPORT_ERR   0x5200    /* Reading data report error */

/* ERRORS IN TU 60 */

/* NO UNIQUE ERROR CODES FOR TU 60 */

struct _mouse_htx
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	struct htx_data *htx_sp;
   };

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */
#define TUTYPE struct _mouse_tu

struct _mouse_tu
   {
	struct tucb_t header;
	struct _mouse_htx mouse_s;
   };
