/* @(#)27  1.14 src/bos/kernext/c327/dftnsDcl.h, sysxc327, bos411, 9430C411a 7/27/94 09:32:27 */
#ifndef _H_DFTNSDCL
#define _H_DFTNSDCL

/*
 * COMPONENT_NAME: (SYSXC327) c327 dft declarations header file
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef _POWER_MP
#include <sys/lock_def.h>
#else
#include <sys/lockl.h>
#endif


/*****************************************************************************/
/* this file contains the non-public declarations for the dft subroutines    */
/*****************************************************************************/

typedef volatile struct                        /* (private) common area     */
{
  struct
  {                                           /* one of these for this pkg */
#ifdef _POWER_MP
    Complex_lock common_locked;
#else
    lock_t   common_locked;                   /* used by (un)lockl         */
#endif
    int      adapters_in_use;                 /* active dds's in table     */
  } miscellaneous;
  struct
  {                                           /* one for these for each crd*/
    DDS_DATA  *dds_ptr;                       /* entry here if card in use */
  } adapter_info [MAX_MINORS];
} COMMON_AREA;

#define CMNMIS    dftnsCommon.miscellaneous
#define CMNADP(x) dftnsCommon.adapter_info[x]

/*****************************************************************************/
/* return values from SendAS (and, privately, from ProcessAsyncReq) */
/*****************************************************************************/
typedef enum
{
   SEND_REQ_TCA_REJECT = -1,          /* TCA not ready for async status   */
   SEND_REQ_QUE_FULL = 0,             /* async que is full                */
   SEND_REQ_COMPLETE_OK,              /* completed ok                     */
   SEND_REQ_POLL_SENT,                /* poll interrupt sent to cu        */
   SEND_REQ_SENT_TCA_OK               /* async status put in TCA ok       */
} SEND_REQ_RESULT;

/**********************************************************************/
/* control parameter sent to RecvData                                 */
/**********************************************************************/
typedef enum
{
   JUST_DATA,                           /* Data only - move to buf    */
   SEND_BYTE,                           /* 3270 CMD only - Send now   */
   STORE_AND_WAIT                       /* Command w/data coming      */
} RECV_CTRL;

/**********************************************************************/
/****                                                              ****/
/****  Write Local Channel Command (WLCC) - LITERALS               ****/
/****                                                              ****/
/**********************************************************************/
# define WLCC_EAU          0x6f         /* Erase All Unprotected      */
# define WLCC_EWA          0x7e         /* Erase Write Alternate      */
# define WLCC_WRITE        0xf1         /* Write                      */
# define WLCC_RD_BUFFER    0xf2         /* Read Buffer                */
# define WLCC_WSF          0xf3         /* Write Structured Field     */
# define WLCC_EW           0xf5         /* Erase Write                */
# define WLCC_RM           0xf6         /* Read Modified              */

/**********************************************************************/
/****                                                              ****/
/****  START OPERATION Command Values - LITERALS                   ****/
/****                                                              ****/
/**********************************************************************/
# define CNOP              0x01      /* Cause Interrupt on device     */
# define WCUS              0x02      /* Write Control Unit Status     */
# define WDAT              0x03      /* Write Data from Host          */
# define WLCC              0x06      /* Wrt Local Channel CMD         */
# define LOCK              0x07      /* Non-SNA Host Select           */
# define RDAT              0x08      /* Gen Inbound Data for Host     */
# define PDAT              0x0a      /* Prepare rd data for host      */
# define CTCCS             0x0b      /* End chained command seq       */

/**********************************************************************/
/****                                                              ****/
/****  WCUS Command Values - LITERALS                              ****/
/****                                                              ****/
/**********************************************************************/
# define MACHINE_CHECK     0x01      /* Machine Check Event           */
# define COMO_CHECK        0x02      /* Communication Check Event     */
# define PROGRAM_CHECK     0x03      /* Program Check Event           */
# define READY             0x10      /* Ready Event                   */
# define DEVICE_ID         0x20      /* Device ID Status              */
# define COMO_CHK_MIND     0x30      /* Como reminder check status    */
# define COMO_CHK_N_MIND   0x31      /* Como no reminder check status */
# define SNA_ACT           0x40      /* ACTLU has been issued         */
# define SNA_DEACT         0x41      /* DACTLU has been issued        */

/**********************************************************************/
/****                                                              ****/
/****  Expedited Status Values - LITERALS                          ****/
/****                                                              ****/
/**********************************************************************/
# define ES_BUSY           0x02          /* Device busy - please wait */
# define ES_RTM_START      0x04          /* Start response timer      */
# define ES_RTM_STOP       0x06          /* Stop response timer       */
# define ACK               0x00          /* Last ES Acknowledged      */
# define UNACK             0x01          /* Last ES Unacknowledged    */
# define EXFLT_FF          0xff          /* Physical Device           */

/**********************************************************************/
/****                                                              ****/
/****  Synchronous Status Values - LITERALS                        ****/
/****                                                              ****/
/**********************************************************************/
# define SS_FCSE           0x02     /* Function Complete w/sync error */
# define SS_FC             0x04     /* Function Complete              */
# define SS_FCIR           0x06     /* Function Complete w/input req. */
# define SS_ERFR           0x08     /* Error in Function Request      */
# define SS_FRA            0x0a     /* Function Request Aborted       */
# define SS_FCDEF          0x0c     /* Function Complete defered stat */
# define FCSE_01           0x01     /* Device Busy                    */
# define FCSE_03           0x03     /* Command Reject                 */
# define FCSE_04           0x04     /* Intervention Required          */
# define FCSE_06           0x06     /* Op Check                       */
# define SS_BUF_ERROR      0x03     /* Buffer Error        (703)      */
# define SS_BAD_CMD        0x01     /* Bad Command         (701)      */
# define SS_UNS_CMD        0x02     /* Unsupported Command (702)      */
# define SS_CU_TIMEOUT     0x10     /* CU Inactive         (x03)      */
# define SS_PIO_ERROR      0x04     /* permanent PIO error (x04)      */
# define SS_6_MASK         0x0600   /* 06XX MASK           (6xx)      */
# define FCIR_00           0x00     /* RDAT must be issued            */
# define FCIR_01           0x01     /* RDAT doesn't need to be is     */
# define ERFR_00           0x00     /* CUAT  Invalid/Unsupported      */
# define ERFR_01           0x01     /* CUFRV Invalid/Unsupported      */
# define ERFR_04           0x04     /* CUSYN  Sync Error              */
# define ERFR_07           0x07     /* CULTAD invalid/unsupported     */
# define FCDEF_00          0x00     /* Start print bit not set        */
# define FCDEF_01          0x01     /* Start print bit set            */
# define START_PRT_MASK    0X08     /* Start print data mask          */

/**********************************************************************/
/****                                                              ****/
/****  Asynchronous Status Values - LITERALS                       ****/
/****                                                              ****/
/**********************************************************************/
# define AEEP              0x22     /* Inbound Event Pending          */
# define AEDV              0x28     /* Device-CU Local Status         */
# define AESTAT            0x34     /* Async Response to Start Op     */
# define AEDV_01           0x01     /* Put Device LT Online           */
# define AEDV_02           0x02     /* Take Device LT(s) Offline      */
# define AEDV_AD           0xff     /* LT ON/OFF Line Address         */
# define AESTAT_00         0x00     /* Good Completion                */
# define AESTAT_02         0x02     /* Device Error                   */
# define AESTAT_03         0x03     /* Command Reject                 */
# define AESTAT_04         0x04     /* Intervention Required          */
# define AESTAT_05         0x05     /* Data Check                     */
# define AESTAT_06         0x06     /* Op Check                       */
# define ASC_Q             TRUE     /* Move parameters to Queue       */
# define ASC_T             TRUE     /* Move parmaeters to TCA         */
# define ASC_L             TRUE     /* Update logical term. addr.     */

/**********************************************************************/
/****                                                              ****/
/****  SNA Transmission header MPF field literals                  ****/
/****  These values represent bits 4 and 5 of BYTE 0  of TH        ****/
/****                                                              ****/
/**********************************************************************/
#define TH_FIRST_SEG       0x08     /* first segment                   */
#define TH_LAST_SEG        0x04     /* Last segment                    */
#define TH_WHOLE_SEG       0x0c     /* There is only one segment       */
#define TH_MASK            0xF3     /* & with this to clear the 2 bits */ 

#define TH_LENGTH          6        /* TH is 6 bytes                   */
#define RH_LENGTH          3        /* RH is 3 bytes                   */

#define BIND               0x31     /* bind command                    */
#define UNBIND             0x32     /* unbind command                  */     


/**********************************************************************/
/****                                                              ****/
/****  TCA HEADER constants for use with SNA segmenting            ****/
/****  The use of the Header lets the controller know that there   ****/
/****  is still more data and to send another RDAT.                ****/
/****                                                              ****/
/**********************************************************************/
#define HEAD_START_SEG     0x80     /* start of segment               */
#define HEAD_END_SEG       0x40     /* end of segment                 */
#define HEAD_MIDDLE_SEG    0x00     /* middle segment                 */
#define HEAD_ONLY_SEG      0xc0     /* there is only one segment      */

#define HEADER_LENGTH      4        /* Header is 4 bytes              */
#define EBM_MSK            0x10     /* Field in CUSLVL for Enhanced
                                       buffer mgmt support            */ 
#define TCA_FC             99       /* flag used to ack WDAT from tcaread
                                       (SNA only)                     */

#endif /* _H_DFTNSDCL */
