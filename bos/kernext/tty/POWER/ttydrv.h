/* @(#)71 1.3 src/bos/kernext/tty/POWER/ttydrv.h, sysxcommon, bos412, 9447A 11/11/94 14:27:22 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS:
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_TTYDRV
#define _H_TTYDRV

#include <sys/cblock.h>
#include <sys/ioctl.h>
#include <sys/lock_def.h>

enum csize {				/* bits per character */
    bits5, bits6, bits7, bits8};	
enum parity {				/* parity */
    nopar, oddpar, markpar, evenpar, spacepar, oddonly};
enum stopbits {				/* stop bits */
    stop1, stop2};

typedef unsigned int baud_t;
typedef int csize_t;
typedef int parity_t;
typedef int stop_t;

#define TTY_BAUD_ERROR_LIMIT 2		/* 2% slop permitted in baud rates */

struct ccblock {
    void *c_vptr;			/* pointer untouched by hardware */
    char *c_ptr;			/* buf address used by hardware */
    ushort c_count;			/* char count altered by hardware */
    ushort c_size;			/* buffer size used by discipline */
};

/* service commands */
enum service_commands {
    TS_PROC,				/* old proc commands */
    TS_SCONTROL,			/* set control lines */
    TS_GCONTROL,			/* get control lines */
    TS_GSTATUS,				/* get status lines */
    TS_SBAUD,				/* set baud rate (in and out) */
    TS_GBAUD,				/* get output baud rate */
    TS_SIBAUD,				/* set input baud rate */
    TS_GIBAUD,				/* get input baud rate */
    TS_SBPC,				/* set bits per character */
    TS_GBPC,				/* get bits per character */
    TS_SPARITY,				/* set parity */
    TS_GPARITY,				/* get parity */
    TS_SSTOPS,				/* set stop bits */
    TS_GSTOPS,				/* get stop bits */
    TS_SBREAK,				/* start break condition */
    TS_CBREAK,				/* clear break condition */
    TS_OPEN,				/* local/remote style on next open */
    TS_DOPACE,				/* do pacing */
    TS_SOFTPACE,			/* software pacing */ 
    TS_SOFTRCHAR,			/* define software pacing chars */
    TS_SOFTLCHAR,			/* define software pacing chars */
    TS_SOFTRSTR,			/* define software pacing strings */
    TS_SOFTLSTR,			/* define software pacing strings */
    TS_HARDRBITS,			/* define hardware pacing bits */
    TS_HARDLBITS,			/* define hardware pacing bits */
    TS_LOOP,				/* enter/exit loop mode */
    TS_PRIOUTPUT			/* Prioritary outout chars */
};

/* proc commands */
enum proc_commands {
    T_OUTPUT,				/* start output flowing */
    T_SUSPEND,				/* suspend output */
    T_RESUME,				/* resume output */
    T_BLOCK,				/* block input */
    T_UNBLOCK,				/* unblock input */
    T_RFLUSH,				/* flush read buffers */
    T_WFLUSH				/* flush write buffers */
};

/* Bits defines for TS_SCONTROL, TS_GCONTROL, and TS_GSTATUS */
#define TSDTR 0x01
#define TSRTS 0x02
#define TSCTS 0x04
#define TSDSR 0x08
#define TSRI  0x10
#define TSCD  0x20

/* 
 * TS_DOPACE is sent to the hardware with the particular type of 
 * pacing being requested.  If the hardware can perform the function, 
 * the request should return with 0.  After this point, all pacing 
 * commands will be sent on to the service routine.
 */
enum str_ts_dopace {			/* used with TS_DOPACE */
    DOPACE_AGAIN,			/* no flow control handled */
    DOPACE_XON,				/* do xon/xoff flow control */
    DOPACE_STR,				/* string style flow control */
    DOPACE_DTR,				/* do dtr flow control */
    DOPACE_RTS				/* do rts flow control */
};

/* 
 * The service requests for softpace are sent down from the line 
 * discipline.  An application will frequently require all 256 
 * possible characters be sent up to it in which case the line 
 * discipline will send down a SOFTPACE_OFF command.  In the case 
 * where the application does not care, then the line discipline will 
 * send down a SOFTPACE_ANY, ON, or STR depending upon the startup 
 * conditions.  The design currently permits multi character sequences 
 * for software pacing by using the STR option.  The other two use 
 * only single chars to improve throughput.  ANY permits any character 
 * for the remote start character.  Hardware pacing should ignore 
 * these commands.
 */
enum ts_softpace {			/* in line software pacing */
    SOFTPACE_ROFF,			/* permit all data to pass through */
    SOFTPACE_RANY,			/* xon/xoff with any char for resume */
    SOFTPACE_RON,			/* xon/xoff */
    SOFTPACE_RSTR,			/* string based flow control */
    SOFTPACE_LOFF,			/* local pacing off */
    SOFTPACE_LON,			/* local pacing on (char) */
    SOFTPACE_LSTR			/* local pacing string */
};

/* Used with TS_OPEN to control how the next open will work */
enum ts_open {
    OPEN_LOCAL,				/* do not wait for carrier */
    OPEN_REMOTE				/* wait for carrier detect */
};

/* 
 * TS_SOFTRCHAR and TS_SOFTLCHAR arg is a pointer to an array of 2
 * chars.  The first is the stop character and the second is the start
 * character. The local chars are sent out to do local pacing.  The
 * input is scanned for the remote chars to do remote pacing.  (Local
 * pacing is pacing generated locally to control the input from the
 * other host.  Remote pacing is generated on the other host to
 * control the output to it.  Usually a person sitting at a terminal
 * wants remote pacing but not local pacing.  When the first
 * DOPACE_XON request is sent down (upon the very first open), ^Q/^S
 * is assumed.
 */

/* TS_SOFTRSTR and TS_SOFTLSTR arg is a uio strcture with 2 iovec's.
 * The first is for block (or suspend) while the second is for unblock
 * (or resume).  The local strings are sent out by the host if
 * everything else is enabled.  The remote strings are matched up.
 * During a multi-byte remote search, there is no timeout implemented
 * (on purpose).  Currently this is not implemented.
 */

/* 
 * The arg for TS_HARDRBITS and TS_HARDLBITS is a pointer to an array
 * of 4 chars.  Each char contains a set of TSxxx bits and are grouped
 * in pairs.  The characters for TS_HARDLBITS is defined as follows.
 * The first pair is for block (or stop) while the second pair is for
 * unblock (or start).  The first character in the group is an OR
 * mask, the second char is an AND mask.  When the host wants to block
 * the other side (as an example) it will set the modem status lines
 * to their ((current settings|OR)&AND).  The same is true with
 * unblock.  Usually only one bit is set but multiple bits are
 * allowed.
 *
 * The characters for TS_HARDRBITS are defined as follows.  The first
 * pair used for suspend (or stop) and the second pair used for resume
 * (or start).  The first char of each pair is the set of bits which
 * must be SET and the second char is the one's complement of the set
 * of bits which must be CLEAR for the output to be suspended or
 * resumed.  Thus to suspend output, the condition (((current settings
 * & SET) == SET) && ((current settings | CLEAR) == CLEAR)) must be
 * true.  This condition is tested only when a modem status change
 * occurs.  This is currently used only by the hardware pacing
 * discipline.
 */

/* Used with TS_LOOP command */
enum ts_loop {
    LOOP_ENTER,				/* Enter Loop mode */
    LOOP_EXIT				/* Exit Loop mode */
};

/* These two must track each other */
/* interrupt priority for tty's from offlevel up */
#define INT_TTY INTOFFL0
/* Initialize a intr struct for a tty offlevel */
#define INIT_TTY_OFFL(a, b, c) INIT_OFFL0(a, b, c)

/* Simple macro for sleeps in all the tty code */
#define TTY_SLEEP(tp) (e_sleep(&tp->t_event, EVENT_SIGRET) == EVENT_SIG)
#define TTY_WAKE(tp)  e_wakeup(&tp->t_event)

#ifdef	_KERNEL

/*
 * Allocate and realease pinned memory.
 */
#define	pinmalloc(x)	xmalloc((x), 3, pinned_heap)
#define	pinfree(x)	xmfree((x), pinned_heap)

#endif /* _KERNEL */

#define DRIVER_ID 112
#define SRS_RDBUFSZ 64

#endif /* _H_TTYDRV */
