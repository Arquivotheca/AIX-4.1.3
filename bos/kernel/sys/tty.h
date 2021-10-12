/* @(#)26	1.48.1.1  src/bos/kernel/sys/tty.h, cmdtty, bos411, 9428A410j 2/10/93 11:57:07 */

/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TTY
#define _H_TTY

#include <sys/cblock.h>
#include <sys/ioctl.h>
#include <sys/lock_def.h>

enum csize {				/* bits per character */
    bits5, bits6, bits7, bits8};	
enum parity {				/* parity */
    nopar, oddpar, markpar, evenpar, spacepar};
enum stopbits {				/* stop bits */
    stop1, stop2};
enum status {
    good_char, overrun, parity_error, framing_error, break_interrupt,
    cts_on, cts_off, dsr_on, dsr_off, ri_on, ri_off, cd_on, cd_off,
    cblock_buf, other_buf};

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

/* ttydd init structure */
struct ttyddinit {
	mid_t	kmid;			/* mid_t for ttydd */
	dev_t	devno;			/* device number for device */
	chan_t	chan;			/* channel number for device */
	char	linedisp[64];		/* name of line discipline to use */
	char	opendisp[64];		/* name of open discipline to use */
	char	mapdisp[64];		/* name of mapping discipline to use */
	char	flowdisp[10][64];	/* name of flow disciplines to use */
	int	flowdispcnt;		/* nuber of control disciplines */
};

/* Discipline structure, etc */

/* The following determine how the disciplines will stack */
enum disp_type {
    hard_disp = 0,			/* hardware is first */
    open_disp = 50,			/* open discipline */
    flow_disp = 100,			/* xon/xoff or rts/ctr */
    map_disp = 150,			/* NLS/kanji */
    line_disp = 255			/* line disp should be last */
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
    TS_NEGOTIATE			/* negotiate who does what */
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
enum ts_dopace {			/* used with TS_DOPACE */
    DOPACE_AGAIN,			/* renegotiate the pacing */
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

/*
 * TS_NEGOTIATE uses the following structure.  The n_oflag, n_iflag,
 * and n_lflag are from are from the termios c_oflag, c_iflag, and
 * c_lflag.  The c_cflag is passed around and IXON, IXOFF, and IXANY
 * are stripped out of the n_iflag since all of those attributes are
 * negotiated using other service calls.  The n_cc array matches the
 * c_cc array of the termios structure but VSTART and VSTOP should be
 * ignored.  Note a very important catch is that these bits are
 * defined by the SVID 5.4 spec and not by Posix.  The 5.4 spec is a
 * super set of Posix.
 *
 * The n_xflag is for bits not in 5.4.  The current implementation has
 * only IMAP and OMAP which are set when the nls mapping discipline
 * are pushed onto the stack.  The IMAP means that input mapping as
 * well as the associated ioctls must be done and OMAP means that
 * output mapping must be done.
 *
 * The process will start from the current line discipline and a
 * TS_NEGOTIATE will be passed down.  As the request is passed down
 * other disciplines may add bits into the structure.  The hardware
 * will then have first change to assume any of the responsibilities
 * that it wants to.  It will do this by turning off the appropriate
 * bits in the structure.  As the service call returns back up the
 * stack, the higher level disciplines can assume responsibilities in
 * the same fashion by stripping out bits.  They must also check to
 * see if someone else below them assumed the responsibilities of what
 * they were asking for.
 *
 * A discipline must start the negotiation by calling the top level
 * service routine in their open and close routines if they perform
 * any of the duties represented in the negotiate structure.
 *
 * N_VERS_NUM will change whenever the structure changes but it will
 * always be in the same place in the xflag which will always be the
 * first field in the structure.  N_VERS_NUM will not change if new
 * bits are added to the xflag.  So if a discipline discovers an
 * unknown version or an unknown bit in the xflag, it should not
 * assume any responsibilities.
 *
 * Note that for those disciplines that want to use this structure and
 * service, termios.h must be included before tty.h.  Otherwise the
 * negotiate structure is not defined.
 *
 * This service is called at INT_TTY since it all need to sync up
 * atomicly.  The line discipline ignores the argument.
 */
#ifdef NCCS
struct negotiate {
    tcflag_t n_xflag;
    tcflag_t n_iflag;
    tcflag_t n_oflag;
    tcflag_t n_lflag;
    cc_t *n_cc;
};

/* Current version of negotiate structure */
#define N_VERS_NUM 1

/* bits for the n_xflag */
#define N_VERS 0x000000FF		/* what vers. of negotiate struct */
#define N_IMAP 0x00000100		/* do input nls mapping */
#define N_OMAP 0x00000200		/* do output nls mapping */

#define N_KNOWN (N_VERS|N_IMAP|N_OMAP)
#endif  
 
enum alloc_commands {
    alloc_push,				/* discipline being pushed in */
    alloc_pop,				/* discipline being popped out */
    alloc_unconfig			/* tty is being unconfigured */
};

typedef struct ctlpath ctlpath_t;
typedef struct disp disp_t;
typedef struct dispx dispx_t;
typedef struct line_disp ldisp_t;
typedef struct tty *ttyp_t;

struct ctlpath {
    ctlpath_t *ctl_next, *ctl_prev;	/* doubly linked list */
    ttyp_t ctl_tp;			/* back to tty structure */
    disp_t *ctl_disp;			/* original pacing structure */

    int (*ctl_input)(ttyp_t tp,		/* receive interrupt routine */
		     ctlpath_t *ctl,	/* pointer to ctlpath record */
		     char c,		/* character received */
		     enum status s);	/* status of character */
    ctlpath_t *ctl_ipath;		/* next on input path */

    int (*ctl_output)(ttyp_t tp,	/* output routine */
		      ctlpath_t *ctl);	/* pointer to ctlpath record */
    ctlpath_t *ctl_opath;		/* next on output path */

    int (*ctl_service)(ttyp_t tp,	/* service entry */
		       ctlpath_t *ctl,	/* pointer to ctlpath record */
		       enum service_commands cmd,
		       void *arg);
    ctlpath_t *ctl_spath;		/* next on service path */
};

struct disp {
    disp_t *ds_next;			/* local/remote pacing lists */
    char *ds_name;			/* pacing name */
    int (*ds_init)(int cmd,		/* command */
		   struct uio *uio);	/* data */
    int (*ds_alloc)(ttyp_t tp,		/* allocate routine */
		    ctlpath_t **ctlp,	/* pointer to ctl pointer */
		    enum alloc_commands cmd); /* what to do */
    int (*ds_open)(ttyp_t tp,		/* open routine */
		   ctlpath_t *ctl,	/* ctlpath ptr */
		   int mode,		/* FREAD, etc */
		   int ext);		/* openx parameter */
    int (*ds_close)(ttyp_t tp,		/* close routine */
		    ctlpath_t *ctl,	/* ctlpath ptr */
		    int ext);		/* closex parameter */
    int (*ds_ioctl)(ttyp_t tp,		/* ioctl routine */
		    ctlpath_t *ctl,	/* pointer to ctlpath record */
		    int cmd,		/* second arg to ioctl sys call */
		    void *arg,		/* third arg to ioctl sys call */
		    int mode,		/* open mode FREAD, etc */
		    int ext);		/* ioctlx parameter */
    int (*ds_print)(ttyp_t tp,		/* print routine */
		    ctlpath_t *ctl,	/* ctl pointer */
		    int v);		/* verbose flag */
    uchar ds_type;			/* type of discipline */
    mid_t ds_kmid;
};

struct dispx {
    int dsx_npush;			/* system wide push count */
    lock_t dsx_lock;			/* lock to protect dsx_npush */
    int (*dsx_alloc)(ttyp_t tp,		/* allocate routine */
		    ctlpath_t **ctlp,	/* pointer to ctl pointer */
		    enum alloc_commands cmd); /* what to do */
    void (*dsx_pin)();			/* argument to (un)pincode */
};

struct line_disp {
    disp_t ld_disp;
    int (*ld_read)(ttyp_t tp,		/* read routine */
		   ctlpath_t *ctl,	/* ctl pointer */
		   struct uio *uio,	/* uio struct */
		   int ext);		/* readx parameter */
    int (*ld_write)(ttyp_t tp,		/* write routine */
		    ctlpath_t *ctl,	/* ctl pointer */
		    struct uio *uio,	/* uio struct */
		    int ext);		/* writex parameter */
    int (*ld_select)(ttyp_t tp,		/* select routine */
		     ctlpath_t *ctl,	/* ctl pointer */
		     short events,	/* events to select for */
		     short *revents);	/* ptr to events selected for */
    int (*ld_revoke)(ttyp_t tp,		/* revoke routine */
		     ctlpath_t *ctl,	/* ctl pointer */
		     int flag);		/* 0 if frevoke, 1 if revoke */
};

struct tty {
    char t_name[16];			/* resource name for this tty */
    dev_t t_dev;			/* major/minor number */
    chan_t t_channel;			/* channel number */
    pid_t t_sid;			/* controlling process, session id */
    pid_t t_pgrp;			/* foreground process group id     */
    pid_t t_tsm;			/* terminal state manager's id */
    int t_id;				/* tty id */
    struct winsize t_winsize;		/* window struct */

    struct ccblock t_rbuf;		/* used only for compatability */
    struct ccblock t_tbuf;		/* transmit buffer */
    struct clist t_rawq;		/* Raw queue */
    struct clist t_canq;		/* canonical queue */
    struct clist t_outq;		/* output queue */

    signed char *t_cmap;		/* character mapping tables */
    ctlpath_t *t_ctl;			/* control path stack */
    ctlpath_t *t_lctl;			/* control path for line discipline */
    void *t_hptr;			/* hardware storage */
					/* other things */
    int t_ihog;				/* input hog limit for this tty */
    int t_ohog;				/* output hog limit for this tty */
    int t_event;			/* event list for e_sleep */
    int t_lock;				/* lock per tty */
    int t_ccnt;				/* closing count */
					/* Bits and things */
    baud_t t_ospeed;			/* output speed */
    baud_t t_ispeed;			/* input speed */
					/* non-disabled flags */
    stop_t t_stopbits : 2;		/* number of stop bits */
    parity_t t_parity : 3;		/* parity style */
    csize_t t_csize : 2;		/* character size */
					/* hardware state flags */
    uint t_carrier : 1;			/* carrier flag */
    uint t_wopen : 1;			/* waiting for open */
    uint t_isopen : 1;			/* tty is open */
    uint t_iclose : 1;			/* tty is closing */
    uint t_trust : 1;			/* trusted path */
    uint t_cons : 1;			/* console redirection active */
    uint t_stack : 1;			/* stack change pending */
    uint t_wstack : 1;			/* entry point waiting stack change */
    ushort unused0;			/* filler */
					/* disabled INT_TTY flags */
    uint t_busy : 1;			/* output in progress */
    uint t_stop : 1;			/* output is stopped, remote pacing */
    uint t_block : 1;			/* input is stopped, local pacing */
    uint t_ctlx : 1;			/* got a control X */
    uint t_sak : 1;			/* sak is enabled */
    uint t_kep : 1;			/* kep test */
    uint t_async : 1;			/* FASYNC mode */
    uint t_nbio : 1;			/* FNDELAY mode */
					/* always sleep on t_event now. */
    uint t_iaslp : 1;			/* asleep for input */
    uint t_oaslp : 1;			/* asleep for output, process pacing */
    uint t_iow : 1;			/* waiting for flush */
};

/*
 * The t_cmap pointer actually points to a csmap structure which
 * contains a width array and a length array.  See TCLEN & TCWIDTH below.
 */
struct csmap {
    char csmap_length[256];
    char csmap_width[256];
};

/* 
 * TCLEN is the length in bytes of the character with "c" as its first 
 * byte
 */
#define TCLEN(tp, c) ((tp)->t_cmap[(unsigned int)(c)])

/* 
 * TCWIDTH is the display width in columns of the character with "c" 
 * as its first byte
 */
#define TCWIDTH(tp, c) ((tp)->t_cmap[(unsigned int)(c)+256])

/*
 * Files used to build code set maps are kept in the following directory.
 */
#define CSMAP_DIR "/usr/lib/nls/csmap/"

/* 
 * To get work done initiated by a timeout to be done at INT_TTY, the
 * func in the trb is set to ttyofflevel and func_data should point to 
 * a struct tty_timer.  The tttmr_func should point to the function 
 * which needs to get called at offlevel time, tttmr_data is the 
 * parameter passed to it (probably the control stack structure for 
 * the discipline).  (ipri in the trb should be INT_TTY -- although 
 * that does little good.
 */
struct tty_timer {
    struct tty_timer *tttmr_next;	/* used by ttyofflevel */
    struct trb *tttmr_trb;		/* trb pointer */
    void (*tttmr_func)();		/* offlevel functin to call */
    void *tttmr_data;
};

/* These two must track each other */
/* interrupt priority for tty's from offlevel up */
#define INT_TTY INTOFFL0
/* Initialize a intr struct for a tty offlevel */
#define INIT_TTY_OFFL(a, b, c) INIT_OFFL0(a, b, c)

/* hi and low water marks for flow control */
#define TTLOWAT(tp) (tp->t_ihog >> 2)
#define TTHIWAT(tp) (tp->t_ihog - (tp->t_ihog >> 2))

/* hi and low water marks for controling output data from process */
#define TTLOPWAT(tp) ((tp->t_ospeed / 100) + 30)
#define TTHIPWAT(tp) ((tp->t_ospeed / 20) + 100)

/* Simple macro for sleeps in all the tty code */
#define TTY_SLEEP(tp) \
    (e_sleepl(&tp->t_lock, &tp->t_event, EVENT_SIGRET) == EVENT_SIG)
#define TTY_WAKE(tp)  e_wakeup(&tp->t_event)

#define ttyread(tp, uio, ext) \
    (((tp)->t_lctl) ? \
     (*((ldisp_t *)(tp)->t_lctl->ctl_disp)->ld_read) \
     ((tp), (tp)->t_lctl, (uio), (ext)) : \
     0)

#define ttywrite(tp, uio, ext) \
    (((tp)->t_lctl) ? \
     (*((ldisp_t *)(tp)->t_lctl->ctl_disp)->ld_write) \
     ((tp), (tp)->t_lctl, (uio), (ext)) : \
     0)

#define ttyselect(tp, evt, revt) \
    (((tp)->t_lctl) ? \
     (*((ldisp_t *)(tp)->t_lctl->ctl_disp)->ld_select) \
     ((tp), (tp)->t_lctl, (evt), (revt)) : \
     0)

#define ttyrevoke(tp, flag) \
    (((tp)->t_lctl) ? \
     (*((ldisp_t *)(tp)->t_lctl->ctl_disp)->ld_revoke) \
     ((tp), (tp)->t_lctl, (flag)) : \
     0)

#define ttyinput(tp, ctl, c, s) \
    ((*(ctl)->ctl_ipath->ctl_input)((tp), (ctl)->ctl_ipath, (c), (s)))
#define ttyoutput(tp, ctl) \
    ((*(ctl)->ctl_opath->ctl_output)((tp), (ctl)->ctl_opath))
#define ttyservice(tp, ctl, cmd, arg) \
    ((*(ctl)->ctl_spath->ctl_service)((tp), (ctl)->ctl_spath, (cmd), \
				      (void *)(arg)))

/* 
 * True when the input path past the current discipline is fed to tty 
 * null
 */
#define tty_noinput(ctl) ((ctl)->ctl_ipath->ctl_input == (void *)ttynull)

/* True when the current path is the last close of the tty */
#define tty_lclose(tp) ((tp)->t_ccnt == 1 && (tp)->t_iclose)

/* Definitions of common tty variables */

extern disp_t *disp_list;

/* Definitions of common tty routines */

extern int ttyioctl(ttyp_t tp, int cmd, void *arg, int mode, int ext);
extern int ttyinit(ttyp_t tp, disp_t *disp);
extern int ttyfree(ttyp_t tp);
extern int ttyalloc(ttyp_t tp, ctlpath_t **cinp,
		    enum alloc_commands cmd, dispx_t *d);
extern int ttyopen(ttyp_t tp, int mode, int ext);
extern int ttyclose(ttyp_t tp, int ext);
extern int ttynull();
extern int ttysak(ttyp_t tp);
extern int ttrevoke(ttyp_t tp, ctlpath_t *ctl, int flag);
extern int ttcwait(ttyp_t tp);
extern int ttypath(ctlpath_t *ctl, int which, int (*f)());
extern int stack_ctl(ttyp_t tp, disp_t *disp, int mode, int ext);
extern int unstack_ctl(ttyp_t tp, ctlpath_t *ctl, int mode, int ext);
extern ctlpath_t *getctlbytype(ttyp_t tp, enum disp_type type);
extern ctlpath_t *getctlbyname(ttyp_t tp, char *name);
extern disp_t *getdispbyname(char *name);
extern disp_t *getdispbytype(enum disp_type type);
extern int disp_add(disp_t *d);
extern int disp_del(disp_t *d);
extern void dbg_clist(char *s, struct clist *cl);
extern void ttyofflevel(struct trb *trb);
extern ttyp_t ttydev2tp(dev_t devno, chan_t channel);

#define _TTYHKID(w) ((TTY_TRCID)|w)

enum tty_trctypes {
    TTY_CONFIG, TTY_ALLOC, TTY_OPEN, TTY_CLOSE, TTY_READ, TTY_WRITE,
    TTY_IOCTL, TTY_SELECT, TTY_REVOKE, TTY_MPX, TTY_INPUT, TTY_OUTPUT,
    TTY_SERVICE, TTY_PROC, TTY_SLIH, TTY_OFF, TTY_LAST };

#ifdef	_KERNEL

/*
 * Allocate and realease pinned memory.
 */
#define	pinmalloc(x)	xmalloc((x), 3, pinned_heap)
#define	pinfree(x)	xmfree((x), pinned_heap)

/* 
 * General entry/exit trace hooks -- probably don't want to use this 
 * for the ioctl entries.
 * The names of the variables used by Enter/Return/Exit are different than
 * the names used in Enterv/Returnv/Exitv to prevent uses of the wrong
 * macros.
 */

#define Enter(type, w, dev, chan, a, b, c)				\
    type Func;								\
    dev_t Dev;								\
    chan_t Chan;							\
    int Flag1;								\
    int Flag2;								\
    int Line;								\
    int Which = !TRC_ISON(0) ? (Flag1 = Flag2 = 0) :			\
	((Dev = dev),							\
	 (Chan = chan),							\
	 (Flag1 = TTY_TRCMK[0]&(1<<w)),					\
	 (Flag2 = TTY_TRCMK[1]&(1<<w)),					\
	 (Flag1 && (TRCHKGT(_TTYHKID(w), Dev, Chan, a, b, c), 0)), w)

#define Enterv(w, dev, chan, a, b, c)					\
    dev_t Dev;								\
    chan_t Chan;							\
    int Flag1;								\
    int Line;								\
    int Which = !TRC_ISON(0) ? (Flag1 = 0) :				\
	((Dev = dev),							\
	 (Chan = chan),							\
	 (Flag1 = TTY_TRCMK[0]&(1<<w)),					\
	 (Flag1 && (TRCHKGT(_TTYHKID(w), Dev, Chan, a, b, c), 0)), w)

#define Return(val) {							\
    Func = (val);							\
    Line = __LINE__;							\
    goto ExitLabel;							\
}

#define Returnv {							\
    Line = __LINE__;							\
    goto vExitLabel;							\
}

#define Exit() {							\
ExitLabel:								\
    if (Flag1 || (Flag2 && Func))					\
	TRCHKGT(_TTYHKID((Which|0x80)), Dev, Chan, Func, Line, 0);	\
    return Func;							\
}

#define Exitv() {							\
vExitLabel:								\
    if (Flag1)								\
	TRCHKGT(_TTYHKID((Which|0x80)), Dev, Chan, 0, Line, 0);	\
    return;								\
}

#define Data(xxx, a, b, c) \
    (Flag1 ? TRCHKGT(_TTYHKID((Which|0x40)), Dev, Chan, a, b, c), 0 : 0)

#define ttputc(ch, p) \
    ((!((p)->c_cl) || (CLSIZE == (p)->c_cl->c_last)) ? \
        putc(ch, p) : \
        ((p)->c_cl->c_data[(p)->c_cl->c_last++] = ch, \
        (p)->c_cc++, \
        0) \
    )

#define ttgetc(p) \
    ((p)->c_cc > 0 ? \
    ((p)->c_cf->c_first + 1 == (p)->c_cf->c_last ? \
        getc(p) : \
        ((p)->c_cc--, (p)->c_cf->c_data[(p)->c_cf->c_first++])) \
    : -1)

#define ttgetcb(bp, p) \
{ \
    if (bp = (p)->c_cf)  { \
        (p)->c_cc -= bp->c_last - bp->c_first; \
        if (!((p)->c_cf = bp->c_next)) \
            (p)->c_cl = 0; \
        bp->c_next = 0; \
    } \
}

#define ttputcb(bp, p) \
{ \
    if (!(p)->c_cl) \
        (p)->c_cf = bp; \
    else \
        (p)->c_cl->c_next = bp; \
    p->c_cl = bp; \
    bp->c_next = 0; \
    p->c_cc += bp->c_last - bp->c_first; \
}

#endif _KERNEL

#endif /* _H_TTY */
