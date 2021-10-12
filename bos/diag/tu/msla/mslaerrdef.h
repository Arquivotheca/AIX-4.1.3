/* @(#)58       1.4  src/bos/diag/tu/msla/mslaerrdef.h, tu_msla, bos411, 9428A410j 4/15/91 17:16:25 */
/*
 * COMPONENT_NAME: (mslaerrdef.h) header file for MSLA diagnostic
 *			application.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef struct err_struct {
      unsigned int testid_word:16;
      unsigned int subtestid_char: 4;
      unsigned int err_id: 12;
} ERR_STRUCT;

struct error_id {
       union erru {
            unsigned int errval;
            ERR_STRUCT errors;
       } erru_id ;
} ;

/* 
*************************
*  MSLAMEMTEST defines  *
*************************
*/ 
#define   MSLARDWR                      1
#define   MSLARDWR32                    2

#define   MSLARDWR_BYTE_FAIL		0x0
#define   MSLARDWR_WORD_FAIL		0x1

#define   MEM32B_TEST_ERR               0x0
#define   SYS_MALLOC_ERR                0x1
#define   NOT_LONGWORD_BOUNDARY         0x2


/* 
*************************
*  MSLAREGTEST defines  *
*************************
*/ 

#define MSLASTST_START_FAIL		0x0
#define MSLASTST_STOP_FAIL		0x1
#define MSLASTOP_START_FAIL  		0x2
#define MSLA_RESET_FAIL  		0x3
#define MSLA_RESET_HALT_FAIL  		0x4
#define INTR_TO_MSLA_FAIL		0x5

/* 
*************************
*  MSLAPOSTEST defines  *
*************************
*/ 
#define   POS2_0TO0                     1
#define   LEVEL2                        2
#define   LEVEL3                        3

#define POS_DRIVER_IOCTL                0x00
#define POS2_BIT0_DISABLE_ERR    	0x01
#define POS2_BIT0_ENABLE_ERR     	0x02
#define TIMING_OUT                	0xEE

#define POSTEST_MEM_LOADED_INCORRECTLY  0x01
#define POSTEST_UCODE_NOT_STARTED    	0x02
#define PARITY_BIT_NOT_SET	 	0x03
#define PAR_ENB_EVEN_PARITY_ERR 	0x04
#define PAR_ENB_ODD_PARITY_ERR 		0x05
#define PAR_ENB_NO_LVL2_PAR 		0x06
#define PAR_ENB_HW_PAR	 		0x07
#define PAR_ENB_INV_LVL2_PAR 		0x08
#define PAR_DISB_FAIL 			0x09
#define PAR_DISB_HW_PAR	 		0x0A

#define PAR_DIS_LVL3_NOT_STARTED 	0x01
#define PAR_ENB_LVL3_INV_PAR_INT 	0x02
#define PAR_ENB_LVL3_NO_PAR_INT 	0x03
#define PAR_ENB_LVL3_NOT_STARTED 	0x04
#define PAR_DIS_LVL3_NOT_STARTED_AGAIN 	0x05


/* 
************************
*  MSLA20TEST defines  *
************************
*/

#define   FTP_START_NO_END                0x0000 
#define   FTP_68K_INSTR_TEST              0x0001 
#define   FTP_68K_INSTR_TEST_FAIL         0x0002 
#define   FTP_68K_INSTR_TEST_OK           0x0003 

#define   FTP_MEMTEST_START_NO_END        0x0004 
#define   FTP_PARITYTEST_START_NO_END     0x0005 
#define   FTP_PARITY_ERROR                0x0006 
#define   FTP_PARITY_INT_INVALID          0x0007 
#define   FTP_DATALINES_HW_FAULT          0x0008 
#define   FTP_RAMTEST_START_NO_END        0x0009 
#define   FTP_RAMTEST_FAIL                0x000A 
#define   FTP_TRAPTEST_START_NO_END       0x000B 
#define   FTP_TRAPTEST_IN_ERROR           0x000C 
#define   FTP_TIMERTEST_START_NO_END      0x000D 
#define   FTP_TIMER_NOT_WORKING           0x000E 
#define   FTP_TIMER_HW_FAULT              0x000F 
#define   FTP_TIMER_SLOW                  0x0010 
#define   FTP_TIMER_FAST                  0x0011 
#define   FTP_NONMTOS_UNEXP_INT           0x0012 
#define   FTP_MTOS_UNEXP_INT              0x0013 
#define   FTP_START_NO_LOOPING            0x0014 
#define   MEM_LOADED_INCORRECTLY          0x0015 
#define   FTP_UCODE_NOT_STARTED           0x0016 


/* 
*************************
*  MSLA19TEST defines   *
*************************
#define  FTP19                            1
#define  FTP18                            2
#define  MWRAP                            3

#define  FTP19_MEM_LOADING_ERR            0x000
#define  FTP19_UCODE_NOT_STARTED          0x001
#define  FTP_RESDBITS_ERR                 0x002
#define  FTP_SDLC_HW_ERR                  0x003
#define  FTP_RCV_UNDRUN_ERR               0x004
#define  FTP_WRAPTEST_XMIT_ERR            0x005
#define  FTP18_MEM_LOADING_ERR            0x000
#define  FTP18_UCODE_NOT_STARTED          0x001
#define  FTP18_MODEMWRAP_ERR              0x002
--------------------------
*/
#define  FTP14                            1
#define  FTP17                            2

#define  FTP14_MEM_LOADING_ERR            0x000
#define  FTP14_UCODE_NOT_STARTED          0x001
#define  FTP_RESDBITS_ERR                 0x002
#define  FTP_SDLC_HW_ERR                  0x003
#define  FTP_RCV_UNDRUN_ERR               0x004
#define  FTP_WRAPTEST_XMIT_ERR            0x005
#define  FTP17_MEM_LOADING_ERR            0x000
#define  FTP17_UCODE_NOT_STARTED          0x001
#define  FTP17_MODEMWRAP_ERR              0x002

/* 
*************************
*  MSLADMATEST defines  *
*************************
*/ 
#define  RIOS_TO_MSLA             1
#define  MSLA_TO_RIOS             2

#define  DMA_UCODE_NOT_STARTED    0x00
#define  TRANSFER_FAIL            0x01
#define  DRIVER_IOCTL             0x02

/* 
*************************
*  MSLAINTRIOS defines  *
*************************
*/ 
#define  GENERAL	          1
#define  EVEN	                  2
#define  ODD	                  3

#define  INT_UCODE_NOT_STARTED    0x00
#define  INTERRUPT_FAIL           0x01
#define  UCODE_NOT_FINISHED       0x02
#define  INT_DRIVER_IOCTL         0x03
#define  IOCTL_COUNT_INT_FAIL     0x01

/*****************************/
/* ARBTEST error definitions */
/*****************************/
#define  ARBT			    0   /* Sub-test (only this )        */

#define  UCODE_NOT_STARTED	  0x1	/* Ucode not started on time    */
#define  DATA_RD_FROM_MSLA	  0x2	/* Error in data read from msla */
#define  ERROR_IN_68K       	  0x3   /* Error in System or Local mem access*/
#define  SDLC_WRAP_ERROR  	  0x4	/* Error during SDLC wrap.      */
#define  UCODE_HAS_STOPPED  	  0x5	/* Ucode stopped/not running properly */
#define  IOCTL_RETURN_FAIL  	  0x6	/* Failure in the driver IOCTL. */


/* 
***************************
* GENERAL FAILURE defines *
***************************
*/ 
#define BUS_OPEN_FAIL   	-1
#define NOT_INITIALIZED		-2
#define SLOT_NOT_FOUND		-3
#define INVALID_MEM_ADDR    	-4
#define INVALID_IO_ADDR    	-5

#define UNKNOWN         	-10

#define UCODE_FILE_NOT_FOUND    -20
#define FILE2MEM_XFER_ERR       -21
