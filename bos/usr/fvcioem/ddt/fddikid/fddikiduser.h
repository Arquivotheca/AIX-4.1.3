/* @(#)13   1.1  src/bos/usr/fvcioem/ddt/fddikid/fddikiduser.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:58 */

/*--------------------------------------------------------------------------
*
*             KIDUSER.H
*
*  COMPONENT_NAME:  Kernel Interface Driver (definitions)
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*--------------------------------------------------------------------------
*/

# define MAX_KOPENS  (16)
# define MAX_BLOCK   (512)
# define QUEUE_SIZE  (32)
# define kid_MAJOR   (53)     /* chosen arbitrarily */
# define KIDPATH  "/usr/lib/drivers/fddikid"
# define KIDSPECIAL  "/dev/fddikid"
# define KIDSTATUS   "/usr/lib/drivers/fddikidstatus"

# define CHAN_AVAIL  (0)
# define CHAN_ALLOCATED (1)

# define KID_IOCTL    ('k'<< 8)  /* KID ioctl commands: */
# define KID_OPEN_DRVR   ( KID_IOCTL | 0x10 )
# define KID_CLOSE_DRVR  ( KID_IOCTL | 0x11 )
# define KID_MBUF_THRESH ( KID_IOCTL | 0x12 )
# define KID_MBUF_WDTO   ( KID_IOCTL | 0x13 )
# define KID_FUNKY_MBUF  ( KID_IOCTL | 0x14 )
# define KID_READ_MULT   ( KID_IOCTL | 0x15 )
# define KID_WRITE_MULT  ( KID_IOCTL | 0x16 )
# define KID_SET_FASTWRT ( KID_IOCTL | 0x17 )
# define KID_FASTWRT     ( KID_IOCTL | 0x18 )
# define KID_GET_STAT    ( KID_IOCTL | 0x19 )
# define KID_TX_STORM    ( KID_IOCTL | 0x20 )
# define KID_BAD_PKT_READ  ( KID_IOCTL | 0x21 )

# define MKNOD_MODE  (  \
   S_IFMPX |           \
   S_IRUSR | S_IWUSR | \
   S_IRGRP | S_IWGRP | \
   S_IROTH | S_IWOTH   \
)

typedef struct
{
   int   siz;     /* size of chain */
   int   num;     /* number of chains */
} TX_STORM;

typedef struct {
    dev_t      devno;      /* device number of device */
    char    channame[20];  /* channel name of device (if any) */
    int        flags;      /* device open flags */
} DRVR_OPEN;

/*  Special write extension that accomodates both MPQP and  */
/*  "everyone else":                */


typedef struct
{
    cio_write_ext_t  ciowe;      /* CIO write extension */
    unsigned char transparent;   /* required by MPQP */
    uchar      fastwrt; /* for fast write support */
    int        frames_req; /* frames per request for fastwrt */
} WRITE_EXT;

/*------------------------------------------------------------------------*/
/*  Used internally by the driver:                                        */
/*------------------------------------------------------------------------*/

# define MBUF_FREE_WAIT    8  /* Eight seconds */

typedef struct {
    short      read_index;
    short      write_index;
    int        slot[ QUEUE_SIZE ];
    int        seq [ QUEUE_SIZE ];
} QUEUE;

typedef struct {
    int        event;      /* process wait for read data */
    int        open;    /* driver under test open flag */
    int        xmitfull;   /* transmit queue full flag */
    int        devno;      /* device number */
    int        selectflags;   /* flags for selnotify */
    int        mbuf_thresh;   /* mbuf size threshold */
    int        mbuf_wdto;  /* mbuf write data transfer offset */
    int        mbuf_funky; /* create funky mbufs? */
    int        mbuf_f_ext; /* our xmalloc'ed page for the funky */
    int        mbuf_f_used;   /* TRUE if used by an mbuf */
    struct file      *fp;     /* kernel file pointer for test */
    QUEUE      read_queue; /* queue of read blocks */
    int        read_overflow; /* nonzero if read queue overflow */
    QUEUE      status_queue;  /* queue of status blocks */
    int        stat_overflow; /* nonzero if status queue overflow */
    /*
     * For managing the freeing of mbufs on write path
     */
    struct mbuf      *free_head; /* mbuf to free on status */
    struct mbuf      *free_tail; /* mbuf to free on status */
    int        free_cnt;   /* number of mbufs waiting to be freed */

    /*
     * For fastwrite implementation
     */
    cio_get_fastwrt_t   fastwrt_parm;  /* fastwrt parameters */
    struct mbuf      *fw_head;   /* head of list of pending fastwrites */
    struct mbuf      *fw_tail;   /* tail of list of pending fastwrites */
    int        fw_cnt;     /* number of pending fastwrites */
    QUEUE      bad_pkt_queue; /* bad pkt queue */
    int        bad_pkt_overflow; /* nonzero if bad pkt que
                   * overflow
                   */
} OPEN_TBL;

typedef struct {
    struct mbuf         *m;      /* mbuf for this read */
    int           seq;
    struct read_extension  rd_ext;  /* read extension data */
} RD_BLOCK;

struct fddikid_bad_read
{
   int   len;  /* length of data buffer */
   caddr_t  data; /* put received data here */
   int   outlen;  /* length of data read */
   int   pktlen; /* length of what the pkt was */
};
typedef struct fddikid_bad_read fddikid_bad_read_t;

