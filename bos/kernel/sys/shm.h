/* @(#)13	1.20  src/bos/kernel/sys/shm.h, sysipc, bos411, 9428A410j 3/22/93 11:38:40 */

/*
 * COMPONENT_NAME: (SYSIPC) IPC Shared Memory Facility
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_SHM
#define _H_SHM

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_IPC
#include <sys/ipc.h>
#endif

#ifdef _ALL_SOURCE
#include <sys/seg.h>
#endif /* _ALL_SOURCE */

#ifdef _XOPEN_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>
#endif

typedef unsigned short	shmatt_t;

/*
 *	Implementation Constants.
 */

#define SHMLBA  (0x10000000) /* segment low boundary address multiple */
			     /* (SHMLBA must be a power of 2) */

/*
 *	Operation Flags.
 */

#define	SHM_RDONLY	010000	/* attach read-only (else read-write) */
#define	SHM_RND		020000	/* round attach address to SHMLBA */

/*
 *	Structure Definitions.
 */

/*
 *      There is a shared mem id data structure for each shared memory
 *      and mapped file segment in the system.
 */

struct shmid_ds {
	struct ipc_perm	shm_perm;	/* operation permission struct */
	int		shm_segsz;	/* segment size */
	pid_t		shm_lpid;	/* pid of last shmop */
	pid_t		shm_cpid;	/* pid of creator */
	shmatt_t	shm_nattch;	/* current # attached */
#ifdef _ALL_SOURCE
	shmatt_t	shm_cnattch;	/* in memory # attached */
#else
	shmatt_t	__shm_cnattch;	/* in memory # attached */
#endif
	time_t		shm_atime;	/* last shmat time */
	time_t		shm_dtime;	/* last shmdt time */
	time_t		shm_ctime;	/* last change time */
#ifdef _ALL_SOURCE
	vmhandle_t      shm_handle;     /* segment identifier */
#else
	/* vmhandle_t is typedef'd to ulong_t in R2/inc/sys/m_types.h.
	   can't use vmhandle_t here because m_types.h in types.h
	   must be in _ALL_SOURCE to prevent name space pollution.    */

	ulong_t      __shm_handle;     /* segment identifier */
#endif
};

#ifdef _NO_PROTO
extern int shmget();
extern void *shmat();
extern int shmdt();
extern int shmctl();
#else
extern int shmget(key_t, int, int);
extern void *shmat(int, const void *, int);
extern int shmdt(const void *);
extern int shmctl(int, int, struct shmid_ds *);
#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#define SHM_MAP         004000  /* map a file instead of share a segment */
#define SHM_FMAP        002000  /* fast file map			 */
#define SHM_COPY	040000	/* deferred update. Should use O_DEFER	 */
#define ZERO_MEM	1	/* for disclaim				 */

#define SHM_CLEAR	0	/* this is going away			*/


#define SHMHISEG        (12)
#define SHMLOSEG        (3)
#define NSHMSEGS        (SHMHISEG-SHMLOSEG+1)

/*
 *	ipc_perm Mode Definitions.
 */
#define	SHM_R		IPC_R	/* read permission */
#define	SHM_W		IPC_W	/* write permission */
#define	SHM_DEST	02000	/* destroy segment when # attached = 0 */

#define SHMID_SIZE ((unsigned)(sizeof(struct shmid_ds) - \
		   (unsigned)(sizeof(vmhandle_t))))


struct	shminfo {
	int	shmmax,	/* max shared memory segment size */
		shmmin,	/* min shared memory segment size */
		shmmni;	/* # of shared memory identifiers */
};


#endif /* _ALL_SOURCE */
#endif /* _H_SHM */
