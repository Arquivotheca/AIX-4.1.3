/* @(#)66	1.6  src/bos/usr/bin/odmget/odmcmd.h, cmdodm, bos411, 9428A410j 4/18/91 21:17:59 */
/*
 * COMPONENT_NAME: odmcmd.h
 *
 * ORIGIN: IBM
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

#include <sys/dir.h>

#define STRSIZE 24*80  /* Max string assumed to be full screen */


#define CMD_EXIT(errstring,exitcode) if (cmddebug) \
    (void) fprintf (stderr,"errstring \n");  \
    (void) odm_terminate (); \
    (void) exit (exitcode);

#define P_EXIT(errstring,exitcode) if (cmddebug) \
    (void) perror ("errstring"); \
    (void) odm_terminate (); \
    (void) exit (exitcode);

#define ODM_EXIT(errstring,exitcode) if (cmddebug) \
    (void) fprintf (stderr,"errstring - odmerrno = %d\n",odmerrno);  \
    (void) odm_terminate (); \
    (void) exit (exitcode);


/*
 * ODM command exit codes.
 * If one of the commands exits with a ODMCMDOK (0) then the
 * command executed with no errors.  A nonzero value indicates
 * an error occured and the value identifies the type of error
 * found.
 */

#define ODMCMDOK       0  /* no errors */
#define ODMCMDNOCLASS  1  /* no class name specified */
#define ODMCMDOPTERR   2  /* command option error */
#define ODMCMDFNAME    3  /* file not accessable or not specified */
#define ODMCMDSYNTAX   4  /* input data syntax error */
#define ODMCMDODMERR   5  /* ODM call returned an error */
#define ODMCMDSYSERR   6  /* system call returned an error */
#define ODMCMDFATAL    7  /* odmcmd fatal internal error */
#define ODMCMDMALLOC   8  /* malloc error   */
#define ODMCMDCONVERT  9  /* data conversion error */
#define ODMCMDLONG     10 /* string too long */

#define ODMCMDACLIDLEN 5
