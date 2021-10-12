/* @(#)91  1.5.2.1  src/bos/kernext/x25/xdhx25.h, sysxx25, bos411, 9428A410j 4/1/94 13:08:50 */
#ifndef _H_XDHX25
#define _H_XDHX25
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

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <sys/types.h>
#include <sys/comio.h>
#include "jsdefs.h"

/* X25USER ON                                                                */

/*****************************************************************************/
/* The session types for the different types of X.25 CIO_STARTs.             */
/* SESSION_SVC_OUT         Initiate an outgoing call                         */
/* SESSION_SVC_LISTEN      Listen for an incoming call                       */
/* SESSION_SVC_IN          Accept an incoming call                           */
/* SESSION_PVC             Claim exclusive use of a PVC                      */
/* SESSION_MONITOR         Claim exclusive use of the line monitor           */
/*****************************************************************************/
#define SESSION_SVC_OUT    (0)
#define SESSION_SVC_LISTEN (SESSION_SVC_OUT+1)
#define SESSION_SVC_IN     (SESSION_SVC_OUT+2)
#define SESSION_PVC        (SESSION_SVC_OUT+3)
#define SESSION_MONITOR    (SESSION_SVC_OUT+4)
/* X25USER OFF                                                               */

/* X25USER ON                                                                */

/*****************************************************************************/
/* The X.25 device driver IOCTL values                                       */
/*****************************************************************************/
#define X25_BASE_IOCTL       ('x'<<8)
#define X25_REJECT_CALL      (X25_BASE_IOCTL + 1)
#define X25_QUERY_SESSION    (X25_BASE_IOCTL + 2)
#define X25_ADD_ROUTER_ID    (X25_BASE_IOCTL + 3)
#define X25_DELETE_ROUTER_ID (X25_BASE_IOCTL + 4)
#define X25_QUERY_ROUTER_ID  (X25_BASE_IOCTL + 5)
#define X25_LINK_CONNECT     (X25_BASE_IOCTL + 6)
#define X25_LINK_DISCONNECT  (X25_BASE_IOCTL + 7)
#define X25_LINK_STATUS      (X25_BASE_IOCTL + 8)
#define X25_LOCAL_BUSY       (X25_BASE_IOCTL + 9)
#define X25_COUNTER_GET      (X25_BASE_IOCTL + 10)
#define X25_COUNTER_WAIT     (X25_BASE_IOCTL + 11)
#define X25_COUNTER_READ     (X25_BASE_IOCTL + 12)
#define X25_COUNTER_REMOVE   (X25_BASE_IOCTL + 13)
#define X25_DIAG_IO_WRITE    (X25_BASE_IOCTL + 14)
#define X25_DIAG_IO_READ     (X25_BASE_IOCTL + 15)
#define X25_DIAG_MEM_WRITE   (X25_BASE_IOCTL + 16)
#define X25_DIAG_MEM_READ    (X25_BASE_IOCTL + 17)
#define X25_DIAG_CARD_READY  (X25_BASE_IOCTL + 18)
#define X25_DIAG_RESET       (X25_BASE_IOCTL + 19)
#define X25_DIAG_TASK        (X25_BASE_IOCTL + 20)
#define X25_DNLD             (X25_BASE_IOCTL + 21)
#define X25_DIAG_INTR_STAT   (X25_BASE_IOCTL + 23)
#define X25_SETUID           (X25_BASE_IOCTL + 25)
#define X25_BUFFER_LIMIT     (X25_BASE_IOCTL + 26) /* rlim */
/* X25USER OFF                                                               */

/*****************************************************************************/
/* Ioctls for debug purposes                                                 */
/*****************************************************************************/
#define X25_COVQUERY         (X25_BASE_IOCTL + 100)
#define X25_COVDUMP          (X25_BASE_IOCTL + 101)
#define X25_DUMP_GP          (X25_BASE_IOCTL + 102)

/* X25USER ON                                                                */

/*****************************************************************************/
/* The X.25 specific status return codes                                     */
/* See <sys/comio.h> for the generic status return codes                     */
/*****************************************************************************/
#define X25_BAD_CALL_ID		 ((CIO_EXCEPT_MAX)+1)
#define X25_CLEAR		 ((CIO_EXCEPT_MAX)+2)
#define X25_INV_CTR		 ((CIO_EXCEPT_MAX)+3)
#define X25_NAME_USED		 ((CIO_EXCEPT_MAX)+4)
#define X25_NOT_PVC		 ((CIO_EXCEPT_MAX)+5)
#define X25_NO_ACK		 ((CIO_EXCEPT_MAX)+6)
#define X25_NO_ACK_REQ		 ((CIO_EXCEPT_MAX)+7)
#define X25_NO_LINK		 ((CIO_EXCEPT_MAX)+8)
#define X25_NO_NAME		 ((CIO_EXCEPT_MAX)+9)
#define X25_PROTOCOL		 ((CIO_EXCEPT_MAX)+10)
#define X25_PVC_USED		 ((CIO_EXCEPT_MAX)+11)
#define X25_RESET		 ((CIO_EXCEPT_MAX)+12)
#define X25_TABLE		 ((CIO_EXCEPT_MAX)+13)
#define X25_TOO_MANY_VCS	 ((CIO_EXCEPT_MAX)+14)
#define X25_AUTH_LISTEN		 ((CIO_EXCEPT_MAX)+15)
#define X25_BAD_PKT_TYPE         ((CIO_EXCEPT_MAX)+16)
#define X25_BAD_SESSION_TYPE     ((CIO_EXCEPT_MAX)+17)
/* X25USER OFF                                                               */

/* X25USER ON                                                                */

/*****************************************************************************/
/* X.25 defined status block codes                                           */
/* See <sys/comio.h> for the generic ones                                    */
/*****************************************************************************/
#define X25_REJECT_DONE (CIO_STATUS_MAX+1)
/* X25USER OFF                                                               */
/*****************************************************************************/
/* the x25_ structures that are used in xdh                                  */
/*****************************************************************************/
/* X25USER ON                                                                */

/*****************************************************************************/
/* The extension structure to pass to the CIO_START ioctl.                   */
/*****************************************************************************/
struct x25_start_data
{
  struct session_blk sb;               /* status and netid of session        */
  char   session_name[16];             /* ASCII name for RAS purposes        */
  ushort session_id;                   /* set by device handler on CIO_START */
  uchar  session_type;                 /* SVC_IN/OUT/LISTEN, PVC or MONITOR  */
  uchar  session_protocol;             /* protocol to use                    */
  int    counter_id;                   /* specify counter or -1 for none     */
  union
  {
    char   listen_name[16];             /* SVC_LISTEN nickname entry         */
    ushort call_id;                     /* From the incoming call for SVC_IN */
    int    logical_channel;             /* LCN if session is PVC             */
  } session_type_data;
};

/*****************************************************************************/
/* The extension structure to pass to the CIO_HALT ioctl                     */
/*****************************************************************************/
struct x25_halt_data
{
  struct session_blk sb;               /* status and netid of session        */
  ushort session_id;                   /* the session to halt                */
};

/*****************************************************************************/
/* The bit mask values for which facilities are supported by the local       */
/* network.  See supported_facilities in the IOCINFO structure               */
/*****************************************************************************/
#define X25_FAC_PSIZ       (0x00000001) /* Packet size                       */
#define X25_FAC_WSIZ       (0x00000002) /* Window size                       */
#define X25_FAC_TCLS       (0x00000004) /* Throughput class                  */
#define X25_FAC_CUG        (0x00000008) /* CUG basic format                  */
#define X25_FAC_EXT_CUG    (0x00000010) /* CUG extended format               */
#define X25_FAC_OA_CUG     (0x00000020) /* CUG with OA selection basic       */
#define X25_FAC_EXT_OA_CUG (0x00000040) /* CUG with OA selection extended    */
#define X25_FAC_BI_CUG     (0x00000080) /* BCUG                              */
#define X25_FAC_REV_CHRG   (0x00000100) /* Reverse charging or fast select   */
#define X25_FAC_NUI_DATA   (0x00000200) /* NUI                               */
#define X25_FAC_CI_REQUEST (0x00000400) /* Charging requesting service       */
#define X25_FAC_CI_MON_UNT (0x00000800) /* Monetary unit information request */
#define X25_FAC_CI_SEG_CNT (0x00001000) /* Segment count information request */
#define X25_FAC_CI_CALL_DUR (0x00002000)/* Call duration information request */
#define X25_FAC_RPOA       (0x00004000) /* RPOA basic format                 */
#define X25_FAC_EXT_RPOA   (0x00008000) /* RPOA extended format              */
#define X25_FAC_CLAMN      (0x00010000) /* Called line addr modified notific.*/
#define X25_FAC_CALL_REDR  (0x00020000) /* Call redirection notification     */
#define X25_FAC_TRAN_DEL   (0x00040000) /* Transit delay selection & indic.  */
#define X25_FAC_MARK_00    (0x00080000) /* Marker code, Calling network facs */
#define X25_FAC_MARK_FF    (0x00100000) /* Marker code, Called network facs  */
#define X25_FAC_MARK_0F    (0x00200000) /* Marker code, CCITT-DTE facs       */
#define X25_DATEXP         1		/* Datex-P network identifier        */
#define X25_DATAPAC        2		/* Datapac network identifier        */
#define X25_TELENET        3		/* Telenet network identifier        */
#define X25_DDN            4		/* Defense Data network identifier   */
#define X25_OPUB           5		/* Other public network identifier   */
#define X25_OPRI           6		/* Other private network identifier  */

struct intl_x25_devinfo_t
{
  char    devtype;				  /* DD_X25                  */
  char    flags;				  /* Undefined               */
  char    devsubtype;				  /* Undefined               */
  union
  {
    struct
    {
      unsigned short support_level;		  /* 1980, 1984              */
      char   nua[20];				  /* X.25 address of port    */
						  /*  in ASCIIZ              */
      unsigned long  supported_facilities;	  /* Which Facilities valid  */
						  /*  for this subscription ?*/
						  /*  See X25_FAC_ values in */
						  /*  x25user.h              */
      u_char network_id;     			  /* Network identifier      */
      int max_tx_packet_size;			  /* Max. bytes of data sent */
						  /*  in a packet            */
      int max_rx_packet_size;			  /* Max. bytes of data rcvd */
						  /*  in a packet            */
      int default_svc_tx_packet_size;		  /* Default transmit packet */
						  /*  size for an SVC        */
      int default_svc_rx_packet_size;		  /* Default receive packet  */
						  /*  size for an SVC        */
      int rdto;					  /* Receive data transfer   */
						  /*  offset as set in the   */
						  /*  configuration          */
      int memory_window_size;			  /* The window size of the  */
						  /*  RAM area on the X.25   */
						  /*  card                   */
    } x25;
  } un;
};
typedef struct intl_x25_devinfo_t x25_devinfo_t;

/*****************************************************************************/
/* The extension structure on a readx() call                                 */
/*****************************************************************************/
struct x25_read_ext
{
  struct read_extension re;			  /* See <sys/comio.h>       */
  ushort                call_id;		  /* Incoming calls only     */
						  /*  Pass to CIO_START or   */
						  /*  X25_REJECT             */
};

/*****************************************************************************/
/* The arg structure on an ioctl(X25_REJECT)                                 */
/*****************************************************************************/
struct x25_reject_data
{
  struct session_blk    sb;			  /* See <sys/comio.h>       */
  ushort                session_id;		  /* The listen session id   */
  ushort                call_id;		  /* From the read_ext of    */
						  /*  the incoming call      */
};

/*****************************************************************************/
/* The extension passed on writex()                                          */
/*****************************************************************************/
struct x25_write_ext
{
  struct write_extension we;			  /* See <sys/comio.h>       */
  ushort                 session_id;		  /* Session to write on     */
};

/*****************************************************************************/
/* The arg structure on an ioctl(X25_DISCONNECT)                             */
/*****************************************************************************/
struct x25_disconnect_data
{
  unsigned long  status;			  /* Error if EIO            */
  int    override;				  /* Override if calls active*/
						  /*  0 means don't override */
						  /*  1 means override       */
};

/*****************************************************************************/
/* The arg structure on an ioctl(X25_CONNECT)                                */
/*****************************************************************************/
struct x25_connect_data
{
  unsigned long  status;			  /* Error if EIO            */
};

/*****************************************************************************/
/* The arg structure on an ioctl(X25_QUERY_SESSION)                          */
/*****************************************************************************/
struct x25_query_session_data
{
  netid_t netid;				  /* User session identifier */
  unsigned short reserve_1;
  char    session_name[16];			  /* Name of session         */
  ushort  session_id;				  /* Kernel session id       */
  uchar   local_busy;				  /* 0 = Not, 1 = Busy       */
  uchar   session_protocol;			  /* X25_PROTOCOL_ISO8208 etc*/
  int     logical_channel;			  /* X.25 channel number     */
  int     tx_tclass;				  /* Transmit throughput     */
  int     rx_tclass;				  /* Receive  throughput     */
  int     tx_packet_size;			  /* Transmit packet size    */
  int     rx_packet_size;			  /* Receive packet size     */
  int     tx_window_size;			  /* Transmit window size    */
  int     rx_window_size;			  /* Receive window size     */
};

/*****************************************************************************/
/* The data structures for the counter ioctls.                               */
/*****************************************************************************/
struct x25_counter_info
{
  unsigned long flags;
  int counter_id;
  int counter_value;				  /* value to test for       */
};

struct x25_counter_list
{
  int counter_num;				  /* Num of counters in array*/
  struct x25_counter_info counter_array [1];	  /* Variable length         */
};

/*****************************************************************************/
/* x25_query_data is the structure returned from the CIO_QUERY ioctl().      */
/* The first section is the standard statistics values as found in           */
/* <sys/comio.h>.  Following that are the X.25 specific statistics.          */
/*****************************************************************************/
typedef unsigned short x25_stat_value_t;
struct x25_stats
{
  /***************************************************************************/
  /* The frame statistics parameters                                         */
  /* The values are counts of the number of times the event has occurred     */
  /***************************************************************************/
  x25_stat_value_t ignored_f_tx;
  x25_stat_value_t rr_f_tx;
  x25_stat_value_t rnr_f_tx;
  x25_stat_value_t rej_f_tx;
  x25_stat_value_t info_f_tx;
  x25_stat_value_t sabm_f_tx;
  x25_stat_value_t sarm_dm_f_tx;
  x25_stat_value_t disc_f_tx;
  x25_stat_value_t ua_f_tx;
  x25_stat_value_t frmr_f_tx;
  x25_stat_value_t bad_nr_f_tx;
  x25_stat_value_t unknown_f_tx;
  x25_stat_value_t xid_f_tx;
  x25_stat_value_t bad_length_f_tx;
  x25_stat_value_t t1_expirations;
  x25_stat_value_t lvl2_connects;
  x25_stat_value_t lvl2_disconnects;
  x25_stat_value_t carrier_loss;
  x25_stat_value_t connect_time;		  /* In seconds              */
  x25_stat_value_t t4_expirations;
  x25_stat_value_t t4_n2_times;
  x25_stat_value_t ignored_f_rx;
  x25_stat_value_t rr_f_rx;
  x25_stat_value_t rnr_f_rx;
  x25_stat_value_t rej_f_rx;
  x25_stat_value_t info_f_rx;
  x25_stat_value_t sabm_f_rx;
  x25_stat_value_t sarm_dm_f_rx;
  x25_stat_value_t disc_f_rx;
  x25_stat_value_t ua_f_rx;
  x25_stat_value_t frmr_f_rx;
  x25_stat_value_t bad_nr_f_rx;
  x25_stat_value_t unknown_f_rx;
  x25_stat_value_t xid_f_rx;
  x25_stat_value_t bad_length_f_rx;
  /***************************************************************************/
  /* The packet level parameters                                             */
  /***************************************************************************/
  x25_stat_value_t data_p_tx;
  x25_stat_value_t rr_p_tx;
  x25_stat_value_t rnr_p_tx;
  x25_stat_value_t interrupt_p_tx;
  x25_stat_value_t interrupt_confirm_p_tx;
  x25_stat_value_t call_request_p_tx;
  x25_stat_value_t call_accept_p_tx;
  x25_stat_value_t clear_request_p_tx;
  x25_stat_value_t clear_confirm_p_tx;
  x25_stat_value_t reset_request_p_tx;
  x25_stat_value_t reset_confirm_p_tx;
  x25_stat_value_t diagnostic_p_tx;
  x25_stat_value_t registration_p_tx;
  x25_stat_value_t registration_confirm_p_tx;
  x25_stat_value_t restart_p_tx;
  x25_stat_value_t restart_confirm_p_tx;
  x25_stat_value_t error_p_tx;
  x25_stat_value_t t20_expirations;
  x25_stat_value_t t21_expirations;
  x25_stat_value_t t22_expirations;
  x25_stat_value_t t23_expirations;
  x25_stat_value_t vc_establishments;
  x25_stat_value_t t24_expirations;
  x25_stat_value_t t25_expirations;
  x25_stat_value_t t26_expirations;
  x25_stat_value_t t28_expirations;
  x25_stat_value_t data_p_rx;
  x25_stat_value_t rr_p_rx;
  x25_stat_value_t rnr_p_rx;
  x25_stat_value_t interrupt_p_rx;
  x25_stat_value_t interrupt_confirm_p_rx;
  x25_stat_value_t incoming_call_p_rx;
  x25_stat_value_t call_connected_p_rx;
  x25_stat_value_t clear_indication_p_rx;
  x25_stat_value_t clear_confirm_p_rx;
  x25_stat_value_t reset_indication_p_rx;
  x25_stat_value_t reset_confirm_p_rx;
  x25_stat_value_t diagnostic_p_rx;
  x25_stat_value_t registration_p_rx;
  x25_stat_value_t registration_confirm_p_rx;
  x25_stat_value_t restart_p_rx;
  x25_stat_value_t restart_confirm_p_rx;
  int transmit_profile [16];
  int receive_profile [16];
};

struct x25_query_data
{
  struct cio_stats cc;				  /* See <sys/comio.h>       */
  struct x25_stats ds;				  /* See above               */
};

/*****************************************************************************/
/* The structure and defines for the X25_SET_LOCAL_BUSY ioctl.               */
/*****************************************************************************/
#define X25_SET_LOCAL_BUSY      (1)
#define X25_SET_LOCAL_NOT_BUSY  (0)

struct x25_local_busy
{
  uchar  busy_mode;				  /* An X25_SET_LOCAL value  */
  ushort session_id;				  /* Session to operate on   */
};

/*****************************************************************************/
/* The structure and defines for the X25_SETUID ioctl.                       */
/*****************************************************************************/
struct x25_setuid
{
  uid_t new_uid;       /* the new uid for the application */
  int session_id;      /* session to operate on */  
};

/*****************************************************************************/
/* The structure and defines for the X25_BUFFER_LIMIT ioctl.                 */
/*****************************************************************************/
#define X25_GET_READ_LIMIT  (1)
#define X25_SET_READ_LIMIT  (2)
#define X25_GET_WRITE_LIMIT (3)
#define X25_SET_WRITE_LIMIT (4)

struct x25_buffer_limit			          /* rlim */
{
  uchar          limit_mode;		          /* An X25_LIMIT value      */
  ushort         session_id;			  /* Session to operate on   */
  unsigned       limit;			          /* Limit to set/read       */
  unsigned       exceeded_count;	          /* # times limit exceeded  */
                                                  /*  valid for GET only     */
};

/*****************************************************************************/
/* Link status definitions                                                   */
/*****************************************************************************/
#define X25_LEVEL_CONNECTED	(2)
#define X25_LEVEL_CONNECTING	(1)
#define X25_LEVEL_DISCONNECTED	(0)

struct x25_link_status
{
  ulong    status;
  uchar    packet;				  /* An X25_LEVEL value      */
  uchar    frame;				  /* An X25_LEVEL value      */
  uchar    physical;				  /* An X25_LEVEL value      */
  unsigned no_of_vcs_in_use;
};

/*****************************************************************************/
/* The following series of structures are for the router ioctls.             */
/*****************************************************************************/
struct x25_router_add
{
  unsigned router_id;
  char     listen_name[16];
  uchar    action;
  int      priority;
  dev_t    devno;				  
  int      uid;
  char     called_subaddress[X25_MAX_ASCII_ADDRESS_LENGTH];
  char     calling_address[X25_MAX_ASCII_ADDRESS_LENGTH];
  char     extended_calling_address[X25_MAX_EXT_ADDR_DIGITS+1];
  char     extended_called_address[X25_MAX_EXT_ADDR_DIGITS+1];
  char     call_user_data[128];
};

struct x25_router_del
{
  unsigned router_id;
  int      override;
};

struct x25_router_query
{
  unsigned router_id;
  int      pid;
};

/*****************************************************************************/
/* The following series of structures are for the diagnostic ioctls.         */
/*****************************************************************************/
/*****************************************************************************/
/* The extension structure to pass to X25_DIAG_MEM_READ and                  */
/* X25_DIAG_MEM_WRITE                                                        */
/*****************************************************************************/
struct x25_diag_mem
{
  uchar   *buffer;
  uchar    card_page;
  ushort   card_offset;
  unsigned length;
};

/*****************************************************************************/
/* The extension structure to pass to X25_DIAG_IO_READ and                   */
/* X25_DIAG_IO_WRITE                                                         */
/*****************************************************************************/
struct x25_diag_io
{
  uchar card_register;
  uchar value;
};

/*****************************************************************************/
/* The extension structure to pass to X25_DIAG_TASK                          */
/*****************************************************************************/
struct x25_diag_addr
{
  uchar page;
  ushort offset;
};

/*****************************************************************************/
/* The interrupt status structure                                            */
/*****************************************************************************/
#define TREG_QUE_SIZE     100			  /* size of status queue    */
struct x25_diag_intr_stat
{
  unsigned count;
  unsigned char value[TREG_QUE_SIZE];
};

/*****************************************************************************/
/* The microcode structure passed to X25_DNLD ioctl                          */
/*****************************************************************************/
struct x25_tasks
{
  uchar    *x25_code;
  uchar    *rcm_code;
  uchar    *diagnostic_code;
  unsigned  x25_length;
  unsigned  rcm_length;
  unsigned  diagnostic_length;
};

/*****************************************************************************/
/* The structure passed to X25_DIAG_CARD_READY                               */
/*****************************************************************************/
struct x25_diag_card_ready
{
  uchar ready;
};
/* X25USER OFF                                                               */

#endif
