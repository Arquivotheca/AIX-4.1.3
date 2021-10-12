/* @(#)49       1.1  src/bldenv/sbtools/include/ode/lockcmds.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:00
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: none
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
 * $Log: lockcmds.h,v $
 * Revision 1.1.5.2  1993/11/09  16:53:33  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:35  damon]
 *
 * Revision 1.1.5.1  1993/11/08  17:58:28  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:20  damon]
 * 
 * Revision 1.1.2.7  1993/05/03  20:16:22  damon
 * 	CR 436. Updated prototypes
 * 	[1993/05/03  20:16:15  damon]
 * 
 * Revision 1.1.2.6  1993/04/29  14:22:42  damon
 * 	CR 463. Added prototypes
 * 	[1993/04/29  14:22:31  damon]
 * 
 * Revision 1.1.2.5  1993/04/08  16:19:20  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  16:18:39  damon]
 * 
 * Revision 1.1.2.4  1993/03/15  22:08:33  damon
 * 	CR 376. sci_rcs.h changed to sci.h
 * 	[1993/03/15  22:08:15  damon]
 * 
 * Revision 1.1.2.3  1993/02/12  23:11:53  damon
 * 	CR 136. Added HELD for logsubmit/lockcmds
 * 	[1993/02/12  23:10:00  damon]
 * 
 * Revision 1.1.2.2  1993/01/13  20:45:03  damon
 * 	CR 392. Moved here from ./bin/bsubmit
 * 	[1993/01/13  20:44:01  damon]
 * 
 * Revision 1.12.4.4  1992/12/03  19:04:14  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:34:12  damon]
 * 
 * Revision 1.12.4.3  1992/09/21  19:51:15  damon
 * 	Converted to sci_rcs
 * 	[1992/09/21  19:50:47  damon]
 * 
 * Revision 1.12.4.2  1992/06/11  19:46:42  damon
 * 	Synched with 2.1.1
 * 	[1992/06/11  19:44:16  damon]
 * 
 * Revision 1.12.2.2  1992/03/23  22:58:42  damon
 * 	Added -tag
 * 	[1992/03/23  22:56:31  damon]
 * 
 * Revision 1.12  1991/12/05  20:40:45  devrcs
 * 	Changed TUTORIAL to ./ode/tutorial
 * 	[1991/11/26  18:39:54  damon]
 * 
 * 	Remove no_merge, no_build_ci, and no_build_co
 * 	[1991/08/16  13:52:41  damon]
 * 
 * 	Added SUBMIT_DONE define
 * 	[91/08/14  13:28:08  damon]
 * 
 * 	Added update_logs field to set_info struct.
 * 	[91/08/10  18:09:38  damon]
 * 
 * 	Putting all bsubmit code in synch
 * 	[91/08/05  17:07:19  damon]
 * 
 * 	Added more defuncting code. Changed to use SCI_ELEM/SCI_LIST
 * 	[91/08/02  16:56:36  damon]
 * 
 * 	First version using library version of SCAPI
 * 	[91/07/31  20:10:19  damon]
 * 
 * 	Merging up
 * 	[91/07/31  19:46:56  damon]
 * 
 * 	now #includes "ode/versions.h"
 * 	[91/07/24  09:55:46  ezf]
 * 
 * 	changes for optional cmd line arguments
 * 	[91/07/15  12:27:32  ezf]
 * 
 * 	Added changes to support RIOS and aix
 * 	[91/01/22  12:59:39  mckeen]
 * 
 * 	Changed sdm to ode
 * 	[91/01/09  17:58:11  randyb]
 * 
 * 	Changed includes to to ode/ and ode/odedefs.
 * 	[91/01/09  15:33:08  randyb]
 * 
 * 	Upgrade to Tools II and new user interface
 * 	[91/01/03  16:48:30  randyb]
 * 
 * 	Added ability to determine backing tree by using the rc variable
 * 	build_list
 * 	[90/12/07  13:53:09  randyb]
 * 
 * 	Put in changes to create and use file-by-file tracking log
 * 	[90/12/05  15:26:10  randyb]
 * 
 * 	Changes to accomodate writing logs over the network.
 * 	[90/11/29  14:58:49  randyb]
 * 
 * 	Added signal.h
 * 	[90/11/15  11:43:35  randyb]
 * 
 * 	Style updates; saber c lint
 * 	[90/11/08  09:17:08  randyb]
 * 
 * $EndLog$
*/
/******************************************************************************
**                          Open Software Foundation                         **
**                              Cambridge, MA                                **
**                              Randy Barbano                                **
**                               April 1990                                  **
*******************************************************************************
**
**  Description:
**	This header file is for bsubmit.c, bcscall.c lockcmds.c, sbutil.c,
**	and sadmin.c.
**
 */
/*                       INCLUDES					     */
#include <stdio.h>
#include <ode/odedefs.h>
#include <ode/public/error.h>
#include <ode/sci.h>
#include <sys/param.h>

/*                       DEFINES					     */

#  define  ALLWRITE	0666
#  define  MEWRITE	0600

#  define  LOCKWAIT	3

#  define  LOCKHOLDF    0                  /* modes to do locking, unlocking */
#  define  LOCKLOGF	1

#  define  EDITHOLD     0                    /* modes to do copying, editing */
#  define  EDITLOG	1
#  define  EDITSNAP	2
#  define  EDITDEF	3

#  define  WAITING      1                  /* return value for legal waiting */
#  define  NO_OPT       2             /* return value for doing no operation */
#  define  HELD		3		              /* some files are held */
#  define  BCSSTEPS	"bcssteps"	/* part of name of file tracking bcs */
#  define  BLOCALRC     "rc_files/bsub.local"  /* name of bsubmit sb rc file */
#  define  LOCAL	"local"  		       /* default sb rc file */
#  define  LOGS         "logs"                                  /* directory */
#  define  LOGSUB	"logsubmit"  	        /* program running on server */
#  define  NO_EDIT      "cat"                  /* just cat the file, no edit */
#  define  NOTIFY	"Please Notify Release Engineering" /* call for help */
#  define  TUTORIAL     "./ode/tutorial/"             /* tutorial area */
#  define  TUT_CT	strlen ( TUTORIAL )

/*                       GLOBAL DECLARATIONS				     */

       /* structure to hold information about the set to submit from and to. */

    typedef struct set_info {
      char      b_name [ NAME_LEN ],                    /* name of the build */
                b_rcfile [ PATH_LEN ],     /* path and name to build rc file */
                b_base [ PATH_LEN ],                    /* path to the build */
                l_base [ PATH_LEN ],                 /* path to the local sb */
                b_src_dir [ PATH_LEN ],       /* path to the build's src dir */
                l_src_dir [ PATH_LEN ],     /* path to local's src directory */
                b_set [ NAME_LEN ],                 /* name of the build set */
                l_set [ NAME_LEN ],            /* full name of the local set */
                setdir [ PATH_LEN ],              /* directory for local set */
                org_dir [ PATH_LEN ],   /* path to user's starting directory */
                submit_cover [ PATH_LEN ],          /* remote submit program */
                submit_host [ NAME_LEN ],            /* machine to submit to */
                submit_base [ PATH_LEN ],      /* remote dir to submit into  */
                submit_owner [ NAME_LEN ],       /* remote owner submissions */
                user [ NAME_LEN ],                       /* name of the user */
                date [ NAME_LEN ],                            /* date to use */
                time [ NAME_LEN ],                            /* time to use */
                tdate [ NAME_LEN ],                  /* tracking date to use */
                ttime [ NAME_LEN ];                  /* tracking time to use */
      FILE    * tracks;                     /* pts to file with tracks in it */
      int       co_mode;                    /* how to do the local check out */
      BOOLEAN   update_snap,                   /* should snapshot be updated */
                update_logs,                   /* should snapshot be updated */
                defunct,                          /* are there defunct files */
                changed,                          /* are there changed files */
                defect_num,            /* should defect numbers be asked for */
		full_list,  		  /* use full list for next bcs step */
                cmerge,                            /* is change merge needed */
                dmerge,                           /* is defunct merge needed */
                do_locking,
                do_merging,
                do_checking_out,
                do_updating,
                cci,                           /* is changed check in needed */
                dci,                           /* is defunct check in needed */
                cco,                          /* is changed check out needed */
                dco;                          /* is defunct check out needed */
    } SETINFO;

				 /* structure to hold command argument array */

    typedef struct table {
      int       argc;                      /* number of arguments + MAX_ARGS */
      int       size;                             /* size of allocated table */
      char   ** argv;                            /* argument list + MAX_ARGS */
    } TABLE;

					  /* structure to hold list of files */

    typedef struct submittal {
      TABLE     changedfiles;                     /* Files that have changed */
      TABLE     defunctfiles;                    /* Files that are vanishing */
      TABLE     allfiles;                                /* All of the above */
    } SUBMITTAL;

				/* structure to hold pairs of users and revs */

    typedef struct pairs {
      char      user [ NAME_LEN ],           /* holds name of user with lock */
                rev [ NAME_LEN ];             /* holds name of rev with lock */
      struct    pairs  * nextpair;                    /* pts to next pairing */
    } PAIRS;

			       /* structure to hold lists of users and locks */

    typedef struct lock_list {
      char      filename [ PATH_LEN ];          /* holds name of file locked */
      PAIRS   * pairing;                     /* holds list of users and revs */
      struct    lock_list  * nextfile;           /* pts to next file in list */
    } LOCKS;

				/* structure to hold the shared set of files */

    typedef struct shared_files {
      char    * usr_log,                           /* string with user's log */
              * usr_snap,                            /* user's snapshot file */
              * usr_defunct,                          /* user's defunct file */
              * usr_hold,                                /* user's hold file */
              * holdfile,                              /* build's hold files */
              * trackfile,                         /* file with tracks in it */
	      * trackbcsstep,		      /* file to track each bcs step */
              * holdline;                         /* line which starts holds */
    } SHARED_F;

BOOLEAN
append_to_hold_file ( char *, SCI_LIST,
                      BOOLEAN, ERR_LOG * );
int
hold_cleanup ( const char * holdline, SCI_LIST, const char *, BOOLEAN );

int
set_lock ( const char * , int * );

int
unset_lock ( const char * file, int fd );

int
set_lock2 ( const char * , int * );
int
unset_lock2 ( const char * , int );
int
copy_hold ( const char * holdfile, const char * copyfile );
int
lock_for_edit ( const char * file );
int
copy_log ( const char * copyfile );
int     copy_admin ( const char * fromfile );
int
add_to_hold_file ( char * holdline, SCI_LIST file_set, BOOLEAN submit );
int     update_logs ( const char * usrlog );
int
cleanup_hold ( const char * string, SCI_LIST file_set, const char * submit_set,
               BOOLEAN submit );
int     check_lock ( const char * lockdir );
int
check_file ( void );
BOOLEAN
update_all_logs (
    BOOLEAN defunct,
    char * usr_snap,
    char * usr_defunct,
    char * usr_log );
