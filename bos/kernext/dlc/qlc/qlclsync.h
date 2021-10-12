/* @(#)61  1.4  src/bos/kernext/dlc/qlc/qlclsync.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:05:39 */
#ifndef _H_QLCLSYNC
#define _H_QLCLSYNC
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

/* Start of declarations for qlclsync.c                                      */
#ifdef _NO_PROTO


/*****************************************************************************/
/* Function     QLM_START_LS                                                 */
/*                                                                           */
/* Description  This procedure is called when the user issues a Start_LS     */
/*              ioctl. It checks that the SAP is able to support the LS, and */
/*              allocates resources for the LS. The LS is initialised.       */
/*                                                                           */
/* Return       qlm_rc_type     rc;                                          */
/*                                                                           */
/* Input        channel_id_type  channel_id;                                 */
/*              start_ls_ioctl_ext_type *parm_block_ptr;                     */
/*                                                                           */
/* Output       On successful completion the parm block will have been       */
/*              updated by the insertion of a qllc_ls_correlator. A new LS   */
/*              will have been added to the LS linked list for the SAP.      */
/*****************************************************************************/
enum qlm_rc_type qlm_start_ls();

/*****************************************************************************/
/* Function     QLM_INITIALISE_STATION                                       */
/*                                                                           */
/* Description  This procedure is used by the QLM_Start_LS procedure to      */
/*              initialise the fields in the station record.                 */
/*                                                                           */
/* Return       void                                                         */
/*                                                                           */
/* Input        station_type *station_ptr;                                   */
/*              channel_id_type channel_id;                                  */
/*              start_ls_ioctl_ext_type *ext_ptr;                            */
/*                                                                           */
/* Output       On successful completion the station record will have been   */
/*              initialised from the config information in the Start_LS ext  */
/*                                                                           */
/*****************************************************************************/
void   qlm_initialise_station();


/*****************************************************************************/
/* Function     qlm_halt_ls                                                  */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
void qlm_halt_ls();

/*****************************************************************************/
/* Function     qlm_query_ls                                                 */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_query_ls();

/*****************************************************************************/
/* Function     qlm_trace                                                    */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_trace();

/*****************************************************************************/
/* Function     qlm_test                                                     */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_test();

/*****************************************************************************/
/* Function     qlm_alter                                                    */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_alter();

/*****************************************************************************/
/* Function     qlm_alter_station_role                                       */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_alter_station_role();

/*****************************************************************************/
/* Function     qlm_contact                                                  */
/*                                                                           */
/* Description                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_contact();

/*****************************************************************************/
/* Function     qlm_enter_local_busy                                         */
/*                                                                           */
/* Description  This procedure is invoked when the user issues an            */
/*              Enter Local Busy Ioctl. It is called by the QDH.             */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   ext containing qllc sap and ls correlators                   */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_enter_local_busy();

/*****************************************************************************/
/* Function     qlm_exit_local_busy                                          */
/*                                                                           */
/* Description  This procedure is invoked when the user issues an            */
/*              Exit Local Busy Ioctl. It is called by the QDH.              */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Parameters   ext containing qllc sap and ls correlators                   */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_exit_local_busy();
/*****************************************************************************/
/* Function     qlm_write_normal_data                                        */
/*                                                                           */
/* Description  This procedure is invoked when the user issues a             */
/*              Write Command with an extension that indicates data          */
/*              type normal.                                                 */
/* Return                                                                    */
/*                                                                           */
/* Parameters                                                                */
/*                                                                           */
/*****************************************************************************/
enum qlm_rc_type qlm_write_normal_data();



#else

extern qlm_rc_type qlm_start_ls(
  channel_id_type channel_id,
  struct qlc_sls_arg *qlc_ext_ptr);

extern void qlm_initialise_station(
  station_type *station_ptr,
  channel_type *channel_id,
  struct qlc_sls_arg *qlc_ext_ptr);

extern void  qlm_convert_sna_facs_to_cb_fac(
  struct sna_facilities_type  *sna_facs,
  cb_fac_t *cb_facs);

extern qlm_rc_type  qlm_halt_ls(
  correlator_type qllc_ls_correlator,
  boolean silent,
  boolean send_disc);

extern qlm_rc_type qlm_query_ls(
  struct dlc_qls_arg *ext_ptr);

extern qlm_rc_type qlm_trace(
  struct dlc_trace_arg  *ext_ptr);

extern qlm_rc_type qlm_test(
  struct dlc_corr_arg *ext_ptr);

extern qlm_rc_type qlm_alter(
  struct dlc_alter_arg *ext_ptr);

extern qlm_rc_type qlm_contact(
  struct dlc_corr_arg  *ext_ptr);

extern qlm_rc_type qlm_enter_local_busy(
  struct dlc_corr_arg *ext);

extern qlm_rc_type  qlm_exit_local_busy(
  struct dlc_corr_arg *ext);

extern qlm_rc_type qlm_write(
  write_ext_type *ext,
  gen_buffer_type *buffer_ptr);

#endif /* _NO_PROTO */
/* End of declarations for qlclsync.c                                        */

#endif

