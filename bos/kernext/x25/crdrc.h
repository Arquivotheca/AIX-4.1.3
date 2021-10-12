/* @(#)72  1.2  src/bos/kernext/x25/crdrc.h, sysxx25, bos411, 9428A410j 6/15/90 18:50:02 */
#ifndef _H_CRDRC
#define _H_CRDRC
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
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
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */

#define crdrc_h


enum crd_rc_enum
{
  crd_awaiting_incoming_reset,
  crd_awaiting_outgoing_reset,

  crd_in_wrong_state,

  crd_lms_data,
  crd_csm_data,
  crd_reset_state,

  crd_cant_on_this_lun,
  crd_not_in_write_state,

  crd_link_down,
  crd_link_up,
  crd_rcm_not_running,

  crd_ok,
  crd_ok_so_far,
  crd_bad_card,
  crd_undefined_card,
  crd_bad_task,
  crd_task_running,
  crd_no_mem,
  crd_bad_task_hdr,
  crd_bad_task_pri,
  crd_task_fail,
  crd_already_defined,
  crd_not_present,
  crd_not_c2x,
  crd_write_ready,
  crd_write_not_ready,
  crd_no_server_data,
  crd_invalid,
  crd_not_initialised,
  crd_lun_open,
  crd_bad_lun,
  crd_slun_closed,
  crd_lms_open,
  crd_bad_adaptor,
  crd_bad_line,
  crd_lun_closed,
  crd_no_pool,
  crd_no_buffers,
  crd_data_trunc,
  crd_bad_init,
  crd_xid_err,
  crd_already_doing,
  crd_alive,
  crd_dump,
  crd_no_vc,
  crd_no_disk_space,
  crd_disk_error,
  crd_d_queue_full,
  crd_d_queue_space,
  crd_z_queue_empty,
  crd_unknown_z,
  crd_incoming_call,
  crd_call_accept,
  crd_data,
  crd_more_data,
  crd_clear_pkt,
  crd_reset_pkt,
  crd_interrupt_pkt,
  crd_interrupt_confirm,
  crd_clear_confirm,
  crd_reset_confirm,
  crd_registration_confirm,
  crd_data_confirm,
  crd_bad_pkt_size,
  crd_bad_pkt,
  crd_vc_clear,
  crd_seq_error,
  crd_cant_read,
  crd_vc_reset,
  crd_ack_outstanding,
  crd_hw_on,
  crd_hw_off,
  crd_hw_already_on,
  crd_hw_already_off,
  crd_frame_on,
  crd_frame_off,
  crd_frame_already_on,
  crd_frame_already_off,
  crd_packet_on,
  crd_packet_off,
  crd_auto_answer,
  crd_read_on_lun,
  crd_write_on_lun,
  crd_dz_write,
  crd_dz_answer,
  crd_dz_read,
  crd_dz_on,
  crd_dz_not_write,
  crd_bad_ioctl,
  crd_bad_d_cmd,
  crd_lun_write,
  crd_invalid_ioctl,
  crd_lun_read,
  crd_pad_1,
  crd_pad_2,
  crd_pad_3,
  crd_pad_4,
  crd_pad_5,
  crd_pad_6,
  crd_coding_error,
  crd_rcm_interrupt,
  crd_unexpected_signal,

  crd_awaiting_incoming_clear,
  crd_awaiting_outgoing_clear,

  crd_undefined_rc
};
typedef enum crd_rc_enum crd_rc_t;


#endif
