/* @(#)45	1.8.1.15  src/bos/usr/include/jfs/inode.h, syspfs, bos41J, 145887.bos 3/3/95 09:44:09 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System 
 *
 * FUNCTIONS: inode.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_INODE
#define _H_JFS_INODE

#include <sys/types.h>
#include <sys/param.h>
#include <jfs/ino.h>
#include <sys/vnode.h>          
#include <sys/mode.h>
#include <jfs/quota.h>

struct	inode {
	struct inode	*i_forw;	/* hash chain forw */
	struct inode	*i_back;	/* hash chain back */
	struct inode	*i_next;        /* next on cached list */
	struct inode	*i_prev;        /* previous on cached list */
	struct gnode	i_gnode;	/* generic node information */
	ino_t		i_number;       /* disk inode number */
	dev_t		i_dev ;         /* device on which inode resides */
	struct inode	*i_ipmnt;	/* inode of mount table entry */
	short		i_flag;
	short		i_locks;	/* inode transition flags */
	short		i_compress;	/* data compression */
	short		i_cflag;        /* commit flags */
	long		i_count;	/* reference count */
	int		i_event;	/* event list for inode activity */
	struct movedfrag *i_movedfrag;	/* ptr to list of movedfrags */
	int		i_openevent;	/* event list for nshare open modes */
#ifdef _I_MULT_RDRS
	Simple_lock	i_nodelock;	/* inode lock */
	Complex_lock	i_rdwrlock;	/* read/write lock */
#else
	int		reserved[2];	/* preserve size of inode struct */
	Simple_lock	i_rdwrlock;	/* read/write lock */
#endif
	struct dquot	*i_dquot[MAXQUOTAS]; /* disk quota structures */
	struct dinode   i_dinode;
	int		i_cluster;	/* last write cluster for wrbehind */
};

/* i_locks */
#define	IXLOCK	   0x0001	/* inode is in transition */
#define	IXWANT	   0x0002	/* some process waiting on lock */
#define	IQUOTING   0x0004	/* binding to a quota */
#define	ILOCKALLOC 0x0010	/* inode lock allocated */
#define	GLOCKALLOC 0x0020	/* gnode lock allocated */
#define	VLOCKALLOC 0x0040	/* vnode lock allocated */

/* i_flag */
#define	IACC	0x0008		/* inode access time to be updated */
#define	ICHG	0x0010		/* inode has been changed */
#define	IUPD	0x0020		/* file has been modified */
#define	IFSYNC	0x0040		/* commit changes to data as well as inode */
#define IDEFER  0x0100		/* defered update */

/* i_cflag */
#define DIRTY		0x0001    /* dirty journalled file */
#define CMNEW		0x0002    /* never committed inode   */
#define CMNOLINK	0x0004    /* inode committed with zero link count */
#define ICLOSE		0x0010    /* inode being recycled */
#define IACTIVITY	0x0020	  /* inode in a file system being unmounted */

/* extended mode bits (i_mode), high order short. */
#define IFJOURNAL 	0200000		/* journalled file */


#define	i_seg        i_gnode.gn_seg 
#define	i_gen        i_dinode.di_gen
#define i_nlink      i_dinode.di_nlink
#define i_mode       i_dinode.di_mode
#define i_uid        i_dinode.di_uid
#define i_gid        i_dinode.di_gid
#define i_size       i_dinode.di_size
#define i_disize     i_dinode.di_size
#define i_nblocks    i_dinode.di_nblocks
#define i_mtime      i_dinode.di_mtime
#define i_mtime_ts   i_dinode.di_mtime_ts
#define i_atime      i_dinode.di_atime
#define i_atime_ts   i_dinode.di_atime_ts
#define i_ctime      i_dinode.di_ctime
#define i_ctime_ts   i_dinode.di_ctime_ts
#define i_nacl       i_dinode.di_nacl
#define i_nsec       i_dinode.di_nsec
#define i_acl        i_dinode.di_acl
#define i_security   i_dinode.di_security
#define i_seclab     i_dinode.di_seclab
#define i_priv       i_dinode.di_priv
#define i_pflags     i_dinode.di_pflags
#define i_privoffset i_dinode.di_privoffset
#define i_privflags  i_dinode.di_privflags
#define i_rdaddr     i_dinode.di_rdaddr
#define i_rindirect  i_dinode.di_rindirect
#define i_vindirect  i_dinode.di_vindirect
#define i_rdev       i_dinode.di_rdev
#define i_max        i_dinode.di_max
#define i_min	     i_dinode.di_min
#define ibn_lastr    i_dinode.di_bnlastr	
#define i_pino	     i_dinode.di_pino
#define i_dgp	     i_dinode.di_dgp
#define i_symlink    i_dinode.di_symlink
#define i_symdaddr   i_dinode.di_symaddr
#define i_logptr     i_dinode.di_logptr
#define i_logsize    i_dinode.di_logsize
#define i_logend     i_dinode.di_logend
#define i_logsync    i_dinode.di_logsync
#define i_nextsync   i_dinode.di_nextsync
#define i_logxor     i_dinode.di_logxor
#define i_llogeor    i_dinode.di_llogeor
#define i_llogxor    i_dinode.di_llogxor
#define i_logx	     i_dinode.di_logx
#define i_loglock    i_dinode.di_loglock
#define i_logdgp     i_dinode.di_logdgp
#define i_iplog      i_dinode.di_iplog
#define i_ipind      i_dinode.di_ipind
#define i_ipinode    i_dinode.di_ipinode
#define i_ipinomap   i_dinode.di_ipinomap
#define i_ipdmap     i_dinode.di_ipdmap
#define i_ipsuper    i_dinode.di_ipsuper
#define	i_ipinodex   i_dinode.di_ipinodex
#define	i_jmpmnt     i_dinode.di_jmpmnt
#define	i_fperpage   i_dinode.di_fperpage
#define	i_agsize     i_dinode.di_agsize
#define	i_iagsize    i_dinode.di_iagsize
#define	i_fscompress i_dinode.di_fscompress
#define ifn_buf      i_dinode.di_ifn_buf 		
#define ifn_poll     i_dinode.di_ifn_poll 
#define ifn_size     i_dinode.di_ifn_size
#define ifn_wcnt     i_dinode.di_ifn_wcnt
#define ifn_rcnt     i_dinode.di_ifn_rcnt
#define ifn_wptr     i_dinode.di_ifn_wptr
#define ifn_rptr     i_dinode.di_ifn_rptr
#define ifn_flag     i_dinode.di_ifn_flag

/* Some useful macros
 */
#define GTOIP(x)	((struct inode *)(((struct gnode *)(x))->gn_data))
#define VTOIP(x)	(GTOIP(VTOGP(x)))
#define ITOGP(x)	((struct gnode *)(&(((struct inode *)(x))->i_gnode)))
#define VTOGFS(x)	(((struct vnode *)(x))->v_vfsp->vfs_gfs)
#define ISVDEV(t) (((t) == VBLK) || ((t) == VCHR)  \
			|| ((t) == VFIFO) || ((t) == VMPC))

/* hash anchors for inode table
 */
struct hinode {
	struct	inode *hi_forw;
	struct	inode *hi_back;
	int	hi_timestamp;
	int	hi_vget;
};

extern int nhino;
extern struct hinode *hinode;

#define IHASH(ino,dev,hip) 			  \
{						  \
        uint hash; 				  \
        hash = (uint)(ino) ^ (uint)(dev); 	  \
        hash = (hash >> 8) + (hash >> 12) + hash; \
        (hip) = &hinode[hash & (uint)(nhino-1)];  \
}

#endif /* _H_JFS_INODE */
