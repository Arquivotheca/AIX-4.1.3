static char sccsid[] = "@(#)21	1.6  src/bos/usr/ccs/lib/libc_r/libc_locks.c, libcthrd, bos411, 9428A410j 2/22/94 06:05:14";
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: LIBC
 *
 *   FUNCTIONS: _libc_locks_init 
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
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
/* libc_locks.c,v $ $Revision: 1.8.2.5 $ (OSF) */

/*
 *  This file contains the declarations of all locks used in libc.
 */

#include "rec_mutex.h"

lib_spinlock_t		_malloc_lock;		/* malloc() routines */

struct rec_mutex	_abort_rmutex;		/* abort() race in _cleanup() */
struct rec_mutex	_alarm_rmutex;		/* per-process alarm */
struct rec_mutex	_atof_rmutex;		/* jmpbuf for mmax fp hack */
struct rec_mutex	_brk_rmutex;		/* sbrk() and brk() in static libc */
struct rec_mutex	_catalog_rmutex;	/* catalog descriptor */
struct rec_mutex	_clock_rmutex;		/* initial clock() call */
struct rec_mutex	_crypt_rmutex;		/* lock for set_up of pointers*/
struct rec_mutex	_ctime_rmutex;		/* static variables */
struct rec_mutex	_domain_rmutex;		/* static data (domain) */
struct rec_mutex	_qecvt_rmutex;		/* _qecvt() not reentrant */
struct rec_mutex	_environ_rmutex;	/* environ data */
struct rec_mutex	_exec_rmutex;		/* static data from _exec_argv() */
struct rec_mutex	_exit_rmutex;		/* atexit() push/ pop and _cleanup */
struct rec_mutex	_AF_rmutex;		/* AF routines are not thread safe */
struct rec_mutex	_iop_rmutex;		/* iobpt[] access */
struct rec_mutex	_getusershell_rmutex;	/* user shell names */
struct rec_mutex	_mktemp_rmutex;		/* Locks the static structure for jrand48_r */
struct rec_mutex	_nanotimer_rmutex;	/* per-process timer state */
struct rec_mutex	_nice_rmutex;		/* per-process priority */
struct rec_mutex	_odm_rmutex;		/* used in getttytent */
struct rec_mutex	_passwd_rmutex;		/* passwd filename and name/id lookup */
struct rec_mutex	_rand_rmutex;		/* static variable (randx) */
struct rec_mutex	_resolv_rmutex;		/* _res struct and socket access */
struct rec_mutex	_stdio_buf_rmutex[3];	/* initial stream locks */
struct rec_mutex	_tmpnam_rmutex;		/* static variable (seed) */
struct rec_mutex	_utmp_rmutex;		/* utmp filename */
struct rec_mutex	_popen_rmutex;		/* popen,pclose */
struct rec_mutex	_validuser_rmutex;  	/* locals need be static */
struct rec_mutex	_libs_rmutex;		/*libs routine locks */
struct rec_mutex	_netrc_rmutex;		/* check netrc file */
struct rec_mutex	_hostservices_rmutex;	/* used in gethostent.c */
struct rec_mutex	_ypresolv_rmutex; 	/* handle yp routines */
struct rec_mutex	_getnetgrent_rmutex;	/* handle yp groups */

extern lib_lock_functions_t 	_libc_lock_funcs;

void
_libc_locks_init()
{

	lib_spinlock_create(_libc_lock_funcs, &_malloc_lock);

	/* Locks for stdio.  These are the iob locks */
	_rec_mutex_init(&_iop_rmutex);
	_rec_mutex_init(&_stdio_buf_rmutex[0]);
	_rec_mutex_init(&_stdio_buf_rmutex[1]);
	_rec_mutex_init(&_stdio_buf_rmutex[2]);

	/* Locks for compat-4.1 */
	_rec_mutex_init(&_rand_rmutex);

	/* Locks for compat-sys5 */
	_rec_mutex_init(&_alarm_rmutex);
	_rec_mutex_init(&_clock_rmutex);
	_rec_mutex_init(&_tmpnam_rmutex);

	/* Locks for gen */
	_rec_mutex_init(&_AF_rmutex);
	_rec_mutex_init(&_ctime_rmutex);
	_rec_mutex_init(&_getusershell_rmutex);
	_rec_mutex_init(&_utmp_rmutex);
	_rec_mutex_init(&_passwd_rmutex);
	_rec_mutex_init(&_environ_rmutex);
	_rec_mutex_init(&_brk_rmutex);
	_rec_mutex_init(&_popen_rmutex);

	/* Locks for n16/gen */
	_rec_mutex_init(&_abort_rmutex);

	_rec_mutex_init(&_odm_rmutex);

	/* lock for internationalization */
	_rec_mutex_init(&_catalog_rmutex);

	_rec_mutex_init(&_exec_rmutex);
	_rec_mutex_init(&_exit_rmutex);
	_rec_mutex_init(&_nice_rmutex);

	_rec_mutex_init(&_mktemp_rmutex);
	_rec_mutex_init(&_nanotimer_rmutex);

	_rec_mutex_init(&_atof_rmutex);

	_rec_mutex_init(&_resolv_rmutex);
	_rec_mutex_init(&_domain_rmutex);
	_rec_mutex_init(&_qecvt_rmutex);
	_rec_mutex_init(&_netrc_rmutex);
	_rec_mutex_init(&_hostservices_rmutex);
	_rec_mutex_init(&_getnetgrent_rmutex);
	_rec_mutex_init(&_ypresolv_rmutex);

	_rec_mutex_init(&_validuser_rmutex);

	/* crypt locks */
	_rec_mutex_init(&_crypt_rmutex);

	/*libs routine locks */
	_rec_mutex_init(&_libs_rmutex);
}
