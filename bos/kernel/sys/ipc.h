/* @(#)17	1.12  src/bos/kernel/sys/ipc.h, sysipc, bos411, 9428A410j 11/17/93 17:14:42 */

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

#ifndef _H_IPC
#define _H_IPC

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifdef _XOPEN_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/* Common IPC Structures */
struct ipc_perm {
	uid_t		uid;		/* owner's user id	*/
	gid_t		gid;		/* owner's group id	*/
	uid_t		cuid;		/* creator's user id	*/
	gid_t		cgid;		/* creator's group id	*/
	mode_t		mode;		/* access modes		*/
	unsigned short	seq;		/* slot usage sequence number */
	key_t		key;		/* key			*/
};

/* common IPC operation flag definitions */
#define	IPC_CREAT	0001000		/* create entry if key doesn't exist */
#define	IPC_EXCL	0002000		/* fail if key exists */
#define	IPC_NOWAIT	0004000		/* error if request must wait */

/* Keys. */
#define	IPC_PRIVATE	(key_t)0	/* private key */

/* Control Commands. */
#define	IPC_RMID	0	/* remove identifier */
#define	IPC_SET		1	/* set options */
#define	IPC_STAT	2	/* get options */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifdef _NO_PROTO
key_t	ftok();
#else /* _NO_PROTO */
key_t	ftok(const char *, int);
#endif /* _NO_PROTO */

/* Common ipc_perm mode Definitions. */
#define	IPC_ALLOC	0100000		/* entry currently allocated        */
#define	IPC_R		0000400		/* read or receive permission       */
#define	IPC_W		0000200		/* write or send permission	    */

/* common IPC operation flag definitions */
#define IPC_NOERROR	0010000		/* truncates a message if too long */

/*
 * control commands
 */
#define SHM_SIZE	6       /* change segment size (shared mem only)*/

#endif /* _ALL_SOURCE */
#endif /* _H_IPC */
