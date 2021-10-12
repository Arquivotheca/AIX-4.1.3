static char sccsid[] = "@(#)63     1.4  src/bos/usr/ccs/lib/libc_r/getgrent.c, libcs, bos41J, 9515B_all 4/12/95 12:45:01";

#ifdef _POWER_PROLOG_
/*
 *
 *   COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions
 *
 *   FUNCTIONS: grskip, grscan, fgetgrent_r, fgetgrent, getgrent_r, getgrent,
 *	getgrgid_r, getgrgid, getgrnam_r, getgrnam, setgrent_r, setgrent,
 *	endgrent_r, endgrent
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1995
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

/* getgrent.c,v $ $Revision: 2.11.1.5 $ (OSF) */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *



 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <errno.h>

#include "ts_supp.h"


#ifdef _THREAD_SAFE

#define	GR_FP		(*gr_fp)

#define	SETGRENT(f)	setgrent_r(f)
#define	GETGRENT(g)	(getgrent_r(g, line, len, &gr_fp) != TS_FAILURE)
#define	ENDGRENT()	endgrent_r(&gr_fp)
#define	GRSCAN(g, l)	grscan(g, l, len, GR_FP)

/* align an address onto a valid char * value */
#define	ALIGN(a)	(char *)(((unsigned)(a))&~(sizeof(char *) - 1))

#else

#define	MAXLINELENGTH	1024
#define	MAXGRP		2000 
/* In GRPLINLEN the 14 represents the ":!:GID LENGTH:" in the group file. */
#define GRPLINLEN	(L_cuserid) + ((L_cuserid + 1) * MAXGRP) + 14

#define	GR_FP		gr_fp

#define	SETGRENT(f)	setgrent()
#define	GETGRENT(g)	((g = getgrent()) != TS_FAILURE)
#define	ENDGRENT()	endgrent()
#define	GRSCAN(g, l)	grscan(g, l, MAXLINELENGTH, GR_FP)

static struct group	gr_group;
static FILE		*gr_fp;
static char		*line;		/* [MAXLINELENGTH] */
static char		*(*gr_mem);	/* [MAXGRP] */

struct _grjunk
	{
	char	*_domain;
	struct	group _NULLGP;
	FILE	*_grf;
	char	*_yp;
	int	_yplen;
	char	*_oldyp;
	int	_oldyplen;
	char	*_agr_mem[MAXGRP];
	struct list 
	{
		char *name;
		struct list *nxt;
	} *_minuslist;
	struct	group _interpgp;
	char	_interpline[GRPLINLEN+1];
};
#endif	/* _THREAD_SAFE */



#define	GROUP_FILE	"/etc/group"

struct setlist {
	char *name;
	gid_t	gid;
	struct setlist *next;
};

static char *
grskip(register char *p, register c)
{
	while (*p && (*p != c))
		++p;
	if (*p)
		*p++ = 0;
	return(p);
}


static int
grscan(struct group *grent, char *line, int len, FILE *fp)
{
	register char	*p, **q;
	char		**end;
	register int	c;
#ifdef _THREAD_SAFE
	char		**gr_mem;
#endif	/* _THREAD_SAFE */

#ifndef _THREAD_SAFE
	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	for (;;) {
		if (!(p = fgets(line, len, fp)))
			return (0);
		/* skip lines that are too big */
		if (!index(line, '\n')) {
			while ((c = getc(fp)) != '\n' && c != EOF)
				;
			continue;
		}
		break;
	}

#ifdef _THREAD_SAFE
	gr_mem = (char **)ALIGN(p + strlen(p) + sizeof(char *) - 1);
	end = (char **)ALIGN(p + len) - sizeof(char *);
#else
	if (!gr_mem && !(gr_mem = malloc(sizeof(char *) * MAXGRP)))
		return (0);
	end = &gr_mem[MAXGRP - 1];
#endif	/* _THREAD_SAFE */

	grent->gr_name = p;
	grent->gr_passwd = p = grskip(p, ':');
	grent->gr_gid = strtoul(p = grskip(p, ':'), NULL, 10);
	grent->gr_mem = gr_mem;
	p = grskip(p, ':');
	grskip(p,'\n');
	q = gr_mem;
	while (*p) {
		if (q < end)
			*q++ = p;
		p = grskip(p, ',');
	}
	*q = NULL;
	return (1);
}


#ifdef _THREAD_SAFE
int
fgetgrent_r(FILE *fp, struct group *grent, char *line, int len)
{
	FILE	**gr_fp = &fp;
#else
struct group *
fgetgrent(FILE *gr_fp)
{
	register struct group	*grent = &gr_group;

	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((grent == 0 || line == 0 || len <= 0));

	if (!GRSCAN(grent, line))
		return (TS_NOTFOUND);
	return (TS_FOUND(grent));
}


#ifdef _THREAD_SAFE
getgrent_r(struct group *grent, char *line, int len, FILE **gr_fp)
{
#else
struct group *
getgrent(void)
{
	register struct group	*grent = &gr_group;

	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((gr_fp == 0 || grent == 0 || line == 0 || len <= 0));

	if (!GR_FP && SETGRENT(gr_fp) != TS_SUCCESS || !GRSCAN(grent, line))
		return (TS_NOTFOUND);
	return (TS_FOUND(grent));
}


#ifdef  _THREAD_SAFE 
int 
getgrgid_r(gid_t gid, struct group *grent, char *line, int len)
{
	FILE	*gr_fp = 0;
#else
struct group *
getgrgid(register gid_t gid)
{
	register struct group	*grent = &gr_group;

	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((grent == 0 || line == 0 || len <= 0));

	SETGRENT(&gr_fp);
	while (GETGRENT(grent))
		if (grent->gr_gid == gid) {
			ENDGRENT();
			return (TS_FOUND(grent));
		}
	ENDGRENT();
	return (TS_NOTFOUND);
}


#ifdef _THREAD_SAFE 
int
getgrnam_r(const char *nam, struct group *grent, char *line, int len)
{
	FILE	*gr_fp = 0;
#else
struct group *
getgrnam(const char *nam)
{
	register struct group	*grent = &gr_group;

	if (!line && !(line = malloc(sizeof(char) * MAXLINELENGTH)))
		return (0);
#endif	/* _THREAD_SAFE */

	TS_EINVAL((grent == 0 || line == 0 || len <= 0));

	SETGRENT(&gr_fp);
	while (GETGRENT(grent))
		if (strcmp(nam, grent->gr_name) == 0) {
			ENDGRENT();
			return (TS_FOUND(grent));
		}
	ENDGRENT();
	return (TS_NOTFOUND);
}


#ifdef _THREAD_SAFE
int
setgrent_r(FILE **gr_fp)
#else
int
setgrent(void)
#endif	/* _THREAD_SAFE */
{
	int  flags;

	TS_EINVAL((gr_fp == 0));

	if (GR_FP) {
		rewind(GR_FP);
		return (TS_SUCCESS);
	}

	if ((GR_FP = fopen(GROUP_FILE, "r")) != NULL) {
		flags = fcntl(fileno(GR_FP), F_GETFD, 0);
		flags |= FD_CLOEXEC;
		if (fcntl(fileno(GR_FP), F_SETFD, flags) == 0)
			return (TS_SUCCESS);
		(void)fclose(GR_FP);
	}
	return (TS_FAILURE);
}


#ifdef _THREAD_SAFE
void
endgrent_r(FILE **gr_fp)
#else
void
endgrent(void)
#endif	/* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (gr_fp == 0) return;
#endif	/* _THREAD_SAFE */

	if (GR_FP) {
		(void)fclose(GR_FP);
		GR_FP = NULL;
	}
}


#ifdef _THREAD_SAFE
int
getgrset_r(register char *user, register struct _grjunk *_gr)
{
#else
char *
getgrset(register char *user)
{
	register struct _grjunk *_gr = setgrjunk();
#endif /* _THREAD_SAFE */
	struct group *gp, *savegp;
	struct passwd *pwp;
	struct	setlist	*slp, *tslp;
	char	*buf, **p;
	int	count, endoffile;
	int	i, j;
	gid_t	gid;
#ifdef _THREAD_SAFE
        char line[BUFSIZ+1];
	int len = BUFSIZ;
	struct group group;
	struct passwd passwd;
	FILE *gr_fp;
	gp = &group;
	pwp = &passwd;
#endif

	if (_gr == 0)
		TS_RETURN(TS_FAILURE, NULL);

	count = 0;

	/* get the users primary group that is specified in the passwd entry
	   get the group name and put it as the first entry in the buffer.
	   */

#ifdef _THREAD_SAFE
	if ( getpwnam_r(user, pwp, line, sizeof(line) - 1) == TS_FAILURE ) 
#else
        if ((pwp = getpwnam(user)) == NULL)
#endif /* _THREAD_SAFE */
		TS_RETURN(TS_FAILURE, NULL);

	gid = pwp->pw_gid;

#ifdef  _THREAD_SAFE
	if ( getgrgid_r(gid, gp, line, sizeof(line) - 1) == TS_FAILURE )
#else
	if ((gp = getgrgid(gid)) == NULL)
#endif /* _THREAD_SAFE */
		TS_RETURN(TS_FAILURE, NULL);

	if ((slp = (struct setlist *)malloc(sizeof(struct setlist))) == NULL)
		TS_RETURN(TS_FAILURE, NULL);

	slp->next = NULL;
	count = 1;
	if ((slp->name = malloc(strlen(gp->gr_name) +1)) == NULL)
		TS_RETURN(TS_FAILURE, NULL);
	strcpy(slp->name, gp->gr_name);
	slp->gid = gid;


	/* let's start our search
	   */
	SETGRENT(&gr_fp);

#ifdef _THREAD_SAFE
	while GETGRENT(gp)
	   addtogrset(user, slp, &count, gp);
#else

/* 
 * As we won't support yellow pages for this release 
 *  check_yellow() stuff is commented out. 

	if (!check_yellow())
	{
		while(gp = getgrent()) {
			addtogrset(user, slp, &count, gp);
		}
	} else {
		if (!gr_fp)
			return(NULL);

		endoffile = 0;
		while (!endoffile) {
			int	linefnd;
			char	*s, *p;
			char	line[GRPLINLEN+1];

			if (yp)	{
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
				if( !(s = p = fgets(line, GRPLINLEN+1, gr_fp))) {
					endoffile = 1;
					continue;
				}
				while(*s != (char) NULL) {
					if((!(isspace((int)(*s)))) || 
					   (*s == '\n')){
						*p++ = *s;
					}
					*s++;
				}
				*p = *s;
				if(line[0] == '\n')
					continue;
			}
			if ((gp = interpret(line, strlen(line))) == NULL)
				continue;
			switch(line[0])	{
			case '+':
				if (!bind_to_yp())
					continue;
				if (strcmp(gp->gr_name, "+") == 0) {
					if (!getgroupsbyuser(pwp->pw_uid,
							     slp, &count)) {
						getfirstfromyellow();
						savegp = save(gp);
					}
					continue;
				}
				 * 
				 * else look up this entry in yellow pages
				 *
				savegp = save(gp);
				gp = getnamefromyellow(gp->gr_name+1, savegp);
				if (gp == NULL)
					continue;
				else if (onminuslist(gp))
					continue;
				else
					addtogrset(user, slp, &count, gp);
				break;
			case '-':
				addtominuslist(gp->gr_name+1);
				break;
			default:
				if (onminuslist(gp))
					continue;
				addtogrset(user, slp, &count, gp);
				break;
			}
		}
	} 
*/
#endif 

	ENDGRENT();

	buf = _gr->_interpline;
	buf[0] = '\0';
	for (tslp = slp; tslp; tslp = tslp->next) {
		int	len;
		sprintf(buf, "%d", tslp->gid);
		len = strlen(buf);
		if (tslp->next) {
			buf[len] = ',';
			len++;
		}
		buf += len;
	}

	TS_RETURN(TS_SUCCESS, (_gr->_interpline));
}


int
addtogrset (char *user, struct setlist *slp, int *count, struct group *gp)
{
	char	*group;
	struct	setlist	*tslp;
	int	i, j, sz, match, offset;

	group = gp->gr_name;
	/* check to see if the group had already been added to the list
	   */
	for (tslp = slp; tslp; tslp = tslp->next) {
		if (!strcmp(tslp->name, group)) {
			return(0);
		}
	}

	/* check group membership to see if the user is part of the group
	   */
	for (i = 0; gp->gr_mem[i]; i++)
	{
		if (!strcmp(user,gp->gr_mem[i]))
		{
			for (; slp->next; slp = slp->next)
				;
			if ((tslp = (struct setlist *)malloc(sizeof(struct setlist))) == NULL)
				return(-1);
			if ((tslp->name = malloc(strlen(group)+1)) == NULL)
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

/*
 *	This routine in libc_r is just a stub.  The real 
 *	routine in libc resets a static variable named authstate
 *	in the _grjunk structure used for NIS and DCE lookups.  
 *	_grjunk is not defined in libc_r because NIS and DCE lookups 
 *	are not thread-safe.  But any threaded program binding to libs 
 *	still needs to find this routine.
 */
void
reset_grjunk_authstate()
{
	return;
}
