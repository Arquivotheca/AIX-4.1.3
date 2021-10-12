/* @(#)16  1.4  src/bos/kernext/dlc/qlc/qlcctrc.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:32:45 */

#ifndef _H_QLCCTRC
#define _H_QLCCTRC
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


struct trace_start_block_type
{
  char		station_tag[DLC_MAX_DIAG];
  char		remote_addr[DLC_MAX_NAME];
};
typedef struct trace_start_block_type trace_start_block_type;

struct trace_halt_block_type
{
  char		station_tag[DLC_MAX_DIAG];
  char		remote_addr[DLC_MAX_NAME];
  unsigned int	result_code;
};
typedef struct trace_halt_block_type trace_halt_block_type;

/*****************************************************************************/
/* Trace function prototype                                                  */
/*****************************************************************************/
void  q_trace(
  correlator_type  correlator,
  unsigned int     hook_word,
  unsigned int     timeout_type,
  gen_buffer_type *buffer_ptr,
  int              op_result
  );

/*****************************************************************************/
/* This Macro is used to determine whether tracing is turned on, and if it   */
/* isn't the actual trace macros don't call q_trace                          */
/*****************************************************************************/
#define TRACING_ON(station_ptr) \
((station_ptr) && (station_ptr->flags & DLC_TRCO))

#define  TRACE_START(station_ptr) \
if (TRACING_ON(station_ptr)) \
q_trace(station_ptr->qllc_ls_correlator,HKWD_SYSX_DLC_START,NULL,NULL,0)

#define  TRACE_HALT(station_ptr,op_result) \
if (TRACING_ON(station_ptr)) \
q_trace(station_ptr->qllc_ls_correlator,HKWD_SYSX_DLC_HALT,NULL,NULL,op_result)

#define  TRC_QLLC_XID		0x01
#define  TRC_QLLC_DATA		0x02

#define  TRACE_XID_XMIT(station_ptr,data_ptr) \
if (TRACING_ON(station_ptr)) \
q_trace(station_ptr->qllc_ls_correlator,HKWD_SYSX_DLC_XMIT,TRC_QLLC_XID,data_ptr,0)

#define  TRACE_DATA_XMIT(station_ptr,data_ptr) \
if (TRACING_ON(station_ptr)) \
q_trace(station_ptr->qllc_ls_correlator,HKWD_SYSX_DLC_XMIT,TRC_QLLC_DATA,data_ptr,0)

#define  TRACE_RECV(station_ptr,data_ptr) \
if (TRACING_ON(station_ptr)) \
q_trace(station_ptr->qllc_ls_correlator,HKWD_SYSX_DLC_RECV,NULL,data_ptr,0)

#define  TRACE_TIMER(station_ptr,timeout_type) \
if (TRACING_ON(station_ptr)) \
q_trace(station_ptr->qllc_ls_correlator,HKWD_SYSX_DLC_TIMER,timeout_type,NULL,0)

#endif

