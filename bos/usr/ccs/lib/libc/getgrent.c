static char sccsid[] = "@(#)10        1.31  src/bos/usr/ccs/lib/libc/getgrent.c, libcs, bos41J, 9511A_all 3/6/95 14:18:30";
/*
 *   COMPONENT_NAME: LIBCS
 *
 *   FUNCTIONS: REPORT
 *		addtogrset
 *		addtominuslist
 *		bind_to_yp
 *		check_dce
 *		check_yellow
 *		endgrent
 *		fgetgrent
 *		freeminuslist
 *		getfirstfromyellow
 *		getgidfromyellow
 *		getgrent
 *		getgrgid
 *		getgrgid_compat
 *		getgrnam
 *		getgrnam_compat
 *		getgroupsbyuser
 *		getgrset
 *		getgrset_compat
 *		getnamefromyellow
 *		getnextfromyellow
 *		gidof
 *		grskip
 *		interpret
 *		interpretwithsave
 *		matchgid
 *		matchname
 *		onminuslist
 *		reset_grjunk_authstate
 *		save
 *		set_getgrent_remote
 *		setgrent
 *		setgrjunk
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
#include <grp.h>
#include <pwd.h>
#include <usersec.h>
#include <rpcsvc/ypclnt.h>

/*
 * Prototypes for externally used routines.
 */
extern unsigned long	strtoul();
extern int		strlen();
extern int		strcmp();
extern char		*strcpy();
extern char		*calloc();
extern char		*malloc();


/*
 * Prototypes for all routines defined within this file
 */
void 			set_getgrent_remote(int);
void			reset_grjunk_authstate();
struct group * 		getgrgid(gid_t);
struct group * 		getgrnam(const char *);
char * 			getgrset(const char *);
int 			getgroupsbyuser(uid_t, struct setlist *, int *);
int 			addtogrset(char *,struct setlist *,int *,struct group *);
struct group * 		getgrent(void);
void 			setgrent();
void 			endgrent();
struct group * 		fgetgrent(FILE *);

static struct _grjunk * setgrjunk();
static struct group * 	getgrgid_compat(gid_t);
static struct group * 	getgrnam_compat(const char *);
static char * 		getgrset_compat(const char *);
static char * 		grskip(char *, int);
static struct group * 	interpret(char *, int);
static int 		freeminuslist();
static struct group * 	interpretwithsave(char *, int, struct group *);
static int 		onminuslist(struct group *);
static int 		getnextfromyellow();
static int		getfirstfromyellow();
static struct group * 	getnamefromyellow(char *, struct group *);
static int 		addtominuslist(char *);
static struct group * 	save(struct group *);
static int 		matchname(char *, struct group **, char *);
static int 		matchgid(char *, struct group **, gid_t);
static gid_t 		gidof(char *);
static struct group * 	getgidfromyellow(int, struct group *);
static int 		check_dce();
static int		check_yellow();
static int		bind_to_yp();


#define MAXINT 0x7fffffff;
#define	MAXGRP	2000

/* 
 * In GRPLINLEN the 14 represents the ":!:GID LENGTH:" in the group file. 
 */
#define GRPLINLEN	(L_cuserid) + ((L_cuserid + 1) * MAXGRP) + 14

static struct  _grjunk
{
	char	*_domain;
	struct group _NULLGP;
	FILE	* _grf;
	char	*_yp;
	int	_yplen;
	char	*_oldyp;
	int	_oldyplen;
	char	*_agr_mem[MAXGRP];
	enum    {FILES, COMPAT, DCE}
			_authstate;     /* first name resolution mechanism    */
	struct secmethod_table          /* Function pointer table for         */
			_dcemethod;     /* loadable module interface (DCE)    */
	struct list {
		char	*name;
		struct list *nxt;
	} *_minuslist;
	struct group _interpgp;
	char	_interpline[GRPLINLEN+1];
	int	_usingyellow; 
} *__grjunk;

#define	domain		(_gr->_domain)
#define	NULLGP		(_gr->_NULLGP)
#define	grf		(_gr->_grf)
#define	yp		(_gr->_yp)
#define	yplen		(_gr->_yplen)
#define	oldyp		(_gr->_oldyp)
#define	oldyplen 	(_gr->_oldyplen)
#define	agr_mem		(_gr->_agr_mem)
#define	minuslist 	(_gr->_minuslist)
#define	interpgp 	(_gr->_interpgp)
#define	interpline 	(_gr->_interpline)
#define usingyellow	(_gr->_usingyellow)
#define authstate       (_gr->_authstate)
#define dcemethod       (_gr->_dcemethod)
#define getgrgid_dce    (_gr->_dcemethod.method_getgrgid)
#define getgrnam_dce    (_gr->_dcemethod.method_getgrnam)
#define getgrset_dce    (_gr->_dcemethod.method_getgrset)


static char	GROUP[] = "/etc/group";
static int	firsttime = 0;

/*
 * The following macros, defines, and static variable define and retrieve
 * what paths, name resolution should take.  This variable is set in the
 * set_getgrent_remote() routine and checked in the getgrnam() routine
 * and whenever an NIS lookup is about to take place.  Before a lookup
 * in another domain (such as NIS) the programmer should use
 * REPORT(NIS_MASK) to determine if lookups are meant to flow into the NIS
 * domain. (see comments in the set_getgrent_remote() for instructions on
 * valid settings.
 */
#define SET_LOCAL       0x1     /* Local only resolution                */
#define SET_ALL         0x2     /* Resolve through all possible methods */
#define SET_LOCAL_NIS   0x4     /* Resolve through local and NIS only   */
#define SET_DCE         0x8     /* Resolve through DCE onlt             */
static  int             _report_remote_entries = SET_ALL;

#define LOCAL_MASK      0x7
#define NIS_MASK        0x6
#define DCE_MASK        0xA
#define REPORT(a)       (_report_remote_entries & a)

struct setlist {
	char	*name;
	gid_t	gid;
	struct setlist *next;
};

/*
 * NAME: setgrjunk
 *
 * FUNCTION: Initialization routine for static and malloc'd variables.
 *           All routines that attempt to update or query the state
 *           of the internal routines should call this first.
 *
 * RETURNS:  0                 - failure in initialzation
 *           _grjunk structure - correct initialization
 */
static struct  _grjunk *
setgrjunk()
{
	register struct  _grjunk *_gr = __grjunk;

	if (_gr == 0)
	{
		char *env;

		_gr = (struct  _grjunk *)calloc(1, sizeof(*__grjunk));
		if (_gr == 0)
			return(0);

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

		__grjunk = _gr;
	}
	return(__grjunk);
}


/*
 * NAME: getgrgid
 *
 * FUNCTION: Given a group id, this routine will attempt to resolve
 *           it to a group structure.  The routine has several possible
 *           callout routines.  If the user authenticated via DCE, this
 *           will be the primary callout.  Otherwise we will use
 *           the normal name resolution routines, herein named
 *           getgrnam_compat()
 *
 *           See design 7673.  AIX/DCE Security Enablement for a rationale
 *           on this approach.
 *
 * RETURNS:  Null               - Name resolution failed.
 *           struct group *     - group structure.
 */
struct group *
getgrgid(register gid_t gid)
{
        register struct _grjunk *_gr = setgrjunk();
        struct group *gr = (struct group *)NULL;

        if (_gr == 0)
                return (0);

        /*
         * If my primary authentication was something other than DCE
         * (meaning local or NIS) then I will check the local files (and NIS)
         * first.  If this resolution does not return a valid entry then I
         * will attempt to load dce and call the getgrgid_dce() routine.
         *
         * Otherwise my authstate environment variable was DCE so I
         * should check DCE and if that fails check the local files.
         */
        if (authstate != DCE)
        {
                if ((gr = getgrgid_compat(gid)) == (struct group *)NULL)
                {
                        if (check_dce())
                                gr = getgrgid_dce(gid);
                }
        }
        else
        {
                if (check_dce())
                {
                        gr = getgrgid_dce(gid);
                }
                if (!gr)
                        gr = getgrgid_compat(gid);
        }
        return gr;
}

/*
 * NAME: getgrgid_compat
 *
 * FUNCTION: Normal name resolution routine which implements the lookup
 *           in the local security database files and NIS if the local
 *           database is configured with the YP plus and minus semantics.
 *
 * Returns:  Null               - Name resolution failed.
 *           struct group *     - group structure
 */
static struct group *
getgrgid_compat(register gid_t gid)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *p;
	char	line[GRPLINLEN+1];
	char	*s, *t;

	if (_gr == 0)
		return (0);
	setgrent();
	if (!check_yellow()) {
		while ( (p = getgrent()) && p->gr_gid != gid )
			;
		endgrent();
		return(p);
	}
	if (!grf)
		return NULL;
	while ( (s = t = fgets(line, GRPLINLEN + 1, grf)) != NULL) {
		/* 
	         * A15852 fix - Read until Newline for lines longer than BUFSIZ.
		 */
		while (*s != (char) NULL) {
			if ((!(isspace((int)(*s)))) || (*s == '\n')) {
				*t++ = *s;
			}
			*s++;
		}
		if (line[0] == '\n')
			continue;
		/* A15852 fix end. */

		if ((p = interpret(line, strlen(line))) == NULL)
			continue;
		if (matchgid(line, &p, gid)) {
			endgrent();
			return p;
		}
	}
	endgrent();
	return NULL;
}


/*
 * NAME: getgrnam
 *
 * FUNCTION: Given a group name, this routine will attempt to resolve
 *           it to a group structure.  The routine has several possible
 *           callout routines.  If the user authenticated via DCE, this
 *           will be the primary callout.  Otherwise we will use
 *           the normal name resolution routines, herein named
 *           getgrnam_compat()
 *
 *           See design 7673.  AIX/DCE Security Enablement for a rationale
 *           on this approach.
 *
 * RETURNS:  Null               - Name resolution failed.
 *           struct group *     - group structure.
 */
struct group *
getgrnam(const register char *nam)
{
        register struct _grjunk *_gr = setgrjunk();
        struct group *gr = (struct group *)NULL;

        if (_gr == 0)
                return (0);

        /*
         * If my primary authentication was something other than DCE
         * (meaning local or NIS) then I will check the local files (and NIS)
         * first.  If this resolution does not return a valid entry then I
         * will attempt to load dce and call the getgrnam_dce() routine.
         *
         * Otherwise my authstate environment variable was DCE so I
         * should check DCE and if that fails check the local files.
         */
        if (authstate != DCE)
        {
                if ((gr = getgrnam_compat(nam)) == (struct group *)NULL)
                {
                        if (check_dce())
                                gr = getgrnam_dce(nam);
                }
        }
        else
        {
                if (check_dce())
                {
                        gr = getgrnam_dce(nam);
                }
                if (!gr)
                        gr = getgrnam_compat(nam);
        }
        return gr;
}


/*
 * NAME: getgrnam_compat
 *
 * FUNCTION: Normal name resolution routine which implements the lookup
 *           in the local security database files and NIS if the local
 *           database is configured with the YP plus and minus semantics.
 *
 * Returns:  Null               - Name resolution failed.
 *           struct group *     - group structure
 */
static struct group *
getgrnam_compat(const register char *name)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *p;
	char	line[GRPLINLEN+1];
	char	*s, *t;

	if (_gr == 0)
		return NULL;
	setgrent();
	if (!check_yellow()) {
		while ( (p = getgrent()) && strcmp(p->gr_name, name) )
			;
		endgrent();
		return(p);
	}
	if (!grf)
		return NULL;
	while ((s = t = fgets(line, GRPLINLEN + 1, grf)) != NULL) {
		/* 
	         * A15852 fix - Read until Newline for lines longer than BUFSIZ.
		 */
		while (*s != (char) NULL) {
			if ((!(isspace((int)(*s)))) || (*s == '\n')) {
				*t++ = *s;
			}
			*s++;
		}
		if (line[0] == '\n')
			continue;
		/* A15852 fix end. */

		if ((p = interpret(line, strlen(line))) == NULL)
			continue;
		if (matchname(line, &p, name)) {
			endgrent();
			return p;
		}
	}
	endgrent();
	return NULL;
}


/*
 * NAME: getgrset
 *
 * FUNCTION: Given a user name, this routine will attempt to resolve
 *           it to a list of primary and supplementary group ids for this
 * 	     user.  The routine has several possible callout routines.  If 
 *	     the user authenticated via DCE, this will be the primary callout.
 *	     Otherwise we will use the normal name resolution routines, herein 
 *	     named getgrset_compat()
 *
 *           See design 7673.  AIX/DCE Security Enablement for a rationale
 *           on this approach.
 *
 * RETURNS:  Null               - Name resolution failed.
 *           struct group *     - group structure.
 */
char *
getgrset(const char *nam)
{
        register struct _grjunk *_gr = setgrjunk();
        char *gr = (char *)NULL;
	char *env;

        if (_gr == 0)
                return (0);

	/*
	 * This is somewhat unusual that I am rereading my AUTHSTATE
	 * variable here, but this is the only routine where it might
	 * make an explicit difference.  This particular routine is 
	 * relying on getgrent() for it's compat behaviour.  However 
	 * DCE does not implement getgrent() therefore we need to be
	 * very explicit and use the getgrset() routine, which DCE 
	 * does implement. 
	 */
        authstate = COMPAT;
        env = getenv(AUTH_ENV);
        if (!(env == NULL || env[0] == '\0'))
        	if (!strcmp(env, AUTH_DCE))
               		authstate = DCE;
	 
        /*
         * If my primary authentication was something other than DCE
         * (meaning local or NIS) then I will check the local files (and NIS)
         * first.  If this resolution does not return a valid entry then I
         * will attempt to load dce and call the getgrset_dce() routine.
         *
         * Otherwise my authstate environment variable was DCE so I
         * should check DCE and if that fails check the local files.
         */
        if (authstate != DCE)
        {
                if ((gr = getgrset_compat(nam)) == (char *)NULL)
                {
                        if (check_dce())
                                gr = getgrset_dce(nam);
                }
        }
        else
        {
                if (check_dce())
                {
                        gr = getgrset_dce(nam);
                }
                if (!gr)
                        gr = getgrset_compat(nam);
        }
        return gr;
}

/*
 * NAME: getgrset_compat
 *
 * FUNCTION: Normal name resolution routine which implements the lookup
 *           in the local security database files and NIS if the local
 *           database is configured with the YP plus and minus semantics.
 *
 * Returns:  Null               - Name resolution failed.
 *           struct group *     - group structure
 */
static char *
getgrset_compat(register char *user)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *gp, *savegp;
	struct passwd *pwp;
	struct setlist *slp, *tslp;
	char	*buf, **p;
	int	count, endoffile;
	int	i, j;
	gid_t	gid;

	if (_gr == 0)
		return NULL;

	count = 0;

	/* get the users primary group that is specified in the passwd entry
	 * get the group name and put it as the first entry in the buffer.
	 */
	if ((pwp = getpwnam(user)) == NULL)
		return(NULL);

	gid = pwp->pw_gid;
	if ((gp = getgrgid(gid)) == NULL)
		return(NULL);

	if ((slp = (struct setlist *)malloc(sizeof(struct setlist))) == NULL)
		return(NULL);

	slp->next = NULL;
	count = 1;
	if ((slp->name = malloc(strlen(gp->gr_name) + 1)) == NULL)
		return(NULL);
	strcpy(slp->name, gp->gr_name);
	slp->gid = gid;

	/* let's start our search */
	setgrent();
	if (!check_yellow()) {
		while (gp = getgrent()) {
			addtogrset(user, slp, &count, gp);
		}
	} else {

		if (!grf)
			return(NULL);

		endoffile = 0;
		while (!endoffile) {
			int	linefnd;
			char	*s, *p;
			char	line[GRPLINLEN+1];

			if (yp)	 {
				gp = interpretwithsave(yp, yplen, savegp);
				free(yp);
				getnextfromyellow();
				if (gp == NULL)
					continue;
				if (onminuslist(gp))
					continue;
				else {
					addtogrset(user, slp, &count, gp);
					continue;
				}
			} else {
				if (!(s = p = fgets(line,GRPLINLEN + 1,grf))) {
					endoffile = 1;
					continue;
				}
				while (*s != (char) NULL) {
					if ((!(isspace((int)(*s)))) || 
					    (*s == '\n')) {
						*p++ = *s;
					}
					*s++;
				}
				*p = *s;
				if (line[0] == '\n')
					continue;
			}
			if ((gp = interpret(line, strlen(line))) == NULL)
				continue;

			switch (line[0])	 {
			case '+':
				if (!bind_to_yp())
					continue;
				if (strcmp(gp->gr_name, "+") == 0) {
					if (!getgroupsbyuser
					    (pwp->pw_uid, slp, &count)) {
						getfirstfromyellow();
						savegp = save(gp);
					}
					continue;
				}
				/* 
				 * else look up this entry in yellow pages
				 */
				savegp = save(gp);
				gp = getnamefromyellow(gp->gr_name + 1, savegp);
				if (gp == NULL)
					continue;
				else if (onminuslist(gp))
					continue;
				else
					addtogrset(user, slp, &count, gp);
				break;
			case '-':
				addtominuslist(gp->gr_name + 1);
				break;
			default:
				if (onminuslist(gp))
					continue;
				addtogrset(user, slp, &count, gp);
				break;
			}
		}
	}
	endgrent();

	buf = interpline;
	buf[0] = '\0';
	for (tslp = slp; tslp; tslp = tslp->next) {
		int	len;
		sprintf(buf, "%lu", tslp->gid);
		len = strlen(buf);
		if (tslp->next) {
			buf[len] = ',';
			len++;
		}
		buf += len;
		
		/* Storage no longer needed */
		if (tslp->name) free(tslp->name);
		free(tslp);
	}
	return(interpline);
}

int
getgroupsbyuser(uid_t uid, struct setlist *slp, int *count)
{
        register struct _grjunk *_gr = setgrjunk();
	int	reason;
	char	*val, *tval, *tgid;
	int	vallen;
#define	MAXGROUPNAM	256
	char	groupid[MAXGROUPNAM];
	char	usernetname[MAXGROUPNAM];  /* for lack of a better define */
	static int	nomap_netid = FALSE;
	struct setlist *tslp;
	gid_t	gid;

	/* 
	 * If there is a minus list then the system has been configured to 
	 * delete groups from the NIS groups map.  Since the standard search 
	 * is trying to be subverted by this function and the list is going 
	 * to be based on group ids and not the group names this short cut
	 * can not be used.
	 */
	if (nomap_netid == TRUE || minuslist != NULL)
		return(0);

	/* Get the users netname so that we can search netid.byname */
	if (user2netname(usernetname, uid, domain) == 0)
		return(0);

	/* Check the NIS map to see if the user's mapping exists */
	if (reason = yp_match(domain, "netid.byname", 
	    usernetname, strlen(usernetname),
	    &val, &vallen)) {
		if (reason == YPERR_MAP)
			nomap_netid = TRUE;
		return(0);
	} else {
		/* 
		 * Walk through the list of groups and add them to the
		 * set that is already started.  First skip to the beginning
	 	 * of the list.
		 */
		for (tval = val; *tval && *tval != ':'; tval++)
			;
		if (*tval != ':')
			return(0);
		for (tval++; (int)tval <= ((int)val + (int)vallen); ) {

			tgid = groupid;
			while (*tval != ',' && *tval != '\0' && *tval != '\n')
				*tgid++ = *tval++;

			tval++;
			*tgid = '\0';

			gid = atol(groupid);

			/* check to see if the gid is already on the list */
			for (tslp = slp; tslp; slp = tslp, tslp = tslp->next) {
				if (gid == tslp->gid)
					break;
			}
			if (tslp)
				continue;

			if ((tslp = (struct setlist *)malloc
			     (sizeof(struct setlist))) == NULL) {
				free(val);
				return(-1);
			}
			slp->next = tslp;
			tslp->next = NULL;
			tslp->name = NULL;
			tslp->gid = gid;
			(*count)++;
		}
		free(val);
	}
	return(1);
}


int
addtogrset (char *user, struct setlist *slp, int *count, struct group *gp)
{
	char	*group;
	struct setlist *tslp;
	int	i, j, sz, match, offset;

	group = gp->gr_name;

	/* check to see if the group had already been added to the list */
	for (tslp = slp; tslp; tslp = tslp->next) {
		if (!strcmp(tslp->name, group)) {
			return(0);
		}
	}

	/* check group membership to see if the user is part of the group */
	for (i = 0; gp->gr_mem[i]; i++) {
		if (!strcmp(user, gp->gr_mem[i])) {
			for (; slp->next; slp = slp->next)
				;

			if ((tslp = (struct setlist *)malloc
			     (sizeof(struct setlist))) == NULL)
				return(-1);
			if ((tslp->name = malloc(strlen(group) + 1)) == NULL)
				return(-1);

			slp->next = tslp;
			tslp->next = NULL;
			strcpy(tslp->name, group);
			tslp->gid = gp->gr_gid;
			(*count)++;
			return(1);
		}
	}
	return(2);
}


void
setgrent()
{
	register struct  _grjunk *_gr = setgrjunk();
	register int	flags;

	if (_gr == 0)
		return;

	if (!grf) {
		/* open /etc/group for close on exec */
		if ((grf = fopen(GROUP, "r")) == NULL)
			return;

		flags = fcntl (grf->_file, F_GETFD, 0);
		flags |= FD_CLOEXEC;
		fcntl (grf->_file, F_SETFD, flags);
	} else
		rewind(grf);
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}


void
endgrent()
{
	register struct  _grjunk *_gr = setgrjunk();

	if (_gr == 0)
		return;
	if (grf) {
		(void) fclose(grf);
		grf = NULL;
	}
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}


struct group *
fgetgrent(FILE *f)
{
	char	line1[GRPLINLEN+1];
	char	*p, *s;
	int	linefnd;
	struct group *gr;

	/* 
	 * A15852 fix - Read until Newline for lines longer than BUFSIZ.
	 * Be sure to loop thru invalid lines.
	 */
	do {
		linefnd = 0;
		while (linefnd == 0) {
			if ( !(s = p = fgets(line1, GRPLINLEN + 1, f)))
				return(NULL);
			while (*s != (char) NULL) {
				if ((!(isspace((int)(*s)))) || (*s == '\n')) {
					*p++ = *s;
				}
				*s++;
			}
			if (line1[0] != '\n')
				linefnd = 1;
		}
		/* A15852 fix end. */
	} while ((gr = interpret(line1, strlen(line1))) == NULL);

	return (gr);
}


static char	*
grskip(register char *p, register int c)
{
	while (*p && *p != c && *p != '\n') 
		++p;
	if (*p == '\n')
		*p = '\0';
	else if (*p != '\0')
		*p++ = '\0';
	return(p);
}


struct group *
getgrent(void)
{
	register struct  _grjunk *_gr = setgrjunk();
	char	line1[GRPLINLEN+1];
	static struct group *savegp;
	struct group *gp;
	register int	flags;
	char	*p, *s;
	int	linefnd;

	if (_gr == 0)
		return(0);

	if (!grf) {
		/* open /etc/group for close on exec */
		if ((grf = fopen(GROUP, "r")) == NULL)
			return ((struct group *)NULL);

		flags = fcntl (grf->_file, F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl (grf->_file, F_SETFD, flags) < 0)
			return ((struct group *)NULL);
	}
again:
	if (yp) {
		gp = interpretwithsave(yp, yplen, savegp);
		free(yp);
		getnextfromyellow();
		if (gp == NULL)
			goto again;
		if (onminuslist(gp))
			goto again;
		else
			return gp;
	} else
	{
		linefnd = 0;
		while ( linefnd == 0 ) {
			if ( !(s = p = fgets(line1, GRPLINLEN + 1, grf)))
				return(NULL);
			while (*s != (char) NULL) {
				if ((!(isspace((int)(*s)))) || (*s == '\n')) {
					*p++ = *s;
				}
				*s++;
			}
			if (line1[0] != '\n')
				linefnd = 1;
		}
	}

	if ((gp = interpret(line1, strlen(line1))) == NULL)
		goto again;
	switch (line1[0]) {
	case '+':
		if (!check_yellow())
			goto again;
		if (!bind_to_yp())
			goto again;
		if (strcmp(gp->gr_name, "+") == 0) {
			getfirstfromyellow();
			savegp = save(gp);
			goto again;
		}
		/* 
		 * else look up this entry in yellow pages
		 */
		savegp = save(gp);
		gp = getnamefromyellow(gp->gr_name + 1, savegp);
		if (gp == NULL)
			goto again;
		else if (onminuslist(gp))
			goto again;
		else
			return gp;
	case '-':
		if (!check_yellow())
			goto again;
		if (!bind_to_yp())
			goto again;
		addtominuslist(gp->gr_name + 1);
		goto again;
	default:
		if (onminuslist(gp))
			goto again;
		return gp;
	}
}


static struct group *
interpret(char *val, int len)
{
	register struct  _grjunk *_gr = setgrjunk();
	register char	*p, **q;
	char	*end;
	unsigned long	x;
	register int	ypentry;

	if (_gr == 0)
		return (0);
	strncpy(interpline, val, len);
	p = interpline;
	interpline[len] = '\n';
	interpline[len+1] = 0;

	/*
 	 * Set "ypentry" if this entry references the Yellow Pages;
	 * if so, null GIDs are allowed (because they will be filled in
	 * from the matching Yellow Pages entry).
	 */
	ypentry = (*p == '+' || *p == '-');

	interpgp.gr_name = p;
	p = grskip(p, ':');
	interpgp.gr_passwd = p;
	p = grskip(p, ':');

	/* check for non-null gid */
	if (*p == ':' && !ypentry)
		return (NULL);

	x = strtoul(p, &end, 10);
	p = end;

	/* check for numeric value - must have stopped on the colon */
	if (*p++ != ':' && !ypentry)
		return (NULL);

	interpgp.gr_gid = x;
	interpgp.gr_mem = agr_mem;
	(void) grskip(p, '\n');
	q = agr_mem;
	while (*p) {
		if (q < &agr_mem[MAXGRP-1])
			*q++ = p;
		p = grskip(p, ',');
	}
	*q = NULL;
	return(&interpgp);
}


static int
freeminuslist()
{
	register struct  _grjunk *_gr = setgrjunk();
	struct list *ls;

	if (_gr == 0)
		return;
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		free(ls->name);
		free(ls);
	}
	minuslist = NULL;
}


static struct group *
interpretwithsave(char *val, int len, struct group *savegp)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *gp;

	if (_gr == 0)
		return NULL;
	if ((gp = interpret(val, len)) == NULL)
		return NULL;
	if (savegp->gr_passwd && *savegp->gr_passwd)
		gp->gr_passwd =  savegp->gr_passwd;
	if (savegp->gr_mem && *savegp->gr_mem)
		gp->gr_mem = savegp->gr_mem;
	return gp;
}


static int
onminuslist(struct group *gp)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct list *ls;
	register char	*nm;

	if (_gr == 0)
		return 0;
	nm = gp->gr_name;
	for (ls = minuslist; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, nm) == 0)
			return 1;
	return 0;
}


static int
getnextfromyellow()
{
	register struct  _grjunk *_gr = setgrjunk();
	int	reason;
	char	*key = NULL;
	int	keylen;

	if (_gr == 0)
		return;

	/* Return if we should not report NIS */
	if (!REPORT(NIS_MASK))
	{
		yp = NULL;
		return 0;
	}

	if (reason = yp_next(domain, "group.byname", oldyp, oldyplen, &key,
	     &keylen,
	    &yp, &yplen)) {
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}


static int
getfirstfromyellow()
{
	register struct  _grjunk *_gr = setgrjunk();
	int	reason;
	char	*key = NULL;
	int	keylen;

	if (_gr == 0)
		return;

	/* Return if we should not report NIS */
	if (!REPORT(NIS_MASK))
	{
		yp = NULL;
		return 0;
	}

	if (reason = yp_first(domain, "group.byname", &key, &keylen, &yp,
	     &yplen))
		yp = NULL;

	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}


static struct group *
getnamefromyellow(char *name, struct group *savegp)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *gp;
	int	reason;
	char	*val;
	int	vallen;

	if (_gr == 0)
		return NULL;

	/* Return if we should not report NIS users */
	if (!REPORT(NIS_MASK))
		return((struct group *)NULL);

	if (reason = yp_match(domain, "group.byname", name, strlen(name),
	     				&val, &vallen))
		return NULL;
	else
	 {
		gp = interpret(val, vallen);
		free(val);
		if (gp == NULL)
			return NULL;
		if (savegp->gr_passwd && *savegp->gr_passwd)
			gp->gr_passwd =  savegp->gr_passwd;
		if (savegp->gr_mem && *savegp->gr_mem)
			gp->gr_mem = savegp->gr_mem;
		return gp;
	}
}


static int
addtominuslist(char *name)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct list *ls;
	char	*buf;

	if (_gr == 0)
		return;
	ls = (struct list *)malloc(sizeof(struct list ));
	buf = (char *)malloc(strlen(name) + 1);
	(void) strcpy(buf, name);
	ls->name = buf;
	ls->nxt = minuslist;
	minuslist = ls;
}


/* 
 * save away psswd, gr_mem fields, which are the only
 * ones which can be specified in a local + entry to override the
 * value in the yellow pages
 */
static struct group *
save(struct group *gp)
{
	register struct  _grjunk *_gr = setgrjunk();
	static struct group *sv;
	char	*gr_mem[MAXGRP];
	char	**av, **q = 0;
	int	lnth;

	if (_gr == 0)
		return 0;
	/* 
	 * free up stuff from last time around
	 */
	if (sv) {
		for (av = sv->gr_mem; *av != NULL; av++) {
			if (q >= &gr_mem[MAXGRP-1])
				break;
			free(*q);
			q++;
		}
		free(sv->gr_passwd);
		free(sv->gr_mem);
		free(sv);
	}

	sv = (struct group *)malloc(sizeof(struct group ));
	sv->gr_passwd = (char *)malloc(strlen(gp->gr_passwd) + 1);
	(void) strcpy(sv->gr_passwd, gp->gr_passwd);

	q = gr_mem;
	for (av = gp->gr_mem; *av != NULL; av++) {
		if (q >= &gr_mem[MAXGRP-1])
			break;
		*q = (char *)malloc(strlen(*av) + 1);
		(void) strcpy(*q, *av);
		q++;
	}
	*q = 0;
	lnth = (sizeof (char *)) * (q - gr_mem + 1);
	sv->gr_mem = (char **)malloc(lnth);
	bcopy((char *)gr_mem, (char *)sv->gr_mem, lnth);
	return sv;
}


static int
matchname(char *line1, struct group **gpp, char *name)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *savegp;
	struct group *gp = *gpp;

	if (_gr == 0)
		return 0;
	switch (line1[0]) {
	case '+':
		if (!bind_to_yp())
			break;
		if (strcmp(gp->gr_name, "+") == 0) {
			savegp = save(gp);
			gp = getnamefromyellow(name, savegp);
			if (gp) {
				*gpp = gp;
				return 1;
			} else
				return 0;
		}
		if (strcmp(gp->gr_name + 1, name) == 0) {
			savegp = save(gp);
			gp = getnamefromyellow(gp->gr_name + 1, savegp);
			if (gp) {
				*gpp = gp;
				return 1;
			} else
				return 0;
		}
		break;
	case '-':
		if (strcmp(gp->gr_name + 1, name) == 0) {
			*gpp = NULL;
			return 1;
		}
		break;
	default:
		if (strcmp(gp->gr_name, name) == 0)
			return 1;
	}
	return 0;
}


static int
matchgid(char *line1, struct group **gpp, gid_t gid)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *savegp;
	struct group *gp = *gpp;

	if (_gr == 0)
		return 0;
	switch (line1[0]) {
	case '+':
		if (strcmp(gp->gr_name, "+") == 0) {
			savegp = save(gp);
			gp = getgidfromyellow(gid, savegp);
			if (gp) {
				*gpp = gp;
				return 1;
			} else
				return 0;
		}
		savegp = save(gp);
		gp = getnamefromyellow(gp->gr_name + 1, savegp);
		if (gp && gp->gr_gid == gid) {
			*gpp = gp;
			return 1;
		} else
			return 0;
	case '-':
		if (!bind_to_yp())
			break;
		if (gid == gidof(gp->gr_name + 1)) {
			*gpp = NULL;
			return 1;
		}
		break;
	default:
		if (gp->gr_gid == gid)
			return 1;
	}
	return 0;
}


static gid_t
gidof(char *name)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *gp;

	if (_gr == 0)
		return 0;
	gp = getnamefromyellow(name, &NULLGP);
	if (gp)
		return gp->gr_gid;
	else
		return MAXINT;
}


static struct group *
getgidfromyellow(int gid, struct group *savegp)
{
	register struct  _grjunk *_gr = setgrjunk();
	struct group *gp;
	int	reason;
	char	*val;
	int	vallen;
	char	gidstr[20];

	if (_gr == 0)
		return 0;

	/* Return if we should not report NIS users */
	if (!REPORT(NIS_MASK))
		return((struct group *)NULL);

	sprintf(gidstr, "%d", gid);
	if (reason = yp_match(domain, "group.bygid", gidstr,
	    strlen(gidstr), &val, &vallen)) {
		return NULL;
	} else
	 {
		gp = interpret(val, vallen);
		free(val);
		if (gp == NULL)
			return NULL;
		if (savegp->gr_passwd && *savegp->gr_passwd)
			gp->gr_passwd =  savegp->gr_passwd;
		if (savegp->gr_mem && *savegp->gr_mem)
			gp->gr_mem = savegp->gr_mem;
		return gp;
	}
}


/*
 * NAME: check_dce
 *
 * FUNCTION:  If the name resolution is to report DCE user names, and
 *            if the DCE module is loaded, or can be loaded then this
 *            module reports true.
 *
 * Returns:   0 - Do not check DCE name resolution.
 *            1 - Check DCE name resolution
 */
static int
check_dce()
{
        static int loaded;
        register struct _grjunk *_gr = setgrjunk();

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
 *            This function returns true if we want to use yellow pages.
 *            It does everything except the bind.  The bind is left until
 *            we hit something which forces us to bind.
 *
 * RETURNS:   1 - NIS should be queried.
 *            0 - NIS should not be queried.
 */
static int	
check_yellow()
{
	register struct  _grjunk *_gr = setgrjunk();

	if (!REPORT(NIS_MASK))
		return(0);

	if (domain == NULL)
		(void) usingypmap(&domain, NULL);

	return domain != NULL;
}


static int	
bind_to_yp()
{
	register struct  _grjunk *_gr = setgrjunk();

	if (check_yellow() && firsttime == 0) {
		firsttime = 1;
		usingyellow = !yp_bind(domain);
	}
	return usingyellow;
}

/*
 * NAME: set_getgrent_remote
 *
 * FUNCTION: Turns on and off remote lookup routines.  This interface
 *           supports the following lookup paths depending on the "value"
 *           parameter.
 *
 *            value       Path
 *            -----       ---------------
 *              0    :    Local
 *              1    :    Local, NIS, DCE
 *              2    :    Local, NIS 	  ---> "COMPAT"
 *              3    :    DCE
 *
 *           The routine is useful in AIX 4.1 since DCE wants to support the
 *           policy that user names and ids are uniquely defined within
 *           all domains (ie. no conflicts).
 */
void
set_getgrent_remote(int value)
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
 * NAME:  reset_grjunk_authstate
 *
 * FUNCTION: reset the value of authstate in the _grjunk static area,  
 *	     based on the AUTHSTATE env variable.  This is only necessary 
 *	     after a call to authenticate (which sets AUTHSTATE) to ensure
 *	     the getpw* / getgr* routines bind properly.
 *
 */

void
reset_grjunk_authstate()
{
	register struct  _grjunk *_gr = setgrjunk();

	if (_gr != NULL) {
		char	*env;
		authstate = COMPAT;
		env = getenv(AUTH_ENV);
		if (!(env == NULL || env[0] == '\0'))
			if (!strcmp(env, AUTH_DCE))
				authstate = DCE;
	}
	return;
}
