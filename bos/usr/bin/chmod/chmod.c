static char sccsid[] = "@(#)19  1.15.1.7  src/bos/usr/bin/chmod/chmod.c, cmdsdac, bos411, 9435B411a 8/31/94 11:40:35";
/*
 *   COMPONENT_NAME: CMDSDAC
 *
 *   FUNCTIONS: MSGSTR
 *		abs
 *		aclmode
 *		change
 *		err
 *		getacl
 *		main
 *		modaclgr
 *		newmode
 *		what
 *		where
 *		who
 *		
 *
 *   ORIGINS: 26,27,65
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 *   ALL RIGHTS RESERVED
 *   OSF/1 Release 1.0  chmod.c	2.7 (OSF)	10/07/90
 * 
 *   Copyright (c) 1980, 1988 Regents of the University of California.
 *   All rights reserved.  The Berkeley software License Agreement
 *   specifies the terms and conditions for redistribution.
 * 
 *   chmod.c	5.7 (Berkeley) 4/21/88
 */

/*
 * chmod options mode files
 * where
 *	mode is [ugoa][+-=][rwxXstugo] or an octal number
 *	options are -Rhf
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <locale.h>
#include <sys/errno.h>
#include <nl_types.h>
#include "chmod_msg.h"

#ifdef _AIX
#include <sys/acl.h>

static int symbolic;
struct acl *getacl(char *);
int        modaclgr(struct acl *, gid_t, int, unsigned int);
int	   aclmode(char *, struct stat *, struct acl *);
#endif /* _AIX */

#define MSGSTR(num,str) catgets(catd,MS_CHMOD,num,str)  /*MSG*/

static int	fflag, hflag, rflag, retval, um;
static char	*modestring, *ms;

nl_catd catd;

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind, opterr;
	int ch = 0;
	int status = 0;

        (void ) setlocale(LC_ALL,"");
        catd = catopen(MF_CHMOD, NL_CAT_LOCALE);

	while (ch != EOF) {
		if ((argv[optind][0] == '-') &&
		    (strchr("rwxXst", argv[optind][1]) != NULL))
			ch = EOF;
		else if ((ch = getopt(argc, argv, "Rhf")) != EOF) {
			switch((char)ch) {
				case 'R':
					rflag++;
					break;
				case 'h':
					hflag++;
					break;
				case 'f':
					fflag++;
					break;
				case '?':
				default:
	fprintf(stderr, MSGSTR(USAGE, "Usage: chmod [-R] [-f] [-h] {u|g|o|a ...} {+|-|=} {r|w|x|X|s|t ...} File ...\n\
\tchmod [-R] [-f] [-h] OctalNumber File ...\n\
\tChanges the permission codes for files or directories.\n"));
					exit(-1);
			}
		}
	}

	argv += optind;
	argc -= optind;

	if (argc < 2) {
        fprintf(stderr, MSGSTR(USAGE, "Usage: chmod [-R] [-f] [-h] {u|g|o|a ...} {+|-|=} {r|w|x|X|s|t ...} File ...\n\
\tchmod [-R] [-f] [-h] OctalNumber File ...\n\
\tChanges the permission codes for files or directories.\n"));
		exit(-1);
	}

	modestring = *argv;
	um = umask(0);
	(void)newmode((u_short)0);

	while (*++argv)
		status += change(*argv);
	exit(status);
}

change(file)
	char *file;
{
	register DIR *dirp;
	register struct dirent *dp;
	struct stat buf;
	struct stat lbuf;
	int islink = 0, rc = 0;

	if (lstat(file, &buf))
		return(err(file));

	if (( buf.st_mode & S_IFMT) == S_IFLNK)
		islink = 1;

#ifdef _AIX
	/*
	 * Symbolic chmod operations will allow the Access Control
	 * list to be maintained and possibly updated (in the case of
	 * group access).  Absolute mode will remove the ACL due to
	 * masking off the S_IXACL bit in the mode.
	 */
	if (symbolic) {
		if (islink) 
		{
			if (!hflag)
			{
				if (stat(file, &lbuf) < 0)
					return( err(file));
				if (aclmode(file, &lbuf, getacl(file)))
					return(err(file));
			}
		}
		else
			if (aclmode(file, &buf, getacl(file)))
				return(err(file));
	}
	else {
		mode_t mode = newmode(buf.st_mode);

 		/* If the filesystem object is a directory the user must 
	 	 * specify symbolic mode in order to strip off the SGID bit.  
	 	 * This was done in order to keep users from accidentally 
	 	 * stripping off this overloaded bit.  XPG4 and POSIX state 
	 	 * that this behaviour is implementation defined and several 
	 	 * of our competitors implement it this way.
	 	 */
		if ((buf.st_mode & S_IFMT) == S_IFDIR)
	 		mode = mode | (buf.st_mode & S_ISGID);	


		if (!hflag  || ( hflag && !islink) ) {
			if (chmod(file, mode))
				return(err(file));
		}

	}
#else
	if (!hflag  || ( hflag && !islink) ) {
		if (chmod(file, newmode(buf.st_mode)))
			return(err(file));
	}
#endif /* _AIX */

	if (rflag && ((buf.st_mode & S_IFMT) == S_IFDIR)) {
		if (chdir(file) < 0 || !(dirp = opendir(".")))
			return(err(file));

		for (dp = readdir(dirp); dp; dp = readdir(dirp)) {
			if (dp->d_name[0] == '.' && (!dp->d_name[1] ||
			    dp->d_name[1] == '.' && !dp->d_name[2]))
				continue;
			rc += change(dp->d_name);
		}
		closedir(dirp);
		if (chdir("..")) {
			err("..");
			exit(fflag ? 0 : -1);
		}
	}
	return(rc);
}

err(s)
	char *s;
{
	if (!fflag) {
		fputs("chmod: ", stderr);
		perror(s);
	}
	return(!fflag);
}

newmode(nm)
	u_short nm;
{
	register int o, m, b;

	ms = modestring;
	m = abs();
	if (*ms == '\0')
		return (m);

#ifdef _AIX
	symbolic = 1; /* User has supplied symbolic operation */
#endif /* _AIX */

	do {
		m = who();
		while (o = what()) {
			b = where((int)nm);
			switch (o) {
			case '+':
				nm |= b & m;
				break;
			case '-':
				nm &= ~(b & m);
				break;
			case '=':
				nm &= ~m;
				nm |= b & m;
				break;
			}
		}
	} while (*ms++ == ',');
	if (*--ms) {
                fprintf(stderr, MSGSTR(EMODE, "chmod: invalid mode\n"));
		exit(-1);
	}
	return ((int)nm);
}

abs()
{
	register int c, i;

	i = 0;
	while ((c = *ms++) >= '0' && c <= '7')
		i = (i << 3) + (c - '0');
	ms--;
	return (i);
}

#define	USER	05700	/* user's bits */
#define	GROUP	02070	/* group's bits */
#define	OTHER	00007	/* other's bits */
#define	ALL	07777	/* all including setuid according to XPG4 */

#define	READ	00444	/* read permit */
#define	WRITE	00222	/* write permit */
#define	EXEC	00111	/* exec permit */
#define	SETID	06000	/* set[ug]id */
#define	STICKY	01000	/* sticky bit */

who()
{
	register int m;

	m = 0;
	for (;;) switch (*ms++) {
	case 'u':
		m |= USER;
		continue;
	case 'g':
		m |= GROUP;
		continue;
	case 'o':
		m |= OTHER;
		continue;
	case 'a':
		m |= ALL;
		continue;
	default:
		ms--;
		if (m == 0)
			m = ALL & ~um;
		return (m);
	}
}

what()
{
	switch (*ms) {
	case '+':
	case '-':
	case '=':
		return (*ms++);
	}
	return (0);
}

where(om)
	register int om;
{
	register int m;

 	m = 0;
	switch (*ms) {
	case 'u':
		m = (om & USER) >> 6;
		goto dup;
	case 'g':
		m = (om & GROUP) >> 3;
		goto dup;
	case 'o':
		m = (om & OTHER);
	dup:
		m &= (READ|WRITE|EXEC);
		m |= (m << 3) | (m << 6);
		++ms;
		return (m);
	}
	for (;;) switch (*ms++) {
	case 'r':
		m |= READ;
		continue;
	case 'w':
		m |= WRITE;
		continue;
	case 'x':
		m |= EXEC;
		continue;
	case 'X':
		if (((om & S_IFMT) == S_IFDIR) || (om & EXEC))
			m |= EXEC;
		continue;
	case 's':
		m |= SETID;
		continue;
	case 't':
		m |= STICKY;
		continue;
	default:
		ms--;
		return (m);
	}
}

#ifdef _AIX
/*
 * NAME:     	getacl
 *
 * FUNCTION: 	AIX specific routine for returning the access control list 
 *		associated with the file.
 *
 * Returns:	aclbuf	ACL of the file
 *		NULL	failure
 */
struct acl *
getacl(char *path)
{
        int length;
	struct acl *aclbuf;

	if ((aclbuf = (struct acl *)malloc(ACL_SIZ)) == NULL)
		return(NULL);

	aclbuf->acl_len = ACL_SIZ;

	while (statacl(path, 0, aclbuf, aclbuf->acl_len))
	{
		if (errno != ENOSPC)
			return(NULL);
		length = aclbuf->acl_len;
		free(aclbuf);
		if ((aclbuf = (struct acl *)malloc(length)) == 0)
			return(NULL);
		aclbuf->acl_len = length;
	}
	return(aclbuf);
}

/*
 * NAME:     	aclmode
 *
 * FUNCTION: 	AIX specific routine for changing the access control list 
 *		associated with the file.
 *
 * Returns:	 0 success
 *		-1 failure
 */
aclmode(char *path, struct stat *buf, struct acl *aclp)
{
	register int o, m, b, rc = 0;
	mode_t nm;

	/*
	 * Set up the mode to work with:  We need all mode bits including
  	 * the file type (in case we have the -X option).  At the end of 
	 * this routine we will update the file's ACL with chacl().
	 */
	nm = buf->st_mode;
	ms = modestring;

	do {
		m = who();
		while (o = what()) {
			b = where((int)nm);
			switch (o) {
			case '+':
				nm |= b & m;
				break;
			case '-':
				nm &= ~(b & m);
				break;
			case '=':
				nm &= ~m;
				nm |= b & m;
				break;
			}
			/*
		 	 * Deny or Allow group access in all the ACL entries
		 	 */
			if (m & S_IRWXG)
				modaclgr(aclp, buf->st_gid, o, b);
		}
	} while (*ms++ == ',');
	if (*--ms) {
		fprintf(stderr, MSGSTR(EMODE, "chmod: invalid mode\n"));
		exit(-1);
	}

	aclp->acl_mode = nm;
	aclp->u_access = (unsigned)((nm >> 6) & 07);
	aclp->g_access = (unsigned)((nm >> 3) & 07);
	aclp->o_access = (unsigned)(nm & 07);
	  
	rc = chacl(path, aclp, aclp->acl_len);
	free(aclp);

	return((rc < 0) ? -1 : 0);
}

/*
 * NAME:     	modaclgr
 *
 * FUNCTION: 	AIX specific routine for adding or deleting group access 
 *		for all entries in the access control list.  If the user
 * 		has denied/allowed access to the file's group then we 
 *	        do not want any ACL entry gaining or being denied access
 *		based on their specific entry.
 */
int
modaclgr(struct acl *aclp, 
	 gid_t	    gid, 
	 int	    what, 
	 unsigned   where)
{
	ushort	grmode;
	struct acl_entry   *acep;

	grmode = (ushort)((where >> 3) & 07);

	for (acep = aclp->acl_ext; acep<acl_last(aclp); acep=acl_nxt(acep))
	{
                /*
                 * IBM ACLs are parsed from the default ACL into the
                 * extended ACLs with each unique user or group having
                 * a PERMIT and DENY vector.  Once the last entry is found 
	         * the DENY bits are removed from the PERMIT entries.  
		 * Therefore in the following entries we are trying to modify
		 * the extended entry as little as possible to produce the
		 * same results.  
		 */
		if ((acep->ace_len == sizeof(struct acl_entry))	&&
		    (acep->ace_id[0].id_type == ACEID_GROUP)	&&
		    (bcmp(acep->ace_id[0].id_data, &gid, sizeof(gid_t)) == 0))
		{
			switch(what)
			{
			    case '+':
				switch(acep->ace_type)
				{
				    case ACC_DENY:
					acep->ace_access &= ~grmode;
					break;
				    case ACC_SPECIFY:
					acep->ace_access |= grmode;
					break;
				}
				break;
			    case '-':
				switch(acep->ace_type)
				{
				    case ACC_PERMIT:
				    case ACC_SPECIFY:
					acep->ace_access &= ~grmode;
				}
				break;
			    case '=':
				switch(acep->ace_type)
				{
				    case ACC_PERMIT:
				    case ACC_SPECIFY:
					acep->ace_access = grmode;
					break;
				    case ACC_DENY:
					acep->ace_access = ~grmode;
					break;
				}
				break;
			}
		}
	}
}
#endif /* _AIX */
