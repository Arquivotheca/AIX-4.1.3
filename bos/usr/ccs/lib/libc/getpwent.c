static char sccsid[] = "@(#)23        1.36.1.16  src/bos/usr/ccs/lib/libc/getpwent.c, libcs, bos41J, 9522A_all 5/30/95 14:49:53";
/*
 *   COMPONENT_NAME: LIBCS
 *
 *   FUNCTIONS: PASSWDFOUND
 *		REPORT
 *		USERFOUND
 *		_pwjunk
 *		addtominuslist
 *		bind_to_yp
 *		check_dce
 *		check_yellow
 *		endpwent
 *		fetchpw
 *		freeminuslist
 *		getfirstfromyellow
 *		getnamefromyellow
 *		getnextfromyellow
 *		getpwent
 *		_getpwent
 *		getpwnam
 *		getpwnam_compat
 *		_getpwnam_shadow
 *		getpwuid
 *		getpwuid_compat
 *		getuidfromyellow
 *		interpret
 *		interpretwithsave
 *		ispassrmtnam
 *		isuserrmtnam
 *		matchname
 *		matchuid
 *		onminuslist
 *		pw_close
 *		pw_open
 *		pwskip
 *		reset_pwjunk_authstate
 *		save
 *		set_getpwent_remote
 *		setpwent
 *		setpwfile
 *		shadow_chk
 *		shadow_pass
 *		stanzmat
 *		stripaftercomma
 *		uidof
 *
 *   ORIGINS: 24,26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1983 Regents of the University of California.
 *   All rights reserved.  The Berkeley software License Agreement
 *   specifies the terms and conditions for redistribution.
 *
 *   Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <pwd.h>
#include <ndbm.h>                   

#ifdef YP
#include <rpcsvc/ypclnt.h>
#endif

#include <sys/stat.h>
#include <sys/file.h>
#include <sys/lockf.h>
#include <errno.h>
#include <userpw.h>
#include <usersec.h>

#define MAXINT 0x7fffffff;

/*
 * Prototypes for all routines defined within this file
 */
struct passwd * 	getpwent();
struct passwd *		_getpwent();
struct passwd * 	getpwnam(const char *);
struct passwd * 	_getpwnam_shadow(const char *,int);
struct passwd * 	getpwuid(uid_t);
void 			setpwent();
void			endpwent();
int 			setpwfile(char *);
void 			set_getpwent_remote(int);
void			reset_pwjunk_authstate();
int			isuserrmtnam(char *);
int			ispassrmtnam(char *);

static struct passwd * 	getpwnam_compat(const char *);
static struct passwd * 	getpwuid_compat(uid_t);
static int		check_dce();
static void 		stripaftercomma(char *);
static struct _pwjunk *	_pwjunk();
static FILE *		pw_open(char *, char *);
static int		pw_close(FILE *);
static struct passwd * 	fetchpw(datum);
static char * 		pwskip(register char *);
static int		matchname(char *, struct passwd **, char *);
static int		matchuid(char *, struct passwd **, uid_t);
static int		uidof(char *);
static int		getnextfromyellow();
static int		getfirstfromyellow();
static struct passwd * 	getnamefromyellow(char *, struct passwd *);
static struct passwd * 	getuidfromyellow(uid_t, struct passwd *);
static struct passwd * 	interpretwithsave(char *, int, struct passwd *);
static struct passwd * 	interpret(char *, int);
static struct passwd * 	save(struct passwd *);
static int		freeminuslist();
static int		addtominuslist(char *);
static int		onminuslist(struct passwd *);
static int		stanzmat(char *, char *);
static char *   	shadow_pass(char *);
static void		shadow_chk(register struct passwd *);
static int		check_yellow();
static int		bind_to_yp();

/*
 * Prototypes for externally used routines.
 */
extern void		free();
extern unsigned long	strtoul();
extern int		strcmp();
extern int		strlen();
extern char		*strcpy();
extern char		*strncpy();
extern char		*malloc();

/* 
 * Externally used variables.
 */
extern int	errno;

/*
 * Locally defined variables.
 */
DBM *		_pw_db;
int		_pw_stayopen = 0;

static struct  _pwjunk {
	struct passwd _NULLPW;
	FILE * _pwf;			/* pointer into /etc/passwd           */
	FILE * _spwf;			/* pointer into /etc/security/passwd  */
	char	*_yp;			/* pointer into yellow pages 	      */
	int	_yplen;
	char	*_oldyp;
	int	_oldyplen;
	enum   	{FILES, COMPAT, DCE}
			_authstate;	/* first name resolution mechanism    */
	struct secmethod_table 		/* Function pointer table for 	      */
			_dcemethod;	/* loadable module interface (DCE)    */
	struct list {
		char	*name;
		struct list *nxt;
	} *_minuslist;
	struct passwd _interppasswd;
	char	_interpline[BUFSIZ+1];
	char	*_domain;
	char	*_PASSWD;
	char	*_SPASSWD;
	int	_usingyellow;
} *__pwjunk;



enum wherefound	{LOCAL, NIS_DOMAIN, DCE_DOMAIN};

/*
 * The following macros and static variable define and retrieve a state 
 * variable signifying where during the lookup the user's password was found.
 * Possible values are the local database (LOCAL), the NIS database (NIS), 
 * or within a DCE cell (DCE).  This variable is initialized within the 
 * ispassrmtnam() routine, subsequently set during a getpwnam() lookup
 * (depending on where the password was found), and finally used in the
 * return from ispassrmtnam().
 */
static 	enum wherefound		_passwdfound;
#define	PASSWDFOUND(a)		_passwdfound = a
#define IS_PASSWD_NIS		_passwdfound == NIS_DOMAIN
#define IS_PASSWD_DCE		_passwdfound == DCE_DOMAIN



/*
 * The following macros and static variable define and retrieve a state 
 * variable signifying where during the lookup the user was found.
 * Possible values are the local database (LOCAL), the NIS database (NIS), 
 * or within a DCE cell (DCE).  This variable is initialized within the 
 * isusrrmtnam() routine, subsequently set during a getpwnam() lookup
 * (depending on where the user was found), and finally used in the
 * return from isuserrmtnam().
 */
static 	enum wherefound		_userfound;
#define	USERFOUND(a)		_userfound = a
#define IS_USER_NIS		_userfound == NIS_DOMAIN
#define IS_USER_DCE		_userfound == DCE_DOMAIN


/*
 * The following macros, defines, and static variable define and retrieve
 * what paths, name resolution should take.  This variable is set in the
 * set_getpwent_remote() routine and checked in the getpwnam() routine
 * and whenever an NIS lookup is about to take place.  Before a lookup
 * in another domain (such as NIS) the programmer should use 
 * REPORT(NIS_MASK) to determine if lookups are meant to flow into the NIS
 * domain. (see comments in the set_getpwent_remote() for instructions on
 * valid settings. 
 */
#define SET_LOCAL	0x1	/* Local only resolution 		*/
#define SET_ALL		0x2	/* Resolve through all possible methods */
#define SET_LOCAL_NIS	0x4	/* Resolve through local and NIS only 	*/
#define SET_DCE		0x8	/* Resolve through DCE only		*/
static  int 		_report_remote_entries = SET_ALL;

#define LOCAL_MASK	0x7
#define NIS_MASK	0x6
#define DCE_MASK	0xA
#define REPORT(a)	(_report_remote_entries & a)

#define	NULLPW 		(_pw->_NULLPW)
#define pwf 		(_pw->_pwf)
#define spwf 		(_pw->_spwf)
#define yp 		(_pw->_yp)
#define yplen 		(_pw->_yplen)
#define	oldyp 		(_pw->_oldyp)
#define	oldyplen 	(_pw->_oldyplen)
#define minuslist 	(_pw->_minuslist)
#define interppasswd 	(_pw->_interppasswd)
#define interpline 	(_pw->_interpline)
#define domain 		(_pw->_domain)
#define PASSWD 		(_pw->_PASSWD)
#define SPASSWD 	(_pw->_SPASSWD)
#define usingyellow 	(_pw->_usingyellow)
#define authstate    	(_pw->_authstate)
#define dcemethod      	(_pw->_dcemethod)
#define getpwuid_dce 	(_pw->_dcemethod.method_getpwuid)
#define getpwnam_dce 	(_pw->_dcemethod.method_getpwnam)



/*
 * NAME: _pwjunk
 *
 * FUNCTION: Initialization routine for static and malloc'd variables.
 * 	     All routines that attempt to update or query the state
 *	     of the internal routines should call this first.
 *
 * RETURNS:  0 		       - failure in initialzation 
 *	     _pwjunk structure - correct initialization
 */
static struct  _pwjunk *             
_pwjunk()
{
	register struct  _pwjunk *_pw = __pwjunk;

	if (_pw == 0) {
		char *env;

		_pw = (struct  _pwjunk * )calloc(1, sizeof (*__pwjunk));
		if (_pw == 0)
			return (0);

		PASSWD = "/etc/passwd";
		SPASSWD = "/etc/security/passwd";

        	/*
         	 * Check the AUTHSTATE environment variable for the primary
         	 * authentication mechanism that was used by this user
         	 * during login.  This mechanism will be attempted first.
         	 * The default resolution is (local files plus NIS) if the
         	 * environment variable is not set.
         	 */
        	authstate = COMPAT;
        	env = getenv(AUTH_ENV);
        	if (!(env == NULL || env[0] == '\0'))
                	if (!strcmp(env, AUTH_DCE))
                        	authstate = DCE;

		__pwjunk = _pw;
	}
	return (__pwjunk);
}

static FILE *
pw_open(char *fn, char *mode)
{
	/* open file with a close on exec */
	int	flags;
	FILE * fp;

	if ((fp = fopen(fn, mode)) != NULL) {
		flags = fcntl (fp->_file, F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl (fp->_file, F_SETFD, flags) < 0) {
			fclose(fp);
			fp = NULL;
		}
	}
	return fp;
}


static int	
pw_close(FILE *fp)
{
	/* close for pw_open */
	return fclose(fp);
}


static struct passwd *
fetchpw(datum key)
{
	register char	*cp, *tp;
	static struct passwd passwd;
	static char	line[BUFSIZ+1];

	if (key.dptr == 0)
		return ((struct passwd *)NULL);
	key = dbm_fetch(_pw_db, key);
	if (key.dptr == 0)
		return ((struct passwd *)NULL);
	cp = key.dptr;
	tp = line;

#define EXPAND(e)       passwd.pw_##e = tp; while (*tp++ = *cp++);
	EXPAND(name);
	EXPAND(passwd);
	bcopy(cp, (char *) & passwd.pw_uid, sizeof (uid_t));
	cp += sizeof (int);
	bcopy(cp, (char *) & passwd.pw_gid, sizeof (gid_t));
	cp += sizeof (int);
	EXPAND(gecos);
	EXPAND(dir);
	EXPAND(shell);

	return (&passwd);
}

/*
 * NAME: _getpwnam_shadow
 * 
 * FUNCTION: Given a user name, this routine will attempt to resolve
 *	     it to a passwd structure.  The routine has several possible
 *	     callout routines.  If the user authenticated via DCE, this
 *	     will be the primary callout.  Otherwise we will use
 *	     the normal name resolution routines, herein named 
 *	     getpwnam_compat().  Shadow checking will be done if
 *	     called with shadow = 1.
 *
 * RETURNS:  Null 		- Name resolution failed.
 *	     struct passwd *	- User's passwd structure.
 *	     shadow		- 0: Don't do shadow checking
 *	     			- 1: Do shadow checking
 */
struct passwd *
_getpwnam_shadow(const char *nam,int shadow)
{
        register struct _pwjunk *_pw = _pwjunk();
        struct passwd *pw = (struct passwd *)NULL;
	int keepopen = 0;

        if (_pw == 0)
                return (0);

	if (pwf != NULL)
		keepopen = 1;

	setpwent();	/* Required for shadow_chk() */

        /*
         * If my primary authentication was something other than DCE
  	 * (meaning local or NIS) then I will check the local files (and NIS) 
	 * first.  If this resolution does not return a valid entry then I 
	 * will attempt to load dce and call the getpwnam_dce() routine.
         *
         * Otherwise my authstate environment variable was DCE so I
         * should check DCE and if that fails check the local files.
         */
        if (authstate != DCE)
        {
                if ((pw = getpwnam_compat(nam)) == (struct passwd *)NULL)
		{
                        if (check_dce()) 
			{
				if ((pw = getpwnam_dce(nam)) != 
				    (struct passwd *)NULL)
				{
					USERFOUND(DCE_DOMAIN);
					PASSWDFOUND(DCE_DOMAIN);
				}
			}
		}
		else
			if (shadow)
				shadow_chk(pw);
        }
        else
        {
                if (check_dce())
		{
                        if((pw = getpwnam_dce(nam)) != (struct passwd *)NULL)
			{
				USERFOUND(DCE_DOMAIN);
				PASSWDFOUND(DCE_DOMAIN);
			}
		}
                if (!pw)
		{
                        pw = getpwnam_compat(nam);
			if (shadow)
				shadow_chk(pw);
		}
        }

	if (!keepopen)
		endpwent();

        return pw;
}

/*
 * NAME: getpwnam
 * 
 * FUNCTION: Calls _getpwnam_shadow with shadow checking turned on.
 * 	     Given a user name, this routine will attempt to resolve
 *	     it to a passwd structure.  The routine has several possible
 *	     callout routines.  If the user authenticated via DCE, this
 *	     will be the primary callout.  Otherwise we will use
 *	     the normal name resolution routines, herein named 
 *	     getpwnam_compat().
 *
 * 	     See design 7673.  AIX/DCE Security Enablement for a rationale
 *	     on this approach.
 *
 * RETURNS:  Null 		- Name resolution failed.
 *	     struct passwd *	- User's passwd structure.
 */
struct passwd *
getpwnam(const char *nam)
{
	return(_getpwnam_shadow(nam,1));
}


/*
 * NAME: getpwnam_compat
 *
 * FUNCTION: Normal name resolution routine which implements the lookup
 * 	     in the local security database files and NIS if the local
 *	     database is configured with the YP plus and minus semantics.
 *	     There are no calls to shadow_chk() in this routine.
 *
 * Returns:  Null		- Name resolution failed.
 *	     struct passwd *	- User's passwd structure 
 */
static struct passwd *
getpwnam_compat(const char *nam)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *pw;
	char	line[BUFSIZ+1];
	datum key;
	struct flock pwd_lock;
	struct stat sbpw;
	struct stat sbtmp;

	if (_pw == 0)
		return (0);

	if (pwf != NULL)
		_pw_stayopen = 1;

	setpwent();

	if (check_yellow()) {
		if (!pwf)
			return NULL;
		while (fgets(line, BUFSIZ, pwf) != NULL) {
			if ((pw = interpret(line, strlen(line))) == NULL)
				continue;
			if (matchname(line, &pw, nam)) {
				if (!_pw_stayopen)
					endpwent();
				return pw;
			}
		}
		if (!_pw_stayopen)
			endpwent();
		return NULL;
	}
	/* otherwise, use the dbm method */
	if (stat (PASSWD, &sbpw) == 0 && stat (SPASSWD, &sbtmp) == 0) {
		if (sbpw.st_mtime < sbtmp.st_mtime)
			sbpw = sbtmp;
	}
	if (_pw_db == (DBM * )0 && 
	    (_pw_db = dbm_open(PASSWD, O_RDONLY, 0)) == (DBM * )0) {

		/* Reset errno so this doesn't return to caller */
		errno = 0;    
oldcode:
		setpwent();

		/*
		 * Since this is a loop looking for the correct user
		 * entry, we will use the shorter method of not going
		 * after the shadow password directly.  Once we have found
		 * the password, we will check the lookaside file for the
		 * password.
		 */
		while ((pw = _getpwent()) && strcmp(nam, pw->pw_name))
			;

		if (!_pw_stayopen)
			endpwent();
		return (pw);
	}
        /*
         * Make sure we stat the .pag file of the database.
         * the .dir file probably won't change unless some
         * index information changes.
         */
        fstat (dbm_pagfno (_pw_db), &sbtmp);
	if (sbpw.st_mtime > sbtmp.st_mtime)
		goto oldcode;

        /*
         * Set a read-only shared non blocking lock on the passwd.dir file.  
	 * This will prevent anyone else from opening the passwd.dir file 
	 * for update during our fetch.  
         */
        pwd_lock.l_type = F_RDLCK;
        pwd_lock.l_whence = SEEK_SET;
        pwd_lock.l_start = 0L;
        pwd_lock.l_len = 0;
        if (fcntl (dbm_dirfno (_pw_db), F_SETLK, &pwd_lock)) {
		dbm_close(_pw_db);
		_pw_db = (DBM * )0;
		goto oldcode;
	}

	key.dptr = nam;
	key.dsize = strlen(nam);
	pw = fetchpw(key);

	/*
         * Give up the lock that was just acquired.  We are done with
         * the DBM files for now.
         */
        pwd_lock.l_type = F_UNLCK;
        (void) fcntl(dbm_dirfno (_pw_db), F_SETLK, &pwd_lock);

	if (!_pw_stayopen)
		endpwent();
	return (pw);
}


/*
 * NAME: getpwuid
 * 
 * FUNCTION: Given a user id, this routine will attempt to resolve
 *	     it to a passwd structure.  The routine has several possible
 *	     callout routines.  If the user authenticated via DCE, this
 *	     will be the primary callout.  Otherwise we will use
 *	     the normal name resolution routines, herein named 
 *	     getpwuid_compat() 
 *
 * 	     See design 7673.  AIX/DCE Security Enablement for a rationale
 *	     on this approach.
 *
 * RETURNS:  Null 		- Name resolution failed.
 *	     struct passwd *	- User's passwd structure.
 */
struct passwd *
getpwuid(uid_t uid)
{
        register struct _pwjunk *_pw = _pwjunk();
        struct passwd *pw = (struct passwd *)NULL;
	int keepopen = 0;

        if (_pw == 0)
                return (0);

	if (pwf != NULL)
		keepopen = 1;

	setpwent();	/* Required for shadow_chk() */

        /*
         * If my primary authentication was something other than DCE
  	 * (meaning local or NIS) then I will check the local files (and NIS) 
	 * first.  If this resolution does not return a valid entry then I 
	 * will attempt to load dce and call the getpwuid_dce() routine.
         *
         * Otherwise my authstate environment variable was DCE so I
         * should check DCE and if that fails check the local files.
         */
        if (authstate != DCE)
        {
                if ((pw = getpwuid_compat(uid)) == (struct passwd *)NULL)
		{
                        if (check_dce()) 
			{
				if ((pw = getpwuid_dce(uid)) != 
				    (struct passwd *)NULL)
				{
					USERFOUND(DCE_DOMAIN);
					PASSWDFOUND(DCE_DOMAIN);
				}
			}
		}
		else
			shadow_chk(pw);
        }
        else
        {
                if (check_dce())
		{
                        if((pw = getpwuid_dce(uid)) != (struct passwd *)NULL)
			{
				USERFOUND(DCE_DOMAIN);
				PASSWDFOUND(DCE_DOMAIN);
			}
		}
                if (!pw)
		{
                        pw = getpwuid_compat(uid);
			shadow_chk(pw);
		}
        }

	if (!keepopen)
		endpwent();

        return pw;
}

/*
 * NAME: getpwuid_compat
 *
 * FUNCTION: Normal name resolution routine which implements the lookup
 * 	     in the local security database files and NIS if the local
 *	     database is configured with the YP plus and minus semantics.
 *	     There are no calls to shadow_chk() in this routine.
 *
 * Returns:  Null		- Name resolution failed.
 *	     struct passwd *	- User's passwd structure 
 */
static struct passwd *
getpwuid_compat(uid_t uid)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *pw;
	char	line[BUFSIZ+1];
	datum key;
	struct flock pwd_lock;
	struct stat sbpw;
	struct stat sbtmp;

	if (_pw == 0)
		return (0);

	if (pwf != NULL)
		_pw_stayopen = 1;

	setpwent();
	if (check_yellow()) {
                struct passwd *pwFromYellow = 0;
                char           nameFromYellow[PW_NAMELEN+1];
                char           initYellow = 1;

                if (!pwf)
                        return NULL;

                while (fgets(line, BUFSIZ, pwf) != NULL) {

                        if ((pw = interpret(line, strlen(line))) == NULL)
                                continue;

                        /**********************************************
                         * PERFORMANCE ENHANCEMENT
                         *   skip checking entries like +username where
                         *   username != nameFromYellow
                         **********************************************/
                        if ((line[0] == '+' || line[0] == '-') &&
                            line[1]!='@' && strcmp(pw->pw_name,"+")) {
                                if (initYellow) {
                                        /*******************************************************
                                         * get a probable name from the passwd.byuid map
                                         * and avoid linearly traversing passwd.byname
                                         * if there are alot of +username entries in /etc/passwd
                                         *******************************************************/
                                        initYellow = 0;
                                        if (pwFromYellow=getuidfromyellow(uid, NULL))
                                                strcpy(nameFromYellow,pwFromYellow->pw_name);
                                        else
                                                nameFromYellow[0]='\0';
                                }

                                if (nameFromYellow[0] && strcmp(nameFromYellow,pw->pw_name+1))
                                        continue;
                        }

                        if (matchuid(line, &pw, uid)) {
                                if (!_pw_stayopen)
                                        endpwent();
                                return pw;
                        }
                }
                if (!_pw_stayopen)
                        endpwent();
                return NULL;
	}

	/* otherwise, use the dbm method */
	if (stat (PASSWD, &sbpw) == 0 && stat (SPASSWD, &sbtmp) == 0) {
		if (sbpw.st_mtime < sbtmp.st_mtime)
			sbpw = sbtmp;
	}
	if (_pw_db == (DBM * )0 && 
	    (_pw_db = dbm_open(PASSWD, O_RDONLY, 0)) == (DBM * )0) {

		/* Reset errno so this doesn't return to caller */
		errno = 0;    
oldcode:
		setpwent();

		/*
		 * Since this is a loop looking for the correct user
		 * entry, we will use the shorter method of not going
		 * after the shadow password directly.  Once we have found
		 * the password, we will check the lookaside file for the
		 * password.
		 */
		while ((pw = _getpwent()) && pw->pw_uid != uid)
			;

		if (!_pw_stayopen)
			endpwent();
		return (pw);
	}
        /*
         * Make sure we stat the .pag file of the database.
         * the .dir file probably won't change unless some
         * index information changes.
         */
        fstat (dbm_pagfno (_pw_db), &sbtmp);
	if (sbpw.st_mtime > sbtmp.st_mtime)
		goto oldcode;
 
        /*
         * Set a read-only shared non blocking lock on the passwd.dir file.
         * This will prevent anyone else from opening the passwd.dir file
         * for update during our fetch.
         */
        pwd_lock.l_type = F_RDLCK;
        pwd_lock.l_whence = SEEK_SET;
        pwd_lock.l_start = 0L;
        pwd_lock.l_len = 0;
        if (fcntl (dbm_dirfno (_pw_db), F_SETLK, &pwd_lock)) {
		dbm_close(_pw_db);
		_pw_db = (DBM * )0;
		goto oldcode;
	}

	key.dptr = (char *) & uid;
	key.dsize = sizeof uid;
	pw = fetchpw(key);

	/*
         * Give up the lock that was just acquired.  We are done with
         * the DBM files for now.
         */
        pwd_lock.l_type = F_UNLCK;
        (void) fcntl(dbm_dirfno (_pw_db), F_SETLK, &pwd_lock);

	if (!_pw_stayopen)
		endpwent();
	return (pw);
}


void
setpwent()
{
	register struct  _pwjunk *_pw = _pwjunk();
	register int	flags;

	if (_pw == 0)
		return;

	if (pwf == NULL)
		pwf = pw_open(PASSWD, "r");
	else
		rewind(pwf);

	if (spwf == NULL)
	{
	  	/* 
		 * Check accessability of shadow password file.
		 * Unnecessary open attempts flood the audit trail.
		 */
                if (!accessx(SPASSWD,R_ACC,ACC_SELF))
			spwf = pw_open(SPASSWD, "r");
	}
	else
		rewind(spwf);

	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}


void
endpwent()
{
	register struct  _pwjunk *_pw = _pwjunk();

	if (_pw == 0)
		return;
	if (pwf != NULL) {
		(void) pw_close(pwf);
		pwf = NULL;
	}
	if (spwf != NULL) {
		(void) pw_close(spwf);
		spwf = NULL;
	}
	if (_pw_db != (DBM * )0) {
		dbm_close(_pw_db);
		_pw_db = (DBM * )0;
	}
	_pw_stayopen = 0;

	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
	endnetgrent();
}


int
setpwfile(char *file)
{
	register struct  _pwjunk *_pw = _pwjunk();

	if (_pw == 0)
		return (0);
	PASSWD = file;
	return (1);
}


static char	*
pwskip(register char *p)
{
	while (*p && *p != ':' && *p != '\n')
		++p;
	if (*p == '\n')
		*p = '\0';
	else if (*p != '\0')
		*p++ = '\0';
	return(p);
}


/*
 * NAME: getpwent()
 *
 * FUNCTION: Normal getpwent() with lookup for the encrypted password
 *	     in /etc/security/passwd.
 * 
 * RETURNS:  struct passwd * : User's password struct.  If a privileged
 *			       user invokes this routine, the password
 *			       structure will also contain the encrypted
 *			       password. 
 */
struct passwd *
getpwent()
{
	register struct  _pwjunk *_pw = _pwjunk();
        struct passwd *pw;

        if ((pw = _getpwent()) != (struct passwd *)NULL) {
		/* 
		 * Check accessability of shadow password file.
		 * Unnecessary open attempts flood the audit trail.
		 */
		if (spwf == NULL && !accessx(SPASSWD,R_ACC,ACC_SELF))
			spwf = pw_open(SPASSWD, "r");

                shadow_chk(pw);
	}

        return(pw);
}

/*
 * NAME: _getpwent()
 *
 * FUNCTION: Normal getpwent() however no lookup for the encrypted password
 *	     in /etc/security/passwd is done.  This is sometimes useful for
 *	     commands when no authentication is being performed, just name
 *	     resolution.  (finger and ksh are examples of this).
 * 
 * RETURNS:  struct passwd * : User's password struct, but no encrypted
 *	     password.
 */
struct passwd *
_getpwent()
{
	register struct  _pwjunk *_pw = _pwjunk();
	char	line1[BUFSIZ+1];
	static struct passwd *savepw;
	struct passwd *pw;
	char	*user;
	char	*mach;
	char	*dom;
	register int	flags;

	if (_pw == 0)
		return (0);

	if (pwf == NULL )
		if ((pwf = pw_open(PASSWD, "r")) == NULL)
			return ((struct passwd *)NULL);

	for (; ; ) {
		if (yp) {
			pw = interpretwithsave(yp, yplen, savepw);
			free(yp);
			if (pw == NULL)
				return(NULL);
			getnextfromyellow();
			if (!onminuslist(pw)) {
				return(pw);
			}
		} else if (getnetgrent(&mach, &user, &dom)) {
			if (user) {
				pw = getnamefromyellow(user, savepw);
				if (pw != NULL && !onminuslist(pw)) {
					return(pw);
				}
			}
		} else {
			endnetgrent();
			if (fgets(line1, BUFSIZ, pwf) == NULL)  {
				return(NULL);
			}
			if ((pw = interpret(line1, strlen(line1))) == NULL)
				return(NULL);
			switch (line1[0]) {
			case '+':
				if (!bind_to_yp())
					continue;
				if (strcmp(pw->pw_name, "+") == 0) {
					getfirstfromyellow();
					savepw = save(pw);
				} else if (line1[1] == '@') {
					savepw = save(pw);

					if (innetgr(pw->pw_name + 2, 
					    (char *)NULL, "*", domain)) {
						/* 
						 * include the whole yp database
						 */
						getfirstfromyellow();
					} else {
						setnetgrent(pw->pw_name + 2);
					}
				} else {
					/* 
					 * else look up entry in yellow pages
				 	 */
					savepw = save(pw);
					pw = getnamefromyellow(pw->pw_name + 1,
						savepw);

					if (pw != NULL && !onminuslist(pw)) {
						return(pw);
					}
				}
				break;
			case '-':
				if (!bind_to_yp()) /* needed ??? */
					continue;
				if (line1[1] == '@') {

					/* Everybody was subtracted */
					if (innetgr(pw->pw_name + 2, 
					    (char *)NULL, "*", domain))
						return(NULL);

					setnetgrent(pw->pw_name + 2);
					while (getnetgrent(&mach, &user, &dom))
					{
						if (user)
							addtominuslist(user);
					}
					endnetgrent();
				} else {
					addtominuslist(pw->pw_name + 1);
				}
				break;
			default:
				if (!onminuslist(pw)) {
					return(pw);
				}
				break;
			}
		}
	}
}


static int
matchname(char *line1, struct passwd **pwp, char *name)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *savepw;
	struct passwd *pw = *pwp;

	if (_pw == 0)
		return (0);
	switch (line1[0]) {
	case '+':
		if (!bind_to_yp())
			break;
		if (strcmp(pw->pw_name, "+") == 0) {
			savepw = save(pw);
			pw = getnamefromyellow(name, savepw);
			if (pw) {
				*pwp = pw;
				return 1;
			} else
				return 0;
		}
		if (line1[1] == '@') {
			if (innetgr(pw->pw_name + 2, (char *) NULL, name,
			     domain)) {
				savepw = save(pw);
				pw = getnamefromyellow(name, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				}
			}
			return 0;
		}
		if (strcmp(pw->pw_name + 1, name) == 0) {

			savepw = save(pw);
			pw = getnamefromyellow(pw->pw_name + 1, savepw);
			if (pw) {
				*pwp = pw;
				return 1;
			} else
				return 0;
		}
		break;
	case '-':
		if (!bind_to_yp())	/* needed ??? */
			break;
		if (line1[1] == '@') {
			if (innetgr(pw->pw_name + 2, (char *) NULL, name,
			     domain)) {
				*pwp = NULL;
				return 1;
			}
		} else if (strcmp(pw->pw_name + 1, name) == 0) {
			*pwp = NULL;
			return 1;
		}
		break;
	default:
		if (strcmp(pw->pw_name, name) == 0)
			return 1;
	}
	return 0;
}


static int
matchuid(char *line1, struct passwd **pwp, uid_t uid)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *savepw;
	struct passwd *pw = *pwp;
	char	group[256];

	if (_pw == 0)
		return (0);
	switch (line1[0]) {
	case '+':
		if (!bind_to_yp())
			break;
		if (strcmp(pw->pw_name, "+") == 0) {
			savepw = save(pw);
			pw = getuidfromyellow(uid, savepw);
			if (pw) {
				*pwp = pw;
				return 1;
			} else {
				return 0;
			}
		}

		if (line1[1] == '@') {
			(void) strcpy(group, pw->pw_name + 2);
			savepw = save(pw);
			pw = getuidfromyellow(uid, savepw);
			if (pw && innetgr(group, (char *) NULL, pw->pw_name,
			    domain)) {
				*pwp = pw;
				return 1;
			} else {
				return 0;
			}
		}

		savepw = save(pw);
		pw = getnamefromyellow(pw->pw_name + 1, savepw);
		if (pw && pw->pw_uid == uid) {
			*pwp = pw;
			return 1;
		} else
			return 0;

	case '-':
		if (!bind_to_yp())
			break;
		if (line1[1] == '@') {
			(void) strcpy(group, pw->pw_name + 2);
			pw = getuidfromyellow(uid, &NULLPW);
			if (pw && innetgr(group, (char *) NULL, pw->pw_name,
			     domain)) {
				*pwp = NULL;
				return 1;
			}
		} else if (uid == uidof(pw->pw_name + 1)) {
			*pwp = NULL;
			return 1;
		}
		break;

	default:
		if (pw->pw_uid == uid)
			return 1;
	}
	return 0;
}


static int
uidof(char *name)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *pw;

	if (_pw == 0)
		return (0);
	pw = getnamefromyellow(name, &NULLPW);
	if (pw)
		return pw->pw_uid;
	else
		return MAXINT;
}


static int
getnextfromyellow()
{
	register struct  _pwjunk *_pw = _pwjunk();
	int	reason;
	char	*key = NULL;
	int	keylen;

	if (_pw == 0)
		return;

	/* Return if we should not report YP users */
	if (!REPORT(NIS_MASK))
	{
		yp = NULL;
		return 0;
	}

#ifdef YP
	reason = yp_next(domain, "passwd.byname", oldyp, oldyplen, &key, 
			&keylen, &yp, &yplen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		yp = NULL;
	}
#endif
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}


static int
getfirstfromyellow()
{
	register struct  _pwjunk *_pw = _pwjunk();
	int	reason;
	char	*key = NULL;
	int	keylen;

	if (_pw == 0)
		return;

	/* Return if we should not report YP users */
	if (!REPORT(NIS_MASK))
	{
		yp = NULL;
		return 0;
	}

#ifdef YP
	reason =  yp_first(domain, "passwd.byname", &key, &keylen, &yp,
	    		&yplen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
		yp = NULL;
	}
#endif
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}


static struct passwd *
getnamefromyellow(char *name, struct passwd *savepw)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *pw;
	int	reason;
	char	*val;
	int	vallen;

	if (_pw == 0)
		return (0);

	/* Return if we should not report YP users */
	if (!REPORT(NIS_MASK))
		return((struct passwd *)NULL);

#ifdef YP
	reason = yp_match(domain, "passwd.byname", name, strlen(name),
			&val, &vallen);

	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return NULL;
	} else
#endif
	{
		/*
 		 * We know the user has some info coming from a yp server
		 */
		USERFOUND(NIS_DOMAIN);
		pw = interpret(val, vallen);
		free(val);
		if (pw == NULL)
			return NULL;
		if (savepw->pw_passwd && *savepw->pw_passwd)
			pw->pw_passwd =  savepw->pw_passwd;
		else
			PASSWDFOUND(NIS_DOMAIN);/* We know the user's passwd  */
						/* is coming from a yp server */

		if (savepw->pw_gecos && *savepw->pw_gecos)
			pw->pw_gecos = savepw->pw_gecos;
		if (savepw->pw_dir && *savepw->pw_dir)
			pw->pw_dir = savepw->pw_dir;
		if (savepw->pw_shell && *savepw->pw_shell)
			pw->pw_shell = savepw->pw_shell;
		return pw;
	}
}


static struct passwd *
getuidfromyellow(uid_t uid, struct passwd *savepw)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct passwd *pw;
	int	reason;
	char	*val;
	int	vallen;
	char	uidstr[20];

	if (_pw == 0)
		return (0);

	/* Return if we should not report YP users */
	if (!REPORT(NIS_MASK))
		return((struct passwd *)NULL);

	(void) sprintf(uidstr, "%d", uid);
#ifdef YP
	reason = yp_match(domain, "passwd.byuid", uidstr, strlen(uidstr),
	    		&val, &vallen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return NULL;
	} else
#endif
	{
		/*
 		 * We know the user has some info coming from a yp server
		 */
		USERFOUND(NIS_DOMAIN);
		pw = interpret(val, vallen);
		free(val);
		if (pw == NULL)
			return NULL;

		if (savepw)
		{
		  if (savepw->pw_passwd && *savepw->pw_passwd)
			  pw->pw_passwd =  savepw->pw_passwd;
		  else
			  PASSWDFOUND(NIS_DOMAIN);

		  if (savepw->pw_gecos && *savepw->pw_gecos)
			  pw->pw_gecos = savepw->pw_gecos;
		  if (savepw->pw_dir && *savepw->pw_dir)
			  pw->pw_dir = savepw->pw_dir;
		  if (savepw->pw_shell && *savepw->pw_shell)
			  pw->pw_shell = savepw->pw_shell;
		}
		return pw;
	}
}


static struct passwd *
interpretwithsave(char *val, int len, struct passwd *savepw)
{
	/* differs from interpret in that it also picks up the password */

	struct passwd *pw;

	if ((pw = interpret(val, len)) == NULL)
		return NULL;
	if (savepw->pw_passwd && *savepw->pw_passwd)
		pw->pw_passwd =  savepw->pw_passwd;
	if (savepw->pw_gecos && *savepw->pw_gecos)
		pw->pw_gecos = savepw->pw_gecos;
	if (savepw->pw_dir && *savepw->pw_dir)
		pw->pw_dir = savepw->pw_dir;
	if (savepw->pw_shell && *savepw->pw_shell)
		pw->pw_shell = savepw->pw_shell;
	return pw;
}


/* 
 * Some non aix nis servers have passwd aging info in the passwd field
 * in the /etc/passwd file (or nis passwd map entry).  This info comes
 * after a comma.  We ignore it with this routine.
 */
static void
stripaftercomma(char *pwd)
{
	for (; *pwd; *pwd++)
		if (*pwd == ',') {
			*pwd = (char)0;
			break;
		}
}


static struct passwd *
interpret(char *val, int len)
{
	register struct  _pwjunk *_pw = _pwjunk();
	register char	*p;
	char	*end;
	unsigned long	x;
	register int	ypentry;

	if (_pw == 0)
		return (0);
	(void) strncpy(interpline, val, len);
	p = interpline;
	interpline[len] = '\n';
	interpline[len+1] = 0;

	/*
 	 * Set "ypentry" if this entry references the Yellow Pages;
	 * if so, null UIDs and GIDs are allowed (because they will be
	 * filled in from the matching Yellow Pages entry).
	 */
	ypentry = (*p == '+' || *p == '-' );

	interppasswd.pw_name = p;
	p = pwskip(p);
	interppasswd.pw_passwd = p;
	p = pwskip(p);
	stripaftercomma(interppasswd.pw_passwd);

	if (*p == ':' && !ypentry)
		/* check for non-null uid */
		return (NULL);
	x = strtoul(p, &end, 10);
	p = end;

	/* check for numeric value - must have stopped on the colon */
	if (*p++ != ':' && !ypentry)
		return (NULL);

	interppasswd.pw_uid = x;

	/* check for non-null gid */
	if (*p == ':' && !ypentry)
		return (NULL);

	x = strtoul(p, &end, 10);
	p = end;

	/* check for numeric value - must have stopped on the colon */
	if (*p++ != ':' && !ypentry)
		return (NULL);

	interppasswd.pw_gid = x;
	interppasswd.pw_gecos = p;
	p = pwskip(p);
	interppasswd.pw_dir = p;
	p = pwskip(p);
	interppasswd.pw_shell = p;
	while (*p && *p != '\n') 
		p++;
	*p = '\0';

	return(&interppasswd);
}


static int
freeminuslist() 
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct list *ls;

	if (_pw == 0)
		return;
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		free(ls->name);
		free((char *) ls);
	}
	minuslist = NULL;
}


static int
addtominuslist(char *name)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct list *ls;
	char	*buf;

	if (_pw == 0)
		return;
	ls = (struct list *) malloc(sizeof(struct list ));
	buf = malloc((unsigned) strlen(name) + 1);
	(void) strcpy(buf, name);
	ls->name = buf;
	ls->nxt = minuslist;
	minuslist = ls;
}


/* 
 * Save away psswd, gecos, dir and shell fields, which are the only
 * ones which can be specified in a local + entry to override the
 * value in the yellow pages
 */
static struct passwd *
save(struct passwd *pw)
{
	static struct passwd *sv;

	/* free up stuff from last call */
	if (sv) {
		free(sv->pw_passwd);
		free(sv->pw_gecos);
		free(sv->pw_dir);
		free(sv->pw_shell);
		free((char *) sv);
	}
	sv = (struct passwd *) malloc(sizeof(struct passwd ));

	sv->pw_passwd = malloc((unsigned) strlen(pw->pw_passwd) + 1);
	(void) strcpy(sv->pw_passwd, pw->pw_passwd);

	sv->pw_gecos = malloc((unsigned) strlen(pw->pw_gecos) + 1);
	(void) strcpy(sv->pw_gecos, pw->pw_gecos);

	sv->pw_dir = malloc((unsigned) strlen(pw->pw_dir) + 1);
	(void) strcpy(sv->pw_dir, pw->pw_dir);

	sv->pw_shell = malloc((unsigned) strlen(pw->pw_shell) + 1);
	(void) strcpy(sv->pw_shell, pw->pw_shell);

	return sv;
}


static int
onminuslist(struct passwd *pw)
{
	register struct  _pwjunk *_pw = _pwjunk();
	struct list *ls;
	register char	*nm;

	if (_pw == 0)
		return (0);
	nm = pw->pw_name;
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		if (strcmp(ls->name, nm) == 0) {
			return(1);
		}
	}
	return(0);
}


static int	
stanzmat(char *sz, char *s)
{
	/* matches a stanza colon line against a username (no colon)
	 * returns 0 if they match, -1 if not
	 */

	while (*sz == *s && *sz && *s && *sz != ':') {
		sz++;
		s++;
	}
	if (*sz == ':' && *s == '\0')
		return 0;
	return - 1;
}


static char	*
shadow_pass(char *u)
{
#	define iswhite(c) ((c) == '\t' || (c) == ' ' || (c) == '\n')

	register struct  _pwjunk *_pw = _pwjunk();
	char	line[BUFSIZ+1];
	long	start;
	long	pos = -1L;		/* pos is kept at -1 until we wrap */

	if (spwf == NULL)
		return NULL;

	start = ftell(spwf);
	do
	{
		if (fgets(line, BUFSIZ, spwf) != NULL) {
			if (!iswhite(*line) && stanzmat(line, u) == 0) {
				/* get the password from the stanza */
				for (; ; ) {
					static char	pass[MAX_PASS+1];
					char	*l, *p;

					l = line;
					p = pass;
					if (fgets(line, BUFSIZ, spwf) == NULL)
						break;

					if (*l == '*' || *l == '#')
						continue;
					if (!isspace(*l))
						break;

					while (isspace(*l))
						l++;
					if (strncmp(l, "password", 8) == 0) {
						l += 8;
						while (isspace(*l))
							l++;
						if (*l++ != '=')
							continue;
						while (isspace(*l))
							l++;
						while (*l && !isspace(*l))
							*p++ = *l++;
						*p = '\0';
						return pass;
					}
				}
				return NULL;
			}
		} else
		{
			if (pos != -1L)
				return (NULL);
			fseek(spwf, 0L, 0);
			pos = 0L;
		}
		if (pos != -1L)
			pos = ftell(spwf);
	} while (pos < start);
	return (NULL);
}


/*
 * shadow_chk -	check for shadowed password entries
 *
 * Input:
 *	struct passwd *p - User's passwd struct from colon /etc/passwd file
 *
 * Returns:
 *	nothing
 *
 * Description:
 *	If shadowing not enabled for this entry, the passwd struct remains
 *	unchanged.  If the shadow exists then the encrypted form of the 
 *	password replaces the pw_passwd entry.  Otherwise "*", is returned
 *	in the pw_passwd entry, which cannot match the encrypted text of 
 *	any password string.
 */
static void	
shadow_chk(register struct passwd *p)
{ 	
	char	*pass;
	static char	buf[MAX_PASS+1];
	char	*name2use;

	/* 
	 * The "!" indicates a shadow password file is being used.
	 */
	if (p) {
		if (strcmp(p->pw_passwd, "!") == 0)	/* shadowed entry? */
		{
			/* if this is a particular user's yp entry, */
			if ((p->pw_name[0] == '+') && (strlen(p->pw_name) > 1) 
			    && (p->pw_name[1] != '@'))
				name2use = &p->pw_name[1]; /* get local data */
			else
				name2use = p->pw_name;

			if ((p->pw_passwd = shadow_pass(name2use)) == NULL)
				p->pw_passwd = "*";	/* no, force NOLOGIN*/
		}
	}
}


/*
 * NAME: ispassrmtnam 
 *
 * FUNCTION: Determines if the user's passwd is defined in the local security 
 *	     database, or is resolved from another name service mechanism.  
 *	     There are specific instances where the user can be resolved from 
 *	     the local passwd file and yet they are a DCE user.  This often
 *	     happens if the DCE registry is replicated locally.  If a user
 *	     is defined locally we still need to check the "registry" variable
 *	     in /etc/security/user to determine if the user is administered
 *	     via DCE.
 *
 * RETURNS:  0	- locally administered password
 *	     1  - remotely administered password
 *	    -1  - user not found in any service
 */
int	
ispassrmtnam(char *nam)
{
	int rc = 0;

	PASSWDFOUND(LOCAL);
	if (getpwnam(nam))
	{
		if (IS_PASSWD_NIS || IS_PASSWD_DCE)
			rc = 1;
		else	/* Check local case "registry" override. */
		{
			char *registry;

			setuserdb(S_READ);
        		/*
         		 * Attempt to retrieve an explicit definition of 
			 * registry from the user table.  Often a DCE registry
			 * is replicated locally, therefore user's look like
			 * they are locally defined.  However, there is a
			 * registry variable in the user database that signals
			 * where the user is administered.  This call of 
			 * course requires privilege.
         		 */
        		if (!getuserattr(nam, S_REGISTRY, &registry, SEC_CHAR))
				if (!strcmp(registry, AUTH_DCE))
					rc = 1;	
			enduserdb();
		}
	}
	else
		rc = -1;

	return(rc);
}


/*
 * NAME: isuserrmtnam 
 *
 * FUNCTION: Determines if the user is defined in the local security database,
 *	     or is resolved from another name service mechanism.  There
 *	     are specific instances where the user can be resolved from the
 *	     local passwd file and yet they are a DCE user.  This often
 *	     happens if the DCE registry is replicated locally.  If a user
 *	     is defined locally we still need to check the "registry" variable
 *	     in /etc/security/user to determine if the user is administered
 *	     via DCE.
 *
 * RETURNS:  0	- locally administered user
 *	     1  - remotely administered user
 *	    -1  - user not found in any service
 */
int	
isuserrmtnam(char *nam)
{
	int rc = 0;

	USERFOUND(LOCAL);	/* Initialize state to local */
	if (getpwnam(nam))
	{
		if (IS_USER_NIS || IS_USER_DCE)
			rc = 1;
		else	/* Check local case "registry" override. */
		{
			char *registry;

			setuserdb(S_READ);
        		/*
         		 * Attempt to retrieve an explicit definition of 
			 * registry from the user table.  Often a DCE registry
			 * is replicated locally, therefore user's look like
			 * they are locally defined.  However, there is a
			 * registry variable in the user database that signals
			 * where the user is administered.  This call of 
			 * course requires privilege.
         		 */
        		if (!getuserattr(nam, S_REGISTRY, &registry, SEC_CHAR))
				if (!strcmp(registry, AUTH_DCE))
					rc = 1;	
			enduserdb();
		}
	}
	else
		rc = -1;

	return(rc);
}


/*
 * NAME: check_dce
 *
 * FUNCTION:  If the name resolution is to report DCE user names, and
 *	      if the DCE module is loaded, or can be loaded then this
 *	      module reports true.
 *
 * Returns:   0 - Do not check DCE name resolution.
 *	      1 - Check DCE name resolution
 */ 
static int
check_dce()
{
        static int loaded;
        register struct _pwjunk *_pw = _pwjunk();

	if (!REPORT(DCE_MASK))
		return(0);

        if (loaded)
		return(loaded);

        /*
	 * Load the security method using the libs.a routine _load_secmethod().
	 * This should either fail or produce a function pointer table 
	 * with name resolution routines.
         */
        if (!_load_secmethod(AUTH_DCE, &dcemethod))
		loaded = TRUE;

        return(loaded);
}

/*
 * NAME: check_yellow
 *
 * FUNCTION:  This is not quite the same as the old usingyellow variable.  
 *	      This function returns true if we want to use yellow pages.  
 *	      It does everything except the bind.  The bind is left until 
 *	      we hit something which forces us to bind.
 *
 * RETURNS:   1 - NIS should be queried.
 *	      0 - NIS should not be queried. 
 */
static int	
check_yellow()
{
	register struct  _pwjunk *_pw = _pwjunk();
#ifdef YP

	if (!REPORT(NIS_MASK))
		return(0);

	if (domain == NULL)
		(void) usingypmap(&domain, NULL);

#endif
	return domain != NULL;
}


static int	
bind_to_yp()
{
	register struct  _pwjunk *_pw = _pwjunk();

#ifdef YP
	if (check_yellow()) {
		usingyellow = !yp_bind(domain);
	}
#endif
	return usingyellow;
}

/*
 * NAME: set_getpwent_remote
 *
 * FUNCTION: Turns on and off remote lookup routines.  This interface
 *	     supports the following lookup paths depending on the "value"
 *	     parameter.
 *
 *	      value	  Path
 *	      -----       ---------------
 *		0    :    Local
 *		1    :	  Local, NIS, DCE
 *		2    :    Local, NIS	  ---> "COMPAT"
 *		3    :    DCE
 *
 * 	     This routine is somewhat odd, to the extent that it is an
 * 	     extension of the routine present in AIX 3.2.  That 
 * 	     routine allowed the values 0 (for local only) and 1 (for
 *	     local and NIS).  The 1 value has been extended with the assumption
 *	     that user's of this routine (of which I can find none) intended
 *	     that all mechanisms would be tried.  At that time we only had
 *	     local and NIS.
 *
 * 	     The routine is useful in AIX 4.1 since DCE wants to support the
 * 	     policy that user names and ids are uniquely defined within
 *	     all domains (ie. no conflicts). 
 */
void
set_getpwent_remote(int value)
{
	switch(value)
	{
		case 0  : _report_remote_entries = SET_LOCAL;
			  break;
		case 1  : _report_remote_entries = SET_ALL;
		 	  break;
		case 2  : _report_remote_entries = SET_LOCAL_NIS; 
			  break;
		case 3  : _report_remote_entries = SET_DCE; 
			  break;
		default : _report_remote_entries = SET_ALL; 
	}
}

/*
 * NAME:  reset_pwjunk_authstate
 *
 * FUNCTION: reset the value of authstate in the _pwjunk static area.  
 *	     This is only necessary after a call to authenticate, to
 *	     ensure the getpw* / getgr* routines bind properly.
 *
 */

void
reset_pwjunk_authstate()
{
	register struct  _pwjunk *_pw = _pwjunk();

	if (_pw != NULL) {
		char	*env;
		authstate = COMPAT;
		env = getenv(AUTH_ENV);
		if (!(env == NULL || env[0] == '\0'))
			if (!strcmp(env, AUTH_DCE))
				authstate = DCE;
	}
	return;
}
