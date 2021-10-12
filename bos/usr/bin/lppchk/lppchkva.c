static char sccsid[] = "@(#)42  1.3  src/bos/usr/bin/lppchk/lppchkva.c, cmdswvpd, bos411, 9428A410j 7/17/91 17:36:47";
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
 * NAME: lppchkva (verify archive member)
 *
 * FUNCTION: Check size and optionally checksum of archive member specified in
 *      the inventory record passed.  If needed and requested update the
 *      size and checksum fields in the inventory record structure
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Verify that the size of the member specified in the inventory
 *      matches the size value in that structure.  Values in the structure
 *      of 0 or -1 will 'match' any real size.  If update is requested
 *      and the size in the structure not -1 (indicating no size value
 *      should be computed), the structure will be updated and the return
 *      code will be set to indicate that the inventory needs to be updated.
 *      If the option is 'c', then the checksum will be computed and verified
 *      and updated as was the size value.
 *
 * INPUT:
 *      opt     - char with value f (fast, size check only) or c (full
 *                checksum validation).
 *      inv     - pointer to inventory structure with file information.
 *      updt    - boolean indicating if update is to be performed
 *      ecnt    - pointer to error counter. To be incremented if an
 *                error is found with the specified file.
 *
 * OUTPUT:
 *      Inventory structure will be updated with size and checksum if
 *      needed.  Error messages as appropriate.
 *
 * RETURNS: CHK_OK - no errors, no need to update inventory
 *          CHK_UPDT - no errors, inventory needs to be updated
 *          CHK_BAD_RET - file size or checksum error, or system
 *                        error.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */


int lppchkva (
  char  opt ,                   /* option letter, 'f' or 'c'            */
  inv_t *inv ,                  /* inventory structure with file data   */
  int   updt ,                  /* boolean, update requested if needed  */
  int   *ecnt )                 /* error counter                        */

  {                             /* begin lppchkvf - verify single file  */

  int   va_rc,                  /* return code from this program        */
        rc,                     /* hold system routine return code      */
        sum,                    /* computed file checksum               */
        szx,                    /* computed file size                   */
        eno ;                   /* copy of errno for printing           */

  char  arcname[2*MAX_INV_LOC0+2] ;
                                /* hold archive:member form of name     */

  int   c ;                     /* character of stderr data from ar     */
  int   stdout_pipe[2],         /* access arrays for pipes              */
        stderr_pipe[2];
  FILE  *fp ;                   /* stdio file control pointer           */
/*----------------------------------------------------------------------*/
  TRC(TRC_ENTRY,"lppchkvf entry - file = %s\n",inv->loc0,0,0);

  va_rc = CHK_OK ;              /* set default return code              */

  rc = access(inv->loc1,R_ACC) ;/* check if we have access to archive   */

  if (rc == -1)
    {
    if (errno == EACCES)        /* determine specific error to report   */
      {
      MSG_S(MSG_ERROR, MSG_CHK_NOPERM, DEF_CHK_NOPERM, inv->loc1, 0, 0);
      }
    else
      {
      MSG_S(MSG_ERROR, MSG_CHK_NOFILE, DEF_CHK_NOFILE, inv->loc1, 0, 0);
      }                         /* end report specific error            */
    *ecnt += 1 ;
    va_rc = CHK_BAD_RET ;       /* indicate error occurred              */
    }                           /* end if error in access request       */
  else
    {                           /* access to archive ok, set up pipes   */

    if ((pipe (stdout_pipe) != 0) ||
        (pipe (stderr_pipe) != 0))
      {                         /* error opening the pipe               */
      eno = errno;
      MSG_S(MSG_SEVERE,MSG_CHK_NOPIPE, DEF_CHK_NOPIPE, eno,0,0) ;
      va_rc = CHK_BAD_RET ;     /* return code indicates error          */
      }                         /* if error opening pipes               */
    else
      {                         /* pipe opened successfully, now fork   */

/************************************************************************/
/* now fork to process archive member, child process will pipe the data */
/* from the member back to this process by invoking 'ar' to send the    */
/* member data to standard output.                                      */
/************************************************************************/

      switch (fork())
        {
        case -1:                /* fork failed */
          eno = errno;
          MSG_S(MSG_SEVERE, MSG_CHK_NOFORK, DEF_CHK_NOFORK, eno, 0,0);
          close (stdout_pipe[READ_END]);
          close (stdout_pipe[WRITE_END]);
          close (stderr_pipe[READ_END]);
          close (stderr_pipe[WRITE_END]);
          va_rc = CHK_BAD_RET ; /* cleanup pipes, record error, exit    */
          return(va_rc) ;       /* return to caller now                 */

        case 0:                 /* child process */
          /* never returns, guaranteed */
          lppchkcp(inv->loc1, inv->loc0,
                   stdout_pipe, stderr_pipe);
        }                       /* end switch on fork return code       */


/************************************************************************/
/* parent process falls out the bottom of the switch statement          */
/************************************************************************/

      close (stdout_pipe[WRITE_END]);
      close (stderr_pipe[WRITE_END]);

/************************************************************************/
/* create a FILE * descriptor corresponding to the readable end of      */
/* the pipe that will be connected to the child process's standard      */
/* output.                                                              */
/************************************************************************/

      fp = fdopen (stdout_pipe[READ_END], "r");

/************************************************************************/
/* Read all the bytes out of the standard output pipe, and compute      */
/* their checksum.                                                      */
/************************************************************************/

      sum = lppchkck(fp,&szx);  /* go compute the checksum value        */
      fclose(fp) ;              /* done with this pipe, close it        */

      strncpy(arcname, inv->loc0, sizeof(inv->loc0)) ;
      strcat(arcname,":");
      strncat(arcname, inv->loc1, sizeof(inv->loc0)) ;
                                /* build up a member:archive name       */

      if ((szx != inv->size) &&
          (inv->size != -1))    /* if size needs checking and differs   */
        {
        if (updt)
          {
          MSG_S(MSG_INFO, MSG_CHK_NEWSZ, DEF_CHK_NEWSZ, arcname,
              inv->size, szx) ;
          inv->size = szx ;
          va_rc = CHK_UPDT ;    /* report change, set struct, set ret   */
          }
        else
          {
          MSG_S(MSG_ERROR, MSG_CHK_BADSZ, DEF_CHK_BADSZ, arcname,
              szx, inv->size) ;
          va_rc = CHK_BAD_RET ;
          *ecnt += 1 ;          /* report error, count it, set return   */
          }                     /* end - is update requested for size   */
        }                       /* end - size is bad                    */

      if ((sum != inv->checksum) &&
          (inv->checksum != -1))/* if cksum needs checking and differs  */
        {
        if (updt)
          {
          MSG_S(MSG_INFO, MSG_CHK_NEWSUM, DEF_CHK_NEWSUM, arcname,
              inv->checksum, sum) ;
          inv->checksum = sum ;
          va_rc = CHK_UPDT ;    /* report change, set struct, set ret   */
          }
        else
          {
          MSG_S(MSG_INFO, MSG_CHK_BADCK, DEF_CHK_BADCK, arcname,
              sum, inv->checksum) ;
          va_rc = CHK_BAD_RET ;
          *ecnt += 1 ;          /* report error, count it, set return   */
          }                     /* end - is update requested for cksum  */
        }                       /* end - cksum is bad                   */

/************************************************************************/
/* Create a FILE * descriptor corresponding to the readable end of      */
/* the pipe that is connected to the child process's error output.      */
/************************************************************************/

      fp = fdopen (stderr_pipe[READ_END], "r");

/************************************************************************/
/* If any text is available from the child's error output, print an     */
/* introduction message, and copy the text to our error output          */
/************************************************************************/

      if ((c = getc (fp)) != EOF)
        {                       /* if any error output from ar          */
                                /* identify that output                 */
        MSG_S(MSG_ERROR, MSG_CHK_ARERR, DEF_CHK_ARERR,
            ARCHIVE_PROG_PATH, 0,0) ;

        while (c != EOF)        /* copy the error data to local stderr  */
          {
          putc (c, stderr);
          c = getc (fp);
          }                     /* end read and print error data from ar*/
        }                       /* end if any error data from 'ar'      */

      fclose (fp);              /* close the pipe file descriptor       */

/************************************************************************/
/* "wait" for the child process to die, so that it does not float       */
/* around in the system as a zombie, taking up a process slot.          */
/************************************************************************/

      wait ((int *) 0);

      }                         /* end - pipes opened successfully      */
    }                           /* end - no error on access call        */

  TRC(TRC_EXIT,"lppchkva exit - return code = %d\n", va_rc, 0,0);
  return (va_rc) ;
  }                             /* end lppchkva                         */
