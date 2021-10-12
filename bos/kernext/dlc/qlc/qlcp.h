/* @(#)69  1.7  src/bos/kernext/dlc/qlc/qlcp.h, sysxdlcq, bos411, 9437B411a 9/13/94 09:42:18 */
#ifndef _H_QLCP
#define _H_QLCP
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
/* Define event bits for interrupt handler.                                  */
/* By convention, start with the msb and work downwards.                     */
/*****************************************************************************/
#define DATA_RECEIVED     (0x80000000)
#define TRANSMIT_AVAIL    (0x40000000)
#define EXCEPTION_ARRIVED (0x20000000)
#define TERMINATION       (0x10000000)
#define TIMER_SERVICE     (0x0F000000)
#define REPOLL_TIMEOUT    (0x08000000)
#define INACT_TIMEOUT     (0x04000000)
#define HALT_TIMEOUT      (0x02000000)
#define RETRY_TIMEOUT     (0x01000000)
/*****************************************************************************/
/* Define the control parameters that will be used to turn local busy mode   */
/* on/off.                                                                   */
/*****************************************************************************/
#define ENTER_LOCAL_BUSY (1)
#define EXIT_LOCAL_BUSY  (0)

/*****************************************************************************/
/* PORT MACROS                                                               */
/*****************************************************************************/

/*****************************************************************************/
/* MACRO : QPM_IS_RECEIVE_QUEUE_EMPTY?                                       */
/* Parameter : struct port_type *port_id                                     */
/* Type : struct mbuf *                                                      */
/* Description :                                                             */
/* If the receive queue is empty, or the port doesn't exist                  */
/* return TRUE                                                               */
/*****************************************************************************/
#define QPM_IS_RECEIVE_QUEUE_EMPTY(port_id) \
 ( ((port_id) == NULL)  || ((port_id)->receive_q_ptr == NULL )

/*****************************************************************************/
/* MACRO : QPM_IS_EXCEPTION_QUEUE_EMPTY?                                     */
/* Parameter : struct port_type *port_id                                     */
/* Type : struct pending_status_block_type *                                 */
/* Description :                                                             */
/* If the exception queue is empty, or the port doesn't exist                */
/* return TRUE                                                               */
/*****************************************************************************/
#define QPM_IS_EXCEPTION_QUEUE_EMPTY(port_id) \
 ( ((port_id) == NULL)  || ((port_id)->exception_q_ptr == NULL )

/*****************************************************************************/
/* MACRO : QPM_RETURN_LOCAL_ADDRESS                                          */
/* Parameter : struct port_type *port_id                                     */
/* Type : char *                                                             */
/* Description :                                                             */
/* Returns the address field of the local address stored in the port         */
/*****************************************************************************/
#define QPM_RETURN_LOCAL_ADDRESS(port_id) \
 ((port_id)->local_address)

/*****************************************************************************/
/* MACRO : QPM_RETURN_MAX_PACKET_SIZE                                        */
/* Parameter : struct port_type *port_id                                     */
/* Type : short                                                              */
/* Description :                                                             */
/* Returns the size of the largest packet that can be transmitted            */
/* irrespective of the default packet size applying on a particular          */
/* virtual cct. It depends on the physical line.                             */
/*****************************************************************************/
#define QPM_RETURN_MAX_PACKET_SIZE(port_id) \
 ((port_id)->ddi_struct.max_tx_pkt_size)

/*****************************************************************************/
/* MACRO : QPM_RETURN_SVC_DEFAULT_PACKET_SIZE                                */
/* Parameter : struct port_type *port_id                                     */
/* Type : x25_pkt_size_type                                                  */
/* Description :                                                             */
/* Returns the default packet size applying on an SVC, in the absence        */
/* of any facilities requests for a non-default pkt size.                    */
/*****************************************************************************/
#define QPM_RETURN_SVC_DEFAULT_PACKET_SIZE(port_id) \
  ((x25_pkt_size_type)((port_id)->ddi_struct.def_rx_pkt_size))

/*****************************************************************************/
/* MACRO : QPM_RETURN_PVC_RESET_DIAGNOSTIC                                   */
/* Parameter : struct port_type *port_id                                     */
/* Type :                                                                    */
/* Description :                                                             */
/* Returns the diag appling to this PVC.                                     */
/*****************************************************************************/
#define QPM_RETURN_PVC_RESET_DIAGNOSTIC(port_id) \
  (port_id)->device_parms.dummy  /* this is only a dummy until DH is done    */

/*****************************************************************************/
/* TYPES                                                                     */
/*****************************************************************************/
enum qpm_rc_type
{
  qpm_rc_ok,
  qpm_rc_alloc_failed,
  qpm_rc_close_failed,
  qpm_rc_creatp_failed,
  qpm_rc_enter_busy_failed,
  qpm_rc_exit_busy_failed,
  qpm_rc_exception_queue_empty,
  qpm_rc_free_failed,
  qpm_rc_halt_failed,
  qpm_rc_initialisation_error,
  qpm_rc_initp_failed,
  qpm_rc_ioctl_failed,
  qpm_rc_no_name,
  qpm_rc_open_failed,
  qpm_rc_pin_failed,
  qpm_rc_port_not_found,
  qpm_rc_program_error,
  qpm_rc_query_failed,
  qpm_rc_receive_queue_empty,
  qpm_rc_reject_failed,
  qpm_rc_start_failed,
  qpm_rc_system_error,
  qpm_rc_termination_error,
  qpm_rc_write_error
};
typedef enum qpm_rc_type qpm_rc_type;

struct qlc_intrpt_hndlr_init_data_t
{
  struct port_type   *port_id;
};
typedef struct qlc_intrpt_hndlr_init_data_t qlc_intrpt_hndlr_init_data_t;

/*****************************************************************************/
/* PENDING STATUS BLOCKS                                                     */
/* A type is defined for pending status blocks as they are chained           */
/* together to form a linked list, and so need forward and                   */
/* backward reference pointers                                               */
/*****************************************************************************/
struct pending_status_block_type
{
  struct status_block               block;
  struct pending_status_block_type  *next_block_ptr;
  struct pending_status_block_type  *prev_block_ptr;
};
typedef struct pending_status_block_type pending_status_block_type;

/* defect 156503 */
struct listen_netid_type
{
  struct listen_netid_type *next;
  unsigned short listener_netid;
};
/* end defect 156503 */


typedef char port_name_type[10];     /* Port Name passed to mpx entry pt and */
	                             /* stored in channel by QDH.            */

/*****************************************************************************/
/* Port Type                                                                 */
/*****************************************************************************/
struct port_type
{
  unsigned int        lock;
  struct port_type   *port_id;
  unsigned int        user_count;
  dev_t               dh_devno;
  char                kproc_process_name[20];
  char                xdh_pathname[20];
  /* dh_chan is chan parm returned by dh on open                             */
  int                 dh_chan;
  struct file        *fp;
  struct listen_netid_type *listen_netids;   /* defect 156503 */
  /***************************************************************************/
  /* The iocinfo ioctl to the dh immediately after open returns the data     */
  /* which is stored in the dh_devinfo structurein the port.                 */
  /***************************************************************************/
  x25_devinfo_t       dh_devinfo;
  pid_t               int_pid;
/* <<< THREADS >>> */
  tid_t               int_tid;
/* <<< end THREADS >>> */
  int                 term_done;  /* event list anchor for termination sleep */
  /***************************************************************************/
  /* Receive_q_ptr is start of chain of incoming mbufs                       */
  /* Exception_q_ptr is start of a chain of pending_status_blocks            */
  /* There is no longer a transmit queue in the port.                        */
  /***************************************************************************/
  jsmlist_t                         receive_data_queue;
  struct pending_status_block_type *exception_queue;

  /***************************************************************************/
  /* address passed in as arg on init                                        */
  /***************************************************************************/
  char               local_address[DLC_MAX_NAME];

  struct port_type  *next_port_ptr;
  struct port_type  *prev_port_ptr;
};
typedef  struct port_type  port_type;



/* Start of declarations for qlcp.c                                          */
#ifdef _NO_PROTO
enum qpm_rc_type qpm_query_device();
void             qpm_interrupt_handler();
enum qpm_rc_type qpm_reject();
void             qpm_exit_local_busy();
void             qpm_enter_local_busy();
enum qpm_rc_type qpm_halt();
enum qpm_rc_type qpm_write ();
enum qpm_rc_type qpm_start();
enum qpm_rc_type qpm_port_terminate();
enum qpm_rc_type qpm_port_initialise();
void             qpm_exception_function();
void             qpm_transmit_function();
void             qpm_receive_function();
enum qpm_rc_type qpm_init_address();

/* defect 156503 */
boolean		rm_listen_netid();
boolean		add_listen_netid();
boolean		listening_netid();
/* end defect 156503 */


#else

extern void  qpm_receive_function (
  unsigned int open_id,
  struct x25_read_ext *dh_read_ext,
  gen_buffer_type *buffer_ptr);

extern void  qpm_transmit_function (
  unsigned int open_id);

extern void  qpm_exception_function (
  unsigned int open_id,
  struct status_block *status_block_ptr);

extern qpm_rc_type qpm_port_initialise(
  struct port_type **port_id,
  char              *path);

extern qpm_rc_type qpm_port_terminate (
  port_type *port_id);

extern qpm_rc_type qpm_query_device(
  port_type   *port_id,
  char        *data_area);

extern qpm_rc_type qpm_start(
  port_type *port_id,
  gen_buffer_type *start_buffer,
  struct x25_start_data *start_data);

extern qpm_rc_type  qpm_write (
  port_type *port_id,
  gen_buffer_type *buffer_ptr,
  struct x25_write_ext  *write_ext);

extern qpm_rc_type   qpm_halt(
  port_type *port_id,
  struct x25_halt_data *halt_data,
  gen_buffer_type *buffer_ptr);

extern void qpm_interrupt_handler (
  unsigned int flag,
  qlc_intrpt_hndlr_init_data_t *init_parms,
  int parms_length);

extern qpm_rc_type  qpm_enter_local_busy (
  port_type *port_id,
  unsigned short session_id);

extern qpm_rc_type  qpm_exit_local_busy (
  port_type *port_id,
  unsigned short session_id);

extern qpm_rc_type  qpm_reject (
  port_type *port_id,
  struct x25_reject_data *reject_data,
  gen_buffer_type *buffer_ptr);

extern qpm_rc_type  qpm_init_address (
  port_type         *port_id,
  char              *address);

/* defect 156503 */
extern boolean rm_listen_netid(
  unsigned    short netid,
  port_type   *port_id);

extern boolean add_listen_netid(
  unsigned    short netid,
  port_type   *port_id);

extern boolean listening_netid(
  unsigned    short netid,
  port_type   *port_id);
/* end defect 156503 */

#endif          /* _NO_PROTO */

/* End of declarations for qlcp.c      */


#endif

