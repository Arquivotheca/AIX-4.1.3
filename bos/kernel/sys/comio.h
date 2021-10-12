/* @(#)60 1.22 5/29/91 13:38:44 */
#ifndef _H_COMIO
#define _H_COMIO

/*
 * COMPONENT_NAME: sysxcio -- Common Communications Code Device Driver Head
 *
 * FUNCTIONS: comio.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* registration for conforming device drivers                                */
/*****************************************************************************/

#define CAT_DRIVER   (0x01)  /* 370 channel attach adapter (catdd)           */
#define CCC_DRIVER   (0x02)  /* common communications code (ciodd)           */
#define ENT_DRIVER   (0x03)  /* baseband adapter for MCA bus (entdd)         */
#define ETH_DRIVER   (0x04)  /* baseband adapter for PC bus (ethdd)          */
#define MPQ_DRIVER   (0x05)  /* multiprotocol adapter for MCA bus (mpqdd)    */
#define TOK_DRIVER   (0x06)  /* tokenring adapter for MCA bus (tokdd)        */
#define X25_DRIVER   (0x07)  /* X25 adapter for MCA bus (x25dd)              */
#define FDDI_DRIVER  (0x08)  /* FDDI device driver (fddidd) 		     */

#define MAX_DRIVER   (0x80)  /* may be used by users -- not any device driver*/

/*****************************************************************************/
/* simple types                                                              */
/*****************************************************************************/

typedef ushort netid_t; /* type used for all comm i/o network identifiers    */

/*****************************************************************************/
/* exception codes for communications i/o subsystem                          */
/*****************************************************************************/

/*
 * These exception codes may be returned in:
 *    status    of struct kopen_ext         (cio_kopen_ext_t)
 *    status    of struct read_extension    (cio_read_ext_t)
 *    status    of struct write_extension   (cio_write_ext_t)
 *    status    of struct session_blk       (cio_sess_blk_t)
 *    status    of struct query_parms       (cio_query_blk_t)
 *    option[0] of struct status_block      (cio_stat_blk_t)
 */

#define CIO_OK             (0x00000000) /* operation was successful          */
#define CIO_BAD_MICROCODE  (0x00000001) /* invalid microcode                 */
#define CIO_BUF_OVFLW      (0x00000002) /* data too large for buffer         */
#define CIO_HARD_FAIL      (0x00000003) /* hard failure                      */
#define CIO_LOST_DATA      (0x00000004) /* data were lost                    */
#define CIO_NOMBUF         (0x00000005) /* no mbuf available                 */
#define CIO_NOT_STARTED    (0x00000006) /* start not performed               */
#define CIO_TIMEOUT        (0x00000007) /* operation timed out               */
#define CIO_NET_RCVRY_ENTER (0x00000008) /* enter network recovery           */
#define CIO_NET_RCVRY_EXIT (0x00000009) /* exit network recovery            */
#define CIO_NET_RCVRY_MODE (0x0000000A) /* network recovery mode             */
#define CIO_INV_CMD        (0x0000000B) /* invalid command                   */
#define CIO_BAD_RANGE      (0x0000000C) /* bad address range                 */
#define CIO_NETID_INV      (0x0000000D) /* invalid net id                    */
#define CIO_NETID_DUP      (0x0000000E) /* duplicate net id                  */
#define CIO_NETID_FULL     (0x0000000F) /* all net are used                  */
#define CIO_TX_FULL        (0x00000010) /* transmit queue is full            */

#define CIO_EXCEPT_MAX     (0x00000040) /* highest in comio.h                */

/* No comio conforming device driver will have codes this high.              */
/* This value and up is for users to create unique internal-use codes.       */

#define CIO_EXCEPT_USER    (0x7FFF0000) /* for users to create unique codes  */

/*****************************************************************************/
/* CIO_IOCTL ioctl's for communications i/o subsystem                        */
/*****************************************************************************/

#define CIO_IOCTL     (DD_CIO<<8)        /* prefix as defined in devinfo.h   */

#define CIO_START     (CIO_IOCTL | 0x01) /* start                            */
#define CIO_HALT      (CIO_IOCTL | 0x02) /* halt                             */
#define CIO_GET_STAT  (CIO_IOCTL | 0x03) /* get status block                 */
#define CIO_QUERY     (CIO_IOCTL | 0x04) /* query statistics (get counters)  */
#define CIO_CONNECT   (CIO_IOCTL | 0x05) /* connect                          */
#define CIO_DISCONN   (CIO_IOCTL | 0x06) /* disconnect                       */
#define CIO_GET_FASTWRT   (CIO_IOCTL | 0x07) /* get fast write info	     */

#define CIO_IOCTL_MAX (CIO_IOCTL | 0x40) /* highest in comio.h               */

/*****************************************************************************/
/* CIO_START and CIO_HALT ioctl parameter definitions                        */
/*****************************************************************************/

struct session_blk {    /* structure passed to CIO_START and CIO_HALT ioctl  */
   ulong   status;      /* exception code (valid if ioctl returns EIO)       */
   ushort  length;      /* number of valid bytes in netid for mixed ethernet */
   netid_t netid;       /* the network id, or similar id for start and halt  */
};
typedef struct session_blk cio_sess_blk_t; /* CIO_START & CIO_HALT parameter */

/*****************************************************************************/
/* CIO_GET_STAT ioctl parameter definitions                                  */
/*****************************************************************************/

/* status block identifier codes returned by CIO_GET_STAT ioctl */
#define CIO_NULL_BLK     (0x00) /* status block was not available            */
#define CIO_START_DONE   (0x01) /* status block is result of CIO_START       */
#define CIO_HALT_DONE    (0x02) /* status block is result of CIO_HALT        */
#define CIO_LOST_STATUS  (0x03) /* status block is result of status que ovrfl*/
#define CIO_ASYNC_STATUS (0x04) /* status block is asynchronous with adapter */
#define CIO_TX_DONE      (0x05) /* status block is result of completed write */
#define CIO_CONNECT_DONE (0x06) /* status block is result of CIO_CONNECT     */
#define CIO_DISCONN_DONE (0x07) /* status block is result of CIO_DISCONN     */

#define CIO_STATUS_MAX   (0x40) /* highest in comio.h                        */

struct status_block { /* structure returned by CIO_GET_STAT ioctl            */
   ulong code;        /* status block identifier code defined above          */
   ulong option[4];   /* meaning varies according to code value              */
};
typedef struct status_block cio_stat_blk_t; /* CIO_GET_STAT parameter        */

/*****************************************************************************/
/* CIO_QUERY ioctl parameter definitions                                     */
/*****************************************************************************/

struct cio_stats {      /* initial part of structure returned by CIO_QUERY   */
                        /* the next 8 words contain 4 32-bit counters        */
                        /* msh is the more significant half (upper 32 bits)  */
                        /* lsh is the less significant half (lower 32 bits)  */
   ulong tx_byte_mcnt;  /* (msh) count of total bytes transmitted correctly  */
   ulong tx_byte_lcnt;  /* (lsh) count of total bytes transmitted correctly  */
   ulong rx_byte_mcnt;  /* (msh) count of total bytes received correctly     */
   ulong rx_byte_lcnt;  /* (lsh) count of total bytes received correctly     */
   ulong tx_frame_mcnt; /* (msh) count of total frames transmitted correctly */
   ulong tx_frame_lcnt; /* (lsh) count of total frames transmitted correctly */
   ulong rx_frame_mcnt; /* (msh) count of total frames received correctly    */
   ulong rx_frame_lcnt; /* (lsh) count of total frames received correctly    */
   ulong tx_err_cnt;    /* count of total frame transmit errors              */
   ulong rx_err_cnt;    /* count of total frame receive errors               */
   ulong nid_tbl_high;  /* maximum netid's in use (for this device)          */
   ulong xmt_que_high;  /* maximum transmits ever queued (for this device)   */
   ulong rec_que_high;  /* maximum receives queued (for any open)            */
   ulong sta_que_high;  /* maximum status blocks queued (for any open)       */
};
typedef struct cio_stats cio_stats_t; /* part of CIO_QUERY returned data     */

#define CIO_QUERY_CLEAR (0xC0C0C0C0)

struct query_parms {  /* structure passed to CIO_QUERY ioctl                 */
   ulong    status;   /* exception code (always valid)                       */
   caddr_t  bufptr;   /* first part is struct cio_stats, rest device-specific*/
   int      buflen;   /* number of bytes available in buffer for stats       */
   int      clearall; /* CIO_QUERY_CLEAR means clear all stats after read    */
};
typedef struct query_parms cio_query_blk_t; /* CIO_QUERY parameter           */

/*
 * Note the expected usage of CIO_QUERY is as follows:
 *
 *   typedef struct {              XXXuser.h may define this structure for user
 *      struct cio_stats cc;       common code counters from comio.h (above)
 *      struct XXX_stats ds;       device specific counters from XXXuser.h
 *   } XXX_query_stats_t;
 *
 *   cio_query_blk_t      query_blk;           parameter to CIO_QUERY ioctl
 *   XXX_query_stats_t    query_stats;         storage for results of CIO_QUERY
 *
 *   query_blk.bufptr   = &query_stats;        tell where storage is located
 *   query_blk.buflen   = sizeof(query_stats); tell size of storage area
 *   query_blk.clearall = 0;                   do not clear counters
 *
 *   if (ioctl (fd, CIO_QUERY, &query_blk) == -1)
 *      handle_error();
 */

/*****************************************************************************/
/* CIO_GET_FASTWRT ioctl parameter definitions                               */
/*****************************************************************************/

struct cio_get_fastwrt {	/* structure passed to CIO_GET_FASTWRT	     */
	ulong	status;
	int	(*fastwrt_fn)();
	chan_t	chan;
	dev_t	devno;
};
typedef struct cio_get_fastwrt cio_get_fastwrt_t; /* GET_FASTWRT parameter   */

/*****************************************************************************/
/* ddopen, ddread, and ddwrite extended parameter definitions                */
/*****************************************************************************/

struct kopen_ext {    /* structure passed to ddopen from kernel process      */
   ulong  status;     /* exception code (valid if openx returns EIO)         */
   void (*rx_fn)();   /* address of read-data "interrupt" entry              */
   void (*tx_fn)();   /* address of transmit-now-possible "interrupt" entry  */
   void (*stat_fn)(); /* address of status-block "interrupt" entry           */
   ulong  open_id;    /* id passed to "interrupt" entries                    */
};
typedef struct kopen_ext cio_kopen_ext_t;  /* kernel devopen parameter       */

/*****************************************************************************/

struct read_extension {  /* read extension passed to user or kernel process  */
   ulong   status;       /* exception code (always valid)                    */
   netid_t netid;        /* not set by all drivers                           */
   ushort  sessid;       /* not set by all drivers                           */
};
typedef struct read_extension cio_read_ext_t; /* rn_fn or readx parameter    */

/*****************************************************************************/

#define CIO_NOFREE_MBUF  (0x01)  /* do not free mbuf after transmit complete */
#define CIO_ACK_TX_DONE  (0x02)  /* notify asynchronously when xmit complete */

struct write_extension { /* write extension from user or kernel process      */
   ulong   status;       /* exception code (valid if writex returns EIO)     */
   ulong   flag;         /* may include CIO_NOFREE_MBUF, CIO_ACK_TX_DONE     */
   ulong   write_id;     /* returned in status block if CIO_ACK_TX_DONE      */
   netid_t netid;        /* not required by all drivers                      */
};
typedef struct write_extension cio_write_ext_t; /* writex parameter          */

#endif /* ! _H_COMIO */
