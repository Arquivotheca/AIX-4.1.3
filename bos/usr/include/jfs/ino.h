/* @(#)44	1.6.1.10  src/bos/usr/include/jfs/ino.h, syspfs, bos41J, 145887.bos 3/3/95 09:42:34 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System 
 *
 * FUNCTIONS: ino.h
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

#ifndef	_H_JFS_INO
#define	_H_JFS_INO

#include <sys/types.h>
#include <sys/time.h>
#include <jfs/fsdefs.h>
#include <jfs/filsys.h>
#include <jfs/jfsmount.h>
#include <sys/priv.h>
#include <sys/acl.h>
#include <sys/lock_def.h>

struct dinode
{
        /* generation number */
        ulong   di_gen;

        /* the mode_t returned by stat() */
        /* format, attributes and permission bits */
        mode_t  di_mode;

        /* number of links to file (if 0, inode is available) */
        ushort  di_nlink;

        /* accounting ID */
        ushort  di_acct;

        /* user id of owner */
        uid_t   di_uid;

        /* group id of owner */
        gid_t   di_gid;

        /* size of file */
        off_t   di_size;

        /* number of blocks actually used by file */
        ulong   di_nblocks;

        /* time last modified */
        struct timestruc_t  di_mtime_ts;

        /* time last accessed */
	struct timestruc_t  di_atime_ts;

        /* time last changed inode */
	struct timestruc_t  di_ctime_ts;

	/* defines for old time_t names */
#define	di_mtime	di_mtime_ts.tv_sec
#define	di_atime	di_atime_ts.tv_sec
#define	di_ctime	di_ctime_ts.tv_sec

        /* extended access control information */
        long	di_acl;         /* acl pointer */
#		define	ACL_INCORE	(1 << 31)
        ulong	di_sec;         /* reserved */

        /* spares */
        ulong   di_rsrvd[5];

        /***** file type-dependent information *****/
        /*
         * size of private data in disk inode is D_PRIVATE.
         * location and size of fields depend on object type.
         */
#       define  D_PRIVATE       48

        union   di_info
        {
                /* all types must fit within d_private */
                char    d_private[D_PRIVATE];

                /* regular file or directory. */
                struct regdir
                {
                        /* disk pointers. NDADDR must <= 8 to fit */
                        ulong   _di_rdaddr[NDADDR]; /* disk addrs */
                        ulong   _di_vindirect; /* page number in .indirect */
                        ulong   _di_rindirect; /* disk addr indirect block */
                        union
                        {
                                /* privilege vector - only for non-directory */
                                struct
                                {
                                        ulong   _di_offset;
                                        ulong   _di_flags;
#                                       define  PCL_ENABLED    (1<<31)
#                                       define  PCL_EXTENDED   (1<<30)
#                                       define  PCL_FLAGS \
                                                (PCL_ENABLED|PCL_EXTENDED)
                                } _di_privinfo;
                                priv_t  _di_priv;
                                /* ACL templates - only for directory */
                                struct
                                {
                                        ulong   _di_aclf;
                                        ulong   _di_acld;
                                } _di_aclinfo;
                        } _di_sec;
                } _di_file;
/* offsets of regular file or directory private data. */
#       define  di_rdaddr       _di_info._di_file._di_rdaddr
#       define  di_vindirect    _di_info._di_file._di_vindirect
#       define  di_rindirect    _di_info._di_file._di_rindirect
#       define  di_privinfo     _di_info._di_file._di_sec._di_privinfo
#       define  di_privoffset   di_privinfo._di_offset
#       define  di_privflags    di_privinfo._di_flags
#       define  di_priv         _di_info._di_file._di_sec._di_priv
#       define  di_aclf         _di_info._di_file._di_sec._di_aclinfo._di_aclf
#       define  di_acld         _di_info._di_file._di_sec._di_aclinfo._di_acld

                /* special file (device) */
                struct
                {
                        dev_t   _di_rdev;       /* device major and minor */
                } _di_dev;
/* offsets of special file private data */
#       define  di_rdev         _di_info._di_dev._di_rdev

                /*
                 * symbolic link. link is stored in inode if its
                 * length is less than D_PRIVATE. Otherwise like
                 * a regular file.
                 */
                union
                {
                        char    _s_private[D_PRIVATE];
                        struct  regdir  _s_symfile;
                } _di_sym;
/* offset of symbolic link private data */
#       define  di_symlink      _di_info._di_sym._s_private

                /*
                 * data for mounted filesystem. kept in inode = 0
                 * and dev = devt of mounted filesystem in inode table.
                 */
                struct mountnode
                {
                        struct  inode   *_iplog;        /* itab of log */
                        struct  inode   *_ipinode;      /* itab of .inodes */
                        struct  inode   *_ipind;        /* itab of .indirect */
                        struct  inode   *_ipinomap;     /* itab of inode map */
                        struct  inode   *_ipdmap;       /* itab of disk map */
                        struct  inode   *_ipsuper;      /* itab of super blk */
                        struct  inode   *_ipinodex;     /* itab of .inodex */
                        struct  jfsmount   *_jmpmnt;    /* ptr to mount data */
			ushort		_fperpage;	/* frags per block */
			ushort		_agsize;	/* frags per ag */
			ushort		_iagsize;	/* inodes per ag */
			ushort		_fscompress;	/* compress */
                } _mt_info;
/* offsets of MOUNT data */
#       define  di_iplog        _di_info._mt_info._iplog
#       define  di_ipinode      _di_info._mt_info._ipinode
#       define  di_ipind        _di_info._mt_info._ipind
#       define  di_ipinomap     _di_info._mt_info._ipinomap
#       define  di_ipdmap       _di_info._mt_info._ipdmap
#       define  di_ipsuper      _di_info._mt_info._ipsuper
#       define  di_ipinodex     _di_info._mt_info._ipinodex
#       define  di_jmpmnt       _di_info._mt_info._jmpmnt
#       define  di_fperpage	_di_info._mt_info._fperpage
#       define  di_agsize	_di_info._mt_info._agsize
#       define  di_iagsize	_di_info._mt_info._iagsize
#       define  di_fscompress	_di_info._mt_info._fscompress

                /*
                 * log info. kept in inode = 0 and dev = devt of
                 * log device filesystem in inode table.
                 */
                struct lognode
                {
                        int     _logptr;        /* page number end of log     */
                        int     _logsize;       /* log size in pages          */
                        int     _logend;        /* eor in page _logptr        */
                        int     _logsync;       /* addr in syncpt record      */
                        int     _nextsync;      /* bytes to next logsyncpt    */
			int	_logxor;	/* cumulative checksum for    */
						/*   all log data in the page */
			int 	_llogeor;	/* end of record for last log */
						/*   record in the page       */
			int	_llogxor;	/* checksum up to and includ- */
						/*   ing last log record      */
			struct ilogx *_logx;	/* log inode extension        */
			struct gnode *_logdgp;  /* pointer to device gnode    */
			Simple_lock   _loglock;	/* lock word for log          */
                } _di_log;
/* offsets of LOG data */
#       define di_logptr        _di_info._di_log._logptr
#       define di_logsize       _di_info._di_log._logsize
#       define di_logend        _di_info._di_log._logend
#       define di_logsync       _di_info._di_log._logsync
#       define di_nextsync      _di_info._di_log._nextsync
#       define di_logdgp        _di_info._di_log._logdgp
#       define di_loglock       _di_info._di_log._loglock
#	define di_logxor	_di_info._di_log._logxor	
#	define di_llogeor	_di_info._di_log._llogeor
#	define di_llogxor	_di_info._di_log._llogxor
#	define di_logx		_di_info._di_log._logx
        } _di_info;
};


/* doubly indirect block has (PAGESIZE/8) entries of form idblock */
struct idblock {
	ulong  id_vaddr;  /* page number in .indirect */
	ulong  id_raddr;  /* disk block address in volume */
};

/* layout of a .indirect segment for 4k pagesize. the indirect
 * blocks of .indirect are in root and indptr. the number of pages
 * in the two is FIRSTIND so FIRSTIND is the first page in .indirect
 * that can be allocated for other uses. the free list of pages
 * is initialized to consist of the pages from FIRSTIND to the last
 * one covered by the first indirect block, i.e. PAGESIZE/4 - 1.
 * the variable free points to the index of a free page and the 
 * free pages are chained thru the corresponding indptr (when 
 * a page is not on the free list indptr contains the disk address)
 * more is the the index of the next page of free indptrs. when
 * the free - list is exhausted the page of indptrs pointed to
 * by more is added to the list.
 */ 

/* while a file is open the pointers to disk blocks in .indirect may
 * have the format of a VMM external page table entry : besides 
 * containing a disk block number, the leftmost bits of the word 
 * may be used for flags. however these flag bits never appear on the 
 * permanent disk copy of an indirect block.
 */


#ifdef _KERNEL
#define  FIRSTIND	65
struct indir {

	struct  idblock root[64];		/* double indirect */
	int	free;				/* index of first free page */
	int	cnt;				/* used for compare/swap */
	int     more;				/* index more free pages */
	int	freebacked;			/* free page disk backed */
	int	pad[1024 - 2*64 - 4]; 		/* pad to page boundary */
	ulong   indptr[SEGSIZE/PAGESIZE];	/* single indirect */  
	union { 
		struct idblock root[PAGESIZE/8]; /* double indirects */
		ulong  ptr[PAGESIZE/4];		 /* single indirects */
	} page[SEGSIZE/PAGESIZE - FIRSTIND];     /* pages to end segment */
};

/* masks for treating disk address fields in disk inodes and
 * indirect blocks as VMM external page table entries.
 */
#define NEWBIT	0x80000000
#define CDADDR  0x0fffffff

/* macros for determining type of indirect blocks required to cover size
 */
#define NOIND(size)     ( ((size) <= NDADDR*PAGESIZE ) ? 1 : 0 )
#define SGLIND(size)    ( ((size) <= (PAGESIZE/4)*PAGESIZE) ? 1 : 0 )
#define DBLIND(size)    ( ((size) >  (PAGESIZE/4)*PAGESIZE) ? 1 : 0 )

/* convert size in bytes and fragment per page to number of fragments
 * in last block of a file.
 */
#define BTOFR(b,f)	(((b) > NDADDR * PAGESIZE) ? (f) : \
	((((b) - 1) & (PAGESIZE - 1)) + (PAGESIZE/(f))) / (PAGESIZE/(f)))

#endif /* _KERNEL */

#endif	/* _H_JFS_INO */
