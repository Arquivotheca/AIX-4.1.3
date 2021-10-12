/* @(#)75       1.2  src/bos/diag/tu/baud/baud_exectu.h, tu_baud, bos411, 9439B411a 9/29/94 10:56:46  */
/*
 * COMPONENT_NAME: tu_baud
 *
 * FUNCTIONS: Declarations only
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef EXECTU_H
#define EXECTU_H


#include <sys/types.h>
#include <diag/atu.h>        /* FOR tucb_t struct DEFINITION */

#define MSG_BUF_LEN      80
#define INVOKED_BY_HTX   99
#define SEC_BUF_LEN      10

/* HOST I/O MAPPED REGISTER NUMBERS */
#define DATA_LOW_BYTE   (uchar) 0x0
#define DATA_HIGH_BYTE  (uchar) 0x1
#define ADDR_LOW_BYTE   (uchar) 0x4
#define ADDR_HIGH_BYTE  (uchar) 0x5
#define COM_STAT_REG    (uchar) 0x6

/* DIAGNOSTIC'S MAILBOX ADDRESSES */
#define HWLR         (ushort) 0x901   /* HOST WRITE LOCAL READ */
#define LWHR         (ushort) 0x900   /* LOCAL WRITE HOST READ */

/* MAILBOX ADDESSES FOR ERROR REPORTING */
#define LADDR        (ushort) 0x905   /* LOCAL ADDRESS */
#define LDWROTE      (ushort) 0x906   /* LOCAL DATA WRITTEN */
#define LDREAD       (ushort) 0x907   /* LOCAL DATA READ */

#define DIGOSC       (ushort) 0x46    /* DIGITAL OSCILATOR */
#define SH_TST_BEGIN (ushort) 0x1000  /* SHARED MEMORY TEST BEGIN */
#define SH_TST_END   (ushort) 0x1FFF  /* SHARED MEMORY TEST END */


/******************************************************************************
** ERROR REPORTING DEFINITIONS AND DECLARATIONS
******************************************************************************/
/*      0x0000 - 0x0fff:  messages
**      0x1000 - 0x1fff:  system errors
**      0x2000 - 0x2fff:  business audio logic errors
**      0x3000 - 0x3fff:  user interaction errors
******************************************************************************/
/* secondary return information buffer length follows */
#define SEC_BUF_LEN 10

#define GET_PTR_TO_IOCC 50
#define GET_PTR_TO_REGS 60
#define REL_PTR_TO_IOCC 55
#define REL_PTR_TO_REGS 65
#define CAPTURE_DMA_STATUS 70
#define PLAYBACK_DMA_STATUS   80

/* Test Unit number definitions follow */

#define TU_OPEN             1    /* Open device */
#define TU_VPD_CHECK        2    /* VPD test */
#define TU_MCI_CHIP         3    /* MCI test */
#define TU_CODEC_TEST       4    /* codec register test */
#define TU_ADAPTER_RESET    5    /* Coded setup */
#define TU_DIGITAL_LOOP     6    /* Digital loopback */
#define TU_REGISTER_CONTROL 7    /* Manual reg set */
#define TU_RECORD_PLAY      8    /* Record/playback*/
#define TU_SIMUL_REC_PLAY   9    /* Record with operator needed */
#define TU_HTX_REC_PLAY     10   /* Record with no operator needed */
#define TU_CLOSE            11   /* Close device */

/* HTX ERROR NUMBERS */
#define  ARG_COUNT_ERROR        0x20
#define  DEV_NAME_ERROR         0x21
#define  HTX_COMM_ERROR         0x22
#define  ILLEGAL_DEV_NAME       0x23
#define  DEV_OPEN_FAILED        0x24
#define  RULES_FILE_OPEN_ERROR  0x25
#define  DEV_CLOSE_ERROR        0x26
#define  CONFIGURE_ERROR        0x27

/* ERROR NUMBERS */
#define POS_REGISTERS   0x2101
#define MCI94C18A_BAD   0x2100
#define CS4231_BAD      0x2200
#define VPD_CHECKSUM    0x2300
#define VPD_READ        0x2301
#define AUDIO_QUALITY   0x2400
#define DMA_CAPTURE     0x2401
#define DMA_PLAYBACK    0x2402

/* xxxx3xxx = USER INTERACTION */
#define MESSAGE         0x00003010         /* DETAILED MESSAGE FOR USER */
#define ILLEGAL_TU      0x00003100         /* ILLEGAL TU */
#define DUP_ERR         (ILLEGAL_TU + 100) /* RULES FILE DUPLICATE */
#define REQ_ERR         (DUP_ERR + 10)     /* REQUIRED WORD ERROR */
#define RULE_ERR        (REQ_ERR + 10)     /* RULES FILE RULE */
#define STANZA_ERR      (RULE_ERR + 10)    /* RULES FILE STANZA */
#define VALUE_ERR       (STANZA_ERR + 10)  /* RULES FILE VALUE */


   typedef struct {
      unsigned long int error_code;
      unsigned long int expected;
      unsigned long int actual;
   } DEVICE_ID_ERROR_DETAILS;

   typedef struct {
     unsigned long int error_code;
     unsigned long int case_number;
     unsigned long int expected_data;
     unsigned long int actual_data;
   } MISR_CASE_ERROR_DETAILS;


   /* Union of error detail types */
   typedef union {
      unsigned long int         error_code;
      DEVICE_ID_ERROR_DETAILS   device_id;
      MISR_CASE_ERROR_DETAILS   misr_case_tests;
   } ERROR_DETAILS;



typedef struct baud_tu {
   struct tucb_t header;
   long secondary_ptr[SEC_BUF_LEN]; /* secondary return information */
   int mdd_fd;       /* File descriptor for /dev/bus0 opened by app */
   int slot;         /* The slot number (location : 1 - 8) */
   char  *device_name;     /* Logical name */
   int   error_log_count;
   ERROR_DETAILS  *error_log;
} TUTYPE, *TU_PTR_TYPE;


#endif /* EXECTU_H */



