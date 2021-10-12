/* @(#)91	1.8  src/bos/usr/include/jfs/quota.h, syspfs, bos41J, 9520B_all 5/19/95 09:09:25 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: quota.h
 *
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_QUOTA
#define _H_JFS_QUOTA

#include <sys/types.h>
#include <sys/param.h>
#include <sys/vmuser.h>

/*
 * Definitions for disk quotas imposed on the average user
 * (big brother finally hits UNIX).
 *
 * The following constants define the amount of time given a user
 * before the soft limits are treated as hard limits (usually resulting
 * in an allocation failure). The timer is started when the user crosses
 * their soft limit, it is reset when they go below their soft limit.
 */
#define	MAX_IQ_TIME	(7*24*60*60)	/* 1 week */
#define	MAX_DQ_TIME	(7*24*60*60)	/* 1 week */

/*
 * The following constants define the usage of the quota file array
 * in the jfsmount structure and dquot array in the inode structure.
 * The semantics of the elements of these arrays are defined in the
 * routine getinoquota; the remainder of the quota code treats them
 * generically and need not be inspected when changing the size of
 * the array.
 */
#define	MAXQUOTAS	2
#define	USRQUOTA	0	/* element used for user quotas */
#define	GRPQUOTA	1	/* element used for group quotas */

/*  Range of valid disk quota ids is MINDQID - MAXDQID
 */
#define	MINDQID		0	
#define	MAXDQID		((MAXFSIZE / sizeof (struct dqblk)) - 1)

/*
 * Definitions for the default names of the quotas files.
 */
#define INITQFNAMES { \
        "user",         /* USRQUOTA */ \
        "group",        /* GRPQUOTA */ \
        "undefined", \
};


#ifndef _KERNEL
/* Declarations for libc quota file stuff.
 */
extern char *qfname;
extern char *qfextension[];
extern char *quotagroup;
#endif /* _KERNEL */

/*
 * Command definitions for the 'quotactl' system call.
 * The commands are broken into a main command defined below
 * and a subcommand that is used to convey the type of
 * quota that is being manipulated (see above).
 */

#define SUBCMDMASK	0x00ff
#define SUBCMDSHIFT	8
#define	QCMD(cmd, type)	(((cmd) << SUBCMDSHIFT) | ((type) & SUBCMDMASK))

#define	Q_QUOTAON	0x0100	/* enable quotas */
#define	Q_QUOTAOFF	0x0200	/* disable quotas */
#define	Q_GETQUOTA	0x0300	/* get limits and usage */
#define	Q_SETQUOTA	0x0400	/* set limits and usage */
#define	Q_SETUSE	0x0500	/* set usage */
#define	Q_SYNC		0x0600	/* sync disk copy of a filesystems quotas */

/*
 * The following structure defines the format of the disk quota file
 * (as it appears on disk) - the file is an array of these structures
 * indexed by user or group number.  The quotactl system call establishes
 * the vnode for each quota file (a pointer is retained in the jfsmount
 * structure).
 */

struct	dqblk {
	u_long	dqb_bhardlimit;	/* absolute limit on disk blks alloc */
	u_long	dqb_bsoftlimit;	/* preferred limit on disk blks */
	u_long	dqb_curblocks;	/* current block count */
	u_long	dqb_ihardlimit;	/* maximum # allocated inodes + 1 */
	u_long	dqb_isoftlimit;	/* preferred inode limit */
	u_long	dqb_curinodes;	/* current # allocated inodes */
	time_t	dqb_btime;	/* time limit for excessive disk use */
	time_t	dqb_itime;	/* time limit for excessive files */
};

/* disk quota data block size.
 */
#define	DQBSIZE		1024

/* miscellanous defines for dealing with partial blocks.
 */
#define DQCBIT		0x80000000
#define DQCARRY(c)	((uint)(c) >> 31)
#define DQBTIME(t)	((uint)(t) & (~DQCBIT))
#define DQSBTIME(c,t)	(((uint)(c) << 31) | ((uint)(t) & (~DQCBIT)))

#ifdef _KERNEL

#include <jfs/jfsmount.h>
#include <jfs/fsparam.h>

/*
 * The following structure records disk usage for a user or group on a
 * filesystem. There is one allocated for each quota that exists on any
 * filesystem for the current user or group. A cache is kept of recently
 * used entries.
 */

struct	dquot {
	struct	dquot *dq_forw;		/* hash chain forward */
	struct	dquot *dq_back;		/* hash chain backword */
	struct	dquot *dq_next;		/* next on cache list */
	struct	dquot *dq_prev;		/* previous on cache list */
	int	dq_lock;		/* VMM bit lock */
	short	dq_flags;		/* flags, see below */
	short	dq_type;		/* quota type of this dquot */
	ulong	dq_cnt;			/* count of active references */
	uid_t	dq_id;			/* identifier this applies to */
	dev_t	dq_dev;			/* filesystem that this is taken from*/
	struct 	jfsmount *dq_jmp;	/* mount structure pointer */
	struct	dqblk dq_dqb;		/* actual usage & quotas */
	int	dq_event;		/* event list for quota activity */
};

/*
 * dq_lock bits used by v_dqlock() and v_dqunlock()
 */
#define	DQ_LOCK		0x01		/* this quota locked (no MODS) */
#define	DQ_WANT		0x02		/* wakeup on unlock */

/*
 * dq_flag values
 */
#define	DQ_MOD		0x04		/* this quota modified since read */
#define	DQ_FAKE		0x08		/* no limits here, just usage */
#define	DQ_BLKS		0x10		/* has been warned about blk limit */
#define	DQ_INODS	0x20		/* has been warned about inode limit */
#define	DQ_QFLOCK	0x40		/* this quota is transforming */
#define	DQ_QFWANT	0x80		/* wakeup on transform complete */
#define	DQ_IOERROR	0x100		/* error reading this quota */

/*
 * Shorthand notation.
 */
#define	dq_bhardlimit	dq_dqb.dqb_bhardlimit
#define	dq_bsoftlimit	dq_dqb.dqb_bsoftlimit
#define	dq_curblocks	dq_dqb.dqb_curblocks
#define	dq_ihardlimit	dq_dqb.dqb_ihardlimit
#define	dq_isoftlimit	dq_dqb.dqb_isoftlimit
#define	dq_curinodes	dq_dqb.dqb_curinodes
#define	dq_btime	dq_dqb.dqb_btime
#define	dq_itime	dq_dqb.dqb_itime

/*
 * If the system has never checked for a quota for this file,
 * then it is set to NODQUOT. Once a write attempt is made
 * the inode pointer is set to reference a dquot structure.
 */
#define	NODQUOT		((struct dquot *) 0)

#define	JFSTODQ(BLKS)	((BLKS) * (BSIZE/DQBSIZE))

/*
 * Hash anchors for dquot table.
 */
struct hdquot {	
	struct dquot *dqh_forw;
	struct dquot *dqh_back;
};

/* 
 * Hash table size and marco for hash function.
 */
# define	NHDQUOT		PAGESIZE/sizeof (struct hdquot)
# define	DQHASH(DEV,ID) \
		(&hdquot[(((int)((DEV))) + ((int)((ID)))) & (NHDQUOT-1)])

/* valid disk quota id.
 */
# define	DQVALID(ID) \
		(((int)((ID))) >= MINDQID && ((int)((ID))) <= MAXDQID ? 1 : 0)

#endif /* _KERNEL */

#endif /* _H_JFS_QUOTA */
