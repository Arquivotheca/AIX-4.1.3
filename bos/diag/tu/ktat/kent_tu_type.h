/* @(#)87	1.2  src/bos/diag/tu/ktat/kent_tu_type.h, tu_ktat, bos41J, 9519A_all 5/3/95 15:02:53  */
/*
 *   COMPONENT_NAME: tu_ktat
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
Header HTX/Mfg. Klickitat Adapter Definitions File

Module Name :  kent_tu_type.h

Header file contains basic definitions needed by three applications for
testing the Klickitat Adapter

        1)  Hardware exerciser invoked by HTX,
        2)  Manufacturing application, and
        3)  Diagnostic application.

*****************************************************************************/
/*  Header file */
/* #include <sys/watchdog.h>  */

/****** variable types *********/
#define uchar   unsigned char
#define ulong   unsigned long
#define ushort  unsigned short
#define uint    unsigned int
/*******************************/
#define BOOLEAN unsigned char
#define BYTE    unsigned char
#define HALFWD  unsigned short
#define WORD    unsigned long

/******************************************************************
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 ******************************************************************/
#define INVOKED_BY_HTX   2

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

struct kent_tu
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
#define ERR_BUF_LEN 3

struct kent_tucb
 {
   struct kent_tu          kent;
   struct device_counter   counter;
   error_details           *errinfo;
   int                     errbuf[ERR_BUF_LEN];
   FILE                    *msg_file;
 };


#define TU_TYPE struct kent_tucb
#define SESBLK struct session_blk

typedef struct {
  int tbadr;
  int tmd1;
  int tmd2;
  int reserved;
} tx_desc_t;

typedef struct {
  int rbadr;
  int rmd1;
  int rmd2;
  int reserved;
} rx_desc_t;

typedef struct {
  int tlen_rlen_mode;
  int padr_lo4;
  int padr_hi2;
  int ladr_lo;
  int ladr_hi;
  ulong  rdra;
  ulong  tdra;
} init_block_t;


/*  structure passed to interrupt handler   */

struct int_data
{
  int pending;
  int status;
  int expect;
  int intr_count;
  int xmit_count;
  int rec_count;
};

