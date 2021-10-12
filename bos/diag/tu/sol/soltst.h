/* @(#)54	1.1  soltst.h, bos320 4/22/91 16:38:13 */
/*
 *   COMPONENT_NAME: SOL Test Unit
 *
 *   FUNCTIONS: none (header file for Serial Optical Channel Converter)
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************** include files: ****************************************/


#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/soluser.h>     /* diagnostic returns from SOLDD, ioctl commands */

/********************** definitions: ******************************************/

/* more diagnostic returns (user mode test unit returns)  */

#define    SOL_PROGRAMMING_ERROR                   17
#define    SOL_BUF_ACCESS_WRITE_FAILED             18
#define    SOL_BUF_ACCESS_READ_FAILED              19
#define    SOL_BUF_ACCESS_CMP_FAILED               20
#define    SOL_ACTIVATE_MODE_FAILED                21
#define    SOL_SCR_MODE_FAILED                     22
#define    SOL_OLS_MODE_FAILED                     23
#define    SOL_RHR_TEST_FAILED                     24
#define    SOL_CRC_TEST_FAILED                     25
#define    SOL_NO_OPTICAL_CARD_PLUGGED             26
#define    SOL_LOCK_TO_XTAL_FAILED                 27
#define    SOL_ILLEGAL_TU_ERR                      28


struct tucb_t {
   long               tu, mfg, loop;
   long               r1, r2;
};

struct diag_t {                         /*  diagnostic structure              */
  int            sla_error;             /*  return error code                 */
};


struct _slatu {
    struct tucb_t     header;
    struct diag_t     diag;
};


#define TUTYPE  struct _slatu

