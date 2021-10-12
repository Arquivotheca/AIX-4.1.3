/* @(#)49  1.4  src/bos/kernext/x25/crddefs.h, sysxx25, bos411, 9428A410j 7/29/92 15:19:29 */
#ifndef _H_CRDDEFS
#define _H_CRDDEFS
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

#include  "jsdefs.h"
#include  <sys/types.h>
#include  "crdrc.h"


/*****************************************************************************/
/* General definitions for the card interface routines                       */
/*****************************************************************************/
#define  CRD_MAX_LINE (0)


#define  CONVERTER_OFFSET (0xf4000000)      /* base memory address           */
                                            /* via converter                 */

#define CRD_ASSUMED_X25_SS_SIZE (80)
#define CRD_ASSUMED_RCM_SS_SIZE (8)
#define C2X_CARD_NUMBER (0xC9)
#define C2X_CARD_NUMBER_ADDRESS (0x40A)

struct crd_page_offset_struct
{
  byte    page;
  ushort  offset;
};
typedef struct crd_page_offset_struct crd_page_offset_t;

/*****************************************************************************/
/* crd_vec_t is used to pass a buffer to the card interface routines.        */
/* A pointer to the start of a list of this type is passed to the routines.  */
/*****************************************************************************/
struct crd_vec_struct
{
  byte *buffer;
  uint length;
};
typedef struct crd_vec_struct crd_vec_t;

/*****************************************************************************/
/* buffer information, location & size                                       */
/*****************************************************************************/
struct crd_buffer_info_struct
{
  byte    page;                     /* page of c2x memory                    */
  ushort  offset;                   /* offset within the page                */
  ushort  length;                   /* number of bytes of buffer             */
};
typedef struct crd_buffer_info_struct crd_buffer_t;

/*****************************************************************************/
/* the state the card is in                                                  */
/*****************************************************************************/
enum crd_state_enum
{
  crd_not_defined,                  /* nothing has been accessed on the card */
  crd_defined,                      /* the card has been registered          */
  crd_rcm,                          /* RCM has been loaded on to the card    */
  crd_rcm_x25,                      /* RCM & X.25 have been loaded           */
  crd_disabled                      /* Though the tasks are loaded the card  */
                                    /* is not be used                        */
};
typedef enum crd_state_enum crd_state_t;

/*****************************************************************************/
/* The information that needs to be kept for each card                       */
/*****************************************************************************/
struct crd_info_struct
{
  crd_state_t       state;          /*the state the card is in               */

  byte              *io_base;       /*base i/o address                       */
#if defined(_POWER)
  ulong             bus_id;         /* Bus id if required                    */
#endif
  byte              *window;        /*start of shared memory window          */
  uint              window_size;    /*size of shared window                  */
  byte              int_level;      /*interrupt level of the card            */

  ushort            first_bcb;      /* offset of the actual address of the   */
                                    /* first BCB                             */
  ushort            ib;

  crd_page_offset_t diag_task;      /* Load page/offset of Diagnostic task   */

  crd_buffer_t      rcm_op;         /*RCM's output buffer                    */
  crd_buffer_t      rcm_ip;         /*RCM's input buffer                     */
  crd_buffer_t      rcm_ss;         /*RCM's secondary status buffer          */

  crd_buffer_t      x25_op;         /*BLADON's output buffer                 */
  crd_buffer_t      x25_ip;         /*BLADON's input buffer                  */
  crd_buffer_t      x25_ss;         /*BLADON's secondary status buffer       */
  crd_page_offset_t dq_start;       /*first element of the q queue           */
  crd_page_offset_t zq_start;       /*first element of the z queue           */
  ushort            dq_size;        /*number of elements in the d queue      */
  ushort            zq_size;        /*number of elements in the z queue      */
  crd_page_offset_t dq;             /*active element of the q queue          */
  crd_page_offset_t zq;             /*active element of the z queue          */

                                    /* area to copy the BLADON SS in to      */
  byte              x25_ss_buffer[CRD_ASSUMED_X25_SS_SIZE];

                                    /* area to copy the RCM SS in to         */
  byte              rcm_ss_buffer[CRD_ASSUMED_RCM_SS_SIZE];
  
  ushort            cable_id;       /* 0=V.24, 1=V.35, 2=X.21 3=none or wrap */
  int               free_mem;       /* memory left for Bladon pools and      */
                                    /* control structures                    */

};
typedef struct crd_info_struct crd_info_t;

/*****************************************************************************/
/* "Useful" information block that can be read from the card - what is       */
/* considered useful may well vary with time                                 */
/*****************************************************************************/
struct crd_query_struct
{
                                    /* area to copy the BLADON SS in to      */
  byte              x25_ss_buffer[CRD_ASSUMED_X25_SS_SIZE];

                                    /* area to copy the RCM SS in to         */
  byte              rcm_ss_buffer[CRD_ASSUMED_RCM_SS_SIZE];
  
  ushort            cable_id;       /* 0=V.24, 1=V.35, 2=X.21 3=none or wrap */
  int               free_mem;       /* memory left for Bladon pools and      */
                                    /* control structures                    */
};
typedef struct crd_query_struct crd_query_t;

/*****************************************************************************/
/* Auto call unit commands                                                   */
/*****************************************************************************/
enum crd_acu_cmd_enum
{
  crd_acu_enable_auto_answer,
  crd_acu_disable_auto_answer,
  crd_acu_dial,
  crd_acu_status,
  crd_acu_abort_call
};
typedef enum crd_acu_cmd_enum crd_acu_cmd_t;


/*****************************************************************************/
/* The adaptors that are available                                           */
/*****************************************************************************/
enum crd_adaptor_enum
{
  crd_x25_adaptor,
  crd_lms_adaptor,
  crd_csm_adaptor
};
typedef enum crd_adaptor_enum crd_adaptor_t;

/*****************************************************************************/
/* LCN numbers                                                               */
/*****************************************************************************/
typedef ushort crd_lcn_t;

/*****************************************************************************/
/* the state of the connection - used at the various layers                  */
/*****************************************************************************/
enum crd_connection_status_enum
{
  crd_disconnected,
  crd_connecting,
  crd_connected
};
typedef enum crd_connection_status_enum crd_connection_status_t;

/*****************************************************************************/
/* status of the link at the three layers                                    */
/*****************************************************************************/
struct crd_link_status_struct
{
  crd_connection_status_t packet;
  crd_connection_status_t frame;
  crd_connection_status_t physical;
  ushort vcs;
};
typedef struct crd_link_status_struct crd_link_status_t;

/*****************************************************************************/
/* types to hold a LUN, card number, packet and window size                  */
/*****************************************************************************/
typedef byte   crd_lun_t;
typedef byte   crd_number_t;
typedef ushort crd_packet_size_t;
typedef byte   crd_window_size_t;
typedef ushort crd_line_number_t;

/*****************************************************************************/
/* diagnostic protocol type                                                  */
/*****************************************************************************/
enum crd_protocol_enum
{
  crd_iso8208,
  crd_sna80,
  crd_sna84
};
typedef enum crd_protocol_enum crd_protocol_t;


/*****************************************************************************/
/* type for the throughput class                                             */
/*****************************************************************************/
typedef byte crd_throughput_t;


/*****************************************************************************/
/* structure used to return details of the VC settings                       */
/*****************************************************************************/
struct crd_vc_settings_stuct
{
  crd_throughput_t  tx_throughput;
  crd_throughput_t  rx_throughput;
  crd_packet_size_t tx_pkt_size;
  crd_packet_size_t rx_pkt_size;
  crd_window_size_t tx_window;
  crd_window_size_t rx_window;
};
typedef struct crd_vc_settings_stuct crd_vc_settings_t;


/*****************************************************************************/
/* structure used to return status of the VC settings                        */
/*****************************************************************************/
struct crd_vc_status_struct
{
  crd_line_number_t line;
  crd_connection_status_t status;
  crd_lcn_t channel;
};
typedef struct crd_vc_status_struct crd_vc_status_t;

/*****************************************************************************/
/* the z structure is generated from the z response from the card            */
/*****************************************************************************/
struct z_struct_struct
{
  crd_number_t      crd;
  crd_lun_t         lun;
  crd_line_number_t line;
  crd_rc_t          rc;
  byte              res;
  byte              sc;
  byte              read_count;
  byte              write_credit;
  byte              pkt_type;
  byte              buffer_page;
  ushort            buffer_offset;
  ushort            buffer_length;
};
typedef struct z_struct_struct z_struct_t;

#define ACU_ENABLE_AA (1)
#define ACU_DISABLE_AA (2)
#define ACU_DIAL (3)
#define ACU_STATUS (4)
#define ACU_ABORT_CALL (5)

struct crd_acu_status_struct
{
  crd_line_number_t line;
  byte status;
};
typedef struct crd_acu_status_struct crd_acu_status_t;

#endif
