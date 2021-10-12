/* @(#)58  1.4  src/bos/kernext/dlc/qlc/qlcl.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:04:06 */
#ifndef _H_QLCL
#define _H_QLCL
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
/* Define constants used in QLM sub-component                                */
/*****************************************************************************/
#define SILENT      (TRUE)

/*****************************************************************************/
/* All the return codes used by Link Station Management functions belong to  */
/* an emumerted type, qlm_rc_type.Define return code type.                   */
/*****************************************************************************/
enum qlm_rc_type
{
  qlm_rc_ok = 0,

  qlm_rc_alloc_failed,
  qlm_rc_bad_listen_name,
  qlm_rc_buffer_unavailable,
  qlm_rc_channel_not_authorised,
  qlm_rc_close_vc_failed,
  qlm_rc_contention,
  qlm_rc_invalid_inact_bits,
  qlm_rc_invalid_remote_name,
  qlm_rc_invalid_set_mode_request,
  qlm_rc_open_vc_failed,
  qlm_rc_port_error,
  qlm_rc_protocol_error,
  qlm_rc_system_error,
  qlm_rc_station_already_local_busy,
  qlm_rc_station_not_contacted,
  qlm_rc_station_not_local_busy,
  qlm_rc_station_limit_reached,
  qlm_rc_station_not_found,
  qlm_rc_user_interface_error
};
typedef enum qlm_rc_type qlm_rc_type;

/*****************************************************************************/
/* The Link Station can be in one of the following states.                   */
/*****************************************************************************/
enum station_state_type
{
  opening,
  opened,
  inactive,
  closing
};

/*****************************************************************************/
/* The link station has sub-states that are represented by the following     */
/* bitfield                                                                  */
/*****************************************************************************/
struct station_sub_state_type
{
  unsigned   calling      : 1;
  unsigned   listening    : 1;
  unsigned   contacted    : 1;
  unsigned   local_busy   : 1;
  unsigned   remote_busy  : 1;
};


/*****************************************************************************/
/* RAS counters                                                              */
/*****************************************************************************/
struct ras_counter_block_type
{
  ras_counter_type test_commands_sent;
  ras_counter_type test_command_failures;
  ras_counter_type test_commands_received;
  ras_counter_type seq_data_packets_transmitted;
  ras_counter_type seq_data_packets_retransmitted;
  ras_counter_type max_contiguous_retransmissions;
  ras_counter_type seq_data_packets_received;
  ras_counter_type invalid_packets_received;
  ras_counter_type adapter_detected_rx_errors;
  ras_counter_type adapter_detected_tx_errors;
  ras_counter_type receive_inactivity_timeouts;
  ras_counter_type command_polls_sent;
  ras_counter_type command_repolls_sent;
  ras_counter_type command_contiguous_repolls;
};
typedef struct ras_counter_block_type ras_counter_block_type;

/*****************************************************************************/
/* QLLC uses a control block definition for Start_LS which is a structure    */
/* that includes the generic start control block, from gdlextcb.h, and also  */
/* contains protocol specific fields which occupy the area left at the end   */
/* of the generic parameter block for such protocol dependencies.            */
/*****************************************************************************/
/*
struct qlc_sls_arg
{
  struct dlc_sls_arg    dlc;   
  struct qlc_start_psd  psd;
};
*/
struct  timer_control_type
{
  unsigned long  inact_timer_id;       /* was cba_id type. 4th Oct */
  unsigned long  retry_timer_id;       /* was cba_id type. 4th Oct */
};
typedef struct timer_control_type timer_control_type;

struct ras_counters_type
{
  ras_counter_type  test_commands_sent;
  ras_counter_type  test_command_failures;
  ras_counter_type  test_commands_received;
  ras_counter_type  inactivity_timeouts;
};
typedef struct ras_counters_type ras_counters_type;

struct retry_indicators_type
{
  boolean  normal_data_retry_pending;
  boolean  xid_data_retry_pending;
  boolean  netd_data_retry_pending;
};
typedef struct retry_indicators_type retry_indicators_type;

/*****************************************************************************/
/* A Link Station is represented by a control block which contains the QLLC  */
/* Finite State Machine and X.25 Virtual Circuit control blocks that apply   */
/* to this station.                                                          */
/* Link Stations are held in a linked list and each station contains the     */
/* list management pointers to join it to the adjacent stations.             */
/*                                                                           */
/* The control block structure is broken down it several sub-sections.       */
/* The first of these sections is the configuration information that is      */
/* supplied by the user in the Start LS argument, or returned to the user    */
/* by QLLC in the Start LS argument.                                         */
/* The second section contains fields that are purley for internal use. They */
/* are initialised by the QLLC Link Station Manager, on Start_LS, and are    */
/* used to monitor the state of various aspects of the Link Station, or to   */
/* hold data values, buffers, timer information, etc.                        */
/*****************************************************************************/
struct station_type
{
  /***************************************************************************/
  /* Fields passed by user on start, or returned on started                  */
  /***************************************************************************/
  correlator_type            qllc_ls_correlator;
  diag_tag_type              station_tag;
  correlator_type            qllc_sap_correlator;
  correlator_type            user_ls_correlator;
  unsigned long              flags;
  trace_channel_type         trace_channel;
  unsigned int               remote_addr_len;
  char                       remote_addr[DLC_MAX_NAME];
  unsigned int               max_i_field;
  unsigned int               receive_window_size;
  unsigned int               transmit_window_size;
  unsigned int               max_repoll;
  unsigned int               repoll_time;
  unsigned int               inact_time;
  unsigned int               force_halt_time;
  cb_fac_t                   facilities;

  /***************************************************************************/
  /* Internal variables and controls                                         */
  /***************************************************************************/
  int                         lock;
  int                         listen_accepted_pdg_start_done;
  struct watchdog             inact_dog;
  unsigned int                inactivity_detected;
  struct watchdog             halt_dog;
  unsigned int                forced_halt_due;
  struct watchdog             retry_dog;
  unsigned int                retry_pending;
  unsigned short              netid;
  unsigned short              support_level;
  char                        listen_name[8];
  struct channel_type        *channel_id;
  unsigned long               station_state;     /* enum station_state_type  */
  unsigned long               station_sub_state;
  int                         reason_for_closure;
	                            /* values of sna_operations_result_type  */
  boolean                     silent_halt;
  struct ras_counters_type    ras_counters;
  struct x25_vc_type          virt_circuit;
  struct qllc_ls_type         link_station;
  /***************************************************************************/
  /* The station contains a data queue for norm, xidd, and netd buffers      */
  /* because it can be put into local busy mode either through a busy return */
  /* from the channel manager on a norm receive, or a retry return on either */
  /* norm, xidd or netd.                                                     */
  /***************************************************************************/
  jsmlist_t                      receive_data_queue;
  struct retry_indicators_type   retry_indicators;
  /***************************************************************************/
  /* And the station contaisns the link management pointers.                 */
  /***************************************************************************/
  struct station_type         *prev_station_ptr;
  struct station_type         *next_station_ptr;
};
typedef struct station_type station_type;
typedef station_type *station_address_type;
/*
enum write_flags_type
{
  norm = 0x80000000,
  xidd = 0x40000000,
  netd = 0x10000000,
  oflo = 0x00000002,
  rspp = 0x00000001
};
*/
struct write_ext_type
{
  correlator_type  qllc_sap_correlator;
  correlator_type  qllc_ls_correlator ;
  unsigned int     write_flags; 
};
typedef struct write_ext_type write_ext_type;


#endif
