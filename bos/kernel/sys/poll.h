/* @(#)11	1.12  src/bos/kernel/sys/poll.h, sysios, bos411, 9431A411a 8/4/94 15:28:43 */
#ifndef _H_POLL
#define _H_POLL
/*
 * COMPONENT_NAME: (SYSIOS) Poll system call header file
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <standards.h>
#include <sys/types.h>

/*
 * Structure to be used (for file descriptors or file pointers)
 * with the POLL system call.
 */
struct	pollfd
{
	long	fd;			/* file descriptor or file ptr	*/
	ushort	reqevents;		/* requested events		*/
	ushort	rtnevents;		/* returned events		*/
#define	events	reqevents		/* SVR3,4 pollfd member name	*/
#define	revents	rtnevents		/* SVR3,4 pollfd member name	*/
};

/*
 * Timeout values - used for poll() system call
 */
#ifndef NO_TIMEOUT
#define NO_TIMEOUT      0       /* don't wait for a response            */
#endif

#ifndef INF_TIMEOUT
#define INF_TIMEOUT     -1      /* wait until a response is received    */
#endif

#ifndef _NO_PROTO

#ifdef _XOPEN_EXTENDED_SRC
int	poll(struct pollfd *, ulong, int);
#else
int	poll(void *listptr, ulong nfds, long timeout);
#endif
/* arguments:
 *	void 		*listptr;	ptr to pollfd/pollmsg/pollist structs
 *	ulong		nfds;		#elements in fds array
 *	long 		timeout;	max time to wait for select criteria
 */
 
int	fp_poll(void *listptr, ulong nfds, long timeout, uint flags);
/* arguments:
 *	void		*listptr;	ptr to pollfd/pollmsg/pollist structs
 *	ulong		nfds;		#elements in fds array
 *	long 		timeout;	max time to wait for select criteria
 *	register uint	flags;		does data contain fd's? or fp's?
 */
 
void	selnotify();
/* arguments:
 *	int	sel_id;			device that reqevents occurred on
 *	int	unique_id;		entity that reqevents occurred on
 *	ushort	rtnevents;		returned events
 */
 
#else

int	poll();
/* arguments:
 *	void 		*listptr;	ptr to pollfd/pollmsg/pollist structs
 *	ulong		nfds;		#elements in fds array
 *	long 		timeout;	max time to wait for select criteria
 */
 
int	fp_poll();
/* arguments:
 *	void		*listptr;	ptr to pollfd/pollmsg/pollist structs
 *	ulong		nfds;		#elements in fds array
 *	long 		timeout;	max time to wait for select criteria
 *	register uint	flags;		does data contain fd's? or fp's?
 */
 
void	selnotify();
/* arguments:
 *	int	sel_id;			device that reqevents occurred on
 *	int	unique_id;		entity that reqevents occurred on
 *	ushort	rtnevents;		returned events
 */
#endif /* not _NO_PROTO */

/*
 * Structure to be used (for message queue ids)
 * with the POLL system call.
 */
struct	pollmsg
{
	long	msgid;			/* message queue id		*/
	ushort	reqevents;		/* requested events		*/
	ushort	rtnevents;		/* returned events		*/
};

/*
 * Macro used to define a pollist structure
 * for the POLL system call.
 */
#define	POLLIST(F,M)			\
struct					\
{					\
	struct pollfd fdlist[F];	\
	struct pollmsg msglist[M];	\
}

/*
 * Flags passed to fp_poll.
 */
#define	POLL_FDMSG	0x0001	/* input contains file descriptors	*/
#define	POLL_NESTED	0x0002	/* select/poll call is nested		*/

/*
 * Requested and returned event flags for the poll system call.
 */
#define	POLLIN		0x0001		/* ready for reading		*/
#define	POLLOUT		0x0002		/* ready for writing		*/
#define	POLLPRI		0x0004		/* has an exceptional condition	*/
#define	POLLWRNORM	POLLOUT		/* same difference ...		*/
#define	POLLRDNORM	0x0010		/* non-priority message ready	*/
#define	POLLRDBAND	0x0020		/* priority message ready to read*/
#define	POLLWRBAND	0x0040		/* writable priority band exists*/
#define	POLLMSG		0x0080		/* signal message ready to read	*/
#define	POLLSYNC	0x8000		/* process requests synchronously*/

/*
 * Device type flags for sel_type parameter on selnotify service.
 * Device drivers use their dev_t as their sel_type identifier.
 */
#define	POLL_FIFO	-1		/* selnotify is for a FIFO	*/
#define	POLL_SOCKET	-2		/* selnotify is for a SOCKET	*/
#define	POLL_MSG	-3		/* selnotify is for a MSG QUEUE	*/

/*
 * Error returned event flags for the poll system call.
 */
#define	POLLNVAL	POLLSYNC	/* invalid data			*/
#define	POLLERR		0x4000		/* error occurred		*/
#define	POLLHUP		0x2000		/* ???				*/

#ifdef _ALL_SOURCE

#define POLLNORM POLLIN

#endif

#endif	/* _H_POLL */
