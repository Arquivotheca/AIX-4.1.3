static char sccsid[] = "@(#)70  1.6  src/bos/kernext/dlc/qlc/qlcpcons.c, sysxdlcq, bos411, 9432A411a 7/10/94 19:47:35";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "qlcg.h"
#include "qlcq.h"
#include "qlcv.h"  
#include "qlcpcons.h"  
#include "qlcb.h"
#include "qlcp.h"
/* #include "qlcc.h"    */

/*****************************************************************************/
/* Function     QPM_MAKE_PVC_START_DATA                                      */
/*                                                                           */
/* Description  This procedure initialises a start data structure            */
/*              so that it can be used to issue a start for a PVC via        */
/*              the QPM_Start procedure.                                     */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   struct x25_start_data *start_data                            */
/*              others..                                                     */
/*                                                                           */
/*****************************************************************************/
void qpm_make_pvc_start_data (

  struct x25_start_data *start_data,
  unsigned short netid,
  diag_tag_type session_name,
  int protocol,
  int logical_channel)
{
  outputf("QPM_MAKE_PVC_START: netid being used  = %d\n",netid);
  outputf("QPM_MAKE_PVC_START: session name used = %s\n",session_name);
  outputf("QPM_MAKE_PVC_START: protocol in use   = %d\n",protocol);
  outputf("QPM_MAKE_PVC_START: logical_channel   = %s\n",logical_channel);

  start_data->sb.netid = netid;
  start_data->sb.status = NULL;
  strncpy (
    start_data->session_name,
    session_name,
    DIAG_TAG_LENGTH
    );
  start_data->session_type = SESSION_PVC;
  start_data->session_protocol = protocol;
  start_data->counter_id = -1;              /* counters are not used by QLLC */
  start_data->session_type_data.logical_channel = logical_channel;
}

/*****************************************************************************/
/* Function     QPM_MAKE_SVC_START_DATA                                      */
/*                                                                           */
/* Description  This procedure initialises a start data structure            */
/*              so that it can be used to issue a start for an SVC via       */
/*              the QPM_Start procedure.                                     */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   struct x25_start_data *start_data                            */
/*              others..                                                     */
/*                                                                           */
/*****************************************************************************/
void qpm_make_svc_start_data (

  struct x25_start_data *start_data,
  unsigned short netid,
  diag_tag_type  session_name,
  int            session_typ,
  int            protocol,
  unsigned short call_id)
{
  outputf("QPM_MAKE_SVC_START: netid being used  = %d\n",netid);
  outputf("QPM_MAKE_SVC_START: session name used = %s\n",session_name);
  outputf("QPM_MAKE_SVC_START: protocol in use   = %d\n",protocol);
  outputf("QPM_MAKE_SVC_START: session_typ       = %s\n",session_typ);

  start_data->sb.netid = netid;
  start_data->sb.status = NULL;
  strncpy (
    start_data->session_name,
    session_name,
    DIAG_TAG_LENGTH
    );
  start_data->session_type = session_typ;
  start_data->session_protocol = protocol;
  if (session_typ == SESSION_SVC_IN)
  {
    outputf("QPM_MAKE_SVC_START: setting call_id to %x\n",call_id);
    start_data->session_type_data.call_id = call_id;
  }
  start_data->counter_id = -1;              /* counters are not used by QLLC */
}

/*****************************************************************************/
/* Function     QPM_MAKE_LISTEN_START_DATA                                   */
/*                                                                           */
/* Description  This procedure initialises a start data structure            */
/*              so that it can be used to issue a start for a listening      */
/*              station using the QPM_Start procedure.                       */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   struct x25_start_data *start_data                            */
/*              others..                                                     */
/*                                                                           */
/*****************************************************************************/
void qpm_make_listen_start_data (

  struct x25_start_data *start_data,
  unsigned short netid,
  diag_tag_type session_name,
  int protocol,
  char *listen_name)
{
  outputf("QPM_MAKE_LISTEN_START: netid being used  = %d\n",netid);
  outputf("QPM_MAKE_LISTEN_START: session name used = %s\n",session_name);
  outputf("QPM_MAKE_LISTEN_START: protocol in use   = %d\n",protocol);
  outputf("QPM_MAKE_LISTEN_START: listen_name is    = %s\n",listen_name);

  start_data->sb.netid = netid;
  start_data->sb.status = NULL;
  strncpy (
    start_data->session_name,
    session_name,
    DLC_MAX_DIAG
    );
  start_data->session_id = NULL;             /* session_id is returned by dh */
  start_data->session_type = SESSION_SVC_LISTEN;
  start_data->session_protocol = protocol;
  start_data->counter_id = -1;              /* counters are not used by QLLC */

  bzero(start_data->session_type_data.listen_name,
	sizeof(start_data->session_type_data.listen_name));
  strncpy(start_data->session_type_data.listen_name,listen_name,8);
}

/*****************************************************************************/
/* Function     QPM_MAKE_HALT_DATA                                           */
/*                                                                           */
/* Description  This procedure initialises a halt data structure             */
/*              so that it can be used to issue a halt on a session, via     */
/*              the QPM_Halt procedure.                                      */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   struct x25_halt_data *halt_data                              */
/*              others..                                                     */
/*                                                                           */
/*****************************************************************************/
void qpm_make_halt_data (

  struct x25_halt_data *halt_data,
  unsigned short netid,
  unsigned short session_id)
{
  outputf("QPM_MAKE_HALT: netid being used  = %d\n",netid);
  outputf("QPM_MAKE_HALT: session id used = %d\n",session_id);

  halt_data->sb.netid = netid;
  halt_data->sb.status = NULL;
  halt_data->session_id = session_id;
}

/*****************************************************************************/
/* Function     QPM_MAKE_WRITE_EXT                                           */
/*                                                                           */
/* Description  This procedure initialises a write_ext structure             */
/*              so that it can be used to issue a write.                     */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   struct x25_write_ext *write_ext                              */
/*              session_id                                                   */
/*              net_id                                                       */
/*****************************************************************************/
void qpm_make_write_ext (

  struct x25_write_ext *write_ext,
  unsigned short session_id,
  unsigned short netid)
{
  /* zero out we.flag so we don't get transmit done status blocks */
  bzero( write_ext, sizeof(struct x25_write_ext) );

  write_ext->we.status = NULL;
  write_ext->we.netid = netid;
  write_ext->session_id = session_id;
}

/*****************************************************************************/
/* Function     QPM_MAKE_REJECT_DATA                                         */
/*                                                                           */
/* Description  This procedure initialises a reject data structure           */
/*              so that it can be used to reject an incoming call, via       */
/*              the QPM_Reject procedure.                                    */
/*                                                                           */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Parameters   struct x25_reject_data *reject_data                          */
/*              others..                                                     */
/*                                                                           */
/*****************************************************************************/
void qpm_make_reject_data (

  struct x25_reject_data *reject_data,
  unsigned short netid,
  unsigned short session_id,
  unsigned short call_id)
{
  reject_data->sb.netid = netid;
  reject_data->sb.status = NULL;
  reject_data->session_id = session_id;
  reject_data->call_id = call_id;

  outputf("MAKE_REJECT_DATA: netid      = %d\n",netid);
  outputf("MAKE_REJECT_DATA: session_id = %d\n",session_id);
  outputf("MAKE_REJECT_DATA: call_id    = %d\n",call_id);
  outputf("MAKE_REJECT_DATA: status     = NULL\n");

}
