static char sccsid[] = "@(#)70	1.12  src/bos/usr/bin/pax/list.c, cmdarch, bos412, 9446B 11/11/94 21:54:05";
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
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 */

/* static char rcsid[] = "RCSfile Revision (OSF) Date"; */
/* $Source: /u/mark/src/pax/RCS/list.c,v $
 *
 * $Revision: 1.2 $
 *
 * list.c - List all files on an archive
 *
 * DESCRIPTION
 *
 *	These function are needed to support archive table of contents and
 *	verbose mode during extraction and creation of achives.
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
 * $Log:	list.c,v $
 * Revision 1.2  89/02/12  10:04:43  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:14  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: list.c,v 1.2 89/02/12 10:04:43 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Defines */

/*
 * isodigit returns non zero iff argument is an octal digit, zero otherwise
 */
#define	ISODIGIT(c)	(((c) >= '0') && ((c) <= '7'))


/* Function Prototypes */


static long signed_checksum(char *);
static void cpio_entry(char *, Stat *);
static void tar_entry(char *, Stat *);
static void pax_entry(char *, Stat *);
static void print_mode(ushort);
static long from_oct(int digs, char *where);

/* read_header - read a header record
 *
 * DESCRIPTION
 *
 * 	Read a record that's supposed to be a header record. Return its 
 *	address in "head", and if it is good, the file's size in 
 *	asb->sb_size.  Decode things from a file header record into a "Stat". 
 *	Also set "head_standard" to !=0 or ==0 depending whether header record 
 *	is "Unix Standard" tar format or regular old tar format. 
 *
 * PARAMETERS
 *
 *	char   *name		- pointer which will contain name of file
 *	Stat   *asb		- pointer which will contain stat info
 *
 * RETURNS
 *
 * 	Return 1 for success, 0 if the checksum is bad, EOF on eof, 2 for a 
 * 	record full of zeros (EOF marker). 
 */


#define header_copy(to,from,n) { strncpy((to), (from), (n)); (to)[(n)]='\0'; }

int read_header(char *name, Stat *asb)

{
    int             i;
    long            sum;
    long	    recsum;
    Link           *link;
    char           *p;
    char            hdrbuf[BLOCKSIZE];

    memset((char *)asb, 0, sizeof(Stat));

    if (f_append)
	lastheader = bufidx;		/* remember for backup */

    /* read the header from the buffer */
    if (buf_read(hdrbuf, BLOCKSIZE) != 0) {
	return (EOF);
    }

    header_copy(name, hdrbuf + 345, PFIXSIZ);
    if ((i = strlen(name)) > 0)
	name[i++] = '/';
    header_copy(name+i, hdrbuf, NAMSIZ);

    recsum = from_oct(8, &hdrbuf[148]);
    sum = 0;
    p = hdrbuf;
    for (i = 0 ; i < 500; i++) {

	/*
	 * We can't use unsigned char here because of old compilers, e.g. V7. 
	 */
	sum += 0xFF & *p++;
    }

    /* Adjust checksum to count the "chksum" field as blanks. */
    for (i = 0; i < 8; i++) {
	sum -= 0xFF & hdrbuf[148 + i];
    }
    sum += ' ' * 8;

    if (sum == 8 * ' ') {

	/*
	 * This is a zeroed record...whole record is 0's except for the 8
	 * blanks we faked for the checksum field. 
	 */
	return (2);
    }
    if ((sum == recsum) || (signed_checksum(hdrbuf) == recsum)) {
	/*
	 * Good record.  Decode file size and return. 
	 */
	if (hdrbuf[156] != LNKTYPE) {
	    asb->sb_size = from_oct(1 + 12, &hdrbuf[124]);
	}
	asb->sb_mtime = from_oct(1 + 12, &hdrbuf[136]);
	asb->sb_mode = from_oct(8, &hdrbuf[100]);
	asb->sb_atime = -1;	/* access time will be 'now' */

	if (strcmp(&hdrbuf[257], TMAGIC) == 0) {
	    /* Unix Standard tar archive */
	    head_standard = 1;
	    asb->sb_uid = from_oct(8, &hdrbuf[108]);
	    asb->sb_gid = from_oct(8, &hdrbuf[116]);
	    switch (hdrbuf[156]) {
	    case BLKTYPE:
	    case CHRTYPE:
		asb->sb_rdev = makedev(from_oct(8, &hdrbuf[329]),
				      from_oct(8, &hdrbuf[337]));
		break;
	    default:
		/* do nothing... */
		break;
	    }
	} else {
	    /* Old fashioned tar archive */
	    head_standard = 0;
	    asb->sb_uid = from_oct(8, &hdrbuf[108]);
	    asb->sb_gid = from_oct(8, &hdrbuf[116]);
	}

	switch (hdrbuf[156]) {
	case REGTYPE:
	case AREGTYPE:
	    /*
	     * Berkeley tar stores directories as regular files with a
	     * trailing /
	     */
	    if (name[strlen(name) - 1] == '/') {
		name[strlen(name) - 1] = '\0';
		asb->sb_mode |= S_IFDIR;
	    } else {
		asb->sb_mode |= S_IFREG;
	    }
	    break;
	case LNKTYPE:
	    asb->sb_nlink = 2;
		/* we need to save the linkname so that it is available later
		 * when we have to search the link chain for this link.
		 */
	    header_copy(linkname, hdrbuf + 157, 100);
	    linkto(linkname, asb);
	    if (rplhead != (Replstr *)NULL) {
		rpl_name(linkname);
	    }
	    linkto(name, asb);
	    asb->sb_mode |= S_IFREG;
	    break;
	case BLKTYPE:
	    asb->sb_mode |= S_IFBLK;
	    break;
	case CHRTYPE:
	    asb->sb_mode |= S_IFCHR;
	    break;
	case DIRTYPE:
	    asb->sb_mode |= S_IFDIR;
	    break;
#ifdef S_IFLNK
	case SYMTYPE:
	    asb->sb_mode |= S_IFLNK;
	    header_copy(asb->sb_link, hdrbuf + 157, 100);
	    break;
#endif
#ifdef S_IFIFO
	case FIFOTYPE:
	    asb->sb_mode |= S_IFIFO;
	    break;
#endif
#ifdef S_IFCTG
	case CONTTYPE:
	    asb->sb_mode |= S_IFCTG;
	    break;
#endif
	}
	return (1);
    }
    return (0);
}


/* signed_checksum - computed signed tar checksum
 *
 * DESCRIPTION
 *
 *	Compute a signed tar checksum.  Some versions of tar produce archives
 *	whose checksums were computed using signed characters.
 *
 * PARAMETERS
 *
 *	char   *hdr		- pointer to tar header.
 *
 * RETURNS
 *
 * 	Returns the signed checksum of the header.
 */

static long signed_checksum(char *hdr)

{
    int i;
    long sum;

    sum = 0;
    for (i = 0 ; i < 500; i++) {
	/* Count chars 148 thru 155 (checksum) as spaces */
	if ((i < 148) || (i > 155))
	    sum += ((signed char) hdr[i]);
	else
	    sum += ' ';
    }
    return (sum);
}

/* print_entry - print a single table-of-contents entry
 *
 * DESCRIPTION
 * 
 *	Print_entry prints a single line of file information.  The format
 *	of the line is the same as that used by the LS command.  For some
 *	archive formats, various fields may not make any sense, such as
 *	the link count on tar archives.  No error checking is done for bad
 *	or invalid data.
 *
 * PARAMETERS
 *
 *	char   *name		- pointer to name to print an entry for
 *	Stat   *asb		- pointer to the stat structure for the file
 */


void print_entry(char *name, Stat *asb)
{
    /* If -o file_count command line option was used, then only the number  */
    /* of files restore is displayed by an increment of 100. The filename   */
    /* or verbose listing is not displayed. This option is used to speed up */
    /* the pax operation on certain display adapter such the Neptune card   */
    /* f_file_count is global extern uint defined in pax.c and pax.h        */

    if (f_file_count != 0) {
	if (f_file_count == 1) {
	    fprintf(msgfile,
		"Number of files restored (updated every %d files) :\n",
		f_file_count_incr);
	}
	if ((f_file_count % f_file_count_incr) == 0) {
	    fprintf(msgfile,"\r%6d ",f_file_count);
	    fflush(msgfile);
	}
	f_file_count++;
    } else {
	switch (ar_interface) {
	    case TAR:
		tar_entry(name, asb);
		break;
	    case CPIO:
		cpio_entry(name, asb);
		break;
	    case PAX:
		pax_entry(name, asb);
		break;
	}
    }
}


/* cpio_entry - print a verbose cpio-style entry
 *
 * DESCRIPTION
 *
 *	Print_entry prints a single line of file information.  The format
 *	of the line is the same as that used by the traditional cpio 
 *	command.  No error checking is done for bad or invalid data.
 *
 * PARAMETERS
 *
 *	char   *name		- pointer to name to print an entry for
 *	Stat   *asb		- pointer to the stat structure for the file
 */


static void cpio_entry(char *name, Stat *asb)

{
    struct tm	       *atm;
    Link	       *from;
    struct passwd      *pwp;
    struct group       *grp;
    char	       mon[4];		/* abreviated month name */

    if (f_list && f_verbose) {
	fprintf(msgfile, "%-7o", asb->sb_mode);
	atm = localtime(&asb->sb_mtime);
	if (pwp = getpwuid((int) USH(asb->sb_uid))) {
	    fprintf(msgfile, "%-6s", pwp->pw_name);
	} else {
	    fprintf(msgfile, "%-6u", USH(asb->sb_uid));
	}
	(void)strftime(mon, sizeof(mon), "%b", atm);
	fprintf(msgfile,"%7ld  %3s %2d %02d:%02d:%02d %4d  ",
	               asb->sb_size, mon, 
		       atm->tm_mday, atm->tm_hour, atm->tm_min, 
		       atm->tm_sec, atm->tm_year + 1900);
    }
    fprintf(msgfile, "%s", name);
    if ((asb->sb_nlink > 1) && (from = islink(name, asb))) {
	fprintf(msgfile, MSGSTR(LS_LINK, " linked to %s"), from->l_name);
    }
#ifdef	S_IFLNK
    if ((asb->sb_mode & S_IFMT) == S_IFLNK) {
	fprintf(msgfile, MSGSTR(LS_SYMLINK, " symbolic link to %s"), asb->sb_link);
    }
#endif	/* S_IFLNK */
    putc('\n', msgfile);
}


/* tar_entry - print a tar verbose mode entry
 *
 * DESCRIPTION
 *
 *	Print_entry prints a single line of tar file information.  The format
 *	of the line is the same as that produced by the traditional tar 
 *	command.  No error checking is done for bad or invalid data.
 *
 * PARAMETERS
 *
 *	char   *name		- pointer to name to print an entry for
 *	Stat   *asb		- pointer to the stat structure for the file
 */


static void tar_entry(char *name, Stat *asb)

{
    struct tm  	       *atm;
    int			i;
    int			mode;
    char               *symnam = "NULL";
    Link               *link;
    char		mon[4];		/* abbreviated month name */

    if ((mode = asb->sb_mode & S_IFMT) == S_IFDIR) {
	return;			/* don't print directories */
    }
    if (f_extract) {
	switch (mode) {
#ifdef S_IFLNK
	case S_IFLNK: 	/* This file is a symbolic link */
	    i = readlink(name, symnam, PATH_MAX - 1);
	    if (i < 0) {		/* Could not find symbolic link */
		warn(MSGSTR(LS_SYM, "can't read symbolic link"), strerror(errno));
	    } else { 		/* Found symbolic link filename */
		symnam[i] = '\0';
		fprintf(msgfile, MSGSTR(LS_XSYM, "x %s symbolic link to %s\n"), name, symnam);
	    }
	    break;
#endif
	case S_IFREG: 	/* It is a link or a file */
	    if ((asb->sb_nlink > 1) && (link = islink(name, asb))) {
		fprintf(msgfile, MSGSTR(LS_LINK2, "%s linked to %s\n"), name, link->l_name); 
	    } else {
		fprintf(msgfile, MSGSTR(LS_SUM, "x %s, %ld bytes, %d tape blocks\n"), 
			name, asb->sb_size, ROUNDUP(asb->sb_size, 
			BLOCKSIZE) / BLOCKSIZE);
	    }
	}
    } else if (f_append || f_create) {
	switch (mode) {
#ifdef S_IFLNK
	case S_IFLNK: 	/* This file is a symbolic link */
	    i = readlink(name, symnam, PATH_MAX - 1);
	    if (i < 0) {		/* Could not find symbolic link */
		warn(MSGSTR(LS_READ, "can't read symbolic link"), strerror(errno));
	    } else { 		/* Found symbolic link filename */
		symnam[i] = '\0';
		fprintf(msgfile, MSGSTR(LS_ASYM, "a %s symbolic link to %s\n"), name, symnam);
	    }
	    break;
#endif
	case S_IFREG: 	/* It is a link or a file */
	    fprintf(msgfile, "a %s ", name);
	    if ((asb->sb_nlink > 1) && (link = islink(name, asb))) {
		fprintf(msgfile, MSGSTR(LS_LINK3, "link to %s\n"), link->l_name); 
	    } else {
		fprintf(msgfile, MSGSTR(BLOCKS, "%ld Blocks\n"), 
			ROUNDUP(asb->sb_size, BLOCKSIZE) / BLOCKSIZE);
	    }
	    break;
	}
    } else if (f_list) {
	if (f_verbose) {
	    atm = localtime(&asb->sb_mtime);
	    (void)strftime(mon, sizeof(mon), "%b", atm);
	    print_mode((ushort)(asb->sb_mode));
	    fprintf(msgfile," %d/%d %6d %3s %2d %02d:%02d %4d %s",
		    asb->sb_uid, asb->sb_gid, asb->sb_size,
		    mon, atm->tm_mday, atm->tm_hour, 
		    atm->tm_min, atm->tm_year + 1900, name);
	} else {
	    fprintf(msgfile, "%s", name);
	}
	switch (mode) {
#ifdef S_IFLNK
	case S_IFLNK: 	/* This file is a symbolic link */
	    i = readlink(name, symnam, PATH_MAX - 1);
	    if (i < 0) {		/* Could not find symbolic link */
		warn(MSGSTR(LS_READ, "can't read symbolic link"), strerror(errno));
	    } else { 		/* Found symbolic link filename */
		symnam[i] = '\0';
		fprintf(msgfile, MSGSTR(LS_SYMLINK, " symbolic link to %s"), symnam);
	    }
	    break;
#endif
	case S_IFREG: 	/* It is a link or a file */
	    if ((asb->sb_nlink > 1) && (link = islink(name, asb))) {
		fprintf(msgfile, MSGSTR(LS_LINK, " linked to %s"), link->l_name);
	    }
	    break;		/* Do not print out directories */
	}
	fputc('\n', msgfile);
    } else {
	fprintf(msgfile, MSGSTR(LS_SUM2, "? %s %ld blocks\n"), name,
		ROUNDUP(asb->sb_size, BLOCKSIZE) / BLOCKSIZE);
    }
}


/* pax_entry - print a verbose pax-style entry
 *
 * DESCRIPTION
 *
 *	Print_entry prints a single line of file information.  The format
 *	of the line is the same as that used by the LS command.  
 *	No error checking is done for bad or invalid data.
 *
 * PARAMETERS
 *
 *	char   *name		- pointer to name to print an entry for
 *	Stat   *asb		- pointer to the stat structure for the file
 */


static void pax_entry(char *name, Stat *asb)

{
    struct tm	       *atm;
    Link	       *from;
    struct passwd      *pwp;
    struct group       *grp;
    char		mon[4];		/* abbreviated month name */
    time_t		six_months_ago;

    if (f_list && f_verbose) {
	print_mode((ushort)(asb->sb_mode));
	fprintf(msgfile, " %2d", asb->sb_nlink);
	atm = localtime(&asb->sb_mtime);
	six_months_ago = now - 6L*30L*24L*60L*60L; /* 6 months ago */
	(void) strftime(mon, sizeof(mon), "%b", atm);
	if (pwp = getpwuid((int) USH(asb->sb_uid))) {
	    fprintf(msgfile, " %-8s", pwp->pw_name);
	} else {
	    fprintf(msgfile, " %-8u", USH(asb->sb_uid));
	}
	if (grp = getgrgid((int) USH(asb->sb_gid))) {
	    fprintf(msgfile, " %-8s", grp->gr_name);
	} else {
	    fprintf(msgfile, " %-8u", USH(asb->sb_gid));
	}
	switch (asb->sb_mode & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
	    fprintf(msgfile, "\t%3d,%3d",
		           major(asb->sb_rdev), minor(asb->sb_rdev));
	    break;
	case S_IFREG:
	    fprintf(msgfile, "\t%7ld", asb->sb_size);
	    break;
	default:
		 fputs("\t       ",msgfile);
	}
	if ((asb->sb_mtime < six_months_ago) || (asb->sb_mtime > now)) {
	    fprintf(msgfile," %3s %2d  %4d ",
	            mon, atm->tm_mday, 
		    atm->tm_year + 1900);
	} else {
	    fprintf(msgfile," %3s %2d %02d:%02d ",
	            mon, atm->tm_mday, 
		    atm->tm_hour, atm->tm_min);
	}

	fputs(name,msgfile);

	if ((asb->sb_nlink > 1) && (from = islink(name, asb))) {
	    fprintf(msgfile, " == %s", from->l_name);
	}
#ifdef	S_IFLNK
	if ((asb->sb_mode & S_IFMT) == S_IFLNK) {
	    fprintf(msgfile, " -> %s", asb->sb_link);
	}
#endif	/* S_IFLNK */
    } else

	fputs(name,msgfile);

    putc('\n', msgfile);
}


/* print_mode - fancy file mode display
 *
 * DESCRIPTION
 *
 *	Print_mode displays a numeric file mode in the standard unix
 *	representation, ala ls (-rwxrwxrwx).  No error checking is done
 *	for bad mode combinations.  FIFOS, sybmbolic links, sticky bits,
 *	block- and character-special devices are supported if supported
 *	by the hosting implementation.
 *
 * PARAMETERS
 *
 *	ushort	mode	- The integer representation of the mode to print.
 */


static void print_mode(ushort mode)

{
    /* Tar does not print the leading identifier... */
    if (ar_interface != TAR) {
	switch (mode & S_IFMT) {
	case S_IFDIR: 
	    putc('d', msgfile); 
	    break;
#ifdef	S_IFLNK
	case S_IFLNK: 
	    putc('l', msgfile); 
	    break;
#endif	/* S_IFLNK */
	case S_IFBLK: 
	    putc('b', msgfile); 
	    break;
	case S_IFCHR: 
	    putc('c', msgfile); 
	    break;
#ifdef	S_IFIFO
	case S_IFIFO: 
	    putc('p', msgfile); 
	    break; 
#endif	/* S_IFIFO */ 
	case S_IFREG: 
	default:
	    putc('-', msgfile); 
	    break;
	}
    }
    putc((mode & 0400 ? 'r' : '-'), msgfile);
    putc((mode & 0200 ? 'w' : '-'), msgfile);
    putc((mode & 0100
	 ? (mode & 04000 ? 's' : 'x')
	 : (mode & 04000 ? 'S' : '-')), msgfile);
    putc((mode & 0040 ? 'r' : '-'), msgfile);
    putc((mode & 0020 ? 'w' : '-'), msgfile);
    putc((mode & 0010
	 ? (mode & 02000 ? 's' : 'x')
	 : (mode & 02000 ? 'S' : '-')), msgfile);
    putc((mode & 0004 ? 'r' : '-'), msgfile);
    putc((mode & 0002 ? 'w' : '-'), msgfile);
    putc((mode & 0001
	 ? (mode & 01000 ? 't' : 'x')
	 : (mode & 01000 ? 'T' : '-')), msgfile);
    putc(' ', msgfile);  /* "alternate access method flag" per POSIX.2 D12 */
}


/* from_oct - quick and dirty octal conversion
 *
 * DESCRIPTION
 *
 *	From_oct will convert an ASCII representation of an octal number
 *	to the numeric representation.  The number of characters to convert
 *	is given by the parameter "digs".  If there are less numbers than
 *	specified by "digs", then the routine returns -1.
 *
 * PARAMETERS
 *
 *	int digs	- Number to of digits to convert 
 *	char *where	- Character representation of octal number
 *
 * RETURNS
 *
 *	The value of the octal number represented by the first digs
 *	characters of the string where.  Result is -1 if the field 
 *	is invalid (all blank, or nonoctal). 
 *
 * ERRORS
 *
 *	If the field is all blank, then the value returned is -1.
 *
 */


static long from_oct(int digs, char *where)

{
    long            value;

    while (isspace(*where)) {	/* Skip spaces */
	where++;
	if (--digs <= 0) {
	    return(-1);		/* All blank field */
	}
    }
    value = 0;
    while (digs > 0 && ISODIGIT(*where)) {	/* Scan til nonoctal */
	value = (value << 3) | (*where++ - '0');
	--digs;
    }

    if (digs > 0 && *where && !isspace(*where)) {
	return(-1);		/* Ended on non-space/nul */
    }
    return(value);
}
