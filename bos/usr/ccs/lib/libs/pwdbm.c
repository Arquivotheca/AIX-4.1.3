static char sccsid[] = "@(#)16	1.6  src/bos/usr/ccs/lib/libs/pwdbm.c, libs, bos411, 9438C411d 9/24/94 20:55:43";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: 
 *		pwdbm_open
 *		pwdbm_update
 *		pwdbm_delete
 *		pwdbm_add
 *		_writeunlock
 *		_writelock
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <usersec.h>
#include <userconf.h>
#include <fcntl.h>
#include <ndbm.h>
#include <sys/stat.h>

/*
 * Global routines.
 */
DBM *	pwdbm_open();
int  	pwdbm_update(DBM *, char *, char *, int);
int  	pwdbm_delete(DBM *, char *);
int  	pwdbm_add(DBM *, char *);


void _writeunlock();
int  _writelock(int);


/*
 * FUNCTION:	pwdbm_open().
 *
 * DESCRIPTION:	checks to see if the dbm files should be updated, opens
 *              the dbm files if so.
 *
 * PASSED: nothing.
 *
 * RETURNS: returns a pointer to a DBM structure if dbm files exist,
 *		were uptodate, and were opened, otherwise returns NULL.
 */
DBM *
pwdbm_open()
{
	static char *pw_file = "/etc/passwd";
	DBM *db;

	struct stat sbpw, sbdbm;
	if ((db = dbm_open(pw_file, O_RDWR, 0)) == (DBM *) 0)
	    return NULL;

	if (stat(pw_file, &sbpw) == 0 && fstat(dbm_pagfno(db), &sbdbm) == 0
	&& sbdbm.st_mtime >= sbpw.st_mtime)
	    return (db);

	dbm_close(db);
	return NULL;
}


/*
 * FUNCTION:	pwdbm_update().
 *
 * DESCRIPTION:	updates the dbm files.
 *
 * PASSED:	db = the DBM pointer,
 *		name = the username, info = the new string,
 *		field =  PWDBM_PASSWD, PWDBM_GECOS, PWDBM_SHELL for
 *			updating the passwd, gecos, or shell info.
 *
 * RETURNS: returns 0 if updated, otherwise returns -1.
 */
int
pwdbm_update(DBM *db, char *name, char *info, int field)
{
	int rc = -1;
        datum key, data;

	/* lock the dbm files */
	if (_writelock(dbm_dirfno(db)))
		return -1;

	/* get the old entry */
	key.dptr = name;
	key.dsize = strlen(name);
	data = dbm_fetch(db, key);
        if (data.dptr != NULL) {
	    char line[BUFSIZ+1], *tp = line;
	    char *cp = data.dptr;
	    uid_t uid;

	    /* copy the entry, updating the info */

	    /* copy name */
	    while (*tp++ = *cp++) ;
	    /* copy passwd */
            if (field == PWDBM_PASSWD) {
		while (*tp++ = *info++) ;
		while (*cp++) ;
	    }
	    else
		while (*tp++ = *cp++) ;

	    /* copy uid */
	    uid = *(uid_t *) tp = *(uid_t *) cp;
	    tp += sizeof (uid_t);
	    cp += sizeof (uid_t);

	    /* copy gid */
	    *(gid_t *) tp = *(gid_t *) cp;
	    tp += sizeof (gid_t);
	    cp += sizeof (gid_t);

	    /* copy gecos */
            if (field == PWDBM_GECOS) {
		while (*tp++ = *info++) ;
		while (*cp++) ;
	    }
	    else
		while (*tp++ = *cp++) ;

	    /* copy dir */
	    while (*tp++ = *cp++) ;

	    /* copy shell */
            if (field == PWDBM_SHELL) {
		while (*tp++ = *info++) ;
		while (*cp++) ;
	    }
	    else
		while (*tp++ = *cp++) ;

	    data.dptr = line;
	    data.dsize = tp - line;

	    /* store updated info under name */
	    rc = dbm_store(db, key, data, DBM_REPLACE);

	    /* store updated info under uid */
	    key.dptr = (char *) &uid;
	    key.dsize = sizeof (uid);
	    rc = dbm_store(db, key, data, DBM_REPLACE);
	}

	_writeunlock(dbm_dirfno(db));

	return rc;
}

/*
 * FUNCTION:	pwdbm_delete().
 *
 * DESCRIPTION:	deletes a record from the dbm files.
 *
 * PASSED:	db = the DBM pointer,
 *		name = the username
 *
 * RETURNS: returns 0 if deleted, otherwise returns -1.
 */
int
pwdbm_delete(DBM *db, char *name)
{
	int rc = -1;
        datum key, data;

	/* lock the dbm files */
	if (_writelock(dbm_dirfno(db)))
		return -1;

	/* get the old entry */
	key.dptr = name;
	key.dsize = strlen(name);
	data = dbm_fetch(db, key);

        if (data.dptr != NULL) {	/* update it */
	    char *cp = data.dptr;
	    uid_t uid;

	    /* skip name */
	    while (*cp++) ;
	    /* skip passwd */
	    while (*cp++) ;

	    /* get uid */
	    uid = *(uid_t *) cp;

	    /* delete updated info using name as the key */
	    rc = dbm_delete(db, key);

	    /* delete updated info using uid as the key */
	    key.dptr = (char *) &uid;
	    key.dsize = sizeof (uid);

	    rc = dbm_delete(db, key);
	}

	/* unlock the dbm files */
	_writeunlock(dbm_dirfno(db));

	return rc;
}

/*
 * FUNCTION:	pwdbm_add()
 *
 * DESCRIPTION:	adds a record to the dbm files.
 *		(copied from the mkpasswd program)
 *
 * PASSED:	db = the DBM pointer, username = the user to add
 *
 * RETURNS: returns 0 if added, otherwise returns -1.
 */
int
pwdbm_add(DBM *db, char *username)
{
	int rc;
	char *passwd;
	uid_t uid;
	gid_t gid;
	char *groupname;
	char *gecos;
	char *dir;
	char *shell;
	/*
	 * not sure why the following are volatile, but mkpasswd.c
	 * had them that way, so perhaps there's a reason...
	 */
	volatile datum	key;
	volatile datum	content;
	char buf[BUFSIZ], *cp = buf;
	
	setuserdb(S_READ);
	/* get attributes to add */
	if (getuserattr(username, S_ID, &uid, SEC_INT) ||
	    getuserattr(username, S_PWD, &passwd, SEC_CHAR) ||
	    getuserattr(username, S_PGRP, &groupname, SEC_CHAR) ||
	    getgroupattr(groupname, S_ID, (void *) &gid,SEC_INT) ||
	    getuserattr(username, S_HOME, &dir, SEC_CHAR) ||
	    getuserattr(username, S_SHELL, &shell, SEC_CHAR))
		return -1;

	/* if gecos fails, it's usually just not set */
	if (getuserattr(username, S_GECOS, &gecos, SEC_CHAR))
		gecos = "";

	/* lock the dbm files */
	if (_writelock(dbm_dirfno(db)))
		return -1;

	/* build the dbm record to be added */
	strcpy (cp, username);
	cp = strchr (cp, '\0') + 1;

	strcpy (cp, passwd);
	cp = strchr (cp, '\0') + 1;

	bcopy ((char *) &uid, cp, sizeof (uid));
	cp += sizeof(uid);

	bcopy ((char *)&gid, cp, sizeof (gid));
	cp += sizeof(gid);

	strcpy (cp, gecos);
	cp = strchr (cp, '\0') + 1;

	strcpy (cp, dir);
	cp = strchr (cp, '\0') + 1;

	strcpy (cp, shell);
	cp = strchr (cp, '\0') + 1;

	/*
	 * Build the values for the key and content structures
	 * to be handed to DBM.  Each record is stored twice -
	 * once by its name and again by its UID.
	 */
	content.dptr = buf;
	content.dsize = cp - buf;

	/* Create a name to entry mapping */
	key.dptr = username;
	key.dsize = strlen (username);

	if ((rc = dbm_store (db, key, content, DBM_REPLACE)) == 0) {
		/* create a uid to entry mapping */
		key.dptr = (char *) &uid;
		key.dsize = sizeof (uid);

		rc = dbm_store (db, key, content, DBM_REPLACE);
		}

	/* unlock the dbm files */
	_writeunlock(dbm_dirfno(db));
	enduserdb();

	return rc;
}

static	struct	flock	pwd_lock;

/*
 * NAME: _writelock
 * 
 * FUNCTION: establish a write lock on the file descriptor 'fd'
 * 
 * RETURNS: 	-1 		- Failure to establish lock
 *	    Other than -1	- Lock established
 */
int
_writelock(int fd)
{
	int	retries;

	pwd_lock.l_type = F_WRLCK;
	pwd_lock.l_whence = SEEK_SET;
	pwd_lock.l_start = 0L;
	pwd_lock.l_len = 0L;

	/*
	 * Allow up to 10 attempts to lock the file, sleeping between
	 * each attempt.
	 */
	for (retries = 0;retries < 10;retries++) {
		if (retries)
			usleep(retries * 5000);

		if (! fcntl (fd, F_SETLK, &pwd_lock))
			return 0;
	}
	return -1;
}

/*
 * NAME: _writeunlock
 *
 * FUNCTION: unlock the write lock on the file descriptor 'fd'
 *
 * RETURNS: nothing
 */
void
_writeunlock(int fd)
{
	pwd_lock.l_type = F_UNLCK;

	(void) fcntl(fd, F_SETLK, &pwd_lock);
}

