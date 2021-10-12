/* @(#)15	1.27  src/bos/kernel/sys/flock.h, syslfs, bos411, 9438C411a 9/22/94 16:47:15 */
#ifndef _H_FLOCK
#define _H_FLOCK

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <standards.h>
#include <sys/types.h>

/* POSIX does not define flock.h, however, the flock struct is required to
 * be included within fcntl.h when _POSIX_SOURCE is defined.  Therefore,
 * it is confined within POSIX ifdefs.
 */
#ifdef _POSIX_SOURCE
 
struct	flock	{
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;		/* len = 0 means until end of file */
	unsigned long	l_sysid;
#ifdef	_NONSTD_TYPES
	ushort	l_pid_ext;
	ushort	l_pid;
#else
	pid_t	l_pid;
#endif	/* _NONSTD_TYPES */
	int	l_vfs;
};
/* file segment locking set data type - information passed to system by user */

/* file segment locking types */
#define	F_RDLCK	01	/* Read lock */
#define	F_WRLCK	02	/* Write lock */
#define	F_UNLCK	03	/* Remove lock(s) */

#endif /* _POSIX_SOURCE */
 
#ifdef _ALL_SOURCE



#ifdef _KERNEL

/* internal version of flock structure */

struct	eflock	{
	short		l_type;
	short		l_whence;
	unsigned long	l_sysid;
	pid_t		l_pid;
	int		l_vfs;
#ifdef	_LONG_LONG
	offset_t	l_start;
	offset_t	l_len;
#else
	int		__l_start_msw;
	off_t		l_start;
	int		__l_len_msw;
	off_t		l_len;
#endif
};

#ifdef	_LONG_LONG

/*
 * Function prototype for common_reclock()
 * This prototype allows the compiler to automatically promote the
 * size and offset parameters to their appropriate sizes without
 * having to use a type cast at the call site.
 */

int
common_reclock(
	struct gnode *	gp,
	offset_t	size,
	offset_t	offset,
	struct eflock *	lckdat,
	int		cmd,
	int (*		retry_fcn)(),
	ulong *		retry_id,
	int (*		lock_fcn)(),
	int (*		rele_fcn)());
#endif

#define INOFLCK		1	/* Inode is locked when reclock() is called. */
#define SETFLCK		2	/* Set a file lock. */
#define SLPFLCK		4	/* Wait if blocked. */

/*
** lock status
**	used to coordinate with sleeping locks
** WASBLOCK is only used for debugging
*/

#define LCK_UNBLOCK	0
#define	LCK_BLOCKER	1
#define	LCK_BLOCKED	2
#define	LCK_WASBLOCK	4

/* file locking structure (connected to gnode) */

#define l_end 		l_len
#define MAXEND  	017777777777

struct	filock	{
	struct	eflock	set;	/* contains type, start, and end (len) */
	short		state;	/* current state, see defines above */

	/* information about the blocking lock, if any */
	short		vfs;
	unsigned long	sysid;
	pid_t		pid;
	struct filock *	filockp;
	int (*		retry_fcn)();

	struct filock *	prev;
	struct filock *	next;
	int		event;	/* event list anchor */
};

/* file and record locking configuration structure */
/* record use total may overflow */
struct flckinfo {
	long recs;	/* number of records configured on system */
	long reccnt;	/* number of records currently in use */
	long recovf;	/* number of times system ran out of record locks. */
	long rectot;	/* number of records used since system boot */
};

#define RMTLOCK 1

#define ENF_LOCK(mode)	(((mode) & (ISGID | IEXEC | (IEXEC >> 3) | (IEXEC >> 6))) == ISGID)

extern struct flckinfo	flckinfo;
extern struct filock	flox[];
#endif /* _KERNEL */
#endif /* _ALL_SOURCE */
#endif /* _H_FLOCK */
