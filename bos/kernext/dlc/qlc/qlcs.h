/* @(#)82  1.6  src/bos/kernext/dlc/qlc/qlcs.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:22:25 */
#ifndef _H_QLCS
#define _H_QLCS
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

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define MAX_WRITE_NETD_LENGTH   (0)

/*****************************************************************************/
/* Function     QSM_RETURN_USER_SAP_CORRELATOR                               */
/*                                                                           */
/* Description  This macro takes a qllc sap correlator and returns           */
/*              the corresponding user sap correlator                        */
/*              The sap correlator is in fact a ptr to the sap control block.*/
/*              The conditional operator checks that the sap is valid, else  */
/*              a NULL value is returned.                                    */
/*                                                                           */
/* Return       user sap correlator if found                                 */
/*              null if sap not found                                        */
/*                                                                           */
/* Input                                                                     */
/*              correlator                                                   */
/*****************************************************************************/
#define QSM_RETURN_USER_SAP_CORRELATOR(correlator) \
  ( \
    ((sap_type *)(correlator))->qllc_sap_correlator == (correlator) \
     ? ((sap_type *)(correlator))->user_sap_correlator : 0 \
  )

/*****************************************************************************/
/* Type definitions                                                          */
/*****************************************************************************/
/*****************************************************************************/
/* The return codes used by the SAP Management functions are all members of  */
/* an enumerated type, qsm_rc_type.                                          */
/*****************************************************************************/
enum qsm_rc_type
{
  qsm_rc_ok,

  qsm_rc_alloc_failed,
  qsm_rc_listener_already_started,
  qsm_rc_max_link_stations_invalid,
  qsm_rc_no_such_sap,
  qsm_rc_query_failed,
  qsm_rc_sap_limit_reached,
  qsm_rc_station_limit_reached,
  qsm_rc_system_error
};
typedef enum qsm_rc_type qsm_rc_type;

/*****************************************************************************/
/* The parameter passed to QSM_Enable_SAP to configure the SAP is defined by */
/* the type Enable_SAP_Ioctl_Ext_Type. This type is exported to the QDH.     */
/*****************************************************************************/


/*****************************************************************************/
/* The parameter passed to QSM_Query_SAP to identify the SAP to be queried.  */
/* This type is exported to the QDH.                                         */
/*****************************************************************************/
struct qlc_qsap_arg
{
  struct dlc_qsap_arg       gdlc_field;
  struct qlc_query_sap_psd  psd;
};


/*****************************************************************************/
/* The current_sap_state field in the Query_SAP_Ioctl_Ext_Type and in the    */
/* SAP_Type can be assigned values defined in enum sap_state                 */
/*****************************************************************************/
enum sap_state_type
{
  sap_opening = 1,
  sap_opened = 2,
  sap_closing = 3
};
typedef enum sap_state_type sap_state_type;

/*****************************************************************************/
/* Finally, the SAP_Type is defined as follows.                              */
/*****************************************************************************/
struct sap_type
{
  int                  lock;
  struct channel_type  *channel_id;          /* ID of channel which owns SAP */
  correlator_type      qllc_sap_correlator;
  correlator_type      user_sap_correlator;
  /***************************************************************************/
  /* limit to num of stations that this SAP can support. Passed user at      */
  /* Enable_SAP.                                                             */
  /***************************************************************************/
  unsigned int         max_link_stations;
  char                 local_x25_address[DLC_MAX_NAME];
  unsigned int         sap_state;            /* enum sap_state_type          */
  unsigned int         max_write_netd_length;
  struct station_type *station_list_ptr;     /* anchor to linked list (stns) */
  struct sap_type     *prev_sap_ptr;         /* Pointers for managing linked */
  struct sap_type     *next_sap_ptr;         /* list of SAPs                 */
};
typedef struct sap_type sap_type;


/* Start of declarations for qlcs.c                                          */
#ifdef _NO_PROTO

/*****************************************************************************/
/* The qlcs.c module provides the following functions.                       */
/*****************************************************************************/
qsm_rc_type qsm_enable_sap();
qsm_rc_type qsm_disable_sap();
qsm_rc_type qsm_query_sap();
qsm_rc_type qsm_check_sap();
sap_type   *qsm_find_sap_given_correlator();

#else

extern qsm_rc_type qsm_enable_sap(
  channel_id_type channel_id,
  struct dlc_esap_arg *ext_ptr);

extern qsm_rc_type  qsm_disable_sap (
  channel_id_type       channel_id,
  correlator_type       qllc_sap_correlator,
  boolean               silent);

extern qsm_rc_type qsm_query_sap (
  channel_type        *channel_id,
  struct qlc_qsap_arg *qlc_ext_ptr);

extern qsm_rc_type qsm_check_sap(
  sap_type *sap_ptr);

extern sap_type *qsm_find_sap_given_correlator(
  correlator_type sap_correlator,
  boolean *unlock);

#endif /* _NO_PROTO */
/* End of declarations for qlcs.c                                            */

#endif
