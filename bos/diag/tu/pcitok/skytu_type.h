/* @(#)28	1.4  src/bos/diag/tu/pcitok/skytu_type.h, tu_pcitok, bos41J, 9521A_all 5/22/95 16:00:42  */
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

/*****************************************************************************
Header HTX/Mfg. MPS Adapter Definitions File

Module Name :  skytu_type.h

Header file contains basic definitions needed by three applications for
testing the MPS Adapter

        1)  Hardware exerciser invoked by HTX,
        2)  Manufacturing application, and
        3)  Diagnostic application.

*****************************************************************************/

#include <sys/watchdog.h>
#include <sys/diagex.h>      /* diagex version with PCI support */

#include "sky_err_codes.h" 
#include "getsky.h"
#include "sky_macros.h"
#include "skytools.h"


/****** variable types *********
#define uchar   unsigned char
#define ulong   unsigned long
#define ushort  unsigned short
#define uint    unsigned int
*******************************/
#define BOOLEAN unsigned char
#define BYTE    unsigned char
#define HALFWD  unsigned short
#define WORD    unsigned long

/******************************************************************
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 ******************************************************************/
#define INVOKED_BY_HTX   2

/******************** TEST UNIT DEFINITIONS *****************************/
#define TU_OPEN         0
#define CFG_TEST      1
#define IO_TEST         2
#define ONCARD_TEST     3
#define CONNECT_TEST    4
#define INT_WRAP_TEST   5
#define EXT_WRAP_TEST   6
#define NETWORK_TEST    7
#define TU_CLOSE        10


  /****************** CFG ADDRESSES AND THEIR WRITABLE MASKS ************/
  #define VID_REG		0X00000000  /* Vendor ID */
  #define DID_REG		0X00000002  /* Device ID */
  #define PCR			0X00000004  /* PCI Command Register */
  #define PCR_WRITE_MASK_HI	0X03   
  #define PCR_WRITE_MASK_LO	0X57 
  #define PSR			0X00000006  /* PCI Status Register */
  #define PSR_WRITE_MASK_HI	0X02
  #define CLS			0X0000000C  /* PCI Cash Line Size */
  #define CLS_WRITE_MASK	0X1C
  #define LTR			0X0000000D  /* PCI Latency Timer Register */
  #define LTR_WRITE_MASK	0XFF
  #define BA0			0X00000010  /* Base Address #0 */
  #define BA1			0X00000014  /* Base Address #1 */
  #define BAR		        0X00000030  /* PCI Expansion Rom Base Address */
  #define BAR1_WRITE_MASK       0X000000C0  /* BAR byte 1 mask */
  #define GPR			0X0000004A  /* General Purpose Port */
  #define GPR_WRITE_MASK	0X7F
  #define ILR			0X0000003C  /* PCI Interrupt Line Register */
  #define ILR_WRITE_MASK	0XFF
  #define WRITE_ALL_BITS_MASK   0XFF

  /******************* IO ADDRESSES ************************************/

  #define BCONFIG	0X7A	/* Basic Configuration Register */
  #define BMCONFIG	0X7C	/* Bus Master Configuration Register */
  #define PCT		0X8C	/* PCI Interface Threshold Control Register */


/*         Initial values for cfg reg                         */
  #define VID_VALUE_HI		0X10    /* IBM Vendor ID */
  #define VID_VALUE_LO		0X14	/* IBM Vendor ID */
  #define DID_VALUE_HI		0X00	/* Skyline Device ID */
  #define DID_VALUE_LO		0X18	/* Skyline Device ID */

#define INTERNAL        0x01    /* Internal loopback */
#define EXT_DISABLE     0x02    /* External loopback.. /LPBK pin inactivated */
#define EXT_ENABLE      0X03    /* External loopback.. /LPBK pin activated */

#define TK_GDFUSE       0x01    /* riser card type */
#define TK_BDFUSE       0x03
#define TN_CARD         0x02
#define TP_CARD         0x00

#define DEVBUS0         "/dev/bus0"      /* machine device driver device */

/**************************************************
 * adapter specific definitions for test units.
 **************************************************/
#define RULE_LEN      8
#define RETRIES_LEN   4
#define YES_NO_LEN    3
#define CONFIG_LEN   15
#define NETADD_LEN    6
#define PARTNO_LEN    8
#define PATTERN_LEN  80

#define DISABLE       0
#define ENABLE        1

/******************************************************
*           Miscellaneous defines
*******************************************************/
#define ERR_BUF_LEN           3
#define MAX_BUF_LEN           4096
#define MIN_FRAME_LEN         15
#define MAX_FRAME_LEN         4076
#define TIMEOUT_LIMIT         200
#define NO_FILE               "|none|"
#define DEFAULT_TEST_STRING   "00112233445566778899AABBCCDDEEFF"

/*******************************************************
*           Structure Definitions
********************************************************/
struct device_counter
 {
   int   good_other;
   int   bad_other;
   int   good_write;
   int   bad_write;
   int   good_read;
   int   bad_read;
   int   byte_read;
   int   byte_write;
 };

struct mps_tu
 {
   ushort      netid;
   uchar       net_addr[NETADD_LEN];
   char        rule_id[RULE_LEN+1];
   char        retries[RETRIES_LEN+1];
   char        show[YES_NO_LEN+1];

   long  tu,        /* test unit number   */
         loop,      /* loop test of tu    */
         mfg;       /* mfg = 1 if running mfg. diagnostics, else 0  */
                    /* mfg = 2 if running HTX */

   int         riser_card;
   int         packet_size;
   int         num_packet;
   char        pattern[PATTERN_LEN+1];   /* reserve */
   long        pat_size;                 /* reserve */
   ushort      default_pk;
   int         loop_remain;
   uchar       adap_flag;
   uchar       open_flag;
#ifdef BEST
   int	       best_orw;
   ulong       buf_addr;
#endif
   ADAPTER_STRUCT *adapter_info;
   
 };

typedef struct _error_details
  {
    char   error_msg[256];
    int    error_code;
    int    error_type;
    int    tunum;
    union {
             struct {
                      int    bad_address;
                      int    expected_value;
                      int    actual_value;
                     } reg;

             struct {
                      int    hrc;
                      int    mrc;
                    } adapter;

             struct {
                      int    return_code;
                    } sys;
            } un;
  } error_details;

/******************************************************************
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 ******************************************************************/
struct mps_tucb
 {
   struct mps_tu           header;
   struct device_counter   counter;
   error_details           *errinfo;
   int                     errbuf[ERR_BUF_LEN];
   FILE                    *msg_file;
 };


#define TUTYPE struct mps_tucb
#define SESBLK struct session_blk


