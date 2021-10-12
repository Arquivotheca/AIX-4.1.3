/* @(#)70       1.3.1.4  src/bos/usr/include/ndbm.h, libcndbm, bos411, 9428A410j 2/8/94 00:58:05 */
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: LIBCNDBM
 *
 *   FUNCTIONS: dbm_clearerr, dbm_dirfno, dbm_error, dbm_pagfno, dbm_rdonly
 *		
 *   ORIGINS: 26,27,71
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
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
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef _H_NDBM
#define _H_NDBM
/*
 * 
 * FUNCTIONS: dbm_clearerr, dbm_dirfno, dbm_error, dbm_pagfno, dbm_rdonly 
 * Hashed key data base library.
 */
#define PBLKSIZ 1024
#define DBLKSIZ 4096

typedef struct {
	int	dbm_dirf;		/* open directory file */
	int	dbm_pagf;		/* open page file */
	int	dbm_flags;		/* flags, see below */
	long	dbm_maxbno;		/* last ``bit'' in dir file */
	long	dbm_bitno;		/* current bit number */
	long	dbm_hmask;		/* hash mask */
	long	dbm_blkptr;		/* current block for dbm_nextkey */
	int	dbm_keyptr;		/* current key for dbm_nextkey */
	long	dbm_blkno;		/* current page to read/write */
	long	dbm_pagbno;		/* current page in pagbuf */
	char	dbm_pagbuf[PBLKSIZ];	/* page file block buffer */
	long	dbm_dirbno;		/* current block in dirbuf */
	char	dbm_dirbuf[DBLKSIZ];	/* directory file block buffer */
#ifdef	_THREAD_SAFE
	void	*dbm_lock;		/* for inter-thread locking */
#endif
} DBM;

#define _DBM_RDONLY	0x1	/* data base open read-only */
#define _DBM_IOERR	0x2	/* data base I/O error */

#define dbm_rdonly(db)	((db)->dbm_flags & _DBM_RDONLY)

#define dbm_error(db)	((db)->dbm_flags & _DBM_IOERR)
	/* use this one at your own risk! */
#define dbm_clearerr(db)	((db)->dbm_flags &= ~_DBM_IOERR)

/* for flock(2) and fstat(2) */
#define dbm_dirfno(db)	((db)->dbm_dirf)
#define dbm_pagfno(db)	((db)->dbm_pagf)

typedef struct {
	char	*dptr;
	int	dsize;
} datum;

/*
 * flags to dbm_store()
 */
#define DBM_INSERT	0
#define DBM_REPLACE	1

#ifdef _NO_PROTO
DBM	*dbm_open();
void	dbm_close();
datum	dbm_fetch();
datum	dbm_firstkey();
datum	dbm_nextkey();
long	dbm_forder();
int	dbm_delete();
int	dbm_store();
#else
DBM	*dbm_open(char *, int, int);
void	dbm_close(DBM *);
datum	dbm_fetch(DBM *, datum);
datum	dbm_firstkey(DBM *);
datum	dbm_nextkey(DBM *);
long	dbm_forder(DBM *, datum);
int	dbm_delete(DBM *, datum);
int	dbm_store(DBM *, datum, datum, int);
#endif /* _NO_PROTO */
#endif /* _H_NDBM */
