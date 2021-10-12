/* @(#)97	1.19  src/bos/kernel/sys/msg.h, sysipc, bos411, 9428A410j 1/12/93 18:22:52 */

/*
 * COMPONENT_NAME: (SYSIPC) IPC Message Facility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_MSG
#define _H_MSG

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_IPC
#include <sys/ipc.h>
#endif

#ifdef _XOPEN_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>
#endif

typedef unsigned short	msgqnum_t;
typedef unsigned short	msglen_t;

/*
 *	Message Operation Flags.
 */

#define	MSG_NOERROR	010000	/* no error if big message */

/*
 *	Structure Definitions
 */

/*
 *	There is one msg queue id data structure for each q in the system.
 */

struct msqid_ds {
	struct ipc_perm	msg_perm;	/* operation permission struct */
#ifdef _ALL_SOURCE
	struct msg	*msg_first;	/* ptr to first message on q */
	struct msg	*msg_last;	/* ptr to last message on q */
	unsigned short	msg_cbytes;	/* current # bytes on q */
#else
	void 		*__msg_first;	/* ptr to first message on q */
	void 		*__msg_last;	/* ptr to last message on q */
	unsigned short	__msg_cbytes;	/* current # bytes on q */
#endif
	msgqnum_t	msg_qnum;	/* # of messages on q */
	msglen_t	msg_qbytes;	/* max # of bytes on q */
	pid_t		msg_lspid;	/* pid of last msgsnd */
	pid_t		msg_lrpid;	/* pid of last msgrcv */
	time_t		msg_stime;	/* last msgsnd time */
	time_t		msg_rtime;	/* last msgrcv time */
	time_t		msg_ctime;	/* last change time */
#ifdef _ALL_SOURCE
	/* event list of messages for this queue */
	int		msg_rwait;       /* wait list for message receive */
	int		msg_wwait;       /* wait list for message send */
	/* The following members are in support of msgselect() */
	unsigned short	msg_reqevents;   /* select/poll requested events */
#else
	/* event list of messages for this queue */
	int		__msg_rwait;     /* wait list for message receive */
	int		__msg_wwait;     /* wait list for message send */
	/* The following members are in support of msgselect() */
	unsigned short	__msg_reqevents; /* select/poll requested events */
#endif
};

#ifdef _NO_PROTO
extern int msgctl();
extern int msgget();
extern int msgrcv();
extern int msgsnd();
#else
extern int msgget(key_t, int);
extern int msgrcv(int, void *, size_t, long, int);
extern int msgsnd(int, const void *, size_t, int);
extern int msgctl(int, int, struct msqid_ds *);
#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

#ifdef	_ALL_SOURCE
/*
 *	ipc_perm Mode Definitions.
 */
#define	MSG_R		IPC_R	/* read permission */
#define	MSG_W		IPC_W	/* write permission */
#define	MSG_RWAIT	01000	/* a reader is waiting for a message */
#define	MSG_WWAIT	02000	/* a writer is waiting to send */

/*
 *	Message header information
 */

#define	MSGX	time_t	mtime;		/* time message was sent */	\
		uid_t	muid;		/* author's effective uid */	\
		gid_t	mgid;		/* author's effective gid */	\
		pid_t	mpid;		/* author's process id */	\

struct msg_hdr	{ 
		 MSGX
		 mtyp_t	mtype;		/* message type */
};

/*
 *	There is one msg structure for each message that may be in the system.
 */

struct msg {
	struct msg     *msg_next;	/* ptr to next message on q */
	struct msg_hdr 	msg_attr;	/* message attributes */
	unsigned short	msg_ts; 	/* message text size */
	char		*msg_spot;	/* pointer to message text */
};


/*
 *	User message buffer template for msgsnd and msgrcv system calls.
 */

struct msgbuf {
	mtyp_t	mtype;		/* message type */
	char	mtext[1];	/* message text */
};

/*
 *	User message buffer template for msgxrcv system call.
 */

struct msgxbuf {
		MSGX
		mtyp_t	mtype;	  /* Message type */
		char	mtext[1]; /* Message text */
};

/* We need to know the length of everything but mtext[1] and padding. */
#define MSGXBUFSIZE (int)(((struct msgxbuf *)NULL)->mtext)



/* rmsgsnd() and rmsgrcv() "flags" parameter definitions */
#define MSG_SYSSPACE	0x01
#define XMSG 		0x02

/*
 *	Message information structure.
 */

struct msginfo {
	int	msgmax,	/* max message size			*/
		msgmnb,	/* max # bytes on queue 		*/
		msgmni,	/* # of message queue identifiers	*/
		msgmnm;	/* max # messages per queue identifier	*/
};

/*
 * Number of bytes to copy for IPC_STAT command
 */
#define MSG_STAT  (int)&(((struct msqid_ds *)NULL)->msg_rwait)

extern int msgxrcv(int, struct msgxbuf*, int, mtyp_t, int);

#endif	/* _ALL_SOURCE */

#endif	/* _H_MSG */
