/* @(#)83  1.4  src/bos/kernext/dlc/qlc/qlcv.h, sysxdlcq, bos411, 9428A410j 11/2/93 09:22:59 */
#ifndef _H_QLCV
#define _H_QLCV
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
/* definition of the QLLC repertoire of diagnostic codes                     */
/*****************************************************************************/
#define NORMAL_TERMINATION                    ( 0)
#define INVALID_LLC_TYPE                      (12)
#define QLLC_ERROR_GENERAL                    (80)
#define UNDEFINED_C_FIELD                     (81)
#define UNEXPECTED_C_FIELD                    (82)
#define TIMEOUT_CONDITION                     (87)
#define INVALID_PACKET_TYPE_RECEIVED         (162)
#define INVALID_D_BIT_RECEIVED               (173)
#define DTE_SPECIFIC_GENERAL                 (192)
#define LOCAL_PROCEDURE_ERROR_GENERAL        (224)
#define INVALID_D_BIT_REQUEST                (233)
#define RESET_INDICATION_ON_VIRTUAL_CALL     (234)

/*****************************************************************************/
/* Define the return codes possible from the QVM                             */
/*****************************************************************************/
enum qvm_rc_type
{
  qvm_rc_ok = 0,
  qvm_rc_no_name,
  qvm_rc_port_error,
  qvm_rc_system_error,
  qvm_rc_program_error,
  qvm_rc_circuit_not_opened
};
typedef enum qvm_rc_type qvm_rc_type;

enum session_type
{
  session_svc_out,
  session_svc_listen,
  session_svc_in,
  session_pvc
};
typedef enum session_type session_type;

/*****************************************************************************/
/* It's also useful to have a structure defined to pass call user data around*/
/* x25_cud_type is defined as a structure containing a length field, and a   */
/* pointer to call user data. Note that no space is allocated for the call   */
/* user data, as this is most likely to be kept in a vrm data buffer.        */
/*****************************************************************************/
struct x25_cud_type
{
  byte  length;
  byte  *cud;
};
typedef struct x25_cud_type x25_cud_type;

/*****************************************************************************/
/* The X.25 initialisation record supplied to Start is defined in SEDL as a  */
/* variant record, whose discriminant is the virtual circuit type.           */
/* In C all possible fields are included in the structure, but some will be  */
/* ignored as they're not relevant to the particular virtual circuit type.   */
/*****************************************************************************/
typedef struct
{
  unsigned short         netid;
  enum session_type      circuit;
  diag_tag_type          session_name;
  correlator_type        correlator;
  cb_fac_t               facilities;  
  /***************************************************************************/
  /* Following fields only specified for SVC                                 */
  /***************************************************************************/
  char                   recipient_address[DLC_MAX_NAME]; /* for SVCs        */
  char                   listen_name[8];
  /***************************************************************************/
  /* Following fields only specified for PVC                                 */
  /***************************************************************************/
  channel_reference_type channel_num;          
  /***************************************************************************/
  /* protocol field takes values of PROTOCOL_QLLC_80 or PROTOCOL_QLLC_84     */
  /***************************************************************************/
  int                    protocol;
} init_rec_type;

/*****************************************************************************/
/* The control field in the QLLC header dictates the meaning of a Q-Packet   */
/*****************************************************************************/
enum qllc_q_pkt_type
{
  qsm_cmd = 0x93,
  qdisc_cmd = 0x53,
  qxid_cmd = 0xBF,
  qtest_cmd = 0xF3,
  qrr_cmd = 0xF1,
  qrd_rsp = 0x53,
  qxid_rsp = 0xBF,
  qtest_rsp = 0xF3,
  qua_rsp = 0x73,
  qdm_rsp = 0x1F,
  qfrmr_rsp = 0x97
};

enum command_or_response_type
{
  response = 0x00,
  command = 0xFF
};

enum qfrmr_reason_type
{
  qfr_erroneous_qllu_command,
  qfr_erroneous_qllu_response,
  qfr_response_received_by_secondary_station,
  qfr_qxid_qtest_cmd_received_in_invalid_state
};

enum x25_state_type
{
  xs_closed,
  xs_opening,
  xs_opened,
  xs_closing
};

/*****************************************************************************/
/* Again, the x25_virtual_circuit type is defined as being variant depending */
/* on virtual circuit type. Some fields will therefore be unspecified, if    */
/* they are not relevant to the particular circuit type.                     */
/*                                                                           */
/* The remote address field that used to be relevant to SVCs has been deleted*/
/* as the address is stored in the SNA LS only.                              */
/*****************************************************************************/
struct x25_vc_type
{
  correlator_type          correlator;
  enum session_type        circuit;
  int                      protocol;                /* PROTOCOL_QLLC_80/84   */
  enum x25_state_type      state;
  int                      logical_channel;
  int                      session_type;
  diag_tag_type            session_name;
  unsigned short           session_id;        
  unsigned short           netid;           
  unsigned short           call_id;
  ras_counter_type         data_packets_tx;
  ras_counter_type         data_packets_rx;
  ras_counter_type         invalid_packets_rx;
  ras_counter_type         adapter_rx_errors;
  ras_counter_type         adapter_tx_errors;
  bool                     locally_initiated;    
  bool                     remote_clear;         
  char                     listen_name[8];
};
typedef struct x25_vc_type x25_vc_type;

/*****************************************************************************/
/* Function:    QVM_VC_IS_PVC                                                */
/*                                                                           */
/* Description: Determines whether the virtual circuit is an SVC or PVC.     */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     a boolean  value indicating whether the virtual circuit is a */
/*              PVC (TRUE) or an SVC (FALSE).                                */
/*                                                                           */
/*****************************************************************************/
#define QVM_VC_IS_PVC(virt_circuit) \
  (((virt_circuit)->circuit == session_pvc) ? TRUE : FALSE)

/*****************************************************************************/
/* Function:    QVM_RETURN_SESSION_TYPE                                      */
/*                                                                           */
/* Description: Returns the session_type pertaining to a session.            */
/*              e.g. SESSION_SVC_IN, SESSION_SVC_OUT, etc.                   */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     a session_type value in type int.                            */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_SESSION_TYPE(virt_circuit) \
  ((virt_circuit)->session_type)

/*****************************************************************************/
/* Function:    QVM_RETURN_SESSION_ID                                        */
/*                                                                           */
/* Description: Returns the session_id relating to a virt circuit.           */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     a session_id value, type unsigned short.                     */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_SESSION_ID(virt_circuit) \
  ((virt_circuit)->session_id)

/*****************************************************************************/
/* Function:    QVM_RETURN_DATA_PACKETS_TX                                   */
/*                                                                           */
/* Description: Returns the value of the tx packet counter.                  */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     the value of the data_packets_tx RAS counter                 */
/*              (ras_counter_type).                                          */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_DATA_PACKETS_TX(virt_circuit) \
  ((virt_circuit)->data_packets_tx)

/*****************************************************************************/
/* Function:    QVM_RETURN_DATA_PACKETS_RX                                   */
/*                                                                           */
/* Description: Returns the value of the rx packet counter.                  */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     the value of the data_packets_rx RAS counter                 */
/*              (ras_counter_type).                                          */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_DATA_PACKETS_RX(virt_circuit) \
  ((virt_circuit)->data_packets_rx)

/*****************************************************************************/
/* Function:    QVM_RETURN_INVALID_PACKETS_RX                                */
/*                                                                           */
/* Description: Returns the value of the invalid rx packet counter.          */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     the value of the invalid_packets_rx RAS counter              */
/*              (ras_counter_type).                                          */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_INVALID_PACKETS_RX(virt_circuit) \
  ((virt_circuit)->invalid_packets_rx)

/*****************************************************************************/
/* Function:    QVM_RETURN_ADAPTER_RX_ERRORS                                 */
/*                                                                           */
/* Description: Returns the value of the adapter detected error counter.     */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     the value of the adapter_rx_errors RAS counter               */
/*              (ras_counter_type).                                          */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_ADAPTER_RX_ERRORS(virt_circuit) \
  ((virt_circuit)->adapter_rx_errors)

/*****************************************************************************/
/* Function:    X25_RETURN_ADAPTER_TX_ERRORS                                 */
/*                                                                           */
/* Description: Returns the value of the adapter detected error counter.     */
/*                                                                           */
/* Parameter:   virt_circuit    - a pointer to a virtual circuit instance.   */
/*                                                                           */
/* Returns:     the value of the adapter_tx_errors RAS counter               */
/*              (ras_counter_type).                                          */
/*                                                                           */
/*****************************************************************************/
#define QVM_RETURN_ADAPTER_TX_ERRORS(virt_circuit) \
  ((virt_circuit)->adapter_tx_errors)

#endif

