/* @(#)48  1.2  src/bos/kernext/x25/crdcsm.h, sysxx25, bos411, 9428A410j 6/15/90 18:47:37 */
#ifndef _H_CRDCSM
#define _H_CRDCSM
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */

#define crdcsm_h

#include <memory.h>
#include <sys/types.h>
#include "jsdefs.h"

#define CRD_TRANSFER_BYTE(var, ptr) {(var) = *((byte *)(ptr)); (ptr)++;}
#define CRD_TRANSFER_USHORT(var, ptr) {(var) = (ushort)(*(ptr)+(*((ptr)+1))); (ptr)+=2;}
#define CRD_TRANSFER_SOME_BYTES(dest, ptr, number) \
{memcpy((void *)(dest), (void *)(ptr), (size_t)(number)); (ptr) += (number);}

#define MAX_ALERT_NUMBER (40)

#define  X_5    5                       /* Modem failure: DCD, DSR, cable    */
#define  X_7    7                       /* Modem failure: ACU not responding */
#define  X_8    8                       /* Modem failure: X21 not connected  */

#define  X_9    9                       /* FRMR type W received              */
#define  X_10   10                      /* FRMR type X received              */
#define  X_11   11                      /* FRMR type Y received              */
#define  X_12   12                      /* FRMR type Z received              */
#define  X_13   13                      /* FRMR type W sent                  */
#define  X_14   14                      /* FRMR type X sent                  */
#define  X_15   15                      /* FRMR type Y sent                  */
#define  X_16   16                      /* FRMR type Z sent                  */
#define  X_17   17                      /* Frame retry N2 reached            */
#define  X_18   18                      /* Unexpected DISC received          */
#define  X_19   19                      /* DM rxd during link activation     */

#define  X_21   21                      /* CLEAR INDICATION rxd              */
#define  X_22   22                      /* RESTART INDICATION rxd            */
#define  X_23   23                      /* RESET REQUEST by X25 adapter      */
#define  X_24   24                      /* CLEAR REQUEST by X25 adapter      */
#define  X_25   25                      /* RESTART REQUEST by X25 adapter    */
#define  X_26   26                      /* Timeout on RESTART REQUEST, T20   */
#define  X_27   27                      /* Timeout on RESET REQUEST, T22     */
#define  X_28   28                      /* Timeout on CALL REQUEST, T21      */
#define  X_29   29                      /* Timeout on CLEAR REQUEST, T23     */
#define  X_30   30                      /* DIAGNOSTIC PACKET received        */
#define  X_31   31                      /* RESET INDICATION rxd              */

#define  X_32   32                      /* RESTART REQUEST rxd               */
#define  X_33   33                      /* RESET INDICATION by X25 adapter   */
#define  X_34   34                      /* CLEAR INDICATION by X25 adapter   */
#define  X_35   35                      /* RESTART INDICATION by X25 adapter */
#define  X_36   36                      /* Timeout on RESTART INDICATION,T10 */
#define  X_37   37                      /* Timeout on RESET INDICATION, T12  */
#define  X_38   38                      /* Timeout on CALL INDICATION, T11   */
#define  X_39   39                      /* Timeout on CLEAR INDICATION, T13  */

                                        /* ---- Sub-Codes for N2 alerts ---- */
#define  CF_FR_N2    1                  /* FRMR sent N2 times                */
#define  CF_DISC_N2  2                  /* DISC sent N2 times                */
#define  CF_SS_N2    3                  /* SABM sent N2 times                */
#define  CF_T1_N2    4                  /* T1 expired N2 times               */
#define  CF_T4_N2    5                  /* T4 expired N2 times               */
#define  CF_INFO_N2  6                  /* Same INFO frame sent N2 times     */

                                        /* -- Filtering selection options -- */
#define  PHYSICAL_FILTER 0x01           /* 1 = physical filtering on, 0=off  */
#define  FRAME_FILTER    0x02           /* 1 = frame filtering on,    0=off  */
#define  PACKET_FILTER   0x04           /* 1 = packet filtering on,   0=off  */

                                        /* --- Call direction constants ---- */
                                        /* For candsm_save_addresses calls   */
#define  CALL_OUTGOING   0              /* Locally initiated call            */
#define  CALL_INCOMING   1              /* Remote initiated call             */
#define  ADDR_UPDATE     2              /* Indicates update addrs from accpt */


#define  T_CANDSM        50             /* Clock ref tick event no for mtr   */
#define  CANDSM_PERIOD   5              /* 5 long time intervals == 5 secs   */

#define  ALERTMSG        77             /* Some number to use for alert msg  */


struct crd_alert_hdr_struct
{
  byte    type;                 /* alert number                              */
  byte    line;                 /* line number                               */
  ushort  lcn;                  /* lcn (0xffff) for physical & frame         */
  ushort  tick;                 /* timer tick in secs                        */
};
typedef struct crd_alert_hdr_struct crd_alert_hdr_t;

struct crd_alert_ref_code_struct
{
  byte    mode;                 /* DCE or DTE mode                           */
  byte    state;                /* state of layer                            */
  byte    misc[10];             /* alert dependent data                      */
};
typedef struct crd_alert_ref_code_struct crd_alert_ref_code_t;

struct crd_alert_call_struct
{
  byte    direction;            /* local (0) or remote (1) initiated         */
  byte    vc_type;              /* PVC (0) or SVC (1)                        */
  byte    calling[8];           /* packed BCD                                */
  byte    called[8];            /* packed BCD                                */
};
typedef struct crd_alert_call_struct crd_alert_call_t;

struct crd_csm_struct
{
  crd_alert_hdr_t       header;
  crd_alert_ref_code_t  ref_code;
  crd_alert_call_t      call;

  union
  {
    struct
    {
      byte    cause;                            /* cause from packet         */
      byte    diag;                             /* diagnostic from packet    */
    } type_1;

    struct
    {
      ushort  timeout;                          /* timeout in seconds        */
      byte    retries;                          /* number of retries         */
    } type_2;

    struct
    {
      byte    diag;                             /* diagnostic from packet    */
      byte    explanation[3];                   /* explanation code from pkt */
    } type_3;

  } type_info;

};
typedef struct crd_csm_struct crd_csm_t;


#endif
