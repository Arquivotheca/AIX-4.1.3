/* @(#)55	1.52.1.7  src/bos/kernel/sys/types.h, incstd, bos411, 9428A410j 3/4/94 10:04:44 */
/*
 *   COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 3, 26, 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *   Copyright (c) 1982, 1986 Regents of the University of California.
 *   All rights reserved.  The Berkeley software License Agreement
 *   specifies the terms and conditions for redistribution.
 *
 *      (#)types.h     7.1 (Berkeley) 6/4/86
 */

#ifndef _H_TYPES
#define _H_TYPES

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *
 *      The ANSI and POSIX standards require that certain values be in types.h.
 *      It also requires that if _ANSI_C_SOURCE or _POSIX_SOURCE is defined
 *	then ONLY those values are present. This header includes all the ANSI
 *	and POSIX required entries.
 *
 *      Other entries are included in _ALL_SOURCE.
 *
 */

#ifdef   _ANSI_C_SOURCE
/*
 * ANSI C required typedefs
 */

#ifndef	_PTRDIFF_T
#define _PTRDIFF_T
typedef int		ptrdiff_t;
#endif

#ifndef	_WCHAR_T
#define _WCHAR_T
typedef unsigned short	wchar_t;
#endif

#ifndef _WCTYPE_T
#define _WCTYPE_T
typedef unsigned int	wctype_t;
#endif

#ifndef	_FPOS_T
#define _FPOS_T
typedef long		fpos_t;
#endif

#ifndef	_TIME_T
#define _TIME_T
typedef	long		time_t;
#endif

#ifndef	_CLOCK_T
#define _CLOCK_T
typedef int		clock_t;
#endif

#ifndef	_SIZE_T
#define _SIZE_T
typedef	unsigned long	size_t;
#endif

#endif   /* _ANSI_C_SOURCE */

#ifdef   _POSIX_SOURCE
/*
 * shorthand type definitions for unsigned storage classes
 */
typedef	unsigned char	uchar_t;
typedef	unsigned short	ushort_t;
typedef	unsigned int	uint_t;
typedef unsigned long	ulong_t;
typedef signed int	ssize_t;

/*
 * standard AIX type definitions
 */
typedef long		level_t;
typedef	long		daddr_t;	/* disk address */
typedef	char *		caddr_t;	/* "core" (i.e. memory) address */
typedef	ulong_t		ino_t;		/* inode number (filesystem) */
typedef short		cnt_t;
typedef ulong_t		dev_t;		/* device number (major+minor) */
typedef	long		chan_t;		/* channel number (minor's minor) */
typedef	long		off_t;		/* file offset */
typedef	long		paddr_t;
typedef	long		key_t;
typedef long		timer_t;	/* timer id */
typedef	short		nlink_t;
typedef	ulong_t		mode_t;		/* file mode */
typedef ulong_t		uid_t;		/* user ID */
typedef ulong_t		gid_t;		/* group ID */
typedef	void *		mid_t;		/* module ID	*/
typedef	int		pid_t;		/* process ID */
typedef int             tid_t;          /* thread ID */
typedef char		slab_t[12];	/* security label */
typedef long            mtyp_t;		/* ipc message type */
typedef int             boolean_t;

#ifndef _WINT_T
#define _WINT_T
	typedef	int		wint_t;		/* Wide character */
#endif

#ifdef _ALL_SOURCE
typedef ulong_t		id_t;		/* General ID */
#endif /* _ALL_SOURCE */

/* typedef and structure for signal mask */
/* This must correspond to the "struct sigset" structure in _ALL_SOURCE below */
typedef struct sigset_t	{
#ifdef _ALL_SOURCE
	unsigned long losigs;
	unsigned long hisigs;
#else
	unsigned long __losigs;
	unsigned long __hisigs;
#endif
} sigset_t;

typedef int signal_t;

#endif /* _POSIX_SOURCE */

#ifdef _ALL_SOURCE

#ifndef _H_M_TYPES
#include <sys/m_types.h>		/* machine-dependent type definitions */
#endif

#ifndef	_VA_LIST
#define _VA_LIST
typedef char *		va_list;
#endif

#ifndef NULL
#define	NULL	0
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/*
 * type definition for Unicode character code.
 */
typedef ushort_t	UniChar;

/*
 * shorthand type definitions for unsigned storage classes
 */
typedef	uchar_t		uchar;
typedef	ushort_t	ushort;
typedef	uint_t		uint;
typedef ulong_t		ulong;

typedef	struct { int r[1]; } *	physadr_t;
typedef	physadr_t	physadr;

/* typedefs for BSD unsigned things */
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

typedef	struct	_quad { long val[2]; } quad;
typedef	long	swblk_t;

#define	NBBY	8		/* number of bits in a byte */

/* The sigset structure must correspond to sigset_t in _POSIX_SOURCE above */
struct sigset	{
	unsigned long losigs;
	unsigned long hisigs;
};

/* typedef for the File System Identifier (fsid) */
struct fsid {
	long	val[2];
};
typedef struct fsid fsid_t;
#define	fsid_dev	val[0]
#define	fsid_type	val[1]

/* typedef for the File Identifier (fid) */
#define FHSIZE		32
#define MAXFIDSZ	(FHSIZE - sizeof(fsid_t) - sizeof(uint_t))

struct fid {
	uint_t	fid_len;
	char	fid_data[MAXFIDSZ];
};
typedef struct fid fid_t;

struct fileid {			/* this is for servers only! */
	uint_t	fid_len;
	ino_t	fid_ino;
	uint_t	fid_gen;
	char	fid_x[MAXFIDSZ - (sizeof(ino_t) + 2) - sizeof(uint_t)];
};

/* typedef for the File Handle (fhandle) */
struct fhandle {
	char x[FHSIZE];		/* allows structure assignments */
};
struct filehandle {			/* this is for servers only! */
	fsid_t		fh_fsid;		/* filesystem id */
	struct fileid	fh_fid;			/* file id */
};
typedef struct fhandle fhandle_t;
#define fh_dev	fh_fsid.fsid_dev
#define fh_type	fh_fsid.fsid_type
#define fh_len	fh_fid.fid_len
#define fh_ino	fh_fid.fid_ino
#define fh_gen	fh_fid.fid_gen

/* structure and typedef for volume group IDs and physical volume IDs */
struct unique_id {
       unsigned long word1;
       unsigned long word2;
       unsigned long word3;
       unsigned long word4;
};
typedef struct unique_id unique_id_t;

#ifdef	_BSD			/* Berkeley source compatibility */

#ifndef _H_SELECT
#include <sys/select.h>
#endif

/*
 * The following macros, major(), minor() and makedev() are identical
 * to the macros in <sys/sysmacros.h>.  Any changes to either need to
 * be reflected in the other.
 */
/* major part of a device */
#define major(__x)        (int)((unsigned)(__x)>>16)

/* minor part of a device */
#define minor(__x)        (int)((__x)&0xFFFF)

/* make a device number */
#define makedev(__x,__y)    (dev_t)(((__x)<<16) | (__y))

#endif /* _BSD */

#ifdef _LONG_LONG
typedef	long long  offset_t;		/* file offset		*/
#endif	/* _LONG_LONG */

#endif   /* _ALL_SOURCE */
#endif /* _H_TYPES */
