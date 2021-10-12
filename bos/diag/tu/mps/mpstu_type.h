/* @(#)42       1.2  src/bos/diag/tu/mps/mpstu_type.h, tu_mps, bos411, 9437B411a 8/28/94 12:35:30 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS Test Unit Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*****************************************************************************
Header HTX/Mfg. MPS Adapter Definitions File

Module Name :  mpstu_type.h

Header file contains basic definitions needed by three applications for
testing the MPS Adapter

        1)  Hardware exerciser invoked by HTX,
        2)  Manufacturing application, and
        3)  Diagnostic application.

*****************************************************************************/
/*  Header file */
#include <sys/watchdog.h>
#include <sys/diagex.h>

#include "mps_err_codes.h"
#include "mpstools.h"
#include "mps_macros.h"

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
#define POS_TEST        1
#define IO_TEST         2
#define ONCARD_TEST     3
#define CONNECT_TEST    4
#define INT_WRAP_TEST   5
#define EXT_WRAP_TEST   6
#define NETWORK_TEST    7
#define TU_CLOSE        10

/******************** POS ADDRESS ************************************/

#define POS0    0
#define POS1    1
#define POS2    2
#define POS3    3
#define POS4    4
#define POS5    5
#define POS6    6
#define POS7    7

/*         Initial values for pos reg                         */
#define  INTPOS0        0xa2    /* adapter id                 */
#define  INTPOS1        0x8f    /* adapter id                 */

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
#define TIMEOUT_LIMIT         6000
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
 };

typedef struct _error_details
  {
    char   *error_msg;
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


