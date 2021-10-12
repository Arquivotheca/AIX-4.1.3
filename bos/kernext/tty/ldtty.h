/* @(#)54	1.19  src/bos/kernext/tty/ldtty.h, sysxldterm, bos41J, 9518A_all 5/2/95 15:14:40 */
/*
 * COMPONENT_NAME: sysxtty
 *
 * FUNCTIONS: header file for ldterm
 *
 * ORIGINS: 40, 71, 83
 *
 */
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef	_H_LDTTY
#define _H_LDTTY

#include <sys/types.h>
#include <sys/limits.h>
#include <sys/ioctl.h>		/* for struct winsize */

#include <sys/eucioctl.h>	/* for multibyte processing */
#include <sys/termiox.h>
#include <sys/termio.h>		/* for compat */
#include <sys/sysconfig.h>
#include <sys/intr.h>		/* for INTOFFL0 definition	*/
#include <sys/lockl.h>
#include <sys/stream.h>

#include <sys/str_tty.h>    /* General define for streams based tty */

/*
 * Flags on character passed to ttyinput
 */
#define	TTY_CHARMASK	0x000000ff
#define	TTY_QUOTE	0x00000100
#define	TTY_ERRORMASK	0xff000000
#define	TTY_FE		0x01000000
#define	TTY_PE		0x02000000

/*
 * Output buffer size used to break up write() requests into smaller chunks.
 * Prevents a single user from using too much output message buffers.
 */
#define	OBUFSIZ	256

/*
 * limits.
 */
#define	NSPEEDS	16
#define	TTMASK	15

/*
 * dds type definition for ldterm module to be use by configuration methods,
 * the dds is declared in the general include stream tty str_tty.h.
 */
struct	ldterm_dds {
	enum	dds_type	which_dds;	/* dds identifier	*/
};

#ifdef	_KERNEL

struct mlist {
  int cc;			/* total bytes in message */
  int chunksize;		/* size of new mblks */
  mblk_t *cf;			/* first mblk */
  mblk_t *cl;			/* last mblk */
};

#endif	/* _KERNEL */
/*
 * Per-tty structure for "ldtty" line discipline STREAMS module
 *
 * Contains only the info pertaining to things that interest the
 * line discipline.  No hardware-specific stuff here.
 */
struct ldtty {
	queue_t 		*t_queue;	/* pointer to our READ queue */
	mblk_t			*t_rawbuf;	/* input buffer */
	mblk_t 			*t_rawtail;	/* tail of input buffer */
	int			t_rawcc;	/* count in input buffer */
	mblk_t			*t_unsent;	/* ldtty_sendraw() */
						/* optimization */
	int			t_unsent_ndx;	/* ldtty_sendraw() */
						/* optimization */
	int			t_shcc;		/* size of shead copy of */
						/* rawbuf */
	mblk_t			*t_outbuf;	/* buffer for output */
						/* characters */
	mblk_t			*t_sparebuf;	/* spare for mem. tricks: no */
						/* b_cont */
#if MACH_ASSERT
	int			t_sparehit;
	int			t_sparemiss;
	int			t_rawtrace;
#endif
	unsigned 	long 	t_state;	/* line discipline state */
	int			t_intimeout_posted;	/* boolean flag for */
							/* read srvp --     */
							/* protected by     */
							/* t_intimeout_lock */
	int 			t_intimeout;	/* timeout() id for reads */
	int 			t_outbid;	/* bufcall() id for outmsg */
	int 			t_ackbid;	/* bufcall() id for ioctl acks*/
	int 			t_hupbid;	/* bufcall() id for M_HANGUP */
	char			t_col;		/* current output column */
	int			t_rocount;	/* characters echoed since */
						/* last output */
	char 			t_rocol;	/* first echo column */
	struct	termiox		t_control;	/* open and hardware control */
						/* disciplines on.	     */
	struct 	winsize 	t_winsize;	/* window size */
	struct 	termios 	t_termios;	/* termios state for flags */
						/* treated only by ldterm  */
	struct	termios		u_termios;	/* termios state for user  */
						/* program point of view   */
	struct	termios		d_termios;	/* termios state for flags */
						/* treated only by drivers */
						/* case of MC_PART_CANON.  */
#define	t_iflag		t_termios.c_iflag
#define	t_oflag		t_termios.c_oflag
#define	t_cflag		t_termios.c_cflag
#define	t_lflag		t_termios.c_lflag
#define	t_min		t_termios.c_min
#define	t_time		t_termios.c_time
#define	t_cc		t_termios.c_cc
#define t_ispeed	t_termios.c_ispeed
#define t_ospeed	t_termios.c_ospeed
	char			t_vmin;		/* defect number 128266	*/
	char			t_vtime;	/* defect number 128266	*/
	int			t_flags;	/* for compat */
	int			t_ioctl_cmd;	/* original ioctl command */
	mblk_t			*t_ioctl_data;	/* original ioctl data	*/
	mblk_t			*t_qioctl;	/* queued ioctl */
	long			t_shad_time;	/* Value of t_cc[VTIME] in */
						/* ticks */
	eucioc_t		t_cswidth;	/* character widths */
	int			t_codeset;	/* current input EUC codeset */
	int			t_eucleft;	/* bytes left for input */
						/* character */
	int			t_eucind;	/* index to eucbytes */
	uchar_t			t_eucbytes[8];	/* place to build input EUC */
						/* character */
	int			t_out_codeset;	/* current output EUC codeset */
	int			t_out_eucleft;	/* bytes left for output */
						/* character */
	int			t_out_eucind;	/* index to out_eucbytes */
	uchar_t			t_out_eucbytes[8];	/* place to build */
							/* output EUC char */
	int			t_ihog;		/* input hog limit for this */
						/* tty */
	int			t_ohog;		/* output hog limit for this */
						/* tty */
	int			t_event;	/* event list for e_sleep */
	int			t_on_close : 1;	/* ldterm during close */
	int			t_on_open : 1;	/* ldterm during open */
	int                     t_quot : 1;     /* just saw a \ */
	char			t_devname[TTNAMEMAX]; /* device name	*/
        dev_t                   t_dev;          /* major/minor number   */
};

typedef	struct	ldtty	*ldttyp_t;

/*
 * The following two values are chosen to be powers of 2 to
 * coincide with STREAMS message block sizes.  This is not absolutely
 * necessary, but may make STREAMS flow control work a little better
 */
#define LDTTYCHUNKSIZE 64	/* size of input mblks */
#define LDTTYMAX 256		/* STREAMS tty max buffer size */
/*
 * Following two values used for input buffer flow control if
 * IXOFF is set, acordingly with TXSETIHOG ioctl. (see AIX3.2)
 */
#define	LDTTYLOWAT(x)	(x->t_ihog >> 2)	       /* ldtty low watermark */
#define	LDTTYHIWAT(x)	(x->t_ihog - (x->t_ihog >> 2)) /* ldtty hi watermark */


/*
 * flag bits for ldtty_input
 */
#define T_POST_WAKEUP 0x1       /* ldtty_wakeup() required */
#define T_POST_START  0x2       /* ldtty_start() required */
#define T_POST_TIMER  0x4       /* need to start timer */
#define T_POST_BACKUP 0x8	/* rawq full -- let data backup on q */
#define T_POST_FLUSH  0x10	/* input flushed -- ignore prev. bits */


/* internal state bits */
#define TS_TIMEOUT    0x00000001 /* delay timeout in progress */
#define TS_SETNEEDED  0x00000002 /* an M_SETOPS is needed */
#define TS_ISOPEN     0x00000004 /* device is open */
#define TS_RAWBACKUP  0x00000008 /* raw input clogged temporarily */
#define TS_CARR_ON    0x00000010 /* software copy of carrier-present */
#define TS_NOCANON    0x00000020 /* no input processing */
#define TS_WAITOUTBUF 0x00000040 /* waiting for bufcall on outbuf */
#define TS_WAITOUTPUT 0x00000080 /* waiting for write side canput */
#define TS_TTSTOP     0x00000100 /* output stopped by ctl-s */
#define TS_VTIME_FLAG 0x00000200 /* first time through read() */
#define TS_TBLOCK     0x00000400 /* tandem queue blocked */
#define TS_WAITEUC    0x00000800 /* full EUC char waiting for outbuf */
#define TS_ASLEEP     0x00001000 /* waiting for output to drain */
#define TS_CLOSING    0x00002000 /* Port is being closed */
#define TS_ONDELAY    0x00008000 /* copy of no delay flag from open */
#define TS_BKSL       0x00010000 /* state for lowercase \ work */
#define TS_MBENABLED  0x00020000 /* multibyte EUC enabled */
#define TS_MINSAT     0x00040000 /* VMIN satisfied; no read yet */
#define TS_ERASE      0x00040000 /* within a \.../ for PRTRUB */
#define TS_LNCH       0x00080000 /* next character is literal */
#define TS_TYPEN      0x00100000 /* retyping suspended input (PENDIN) */
#define TS_CNTTB      0x00200000 /* counting tab width, leave FLUSHO alone */
#define TS_DSUSP      0x00400000 /* Delayed suspend. */
#define TS_INTIMEOUT  0x02000000 /* A input timeout is active. */

#define	TS_LOCAL	(TS_BKSL|TS_ERASE|TS_LNCH|TS_TYPEN|TS_CNTTB)


/* define partab character types */
#define ORDINARY	0
#define CONTROL		1
#define BACKSPACE	2
#define NEWLINE		3
#define TAB		4
#define VTAB		5
#define RETURN		6
#define FF		7

#define ldtty_iocack_msg(iocp, count, mp) do  { 			\
	(mp)->b_datap->db_type = M_IOCACK; 				\
	iocp->ioc_error = 0;						\
	if (((iocp)->ioc_count = (count)) == 0 && (mp)->b_cont) {	\
		freemsg((mp)->b_cont);					\
		(mp)->b_cont = 0;					\
	}								\
} while (0)

#define ldtty_breakc(c, lflag) 						\
	((c) == '\n' || CCEQ(cc[VEOF], (c)) || 				\
	 CCEQ(cc[VEOL], (c)) || 					\
	 (CCEQ(cc[VEOL2], (c)) && ((lflag) & IEXTEN)))

#define	ldtty_msgdsize(mp)	((mp) ? msgdsize(mp) : 0)

#define	ldtty_newrawbuf(tp, mp) do {					\
	(tp)->t_rawbuf = (tp)->t_rawtail = (tp)->t_unsent = (mp); 	\
	(tp)->t_unsent_ndx = ((tp)->t_rawtail->b_rptr -			\
			(tp)->t_rawtail->b_datap->db_base); 		\
} while (0)

#define ldtty_mbenabled(tp) 						\
	((tp)->t_state & TS_MBENABLED)

#define ldtty_bufclr(tp) do  { 						\
    (tp)->t_unsent = (tp)->t_rawbuf = (tp)->t_rawtail = NULL; 		\
    (tp)->t_unsent_ndx = (tp)->t_rawcc = 0; 				\
} while (0)

/*
 * defect number 148029, verify the b_cont to avoid the call to
 * unlinkb.
 */
#define	ldtty_bufreset(tp) do {						\
	mblk_t	*mp;							\
	assert((tp)->t_rawbuf);						\
	if (((tp)->t_rawbuf->b_cont) && (mp = unlinkb((tp)->t_rawbuf)))	\
		freemsg(mp);						\
	(tp)->t_rawbuf->b_rptr = (tp)->t_rawbuf->b_wptr = 		\
		(tp)->t_rawbuf->b_datap->db_base;			\
	(tp)->t_rawcc = 0;						\
	(tp)->t_unsent = (tp)->t_rawtail = (tp)->t_rawbuf; 		\
	(tp)->t_unsent_ndx = 0;						\
} while (0)

#define	ldtty_iocnak_msg(iocp, error, mp) do {				\
	(mp)->b_datap->db_type = M_IOCNAK;				\
	(iocp)->ioc_error = (error);					\
	if ((mp)->b_cont) {						\
		freemsg((mp)->b_cont);					\
		(mp)->b_cont = 0;					\
	}								\
} while (0)


/*
 * TOSTOP changed or ICANON went from on to off.
 */
#define	ldtty_need_setopts(tp1, tp2)					\
	(((tp1)->c_lflag & TOSTOP) != ((tp2)->c_lflag & TOSTOP) ||      \
	 (((tp1)->c_lflag & ICANON) != ((tp2)->c_lflag & ICANON)))

#ifdef _KERNEL
/*
 * Macros for raw input timeouts.
 * defect number 148029, just replaced locks by atomic operations in macros.
 */

#define	ldtty_set_intimeout(tp, tmo) do {				\
	ldtty_unset_intimeout(tp);					\
	(tp)->t_state |= TS_INTIMEOUT;					\
	(tp)->t_intimeout = pse_timeout(ldtty_intimeout,		\
					(timeout_arg_t)(tp), tmo); 	\
} while (0)

#define	ldtty_unset_intimeout(tp) do {					\
	(tp)->t_state &= ~TS_INTIMEOUT;					\
	if ((tp)->t_intimeout) {					\
		pse_untimeout((tp)->t_intimeout);			\
		(tp)->t_intimeout = 0;					\
	}								\
	fetch_and_and(&((tp)->t_intimeout_posted), 0);			\
} while (0)

#define	ldtty_post_intimeout(tp) do {					\
	fetch_and_or(&((tp)->t_intimeout_posted), 1);			\
} while (0)

#define	ldtty_check_intimeout(tp, retp) do {				\
	int	swapval = 1;						\
	if (tp->t_state & TS_INTIMEOUT) {				\
		if (compare_and_swap(&((tp)->t_intimeout_posted),&swapval,0)) {\
			*(retp) = 1;					\
			(tp)->t_intimeout = 0;				\
			(tp)->t_state &= ~TS_INTIMEOUT;			\
		}							\
	}								\
} while (0)

void ldtty_intimeout(timeout_arg_t);
#endif  /* _KERNEL */

#endif	/* _H_LDTTY */
