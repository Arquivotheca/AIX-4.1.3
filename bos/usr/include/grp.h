/* @(#)16       1.11.1.6  src/bos/usr/include/grp.h, libcs, bos411, 9428A410j 5/9/94 16:46:28 */

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

#ifndef _H_GRP
#define _H_GRP

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/* The POSIX standard requires that certain elements be included in grp.h. 
 * It also requires that when _POSIX_SOURCE is defined, only those standard
 * specific elements are made available.  If _THREAD_SAFE is also defined,
 * thread-safe versions of the POSIX names are also made available.
 *
 * This header includes all the POSIX-required entries.
 */

#ifdef _POSIX_SOURCE

struct	group {				/* see getgrent(3) */
	char	*gr_name;
	char	*gr_passwd;
	gid_t	gr_gid;
	char	**gr_mem;
};

#ifdef _NO_PROTO
	extern struct group	*getgrgid();
	extern struct group	*getgrnam();
#ifdef _THREAD_SAFE
	extern int		getgrgid_r();
	extern int		getgrnam_r();
#endif /* _THREAD_SAFE */
#else /* _NO_PROTO */
	extern struct group	*getgrgid(gid_t);
	extern struct group	*getgrnam(const char *);
#ifdef _THREAD_SAFE
	extern int	getgrgid_r(gid_t, struct group *, char *, int);
	extern int	getgrnam_r(const char *, struct group *, char *, int);
#endif /* _THREAD_SAFE */
#endif /* _NO_PROTO */
#endif /* _POSIX_SOURCE */

#ifdef _ALL_SOURCE 

#ifndef _H_STDIO
#include <stdio.h>
#endif

/* <limits.h> has historically been included in <grp.h>.  However,
   it is not allowed by Posix.1.  The include must remain in _ALL_SOURCE.
*/

#ifndef _H_LIMITS
#include <limits.h>
#endif

#define NGROUPS NGROUPS_MAX       /** as many as there are **/

#ifdef _THREAD_SAFE
#define MAXGRP          2000
/* In GRPLINLEN the 14 represents the ":!:GID LENGTH:" in the group file. */
#define GRPLINLEN       (L_cuserid) + ((L_cuserid + 1) * MAXGRP) + 14

struct _grjunk
        {
        char    *_domain;
        struct  group _NULLGP;
        FILE    *_grf;
        char    *_yp;
        int     _yplen;
        char    *_oldyp;
        int     _oldyplen;
        char    *_agr_mem[MAXGRP];
        struct list
        {
                char *name;
                struct list *nxt;
        } *_minuslist;
        struct  group _interpgp;
        char    _interpline[GRPLINLEN+1];
};
#endif /* _THREAD_SAFE */

#ifdef _NO_PROTO
	extern struct group	*getgrent();
	extern char		*getgrset();
	extern void		setgrent();
	extern struct group	*fgetgrent();
	extern void		endgrent();
#ifdef _THREAD_SAFE
	extern int		getgrset_r();
	extern int		getgrent_r();
	extern int		setgrent_r();
	extern int		fgetgrent_r();
	extern void		endgrent_r();
#endif /* _THREAD_SAFE */
#else /* _NO_PROTO */
	extern struct group	*getgrent(void);
	extern char		*getgrset(const char *);
	extern void		setgrent(void);
	extern struct group	*fgetgrent(FILE *);
	extern void		endgrent(void);
#ifdef _THREAD_SAFE
	extern int	getgrent_r(struct group *, char *, int, FILE **);
	extern int	getgrset_r(register char *, struct _grjunk *);
	extern int	setgrent_r(FILE **);
	extern int	fgetgrent_r(FILE *, struct group *, char *, int);
	extern void	endgrent_r(FILE **);
#endif /* _THREAD_SAFE */

#endif /* _NO_PROTO */

#endif /* _ALL_SOURCE */

#endif /* _H_GRP */
