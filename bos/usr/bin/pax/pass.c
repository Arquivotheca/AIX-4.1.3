static char sccsid[] = "@(#)74	1.6  src/bos/usr/bin/pax/pass.c, cmdarch, bos41B, 9506B 1/30/95 13:23:13";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 18, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 * 
 */

/* static char rcsid[] = "RCSfile Revision (OSF) Date"; */
/* $Source: /u/mark/src/pax/RCS/pass.c,v $
 *
 * $Revision: 1.3 $
 *
 * pass.c - handle the pass option of cpio
 *
 * DESCRIPTION
 *
 *	These functions implement the pass options in PAX.  The pass option
 *	copies files from one directory hierarchy to another.
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
 * $Log:	pass.c,v $
 * Revision 1.3  89/02/12  10:29:51  mark
 * Fixed misspelling of Replstr
 * 
 * Revision 1.2  89/02/12  10:05:09  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:20  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: pass.c,v 1.3 89/02/12 10:29:51 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* pass - copy within the filesystem
 *
 * DESCRIPTION
 *
 *	Pass copies the named files from the current directory hierarchy to
 *	the directory pointed to by dirname.
 *
 * PARAMETERS
 *
 *	char	*dirname	- name of directory to copy named files to.
 *
 */

    
int pass(char *dirname)

{
    char            name[PATH_MAX + 1];
    int             fd;
    Stat            sb;
    struct utimbuf  tstamp;

    while (name_next(name, &sb) >= 0 && (fd = openin(name, &sb)) >= 0) {

	if (rplhead != (Replstr *)NULL) {
	    rpl_name(name);
	    if (strlen(name) == 0) {
		if (fd)
		    close(fd);
		continue;
	    }
	}
	if (get_disposition(PASS, name) || get_newname(name, sizeof(name))) {
	    /* skip file... */
	    if (fd) {
		close(fd);
	    }
	    continue;
	} 

	if (passitem(name, &sb, fd, dirname)) {
	    close(fd);
	    if (f_access_time) {
		 /* -t option: preserve access time of input */
		tstamp.actime  = sb.sb_atime;
		tstamp.modtime = sb.sb_mtime;
		utime(name, &tstamp);
	    }
	}
	if (f_verbose) {
	    char buf[PATH_MAX+1];

	    strcpy(buf, dirname);
	    if (buf[strlen(buf)-1] != '/')
		strcat(buf, "/");
	    strcat(buf, (name[0]=='/' ? &name[1] : name));
	    print_entry(buf, &sb);
	}
    }
}


/* passitem - copy one file
 *
 * DESCRIPTION
 *
 *	Passitem copies a specific file to the named directory
 *
 * PARAMETERS
 *
 *	char   *from	- the name of the file to open
 *	Stat   *asb	- the stat block associated with the file to copy
 *	int	ifd	- the input file descriptor for the file to copy
 *	char   *dir	- the directory to copy it to
 *
 * RETURNS
 *
 * 	Returns given input file descriptor or -1 if an error occurs.
 *
 * ERRORS
 */


static int 
passitem(char *from, Stat *asb, int ifd, char *dir)

{
    int             ofd;
    struct utimbuf  tstamp;
    char            to[PATH_MAX + 1];

    if (nameopt(strcat(strcat(strcpy(to, dir), "/"), from)) < 0) {
	return (-1);
    }
    if (asb->sb_nlink > 1) {
	linkto(to, asb);
    }
    if (f_link && islink(from, asb) == (Link *)NULL) {
	linkto(from, asb);
    }
    if ((ofd = openout(to, asb, islink(to, asb), 1)) < 0) {
	return (-1);
    }
    if (ofd > 0) {
	passdata(from, ifd, to, ofd);
    }
    tstamp.actime = f_extract_access_time ? asb->sb_atime :  time((time_t *) 0);
    tstamp.modtime = f_mtime ? asb->sb_mtime : time((time_t *) 0);
    utime(to, &tstamp);

    if (f_mode)
	chmod(to, asb->sb_mode & S_IPERM);

    if (f_owner) {
	/*
	 * if lchown() does not exist on your system, then be careful not to
	 * call chown() on symbolic links.
	 */
	if (lchown(to, asb->sb_uid, asb->sb_gid) < 0)
	    warn(to, MSGSTR(BUF_PRESERVE, "unable to preserve owner/group"));
	else if (f_mode)
	    chmod(to, asb->sb_mode & (S_IPERM | S_ISUID | S_ISGID));
    }

    return (ifd);
}
