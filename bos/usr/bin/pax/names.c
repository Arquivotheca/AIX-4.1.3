static char sccsid[] = "@(#)73	1.3  src/bos/usr/bin/pax/names.c, cmdarch, bos412, 9446B 11/11/94 21:54:10";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log$
 *
 */
/* static char rcsid[] = "RCSfile Revision (OSF) Date"; */
/* $Source: /u/mark/src/pax/RCS/names.c,v $
 *
 * $Revision: 1.2 $
 *
 * names.c - Look up user and/or group names. 
 *
 * DESCRIPTION
 *
 *	These functions support UID and GID name lookup.  The results are
 *	cached to improve performance.
 *
 * AUTHOR
 *
 *	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log:	names.c,v $
 * Revision 1.2  89/02/12  10:05:05  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:19  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: names.c,v 1.2 89/02/12 10:05:05 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Defines */

#define myuid	( my_uid < 0? (my_uid = getuid()): my_uid )
#define	mygid	( my_gid < 0? (my_gid = getgid()): my_gid )


/* Internal Identifiers */

static int      saveuid = -993;
static char     saveuname[TUNMLEN];
static int      my_uid = -993;

static int      savegid = -993;
static char     savegname[TGNMLEN];
static int      my_gid = -993;


/* finduname - find a user or group name from a uid or gid
 *
 * DESCRIPTION
 *
 * 	Look up a user name from a uid/gid, maintaining a cache. 
 *
 * PARAMETERS
 *
 *	char	uname[]		- name (to be returned to user)
 *	int	uuid		- id of name to find
 *
 *
 * RETURNS
 *
 *	Returns a name which is associated with the user id given.  If there
 *	is not name which corresponds to the user-id given, then a pointer
 *	to a string of zero length is returned.
 *	
 * Possible improvements:
 *
 * 	1. for now it's a one-entry cache. 
 *	2. The "-993" is to reduce the chance of a hit on the first lookup. 
 */


char *finduname(int uuid)

{
    struct passwd  *pw;

    if (uuid != saveuid) {
	saveuid = uuid;
	saveuname[0] = '\0';
	pw = getpwuid(uuid);
	if (pw) {
	    strncpy(saveuname, pw->pw_name, TUNMLEN);
	}
    }
    return(saveuname);
}

/* findgname - look up a group name from a gid
 *
 * DESCRIPTION
 *
 * 	Look up a group name from a gid, maintaining a cache.
 *	
 *
 * PARAMETERS
 *
 *	int	ggid		- goupid of group to find
 *
 * RETURNS
 *
 *	A string which is associated with the group ID given.  If no name
 *	can be found, a string of zero length is returned.
 */


char *findgname(int ggid)

{
    struct group   *gr;

    if (ggid != savegid) {
	savegid = ggid;
	savegname[0] = '\0';
	setgrent();
	gr = getgrgid(ggid);
	if (gr) {
	    strncpy(savegname, gr->gr_name, TGNMLEN);
	}
    }
    return(savegname);
}
