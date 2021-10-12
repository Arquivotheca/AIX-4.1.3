static char sccsid[] = "@(#)63	1.2  src/bos/usr/bin/pax/cpio.c, cmdarch, bos411, 9428A410j 6/26/91 13:06:22";
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
 * (C) COPYRIGHT International Business Machines Corp. 1991
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
/* $Source: /u/mark/src/pax/RCS/cpio.c,v $
 *
 * $Revision: 1.2 $
 *
 * cpio.c - Cpio specific functions for archive handling
 *
 * DESCRIPTION
 *
 *	These function provide a cpio conformant interface to the pax
 *	program.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
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
 * $Log:	cpio.c,v $
 * Revision 1.2  89/02/12  10:04:13  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:05  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: cpio.c,v 1.2 89/02/12 10:04:13 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Function Prototypes */

static void 	cpio_usage(void);


/* do_cpio - handle cpio format archives
 *
 * DESCRIPTION
 *
 *	Do_cpio provides a standard CPIO interface to the PAX program.  All
 *	of the standard cpio flags are available, and the behavior of the
 *	program mimics traditonal cpio.
 *
 * PARAMETERS
 *
 *	int	argc	- command line argument count
 *	char	**argv	- pointer to command line arguments
 *
 * RETURNS
 *
 *	Nothing.
 */


int do_cpio(int argc, char **argv)

{
    int             c;
    char           *dirname;
    Stat            st;

    /* default input/output file for CPIO is STDIN/STDOUT */
    ar_file = "-";
    names_from_stdin = 1;

    /* set up the flags to reflect the default CPIO inteface. */
    blocksize = BLOCKSIZE;
    ar_interface = CPIO;
    ar_format = CPIO;
    msgfile=stderr;

    while ((c = getopt(argc, argv, "D:Bacdfilmoprtuv")) != EOF) {
	switch (c) {
	case 'i':
	    f_extract = 1;
	    break;
	case 'o':
	    f_create = 1;
	    break;
	case 'p':
	    f_pass = 1;
	    dirname = argv[--argc];

	    /* check to make sure that the argument is a directory */
	    if (LSTAT(dirname, &st) < 0) {
		fatal(strerror(errno));
	    }
	    if ((st.sb_mode & S_IFMT) != S_IFDIR) {
		fatal(MSGSTR(NOTDIR, "Not a directory"));
	    }
	    break;
	case 'B':
	    blocksize = BLOCK;
	    break;
	case 'a':
	    f_access_time = 1;
	    break;
	case 'c':
	    break;
	case 'D':
	    ar_file = optarg;
	    break;
	case 'd':
	    f_dir_create = 1;
	    break;
	case 'f':
	    f_reverse_match = 1;
	    break;
	case 'l':
	    f_link = 1;
	    break;
	case 'm':
	    f_mtime = 1;
	    break;
	case 'r':
	    f_interactive = 1;
	    break;
	case 't':
	    f_list = 1;
	    break;
	case 'u':
	    f_unconditional = 1;
	    break;
	case 'v':
	    f_verbose = 1;
	    break;
	default:
	    cpio_usage();
	}
    }

    if (f_create + f_pass + f_extract != 1) {
	cpio_usage();
    }
    if (!f_pass) {
	buf_allocate((OFFSET) blocksize);
    }
    if (f_extract) {
	open_archive(AR_READ);	/* Open for reading */
	read_archive();
    } else if (f_create) {
	open_archive(AR_WRITE);
	create_archive();
    } else if (f_pass) {
	pass(dirname);
    }

    /* print out the total block count transfered */
    fprintf(stderr, MSGSTR(BLOCKS, "%ld Blocks\n"), ROUNDUP(total, BLOCKSIZE) / BLOCKSIZE);
    
    exit(0);
    /* NOTREACHED */
}


/* cpio_usage - print a helpful message and exit
 *
 * DESCRIPTION
 *
 *	Cpio_usage prints out the usage message for the CPIO interface and then
 *	exits with a non-zero termination status.  This is used when a user
 *	has provided non-existant or incompatible command line arguments.
 *
 * RETURNS
 *
 *	Returns an exit status of 1 to the parent process.
 *
 */


static void cpio_usage(void)

{
    fprintf(stderr, MSGSTR(CPIO_USAGE1, "Usage: %s -o[Bacv]\n"), myname);
    fprintf(stderr, MSGSTR(CPIO_USAGE2, "       %s -i[Bcdmrtuvf] [pattern...]\n"), myname);
    fprintf(stderr, MSGSTR(CPIO_USAGE3, "       %s -p[adlmruv] directory\n"), myname);
    exit(1);
}
