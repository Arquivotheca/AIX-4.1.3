/* @(#)58	1.6  src/bos/kernel/sys/fstypes.h, syspfs, bos411, 9428A410j 12/9/92 08:13:20 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: fstypes header
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FTYP
#define _H_FTYP

/*
 *
 *      This file defines legal file system type codes, and includes macros
 *      that calculate blocksize-dependent file system parameters as a
 *      function of the type.
 */

#include <sys/types.h>

/*
 *  File system type codes:
 *      These codes reside in the last byte f the file system type code
 *      (i.e, "s_type[3]").
 */

#define FsVar   0               /* Variable (or unknown) blocksize         */
#define Fs1b    1               /* 512-byte blocksize file system          */
#define Fs2b    2               /* 1024-byte blocksize                     */
#define Fs4b    3               /* 2048-byte blocksize                     */
#define Fs8b    4               /* 4096-byte blocksize                     */
#define Fs16b   5               /* 8192-byte blocksize                     */
#define Fs32b   6               /* 16384-byte blocksize                    */
#define Fs64b   7               /* 32768-byte blocksize                    */

/*
 *  File System Parameter Table Definitions:
 *
 *      The File System Parameter table provides blocksize-dependent
 *      file system information as a function of file system type.  The
 *      structure definition describes a single table entry, and the
 *      macros that follow can be used to extract data from the table.
 */

struct _FSpmap          /* Format of a file system parm table entry     */
{   ushort FS_typecd;   /* ... file system type code                    */
    ushort FS_blksize;  /* ... file system block size                   */
    ushort FS_bmask;    /* ... block offset mask                        */
    ushort FS_nmask;    /* ... mask to locate ptrs in indirect blocks   */
    ushort FS_inopb;    /* ... number of I-nodes per block              */
    ushort FS_nindir;   /* ... number of daddr_t's per indirect block   */
    ushort FS_inomask;  /* ... mask to get I-node within a block        */
    ushort FS_bshift;   /* ... log2(blocksize)                          */
    ushort FS_ishift;   /* ... log2(I-nodes per block)                  */
    ushort FS_nshift;   /* ... log2(daddr_t's per block)                */
    ushort FS_chnksize; /* ... blocksize/256                            */
};

#if !defined(lint) && defined(_FSpgen)

/*
 *  The file system parameter table.  If this file is included in a module
 *      that has the "_FSpgen" symbol defined, we compile the file system
 *      parameter table as a static variable.
 */

#define LOG2(x) ( ((x) <   2) ? 0 : (          \
		  ((x) <   4) ? 1 : (          \
		  ((x) <   8) ? 2 : (          \
		  ((x) <  16) ? 3 : (          \
		  ((x) <  32) ? 4 : (          \
		  ((x) <  64) ? 5 : (          \
		  ((x) < 128) ? 6 : (          \
		  ((x) < 256) ? 7 : (          \
		  ((x) < 512) ? 8 : 9))))))))  \
		)

#define BSmin   LOG2(UBSIZE)
#define ISmin   LOG2(UBSIZE/sizeof(struct dinode))
#define NSmin   LOG2(UBSIZE/sizeof(daddr_t))

#define FSPARM(t,b)  { (t),                             /* TYPE CODE    */ \
		       (b),                             /* BLKSIZE      */ \
		       ((b)-1),                         /* BMASK        */ \
		       (((b)/sizeof(daddr_t))-1),       /* NMASK        */ \
		       ((b)/sizeof(struct dinode)),     /* INOPB        */ \
		       ((b)/sizeof(daddr_t)),           /* NINDIR       */ \
		       (((b)/sizeof(struct dinode))-1), /* INOMASK      */ \
		       (BSmin + ((t)-1)),               /* BSHIFT       */ \
		       (ISmin + ((t)-1)),               /* ISHIFT       */ \
		       (NSmin + ((t)-1)),               /* NSHIFT       */ \
		       ((b)/256)                        /* CHNKSIZE     */ \
		     }


struct _FSpmap _FSparms[] = { FSPARM(Fs1b,   UBSIZE),
			      FSPARM(Fs2b,  (UBSIZE * 2)),
			      FSPARM(Fs4b,  (UBSIZE * 4)),
			      FSPARM(Fs8b,  (UBSIZE * 8)),
			      FSPARM(Fs16b, (UBSIZE * 16)),
			      FSPARM(Fs32b, (UBSIZE * 32)),
			      FSPARM(Fs64b, (UBSIZE * 64))
			    };

#else
extern struct _FSpmap _FSparms[];
#endif

/*
 *  Macros to extract parameters from FSparm table.  "t" is a file system
 *      type code (e.g, "s_type" field from superblock).
 */

#ifdef  BMASK
#undef  BMASK
#undef  NMASK
#undef  INOPB
#undef  NINDIR
#undef  INOMASK
#undef  BSHIFT
#undef  NSHIFT
#endif

#define BLKSIZE(t)      _FSparms[(t)-1].FS_blksize
#define BMASK(t)        _FSparms[(t)-1].FS_bmask
#define NMASK(t)        _FSparms[(t)-1].FS_nmask
#define INOPB(t)        _FSparms[(t)-1].FS_inopb
#define NINDIR(t)       _FSparms[(t)-1].FS_nindir
#define INOMASK(t)      _FSparms[(t)-1].FS_inomask
#define BSHIFT(t)       _FSparms[(t)-1].FS_bshift
#define ISHIFT(t)       _FSparms[(t)-1].FS_ishift
#define NSHIFT(t)       _FSparms[(t)-1].FS_nshift
#define CHNKSIZE(t)     _FSparms[(t)-1].FS_chnksize

/*
 *  Macros to convert I-node numbers to block numbers and visa versa.
 */

#ifdef  itod
#undef  itod
#undef  itoo
#endif

#define itod(x,t) (daddr_t)((((unsigned)(x)-1)>>ISHIFT(t))+2)
#define itoo(x,t) (int)(((x)-1)&INOMASK(t))

#endif /* _H_FTYP */
