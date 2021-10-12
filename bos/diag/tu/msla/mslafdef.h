/* @(#)22  1.3  src/bos/diag/tu/msla/mslafdef.h, tu_msla, bos41J, 9519A_all 5/10/95 09:57:44 */
/*
 * COMPONENT_NAME: (mslafdef.h) header file for MSLA diagnostic
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

/*
** The following are the system wide hash defines
*/

#define NULL_DATA	0x0000
#define MAX_16BIT	65536
#define M32K_BYTES	0x8000
#define M16K_BYTES	0x4000
#define  M4K_BYTES	0x1000
#define MAX_ERROR	3
#define MAX_INTERUPT_COUNT	100
#define TRY_COUNT		99999
#define MSLA_DONT_COUNT		3
#define NO_OF_TESTBYTES	        7
#define NO_INTERUPT		2
#define MSLA_STATUS_NOINTR	8
#define  ERROR_IN_VALID_INT     5
#define SUCCESS			0
#define FAIL			-1
#define NO			0
#define YES			1
#define NOT_FOUND		0
#define NO_ERROR		0
#define ERROR_ON		1
#define CRRA_ADDR	((char *)0xF0008C40)
#define DELAY_ADDR      ((unsigned *) 0xF00080E0)
#define DELAY           0xffffffff

#define RESET_ALL       0xFF
#define EMPTY		0xFF
/*
	These are example definitions to use with RT PC lposts.
*/
#define MSLA_COUNT_ROUT_LEN	14	        /* SEE IN utils code */
#define START_PARM	 	0X00000000	/* SEE IN utils code */
#define MSLA_LOOK_COUNTER 	0X00000018	/* SEE IN utils code */
#define SLOT_TBL_ROW	8
#define SLOT_TBL_COL	8
#define MSLARDWR_FAIL		2
#define MSLA_INTRUPT_ROUT1_LEN  4
#define MSLA_INTRUPT_ROUT2_LEN 	2
#define MSLA_INTRUPT_ROUT3_LEN	4
#define MSLA_INTRUPT_ROUT4_LEN	36
#define ROUT1_ADDR		0x00000000
#define ROUT2_ADDR		0x0000006C
#define ROUT3_ADDR		0x00000100
#define ROUT4_ADDR		0x00001000
#define ROUT5_ADDR		0x00000103
#define ROUT6_ADDR		0x00000107
#define INTR_ROUT1_ADDR		0x00000000
#define INTR_ROUT2_ADDR		0x00000100
#define INTR_ROUT3_ADDR		0x00001000

/*#define VALID_INTRUPT_LEVEL2	2*/

# define DISABLE_FAIL		4
#define RDWR_ADDR_ROUT1		0x00000000
#define RDWR_ADDR_ROUT2		0x00000100
#define RDWR_ROUT1_LEN		4
#define RDWR_ROUT2_LEN		26
#define COMMAND_REGISTER     	0xF0008870
#define DMA_BUFFER_REG		0xF00088C0
#define MSLA_TRANSFER_FAIL	1
#define MSLA_XFER_ERROR		3
#define MSLA_TIME_OUT		2
#define WRITE_DMA_MASK       	0xF000887E
#define MASTER_CLEAR_AGAIN   	0xF000887A
#define MASK_32_TO_24 		0x00FFFFFF
#define TEST_FAIL	7
#define INV_PROGRESS	3

#define  FTP_FLAG0_HI_WORD 	0x40
#define  FTP_FLAG0_LO_WORD 	0x42

#define  FTP_FLAG1_HI_WORD 	0x44
#define  FTP_FLAG1_LO_WORD 	0x46

#define  FTP_FLAG2_HI_WORD 	0x48
#define  FTP_FLAG2_LO_WORD 	0x4A

#define  FTP_FLAG3_HI_WORD 	0x4C
#define  FTP_FLAG3_LO_WORD 	0x4E

#define  FTP_FLAG4_HI_WORD 	0x50
#define  FTP_FLAG4_LO_WORD 	0x52

#define  FTP_FLAG5_HI_WORD 	0x54
#define  FTP_FLAG5_LO_WORD 	0x56

#define OK_DATA		0x00EE

#define VALID_INTRUPT_LEVEL2	2
#define INTRUPT_TIME_OUT	9
#define GOOD_PROGRESS          5

#ifdef R2
#define NSLOTS          16          /* number of micro channel slots          */
#define STAMPIT(x) printf("%s: %s %s\n",__FILE__,__DATE__,__TIME__)
#else
#define STAMPIT(x) printf("%s\n","compiled today")
#endif

#define INTERUPT_MSLA 	0X00000090
#define RSET_MSLA 	0X00000092
#define HLT_MSLA 	0X00000094
#define START_MSLA 	0X00000096
#define READ_SR_MSLA 	0X00000098

#define RESET_INTERUPT_MSLA 	0X0000009A
#define ENABLE_INTERUPT_MSLA 	0X0000009C
#define DISABLE_INTERUPT_MSLA 	0X0000009E
#define ALLOW_SHARE_INTERUPT	0x000006F2

#define IO_STR_INT_MASK   0x01
#define IO_STR_PAR_MASK   0x02

#define RT_DATA1_LEN  4
#define RT_DATA2_LEN  4
#define RT_DATA3_LEN  28

/* Define PRINT and PRNTE as follows for debugging trace. */
/* Leave as is for normal use.				*/
/* #define PRINT(a)	printf a	*/
/* #define PRNTE(a)	printf a	*/
#define PRINT(a)
#define PRNTE(a)
