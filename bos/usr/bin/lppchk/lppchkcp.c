static char sccsid[] = "@(#)37  1.1  src/bos/usr/bin/lppchk/lppchkcp.c, cmdswvpd, bos411, 9428A410j 5/29/91 16:06:24";
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: lppchk (main)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: lppchkcp (Child process)
 *
 * FUNCTION: Set up and invoke 'ar' to read an archive member
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine will be invoked from the child process side of the
 *      fork operation done during verification of an archive member.
 *      It will set up standard out and standard error to used the pipes
 *      which have been opened and will then exec 'ar' to read the specified
 *      member from the archive, and send the data to standard out.
 *      The parent process will process that data and any data from the
 *      standard error pipe.
 *
 * INPUT:
 *      arch    - Pointer to the archive file name
 *      memb    - pointer to name of the member of the archive file.
 *      so_pipe - standard output pipe (pair of file descriptors)
 *      se_pipe - Standard error pipe (pair of file descriptors)
 *
 * OUTPUT:
 *      Normally output from ar will be directed to the output side of the
 *      pipes passed in and that data will be processed by the parent.
 *      If the exec fails, we will attempt to output a message.
 *
 * RETURNS: DOES NOT RETURN TO CALLER.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */


int lppchkcp(
  char  *arch ,                 /* pointer to archive file name         */
  char  *memb ,                 /* pointer to archive member name       */
  int   so_pipe[2] ,            /* descriptors for standard out         */
  int   se_pipe[2]              /* descriptors for standard error       */
  )

                                /* begin lppchkcp - child process       */

  {
  int           eno;            /* temporary copy of errno              */
  int           i;


  fclose (stdout);              /* swap standard output to pipe         */
  dup (so_pipe[WRITE_END]);

  fclose (stderr);              /* swap standard error to pipe          */
  dup (se_pipe[WRITE_END]);

  /* close all file descriptors that aren't needed */
  /* note that this closes the pipe file descriptors, except for fd 1 */
  /* and 2, which were dup'ed from the original pipe descriptors. */

  for (i = 3; i <= _NFILE; i++)
    close(i);

  /* invoke ar over this process with parameters to read the specified  */
  /* archive member and send data to standard out.                      */

  execl (ARCHIVE_PROG_PATH, ARCHIVE_PROG_NAME, ARCHIVE_PROG_ARG,
         arch, memb, 0);

  /* If we get to here, the exec failed.  Try our best to produce an    */
  /* error message                                                      */
  eno = errno;
  MSG_S(MSG_SEVERE,MSG_CHK_EXEC,DEF_CHK_EXEC,ARCHIVE_PROG_PATH,eno,0) ;

  exit (1);                     /* must not return - caller does not    */
                                /* want to see us ever again            */
  }                             /* end lppchkcp                         */
