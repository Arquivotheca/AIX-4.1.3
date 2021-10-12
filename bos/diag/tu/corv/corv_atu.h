/* @(#)27       1.6  src/bos/diag/tu/corv/corv_atu.h, tu_corv, bos411, 9428A410j 12/17/93 09:01:39 */
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*-- adapter header files --*/
#include "corvette.h"
ADAPTER_HANDLE ahandle;    /* pointer for opened adapter handle */

/*-- defines for test unit numbers --*/
#define COATU_INIT_TUS         (0x01)  /*- intialize adapter for test units -*/
#define COATU_TERM_TUS         (0x02)  /*- terminate test / resore adapter/children -*/
#define COATU_REG_TEST         (0x03)  /*- command interface register test -*/
#define COATU_PACE_CMD         (0x04)  /*- pace command test -*/
#define COATU_SCB_CMD          (0x05)  /*- subsystem control block test -*/
#define COATU_ADDR_TEST        (0x06)  /*- card address line test -*/
#define COATU_MCODE_DWNLD      (0x08)  /*- download microcode to the adapter -*/

/*-- general return codes from CORV ATUs --*/
#define COATU_SUCCESS           (0)    /*- test unit completed nornmally -*/

#define COATU_TIMEOUT           (-10)  /*- timeout occurred before test unit completion -*/
#define COATU_NOT_DIAGNOSE      (-11)  /*- adapter not set to diagnose state -*/
#define COATU_FAILED            (-12)  /*- test unit failed to complete normally -*/
#define COATU_INVALID_PARAM     (-13)  /*- invalid parameter passed to test unit -*/
#define COATU_UNABLE_UNCONFIG   (-14)  /*- adapter or child can not be unconfigured -*/

#define COATU_ALREADY_DIAGNOSE  (-20)  /*- attempted to set diagnose when already in diagnose state -*/
#define COATU_UNABLE_DIAGNOSE   (-21)  /*- unable to set adapter to diagnose state -*/
#define COATU_UNABLE_OPEN       (-24)  /*- unable to open the adapter in diagex mode -*/
#define COATU_UNABLE_DEFINE     (-25)  /*- unable to place adapter in to defined state -*/
#define COATU_UNABLE_RECONFIG   (-26)  /*- unable to reconfigure adapter or children to their original state -*/

/*-- general defines and structure definitions --*/
#define _50_PERCENT 50

typedef struct {
           char *ldev_name; /* adapter device name          */
  unsigned long tu;         /* test unit number                */
  unsigned long loop;       /* test unit loop count            */
           char *io_buff;         /* input/output address pointer    */
           char isr;        /* interrupt status register value */
} CORV_TUTYPE;

      CORV_TUTYPE tx_corv;  /* structure defined for application use */

