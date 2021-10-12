/* @(#)03 1.13.1.1 src/bos/kernext/audio/acpa/oldacpa.h, sysxacpa, bos411, 9428A410j 92/03/03 18:51:58 */

/*
 * COMPONENT_NAME: SYSXACPA     Multimedia Audio Capture and Playback Adapter
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Number of kernel buffers to keep for each channel */

#ifdef MORE
#define False (0)		/* Incorrect */
#define True  (1)		/* Correct   */
#endif

#define MAXNKB (1800)            /* Maximum number of kernel buffers */
				 /* per track is 10 seconds * 158 */
				 /* interrupts per second */

typedef struct {
  unsigned short* data;		/* Ptr to the data or NULL */
  int   len;			/* Length of the data (number of shorts) */
} Shorts;			/* A vector of shorts */

#define STOP_EVENT      1       /* this is a stop event */
#define RING_EVENT      2       /* this is a ring event */
struct wdog_info
{
	struct watchdog wd;     /* watchdog information */
	int min;                /* minor number associated with adapter */
	int mpxchannel;         /* channel number for this device */
	int event_type;         /* is it a STOP or RING event? */
} ;

#define ENQUEUED        1       /* request is coming from the queue */
#define NOT_ENQUEUED    2       /* request is to be executed immediately */

/* This is the structure defining each request on the queue. */
struct audio_request
{
	unsigned int ioctl_request;     /* the desired ioctl request */
	void *request_info;             /* the request specifics */
	unsigned long position;         /* # of units before executing it */
	struct audio_request *next_ptr; /* pointer to list of requests */
	int min;                        /* minor number of adapter */
	int mpxchan;                    /* mpx channel id */
	int pid;                        /* process id of enqueuer */
} ;

typedef struct {
  int in_use;			/* True/False */
  /* This order is important; all "stopped" states must be <= Stopped. */
  enum { Uninitialized, Opened, Stopped, Almost_Started, Started, Waiting, Pausing }
  current_state;                /* current state of the trace */
  int open_flags;               /* the flags used to open the device */
  int pid;                      /* contains a process ID */
  struct wdog_info watch_it;    /* watchdog timer structure */
  audio_init rinfo;             /* sampling rate status */
  audio_status ainfo;           /* adapter status */
  track_info tinfo;             /* track status */
  int cur_buf;			/* Current DSP buffer (0-3) from DSP */
  int host_cur_buf;		/* Current host buffer from DSP */
  int buf_size;			/* Buffer size (in words), depends on rate */
  int buf_loc[16];              /* Location of buffers in DSP memory */
  int first_underrun;		/* Flags whether underrun occurred already */
  unsigned short host_buf_count;/* driver's counter of blocks processed */
  unsigned short host_buf_ptr;  /* indicates the next adapter buffer to fill */
  int preload_count;            /* number of buffers preloaded into DSP */
  int preload_max;              /* maximum number of buffers in DSP */
  int modulo_factor;            /* what number to use in modulo divisions */
  audio_buffer buffer;          /* various buffer information fields */
  unsigned short* rbuf[MAXNKB]; /* Pointers to the read buffers */
  unsigned short* wbuf[MAXNKB]; /* Pointers to the write buffers */
  int host_head;                /* Most recently filled kernel buffer */
  int host_tail;		/* Buffer "behind" least recently filled */
  int host_n_bufs;		/* N. filled buffers in kernel queue */
  int waiting;			/* Someone's waiting on this channel */
  int ring_event;		/* Event used by wakeup/sleep between */
				/* the interrupt handler and the */
				/* read/write routines to manage the */
				/* ring of kernel buffers. */  
  int stop_event;		/* Event used by wakeup/sleep between */
				/* the interrupt handler and the close */
				/* routine */
  int rnkb;                     /* # of read kernel buffers for track */
  int wnkb;                     /* # of write kernel buffers for track */
  struct audio_request *request_list;
				/* pointer to start of request queue */
  int num_requests;             /* the number of requests in the queue */
  int write_flag;               /* Flags when write work is being done */
				/* for synchronization with AUDIO_WAIT. */
  int extra_data_left_over;     /* true when a read does not read a full block */
  int extra_data_size;          /* number of bytes left from a partial read */
  int partial_block_left_over;  /* true when a write does not fill a full block */
  int partial_block_size;       /* number of bytes left from a partial write */
} Track;                        /* Information about a Track */

#ifndef i386
struct acpa_dds {
	int int_level;
	int priority;
	int slot_number;
	ulong bus_addr;
	dev_t devno;
	int fd_arr[4];
	char mcode[6][MAXPATHLEN];        /* to allow additional microcode */
	unsigned int MACPA_5r_secs;
	unsigned int MACPA_8r_secs;
	unsigned int MACPA_11r_secs;
	unsigned int MACPA_22r_secs;
	unsigned int MACPA_44r_secs;
	unsigned int MACPA_5w_secs;
	unsigned int MACPA_8w_secs;
	unsigned int MACPA_11w_secs;
	unsigned int MACPA_22w_secs;
	unsigned int MACPA_44w_secs;
	unsigned int MACPA_request_buf;
};
#endif

/* These contain the images of the microcode in memory. */

/* acpa[].ucode_loaded points to NULL or one of the following: */
Shorts playback_ucode = { NULL, 0 };
			      /* Image of the ADPCM playback */
			      /* microcode to be loaded. */
Shorts record_ucode = { NULL, 0 };
			      /* Image of the ADPCM record */
			      /* microcode to be loaded. */
Shorts playback_pcm_ucode = { NULL, 0 };
			      /* Image of the PCM playback */
			      /* microcode to be loaded. */
Shorts record_pcm_ucode = { NULL, 0 };
			      /* Image of the PCM recording */
			      /* microcode to be loaded. */
Shorts playback_22_ucode = { NULL, 0 };
                              /* Image of the 22kHz ADPCM playback */
                              /* microcode to be loaded. */
Shorts record_22_ucode = { NULL, 0 };
                              /* Image of the 22kHz ADPCM record */
                              /* microcode to be loaded. */

struct intr_status
{
	struct intr handler;    /* standard handler information */
	int min;                /* minor number associated with adapter */
} ;

struct acpa_status {
#ifdef i386
  /* AIX PS/2=specific items. */
#else
  /* AIX V3-specific items. */

  struct intr_status intr_info; /* Interrupt handler definition */
  struct acpa_dds dds;		/* Device-dependent structure */
  struct devsw dev;		/* Device switch table entry */
  int sleep_code;		/* contains return code for */
				/* sleep: 0 = success */
#endif 

  /* State of the card */

  int running;			/* True or False */
  enum { Nothing, Playing, Recording }
  doing_what;                   /* What are we doing if it's in use? */
  int ps2_speaker_enabled;	/* True or False */

  int can_interrupt_host;	/* True or False */
  Shorts* ucode_loaded;         /* Which set of microcode is loaded */
				/* into the Shared Memory? Points to */
				/* buffer with microcode or NULL. */
  /* Track status */

  Track track[2];               /* status for one track */

  /* Pre-defined port numbers (for efficiency) */

  int base;			/* Base I/O address (0 = no card in system) */
  int data_low_reg;		/* Low order byte of data read or written */
  int data_hi_reg;		/* High order byte of data read/written */
  int addr_low_reg;		/* Low order byte of address */
  int addr_hi_reg;		/* High order byte of address */
  int cmd_stat_reg;		/* Command and status register */

  int irqlevel;			/* Interrupt request level (hardware) */
  int int_count;		/* Interrupt count */

  int recursion;		/* Recursion occurred flag */

  int closing;			/* Playback CLOSE is in progress */

  int timeout;			/* N. seconds for watchdog timer */
  int secondtime;		/* Flags whether 2 playback tracks are open */
				/* simultaneously. */

} acpa[16];                     /* one for each possible slot position */

/* Macros to differentiate between the various versions of the driver */

/* Used to return out of a process level routine (open, close, read, */
/* write, etc) with a given "errno" */

#ifdef i386
#define RETURN(errcode) {\
			u.u_error = errcode;\
			return (0);         \
                        }
#else
#define RETURN(errcode) return(errcode)
#endif 


/* Routines used to perform in and out operations.  On the PS/2 these */
/* are synonyms for existing kernel routines; in V3 these are real */
/* routines that we provide. */

#ifdef i386
#define acpaout(port, data)  ioout(port, data)
#define acpaoutb(port, data) iooutb(port, data)
#define acpain(port)         ioin(port)
#define acpainb(port)        ioinb(port)
#else
void           acpaout(int port, unsigned short data);
void           acpaoutb(int port, unsigned char data);
unsigned short acpain(int port);	
unsigned char  acpainb(int port);
#endif 

/* AIX PS/2 and AIX V3 use different names to refer to flag bits given */
/* to the open entry point.  Map them to a consistent set. */

#ifdef i386
#define OPEN_NDELAY          FNDELAY
#define OPEN_READ            FREAD
#define OPEN_WRITE           FWRITE
#define OPEN_APPEND          FAPPEND
#else
#define OPEN_NDELAY          DNDELAY
#define OPEN_READ            DREAD
#define OPEN_WRITE           DWRITE
#define OPEN_APPEND          DAPPEND
#endif /* i386 */

/* AIX PS/2 and AIX V3 use different mechanisms for getting kernel */
/* memory. */

#ifdef i386
#define GET_MEMORY(size)     kmemalloc(size, MA_DBLWD|MA_OK2SLEEP|MA_LONGTERM)
#define FREE_MEMORY(addr)    mfree(addr)
#else
#define GET_MEMORY(size)     xmalloc(size, 3, pinned_heap)
#define FREE_MEMORY(addr)    xmfree(addr, pinned_heap)
#endif /* i386 */

/* In AIX PS/2 the contents of the event words are irrelevant; in AIX */
/* V3 they must be initialized. */

#ifdef i386
#define EVENT_NULL (0)		/* Defined already in V3 */
#endif 

/* Routines for enabling and disabling interrupts */

#ifdef i386
#define ENABLE_INTERRUPTS(old)   splx(old)
#define DISABLE_INTERRUPTS()     spl7()
#else
#define ENABLE_INTERRUPTS(old)   i_enable(old)
#define DISABLE_INTERRUPTS()     i_disable(INTMAX)
#endif /* i386 */

/* Routines for sleeping on events and waking up sleeping processes */

#ifdef i386
#define SLEEP_BROKEN_BY_SIGNAL        TS_SIG
#define SLEEP_BROKEN_BY_WAKEUP        TS_OK
#define TIMED_SLEEP(event, rc)        rc = tsleep(event, PSWP, acpa.timeout)
#define SLEEP_TIMED_OUT               TS_TIME
#define WAKEUP(event)                 wakeup(event)
#else
#define SLEEP_BROKEN_BY_SIGNAL        EVENT_SIG
#define SLEEP_BROKEN_BY_WAKEUP        EVENT_SUCC
#define TIMED_SLEEP(event, rc, type) {                                  \
	acpa[min].track[mpxchannel].watch_it.event_type = type;         \
	w_start(&(acpa[min].track[mpxchannel].watch_it.wd));            \
	rc = e_sleep (event, EVENT_SIGRET);                             \
        if (rc != SLEEP_BROKEN_BY_SIGNAL) 	                        \
	    rc = acpa[min].sleep_code;                                  \
	}                                                               \
	w_stop (&(acpa[min].track[mpxchannel].watch_it.wd));
#define SLEEP_TIMED_OUT               146
#define WAKEUP(event, type) {                                           \
	acpa[min].sleep_code = SLEEP_BROKEN_BY_WAKEUP;                  \
	e_wakeup(event);                                                \
	}
#define WAKEUP2(event, type) {                                          \
	acpa[min].sleep_code = SLEEP_BROKEN_BY_WAKEUP;                  \
	e_wakeup(event);                                                \
	}
#endif 

/* Various missing (mostly ignored) constants in AIX PS/2 */

#ifdef i386
#define INTR_FAIL                      (0) /* Intr routine failed */
#define INTR_SUCC                      (0) /* Intr routine succeeded */
#endif 

/* Fields in the "u" and/or "uio" blocks */

#ifdef i386
#define USER_OFFSET             u.u_offset
#define USER_COUNT              u.u_count
#else
#define USER_OFFSET             uiop->uio_offset
#define USER_COUNT              uiop->uio_resid
#endif 

/* Routine to move data between user space and kernel space */

#ifdef i386
#define IO_MOVE_READ              B_READ
#define IO_MOVE_WRITE             B_WRITE
#define IO_MOVE(where, size, rw)  (void) iomove(where, size, rw)
#define IO_MOVE_RC                u.u_error
#else
#define IO_MOVE_READ              UIO_READ
#define IO_MOVE_WRITE             UIO_WRITE
#define IO_MOVE(where, size, rw)  uiomove_rc = uiomove(where, size, rw, uiop)
#define IO_MOVE_RC                uiomove_rc
#endif 

/* Routine to delay for a number of ticks */

#ifdef i386
#define DELAYTICKS(ticks)              delayticks(ticks)
#else
#define DELAYTICKS(ticks)              delay(ticks)
#endif 

