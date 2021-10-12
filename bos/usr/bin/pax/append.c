static char sccsid[] = "@(#)57	1.5  src/bos/usr/bin/pax/append.c, cmdarch, bos412, 9446B 11/11/94 21:53:57";
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 *
 */

/*
 * static char rcsid[] = "RCSfile Revision (OSF) Date";
 */
/* $Source: /u/mark/src/pax/RCS/append.c,v $
 *
 * $Revision: 1.2 $
 *
 * append.c - append to a tape archive. 
 *
 * DESCRIPTION
 *
 *	Routines to allow appending of archives
 *
 * AUTHORS
 *
 *     	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
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
 * $Log:	append.c,v $
 * Revision 1.2  89/02/12  10:03:58  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:00  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: append.c,v 1.2 89/02/12 10:03:58 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"

static void backup(void);

/* append_archive - main loop for appending to a tar archive
 *
 * DESCRIPTION
 *
 *	Append_archive reads an archive until the end of the archive is
 *	reached once the archive is reached, the buffers are reset and the
 *	create_archive function is called to handle the actual writing of
 *	the appended archive data.  This is quite similar to the
 *	read_archive function, however, it does not do all the processing.
 */


void
append_archive(void)
{
    Stat            sb;
    char            name[PATH_MAX + 1];

    name[0] = '\0';
    while (get_header(name, &sb) == 0) {
	if (!f_unconditional)
	    hash_name(name, &sb);

	if (((ar_format == TAR)
	     ? buf_skip(ROUNDUP((OFFSET) sb.sb_size, BLOCKSIZE))
	     : buf_skip((OFFSET) sb.sb_size)) < 0) {
	    warn(name, MSGSTR(APPEND_CORRUPT, "File data is corrupt"));
	}
    }
    /* we have now gotten to the end of the archive... */

    backup();   /* adjusts the file descriptor and buffer pointers */

	/* reset the buffer now that we have read the entire archive */
	/* bufend = bufidx = bufstart; */
    create_archive();
}


/* backup - back the tape up to the end of data in the archive.
 *
 * DESCRIPTION
 *
 *	The last header we have read is either the cpio TRAILER!!! entry
 *	or the two blocks (512 bytes each) of zero's for tar archives.
 * 	adjust the file pointer and the buffer pointers to point to
 * 	the beginning of the trailer headers.
 */


static void 
backup(void)

{
	int count;
	struct devinfo devnfo ;
	struct stop st_com;
	
	if (ioctl(archivefd, IOCINFO, &devnfo) == -1)
		devnfo.devtype = '#';
	if ((devnfo.devtype == DD_TAPE) || (devnfo.devtype == DD_SCTAPE)) {
	    if (devnfo.un.scmt.blksize == 0)
		count = (bufend-bufstart)/blocksize;
	    else
		count = (bufend-bufstart)/devnfo.un.scmt.blksize;
	    st_com.st_op = STRSR;
	    st_com.st_count = count;
	    if (ioctl(archivefd, STIOCTOP, &st_com) < 0) {
		warn("ioctl", strerror(errno));
		fatal(MSGSTR(APPEND_BACK2, "backspace error"));
	    }
	} else {
	    if (lseek(archivefd, -(off_t)(bufend-bufstart), SEEK_CUR) == -1) {
		warn("lseek", strerror(errno));
		fatal(MSGSTR(APPEND_BACK2, "backspace error"));
	    }
	}

	bufidx = lastheader;	/* point to beginning of trailer */
	/*
	 * if lastheader points to the very end of the buffer
	 * Then the trailer really started at the beginning of this buffer
	 */
	if (bufidx == bufstart+blocksize)
		bufidx = bufstart;
}
