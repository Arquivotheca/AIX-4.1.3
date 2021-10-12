static char sccsid[] = "@(#)65  1.4  src/bos/usr/ccs/lib/libc_r/getpwnamuid.c, libcs, bos411, 9428A410j 5/18/94 18:18:05";
#ifdef _POWER_PROLOG_
/*
 *
 *   COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions
 *
 *   FUNCTIONS: fetchpw, getpwnam_r, getpwnam, getpwuid_r, getpwuid
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
/* getpwnamuid.c,v $ $Revision: 2.10.2.2 $ (OSF) */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *#if defined(LIBC_SCCS) && !defined(lint)
 *getpwnamuid.c	5.3 (Berkeley) 12/21/87";
 *#endif LIBC_SCCS and not lint
 */

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <ndbm.h>

#include <sys/file.h>
#include <sys/lockf.h>

#include "ts_supp.h"
#include "push_pop.h"

extern void __shadow_chk(struct passwd *);
extern char * __shadow_pass(char *);

#ifdef _THREAD_SAFE

#include "rec_mutex.h"
/*
 * Protect access to db and pw file name.
 *
 * A lock is required at a minimum to protect the passwd file name.
 * For a decent thread safe interface the db-key/ fp would be passed
 * in - alas POSIX says otherwise.
 * We could open/ close the db _every_ time or use a lock while we access it.
 * So we extend the lock to cover use of the db (but not the file).
 * [This is only useful if the db is used and stay _pw_stayopen is set.]
 */

extern struct rec_mutex	_passwd_rmutex;

#define	STAY_OPEN		0	/* _must_ close fp to passwd file */

#define	SETPWENT()		setpwent_r(&pw_fp)
#define	GETPWENT(p)		(_getpwent_r(p, line, len, &pw_fp) != TS_FAILURE)
#define	ENDPWENT()		endpwent_r(&pw_fp)
#define	FETCHPW(k, p, l)	fetchpw(k, p, l, len)

#else

#define	MAXLINELENGTH	1024

#define	STAY_OPEN		_pw_stayopen

#define	SETPWENT()		setpwent()
#define	GETPWENT(p)		((p = _getpwent()) != TS_FAILURE)
#define	ENDPWENT()		endpwent()
#define	FETCHPW(k, p, l)	fetchpw(k, p, l, MAXLINELENGTH)

static char		*line;		/* [MAXLINELENGTH] */
static struct passwd	pw_passwd;

#endif	/* _THREAD_SAFE */

/*
 * The following are shared with getpwent.c
 */
extern char	*_pw_file;
DBM		*_pw_db;
int		_pw_stayopen;


static int
fetchpw(datum key, struct passwd *pwent, char *line, int len)
{
	register char *cp, *tp;
	int i;

	if (key.dptr == 0)
		TS_RETURN((errno = EINVAL, 0), 0);

	key = dbm_fetch(_pw_db, key);

	if (key.dptr == 0)
		TS_RETURN((errno = EINVAL, 0), 0);

	cp = key.dptr;
	tp = line;

#define	EXPAND(e)	pwent->e = tp; while (*tp++ = *cp++);

	EXPAND(pw_name);
	EXPAND(pw_passwd);

	bcopy(cp, (char *)&i, sizeof(i));
	cp += sizeof(i);
	pwent->pw_uid = i;
	bcopy(cp, (char *)&i, sizeof(i));
	cp += sizeof(i);
	pwent->pw_gid = i;
	bcopy(cp, (char *)&i, sizeof(i));
	cp += sizeof(i);

	EXPAND(pw_gecos);
	EXPAND(pw_dir);
	EXPAND(pw_shell);
	return (1);
}


#ifdef _THREAD_SAFE
int 
getpwnam_r(const char *nam, struct passwd *pwent, char *line, int len)
{
	FILE	*pw_fp = 0;
#else 
struct passwd *
getpwnam(const char *nam)
{
	register struct passwd	*pwent = &pw_passwd;
#endif	/* _THREAD_SAFE */

	int	retval = 0;
	datum	key;

#ifndef _THREAD_SAFE
	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((pwent == 0 || line == 0 || len <= 0));
	TS_LOCK(&_passwd_rmutex);
	TS_PUSH_CLNUP(&_passwd_rmutex);

	if (_pw_db == (DBM *)0 &&
	    (_pw_db = dbm_open(_pw_file, O_RDONLY, 0)) == (DBM *)0) {
oldcode:
		SETPWENT();
		while (GETPWENT(pwent))
			if (strcmp(nam, pwent->pw_name) == 0) {
				retval = 1;
				break;
			}
		if (!STAY_OPEN)
			ENDPWENT();

		if(!retval)
			retval = -1;

	} 
	if(!retval){
		(void) lseek(dbm_dirfno(_pw_db), 0, SEEK_SET);
		if (lockf(dbm_dirfno(_pw_db), F_LOCK, 0) < 0) {
			dbm_close(_pw_db);
			_pw_db = (DBM *)0;
			goto oldcode;
		}
		key.dptr = (char *)nam;
		key.dsize = strlen(nam);
		retval = FETCHPW(key, pwent, line);	/* retval=0 on fail, 1 on success */
		(void) lseek(dbm_dirfno(_pw_db), 0, SEEK_SET);
		(void) lockf(dbm_dirfno(_pw_db), F_ULOCK, 0);
		if (!_pw_stayopen) {
			dbm_close(_pw_db);
			_pw_db = (DBM *)0;
		}
	}
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_passwd_rmutex);
	if (retval == 1) {
		__shadow_chk(pwent);
		return (TS_FOUND(pwent));
	}
	if (retval == -1)
		return (TS_NOTFOUND);
	return (TS_FAILURE);
}


#ifdef _THREAD_SAFE
int
getpwuid_r(uid_t uid, struct passwd *pwent, char *line, int len)
{
	FILE	*pw_fp = 0;
#else
struct passwd *
getpwuid(uid_t uid)
{
	register struct passwd	*pwent = &pw_passwd;
#endif	/* _THREAD_SAFE */

	int	retval = 0;
	datum	key;

#ifndef _THREAD_SAFE
	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((pwent == 0 || line == 0 || len <= 0));
	TS_LOCK(&_passwd_rmutex);
	TS_PUSH_CLNUP(&_passwd_rmutex);

	if (_pw_db == (DBM *)0 &&
	    (_pw_db = dbm_open(_pw_file, O_RDONLY, 0)) == (DBM *)0) {
oldcode:
		SETPWENT();
		while (GETPWENT(pwent))
			if (pwent->pw_uid == uid) {
				retval = 1;
				break;
			}
		if (!STAY_OPEN)
			ENDPWENT();
		if(!retval)
			retval = -1;
	} else {
		(void) lseek(dbm_dirfno(_pw_db), 0, SEEK_SET);
		if (lockf(dbm_dirfno(_pw_db), F_LOCK, 0) < 0){
			dbm_close(_pw_db);
			_pw_db = (DBM *)0;
			goto oldcode;
		}
		key.dptr = (char *) &uid;
		key.dsize = sizeof uid;
		retval = FETCHPW(key, pwent, line);
		if (retval)
			retval = 1;
		(void) lseek(dbm_dirfno(_pw_db), 0, SEEK_SET);
		(void) lockf(dbm_dirfno(_pw_db), F_ULOCK, 0);
		if (!_pw_stayopen) {
			dbm_close(_pw_db);
			_pw_db = (DBM *)0;
		}
	}
	TS_POP_CLNUP(0);
	TS_UNLOCK(&_passwd_rmutex);
	if (retval == 1) {
		__shadow_chk(pwent);
		return (TS_FOUND(pwent));
	}
	if (retval == -1)
		return (TS_NOTFOUND);
	return (TS_FAILURE);
}
