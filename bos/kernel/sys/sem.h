/* @(#)02	1.14  src/bos/kernel/sys/sem.h, sysipc, bos411, 9428A410j 1/12/93 18:22:56 */

/*
 * COMPONENT_NAME: (SYSIPC) IPC Semapore Facility
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

#ifndef _H_SEM
#define _H_SEM

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_IPC
#include <sys/ipc.h>
#endif

#ifdef _XOPEN_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>		/* pid_t, time_t, key_t, size_t */
#endif

/*
 *	Semaphore Operation Flags.
 */

#define	SEM_UNDO	010000	/* set up adjust on exit entry */

/*
 *	Semctl Command Definitions.
 */

#define	GETNCNT	3	/* get semncnt */
#define	GETPID	4	/* get sempid */
#define	GETVAL	5	/* get semval */
#define	GETALL	6	/* get all semval's */
#define	GETZCNT	7	/* get semzcnt */
#define	SETVAL	8	/* set semval */
#define	SETALL	9	/* set all semval's */

/*
 *	There is one semaphore id data structure for each set of semaphores
 *		in the system.
 */

struct semid_ds {
	struct ipc_perm	sem_perm;	/* operation permission struct	*/
#ifdef _ALL_SOURCE
	struct sem	*sem_base;	/* ptr to first semaphore in set*/
#else
	struct sem	*__sem_base;	/* ptr to first semaphore in set*/
#endif
	unsigned short	sem_nsems;	/* # of semaphores in set	*/
	time_t		sem_otime;	/* last semop time		*/
	time_t		sem_ctime;	/* last change time		*/
};

/*
 *	There is one semaphore structure for each semaphore in the system.
 */

struct sem {
	unsigned short	semval;		/* semaphore text map address	*/
	pid_t		sempid;		/* pid of last operation	*/
	unsigned short	semncnt;	/* # awaiting semval > cval	*/
	unsigned short	semzcnt;	/* # awaiting semval = 0	*/
#ifdef _ALL_SOURCE
	int		semnwait;	/* wait list for seval > cval	*/
	int		semzwait;	/* wait list for semval = 0	*/
#else
	int		__semnwait;	/* wait list for seval > cval	*/
	int		__semzwait;	/* wait list for semval = 0	*/
#endif
};

/*
 *	User semaphore template for semop system calls.
 */

struct sembuf {
	unsigned short	sem_num;	/* semaphore #			*/
	short		sem_op;		/* semaphore operation		*/
	short		sem_flg;	/* operation flags		*/
};


#ifdef _NO_PROTO
extern int semctl();
extern int semget();
extern int semop();
#else
#ifndef _KERNEL
extern int semget(key_t, int, int);
extern int semop(int, struct sembuf *, size_t);
extern int semctl(int, int, int, ...);
#endif
#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

/*
 *	ipc_perm Mode Definitions.
 */
#define	SEM_A	IPC_W	/* alter permission */
#define	SEM_R	IPC_R	/* read permission */

/*
 *	Semaphore Operation Flags.
 */

#define SEM_ORDER	020000  /* perfore operations non-atomically */
#define SEM_ERR		040000  /* set if error encountered on semop */


/*
 *	There is one undo structure per process in the system.
 */

struct sem_undo {
	struct sem_undo	*un_np;	/* ptr to next active undo structure */
	short		un_cnt;	/* # of active entries */
	struct undo {
		short	un_aoe;	/* adjust on exit values */
		ushort	un_num;	/* semaphore # */
		int	un_id;	/* semid */
	}	un_ent[1];	/* undo entries (one minimum) */
};



/*
 * semaphore information structure
 */
struct	seminfo	{
	int	semmni,		/* # of semaphore identifiers */
		semmsl,		/* max # of semaphores per id */
		semopm,		/* max # of operations per semop call */
		semume,		/* max # of undo entries per process */
		semusz,		/* size in bytes of undo structure */
		semvmx,		/* semaphore maximum value */
		semaem;		/* adjust on exit max value */
};

#endif /* _ALL_SOURCE */

#endif /* _H_SEM */
