#ifndef lint
static char sccsid[] = "@(#)00  1.1  src/bos/usr/lbin/tty/crash/POWER/crash-rs.c, cmdtty, bos411, 9428A410j 3/3/94 10:16:11";
#endif
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27, 83 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <errno.h>
#include "unpack.h"

char *progname;

static void usage();
static void error(char *s);


main(int argc, char *argv[])
{
    long address;
    int verb;

    /* init globals */
    dump_mem = 0;
    readflag = 0;

    if (progname = strrchr(argv[0], '/'))
	++progname;
    else
	progname = argv[0];

    if (sscanf(argv[1], "%d", &dump_mem) != 1)
	usage();
    
    if (sscanf(argv[2], "%d", &readflag) != 1)
	usage();
    
    if (sscanf(argv[3], "%lx", &address) != 1)
	usage();

    if (sscanf(argv[4], "%d", &verb) != 1)
	usage();

    if (srs_print((void *)address, verb) < 0) 
	    error("print failed");
    
    exit(0);
}

/* need to change the usage message for these */
static void usage()
{
    fprintf(stderr, "Usage: %s kmem_fd [0|1] private data v\n", progname);
    exit(1);
}


static void error(char *s)
{
    extern int errno;

    if (errno) {
	fprintf(stderr, "%s: ", progname);
	perror(s);
    } else
	fprintf(stderr, "%s: %s\n", progname, s);
    exit(1);
}





