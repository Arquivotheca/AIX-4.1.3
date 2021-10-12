/* @(#)30	1.6  src/bos/kernext/c327/dftnsSubr.h, sysxc327, bos411, 9428A410j 10/31/90 14:36:38 */
#ifndef _H_DFTNSSUBR
#define _H_DFTNSSUBR
/*
 * COMPONENT_NAME: (SYSXC327) c327 dft Subroutine header file
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "dftnsDcl.h"

/*****************************************************************************/
/* Declarations of public functions in dftnsSubr.c                           */
/* This file should be included in dftnsCmd.c and dftnsOffl.c                */
/*****************************************************************************/
/*dftnsSubr.c */
void UnprotectedSetTimer(DDS_DATA *, int, TIMER_TYPE, int);
void SetTimer(DDS_DATA *, int, TIMER_TYPE, int);
void BuildAsyncReq(uchar, uchar, uchar, uchar, uchar, uchar, ASYNC_REQ *);
void CopyAsyncReq(ASYNC_REQ *, ASYNC_REQ *);
void InterruptDH(DDS_DATA *, int, INTR_TYPE, int, int);
void BroadcastUnSol(DDS_DATA *, int, int);
void AckCmd(DDS_DATA *, int, INTR_TYPE);
SEND_REQ_RESULT ProcessAsyncReq(DDS_DATA *, ASYNC_REQ *);
void EnqueAsyncRequest(DDS_DATA *, ASYNC_REQ *);
SEND_REQ_RESULT SendAS(DDS_DATA *, ASYNC_REQ *);
void SendNextAS(DDS_DATA *);
void SendSS (DDS_DATA *, uchar , uchar, uchar);
void XmitData (DDS_DATA *, int, boolean);
void InitCard (DDS_DATA *);
void ShutDown(DDS_DATA *);
int ValidNetID (NETWORK_ID, DDS_DATA **, int *);
int dftnsSend (NETWORK_ID, int);
int dftnsOpen (DDS_DATA *);
/* dftnsOffl.c */
void RecvData(DDS_DATA *, int, RECV_CTRL);
void ProcessCTCCS(DDS_DATA *, int);
void RecvCmd(DDS_DATA *, int, uchar);
void ProcessWCUS(DDS_DATA *);
void AckAsync(DDS_DATA *);
void StartOp(DDS_DATA *);
void AckAllCmd(DDS_DATA *);
void ForceClose(DDS_DATA *);
void Restart (DDS_DATA *);
void dftnsProcessInterrupt(DDS_DATA *); 
void dftnsProcessTimeout(DDS_DATA *);
void dftnsPIOError(DDS_DATA *);
void TimerSendES (DDS_DATA *);
int dftnsTimer(void);

#endif /* _H_DFTNSSUBR */

