/* @(#)00       1.3  src/bos/kernel/include/pse/str_ioctl.h, sysxpse, bos411, 9428A410j 10/28/93 11:39:29 */
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * ORIGINS: 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _STR_IOCTL_H
#define	_STR_IOCTL_H

/* ioctl command values */
#define	STR_IO(t,v)	(('t' << 8)|v)

#define	OLD_INREAD	STR_IO(t,1)	/* get the number of bytes in 1st msg */
#define	OLD_IPUSH	STR_IO(t,2)	/* push module just below stream head */
#define	OLD_IPOP	STR_IO(t,3)	/* pop module below stream head */
#define	OLD_ILOOK	STR_IO(t,4)	/* get name of first stream module */
#define	OLD_IFLUSH	STR_IO(t,5)	/* flush all input and/or output q's */
#define	OLD_ISRDOPT	STR_IO(t,6)	/* set the read mode */
#define	OLD_IGRDOPT	STR_IO(t,7)	/* get the current read mode */
#define	OLD_ISTR	STR_IO(t,8)	/* create an internal ioctl message */
#define	OLD_ISETSIG	STR_IO(t,9)	/* request SIGPOLL signal on events */
#define	OLD_IGETSIG	STR_IO(t,10)	/* query the registered events */
#define	OLD_IFIND	STR_IO(t,11)	/* check for module in stream */
#define	OLD_ILINK	STR_IO(t,12)	/* connect stream under mux fd */
#define	OLD_IUNLINK	STR_IO(t,13)	/* disconnect two streams */
#define	OLD_IPEEK	STR_IO(t,15)	/* peek at data on read queue */
#define	OLD_IFDINSERT	STR_IO(t,16)	/* create a msg and send downstream */
#define	OLD_ISENDFD	STR_IO(t,17)	/* send fd to a connected pipe stream */
#define	OLD_IRECVFD	STR_IO(t,18)	/* retrieve a file descriptor */
#define	OLD_IFLUSHBAND	STR_IO(t,19)	/* flush an input and/or output band */
#define	OLD_ISWROPT	STR_IO(t,20)	/* set the write mode */
#define	OLD_IGWROPT	STR_IO(t,21)	/* get the current write mode */
#define	OLD_ILIST	STR_IO(t,22)	/* get list of all modules on stream */
#define	OLD_IATMARK	STR_IO(t,23)	/* is next message "marked" */
#define	OLD_ICKBAND	STR_IO(t,24)	/* check for a message on a band */
#define	OLD_IGETBAND	STR_IO(t,25)	/* get band of next msg to be read */
#define	OLD_ICANPUT	STR_IO(t,26)	/* can message be passed on a stream */
#define	OLD_ISETCLTIME	STR_IO(t,27)	/* set the close timeout wait */
#define	OLD_IGETCLTIME	STR_IO(t,28)	/* get the current close timeout wait */
#define	OLD_IPLINK	STR_IO(t,29)	/* connect a permanent link */
#define	OLD_IPUNLINK	STR_IO(t,30)	/* disconnect a permanent link */

/* ioctl values needed on non-SYS V systems */
/* XXX functionality unused - remove from system */
#define	OLD_IGETMSG	STR_IO(t,40)	/* getmsg() system call */
#define	OLD_IPUTMSG	STR_IO(t,41)	/* putmsg() system call */
#define	OLD_IPOLL	STR_IO(t,42)	/* poll() system call */
#define	OLD_ISETDELAY	STR_IO(t,43)	/* set blocking status */
#define	OLD_IGETDELAY	STR_IO(t,44)	/* get blocking status */
#define	OLD_IRUN_QUEUES	STR_IO(t,45)	/* sacrifice for the greater good */
#define	OLD_IGETPMSG	STR_IO(t,46)	/* getpmsg() system call */
#define	OLD_IPUTPMSG	STR_IO(t,47)	/* putpmsg() system call */
#define	OLD_IAUTOPUSH	STR_IO(t,48)	/* for systems that cannot do the autopush in open */
#define	OLD_IPIPE	STR_IO(t,49)	/* pipe() system call - not implemented */

#define       IOCPARM_LEN(x)  (((x) >> 16) & IOCPARM_MASK)
#define       IOCGROUP(x)     (((x) >> 8) & 0xff)

#endif /* _STR_IOCTL_H */
