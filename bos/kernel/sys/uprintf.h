/* @(#)11	1.2  src/bos/kernel/sys/uprintf.h, sysproc, bos411, 9428A410j 5/7/91 17:21:32 */
/*
 * COMPONENT_NAME: SYSPROC
 *
 * FUNCTIONS: uprintf.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_UPRINTF
#define _H_UPRINTF

#define	UP_MAXARGS	8	/* maximum number of print arg */
#define	UP_MAXCAT	255	/* maximum catalog name length */
#define	UP_MAXSTR	1024	/* max default format or print arg strlen */
#define	UP_MAXMSG	4096	/* maximum message length */
 
/* the uprintf structure describes the message to
 * be delivered.
 */
struct uprintf
{
	char	*upf_defmsg;	/* pointer to default format string */
	void 	*upf_args[UP_MAXARGS];	/* print args - value or string ptr */
	char	*upf_NLcatname;	/* pointer to NLS catalog name */ 
	int	upf_NLsetno;	/* message set number within catalog */
	int	upf_NLmsgno;	/* message number within catalog */
};

/* upfbuf described queued messages.
 */
struct upfbuf
{
	struct	upfbuf *up_next; /* next upfbuf on message or free list */
	pid_t	up_pid;		/* target process id */
	dev_t	up_ttyd;	/* devid of proccess's controlling tty */
	chan_t	up_ttyc;	/* chan number of proccess's controlling tty */
	int	up_nargs;	/* number of print arguments */
	ushort	up_fmtvec;	/* bit vector describing the format string */
	ushort	up_flags;	/* message type flags */
	uint	up_seq;		/* arrival sequence number for this upfbuf */
	struct  uprintf up_uprintf; /* uprintf struct describing the message */
};

/* upfdata describes the uprintf data returned by the
 * upfget() system call.
 */
struct upfdata
{
	char	upd_NLcatname[UP_MAXCAT+1];	/* NLS catalog name */
	char	upd_defmsg[UP_MAXSTR+1];	/* default format string */
	char	upd_prtargs[UP_MAXARGS * (UP_MAXSTR+1)]; /* print args */
};

/* upfbuf flags.
 */
#define	UP_NLS		0x01	/* NLS required for this entry */
#define	UP_NONLS	0x02	/* NLS not required */
#define	UP_INTR 	0x04	/* pinned upfbuf */

/* shorthand notation.
 */
#define	up_NLsetno	up_uprintf.upf_NLsetno
#define	up_NLmsgno	up_uprintf.upf_NLmsgno
#define	up_args		up_uprintf.upf_args
#define	up_NLcatname	up_uprintf.upf_NLcatname
#define	up_defmsg	up_uprintf.upf_defmsg

/* macro used in manipulating and examining format vector.
 */
#define UPF_VBIT(argno)			(1 << (argno))
#define UPF_ISSTR(vec,argno)		((vec) & (1 << (argno)))

#ifdef _KERNEL

#define	UP_NBUFS	64	/* number of pinned upfbufs */

#endif /* _KERNEL */

#endif /* _H_UPRINTF */
