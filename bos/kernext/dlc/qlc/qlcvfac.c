static char sccsid[] = "@(#)87	1.5  src/bos/kernext/dlc/qlc/qlcvfac.c, sysxdlcq, bos411, 9428A410j 11/2/93 09:23:43";
/*
 * COMPONENT_NAME: (sysxdlcq) X.25 QLLC module
 *
 * FUNCTIONS: qvm_convert_sna_facs_to_cb_fac
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

#include "qlcg.h"                   /* correlator_type                       */
	                            /* diag_tag_type                         */
	                            /* trace_channel_type                    */
	                            /* x25_address_type                      */
	                            /* lcn_type                              */
	                            /* ras_counter_type                      */
#include "qlcq.h"
#include "qlcv.h"
#include "qlcvfac.h"                /* facilities types                      */
#include "qlcb.h"                   /* gen_buffer_type                       */
#include "qlcvbuf.h"
#include "qlcp.h"                   /* port_type                             */
#include "qlcvpkt.h"

/*****************************************************************************/
/* Function     QVM_CONVERT_SNA_FACS_TO_CB_FAC                               */
/*                                                                           */
/* Description  This function accepts a pointer to a sna facilities struct   */
/*              (which is the struct used by the user to pass facilities to  */
/*              QLLC at the end of the Start_LS arg block), and it converts  */
/*              that into a cb_fac_t internal structure.                     */
/*                                                                           */
/* Parameters:  sna_facs is a ptr to facs in start extension    source       */
/*              cb_facs is a ptr to facs area in station        destination  */
/*****************************************************************************/
void  qvm_convert_sna_facs_to_cb_fac(

  struct sna_facilities_type  *sna_facs,
  cb_fac_t *cb_facs)
{
  int no_of_pad_bytes;

  outputf("FACS_PARSER: ...has been called\n");
  /***************************************************************************/
  /* Always initialise cb_facs->flags to zero, then turn on just those facs  */
  /* that are requested.                                                     */
  /***************************************************************************/
  cb_facs->flags = 0;

  if (sna_facs->facs == FALSE)
  {
    outputf("FACS_PARSER: facs flag is false returning...\n");
    /*************************************************************************/
    /* there are no facilities requested. You can ignore the remainder of    */
    /* the sna facilities structure.                                         */
    /*************************************************************************/
    return;
  }
  /***************************************************************************/
  /* Read the sna_facs bit fields determining which facs have been           */
  /* requested. Each requested fac must be "copied" into the cb_fac struct   */
  /***************************************************************************/
  if (sna_facs->revc == TRUE)
  {
    outputf("FACS_PARSER: REVC is true\n");
    /*************************************************************************/
    /* Reverse charging has been selected. Build the appropriate request into*/
    /* cb_facs.                                                              */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_REV_CHRG;
  }
  if (sna_facs->rpoa == TRUE)
  {
    outputf("FACS_PARSER: RPOA is true\n");
    /*************************************************************************/
    /* Recognised Private Operating Agency/ies have been requested.          */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_RPOA;
    cb_facs->rpoa_id_len = (unsigned int)sna_facs->rpoa_id_count;
    cb_facs->rpoa_id = sna_facs->rpoa_id;
  }
  if (sna_facs->psiz == TRUE)
  {
    outputf("FACS_PARSER: PSIZ is true\n");
    /*************************************************************************/
    /* Non-default packet size has been requested.                           */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_PSIZ;
    cb_facs->psiz_clg = sna_facs->recipient_tx_psiz;
    cb_facs->psiz_cld = sna_facs->originator_tx_psiz;
  }
  if (sna_facs->wsiz == TRUE)
  {
    outputf("FACS_PARSER: WSIZ is true\n");
    /*************************************************************************/
    /* Non-default window size has been requested.                           */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_WSIZ;
    cb_facs->wsiz_clg = sna_facs->recipient_tx_wsiz;
    cb_facs->wsiz_cld = sna_facs->originator_tx_wsiz;
  }
  if (sna_facs->tcls == TRUE)
  {
    outputf("FACS_PARSER: TCLS is true\n");
    /*************************************************************************/
    /* Non-default throughput class has been requested.                      */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_TCLS;
    cb_facs->tcls_cld = sna_facs->recipient_tx_tcls;
    cb_facs->tcls_clg = sna_facs->originator_tx_tcls;
  }
  if (sna_facs->cug == TRUE)
  {
    outputf("FACS_PARSER: CUG is true\n");
    /*************************************************************************/
    /* A Closed User Group has been requested.                               */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_CUG;
    cb_facs->cug_id = (unsigned short)(sna_facs->cug_index);
  }
  if (sna_facs->cugo == TRUE)
  {
    outputf("FACS_PARSER: CUGO is true\n");
    /*************************************************************************/
    /* A Closed User Group with Outgoing Access has been requested.          */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_OA_CUG;
    cb_facs->cug_id = (unsigned short)(sna_facs->cug_index);
  }
  if (sna_facs->nui == TRUE)
  {
    outputf("FACS_PARSER: NUI is true\n");
    /*************************************************************************/
    /* Network User Identification is being supplied with the call.          */
    /* Build the appropriate request into cb_facs.                           */
    /*************************************************************************/
    cb_facs->flags |= X25FLG_NUI_DATA;
    cb_facs->nui_data_len = sna_facs->nui_length;
    cb_facs->nui_data = sna_facs->nui_data;
  }
}

	   /* THINK YOU CAN DELETE ALL THE FUNCTIONS BELOW */

/*****************************************************************************/
/* Function:    sna_fac_originator_tx_pkt_size                               */
/*                                                                           */
/* Parameters:  sna_facilities  - a pointer to the facilities requests       */
/*                                formatted as for the link station config   */
/*                                area.                                      */
/*              x25_port        - pointer to the X.25 port instance.         */
/*                                                                           */
/* Return Code: This function returns the packet size requested for          */
/*              transmission by the calling link station. If the facilities  */
/*              supplied by the caller as a parameter to the function specify*/
/*              a non-default packet size, then the transmit packet size     */
/*              requested in those facilities is returned. Otherwise the     */
/*              default packet size is returned. The return value is of type */
/*              x25_pkt_size_type.                                           */
/*                                                                           */
/* Interface:   x25_pkt_size_type sna_fac_originator_tx_pkt_size (           */
/*                  sna_facilities,                                          */
/*                  x25_port                                                 */
/*              )                                                            */
/*              sna_facilities_type  *sna_facilities;                        */
/*              x25_port_type        *x25_port;                              */
/*                                                                           */
/*****************************************************************************/
/* x25_pkt_size_type sna_fac_originator_tx_pkt_size(); */


/******************************************************************************
** CONVERT SNA FACILITIES TO X.25 FORMAT
** Function:    sna_fac_x25_format
**
** Description: This function takes a set of facilities requests, in the format
**              provided by SNA Services as part of the Open Link Station
**              config area. The function formats the facilities requests into
**              the format specified by X.25, and places the coded facilities
**              into a structure of x25_facilities_type. The user must provide
**              a pointer to such a structure.
**
** Parameters:  sna_facilities  - a pointer to the facilities requests
**                                formatted as for the link station config
**                                area.
**              x25_facilities  - pointer to an empty structure to be used
**                                to hold the returned facilities, after they
**                                have been converted to X.25 format.
**
** Return Code: function returns a pointer to the modified X.25 facilities
**              structure, or NULL if an error is detected.
**
** Interface:   x25_facilities_type sna_fac_x25_format (
**              sna_facilities_type         *sna_facilities,
**              x25_facilities_type         *x25_facilities**
******************************************************************************/
/* struct x25_facilities_type *sna_fac_x25_format() */


/******************************************************************************
** X.25 NON DEFAULT PACKET SIZE
** Function:    x25_fac_non_default_pkt_size
** Description: This function takes a set of facilities requests, in the X.25
**              encoded format, and returns a boolean, indicating TRUE if the
**              facilities include a request for non-default packet size, or
**              FALSE if there is no non-default packet size request.
**
** Parameters:  x25_facilities  - pointer to an structure containing the
**                                encoded X.25 facilities to be searched for a
**                                non-default packet size request.
**
** Return Code: TRUE            - if the facilities do contain a packet size
**                                request.
**              FALSE           - if the facilities do not contain a packet
**                                size request.
**
** Interface:   bool x25_fac_non_default_pkt_size (
**   x25_facilities_type *x25_facilities
***************************************************************************/
/* bool x25_fac_non_default_pkt_size() */


/******************************************************************************
** X.25 FACILITIES GET ORIGINATOR TRANSMIT PACKET SIZE
** Function:    x25_fac_get_originator_tx_pkt_size
** Description: This function takes a set of facilities requests, in the X.25
**              encoded format, and returns a value indicating the packet size
**              requested for transmission from the calling to the called DTE.
**
**              The caller is expected to use x25_fac_non_default_pkt_siz(
**              determine whether there is a PSIZ request in the facilities,
**              so this function will return zero if no such request is found.
**
** Parameters:  x25_facilities  - pointer to an structure containing the
**                                encoded X.25 facilities to be searched for a
**                                non-default packet size request.
**
** Return Code: A value containing the X.25 packet size for data transmitted
**              from the calling to the called DTE. The function will return
**              zero if there it cannot find a PSIZ facilities request (**
** Interface:   x25_pkt_size_type x25_fac_get_originator_tx_pkt_size (
**              x25_facilities_type  *x25_facilities
***************************************************************************/
/* x25_pkt_size_type x25_fac_get_orig_tx_pkt_size() */

/******************************************************************************
** X.25 FACILITIES GET RECIPIENT TRANSMIT PACKET SIZE
** Function:    x25_fac_get_recipient_tx_pkt_size
** Description: This function takes a set of facilities requests, in the X.25
**              encoded format, and returns a value indicating the packet size
**              requested for transmission from the called to the calling DTE.
**
**              The caller is expected to use x25_fac_non_default_pkt_siz(
**              determine whether there is a PSIZ request in the facilities,
**              so this function will return zero if no such request is found.
**
** Parameters:  x25_facilities  - pointer to an structure containing the
**                                encoded X.25 facilities to be searched for a
**                                non-default packet size request.
**
** Return Code: A value containing the X.25 packet size for data transmitted
**              from the called to the calling DTE. The function will return
**              zero if there it cannot find a PSIZ facilities request (**
** Interface:   x25_pkt_size_type x25_fac_get_recipient_tx_pkt_size (
**              x25_facilities_type     *x25_facilities**
******************************************************************************/
/* x25_pkt_size_type x25_fac_get_recip_tx_pkt_size()  */

/******************************************************************************
** X.25 FACILITIES ALTER PACKET SIZE
** Function:    x25_fac_alter_pkt_size
** Description: This function takes a set of facilities requests, in the X.25
**              encoded format, and the packet sizes requested for transmission
**              by the originator and the recipient. The function locates the
**              packet size request in the facilities, and alters the requested
**              packet sizes according to the values supplied as parameters.
**
**              The caller is expected to use x25_fac_non_default_pkt_siz(
**              determine whether there is a PSIZ request in the facilities,
**              so this function will return zero if no such request is found.
**
**              The function uses the local utility x25_fac_find_fac_code(**
** Parameters:  x25_facilities  - pointer to an structure containing the
**                                encoded X.25 facilities to be searched for a
**                                non-default packet size request.
**              org_tx_psize    - the packet size requested for transmission
**                                by the originator (
**                                recipient (
**              rec_tx_psize    - the packet size requested for transmission
**                                by the recipient (
**                                originator (**
** Return Code: A pointer to the modified facilities. The function will return
**              zero if there it cannot find a PSIZ facilities request (**
** Interface:   x25_facilities_type *x25_fac_alter_pkt_size (
**                  x25_facilities_type  *x25_facilities,
**                  x25_pkt_size_type    org_tx_psize,
**                  x25_pkt_size_type    rec_tx_psize**
******************************************************************************/
/* x25_facilities_type *x25_fac_alter_pkt_size()    */






