/* @(#)04	1.6.1.2  src/bos/usr/include/fstab.h, cmdfs, bos411, 9428A410j 1/7/93 11:08:42 */
#ifndef _H_FSTAB
#define _H_FSTAB 
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: CMDFS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 26,27,71
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>

/*
 * File system (filesystems) 
 *
 * The fs_spec field is the block special name.  Programs
 * that want to use the character special name must create
 * that name by prepending a 'r' after the right most slash.
 * Quota files are always named "quotas", so if type is "rq",
 * then use concatenation of fs_file and "quotas" to locate
 * quota file.
 */

#define	FSTAB_RW	"rw"	/* read/write device */
#define	FSTAB_RQ	"rq"	/* read/write with quotas */
#define	FSTAB_RO	"ro"	/* read-only device */
#define	FSTAB_SW	"sw"	/* swap device */
#define	FSTAB_XX	"xx"	/* ignore totally */

struct	fstab{
	char	*fs_spec;		/* block special device name */
	char	*fs_file;		/* file system path prefix */
	char	*fs_type;		/* read/write, etc see above defines */
	int	fs_check;		/* true=0, false=-1, else "check" val */
	int	fs_freq;		/* not used */
	int	fs_passno;		/* not used */
};

extern  struct	fstab *getfsent();
extern  struct	fstab *getfsspec();
extern  struct	fstab *getfsfile();
extern  struct	fstab *getfstype();
extern  int	setfsent();
extern  int	endfsent();

#ifdef _THREAD_SAFE
#ifdef _NO_PROTO
extern	int	setfsent_r();
extern	int	endfsent_r();
extern	int     getfsent_r();
extern	int     getfsspec_r();
extern	int     getfstype_r();
extern	int     getfsfile_r();
#else  /* _NO_PROTO */
extern int setfsent_r(AFILE_t *, int *);
extern int endfsent_r(AFILE_t *);
extern int getfsent_r(struct fstab *, AFILE_t *, int *);
extern int getfsspec_r(const char *, struct fstab *, AFILE_t *, int *);
extern int getfstype_r(const char *, struct fstab *, AFILE_t *, int *);
extern int getfsfile_r(const char *, struct fstab *, AFILE_t *, int *);
#endif /* _NO_PROTO */
#endif /* _THREAD_SAFE */

#endif /* _H_FSTAB */
