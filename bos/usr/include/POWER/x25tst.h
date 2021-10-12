/* @(#)92       1.2  src/bos/usr/include/POWER/x25tst.h, dax25, bos411, 9428A410j 11/17/92 11:19:25 */
/*
 * COMPONENT_NAME: TU_X25       x25 Test Unit
 *
 * FUNCTIONS: X25 Test Unit Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: x25tst.h
 *
 * FUNCTION: X25 Test Unit Header File.
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This header file contains the basic definitions needed by
 *          three applications for testing the X25 Adapter. These
 *          applications are:
 *             1) Hardware exerciser invoked by HTX,
 *             2) Manufacturing application, and
 *             3) Diagnostic application.
 *
 *
 */

/*
 * We include the HTX header file hxihtx.h because we have a need
 * to access this structure within test units in case the invoker
 * is an HTX Hardware Exerciser application (i.e., NOT the
 * manufacturing diagnostic application).  Since, the main driver
 * function, exectu(), has already been defined for use by both
 * types of applications, we will "sneak" in this structure
 * inside the TUTYPE one that we are allowed to define and pass in.
 */
#include "hxihtx.h"

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define INVOKED_BY_HTX   99


/*****************************************************/
/*   X25 Adapter Board Test Units Header File   */
/*****************************************************/


/*
 * standard structure used by manufacturing diagnostics.
 */
struct tucb_t
   {
	long tu,        /* test unit number   */
	     loop,      /* loop test of tu    */
	     mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */

	long r1,        /* reserved */
	     r2;        /* reserved */
   };

#define byte unsigned char

/*
 * error types and error codes for tca test units.
 */
#define SYS_ERR     0x00
#define LOG_ERR     0x02

/*
 * LOG_ERR (logic errors)
 */

/*
 * adapter specific definitions for test units.
 */
#define RULE_LEN      8
#define RETRIES_LEN   3
#define YES_NO_LEN    3
#define NAME_LEN      30
#define MAX_FILENAME  60

struct _x25_htx
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	char show[YES_NO_LEN+1];
	char pattern[NAME_LEN+1];

	unsigned char task_page;
	unsigned short task_offset;

	struct htx_data *htx_sp;
   };

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */
#define TUTYPE struct _x25_tu

struct _x25_tu
   {
	struct tucb_t header;
	struct _x25_htx x25_htx_s;
   };

/******************************************************************************
*                                                                             *
*  max buffer for RAM read/write                                              *
*                                                                             *
******************************************************************************/
#define TOT_BYTES        65536      /* This amounts to one page of memory */

/******************************************************************************
*                                                                             *
*  INT0 Command Codes                                                         *
*                                                                             *
******************************************************************************/
#define START_TASK_0        5      /* Start Task 0 command code */

/******************************************************************************
*                                                                             *
*  DIATSK Registers                                                           *
*                                                                             *
******************************************************************************/
#define TASK_SEGMENT_LOW    (unsigned long)0x42E
#define TASK_SEGMENT_HIGH   (unsigned long)0x42F
#define TASK_COMMAND        (unsigned long)0x430
#define TASK_ERROR_STATUS   (unsigned long)0x431
#define TASK_NUMBER         (unsigned long)0x440
#define INT_STATUS          (unsigned long)0x441
#define TASK_NUM_2          (unsigned long)0x484
#define MIF                 (unsigned long)0x4EC /* Indicates to TASK command pending */

/******************************************************************************
*                                                                             *
*  Loadable Microcode Command Codes (DIATSK)                                  *
*                                                                             *
******************************************************************************/
#define CPU_COM_CODE            0x00
#define RAM_COM_CODE            0x01
#define CIO_COM_CODE            0x02
#define GATEARRAY_COM_CODE      0x03
#define SCC_COM_CODE            0x04
#define WRAPV24_COM_CODE        0x11
#define WRAPV35_COM_CODE        0x12
#define WRAPX21_COM_CODE        0x13
#define WRAPCIOV35_COM_CODE     0x15

/******************************************************************************
* Adapter I/O port structure                                                  *
******************************************************************************/
#define adapter_LOCREG0   0x0 /* 0 location register ls part */
#define adapter_LOCREG1   0x1 /* 1 location register ms part */
#define adapter_PTRREG    0x2 /* 2 pointer register          */
#define adapter_DREG      0x3 /* 3 data register             */
#define adapter_TREG      0x4 /* 4 task register             */
#define adapter_CPUPG     0x5 /* 5 page number of window into RAM */
#define adapter_COMREG    0x6 /* 6 command register          */

#define indirect_INITREG2 0x08
#define indirect_INTCOM   0x09
#define indirect_PAR0     0x0A
#define indirect_PAR1     0x0B
#define indirect_CAD0     0x0C
#define indirect_CAD1     0x0D
#define indirect_CAD2     0x0E
#define indirect_GAID     0x0F
#define indirect_INITREG1 0x10
#define indirect_PAR2     0x11
#define indirect_INITREG0 0x12
#define indirect_INITREG3 0x13


/*
 * Error codes
 */
#define NO_FILE         0x02
#define LRAM            0x11
#define HRAM            0x12
#define POSTER          0x13
#define P_CPU_ER        0x14
#define P_ROS_CK_ER     0x15
#define P_CIO_ER        0x16
#define P_SCC_ER        0x17
#define P_GA_ER         0x18
#define P_PARITY_ER     0x19
#define P_DMA_ER        0x1A
#define POSTTIMEOUT     0x1B
#define POS_ER          0x21
#define INT_ER          0x31
#define CPU_ER          0x41
#define LDRAM_ER        0x51
#define HDRAM_ER        0x52
#define GATE_ER         0x61
#define CIO_ER          0x71
#define SCC_ER          0x81
#define WRAP_INT        0x91
#define WRAP_BASE       0xA1
#define WX21_37_ER      0xA1
#define WV24_37_ER      0xA2
#define WV35_37_ER      0xA3
#define WX21_ER         0xB1
#define WV24_ER         0xC1
#define WV35_ER         0xD1
#define WCX21_ER        0xE1
#define WCV24_ER        0xF1
#define WCV35_ER        0x101
#define COMREG_ER       0x125
#define LUCODE_ER       0x131
#define DDGRAM1         0x0321
#define DDGRAM2         0x0322
#define DDINT1          0x0331
#define DDINPB1         0x0341
#define DDINPB2         0x0342
#define DDINPB3         0x0343
#define DDINPB4         0x0344
#define DDINPB5         0x0345
#define DDINPB6         0x0346
#define DDINPB7         0x0347
#define DDINPB8         0x0348
#define DDINPB9         0x0349
#define DDINPR1         0x0351
#define DDINPR2         0x0352
#define DDINPR3         0x0353
#define DDINPR4         0x0354
#define DDINPR5         0x0355
#define DDINPR6         0x0356
#define DDINPR7         0x0357
#define DDINPR8         0x0358
#define READ_ER1        0x0361
#define GATE_RAM        0x0386
#define GATE_RAM1       0x0387
#define GATE_RAM2       0x0388
#define RESET_RAM_TIMEOUT 0x391
#define RESET_ROS_TIMEOUT 0x392
#define RESET_RCP_TIMEOUT 0x393
#define REG_ERR1        0x0401
#define REG_ERR2        0x0402
#define REG_ERR3        0x0403
#define REG_ERR4        0x0404
#define REG_ERR5        0x0405
#define REG_ERR6        0x0406
#define REG_ERR7        0x0407
#define REG_ERR8        0x0408
#define REG_ERR9        0x0409
#define REG_ERR10       0x0410
#define REG_ERR11       0x0411
#define REG_ERR12       0x0412
#define REG_ERR13       0x0413
#define REG_ERR14       0x0414
#define REG_ERR15       0x0415
#define REG_ERR16       0x0416
#define REG_ERR17       0x0417
#define CAD_ER1         0x0501
#define CAD_ER2         0x0502
#define CAD_ER3         0x0503
#define CAD_ER4         0x0504
#define CAD_ER5         0x0505
#define CAD_ER6         0x0506
#define CAD_ER7         0x0507
#define CAD_ER8         0x0508
#define CAD_ER9         0x0509
#define CAD_ER10        0x0510
#define CAD_ER11        0x0511
#define CAD_ER12        0x0512
#define CAD_ER13        0x0513
#define NOUCODE         0x0601
#define RAM_CMP_ERR     0x0700
#define INV_BUF_LEN     0x0710
#define MALLOC_FAIL     0x0720
#define O_PAT_FAIL      0x0730
#define R_PAT_FAIL      0x0735
#define TUTCTO          0x8888
#define TUTCERR         0x88FE
#define REG_ERR         0xFFFF
#define DRV_ERR         0xFFFE
#define RES_ERR         0xFFFD
#define ILLEGAL_TU_ERR  0x9999
