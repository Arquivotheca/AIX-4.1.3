/* @(#)17	1.10  src/bos/usr/include/IN/FSio.h, libIN, bos411, 9428A410j 6/16/90 00:17:08 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *      This file contains definitions and declarations associated with
 *      file system level I/O operations.  File system utilities such as mkfs
 *      and fsck use the file system I/O routines to access file systems
 *      without the aid of the kernel.
 */

#ifndef _H_FSIO
#define _H_FSIO

#ifndef NADDR
#include <sys/inode.h>
#endif

/*
 *  FS_INODE structure
 *      The FS_INODE structure is FSio's version of the kernel's in-core
 *      i-node.  The FSi2mem()/FSmem2i() routines can be used to convert
 *      disk i-nodes to this format (and vice versa)
 */

typedef struct FSinode              /* Format of a FS_INODE struct ...  */
{
	short      mi_maj;          /* ... file sys major device number */
	ushort     mi_min;          /* ... file sys minor device number */
	ino_t      mi_number;       /* ... i-node number                */
	ushort     mi_flag;         /* ... i-node status flags          */
	short      mi_count;        /* ... open/reference count         */

	ushort     mi_mode;         /* ... file mode flags              */
	short      mi_nlink;        /* ... file's link count            */
	ushort     mi_uid;          /* ... owner's user ID              */
	ushort     mi_gid;          /* ... owner's group ID             */
	off_t      mi_size;         /* ... file size                    */

	union                       /* ... Address overlay area         */
	{  ushort  mi_odev[2];      /* ... ... Major/minor device nums  */
	   daddr_t mi_oadd[NADDR];  /* ... ... Disk addresses           */
	}          mi_ovly;
}  FS_INODE;

#define mi_rmaj  mi_ovly.mi_odev[0] /* Major device number!             */
#define mi_rmin  mi_ovly.mi_odev[1] /* Minor device number!             */
#define mi_dadd  mi_ovly.mi_oadd    /* Disk address!                    */


/*
 *  FS_FILE structure
 *      The FS_FILE structure is FSio's equivalent of the FILE structure
 *      used by the standard I/O package.  The FSopen routine returns
 *      a pointer to such a structure, and all of the other file system
 *      I/O routines want to see an FS_FILE address as their first parameter.
 */

typedef struct FSfile   /* Format of an FS_FILE structure               */
{
    struct FSfile  *FSF_next;   /* ... Next open FS_FILE struct         */
    struct FSfile  *FSF_prev;   /* ... Previous open FS_FILE struct     */
    struct FSmtab  *FSF_mtp;    /* ... Ptr to mount structure           */
    time_t          FSF_atime;  /* ... Time of last access              */
    time_t          FSF_mtime;  /* ... Time of last modification        */
    time_t          FSF_ctime;  /* ... Time of last I-node update       */
    FS_INODE        FSF_inode;  /* ... In-core I-node                   */
} FS_FILE;

#define FS_FMAX 8       /* Max concurrently open FS_FILE structs        */

#ifndef ITIMES          /* Update times from FSF_FILE struct            */
#define ITIMES  IMOUNT
#endif
#ifndef IRONLY          /* Read-only file                               */
#define IRONLY  ITEXT
#endif

/*
 *  File system mount structure
 *      Structures of the following type are passed to FSmount.  These
 *      structures define the currently active file systems on which
 *      pseudo-files can be processed.
 */

typedef struct FSmtab
{   struct FSmtab    *FSM_next;     /* Ptr to next active entry         */
	   int      (*FSM_biop)();  /* Ptr to block I/O routine         */
	   daddr_t  (*FSM_allo)();  /* Ptr to 2ndary allocation routine */
	   char      *FSM_user;     /* Available to user pgms           */
	   int        FSM_ocnt;     /* Number of open files             */
	   int        FSM_flag;     /* Flag word (see below)            */

    union                           /* Superblock buffer ...            */
    {   char   FSMuchr[BSIZE];      /* ... Fill out to a full block     */
	struct filsys FSMustr;
    }                 FSM_u;
} FSmnt_t;

#define FSM_sblk FSM_u.FSMustr      /* #define away the union           */

/*
 *      Bit definitions for flag word ...
 */

#define FM_FDIRTY 1                 /* Superblock dirty when set        */
#define FM_FRONLY 2                 /* Mounted read-only if set         */

/*
 *  Buffer Cache Entries:
 *      The following structure maps the pseudo-file buffer cache entries
 *      that control internal buffers used to hold file indirect blocks.
 */

typedef struct FSbufr               /*  Format of a cache buffer ...    */
{   struct FSbufr  *FSB_next;       /*  ... next buffer on LRU chain    */
    struct FSbufr  *FSB_prev;       /*  ... prev buffer on LRU chain    */
    struct FSmtab  *FSB_mtp;        /*  ... ptr to FSmnt_t  structure   */
    daddr_t         FSB_bnum;       /*  ... block number                */
    int             FSB_flag;       /*  ... Header flags                */
    char            FSB_buff[BSIZE];/*  ... the buffer proper           */
} FS_BUFF;

/*
 *  Definition of flag bits:
 */

#define FB_FDIRTY 1     /*  Buffer is dirty                             */
#define FS_BMAX  4      /*  Number of buffers in the cache (initial)    */

#define _FSbput(bhp) ((bhp)->FSB_flag |= FB_FDIRTY)

/*
 *  Routine declarations
 *      The following routines make up the file system I/O package
 */

extern FS_FILE *FSopen();       /* Pseudo-file open routine             */
extern daddr_t  FSbmap();       /* Pseudo-file block mapping function   */
extern daddr_t  FSnxtblk();     /* Get next block in file system        */
extern daddr_t  FSalloc();      /* Free block allocater                 */
extern int      FSfree();       /* Free block deallocater               */
extern int      FSmapb();       /* Build pseudo-file block map          */
extern int      FSread();       /* Pseudo-file read routine             */
extern int      FSwrite();      /* Pseudo-file write routine            */
extern int      FSclose();      /* Pseudo-file close routine            */
extern int      FSmount();      /* Mounts file systems                  */
extern int      FSumount();     /* Unmounts them                        */
extern void     FSi2mem();      /* Disk i-node load routine             */
extern void     FSmem2i();      /* Disk i-node store routine            */
extern void     FSbuff();       /* Adds buffers to cache                */

/*
 *  FSclose() disposition codes
 *      These codes are used to specify how FSclose() should dispose
 *      of a given pseudo-file and the associated I-node.  The first three
 *      definitions are used internally by FSclose().  The following 4
 *      definitions are used by the calling progams.
 */

#define FSC_NOFR 1      /* Don't fragment file if set                   */
#define FSC_TRNC 2      /* Truncate file if set                         */
#define FSC_DELT 4      /* Deallocate I-node if set                     */

#define FS_CSAVE 0                      /* ... Save I-node and file     */
#define FS_CNOFR FSC_NOFR               /* ... Save but don't fragment  */
#define FS_CTRNC FSC_TRNC               /* ... Truncate but save file   */
#define FS_CDELT FSC_TRNC+FSC_DELT      /* ... Delete file              */

/*
 *  Block I/O op codes
 *      These op-code definitions are used by the block I/O subroutines
 *      that are used to perform the actual I/O.  The user provides the
 *      address of the block I/O routine when he/she calls FSopen().
 */

#define FS_BREAD 0      /* Block I/O read operation                     */
#define FS_BWRIT 1      /* Block I/O write operation                    */

/*
 *  _FSsync() operation codes
 *      These op-codes specify the operations to be performed by the
 *      _FSsync() routine:
 */

#define FS_SUPDT  1     /* Update buffers and superblocks               */
#define FS_SREFR  2     /* Refresh buffers and superblocks              */

/*
 *  Miscellaneous macro definitions
 *      These macros differ depending on superblock format
 */

#define IORG ((daddr_t)2)       /* I-node area origin                   */

#ifdef  _s_NEWF         /* New format superblocks ...                   */
#define SBtype(sbp)     (sbp)->s_type
#define SBbsize(sbp)    (sbp)->s_bsize
#define SBcpu(sbp)      (sbp)->s_cpu
#define SBibase(sbp)    (sbp)->s_ibase
#define NICfree(sbp)    (sbp)->s_nicfree
#define NICinod(sbp)    (sbp)->s_nicino

#else                   /* Old format superblocks ...                   */
#if BSIZE == 512
#define SBtype(sbp)     Fs1b
#endif
#if BSIZE == 1024
#define SBtype(sbp)     Fs2b
#endif
#if BSIZE == 2048
#define SBtype(sbp)     Fs4b
#endif
#if BSIZE == 4096
#define SBtype(sbp)     Fs8b
#endif

#define SBbsize(sbp)    BSIZE
#define SBcpu(sbp)      0
#define SBibase(sbp)    ((daddr_t)0)
#define NICfree(sbp)    NICFREE
#define NICinod(sbp)    NICINOD
#define FREEblk(sbp,n)  (sbp)->s_free[n]
#define FREEino(sbp,n)  (sbp)->s_inode[n]
#define s_cyl           s_n
#define s_skip          s_m
#endif

#ifndef _s_ALGN         /* Disk I/O does not require alignment          */
#define FSbio(mtp,op,buf,blk) (*((mtp)->FSM_biop))(mtp,op,buf,blk)
#endif

extern  int     FSxsblk();      /* Re-formats superblocks               */

#ifdef  _s_XCPU         /* Multi-cpu file system support ...            */
extern  void    FSxdblk();      /* Re-formats directory blocks          */
extern  void    FSxfblk();      /* Re-formats free list blocks          */
extern  void    FSxiblk();      /* Re-formats indirect blocks           */
extern  void    FSxnode();      /* Re-formats disk i-nodes              */
extern  void    FSxgblk();      /* Re-formats fragmented data blocks    */
extern  void    FSl3tol();      /* Converts 3-byte ints to 4-byte ints  */
extern  void    FSltol3();      /* Converts 4-byte ints to 3-byte ints  */
extern  short   _XXshort();     /* Swaps bytes in 16-bit integers       */
extern  long    _XXlong();      /* Swaps bytes in 32-bit integers       */

#define swapS(x,sbp) _XXshort(x,(sbp)->s_cpu)
#define swapL(x,sbp) _XXlong(x,(sbp)->s_cpu)

#else                   /* No multi-cpu support ...                     */
#define FSxdblk(buf,sbp,flag)
#define FSxfblk(buf,sbp,flag)
#define FSxiblk(buf,sbp,flag)
#define FSxnode(buf,sbp,flag)
#define FSxgblk(buf,sbp,flag)

#define FSl3tol(lp,cp,n,cpu)  l3tol(lp,cp,n)
#define FSltol3(cp,lp,n,cpu)  ltol3(cp,lp,n)
#define swapS(x,sbp)          ((short)(x))
#define swapL(x,sbp)          ((long)(x))
#endif

#ifdef  _s_FRAG         /* Number of entries in arry depends on _s_FRAG */
#define FS_NPCTS   3    /* ... use all 3 if fragment support enabled    */
#else
#define FS_NPCTS   2    /* ... don't set FTPCT if no frag support       */
#endif

#endif /* FSIO */
