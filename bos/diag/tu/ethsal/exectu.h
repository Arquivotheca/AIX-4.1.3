/* @(#)51       1.1.1.4  src/bos/diag/tu/ethsal/exectu.h, tu_ethsal, bos411, 9428A410j 10/22/93 09:50:08 */
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "diag/atu.h"

#ifndef BOOL
#define BOOL uchar_t
#endif

#ifdef TRUE
#undef TRUE
#define TRUE  ((BOOL) 1)
#endif

#ifdef FALSE
#undef FALSE
#define FALSE  ((BOOL) 0)
#endif

#define SUCCESS  0
#define FAIL     1

#define INVALID_MACHINE_MODEL -1
#define INVALID_MODE_POSITION -1

/* These values are for differentiating between a twisted pair or thick mode
   setup */
enum { NON_TWISTED_PAIR_POSITION = 0, TWISTED_PAIR_POSITION };

/* Differentiate between PowerPC and RSC machines */
enum { POWER_MACHINE_MODEL = 1, RSC_MACHINE_MODEL };

BOOL power_flag;     /* default set for RSC machine */

typedef struct
{
  ulong_t eth_mode_setting;   /* Thick/Twisted Pair status */
  ulong_t machine_model;      /* machine model            */
  ulong_t r1;                           /* reserved                 */
  ulong_t r2;                           /* reserved                 */
} TU_PARMS;


typedef struct
{
  struct tucb_t header;   /* common TU header              */
  int mdd_fdes;           /* Machine dd file descriptor    */
  int slot;               /* reserved                      */
  int errno;              /* updated with "errno" variable */
                          /* when a system call fails      */
  TU_PARMS tu_parms;       /* structure for passing config. info to DA */

} TUCB;


/* Next struct. used for keeping statistics */

typedef struct
{
  ulong_t  bad_others;      /* Count of bad ioctl operations        */
  ulong_t  bad_reads;       /* Count of bad read operations         */
  ulong_t  bad_writes;      /* Count of bad write operations        */
  ulong_t  bytes_read;      /* Total bytes read                     */
  ulong_t  bytes_writ;      /* Total bytes written                  */
  ulong_t  good_others;     /* Count of good ioctl operations       */
  ulong_t  good_reads;      /* Count of good read operations        */
  ulong_t  good_writes;     /* Count of good write operations       */

} STATIST_STRUCT;


/* TUs implemented   */

enum
{ POS_TU = 1,                    /* POS registers                   */
  VPD_TU,                        /* VPD                             */
  IO_TU,                         /* I/O registers                   */
  SELFTEST_TU,                   /* 82596 selftest                  */
  LB_82596_TU,                   /* loopback at the 82596           */
  LB_825XX_TU,                   /* loopback at the 8250X           */
  LB_EXT_TU,                     /* loopback at the DIX conn.       */
  LB_EXT_EMC_TU                  /* loopback at the DIX conn. (EMC) */
};


/* common return codes */
enum { TU_NUMBER_ERR = 1, INPUT_PARMS_ERR, MIOCCPUT_ERR, MIOCCGET_ERR,
       ENT_POS_ERR, CIO_START_ERR, CIO_HALT_ERR
      };

/* POS_TU return codes */
enum { POS_ID_ERR = 100, CONTROL_REG_ERR, DMA_CONTROL_REG_ERR,
       POS_STATUS_REG_ERR, VPD_ADDR_REG_ERR
     };

/* VPD_TU return codes */
enum { VPD_NOT_FOUND_ERR = 200, VPD_LENGTH_ERR, VPD_CRC_ERR,
       VPD_DELIM_ERR, ETHER_VPD_MSG_ERR, ETHER_NADDR_NOT_FOUND_ERR,
       ETHER_NADDR_MULTICAST_ERR
     };

/* IO_TU return codes */
enum { SYNC_BIT_ERR = 300, PARITY_BIT_ERR, FEEDBACK_ERR,
       CHANNEL_CHECK_ERR, FUSE_ERR
     };

/* SELFTEST_TU return codes */
enum { ENT_SELFTEST_ERR = 400, SELFTEST_ERR };

/* Loopback TUs return codes */
enum { COLLISION_ERR = 500, CARRIER_LOST_ERR, FRAME_COMPARE_ERR,
       CIO_GET_STAT_ERR, CIO_QUERY_ERR, ENT_CFG_ERR,
       WRITE_ERR, READ_ERR, POLL_WRITE_TIMEOUT_ERR,
       POLL_READ_TIMEOUT_ERR, POLL_ERR,
       UNDERRUN_ERR, OVERRUN_ERR, POLL_TIMEOUT_ERR
     };

