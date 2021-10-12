/* @(#)12   1.1  src/bos/usr/fvcioem/ddt/etkid/etkiduser.h, fvcioem, bos411, 9428A410j 4/26/94 13:54:55 */

/*--------------------------------------------------------------------------
*
*
*  COMPONENT_NAME:  Kernel Interface Driver (definitions)
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
# define kid_MAJOR   (52)     /* chosen arbitrarily */
# define KIDPATH  "/etc/drivers/kid"
# define KIDSPECIAL  "/dev/kid"
# define KIDSTATUS   "/etc/drivers/kidstatus"

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
# define KID_DUMP_CMD    ( KID_IOCTL | 0x19 )
# define KID_DUMP_WRITE  ( KID_IOCTL | 0x1a )
# define KID_BAD_PKT_READ  ( KID_IOCTL | 0x1b )

# define MKNOD_MODE  (  \
   S_IFMPX |           \
   S_IRUSR | S_IWUSR | \
   S_IRGRP | S_IWGRP | \
   S_IROTH | S_IWOTH   \
)

typedef struct {
    dev_t      devno;      /* device number of device */
    char    channame[20];  /* channel name of device (if any) */
    int        flags;      /* device open flags */
} DRVR_OPEN;

/*  Special write extension that accomodates both MPQP and  */
/*  "everyone else":                */

struct fastwrt_options{
   int   frames_req; /* frames per request for fastwrt */
   uchar offlevel;   /* send on offlevel if TRUE */
   int   oplvl;      /* offlevel level */
};

typedef struct {
    cio_write_ext_t  ciowe;      /* CIO write extension */
    unsigned char transparent;   /* required by MPQP */
    uchar      fastwrt_type;  /* for fast write support */

#define SHORT_MBUF_CHAIN   1  /* 4 or less mbufs */
#define LONG_MBUF_CHAIN    2  /* more than 4 mbufs */
#define SHORT_CLUSTER_CHAIN   3  /* 15 or less clusters */
#define LONG_CLUSTER_CHAIN 4  /* more than 15 clusters */
#define MIXED_CHAIN     5  /* mixture of clusters and mbufs */
#define NORMAL_CHAIN    6  /* normal chain of clusters and mbufs */

    struct fastwrt_options fastwrt_opt;   /* fastwrt options */
} WRITE_EXT;

/*
 *  This structure is used to pass all necessary dump information to the
 *  kid driver so that it can perform the tokdump calls.
 */
typedef struct {
   dev_t       devno;
   int         dump_cmd;
   struct dmp_query  dmp_query;
   struct uio     p_uio;
   char        *writebuf;
   int         writelen;
} DUMP_EXT;

/*------------------------------------------------------------------------*/
/*  Used internally by the driver:                                        */
/*------------------------------------------------------------------------*/

# define MBUF_FREE_WAIT    8  /* Eight seconds */

typedef struct {
    short      read_index;
    short      write_index;
    int        slot[ QUEUE_SIZE ];
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
    struct mbuf      *mbuf_to_free; /* mbuf to free on status */
    struct file      *fp;     /* kernel file pointer for test */
    QUEUE      read_queue; /* queue of read blocks */
    int        read_overflow; /* nonzero if read queue overflow */
    QUEUE      status_queue;  /* queue of status blocks */
    int        stat_overflow; /* nonzero if status queue overflow */
    struct cio_get_fastwrt fastwrt_parm;  /* fastwrt parameters */
    QUEUE      bad_pkt_queue; /* bad pkt queue */
    int        bad_pkt_overflow; /* nonzero if bad pkt que
                   * overflow
                   */
} OPEN_TBL;

typedef struct {
    struct mbuf         *m;      /* mbuf for this read */
    struct read_extension  rd_ext;  /* read extension data */
} RD_BLOCK;

struct etkid_bad_read
{
   int   len;  /* length of data buffer */
   caddr_t  data; /* put received data here */
   int   outlen;  /* length of data read */
   int   pktlen; /* length of what the pkt was */
};
typedef struct etkid_bad_read etkid_bad_read_t;

#ifdef DEBUG
#define  KID_NUM_TRACE  (256)
#define  KID_TRACE_SIZE (4)

static uint top_marker;
static uint trace_table[KID_NUM_TRACE][KID_TRACE_SIZE];
static uint trace_index;
static uint bot_marker;

#define KID_TRACE(tag,w1,w2)     \
   kid_trace((uint)tag, (uint)w1, (uint)w2)

#define  FASTWRT_ENTRY  0x66777245
#define  FASTWRT_EXIT   0x66777258
#define LAST_TRACE   0x6c617374
#define TOP_TRACE 0x746f7020
#define BOT_TRACE 0x626f7420

#endif
