static char sccsid[] = "@(#)65	1.7  src/bos/usr/bin/pax/extract.c, cmdarch, bos412, 9446B 11/11/94 21:54:02";
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
/* $Source: /u/mark/src/pax/RCS/extract.c,v $
 *
 * $Revision: 1.3 $
 *
 * extract.c - Extract files from a tar archive. 
 *
 * DESCRIPTION
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
 * $Log:	extract.c,v $
 * Revision 1.3  89/02/12  10:29:43  mark
 * Fixed misspelling of Replstr
 * 
 * Revision 1.2  89/02/12  10:04:24  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:07  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: extract.c,v 1.3 89/02/12 10:29:43 mark Exp Locker: mark $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Defines */

/*
 * Swap bytes. 
 */
#define	SWAB(n)	((((ushort)(n) >> 8) & 0xff) | (((ushort)(n) << 8) & 0xff00))


/* Function Prototypes */


static int inbinary(char *, char *, Stat *);
static int inascii(char *, char *, Stat *);
static int inswab(char *, char *, Stat *);
static int readtar(char *, Stat *);
static int readcpio(char *, Stat *);
static void skip_file(char *, Stat *);
static void reset_directories(void);

/* read_archive - read in an archive
 *
 * DESCRIPTION
 *
 *	Read_archive is the central entry point for reading archives.
 *	Read_archive determines the proper archive functions to call
 *	based upon the archive type being processed.
 *
 * RETURNS
 *
 */


int read_archive(void)

{
    Stat            sb;
    char            name[PATH_MAX + 1];
    int             match;
    int		    pad;

    name_gather();		/* get names from command line */
    name[0] = '\0';
    while (get_header(name, &sb) == 0) {

	if (f_charmap && charmap_convert(name) < 0) 
	    continue;

	match = name_match(name, ((sb.sb_mode & S_IFMT) == S_IFDIR));

	if (match == -1)
		break;

	match ^= f_reverse_match;

	if (!match) {
	    skip_file(name, &sb);
	    continue;
	}

	if (rplhead != (Replstr *)NULL) {
	    rpl_name(name);
	    if (strlen(name) == 0) {
		skip_file(name, &sb);
		continue;
	    }
	}

	if (f_list) {		/* only wanted a table of contents */
	    print_entry(name, &sb);
	    skip_file(name, &sb);
	    continue;
	}

	if (get_disposition(EXTRACT, name) || 
	    get_newname(name, sizeof(name))) {
	    skip_file(name, &sb);
	    continue;
	} 

	if (inentry(name, &sb) < 0) {
	    warn(name, MSGSTR(EX_CORRUPT, "File data is corrupt"));
	}

	if (f_verbose) {
	    print_entry(name, &sb);
	}

	if (ar_format == TAR && sb.sb_nlink > 1) {
	    /*
	     * This kludge makes sure that the link table is cleared
	     * before attempting to process any other links.
	     */
	    if (sb.sb_nlink > 1) {
		linkfrom(name, &sb);
	    }
	}

	if (ar_format == TAR && (pad = sb.sb_size % BLOCKSIZE) != 0) {
	    pad = BLOCKSIZE - pad;
	    buf_skip((OFFSET) pad);
	}
    }

    close_archive();
    if (!f_list)
	reset_directories();
}


/* skip_file - skip a file on an archive
 *
 * DESCRIPTION
 *
 *	Skip_file will skip the file data on the archive in the event
 *	that the data does not need to be read.  This includes such things
 *	as f_list mode and cases where a pattern match failed.
 *
 * PARAMETERS
 *
 *	char	*name	- name of the file
 *	Stat	*asb	- Stat block for the file.
 */


static void 
skip_file(char *name, Stat *asb)

{
    int rc;

    if (ar_format == TAR) {
	rc = buf_skip(ROUNDUP((OFFSET) asb->sb_size, BLOCKSIZE));
    } else {
	rc = buf_skip((OFFSET) asb->sb_size);
    }

    if (rc < 0) {
	warn(name, MSGSTR(EX_CORRUPT, "File data is corrupt"));
    }
}


/* get_header - figures which type of header needs to be read.
 *
 * DESCRIPTION
 *
 *	This is merely a single entry point for the two types of archive
 *	headers which are supported.  The correct header is selected
 *	depending on the archive type.
 *
 * PARAMETERS
 *
 *	char	*name	- name of the file (passed to header routine)
 *	Stat	*asb	- Stat block for the file (passed to header routine)
 *
 * RETURNS
 *
 *	Returns the value which was returned by the proper header
 *	function.
 */


int get_header(char *name, Stat *asb)
{
    if (ar_format == TAR) {
	return(readtar(name, asb));
    } else {
	return(readcpio(name, asb));
    }
}


/* readtar - read a tar header
 *
 * DESCRIPTION
 *
 *	Tar_head read a tar format header from the archive.  The name
 *	and asb parameters are modified as appropriate for the file listed
 *	in the header.   Name is assumed to be a pointer to an array of
 *	at least PATH_MAX bytes.
 *
 * PARAMETERS
 *
 *	char	*name 	- name of the file for which the header is
 *			  for.  This is modified and passed back to
 *			  the caller.
 *	Stat	*asb	- Stat block for the file for which the header
 *			  is for.  The fields of the stat structure are
 *			  extracted from the archive header.  This is
 *			  also passed back to the caller.
 *
 * RETURNS
 *
 *	Returns 0 if a valid header was found, or -1 if EOF is
 *	encountered.
 */


static int readtar(char *name, Stat *asb)

{
    int             status = 3;	/* Initial status at start of archive */
    static int      prev_status;

    for (;;) {
	prev_status = status;
	status = read_header(name, asb);
	switch (status) {
	case 1:		/* Valid header */
		return(0);
	case 0:		/* Invalid header */
	    switch (prev_status) {
	    case 3:		/* Error on first record */
		warn(ar_file, MSGSTR(EX_NOTAR, "This doesn't look like a tar archive"));
		/* FALLTHRU */
	    case 2:		/* Error after record of zeroes */
	    case 1:		/* Error after header rec */
		warn(ar_file, MSGSTR(EX_SKIP, "Skipping to next file..."));
		/* FALLTHRU */
	    default:
	    case 0:		/* Error after error */
		break;
	    }
	    break;

	case 2:			/* Record of zeroes */
	case EOF:		/* End of archive */
	default:
	    return(-1);
	}
    }
}


/* readcpio - read a CPIO header 
 *
 * DESCRIPTION
 *
 *	Read in a cpio header.  Understands how to determine and read ASCII, 
 *	binary and byte-swapped binary headers.  Quietly translates 
 *	old-fashioned binary cpio headers (and arranges to skip the possible 
 *	alignment byte). Returns zero if successful, -1 upon archive trailer. 
 *
 * PARAMETERS
 *
 *	char	*name 	- name of the file for which the header is
 *			  for.  This is modified and passed back to
 *			  the caller.
 *	Stat	*asb	- Stat block for the file for which the header
 *			  is for.  The fields of the stat structure are
 *			  extracted from the archive header.  This is
 *			  also passed back to the caller.
 *
 * RETURNS
 *
 *	Returns 0 if a valid header was found, or -1 if EOF is
 *	encountered.
 */


static int readcpio(char *name, Stat *asb)

{
    OFFSET          skipped;
    char            magic[M_STRLEN];
    static int      align;

    if (align > 0) {
	buf_skip((OFFSET) align);
    }
    align = 0;
    for (;;) {
	if (f_append)
	    lastheader = bufidx;		/* remember for backup */
	buf_read(magic, M_STRLEN);
	skipped = 0;
	while ((align = inascii(magic, name, asb)) < 0
	       && (align = inbinary(magic, name, asb)) < 0
	       && (align = inswab(magic, name, asb)) < 0) {
	    if (++skipped == 1) {
		if (total - sizeof(magic) == 0) {
		    fatal(MSGSTR(EX_UNREG, "Unrecognizable archive"));
		}
		warnarch(MSGSTR(EX_BADMAG, "Bad magic number"), (OFFSET) sizeof(magic));
		if (name[0]) {
		    warn(name, MSGSTR(EX_MAY, "May be corrupt"));
		}
	    }
	    memcpy(magic, magic + 1, sizeof(magic) - 1);
	    buf_read(magic + sizeof(magic) - 1, 1);
	}
	if (skipped) {
	    warnarch(MSGSTR(EX_RESYNC, "Apparently resynchronized"), (OFFSET) sizeof(magic));
	    warn(name, MSGSTR(EX_CONT, "Continuing"));
	}
	if (strcmp(name, TRAILER) == 0) {
	    return (-1);
	}
	if (nameopt(name) >= 0) {
	    break;
	}
	buf_skip((OFFSET) asb->sb_size + align);
    }
#ifdef	S_IFLNK
    if ((asb->sb_mode & S_IFMT) == S_IFLNK) {
	if (buf_read(asb->sb_link, (uint) asb->sb_size) < 0) {
	    warn(name, MSGSTR(EX_SYM, "Corrupt symbolic link"));
	    return (readcpio(name, asb));
	}
	asb->sb_link[asb->sb_size] = '\0';
	asb->sb_size = 0;
    }
#endif				/* S_IFLNK */

    /* destroy absolute pathnames for security reasons */
    if (name[0] == '/') {
	if (name[1]) {
	    while (name[0] = name[1]) {
		++name;
	    }
	} else {
	    name[0] = '.';
	}
    }
    asb->sb_ctime = asb->sb_mtime;
    asb->sb_atime = -1;			/* the access time will be 'now' */
    if (asb->sb_nlink > 1) {
	linkto(name, asb);
    }
    return (0);
}


/* inswab - read a reversed by order binary header
 *
 * DESCRIPTIONS
 *
 *	Reads a byte-swapped CPIO binary archive header
 *
 * PARMAMETERS
 *
 *	char	*magic	- magic number to match
 *	char	*name	- name of the file which is stored in the header.
 *			  (modified and passed back to caller).
 *	Stat	*asb	- stat block for the file (modified and passed back
 *			  to the caller).
 *
 *
 * RETURNS
 *
 * 	Returns the number of trailing alignment bytes to skip; -1 if 
 *	unsuccessful. 
 *
 */


static int inswab(char *magic, char *name, Stat *asb)

{
    ushort          namesize;
    uint            namefull;
    Binary          binary;

    if (*((ushort *) magic) != SWAB(M_BINARY)) {
	return (-1);
    }
    memcpy((char *) &binary,
		  magic + sizeof(ushort),
		  M_STRLEN - sizeof(ushort));
    if (buf_read((char *) &binary + M_STRLEN - sizeof(ushort),
		 sizeof(binary) - (M_STRLEN - sizeof(ushort))) < 0) {
	warnarch(MSGSTR(EX_HDR, "Corrupt swapped header"),
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    asb->sb_dev = (dev_t) SWAB(binary.b_dev);
    asb->sb_ino = (ino_t) SWAB(binary.b_ino);
    asb->sb_mode = SWAB(binary.b_mode);
    asb->sb_uid = SWAB(binary.b_uid);
    asb->sb_gid = SWAB(binary.b_gid);
    asb->sb_nlink = SWAB(binary.b_nlink);
    asb->sb_rdev = (dev_t) SWAB(binary.b_rdev);
    asb->sb_mtime = SWAB(binary.b_mtime[0]) << 16 | SWAB(binary.b_mtime[1]);
    asb->sb_size = SWAB(binary.b_size[0]) << 16 | SWAB(binary.b_size[1]);
    if ((namesize = SWAB(binary.b_name)) == 0 || namesize >= PATH_MAX) {
	warnarch(MSGSTR(EX_PLEN1, "Bad swapped pathname length"),
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    if (buf_read(name, namefull = namesize + namesize % 2) < 0) {
	warnarch(MSGSTR(EX_PATH, "Corrupt swapped pathname"),(OFFSET) namefull);
	return (-1);
    }
    if (name[namesize - 1] != '\0') {
	warnarch(MSGSTR(EX_BADPATH, "Bad swapped pathname"), (OFFSET) namefull);
	return (-1);
    }
    return (asb->sb_size % 2);
}


/* inascii - read in an ASCII cpio header
 *
 * DESCRIPTION
 *
 *	Reads an ASCII format cpio header
 *
 * PARAMETERS
 *
 *	char	*magic	- magic number to match
 *	char	*name	- name of the file which is stored in the header.
 *			  (modified and passed back to caller).
 *	Stat	*asb	- stat block for the file (modified and passed back
 *			  to the caller).
 *
 * RETURNS
 *
 * 	Returns zero if successful; -1 otherwise. Assumes that  the entire 
 *	magic number has been read. 
 */


static int inascii(char *magic, char *name, Stat *asb)

{
    uint            namelen;
    char            header[H_STRLEN + 1];

    if (strncmp(magic, M_ASCII, M_STRLEN) != 0) {
	return (-1);
    }
    if (buf_read(header, H_STRLEN) < 0) {
	warnarch(MSGSTR(EX_ASCII, "Corrupt ASCII header"), (OFFSET) H_STRLEN);
	return (-1);
    }
    header[H_STRLEN] = '\0';
    if (sscanf(header, H_SCAN, &asb->sb_dev,
	       &asb->sb_ino, &asb->sb_mode, &asb->sb_uid,
	       &asb->sb_gid, &asb->sb_nlink, &asb->sb_rdev,
	       &asb->sb_mtime, &namelen, &asb->sb_size) != H_COUNT) {
	warnarch(MSGSTR(EX_BADHDR, "Bad ASCII header"), (OFFSET) H_STRLEN);
	return (-1);
    }
    if (namelen == 0 || namelen >= PATH_MAX) {
	warnarch(MSGSTR(EX_PLEN2, "Bad ASCII pathname length"), (OFFSET) H_STRLEN);
	return (-1);
    }
    if (buf_read(name, namelen) < 0) {
	warnarch(MSGSTR(EX_APATH, "Corrupt ASCII pathname"), (OFFSET) namelen);
	return (-1);
    }
    if (name[namelen - 1] != '\0') {
	warnarch(MSGSTR(EX_BADAPATH, "Bad ASCII pathname"), (OFFSET) namelen);
	return (-1);
    }
    return (0);
}


/* inbinary - read a binary header
 *
 * DESCRIPTION
 *
 *	Reads a CPIO format binary header.
 *
 * PARAMETERS
 *
 *	char	*magic	- magic number to match
 *	char	*name	- name of the file which is stored in the header.
 *			  (modified and passed back to caller).
 *	Stat	*asb	- stat block for the file (modified and passed back
 *			  to the caller).
 *
 * RETURNS
 *
 * 	Returns the number of trailing alignment bytes to skip; -1 if 
 *	unsuccessful. 
 */


static int inbinary(char *magic, char *name, Stat *asb)

{
    uint            namefull;
    Binary          binary;

    if (*((ushort *) magic) != M_BINARY) {
	return (-1);
    }
    memcpy((char *) &binary,
		  magic + sizeof(ushort),
		  M_STRLEN - sizeof(ushort));
    if (buf_read((char *) &binary + M_STRLEN - sizeof(ushort),
		 sizeof(binary) - (M_STRLEN - sizeof(ushort))) < 0) {
	warnarch(MSGSTR(EX_BHDR, "Corrupt binary header"),
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    asb->sb_dev = binary.b_dev;
    asb->sb_ino = binary.b_ino;
    asb->sb_mode = binary.b_mode;
    asb->sb_uid = binary.b_uid;
    asb->sb_gid = binary.b_gid;
    asb->sb_nlink = binary.b_nlink;
    asb->sb_rdev = binary.b_rdev;
    asb->sb_mtime = binary.b_mtime[0] << 16 | binary.b_mtime[1];
    asb->sb_size = binary.b_size[0] << 16 | binary.b_size[1];
    if (binary.b_name == 0 || binary.b_name >= PATH_MAX) {
	warnarch(MSGSTR(EX_PLEN3, "Bad binary pathname length"),
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    if (buf_read(name, namefull = binary.b_name + binary.b_name % 2) < 0) {
	warnarch(MSGSTR(EX_BPATH, "Corrupt binary pathname"),(OFFSET) namefull);
	return (-1);
    }
    if (name[binary.b_name - 1] != '\0') {
	warnarch(MSGSTR(EX_BADP, "Bad binary pathname"), (OFFSET) namefull);
	return (-1);
    }
    return (asb->sb_size % 2);
}


/* reset_directories - reset time/mode on directories we have restored.
 *
 * DESCRIPTION
 *
 *	Walk through the list of directories we have extracted from
 *	the archive (dirhead) and reset the permissions, the
 *	modify times, the owner and group and if we have it, 
 * 	the access times.
 *
 */


static void 
reset_directories(void)

{
    Dirlist		*dp;
    mode_t		perm;
    struct utimbuf	tstamp;
    time_t		now;
    char 		*name;

    now = time((time_t) 0);	/* cut down on time calls */

    for (dp = dirhead; dp; dp=dp->next) {
	name = dp->name;
	if (f_mode && f_owner) 		/* restore all mode bits */
	    perm = dp->perm & (S_IPERM | S_ISUID | S_ISGID);
	else if (f_mode)
	    perm = dp->perm & S_IPERM;
	else
	    perm = (dp->perm & S_IPOPN) & ~mask; 	/* use umask */
	if (chmod(name, perm) < 0)
	    warn(name, strerror(errno));

	if (f_extract_access_time || f_mtime) {
	    if ((dp->atime == -1) && f_extract_access_time)
		dp->atime = now;
	    tstamp.actime = f_extract_access_time ?  dp->atime  : now;
	    tstamp.modtime = f_mtime ? dp->mtime : now;
	    utime(name, &tstamp);
	}

	if (f_owner && (chown(name, dp->uid, dp->gid) < 0))  {
	    warn(name, MSGSTR(EX_OWN, "could not restore owner/group"));
	    chmod(name, (perm & S_IPERM));	/* Clear SUID/SGID bits */
	}
    }
    return;
}
