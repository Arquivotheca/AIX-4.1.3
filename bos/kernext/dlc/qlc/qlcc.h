/* @(#)51  1.5  src/bos/kernext/dlc/qlc/qlcc.h, sysxdlcq, bos411, 9428A410j 9/23/93 18:05:42 */
#ifndef _H_QLCC
#define _H_QLCC
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

#define SAPE_RESULT_EXT_LENGTH         (8)
#define STAS_RESULT_EXT_LENGTH        (48)
#define CONTENTION_RESULT_EXT_LENGTH   (4)
#define MAXIMUM_SAPS_DEFAULT           (1)

/*****************************************************************************/
/* The return codes used by the Channel Management functions all belong to   */
/* an enumerated type, qcm_rc_type.                                          */
/*****************************************************************************/
enum qcm_rc_type
{
  qcm_rc_ok,

  qcm_rc_alloc_failed,
  qcm_rc_channel_not_found,
  qcm_rc_device_not_found,
  qcm_rc_free_failed,
  qcm_rc_invalid_command,
  qcm_rc_invalid_flags,
  qcm_rc_invalid_user_type,
  qcm_rc_local_busy,
  qcm_rc_no_data,
  qcm_rc_port_error,
  qcm_rc_retry,
  qcm_rc_sap_limit_reached,
  qcm_rc_system_error
};
typedef enum qcm_rc_type qcm_rc_type;


/*****************************************************************************/
/* The type of user who opened a channel is stored in the user_nature field  */
/* in the channel control block. This field takes one of the values from the */
/* user_nature_type enumeration.                                             */
/*****************************************************************************/
enum user_nature_type
{
  application_user,
  kernel_user
};
typedef enum user_nature_type user_nature_type;

enum result_indicators_type
{
  test_completed  = 0x08000000,
  sap_enabled     = 0x04000000,
  sap_disabled    = 0x02000000,
  station_started = 0x01000000,
  station_halted  = 0x00800000,
  inact_wo_term   = 0x00200000,
  inact_ended     = 0x00100000,
  contacted       = 0x00080000
  };
typedef enum result_indicators_type result_indicators_type;

enum result_code_type
{
  successful                                   =     0,

/* Defect 103644 */
  test_completed_ok                            =     0,
  protocol_error                               =  -906,
  bad_data_compare_on_test                     =  -908,
  no_remote_buffering                          =  -910,
  remote_initiated_discontact                  =  -912,
  discontact_abort_timeout                     =  -914,
  inactivity_timeout                           =  -916,
  mid_session_reset                            =  -918,
  cannot_find_remote_name                      =  -920,
  invalid_remote_name                          =  -922,
  session_limit_exceeded                       =  -924,
  listen_already_in_progress                   =  -926,
  link_station_unusual_network_condition       =  -928,
  link_station_resource_outage                 =  -930,
  inactivity_without_termination               =  -932,
  inactivity_has_ended                         =  -934,
  remote_name_already_connected                =  -936,

  local_name_already_in_use                    =  -901,
  invalid_local_name                           =  -903,
  sap_unusual_network_condition                =  -905,
  sap_resource_outage                          =  -907,
  user_interface_error                         =  -909,
  error_detected_in_code                       =  -911,
  sna_system_error                             =  -913,
  sna_path_control_does_not_exist              =  -915
};
typedef enum result_code_type result_code_type;

/*****************************************************************************/
/* Network Data buffers                                                      */
/*****************************************************************************/
enum type_of_netd_t
{
  transit_delay,
  charging_info
};
typedef enum type_of_netd_t type_of_netd_t;

struct charging_info_t
{
  unsigned int   dummy;
};
typedef struct charging_info_t charging_info_t;

union netd_data_t
{
  unsigned int   transit_delay;
  struct charging_info_t  charging_info;
};
typedef union netd_data_t netd_data_t;



union result_extension_type
{
  struct
  {
    unsigned int  max_write_netd_length;
  } sape;
  struct
  {
    unsigned int  maximum_i_field_size;
    unsigned int  reserve_1[3];
    unsigned int  remote_station_id_length;
    char          remote_station_id[DLC_MAX_NAME];
    unsigned int  reserve_2[1];
    unsigned int  write_data_offset;
  } stas;
  struct
  {
    unsigned int  conflicting_user_ls_correlator;
  } stah;
};

struct get_exception_ioctl_ext_type
{
  correlator_type  user_sap_correlator;
  correlator_type  user_ls_correlator;
  unsigned int result_indicators;        /* enum result_indicators_type      */
  unsigned int result_code;              /* enum result_code_type            */
  unsigned int result_extension_length;
  union  result_extension_type  result_extension;
};
typedef struct get_exception_ioctl_ext_type get_exception_ioctl_ext_type;


/*****************************************************************************/
/* Throughout QLLC a general type is used for the buffers. This type is used */
/* to implement the User Buffers and DH Buffers found in the QLLC Buffer     */
/* Management Design.                                                        */
/*****************************************************************************/
typedef gen_buffer_type *data_queue_type;


/*****************************************************************************/
/* Exceptions are queeud in the channel for application users. The queue     */
/* consists of a linked list of exceptions, so an imedding structure is      */
/* defined, pending_exception_type, which contains the address of the        */
/* exception block that it represents, together with the link management     */
/* pointers.                                                                 */
/*****************************************************************************/
struct  pending_exception_type
{
  struct dlc_getx_arg *exception_data;
  struct pending_exception_type *prev_pending_exception_ptr;
  struct pending_exception_type *next_pending_exception_ptr;
};
typedef struct pending_exception_type pending_exception_type;
/*****************************************************************************/
/* And the queue is typedeffed as follows.                                   */
/*****************************************************************************/
typedef pending_exception_type *exception_queue_type;

/*****************************************************************************/
/* The criteria which are tested on a select call take the following type.   */
/*****************************************************************************/
typedef unsigned int select_criteria_type;
/*****************************************************************************/
/* The status of the select state of the channel is defined as follows.      */
/*****************************************************************************/
typedef struct
{
/* Defect 101380 
  struct proc *user_proc;
*/
  select_criteria_type selected_events;
} select_status_type;

/*****************************************************************************/
/* A Channel is represented by a control block which has the following type. */
/*****************************************************************************/
struct channel_type
{
  unsigned int                 lock;
  dev_t                        devno;            /* QLLC's devno             */
  struct channel_type         *channel_id;
  unsigned int                 devflag;
/* Defect 101380 
  struct proc                 *proc_id;
*/
  pid_t				proc_id;	
  unsigned int                 user_nature;    /* enum user_nature_type      */
  unsigned long                maximum_saps;
  struct sap_type             *sap_list_ptr;   /* anchor to sap_list         */
  struct port_type            *port_id;

  /***************************************************************************/
  /* Function addresses for the kernel services functions. These are the     */
  /* functions which the kernel user supplies, and which are called on       */
  /* QLLC's port servicing kernel process to notify the user of incoming     */
  /* data, or exception conditions.                                          */
  /***************************************************************************/
  function_address_type        rx_normal_data_fn_addr;
  function_address_type        rx_xid_data_fn_addr;
  function_address_type        rx_netd_data_fn_addr;
  function_address_type        exception_fn_addr;

  /***************************************************************************/
  /* Application users are not called by QLLC's kernel process. Instead      */
  /* incoming data and exceptions are queued in the channel, and they are    */
  /* notified of their presence by select and wakeup.                        */
  /***************************************************************************/
  exception_queue_type         exception_queue;
  jsmlist_t                    data_queue;
  select_status_type           select_status;
  /* read_status - used to remember that the user is sleeping on read        */
  /* readsleep is proc * of reading process that is slept upon.              */
/* Defect 101380
  struct proc                  *readsleep;
*/
  int				readsleep;
  unsigned int                 read_status;

  /***************************************************************************/
  /* Channel control blocks are held in a linked list. Each control block    */
  /* therefore contains the link pointers to chain it to the adjacent        */
  /* channels.                                                               */
  /***************************************************************************/
  struct channel_type         *prev_channel_ptr;
  struct channel_type         *next_channel_ptr;
};
typedef struct channel_type channel_type;
typedef channel_type *channel_id_type;

/*****************************************************************************/
/* The channel list and the port list are both anchored in the channel_list  */
/* which is a global structure which contains the two anchors and a lock.    */
/*****************************************************************************/
struct channel_list_type
{
  unsigned int lock;
  channel_type *channel_ptr;
  struct port_type *port_ptr;
  unsigned int debug_control;
};
typedef struct channel_list_type channel_list_type;

/*****************************************************************************/
/* MACROS:                                                                   */
/*****************************************************************************/
/*****************************************************************************/
/* QCM_RECEIVE_QUEUES_ARE_NOT_EMPTY                                          */
/* Accepts a channel_id and returns TRUE if there is data on at least        */
/* one of the receive queues in the channel.                                 */
/*****************************************************************************/
#define QCM_RECEIVE_QUEUES_ARE_NOT_EMPTY(channel_id) \
  (((channel_id)->data_queue != (gen_buffer_type *)NULL))

/*****************************************************************************/
/* QCM_EXCEPTION_QUEUE_IS_NOT_EMPTY                                          */
/* Accepts a channel_id and returns TRUE if there is an exception on the     */
/* exception queue in the channel.                                           */
/*****************************************************************************/
#define QCM_EXCEPTION_QUEUE_IS_NOT_EMPTY(channel_id) \
  ((channel_id)->exception_queue != (pending_exception_type *)NULL)

/*****************************************************************************/
/* QCM_SET_SELECTED_EVENTS                                                   */
/* Accepts a channel_id and sets the selected events field in the select     */
/* status structure.                                                         */
/*****************************************************************************/
#define QCM_SET_SELECTED_EVENTS(channel_id,events) \
  ((channel_id)->select_status.selected_events = (events))

/*****************************************************************************/
/* Macro        QCM_RETURN_PORT_ID                                           */
/* Description  This macro returns the Port_ID that the channel is using.    */
/* Return       port_type *                                                  */
/* Input        channel_id_type    channel_id;                               */
/* Output       none                                                         */
/*****************************************************************************/
#define QCM_RETURN_PORT_ID(channel_id) (port_type *)((channel_id)->port_id)

/*****************************************************************************/
/* The qlcc.c module provides the following functions.                       */
/*****************************************************************************/

/* Start of declarations for qlcc.c                                          */
#ifdef _NO_PROTO

qcm_rc_type qcm_allocate_channel();
qcm_rc_type qcm_free_channel();
qcm_rc_type qcm_open_channel();
qcm_rc_type qcm_close_channel();
qcm_rc_type qcm_read_data_queue();
qcm_rc_type qcm_read_exception_queue();
qcm_rc_type qcm_receive_data();
qcm_rc_type qcm_select();
qcm_rc_type qcm_make_result();
qcm_rc_type qcm_make_sape_result();
qcm_rc_type qcm_make_stas_result();
qcm_rc_type qcm_make_contention_result();
qcm_rc_type qcm_make_netd_buffer();
unsigned int qcm_saps_using_port();
qcm_rc_type qcm_check_channel();
void qcm_port_error();

#else

extern qcm_rc_type qcm_allocate_channel(
  channel_id_type *channel_id);

extern qcm_rc_type qcm_free_channel (
  channel_id_type channel_id);

extern qcm_rc_type qcm_open_channel(
  channel_id_type channel_id,
  struct dlc_open_ext *open_extension,
  unsigned long open_flag);

extern qcm_rc_type  qcm_close_channel (
  channel_id_type channel_id);

extern qcm_rc_type  qcm_read_data_queue(
  channel_id_type  channel_id,
  gen_buffer_type  **buffer_ptr,
  read_ext_type  *read_ext_ptr);

extern qcm_rc_type qcm_read_exception_queue(
  channel_id_type channel_id,
  struct dlc_getx_arg **exc_ptr);

extern qcm_rc_type qcm_receive_data(
  channel_id_type channel_id,
  gen_buffer_type *buffer_ptr);

extern qcm_rc_type qcm_select(
  channel_id_type channel_id,
  unsigned short  selected_events,
  unsigned short *available_events);

extern qcm_rc_type qcm_make_result (
  channel_id_type channel_id,
  correlator_type  user_sap_correlator,
  correlator_type  user_ls_correlator,
  result_indicators_type result_indicators,
  result_code_type result_code);

extern qcm_rc_type  qcm_make_sape_result (
  channel_id_type  channel_id,
  correlator_type  user_sap_correlator,
  correlator_type  user_ls_correlator,
  unsigned int     max_write_netd_length);

extern qcm_rc_type  qcm_make_stas_result (
  channel_id_type channel_id,
  correlator_type user_sap_correlator,
  correlator_type user_ls_correlator,
  unsigned int    maximum_i_field_size,
  unsigned int    remote_station_id_length,
  char            *remote_station_id);

extern qcm_rc_type qcm_make_contention_result (
  channel_id_type   channel_id,
  correlator_type   user_sap_correlator,
  correlator_type   user_ls_correlator,
  correlator_type    conflicting_user_ls_correlator);

extern qcm_rc_type qcm_check_channel(
  channel_id_type channel_id);

extern void qcm_port_error(
  port_type *port_id);

#endif /* _NO_PROTO */
/* End of declarations for qlcc.c                                            */

#endif



