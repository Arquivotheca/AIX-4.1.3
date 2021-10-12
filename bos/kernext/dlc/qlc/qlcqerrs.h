/* @(#)73  1.3  src/bos/kernext/dlc/qlc/qlcqerrs.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:13:03 */
#ifndef _H_QLCQERRS
#define _H_QLCQERRS
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
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
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>

/******************************************************************************
** The folowing constants are used to pass error information from the QLLC
** code modules to the error handlers. Initially these error IDs are defined
** as strings, which will be written to stdout by the eror handling macros.
******************************************************************************/

#if !defined(VRM)

/******************************************************************************
** QLLC LOCAL ERROR CODES:
******************************************************************************/
#define Q_LERROR_LLTOX "function: qllc_lltox\nstate = INOP\n"
#define Q_LERROR_PVC_RESET "function: qllc_pvc_reset\nstate = INOP\n"
#define Q_LERROR_LSTRT "function: qllc_lstrt\ninvalid state\n"
#define Q_LERROR_LSTOP_1 "function: qllc_lstop\nincoming test or xid response pending\n"
#define Q_LERROR_LSTOP_2 "function: qllc_lstop\ninvalid state\n"
#define Q_LERROR_XCHID_1 "function: qllc_xchid\nrole = PRIMARY\n"
#define Q_LERROR_XCHID_2 "function: qllc_xchid\nrole = SECONDARY\n"
#define Q_LERROR_COM_CMD_PRI "function: qllc_common_cmd_pri\ncommand received by primary\n"
#define Q_LERROR_COM_RSP_SEC "function: qllc_common_rsp_sec\nresponse received by secondary\n"
#define Q_LERROR_QSM_SEC "function: qllc_qsm_sec\nqsm received by secondary station in INOP state\n"
#define Q_LERROR_QDISC_SEC "function: qllc_qdisc_sec\nqdisc received by secondary station in INOP state\n"
#define Q_LERROR_QXID_CMD_SEC "function: qllc_qxid_cmd_sec\nqxid cmd received by secondary station in INOP state\n"
#define Q_LERROR_QXID_RSP_PRI "function: qllc_qxid_rsp_pri\nqxid rsp received by primary station in INOP state\n"
#define Q_LERROR_QTEST_RSP_PRI "function: qllc_qtest_rsp_pri\nqtest rsp received by primary station in INOP state\n"
#define Q_LERROR_QRR "function: qllc_qrr\nqrr received by primary station in INOP state\n"
#define Q_LERROR_QUA "function: qllc_qua\nqua received by primary station in INOP state\n"
#define Q_LERROR_QRD "function: qllc_qrd\nqrd received by primary station in INOP state\n"
#define Q_LERROR_QDM "function: qllc_qdm\nqdm received by primary station in INOP state\n"
#define Q_LERROR_QFRMR "function: qllc_qfrmr\nqfrmr received by primary station in INOP state\n"
#define Q_LERROR_LTEST_PRI "function: qllc_ltest\nprimary station in invalid state\n"
#define Q_LERROR_LTEST_SEC "function: qllc_ltest\nsecondary station in invalid state\n"
#define Q_LERROR_SDATA "function: qllc_sdata\nattempt to send data in wrong state\n"
#define Q_LERROR_L3RDY "function: qllc_l3rdy\npacket level ready in CLOSED state\n"
#define Q_LERROR_L3NOP "function: qllc_l3nop\npacket level inoperative in INOP state\n"
#define Q_LERROR_ERRPK "function: qllc_errpk\nerroneous packet received in INOP state\n"
#define Q_LERROR_RDATA "function: qllc_rdata\ndata received in INOP state\n"


/******************************************************************************
** QLLC REMOTE ERROR CODES:
******************************************************************************/
#define Q_RERROR_COM_CMD_PRI "function: qllc_common_cmd_pri\ncommand received by primary\n"
#define Q_RERROR_COM_RSP_SEC "function: qllc_common_rsp_sec\nresponse received by secondary\n"
#define Q_RERROR_QSM_SEC "function: qllc_qsm_sec\nqsm received by secondary station in CLOSED/(OXRP OR OTRP) state\n"
#define Q_RERROR_XID_CMD_SEC_1 "function: qllc_qxid_cmd_sec\nqxid received in CLOSING state\n"
#define Q_RERROR_XID_CMD_SEC_2 "function: qllc_qxid_cmd_sec\nqxid received in RECOVERY state\n"
#define Q_RERROR_TEST_CMD_SEC_1 "function: qllc_qtest_cmd_sec\nqtest received in CLOSED state\n"
#define Q_RERROR_TEST_CMD_SEC_2 "function: qllc_qtest_cmd_sec\nqtest received in CLOSING state\n"
#define Q_RERROR_TEST_CMD_SEC_3 "function: qllc_qtest_cmd_sec\nqtest received in RECOVERY state\n"
#define Q_RERROR_XID_RSP_PRI_1 "function: qllc_qxid_rsp_pri\nqxid received in OPENING or CLOSING state\n"
#define Q_RERROR_XID_RSP_PRI_2 "function: qllc_qxid_rsp_pri\nqxid received in CLOSED state, pred != IXRP\n"
#define Q_RERROR_TEST_RSP_PRI_1 "function: qllc_qtest_rsp_pri\nqtest received in OPENING or CLOSING state\n"
#define Q_RERROR_TEST_RSP_PRI_2 "function: qllc_qtest_rsp_pri\nqtest received in CLOSED state, pred != ITRP\n"
#define Q_RERROR_UA "function: qllc_qua\nqua received in CLOSED or OPENED state\n"
#define Q_RERROR_RD "function: qllc_qrd\nqrd received in CLOSED state\n"
#define Q_RERROR_ERRPK "function: qllc_errpk\nerroneous packet received\n"
#define Q_RERROR_RDATA "function: qllc_rdata\ndata received in invalid state\n"


/******************************************************************************
** QLLC PROGRAM ERROR CODES:
******************************************************************************/
#define Q_PERROR_LLTOX "function: qllc_lltox\nsecondary link station timeout expiry\n"
#define Q_PERROR_REPOLL "function: qllc_repoll\npending command set is empty\n"
#define Q_PERROR_LSTRT "function: qllc_lstrt\nprimary station in RECOVERY state\n"
#define Q_PERROR_LSTOP "function: qllc_lstop\nprimary station in RECOVERY state\n"
#define Q_PERROR_CLR_PEND "function: qllc_clear_pending_cmd\ncannot be invoked with secondary station\n"
#define Q_PERROR_LTEST "function: qllc_ltest\nprimary station in RECOVERY state\n"
#define Q_PERROR_ERRPK "function: qllc_errpk\nprimary station in RECOVERY state\n"
#define Q_PERROR_QXID_RSP_PRI "function: qllc_qxid_rsp_pri\nprimary station in RECOVERY state\n"
#define Q_PERROR_QTEST_RSP_PRI "function: qllc_qtest_rsp_pri\nprimary station in RECOVERY state\n"

#else /* VRM is defined, therefore a different error handling method is used */
/******************************************************************************
** The error codes for VRM error handling are defined here:
******************************************************************************/

/******************************************************************************
** QLLC LOCAL ERROR CODES:
******************************************************************************/
#define Q_LERROR_LLTOX             (0x00)
#define Q_LERROR_PVC_RESET         (0x01)
#define Q_LERROR_LSTRT             (0x02)
#define Q_LERROR_LSTOP_1           (0x03)
#define Q_LERROR_LSTOP_2           (0x04)
#define Q_LERROR_XCHID_1           (0x05)
#define Q_LERROR_XCHID_2           (0x06)
#define Q_LERROR_COM_CMD_PRI       (0x07)
#define Q_LERROR_COM_RSP_SEC       (0x08)
#define Q_LERROR_QSM_SEC           (0x09)
#define Q_LERROR_QDISC_SEC         (0x0a)
#define Q_LERROR_QXID_CMD_SEC      (0x0b)
#define Q_LERROR_QXID_RSP_PRI      (0x0c)
#define Q_LERROR_QTEST_RSP_PRI     (0x0d)
#define Q_LERROR_QRR               (0x0e)
#define Q_LERROR_QUA               (0x0f)
#define Q_LERROR_QRD               (0x10)
#define Q_LERROR_QDM               (0x11)
#define Q_LERROR_QFRMR             (0x12)
#define Q_LERROR_LTEST_PRI         (0x13)
#define Q_LERROR_LTEST_SEC         (0x14)
#define Q_LERROR_SDATA             (0x15)
#define Q_LERROR_L3RDY             (0x16)
#define Q_LERROR_L3NOP             (0x17)
#define Q_LERROR_ERRPK             (0x18)
#define Q_LERROR_RDATA             (0x19)

/******************************************************************************
** QLLC REMOTE ERROR CODES:
******************************************************************************/
#define Q_RERROR_COM_CMD_PRI       (0x00)
#define Q_RERROR_COM_RSP_SEC       (0x02)
#define Q_RERROR_QSM_SEC           (0x03)
#define Q_RERROR_XID_CMD_SEC_1     (0x04)
#define Q_RERROR_XID_CMD_SEC_2     (0x05)
#define Q_RERROR_TEST_CMD_SEC_1    (0x06) 
#define Q_RERROR_TEST_CMD_SEC_2    (0x07) 
#define Q_RERROR_TEST_CMD_SEC_3    (0x08)
#define Q_RERROR_XID_RSP_PRI_1     (0x09) 
#define Q_RERROR_XID_RSP_PRI_2     (0x0a)
#define Q_RERROR_TEST_RSP_PRI_1    (0x0b) 
#define Q_RERROR_TEST_RSP_PRI_2    (0x0c)
#define Q_RERROR_UA                (0x0d)
#define Q_RERROR_RD                (0x0e)
#define Q_RERROR_ERRPK             (0x0f)
#define Q_RERROR_RDATA             (0x10)

/******************************************************************************
** QLLC PROGRAM ERROR CODES:
******************************************************************************/
#define Q_PERROR_LLTOX             (0x00)
#define Q_PERROR_REPOLL            (0x01)
#define Q_PERROR_LSTRT             (0x02)
#define Q_PERROR_LSTOP             (0x03)
#define Q_PERROR_CLR_PEND          (0x04)
#define Q_PERROR_LTEST             (0x05)
#define Q_PERROR_ERRPK             (0x06)
#define Q_PERROR_QXID_RSP_PRI      (0x07)
#define Q_PERROR_QTEST_RSP_PRI     (0x08)


#endif



/******************************************************************************
** QLLC LOCAL ERROR
** This macro is called each time a QLLC local protocol error is discovered.
** The macro will invoke error logging procedures. Initially this will write an
** error message to stdout, but in production code, it will make an entry in
** the VRM error log.
******************************************************************************/
#ifdef QLLC_ERROR_LOGGING_ON
#define QLLC_LOCAL_ERROR(msg, corr) \
   (void)outputf("====> QLLC local protocol error, station %d: %s\n",corr,msg)
#else
#define QLLC_LOCAL_ERROR(msg,corr)
#endif

/******************************************************************************
** QLLC REMOTE ERROR
** This macro is called each time a QLLC remote protocol error is discovered.
** The macro will invoke error logging procedures. Initially this will write an
** error message to stdout, but in production code, it will make an entry in
** the VRM error log.
******************************************************************************/
#ifdef QLLC_ERROR_LOGGING_ON
#define QLLC_REMOTE_ERROR(msg, corr) \
  (void)outputf("====> QLLC remote protocol error, station %d: %s\n",corr,msg)
#else
#define QLLC_REMOTE_ERROR(msg,corr) 
#endif

/******************************************************************************
** QLLC PROGRAM ERROR
** This macro is called each time a QLLC programming error is discovered.
** The macro will invoke error logging procedures. Initially this will write an
** error message to stdout, but once the code is running under VRM, it will
** make an entry in the VRM error log. In production code this may be turned
** off altogether, as it is assumed that programming errors will have been
** removed !!!
******************************************************************************/
#ifdef QLLC_ERROR_LOGGING_ON
#define QLLC_PROGRAM_ERROR(msg, corr) \
    (void)outputf("====> QLLC program error, station %d: %s\n", corr, msg)
#else
#define QLLC_PROGRAM_ERROR(msg,corr)
#endif

/******************************************************************************
** QLLC DIAGNOSTIC
** This macro is called each time a reset or clear packet is received by a QLLC
** link station. The macro is used to log the cause and diagnostic values from 
** the packet. Initially these will be written to stdout, but once the code is 
** running under VRM, it will make an entry in the VRM error log. 
******************************************************************************/
#ifdef QLLC_ERROR_LOGGING
#define QLLC_DIAGNOSTIC(cause, diagnostic, corr) \
    (void)printf("====> QLLC link station %d cleared/reset:\n\tcause:\t\t0x%02X\n\tdiagnostic:\t0x%02X\n", corr, cause, diagnostic)
#else
#define QLLC_DIAGNOSTIC(cause, diagnostic, corr)
#endif

#endif
