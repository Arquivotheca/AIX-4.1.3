/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: add_hold_entries
 *		add_to_hold_file1001
 *		append_to_hold_file
 *		check_file
 *		check_lock
 *		cleanup_hold
 *		copy_admin
 *		copy_hold
 *		copy_log
 *		delete_from_hold_file1595
 *		filecp
 *		filename
 *		files_are_held
 *		hold_cleanup
 *		lock_for_edit
 *		set_lock
 *		set_lock2
 *		sort_defunct
 *		sort_snap
 *		unset_lock
 *		unset_lock2
 *		update_all_logs
 *		update_defunct
 *		update_logs
 *		update_snapshot
 *		update_submit_log1550
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: lockcmds.c,v $
 * Revision 1.11.12.8  1993/11/18  17:14:23  damon
 * 	CR 745. Fixed unlocking code
 * 	[1993/11/18  17:14:07  damon]
 *
 * Revision 1.11.12.7  1993/11/10  23:03:28  damon
 * 	CR 463. changed done=TRUE to done=FALSE
 * 	[1993/11/10  23:03:19  damon]
 * 
 * Revision 1.11.12.6  1993/11/10  18:44:39  root
 * 	CR 463. Pedantic changes
 * 	[1993/11/10  18:43:11  root]
 * 
 * Revision 1.11.12.5  1993/11/09  16:53:44  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:42  damon]
 * 
 * Revision 1.11.12.4  1993/11/08  17:58:45  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:33  damon]
 * 
 * Revision 1.11.12.3  1993/11/05  18:50:42  damon
 * 	CR 745. Fixed bad comment
 * 	[1993/11/05  18:49:46  damon]
 * 
 * Revision 1.11.12.2  1993/11/05  16:42:01  robert
 * 	Merged with changes from 1.11.12.1
 * 	[1993/11/05  16:41:50  robert]
 * 
 * 	clean up again
 * 	[1993/11/04  22:19:59  robert]
 * 
 * 	clean up
 * 	[1993/11/04  22:09:02  robert]
 * 
 * 	add call to unset_lock() for case where "if (submit)" false
 * 	in add_to_hold_file()
 * 	[1993/11/04  16:51:18  robert]
 * 
 * 	In both add_to_hold_file() and rename(), the order of calls
 * 	to rename() and unset_lock() were changexids so that if a
 * 	rename() occurs successfully, the call to unset_lock()
 * 	is not made. This is because rename(), in manipulating
 * 	the bsubmit.hold and bsubmit.log file, breaks the write
 * 	lock on the file, and a subsequent call to unset_lock()
 * 	can generate an error.
 * 
 * 	Added a check to unset_lock() so that in debug mode a
 * 	warning is printed out if thet target file to unlock
 * 	is already unlocked. This should help diagnose problems
 * 	like the above. This is apparently still happening with
 * 	bsubmit, though I'm not sure why.
 * 	[1993/10/28  20:38:42  robert]
 * 
 * Revision 1.11.12.1  1993/11/03  20:40:33  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:09  damon]
 * 
 * Revision 1.11.3.6  1993/06/11  14:35:20  damon
 * 	CR 588. Changed a HOLD_FILE to HOLD_FILE_23
 * 	[1993/06/11  14:35:04  damon]
 * 
 * Revision 1.11.3.5  1993/06/07  20:22:16  damon
 * 	CR 573. Added set_lock2 & unset_lock2 as meta-rcs locks
 * 	[1993/06/07  20:21:57  damon]
 * 
 * Revision 1.11.3.4  1993/05/25  15:09:04  damon
 * 	CR 395. Added HOLD_FILE_23
 * 	[1993/05/25  15:07:19  damon]
 * 
 * Revision 1.11.3.3  1993/05/21  18:07:17  damon
 * 	CR 395. Added lock_for_edit()
 * 	[1993/05/21  18:06:04  damon]
 * 
 * Revision 1.11.3.2  1993/05/11  21:27:42  damon
 * 	Submitting to correct problem with this file being outdated
 * 	[1993/05/11  21:27:26  damon]
 * 
 * Revision 1.11.6.26  1993/05/07  14:36:11  damon
 * 	CR 504. Check for EACCES after fnctl() too
 * 	[1993/05/07  14:36:01  damon]
 * 
 * Revision 1.11.6.25  1993/05/06  22:57:04  damon
 * 	CR 504. Set file writeable before fcntl()
 * 	[1993/05/06  22:52:02  damon]
 * 
 * Revision 1.11.6.24  1993/05/06  17:46:12  marty
 * 	Fix fcntl() bugs.
 * 	[1993/05/06  17:45:54  marty]
 * 
 * Revision 1.11.6.23  1993/05/06  15:59:26  marty
 * 	Correct argument call to fcntl().
 * 	[1993/05/06  15:59:09  marty]
 * 
 * Revision 1.11.6.22  1993/05/05  20:00:07  marty
 * 	Use fcntl instead of flock().
 * 	[1993/05/05  19:59:36  marty]
 * 
 * Revision 1.11.6.21  1993/05/05  18:40:22  damon
 * 	CR 473. files_are_held needed set passed in
 * 	[1993/05/05  18:40:15  damon]
 * 
 * Revision 1.11.6.20  1993/05/04  16:47:10  damon
 * 	CR 473. Fixed files_are_held
 * 	[1993/05/04  16:47:00  damon]
 * 
 * Revision 1.11.6.19  1993/05/03  21:30:08  damon
 * 	CR 473. Removed unneeded functions
 * 	[1993/05/03  21:29:59  damon]
 * 
 * Revision 1.11.6.18  1993/05/03  20:17:08  damon
 * 	CR 473. Simplified outdated functions
 * 	[1993/05/03  20:17:01  damon]
 * 
 * Revision 1.11.6.17  1993/04/29  21:20:00  marty
 * 	Remove STATIC from add_to_hold_file(), cleanup_hold(), and
 * 	check_file().
 * 	[1993/04/29  21:19:45  marty]
 * 
 * Revision 1.11.6.16  1993/04/29  20:51:29  marty
 * 	update_all_logs() is no longer STATIC.
 * 	[1993/04/29  20:51:12  marty]
 * 
 * Revision 1.11.6.15  1993/04/29  14:23:48  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  14:23:39  damon]
 * 
 * Revision 1.11.6.14  1993/04/14  19:17:48  damon
 * 	CR 193. Put strict locking code into serer side
 * 	[1993/04/14  19:17:33  damon]
 * 
 * Revision 1.11.6.13  1993/04/14  14:43:24  damon
 * 	CR 457. Keep flock on an open file desc.
 * 	[1993/04/14  14:43:12  damon]
 * 
 * Revision 1.11.6.12  1993/04/09  17:15:53  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:06  damon]
 * 
 * Revision 1.11.6.11  1993/04/08  18:42:55  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  18:41:51  damon]
 * 
 * Revision 1.11.6.10  1993/04/05  17:42:16  damon
 * 	CR 457. Removed refs to cleanup()
 * 	[1993/04/05  17:42:07  damon]
 * 
 * Revision 1.11.6.9  1993/04/05  17:26:29  damon
 * 	CR 457. Replaced lock dirs with flock calls
 * 	[1993/04/05  17:04:22  damon]
 * 
 * Revision 1.11.6.8  1993/03/17  16:13:06  damon
 * 	CR 436. Added code from logsubmit.c
 * 	[1993/03/17  16:02:46  damon]
 * 
 * Revision 1.11.6.7  1993/02/19  18:56:05  damon
 * 	CR 193. Added strict locking to lockcmds.c
 * 	[1993/02/19  18:55:18  damon]
 * 
 * Revision 1.11.6.6  1993/02/16  18:22:45  damon
 * 	CR 397. Generalized functions
 * 	[1993/02/16  18:21:42  damon]
 * 
 * Revision 1.11.6.5  1993/02/12  23:11:58  damon
 * 	CR 136. Hold file checking now done by logsubmit
 * 	[1993/02/12  23:08:28  damon]
 * 
 * Revision 1.11.6.4  1993/02/12  16:35:49  damon
 * 	CR 417. Move bsubmit.hold to rcs tree.
 * 	CR 193. Prepare for doing strict locking using bsubmit.hold
 * 	[1993/02/12  16:33:03  damon]
 * 
 * Revision 1.11.6.3  1993/01/14  16:30:45  damon
 * 	CR 397. Removed set_environ
 * 	[1993/01/14  16:30:30  damon]
 * 
 * Revision 1.11.6.2  1993/01/13  20:31:34  damon
 * 	CR 392. Moved lockcmds to lib/libode
 * 	[1993/01/13  20:31:03  damon]
 * 
 * Revision 1.11.2.5  1992/12/03  19:04:17  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:14  damon]
 * 
 * Revision 1.11.2.4  1992/10/27  20:10:12  damon
 * 	CR 290. Fixed defuncting problem.
 * 	[1992/10/27  20:09:50  damon]
 * 
 * Revision 1.11.2.3  1992/09/21  20:00:42  damon
 * 	CR 240. Converted to odexm
 * 	[1992/09/21  20:00:21  damon]
 * 
 * 	Converted to sci_rcs
 * 	[1992/09/09  20:22:12  damon]
 * 
 * Revision 1.11.2.2  1992/01/27  19:19:33  damon
 * 	Fixes bsubmit facet of bug 136.
 * 	[1992/01/27  18:47:07  damon]
 * 
 * Revision 1.11  1991/12/17  20:58:53  devrcs
 * 	Ported to hp300_hpux
 * 	[1991/12/17  14:30:38  damon]
 * 
 * Revision 1.10  1991/12/05  20:40:49  devrcs
 * 	Changed COPYHOLD to COPYFILE
 * 	[1991/10/30  21:48:48  damon]
 * 
 * 	Info switch now works better
 * 	[91/10/30  09:17:09  damon]
 * 
 * 	Since the 'files held' message was a level VWARN message,
 * 	if the user used -quiet, he would only see that bsubmit
 * 	exitted. He would not know why. The message has been changed
 * 	to level VALWAYS.
 * 	[91/10/29  15:02:41  damon]
 * 
 * 	lock_hold_file() and unlock_hold_file() are now boolean functions
 * 	[91/09/18  10:56:59  ezf]
 * 
 * 	don't attempt to unlink track file unless it exists
 * 	[91/09/13  15:19:57  ezf]
 * 
 * 	Now uses COPYHOLD function to read hold file.
 * 	[91/09/09  15:09:59  damon]
 * 
 * 	Changed to use new COPYHOLD command of logsubmit.
 * 	[91/09/05  15:46:03  damon]
 * 
 * 	now uses more descriptive error messages
 * 	[91/08/29  15:46:54  ezf]
 * 
 * 	Cleaned up append_to_hold_file.
 * 	Changed check_for_holds to be files_are_held.
 * 	Changed return value for files_are_held. FALSE->TRUE TRUE->FALSE.
 * 	[91/08/10  18:15:35  damon]
 * 
 * 	Putting all bsubmit code in synch
 * 	[91/08/05  17:07:26  damon]
 * 
 * 	Added more defuncting code. Changed to use SCI_ELEM/SCI_LIST
 * 	[91/08/02  16:56:42  damon]
 * 
 * 	First version using library version of SCAPI
 * 	[91/07/31  20:10:30  damon]
 * 
 * 	Changed check_for_holds and append_to_hold file to use new SCAPI
 * 	[91/07/15  14:49:44  damon]
 * 
 * 	Upgrade to Tools II and new user interface
 * 	[91/01/03  16:48:59  randyb]
 * 
 * 	Added ability to determine backing tree by using the rc variable
 * 	build_list
 * 	[90/12/07  13:53:18  randyb]
 * 
 * 	Put in changes to create and use file-by-file tracking log
 * 	[90/12/05  15:26:16  randyb]
 * 
 * 	This is essentially a new file though many of the routines have the
 * 	same names as the old ones.  The reason for the complete re-write is
 * 	to make all the log processing steps happen over the network.  The
 * 	same functionality is done but through a different method.
 * 	[90/11/29  15:01:37  randyb]
 * 
 * 	Style updates; saber c lint
 * 	[90/11/08  09:17:17  randyb]
 * 
 * Revision 1.8  90/10/07  21:50:21  devrcs
 * 	pre OSF1.0 changes
 * 
 * $EndLog$
*/
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                              Randy Barbano                                **
**                               June 1990                                   **
*******************************************************************************
**
**  Description:
**	These routines provide the commands to lock and unlock the
**	various files used in submitting.
**
 */

#ifndef lint
static char sccsid[] = "@(#)89  1.1  src/bldenv/sbtools/libode/lockcmds.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:20";
#endif /* not lint */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ode/errno.h>
#include <ode/interface.h>
#include <ode/lockcmds.h>
#include <ode/odedefs.h>
#include <ode/sci.h>
#include <ode/public/odexm_client.h>
#include <ode/util.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>


extern  int             errno;                         /* records error type */

#  define  WAITING      1                  /* return value for legal waiting */
#  define  NO_OPT       2             /* return value for doing no operation */
#  define  HELD         3     /* return value indicating that files are held */
#  define  MAX_ARGS     1
#  define  ONEWRITE     0600		/* mode for lock directories */

#  define  LOCAL_SNAP   "LOCAL_SNAP"         /* name of temp, local snapshot */
#  define  LOCAL_DEF    "LOCAL_DEF"      /* name of temp, local defunct file */
#  define  LOCAL_HOLD   "LOCAL_HOLD"        /* name of temp, local hold file */
#  define  LOCAL_LOG    "LOCAL_LOG"          /* name of temp, local log file */

/*
 * Prototypes
 */
int
add_to_hold_file ( char *, SCI_LIST, BOOLEAN );
int
cleanup_hold ( const char * , SCI_LIST, const char *, BOOLEAN );
STATIC char *
filename ( const char * string );
STATIC int
filecp ( const char * from, const char * to, const char * mode );
int
update_snapshot ( const char *, const char *, const char *, const char *,
                  BOOLEAN );
int
update_defunct ( const char *, const char *, const char * );
int
delete_from_hold_file ( const char * start, SCI_LIST, const char *, BOOLEAN );
int
update_submit_log ( const char *, const char *, const char * );
int
sort_snap ( FILE *, FILE *, FILE * ptr_df, FILE * ptr_tmp );
int
sort_defunct ( FILE *, FILE * ptr_up, FILE * ptr_tmp );

ERR_LOG
add_hold_entries ( const char * holdline, const char * hold_file,
                   const char * mode, SCI_LIST file_set, BOOLEAN submit )

	/* This procedure creates the user's hold file which contains
	   the list of files being submitted.  It then sends that
	   file to server and calls the routine to have the server
	   append the file to the build's hold file. */

{

    FILE      * ptr_uhold;                              /* misc file pointer */
  SCI_ELEM sci_ptr;

  ui_print ( VDEBUG, "Entering append_to_hold_file\n" );

  if (( ptr_uhold = fopen ( hold_file, mode )) == NULL ) {
    ui_print ( VFATAL, "Could not open user hold file: %s.\n",
            hold_file );
    return ( err_log ( OE_OPEN ) );
  } /* if */
  if ( submit ) {
    ui_print ( VDEBUG, "holdline :%s:\n", holdline );
    fprintf ( ptr_uhold, "%s\n", holdline );
  } /* if */
  for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr) ) {
    if ( submit ) 
      fprintf ( ptr_uhold, "%s\n", sci_ptr -> name );
    /* Don't add co locks to already co'd files */
    else if ( sci_ptr -> ver_user == NULL ) {
      fprintf ( ptr_uhold, "%s %s\n", holdline, sci_ptr -> name );
    } /* if */
  } /* for */
  fclose ( ptr_uhold );
  return ( OE_OK );
}

BOOLEAN
append_to_hold_file ( char * holdline, SCI_LIST file_set,
                      BOOLEAN submit, ERR_LOG * log )

	/* This procedure calls the routine to append the user's hold
	   file to the submission's hold file.  It does this by sending
	   the request to the server through runcmd. */


{
    int         status;                                      /* error report */
  BOOLEAN first = TRUE;
  BOOLEAN done=FALSE;
  BOOLEAN result=FALSE;

  if ( file_set == NULL ) {
    ui_print ( VFATAL, "list of files to lock is NULL.\n" );
    return ( FALSE );
  } /* end if */

  *log = OE_OK;
  while ( ! done ) {
  status = add_to_hold_file ( holdline, file_set, submit );
  switch ( status ) {
    case ERROR: 
      ui_print ( VFATAL, "Could not append to hold file.\n" );
      done = TRUE;
      result = FALSE;
      break;
    case OK:
      done = TRUE;
      result = TRUE;
      break;
    case HELD:
      done = TRUE;
      result = FALSE;
      break;
    case WAITING:
      if ( first ) {
        ui_print ( VNORMAL, "Lock file is temporarily unavailable.  Waiting" );
        first = FALSE;
      } /* if */
      ui_print ( VNORMAL, "." );
      fflush ( stdout );
      sleep ( LOCKWAIT );
      break;
    case NO_OPT: 
    default:
      ui_print ( VFATAL, "logsubmit exit status=%d while appending to hold file.\n%s\n",
			    status, NOTIFY );
      done = TRUE;
      result = FALSE;
      break;
  } /* switch */
  } /* while */
  return ( result );
}                                                        /* unlock log files */


BOOLEAN
update_all_logs (
    BOOLEAN defunct,     /* are any files defunct? */
    char * usr_snap,
    char * usr_defunct,
    char * usr_log )

	/* This function calls the server routine which updates all
	   the update logs: SNAPSHOT, DEFUNCT, and bsubmit.log.  This
	   routine also passes the user's log file to the server.  It
	   returns TRUE if all went well, FALSE otherwise. */


{
  int         status;                                      /* error report */
  OXM_CNXTN build_monitor;
  int i;
  ERR_LOG log;
  const char *av [16];
  BOOLEAN     locked = FALSE,                              /* misc boolean */
              first = TRUE;                                /* misc boolean */

  ui_print ( VDETAIL, "updating SNAPSHOT, DEFUNCT, and submit logs.\n" );

  oxm_open ( &build_monitor, 2 );
  i = 0;
  av [i++] = "-t1";
  av [i++] = "1";
  av [i++] = COPYLOG;
  av [i++] = usr_snap;
  av [i++] = ui_ver_switch ();
  log = oxm_runcmd ( build_monitor, i, av, NULL );
  log = oxm_endcmd ( build_monitor, &status );
  if ( defunct ) {
    i = 0;
    av [i++] = "-t1";
    av [i++] = "1";
    av [i++] = COPYLOG;
    av [i++] = usr_defunct;
    av [i++] = ui_ver_switch ();
    log = oxm_runcmd ( build_monitor, i, av, NULL );
    log = oxm_endcmd ( build_monitor, &status );
  } /* if */
  log = oxm_close ( build_monitor );

  if ( status == OK ) {
    while ( ! locked ) {
      oxm_open ( &build_monitor, 2 );
      i = 0;
      av [i++] = "-t1";
      av [i++] = "1";
      av [i++] = UPDATELOGS;
      av [i++] = usr_log;
      av [i++] = ui_ver_switch ();
      log = oxm_runcmd ( build_monitor, i, av, NULL );
      log = oxm_endcmd ( build_monitor, &status );
      log = oxm_close ( build_monitor );

      switch ( status ) {
        case ERROR: 
          return ( FALSE );
  
        case OK:
	  locked = TRUE;
          break;
  
        case WAITING:
          if ( first == TRUE ) {
            ui_print ( VNORMAL,
                       "Lock file is temporarily unavailable.  Waiting" );
            first = FALSE;
          } /* if */

          ui_print ( VNORMAL, "." );
          fflush ( stdout );
          sleep ( LOCKWAIT );
          break;

        case NO_OPT: 
        default:
          ui_print ( VFATAL, "logsubmit exit status=%d while updating log files.\n%s\n",
    			  status, NOTIFY );
          return ( FALSE );
      } /* switch */
    }
  } else
    return ( FALSE );
  /* if */
  return ( TRUE );
}                                                         /* update all logs */

int
hold_cleanup ( const char * holdline, SCI_LIST file_set,
               const char * submit_set, BOOLEAN submit )

	/* This procedure cleans up the hold file by calling the
	   server to remove the files held. */

{
    int         status;                                      /* error report */
  BOOLEAN first = TRUE;
  BOOLEAN done = FALSE;

  ui_print ( VDETAIL, "cleaning up hold file; removing entries.\n" );
  while ( !done ) {
      status = cleanup_hold ( holdline, file_set, submit_set, submit );
      switch ( status ) {
        case ERROR: 
          ui_print ( VFATAL, "submitted files may still be held.\n%s\n", NOTIFY );
          return ( ERROR );

        case OK:
          done = TRUE;
          break;

        case WAITING:
          if ( first == TRUE ) {
            ui_print ( VNORMAL,
                       "Lock file is temporarily unavailable.  Waiting" );
            first = FALSE;
          } /* if */

          ui_print ( VNORMAL, "." );
          fflush ( stdout );
          sleep ( LOCKWAIT );
          break;

        case NO_OPT: 
        default:
          ui_print ( VFATAL, "logsubmit exit status=%d while cleaning up hold file.\n%s\n",
    			 status, NOTIFY );
          ui_print ( VWARN, "submitted files are still held.\n" );
          return ( ERROR );
      } /* switch */
    } /* while */
  return ( OK );
}                                                            /* hold cleanup */

/* logsubmit */
int
check_file ( void )

	/* This procedure makes sure the file to append to is there. */

{
  if ( ui_entry_cnt ( ARGS_OP ) != 1 ) {
    ui_print ( VFATAL, "file to use is required argument.\n" );
    return ( NO_OPT );
  } /* if */

  if ( access ( ui_entry_value ( ARGS_OP, 1 ), R_OK ) != OK ) {
    ui_print ( VFATAL, "File to use, %s, not accessible.\n",
			ui_entry_value ( ARGS_OP, 1 ));
    return ( ERROR );
  } /* if */

  return ( OK );
}                                                              /* check file */



int 	check_lock ( const char * lockdir )

	/* This procedure sets up the lock type then tries to lock
	   the file.  If it succeeds, it returns OK, else ERROR. */

{
/*
 * FIXME: this is bogus, no waiting is done. What is really needed is
          a counting lock.
 */
    int         rval = WAITING;                              /* return value */

  if ( access ( lockdir, R_OK ) == ERROR ) {
    rval = OK;
  } /* if */

  return ( rval );
}                                                                /* set lock */

int
lock_for_edit ( const char * file )
{
  int fd;
  char buf[MAXPATHLEN];
  int status;

  if ( ( status = set_lock ( file, &fd ) ) != OK ) {
    ui_print ( VALWAYS, "locked\n" );
    fflush ( stdout );
    return ( status );
  } /* if */
  ui_print ( VALWAYS, "locking\n" );
  fflush ( stdout );
 /*
  * Wait for input (any input is fine) from calling process
  * before removing lock.
  */
  read ( 0, buf, sizeof(buf) );

  unset_lock ( file, fd );
  return ( OK );
} /* end lock_for_edit */

int
set_lock ( const char * file, int * fd )

	/* This procedure sets up the lock type then tries to lock
	   the file.  If it succeeds, it returns OK, else ERROR. */

{
  int rval = OK;                                             /* return value */
  struct flock flk_desc;
  struct flock *flk_ptr;

  flk_desc.l_type = F_WRLCK;
  flk_desc.l_whence    = 0;
  flk_desc.l_start     = 0;
  flk_desc.l_len       = 0;
  flk_ptr = &flk_desc;
  chmod ( file, S_IWUSR | S_IRUSR );
 /*
  * After the chmod, it is possible for another process to have already
  * gained the lock and performed an operation setting the file read only.
  * This will result in a O_WRONLY open to fail.
  */
  *fd = open ( file, O_WRONLY ); 
  if ( *fd == -1 && errno == EACCES )
    return ( WAITING );
  /* if */
  if ( fcntl ( *fd,  F_SETLK, flk_ptr ) == -1 ) {
    if ( errno == EWOULDBLOCK ) {            /* okay to err on existing lock */
      ui_print ( VDEBUG, "\tfile, %s, already locked.\n", file );
      rval = WAITING;
    } else if ( errno == EACCES )
      return ( WAITING );
    else {
      ui_print ( VFATAL, "could not lock: %s; error: %d.\n", file, errno );
      rval = ERROR;
    } /* else */
  } /* if */
  return ( rval );
}                                                                /* set lock */

int
set_lock2 ( const char * file, int * fd )
{
  char f [MAXPATHLEN];
  char d [MAXPATHLEN];
  char dbl_comma_file [MAXPATHLEN];

  path ( file, d, f );
  concat ( dbl_comma_file, sizeof(dbl_comma_file), d, "/,,", f, NULL );
  *fd = open ( dbl_comma_file, O_CREAT | O_EXCL, 0600 );
  if ( *fd == -1 ) {
    return ( WAITING );
  } else {
    return ( OK );
  } /* if */
} /* end set_lock2 */

int
unset_lock ( const char * file, int fd )

	/* This procedure sets the lock type then tries to unlock
	   the directory.  If it succeeds, it returns OK, else ERROR. */

{
  struct flock flk_desc;
  struct flock *flk_ptr;

  flk_desc.l_type = F_UNLCK;
  flk_desc.l_whence    = 0;
  flk_desc.l_start     = 0;
  flk_desc.l_len       = 0;
  flk_ptr = &flk_desc;

/*
 * The code in the following ifdef does not properly check the state
 * of the file and thus the unlock operation may never occur.
 */
#ifdef notdef
  /* add more debug information about state of the target file */
 
  if (fcntl( fd, F_GETLK, flk_ptr) < 0) {
    ui_print (VDEBUG, "could not check lock status on %s", file);
    return(ERROR);
  }

  if (flk_desc.l_type == F_WRLCK) {
    ui_print (VDEBUG, " ok to unlock - %s already locked\n", file);
  }

  /* if somehow the lock on the file was broken by some file operation, */
  /* print a warning, but allow OK return. For now assume this is OK */
  /* and allow an OK return until the locking scheme is improved (this */
  /* should really never happen */

  if (flk_desc.l_type == F_UNLCK) {
    ui_print (VDEBUG, " WARNING - %s was unlocked previous to unset_lock \n", file);
    return ( OK );
  }
#endif
  if ( fcntl ( fd, F_SETLK, flk_ptr ) == -1 ) {
    ui_print ( VFATAL, "could not unlock: %s, error: %d.\n", file, errno );
    return ( ERROR );
  } /* if */
  close (fd);
  return ( OK );
}                                                              /* unset lock */

int
unset_lock2 ( const char * file, int fd )

	/* This procedure sets the lock type then tries to unlock
	   the directory.  If it succeeds, it returns OK, else ERROR. */

{
  char f [MAXPATHLEN];
  char d [MAXPATHLEN];
  char dbl_comma_file [MAXPATHLEN];

  path ( file, d, f );
  concat ( dbl_comma_file, sizeof(dbl_comma_file), d, "/,,", f, NULL );
  close ( fd );
  if ( unlink ( dbl_comma_file ) == -1 ) {
    ui_print ( VFATAL, "could not unlock: %s, error: %d.\n", file, errno );
    return ( ERROR );
  } /* if */
  return ( OK );
}                                                              /* unset lock */


int
copy_log ( const char * copyfile )

	/* This function copys the file designated by the first argument
	   in the args list to the second argument.  It returns OK if
	   all went well, ERROR otherwise. */

{
    char      * chptr;                                 /* points to end name */

  if (( chptr = filename ( copyfile )) == NULL )
    return ( ERROR );

  if ( streq ( chptr, TMP_DEFUNCT ) || streq ( chptr, TMP_SNAP ))
    return ( filecp ( copyfile, chptr, WRITE ));

  else {
    ui_print ( VFATAL, "ERROR: file to copy, %s, is not a legal name.\n",
		        copyfile );
    return ( ERROR );
  } /* else */
}                                                                /* copy log */

int
copy_hold ( const char * holdfile, const char * copyfile )
        /* This function copys the file designated by the first argument
           in the args list to the second argument.  It returns OK if
           all went well, ERROR otherwise. */

{
  ui_print ( VDEBUG, "copy_hold: %s --> %s\n", holdfile, copyfile );

  if (( filename ( copyfile )) == NULL )
    return ( ERROR );

  return ( filecp ( holdfile, copyfile, WRITE ));
}                                                               /* copy hold */

STATIC char *
filename ( const char * string )

	/* This function returns the name of the file which is at the end
	   of string.  It accounts for the date stamp in front of the
	   name.  It returns NULL if anything goes wrong. */

{

    char      * ptr;                             /* points to name in string */

  if (( ptr = strrchr ( string, SLASH )) == NULL ) {
#ifdef notdef
    ui_print ( VFATAL, "ERROR: file to copy, %s, is not in legal directory.\n",
		       string );
    return (( char *) NULL );
#endif
    ptr = (char *)string;
  } /* if */
  if ( *(ptr + 3) == COLON )
    ptr += 7;                                /* advance beyond hh.mm. prefix */
  else
    ptr += 6;                                 /* advance beyond h.mm. prefix */

  ui_print ( VDEBUG, "filename is: %s.\n", ptr );
  return ( ptr );
}                                                                /* filename */



STATIC int
filecp ( const char * from, const char * to, const char * mode )

	/* This function appends the from file information to the to
	   file.  It returns OK if all went well, ERROR otherwise. */

{
    FILE      * ptr_to,                                 /* file to append to */
              * ptr_from;                                  /* file to append */

  if (( ptr_to = fopen ( to, mode )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not open file: %s, to copy to.\n", to );
    return ( ERROR );
  } /* if */

  if (( ptr_from = fopen ( from, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not open user file: %s.\n", from );
    return ( ERROR );
  } /* if */

  if ( ffilecopy ( ptr_from, ptr_to ) != OK ) {
    ui_print ( VFATAL, "file copy, mode %s failed: %s to %s.\n",
			mode, from, to );
    return ( ERROR );
  } /* if */

  fclose ( ptr_to );
  fclose ( ptr_from );
  return ( OK );
}                                                                  /* filecp */
  


int	copy_admin ( const char * fromfile )

	/* This function copies the file designated by the first argument
	   in the args list to the second argument.  It returns OK if
	   all went well, ERROR otherwise. */

{
    char      * chptr;                                 /* points to end name */

  if (( chptr = strrchr ( fromfile, SLASH )) == NULL ) {
    ui_print ( VFATAL, "ERROR: file to copy, %s, is not in legal directory.\n",
		        fromfile );
    return ( ERROR );
  } /* if */

  chptr++;
  ui_print ( VDEBUG, "filename is: %s.\n", chptr );
#ifdef notdef
  if ( streq ( chptr, DEFUNCT ) || streq ( chptr, SNAPSHOT ) ||
       streq ( chptr, SUBLOG ) || streq ( chptr, HOLD_FILE ))
#endif
    if ( strcmp ( chptr, HOLD_FILE ) == 0 ) {
      return ( filecp ( fromfile, HOLD_FILE_23, WRITE ));
    } else {
      return ( filecp ( fromfile, chptr, WRITE ));
    } /* if */
#ifdef notdef
  else {
    ui_print ( VFATAL, "ERROR: file to copy, %s, is not a legal name.\n",
		       fromfile );
    return ( ERROR );
  } /* else */
#endif
}                                                              /* copy admin */

STATIC BOOLEAN
files_are_held ( SCI_LIST file_set, int * status, BOOLEAN submit,
                 const char * set )

	/* This function compares the list of held files with the usrhold
           file to see if any of the files are already held */

{
  FILE      * real_hold;                           /* pointer to hold file */
  char        line [ PATH_LEN ];                            /* misc string */
  char * line_ptr;
  BOOLEAN files_held = FALSE;
  char hold_holder [ STRING_LEN ];       /* name and date of holder */
  char * token;
  char * user;
  char * locked_set;
  SCI_ELEM sci_ptr; 

  *status = OK;

  if (( real_hold = fopen ( HOLD_FILE_23, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not read from hold file: %s.\n",
			HOLD_FILE_23 );
    fclose ( real_hold );
    *status = ERROR;
    return ( TRUE );
  } /* if */

  while ( fgets ( line, PATH_LEN, real_hold ) != NULL ) {
    line[strlen(line) - 1] = NUL;
    if ( line[0] == ':' ) {
      if ( line[1] != ':' ) {
        strcpy ( hold_holder, line );
        continue;

      /* If a '::' line is encountered, then it is a co lock. If */
      /* hold_skip == 0, then this is a submission and co locks don't */
      /* need to be checked since this stage can't be reached if */
      /* the files aren't already checked-out. */
      } else if ( line[1] == ':' && !submit ) {
        line_ptr = line;
        token = nxtarg ( &line_ptr, WHITESPACE );
        locked_set = nxtarg ( &line_ptr, WHITESPACE );

        /* Is the lock for the same set? If not, don't check this lock. */
        if ( strcmp ( set, locked_set ) == 0 ) {
          user = nxtarg ( &line_ptr, WHITESPACE );
          token = nxtarg ( &line_ptr, WHITESPACE );
          for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
                sci_ptr = sci_next ( sci_ptr ) ) {
            if ( strcmp ( sci_ptr -> name, token ) == 0) {
              fprintf ( stderr, "File: %s\nis held by %s\n", token, user );
              files_held = TRUE;
              continue;
            } /* if */
          } /* for */
        } /* if */
      } /* if */
    } else
      for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
            sci_ptr = sci_next ( sci_ptr ) ) {
        if ( gmatch ( sci_ptr -> name, line ) ) {
          fprintf ( stderr, "File: %s\nis held by %s\n", line, hold_holder );
          files_held = TRUE;
          continue;
        } /* if */
      } /* for */
    /* if */
  } /* while */

  fclose ( real_hold );

  return ( files_held );
} /* files_are_held */

int
add_to_hold_file ( char * holdline, SCI_LIST file_set, BOOLEAN submit )

	/* This function appends the new hold file information to the
	   existing hold file.  It also unsets the lock on the hold
	   file.  It returns OK if all went well, ERROR otherwise. */


{
   int status;
   int lock_fd;
   ERR_LOG log;
   char * set_ptr;
   char * set_tmp;
   char * set;
 
   if ( (status = set_lock ( HOLD_FILE_23, &lock_fd ) ) != OK ) return ( status );
   set_tmp = strdup ( holdline );
   set_ptr = set_tmp;
   set = nxtarg ( &set_ptr, WHITESPACE ); /* skip first token */
   set = nxtarg ( &set_ptr, WHITESPACE );
   if (    !files_are_held (file_set, &status, submit, set )
        && status != ERROR ) { 
 
      free ( set_tmp );
 
      /* co locks go at the end of the file, submit locks */
      /* go at the beginning of the file */
 
      if ( submit ) {
 
         log = add_hold_entries ( holdline, LOCAL_HOLD, WRITE, file_set, submit );
         status = filecp ( HOLD_FILE_23, LOCAL_HOLD, APPEND );
         if ( status == ERROR ) { 
            status = unset_lock ( HOLD_FILE_23, lock_fd );
	    return (status);
         } /* if */
         status = rename ( LOCAL_HOLD, HOLD_FILE_23 );   /* breaking the lock */
         if ( status == ERROR ) {
            ui_print ( VFATAL, "ERROR: could not mv %s to %s.\n", LOCAL_HOLD, HOLD_FILE_23 );
         } /* if */
	 return(status);

      } else {

         log = add_hold_entries ( holdline, HOLD_FILE_23, APPEND, file_set, submit );
         status = unset_lock ( HOLD_FILE_23, lock_fd );
         return (status);

      } /* if */
 
   } else {

      free ( set_tmp );
      unset_lock ( HOLD_FILE_23, lock_fd );
      return ( HELD );
 
   } /* if */
}                                                        /* add to hold file */


int	update_logs ( const char * usrlog )

	/* This function updates the submissions logs.  There are three
	   logs which can be updated; the submission log and SNAPSHOT
	   file, both of which are always updated, and the DEFUNCT file
	   which is updated if is_defunct is TRUE.  If the logs are
	   updated successfully, this routine returns OK, else ERROR. */

{
    char      * name;                                  /* points to end name */
    BOOLEAN     isdefunct;                                   /* misc boolean */
    int status;
    int lock_fd;

  isdefunct = ( access ( TMP_DEFUNCT, R_OK ) == OK );

  if (( name = filename ( usrlog )) == NULL )
    return ( ERROR );

  ui_print ( VDEBUG, "opened tmp defunct file\n" );
  if ( ! streq ( name, TMP_LOG )) {
    ui_print ( VFATAL, "ERROR: user log file, %s, is not a legal name.\n",
		       usrlog );
    return ( ERROR );
  } /* if */

  if ( ( status = set_lock ( SUBLOG, &lock_fd ) ) != OK)
    return ( status );
  /* if */
  ui_print ( VDEBUG, "about to update snapshot file\n" );
  if ( update_snapshot ( SNAPSHOT, TMP_SNAP, LOCAL_SNAP, TMP_DEFUNCT, isdefunct)
			 == ERROR ) {
    unset_lock ( SUBLOG, lock_fd );
    return ( ERROR );
  } /* if */
  ui_print ( VDEBUG, "about to update defunct file\n" );
  if ( isdefunct ) {
    if ( update_defunct ( DEFUNCT, TMP_DEFUNCT, LOCAL_DEF ) == ERROR ) {
      unset_lock ( SUBLOG, lock_fd );
      return ( ERROR );
    } /* if */
  } /* if */

  ui_print ( VDEBUG, "about to update submit log\n" );
  if ( update_submit_log ( SUBLOG, usrlog, LOCAL_LOG ) == ERROR ) {
    unset_lock ( SUBLOG, lock_fd );
    return ( ERROR );
  }
  ui_print ( VDEBUG, "about to replace snapshot file\n" );
  if ( rename ( LOCAL_SNAP, SNAPSHOT ) == ERROR ) {
    ui_print ( VFATAL, "ERROR: could not mv %s to %s.\n", LOCAL_SNAP, SNAPSHOT);
    unset_lock ( SUBLOG, lock_fd );
    return ( ERROR );
  } /* if */

  ui_print ( VDEBUG, "about to replace defunct file\n" );
  if ( isdefunct ) {
    if ( rename ( LOCAL_DEF, DEFUNCT ) == ERROR ) {
      ui_print ( VFATAL, "ERROR: could not mv %s to %s.\n", LOCAL_DEF, DEFUNCT);
      unset_lock ( SUBLOG, lock_fd );
      return ( ERROR );
    } /* if */
  } /* if */

  ui_print ( VDEBUG, "about to remove tmp defunct file\n" );
  if ( access ( TMP_DEFUNCT, F_OK ) == OK ) {
    if ( unlink ( TMP_DEFUNCT ) == ERROR )
      ui_print ( VFATAL, "could not remove tmp log: %s.\n",
			 TMP_DEFUNCT );
  } /* if */

  ui_print ( VDEBUG, "about to remove tmp snapshot file\n" );
  if ( access ( TMP_SNAP, F_OK ) == OK ) {
    if ( unlink ( TMP_SNAP ) == ERROR )
      ui_print ( VFATAL, "could not remove tmp log: %s.\n", TMP_SNAP );
  } /* if */

  ui_print ( VDEBUG, "about to replace submit log file\n" );
  status = rename ( LOCAL_LOG, SUBLOG );
  if (status == ERROR ) {
    ui_print ( VFATAL, "ERROR: could not mv %s to %s.\n", LOCAL_LOG, SUBLOG );
    unset_lock ( SUBLOG, lock_fd );
    return ( ERROR );
  } /* if */
  /* and the rename() call has broken the lock */

  ui_print ( VDEBUG, "done updating logs, unlocking\n" );
  return (status);
}                                                             /* update logs */



int
update_snapshot ( const char * snapshot_f, const char * update_f,
                       const char * tmp_f, const char * defunct_f,
                       BOOLEAN defunct )

	/* This function attempts to update the SNAPSHOT file.
	   It returns OK if the update and snapshot files were
	   merged and moved to SNAPSHOT, ERROR if not. */

{
    FILE      * ptr_ss,                          /* pointer to snapshot file */
              * ptr_df = NULL,                    /* pointer to defunct file */
              * ptr_up,                            /* pointer to update file */
              * ptr_tmp;                              /* pointer to tmp file */
   int          rvalue;                                      /* return value */

  if (( ptr_ss = fopen ( snapshot_f, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not read SNAPSHOT file: %s.\n",
			snapshot_f );
    return ( ERROR );
  } /* if */

  if (( ptr_up = fopen ( update_f, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not read user snapshot file: %s.\n",
		        update_f );
    return ( ERROR );
  } /* if */

  if (( ptr_tmp = fopen ( tmp_f, WRITE )) == NULL ) {
    ui_print ( VFATAL,
	   "ERROR: could not write to temporary snapshot file: %s.\n", tmp_f );
    return ( ERROR );
  } /* if */

  if ( defunct ) {
    if (( ptr_df = fopen ( defunct_f, READ )) == NULL ) {
      ui_print ( VFATAL, "ERROR: could not read defunct file: %s.\n",
			  defunct_f );
      return ( ERROR );
    } /* if */

    rvalue = sort_snap ( ptr_ss, ptr_up, ptr_df, ptr_tmp );
  } /* if */

  else
    rvalue = sort_snap ( ptr_ss, ptr_up, (FILE *) NULL, ptr_tmp );

  fclose ( ptr_ss );
  if ( defunct ) {
    fclose ( ptr_df );
  }
  fclose ( ptr_up );
  fclose ( ptr_tmp );
  return ( rvalue );
}                                                         /* update snapshot */



int
sort_snap ( 
    FILE      * ptr_ss,                          /* pointer to snapshot file */
    FILE      * ptr_up,                            /* pointer to update file */
    FILE      * ptr_df,                           /* pointer to defunct file */
    FILE      * ptr_tmp )                             /* pointer to tmp file */


	/* This function merges the SNAPSHOT file with the new
	   changes.  It does it by assuming each file is an ascii
	   sorted list.  It is all in one long procedure to save
	   time; SNAPSHOT is a long file.  If everything works,
	   it returns OK, ERROR otherwise. */

{
    char        lnss [ STRING_LEN ],                          /* misc string */
                lnup [ STRING_LEN ],                          /* misc string */
                lndf [ STRING_LEN ],                          /* misc string */
              * chss,                                   /* points to each ch */
              * chnew;                                  /* points to each ch */
    BOOLEAN     endmerge,                                    /* misc boolean */
                next_line,      /* -1 if line from defunct, 1 if from update */
                endss = FALSE,                               /* misc boolean */
                endup = FALSE,                               /* misc boolean */
                enddf = FALSE;                               /* misc boolean */

  ui_print ( VDEBUG, "sorting snapshot file\n" );
  if ( fgets ( lnss, STRING_LEN, ptr_ss ) == NULL ) {
    ui_print ( VFATAL, "SNAPSHOT file is empty.\n" );
    return ( ERROR );
  } /* if */

  if ( fgets ( lnup, STRING_LEN, ptr_up ) == NULL )
    endup = TRUE;

  if ( ptr_df == NULL || fgets ( lndf, STRING_LEN, ptr_df ) == NULL )
    enddf = TRUE;

  if ( endup && enddf ) {
    ui_print ( VFATAL, "ERROR: user update and defunct files are empty.\n" );
    return ( ERROR );
  } /* if */

  if ( ! endup ) {                                   /* more lines in update */
    if (( enddf ) || ( strcmp ( lnup, lndf ) < 0 ))
      next_line = 1;
    else
      next_line = -1;
  } /* if */

  else if ( enddf )                               /* no more lines in either */
    next_line = 0;
  else                                              /* lines left in defunct */
    next_line = -1;

  endmerge = ( endss ) || ( next_line == 0 );

  while ( ! endmerge ) {
    chss = lnss;

    if ( next_line == 1 )
      chnew = lnup;
    else
      chnew = lndf;

    while (( *chss == *chnew ) &&                      /* check char by char */
	   (( *chss != TAB ) && ( *chss != SPACE ))) {
      chss++;
      chnew++;
    } /* while */

    if ( *chss == *chnew ) {                           /* first fields match */
      if ( next_line == 1 ) {         /* put in update line and get two more */
        if ( fputs ( lnup, ptr_tmp ) == EOF ) {
	  ui_print ( VFATAL, "ERROR: fputs: %d, in updating SNAPSHOT(1).\n",
			      errno );
	  return ( ERROR );
	} /* if */
      } /* if */
      /* the else here would be to put in lndf but defunct lines are skipped */

      if ( fgets ( lnss, STRING_LEN, ptr_ss ) == NULL )
        endss = TRUE;

      if ( next_line == 1 ) {
	if ( fgets ( lnup, STRING_LEN, ptr_up ) == NULL )
	  endup = TRUE;
      } /* if */

      else if ( next_line == -1 ) {
	if ( fgets ( lndf, STRING_LEN, ptr_df ) == NULL )
	  enddf = TRUE;
      } /* else */

      else {
	ui_print ( VFATAL, "ERROR: reached impossible point(1).\n" );
	return ( ERROR );
      } /* else */
    } /* if */

    else if ( *chss < *chnew ) {                /*  line not in update files */
      if ( fputs ( lnss, ptr_tmp ) == EOF ) {
	ui_print ( VFATAL, "ERROR: fputs: %d, in updating SNAPSHOT(2).\n",
			    errno );
	return ( ERROR );
      } /* if */

      if ( fgets ( lnss, STRING_LEN, ptr_ss ) == NULL )
        endss = TRUE;
    } /* if */

    else if ( next_line == 1 ) {         /* update line not in snapshot file */
      if ( fputs ( lnup, ptr_tmp ) == EOF ) {
	ui_print ( VFATAL, "ERROR: fputs: %d, in updating SNAPSHOT(3).\n",
			    errno );
	return ( ERROR );
      } /* if */

      if ( fgets ( lnup, STRING_LEN, ptr_up ) == NULL )
	endup = TRUE;
    } /* else */
    
    else if ( next_line == -1 ) {       /* defunct line not in snapshot file */
      if ( fgets ( lndf, STRING_LEN, ptr_df ) == NULL )
	enddf = TRUE;
    } /* else if */

    else {
      ui_print ( VFATAL, "ERROR: reached impossible point(2).\n" );
      return ( ERROR );
    } /* else */

    if ( ! endup ) {                                 /* more lines in update */
      if (( enddf ) || ( strcmp ( lnup, lndf ) < 0 ))
	next_line = 1;
      else
	next_line = -1;
    } /* if */

    else if ( enddf )                             /* no more lines in either */
      next_line = 0;
    else                                            /* lines left in defunct */
      next_line = -1;

    endmerge = ( endss ) || ( next_line == 0 );
  } /* while */

  if ( endup && ! endss ) {            /* finish putting one of the files in */
    if ( fputs ( lnss, ptr_tmp ) == EOF ) {
      ui_print ( VFATAL, "ERROR: fputs: %d, in updating SNAPSHOT(4).\n", errno);
      return ( ERROR );
    } /* if */

    if ( ffilecopy ( ptr_ss, ptr_tmp ) != OK ) {
      ui_print ( VFATAL, "ERROR: file copy error: %s to %s.\n", ptr_ss,
			  ptr_tmp );
      return ( ERROR );
    } /* if */
  } /* if */

  else if ( ! endup ) {
    if ( fputs ( lnup, ptr_tmp ) == EOF ) {
      ui_print ( VFATAL, "ERROR: fputs: %d, in updating SNAPSHOT(5).\n", errno);
      return ( ERROR );
    } /* if */
    
    if ( ffilecopy ( ptr_up, ptr_tmp ) != OK ) {
      ui_print ( VFATAL, "ERROR: file copy error: %s to %s.\n", ptr_up,
			  ptr_tmp );
      return ( ERROR );
    } /* if */
  } /* else if */

  return ( OK );
}                                                               /* sort snap */



int	update_defunct ( 
    const char * defunct_f,                             /* defunct file name */
    const char * update_f,                               /* update file name */
    const char * tmp_f )                              /* temporary file name */


	/* This function attempts to update the DEFUNCT file.
	   It returns OK if the update and defunct files were
	   merged and moved to DEFUNCT, ERROR otherwise. */

{
    FILE      * ptr_df,                           /* pointer to defunct file */
              * ptr_up,                            /* pointer to update file */
              * ptr_tmp;                              /* pointer to tmp file */
    int         rvalue;                                      /* return value */

  if (( ptr_df = fopen ( defunct_f, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not read defunct file: %s.\n", defunct_f );
    return ( ERROR );
  } /* if */

  if (( ptr_up = fopen ( update_f, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not read file user defunct file: %s.\n",
		        update_f );
    return ( ERROR );
  } /* if */

  if (( ptr_tmp = fopen ( tmp_f, WRITE )) == NULL ) {
    ui_print ( VFATAL,
	     "ERROR: could not write to temporary defunct file: %s.\n", tmp_f );
    return ( ERROR );
  } /* if */

  rvalue = sort_defunct ( ptr_df, ptr_up, ptr_tmp );
  fclose ( ptr_df );
  fclose ( ptr_up );
  fclose ( ptr_tmp );
  return ( rvalue );
}                                                          /* update defunct */



int
sort_defunct ( 
    FILE      * ptr_df,                           /* pointer to defunct file */
    FILE      * ptr_up,                            /* pointer to update file */
    FILE      * ptr_tmp )                             /* pointer to tmp file */


	/* This function merges the DEFUNCT file with the new changes.
	   It does it by assuming each file is an ascii sorted list.
	   It is all in one long procedure to save time; DEFUNCT is a
	   long file.  If the merge works, it returns OK, else ERROR. */

{
    char        lndf [ STRING_LEN ],                          /* misc string */
                lnup [ STRING_LEN ],                          /* misc string */
              * chdf,                                   /* points to each ch */
              * chup;                                   /* points to each ch */
    BOOLEAN     more = TRUE,                                 /* misc boolean */
                enddf = FALSE,                               /* misc boolean */
                endup = FALSE;                               /* misc boolean */

  if ( fgets ( lndf, STRING_LEN, ptr_df ) == NULL ) {
    enddf = TRUE;
    more = FALSE;
  } /* if */

  if ( fgets ( lnup, STRING_LEN, ptr_up ) == NULL ) {
    endup = TRUE;
    more = FALSE;
  } /* if */

  while ( more ) {
    chdf = lndf;
    chup = lnup;

    while (( *chdf == *chup ) &&                       /* check char by char */
	   (( *chdf != TAB ) && ( *chdf != SPACE ))) {
      chdf++;
      chup++;
    } /* while */

    if ( *chdf == *chup ) {                            /* first fields match */
      if ( fputs ( lnup, ptr_tmp ) == EOF ) {
				      /* put in update line and get two more */
        ui_print ( VFATAL, "ERROR: fputs: %d, in updating DEFUNCT(1).\n",
			    errno );
	return ( ERROR );
      } /* if */

      if ( fgets ( lndf, STRING_LEN, ptr_df ) == NULL ) {
        more = FALSE;
        enddf = TRUE;
      } /* if */

      if ( fgets ( lnup, STRING_LEN, ptr_up ) == NULL ) {
        more = FALSE;
        endup = TRUE;
      } /* if */

    } /* if */

    else if ( *chdf < *chup ) {
      if ( fputs ( lndf, ptr_tmp ) == EOF ) {
        ui_print ( VFATAL, "ERROR: fputs: %d, in updating DEFUNCT(2).\n",
			    errno );
	return ( ERROR );
      } /* if */

      if ( fgets ( lndf, STRING_LEN, ptr_df ) == NULL ) {
        more = FALSE;
        enddf = TRUE;
      } /* if */
    } /* else if */

    else {
      if ( fputs ( lnup, ptr_tmp ) == EOF ) {
        ui_print ( VFATAL, "ERROR: fputs: %d, in updating DEFUNCT(3).\n",
			    errno );
	return ( ERROR );
      } /* if */

      if ( fgets ( lnup, STRING_LEN, ptr_up ) == NULL ) {
        more = FALSE;
        endup = TRUE;
      } /* if */
    } /* else */
  } /* while */

  if ( endup  && ! enddf ) {           /* finish putting one of the files in */
    if ( fputs ( lndf, ptr_tmp ) == EOF ) {
      ui_print ( VFATAL, "ERROR: fputs: %d, in updating DEFUNCT(4).\n", errno );
      return ( ERROR );
    } /* if */

    if ( ffilecopy ( ptr_df, ptr_tmp ) != OK ) {
      ui_print ( VFATAL, "ERROR: file copy error: %s to %s.\n", ptr_df,
			  ptr_tmp );
      return ( ERROR );
    } /* if */
  } /* if */

  else if ( ! endup ) {
    if ( fputs ( lnup, ptr_tmp ) == EOF ) {
      ui_print ( VFATAL, "ERROR: fputs: %d, in updating DEFUNCT(5).\n", errno );
      return ( ERROR );
    } /* if */

    if ( ffilecopy ( ptr_up, ptr_tmp ) != OK ) {
      ui_print ( VFATAL, "ERROR: file copy error: %s to %s.\n", ptr_up,
			  ptr_tmp );
      return ( ERROR );
    } /* if */
  } /* else if */

  return ( OK );
}                                                            /* sort defunct */



int	update_submit_log (
    const char * submitlog,                       /* original submission log */
    const char * usrlog,                      /* user's added submission log */
    const char * tmplog )                               /* new log to create */


	/* This function appends the user's log to the bsubmit log.
	   It returns OK if that operation was successful, ERROR if not. */

{
  if ( filecp ( submitlog, tmplog, WRITE ) == OK )
    return ( filecp ( usrlog, tmplog, APPEND ));
  else
    return ( ERROR );
}                                                       /* update submit log */



int
cleanup_hold ( 
  const char * string,                       /* string with info to pass on */
  SCI_LIST file_set, const char * submit_set, BOOLEAN submit ) 

	/* This function locks the hold file, removes entries from
	   it, then unlocks it.  It does no real work itself.  It
	   returns OK or ERROR. */

{
  int         rvalue;                                      /* return value */
  int lock_fd;

  if (( rvalue = set_lock ( HOLD_FILE_23, &lock_fd )) == OK ) {
    if (( rvalue = delete_from_hold_file ( string, file_set, submit_set,
                                           submit )) == OK )
      rvalue = unset_lock ( HOLD_FILE_23, lock_fd );
    else	/* don't change rvalue if delete_from_hold_file() failed */
      (void) unset_lock ( HOLD_FILE_23, lock_fd );
  } /* if */

  return ( rvalue );
}                                                            /* cleanup hold */



int
delete_from_hold_file ( const char * start, SCI_LIST file_set,
                        const char * submit_set, BOOLEAN submit )

	/* This function removes the list of held files from the
	   hold file.  It returns OK or ERROR depending on the
	   results. */

{
  FILE      * real_hold,                           /* pointer to hold file */
            * tmp_hold;                             /* pointer to tmp file */
  char        line [ PATH_LEN ];                            /* misc string */
  char        line_tmp [ PATH_LEN ];                        /* misc string */
  BOOLEAN     found_start = FALSE;                         /* misc boolean */
  char * token;
  char * line_ptr;
  char * locked_set;
  char * user;
  BOOLEAN match = FALSE;
  char * res;
  SCI_ELEM sci_ptr;

  if (( tmp_hold = fopen ( LOCAL_HOLD, WRITE )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not write to temporary hold file: %s.\n",
		        LOCAL_HOLD );
    return ( ERROR );
  } /* if */

  if (( real_hold = fopen ( HOLD_FILE_23, READ )) == NULL ) {
    ui_print ( VFATAL, "ERROR: could not read from hold file: %s.\n",
			HOLD_FILE );
    fclose ( tmp_hold );
    return ( ERROR );
  } /* if */

  while (( ! found_start ) &&
	 (( fgets ( line, PATH_LEN, real_hold )) != NULL )) {
    if ( *line == COLON ) {
      line[strlen ( line ) - 1] = NUL;

      if ( line[1] != ':' ) {
        if ( streq ( line, start ))
          found_start = TRUE;
        else
          fprintf ( tmp_hold, "%s\n", line );
        /* if */
      } else if ( line[1] == ':' ) {
        strcpy ( line_tmp, line );
        line_ptr = line_tmp;
        token = nxtarg ( &line_ptr, WHITESPACE );
        locked_set = nxtarg ( &line_ptr, WHITESPACE );

        /* Is the lock for the same set? If not, don't check this lock. */
        if ( strcmp ( submit_set, locked_set ) == 0 ) {
          user = nxtarg ( &line_ptr, WHITESPACE );
          token = nxtarg ( &line_ptr, WHITESPACE );
          match = FALSE;
          for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
                sci_ptr = sci_next ( sci_ptr ) ) {
            if ( strcmp ( sci_ptr -> name, token ) == 0 ) {
              match = TRUE;
            } /* if */
          } /* for */
        } /* if */
        if ( ! match )
          fprintf ( tmp_hold, "%s\n", line );
        /* if */
      } else
        fprintf ( tmp_hold, "%s\n", line );
      /* if */
    } /* if */

    else
      fputs ( line, tmp_hold );
  } /* while */

  if ( submit && !found_start ) {
    ui_print ( VWARN, "No startline, %s, in hold file: %s.\n",
		       start, HOLD_FILE_23 );
    fclose ( real_hold );
    fclose ( tmp_hold );
    ( void ) unlink ( LOCAL_HOLD );
    return ( ERROR );
  } /* if */

  while ( (res = fgets ( line, PATH_LEN, real_hold )) != NULL &&
          *line != COLON );                        /* skip until next user */

  while ( res != NULL ) {
    if ( *line == COLON ) {
      line[strlen ( line ) - 1] = NUL;

      if ( line[1] != ':' ) {
        fprintf ( tmp_hold, "%s\n", line );
      } else if ( line[1] == ':' ) {
        strcpy ( line_tmp, line );
        line_ptr = line_tmp;
        token = nxtarg ( &line_ptr, WHITESPACE );
        locked_set = nxtarg ( &line_ptr, WHITESPACE );

        /* Is the lock for the same set? If not, don't check this lock. */
        if ( strcmp ( submit_set, locked_set ) == 0 ) {
          user = nxtarg ( &line_ptr, WHITESPACE );
          token = nxtarg ( &line_ptr, WHITESPACE );
          match = FALSE;
          for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
                sci_ptr = sci_next ( sci_ptr ) ) {
            if ( strcmp ( sci_ptr -> name, token ) == 0 ) {
              match = TRUE;
            } /* if */
          } /* for */
        } /* if */
        if ( ! match )
          fprintf ( tmp_hold, "%s\n", line );
        /* if */
      } else
        fprintf ( tmp_hold, "%s\n", line );
      /* if */
    } else
      fputs ( line, tmp_hold );
    /* if */
    res = fgets ( line, PATH_LEN, real_hold );
  } /* while */

  fclose ( real_hold );
  fclose ( tmp_hold );

  if ( rename ( LOCAL_HOLD, HOLD_FILE_23 ) == ERROR ) {
    ui_print ( VFATAL, "ERROR: could not mv %s to %s.\n", LOCAL_HOLD,
			HOLD_FILE_23 );
    return ( ERROR );
  } /* if */

  return ( OK );
}                                                   /* delete from hold file */
