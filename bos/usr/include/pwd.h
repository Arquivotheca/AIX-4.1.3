/* @(#)29	1.13.1.6  src/bos/usr/include/pwd.h, libcs, bos411, 9428A410j 5/9/94 16:46:26 */

#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (LIBCS) C Library Security Routines
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_PWD
#define _H_PWD

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/* The POSIX standard requires that certain elements be included in pwd.h. 
 * It also requires that when _POSIX_SOURCE is defined, only those standard
 * specific elements are made available.  If _THREAD_SAFE is also defined,
 * thread-safe versions of the POSIX names are also made available.
 *
 * This header includes all the POSIX-required entries.
 */

#ifdef _POSIX_SOURCE

struct passwd {
	char	*pw_name;
	char	*pw_passwd;
	uid_t	pw_uid;
	gid_t	pw_gid;
	char    *pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

#ifdef _NO_PROTO
	extern struct passwd	*getpwuid();
	extern struct passwd	*getpwnam();
#ifdef _THREAD_SAFE
	extern int		getpwuid_r();
	extern int		getpwnam_r();
#endif /* _THREAD_SAFE */
#else  /* _NO_PROTO */
	extern struct passwd	*getpwuid(uid_t);
	extern struct passwd	*getpwnam(const char *);
#ifdef _THREAD_SAFE
	extern int	getpwuid_r(uid_t, struct passwd *, char *, int);
	extern int	getpwnam_r(const char *, struct passwd *, char *, int);
#endif /* _THREAD_SAFE */
#endif /* _NO_PROTO */
#endif /* _POSIX_SOURCE */

#ifdef _ALL_SOURCE 

#ifndef _H_STDIO
#include <stdio.h>
#endif

#ifdef _NO_PROTO
	extern struct passwd	*getpwent();
	extern int		putpwent();
	extern void		setpwent();
	extern void		endpwent();
#ifdef _THREAD_SAFE
	extern int		getpwent_r();
	extern int		fgetpwent_r();
	extern int		setpwent_r();
	extern void		endpwent_r();
#endif /*_THREAD_SAFE */
#else  /* _NO_PROTO */
	extern struct passwd	*getpwent(void);
	extern int		putpwent(struct passwd *, FILE *);
	extern void		setpwent(void);
	extern void		endpwent(void);
#ifdef _THREAD_SAFE
	extern int	getpwent_r(struct passwd *, char *, int, FILE **);
	extern int	fgetpwent_r(FILE *, struct passwd *, char *, int);
	extern int	setpwent_r(FILE **);
	extern void	endpwent_r(FILE **);
#endif /* _THREAD_SAFE */

#endif /* _NO_PROTO */

#define pw_etc pw_gecos

#endif /* _ALL_SOURCE */ 

#endif /* _H_PWD */
