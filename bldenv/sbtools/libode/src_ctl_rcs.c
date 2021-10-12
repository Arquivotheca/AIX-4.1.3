/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ATOI
 *		checkancestor
 *		for
 *		get_ancestor
 *		get_ancestry
 *		get_branch
 *		getancestor
 *		getancestor1
 *		getancestor2
 *		is_base_of_branch2134
 *		is_revision
 *		numfields
 *		rcs_cmp_func
 *		rcsfaststat
 *		rcsfullstat
 *		set_ancestor
 *		src_ctl_add_ancestry1831
 *		src_ctl_add_ancestry21855
 *		src_ctl_add_symbol
 *		src_ctl_ancestor1132
 *		src_ctl_check_in1160
 *		src_ctl_check_out
 *		src_ctl_check_out_with_fd
 *		src_ctl_create_branch1329
 *		src_ctl_create_branch21456
 *		src_ctl_diff_rev_with_file
 *		src_ctl_fast_lookup_revision
 *		src_ctl_file_exists1780
 *		src_ctl_get_ancestry1799
 *		src_ctl_lock_revision1260
 *		src_ctl_lookup_revision
 *		src_ctl_outdate
 *		src_ctl_remove_file
 *		src_ctl_remove_symbol
 *		src_ctl_show_diffs
 *		src_ctl_show_log1645
 *		src_ctl_undo_create
 *		src_ctl_unlock_revision1292
 *		tip_revision
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
 * COPYRIGHT NOTICE
 * Copyright (c) 1993, 1992, 1991, 1990
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
 *
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU
 * School of Computer Science
 * Carnegie Mellon University
 * Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log: src_ctl_rcs.c,v $
 * Revision 1.1.8.8  1993/11/19  21:32:21  damon
 * 	CR 844. Fixed ancestry comparisons
 * 	[1993/11/19  21:31:18  damon]
 *
 * Revision 1.1.8.7  1993/11/10  16:57:08  root
 * 	CR 463. Cast stdrup paramater to (char *)
 * 	[1993/11/10  16:56:10  root]
 * 
 * Revision 1.1.8.6  1993/11/08  17:58:59  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:44  damon]
 * 
 * Revision 1.1.8.5  1993/11/05  22:43:41  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  22:41:48  damon]
 * 
 * Revision 1.1.8.4  1993/10/26  22:50:05  damon
 * 	CR 752. Give more detail
 * 	[1993/10/26  22:49:44  damon]
 * 
 * Revision 1.1.8.3  1993/10/21  15:40:47  damon
 * 	CR 756. More correct way to parse ancestry/branches info
 * 	[1993/10/21  15:40:35  damon]
 * 
 * Revision 1.1.8.2  1993/10/20  15:01:27  damon
 * 	CR 754. Added 1 to length of ancestry malloc 7 -> 8
 * 	[1993/10/20  15:01:11  damon]
 * 
 * Revision 1.1.8.1  1993/10/12  21:08:26  damon
 * 	Merged from 2.3.2 branch
 * 	[1993/10/12  21:08:14  damon]
 * 
 * Revision 1.1.6.7  1993/10/12  21:03:04  damon
 * 	CR 735. Compares of revs should be int to int not char to char
 * 	[1993/10/12  21:01:31  damon]
 * 
 * Revision 1.1.6.6  1993/10/06  21:38:49  damon
 * 	CR 729. Deal with trunk revisions properly
 * 	[1993/10/06  21:38:35  damon]
 * 
 * Revision 1.1.6.5  1993/09/16  17:36:35  damon
 * 	Fixed COPYRIGHT NOTICE section
 * 	[1993/09/16  17:35:13  damon]
 * 
 * Revision 1.1.6.4  1993/08/31  21:25:36  damon
 * 	CR 641. Take care of case where all files are being defuncted
 * 	[1993/08/31  21:25:06  damon]
 * 
 * Revision 1.1.6.3  1993/08/30  19:12:18  damon
 * 	CR 633. Restored writing to file for src_ctl_diff_rev_with_file
 * 	[1993/08/30  19:11:20  damon]
 * 
 * Revision 1.1.6.2  1993/08/30  17:25:50  root
 * 	CR 634. Added 1 to length of final_ancestry
 * 	[1993/08/30  17:25:33  root]
 * 
 * Revision 1.1.6.1  1993/08/30  16:36:29  damon
 * 	CR 634. Ancestry initial value set properly
 * 	[1993/08/30  16:36:15  damon]
 * 
 * Revision 1.1.4.1  1993/08/19  18:35:26  damon
 * 	CR 622. Changed assignment to tempslots
 * 	[1993/08/19  18:33:48  damon]
 * 
 * Revision 1.1.2.13  1993/06/08  19:00:54  marty
 * 	CR # 476 - Add routine src_ctl_remove_file().
 * 	[1993/06/08  18:56:44  marty]
 * 
 * Revision 1.1.2.12  1993/06/07  20:22:20  damon
 * 	CR 573. Added set_lock2 & unset_lock2 as meta-rcs locks
 * 	[1993/06/07  20:22:02  damon]
 * 
 * Revision 1.1.2.11  1993/06/03  13:57:35  damon
 * 	CR 69. Cleaned up src_ctl_show_diffs. No longer sends file over
 * 	[1993/06/03  13:57:28  damon]
 * 
 * Revision 1.1.2.10  1993/06/02  17:51:10  damon
 * 	CR 563. No longer enter Initial Revision marker during submit of new file
 * 	[1993/06/02  17:50:07  damon]
 * 
 * Revision 1.1.2.9  1993/06/02  16:11:44  damon
 * 	CR 560. Removed debugging code
 * 	[1993/06/02  16:11:38  damon]
 * 
 * Revision 1.1.2.8  1993/06/02  16:10:50  damon
 * 	CR 560. Fixed problem with 1.1 rev
 * 	[1993/06/02  16:10:30  damon]
 * 
 * Revision 1.1.2.7  1993/06/01  21:03:30  damon
 * 	CR 560. Extended ancestry for more general case
 * 	[1993/06/01  21:03:05  damon]
 * 
 * Revision 1.1.2.6  1993/05/13  16:34:16  marty
 * 	CR# 516 - Found a bug in is_revision (not regignizing trunk revisions).
 * 	[1993/05/13  16:34:04  marty]
 * 
 * Revision 1.1.2.5  1993/05/13  15:30:27  damon
 * 	CR 525. Fixed ancestry for 1.1 case
 * 	[1993/05/13  15:30:17  damon]
 * 
 * Revision 1.1.2.4  1993/05/12  19:42:18  marty
 * 	CR # 480 - fix ancestry updating for outdates of single revisions.
 * 	[1993/05/12  19:34:45  marty]
 * 
 * 	CR # 480 - Support for "bcs -o -r", change src_Ctl_outdate() to allow
 * 	outdating of single revision.
 * 	[1993/05/12  16:26:42  marty]
 * 
 * Revision 1.1.2.3  1993/05/11  17:34:55  marty
 * 	CR # 480 - Add "av[i]=NULL;" to src_ctl_add_symbol().
 * 	[1993/05/11  17:34:43  marty]
 * 
 * Revision 1.1.2.2  1993/05/11  17:32:00  marty
 * 	File got zapped!  Putting it back from the backing tree.
 * 	[1993/05/11  17:31:44  marty]
 * 
 * Revision 1.1.2.51  1993/05/10  20:50:50  damon
 * 	CR 178. Extended ancestry calculation
 * 	[1993/05/10  20:48:21  damon]
 * 
 * Revision 1.1.2.50  1993/05/10  16:56:27  damon
 * 	CR 512. made branch_buf[] bigger
 * 	[1993/05/10  16:54:20  damon]
 * 
 * Revision 1.1.2.49  1993/05/07  21:37:29  damon
 * 	CR 511. Removed debuging code
 * 	[1993/05/07  21:37:18  damon]
 * 
 * Revision 1.1.2.48  1993/05/07  21:32:52  damon
 * 	CR 511. Cleaned up ancestry logic
 * 	[1993/05/07  21:31:58  damon]
 * 
 * Revision 1.1.2.47  1993/05/07  20:08:00  marty
 * 	CR # 506, Don't add too many ";"
 * 	[1993/05/07  20:06:53  marty]
 * 
 * Revision 1.1.2.46  1993/05/07  19:39:34  marty
 * 	 test
 * 	[1993/05/07  19:36:11  marty]
 * 
 * Revision 1.1.2.45  1993/05/07  19:33:29  marty
 * 	Change src_ctl_create_branch2 to tack on ";" at the end of the ancestry list.
 * 	[1993/05/07  19:32:00  marty]
 * 
 * Revision 1.1.2.44  1993/05/07  13:24:03  damon
 * 	CR 477. Removed unneeded strcpy
 * 	[1993/05/07  13:23:56  damon]
 * 
 * Revision 1.1.2.43  1993/05/06  16:12:21  damon
 * 	CR 477. Changed ancestry buffer size back to realistic size
 * 	[1993/05/06  16:11:33  damon]
 * 
 * Revision 1.1.2.42  1993/05/06  16:01:24  damon
 * 	CR 477. Ancestry buffers are now dynamic
 * 	[1993/05/06  16:01:16  damon]
 * 
 * Revision 1.1.2.41  1993/05/05  15:22:18  damon
 * 	CR 494. Removed debugging code
 * 	[1993/05/05  15:22:08  damon]
 * 
 * Revision 1.1.2.40  1993/05/05  15:17:38  damon
 * 	CR 494. Fixed return status of rcsfullstat
 * 	[1993/05/05  15:17:13  damon]
 * 
 * Revision 1.1.2.39  1993/05/04  21:02:46  damon
 * 	CR 483. getancestor now returns OE_ANCESTOR when no ancestor
 * 	[1993/05/04  21:02:17  damon]
 * 
 * Revision 1.1.2.38  1993/05/04  19:46:11  damon
 * 	CR 486. Added size parameter to oxm_gets
 * 	[1993/05/04  19:45:53  damon]
 * 
 * Revision 1.1.2.37  1993/05/04  17:47:25  marty
 * 	Changed call to rcsstat_f to rcsstat.
 * 	[1993/05/04  17:46:42  marty]
 * 
 * Revision 1.1.2.36  1993/05/04  16:58:04  damon
 * 	CR 463. Added include of util.h
 * 	[1993/05/04  16:57:57  damon]
 * 
 * Revision 1.1.2.35  1993/05/03  22:37:17  marty
 * 	src_ctl_create_branch2() needs to pass along the comment leader to "rcs".
 * 	[1993/05/03  22:36:41  marty]
 * 
 * Revision 1.1.2.34  1993/04/29  15:45:44  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:44:39  damon]
 * 
 * Revision 1.1.2.33  1993/04/29  14:25:25  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  14:25:06  damon]
 * 
 * Revision 1.1.2.32  1993/04/27  15:39:46  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:39:22  damon]
 * 
 * Revision 1.1.2.31  1993/04/26  19:23:49  damon
 * 	CR 428. CR 411. CR 135. ODE 2.2.1 patches
 * 	[1993/04/26  19:21:52  damon]
 * 
 * Revision 1.1.2.30  1993/04/26  15:15:19  marty
 * 	src_ctl_create_branch was freeing space that was not allocated.
 * 	[1993/04/26  15:14:58  marty]
 * 
 * Revision 1.1.2.29  1993/04/22  15:51:14  marty
 * 	Fix argument to branch_ci in src_ctl_create_branch()
 * 	[1993/04/22  15:50:56  marty]
 * 
 * Revision 1.1.2.28  1993/04/22  15:21:05  marty
 * 	Cleaned up more memory leaks.
 * 	[1993/04/22  15:20:39  marty]
 * 
 * Revision 1.1.2.27  1993/04/21  21:49:04  marty
 * 	Cleaning up memory leaks.
 * 	[1993/04/21  21:46:58  marty]
 * 
 * Revision 1.1.2.26  1993/04/16  21:08:47  damon
 * 	CR 463. Fixed unterminated strings
 * 	[1993/04/16  21:08:37  damon]
 * 
 * Revision 1.1.2.25  1993/04/16  15:37:47  damon
 * 	CR 436. Synching create_branch code
 * 	[1993/04/16  15:37:40  damon]
 * 
 * Revision 1.1.2.24  1993/04/14  14:43:19  damon
 * 	CR 457. Keep flock on an open file desc.
 * 	[1993/04/14  14:43:09  damon]
 * 
 * Revision 1.1.2.23  1993/04/13  12:17:46  damon
 * 	CR 432
 * 	Added code in src_ctl_outdate to change ancestors of revs when
 * 	the original ancestor is being outdated.
 * 	[1993/04/13  12:17:02  damon]
 * 
 * Revision 1.1.2.22  1993/04/09  19:15:20  marty
 * 	Removed include file odexm.h
 * 	[1993/04/09  19:15:03  marty]
 * 
 * Revision 1.1.2.21  1993/04/09  18:20:55  damon
 * 	CR 432. Changed locking for ancestry to flocks
 * 	[1993/04/09  18:20:48  damon]
 * 
 * Revision 1.1.2.20  1993/04/09  17:23:00  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:22:04  damon]
 * 
 * Revision 1.1.2.19  1993/04/09  14:11:31  damon
 * 	CR 432. Moved added ancestry tracking for outdates
 * 	[1993/04/09  14:10:48  damon]
 * 
 * Revision 1.1.2.18  1993/04/06  20:47:53  damon
 * 	CR 436. Folded old branch_ci code into this module
 * 	[1993/04/06  20:47:32  damon]
 * 
 * Revision 1.1.2.17  1993/03/30  20:44:32  damon
 * 	CR 436. Made ancestry update for bmerge better
 * 	[1993/03/30  20:44:04  damon]
 * 
 * Revision 1.1.2.16  1993/03/26  18:04:13  marty
 * 	Fix arguments past to fast revision lookup routines.
 * 	[1993/03/26  18:03:58  marty]
 * 
 * Revision 1.1.2.15  1993/03/25  21:24:37  marty
 * 	Added rcsfaststat() and src_ctl_fast_lookup_rev() to lookup revisions
 * 	for a large group of files.
 * 	[1993/03/25  21:24:23  marty]
 * 
 * Revision 1.1.2.14  1993/03/22  22:08:26  marty
 * 	Change src_ctl_create_branch() to set the comment leader on
 * 	a file if it is passed.
 * 	[1993/03/22  22:07:40  marty]
 * 
 * Revision 1.1.2.13  1993/03/17  20:42:19  damon
 * 	CR 446. Fixed include files/ forward decls.
 * 	[1993/03/17  20:41:30  damon]
 * 
 * Revision 1.1.2.12  1993/03/17  19:41:09  damon
 * 	CR 432. Added src_ctl_set_ancestry for merges
 * 	[1993/03/17  19:39:29  damon]
 * 
 * Revision 1.1.2.11  1993/03/15  22:05:16  damon
 * 	CR 436. Fixed tempslot count in _create_branch
 * 	[1993/03/15  22:05:01  damon]
 * 
 * Revision 1.1.2.10  1993/03/15  21:58:14  damon
 * 	CR 436. Moved -j option back to -r
 * 	[1993/03/15  21:57:34  damon]
 * 
 * Revision 1.1.2.9  1993/03/15  18:32:30  damon
 * 	CR 436. Adjusted some routines to work locally
 * 	[1993/03/15  18:32:24  damon]
 * 
 * 	Revision 1.1.2.7  1993/03/04  21:29:55  damon
 * 	CR 436. Fixed to run show_log and rcsfullstat locally
 * 	[1993/03/04  20:04:20  damon]
 * 
 * Revision 1.1.2.6  1993/02/10  18:34:58  damon
 * 	CR 432. Tracks simple ancestry
 * 	[1993/02/10  18:34:19  damon]
 * 
 * Revision 1.1.2.5  1993/01/26  16:34:39  damon
 * 	CR 396. Conversion to err_log
 * 	[1993/01/26  16:34:00  damon]
 * 
 * Revision 1.1.2.4  1993/01/25  21:28:27  damon
 * 	CR 396. Converted history.c to err_log
 * 	[1993/01/25  21:27:00  damon]
 * 
 * Revision 1.1.2.3  1993/01/20  22:21:35  damon
 * 	CR 376. Moved more code out from sci.c
 * 	[1993/01/20  22:16:20  damon]
 * 
 * Revision 1.1.2.2  1993/01/15  16:16:23  damon
 * 	CR 376. Moved code from sci_rcs.c
 * 	[1993/01/15  16:16:06  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)25  1.1  src/bldenv/sbtools/libode/src_ctl_rcs.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:40";
#endif /* not lint */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ode/errno.h>
#include <ode/interface.h>
#include <ode/lockcmds.h>
#include <ode/misc.h>
#include <ode/odedefs.h>
#include <ode/public/error.h>
#include <ode/public/odexm_client.h>
#include <ode/src_ctl_rcs.h>
#include <ode/util.h>
#include <sys/param.h>

#define IBR_MARKER      "*** Initial Branch Revision ***"

/*
 * FIXME: rework the following.
 */
#define ATOI(n, p) \
    (n) = 0; \
    if ('0' > *(p) || *(p) > '9') \
        return(1); \
    while ('0' <= *(p) && *(p) <= '9') { \
        (n) = (n) * 10 + (*(p) - '0'); \
        (p)++; \
    }

extern OXM_CNXTN rcs_monitor;
extern char working_file[];
extern char working_file_tail[];
extern char *BCSTEMP;
extern char temp_working_file[];
extern sci_local;

/*
 * PROTOTYPES
 */
int
tip_revision ( const char * );
BOOLEAN getancestor1( const char * ver_merge, const char * ver_user, char * r3,
                     const char * ancestry, int depth1, int depth2,
                     int * min_depth );

/*
 * ENDME
 */

ERR_LOG
src_ctl_undo_create( const char * name, const char * rev, BOOLEAN unlock )
{
  int i;
  const char *av[16];
  int status;
  char rcs_file_name [MAXPATHLEN];
  ERR_LOG log;

    if (strcmp(rev, "1.1.1.1") != 0) {
        ui_print ( VFATAL, "[ should not undo revision %s -- try bco -undo ]\n",
             rev);
        return( err_log ( OE_INTERNAL ) );
    }

    /* if file is writeable, then unlock it */

    /* outdate revision 1.1.1.1 for bcreate -undo */
    concat ( rcs_file_name, sizeof (rcs_file_name), name, ",v",
             NULL );
    i = 0;
    av[i++] = "rcs";
    if (unlock)
        av[i++] = "-u1.1.1.1";
    av[i++] = "-o1.1.1.1";
    av[i++] = rcs_file_name;
    if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
      return ( log );
    if ( ( log = oxm_endcmd ( rcs_monitor, &status ) ) != OE_OK )
      return ( log );
  return ( OE_OK );
}

ERR_LOG
src_ctl_add_symbol( const char * name, const char * sym_str, BOOLEAN override )
{
  ERR_LOG log;
  const char *av [16];
  int i;
  int status;
  char rcs_file_name [MAXPATHLEN];
  char sym_arg [MAXPATHLEN];

  concat ( rcs_file_name, sizeof ( rcs_file_name ), name,
           ",v", NULL );
  if ( override )
    concat ( sym_arg, sizeof ( sym_arg ), "-N", sym_str, NULL );
  else
    concat ( sym_arg, sizeof ( sym_arg ), "-n", sym_str, NULL );
  /* if */
  i = 0;
  av [i++] = "rcs";
  av [i++] = sym_arg;
  av [i++] = rcs_file_name;
  av [i]   = NULL;
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  if ( log != OE_OK )
    return ( log );
  log = oxm_endcmd ( rcs_monitor, &status );
  if ( log != OE_OK )
    return ( log );
  return ( OE_OK );
} /* end src_ctl_add_symbol */

ERR_LOG
src_ctl_remove_symbol( const char * name, const char * set_name)
{
  char *symbol;
  int status;
  const char *av[16];
  int i;
  char rcs_file_name [MAXPATHLEN];
  ERR_LOG log;

    ui_print ( VNORMAL, "[ deleting branch name '%s' ]\n", set_name );

    if ((symbol = alloc_switch('N', set_name )) == NULL) {
      return ( err_log ( OE_ALLOC ) );
    } /* if */
    concat ( rcs_file_name, sizeof (rcs_file_name), name, ",v",
             NULL );
    i = 0;
    av[i++] = "rcs";
    av[i++] = "-q";
    av[i++] = symbol;
    av[i++] = rcs_file_name;
    log = oxm_runcmd ( rcs_monitor, i, av, NULL );
    if ( log != OE_OK )
      return ( log );
    log = oxm_endcmd ( rcs_monitor, &status );
    if ( log != OE_OK )
      return ( log );
    return ( OE_OK );
}

ERR_LOG
src_ctl_remove_file( const char * name )
{
  int status;
  const char *av[16];
  int i;
  char rcs_file_name [MAXPATHLEN];
  ERR_LOG log;

    concat ( rcs_file_name, sizeof (rcs_file_name), name, ",v",
             NULL );
    i = 0;
    av[i++] = "rm_rcs";
    av[i++] = rcs_file_name;
    log = oxm_runcmd ( rcs_monitor, i, av, NULL );
    if ( log != OE_OK )
      return ( log );
    log = oxm_endcmd ( rcs_monitor, &status );
    if ( log != OE_OK )
      return ( log );
    return ( OE_OK );
}

STATIC
int rcsfullstat( const char *file_name, const char *rev_str, char *revision,
                 ERR_LOG * log)
{
    char buffer[MAXPATHLEN];
    char *ptr;
    int status;
    int i;
    const char *av[16];
    char *rev_sw;

#ifndef NO_DEBUG
  enter ( "rcsfullstat" );
#endif
  ui_print ( VDETAIL, "The revision corresponding to\n" );
  ui_print ( VCONT, "rev: %s\n", rev_str );
  rev_sw = alloc_switch ( 'r', rev_str );
  if ( rev_sw == NULL ) {
    leave ( );
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* end if */
  i = 0;
  av[i++] = "rcsstat";
  av[i++] = "-q";
  av[i++] = "-D";
  av[i++] = "-V";
  av[i++] = rev_sw;
  av[i++] = file_name;
  av[i] = NULL;
  if ( ( *log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK ) {
    free(rev_sw);
    return ( ERROR );
  } /* if */
  *revision = '\0';
  if ( oxm_gets ( rcs_monitor, buffer, sizeof(buffer), log ) != NULL ) {
    if ((ptr = strchr(buffer, '\n')) != NULL )
      *ptr = '\0';
    (void) strcpy ( revision, buffer );
  } /* if */

  if ( ( *log = oxm_endcmd ( rcs_monitor, &status ) ) != OE_OK ) {
    free(rev_sw);
    return ( status );
  } /* if */
  free(rev_sw);
  if (status == 0 && *revision == '\0')
    status = ERROR;
  else
    ui_print ( VDETAIL, "is revision: %s.\n", revision );
  /* if */
  leave ( );
  return ( status );
}

int rcs_cmp_func( char * str1, char * str2, int len, int * skipped)
{
    int cmp;

    cmp = strncmp(str1, str2, len);
    if (cmp == 0) {
        if (strcmp(str1 + len, str2 + len) == 0)
            *skipped = TRUE;
    }
    return(cmp);
} /* rcs_cmp_func */

int src_ctl_check_out_with_fd( const char * file_name, const char *rev, int fd,
                               const char *leader, ERR_LOG * log)
{
  int status;
  int i;
  const char *av[16];
  char rcs_file_name [MAXPATHLEN];

  enter ( "src_ctl_check_out_with_fd" );
  ui_print ( VDETAIL, "Checking out revision: %s\n", rev );
    if ((rev = alloc_switch('p', rev)) == NULL) {
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    }
    i = 0;
    concat ( rcs_file_name, sizeof (rcs_file_name), file_name, ",v", NULL );
    av[i++] = "co";
    av[i++] = "-q";
/*
    if (locked)
        av[i++] = "-l";
*/
    av[i++] = rev;
    if (strcmp(leader, "BIN") == 0)   /* test if file is binary */
      av[i++] = "-ko";
    av[i++] = rcs_file_name;
    av[i] = NULL;
    if ( ( *log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK ) {
      leave ();
      return ( ERROR );
    } /* if */
    if ( ( *log = oxm_get_all ( rcs_monitor, fd ) ) != OE_OK ) {
      leave ();
      return ( ERROR );
    } /* if */
    *log = oxm_endcmd( rcs_monitor, &status );
  if (status != 0)
    ui_print ( VFATAL, "co command failed\n");
  else
    status = OK;
  /* end if */
  leave ();
  return(status);
} /* end src_ctl_check_out_with_fd */

int src_ctl_check_out ( const char * file_name, const char * rev,
                        const char * leader, ERR_LOG * log)
{
  int i;
  const char *av[16];
  char ch2[2];
  char rcs_file_name [MAXPATHLEN];
  int status;
  int tempslot = 0;

  ui_print ( VDETAIL, "Checking out revision: %s\n", rev );
  if ((rev = alloc_switch('r', rev)) == NULL) {
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  }

  concat ( rcs_file_name, sizeof ( rcs_file_name ), file_name, ",v", NULL );
  i = 0;
  if ( !sci_local ) {
    av[i++] = "-t2";
    tempslot = i++;
  } /* if */
  av[i++] = "co";
  av[i++] = "-q";
  av[i++] = rev;
  if (strcmp(leader, "BIN") == 0)   /* test if file is binary */
    av[i++] = "-ko";
  if ( sci_local )
    av[i++] = temp_working_file;
  else {
    ch2[0] = '0' + i - 2; ch2[1] = '\0';
    av[tempslot] = strdup( ch2 );
    av[i++] = working_file_tail;
  } /* if */
  av[i++] = rcs_file_name;
  av[i] = NULL;
  *log = oxm_runcmd ( rcs_monitor, i, av, BCSTEMP );
  *log = oxm_endcmd ( rcs_monitor, &status );
  if ( !sci_local ) {
    free ( (char *)av[tempslot] );
  } /* if */
  return( status );
} /* end src_ctl_check_out */

int numfields( const char *rev, ERR_LOG * log )
{
    register char *r;
    int n[2];
    int c = 0;

    r = (char *)rev;
    for (;;) {
        ATOI(n[0], r)
        if (*r++ != '.') {
            *log = err_log ( OE_BADREV, rev );
            return ( ERROR );
        } /* if */
        c++;
        ATOI(n[1], r)
        if (*r != '.' && *r != '\0' && *r != ' ') {
            *log = err_log ( OE_BADREV );
            return ( ERROR );
        } /* if */
        c++;
        if (*r++ == '.')
            continue;
        return(c);
    }
}

int rcsfaststat( char * file_name, char * rev_str, BOOLEAN print_filename,
                 ERR_LOG * log )
{
    char buffer[MAXPATHLEN];
    int status;
    int i;
    const char *av[16];
    char *rev_sw;
    int oxmstatus;

#ifndef NO_DEBUG
  enter ( "rcsfullstat" );
#endif
  rev_sw = alloc_switch ( 'r', rev_str );
  if ( rev_sw == NULL ) {
    leave ( );
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* end if */
  i = 0;
  av[i++] = "rcsstat";
  av[i++] = "-q";
  av[i++] = "-D";
  av[i++] = "-V";
  if (print_filename)
  	av[i++] = "-R";
  av[i++] = rev_sw;
  av[i++] = strdup(alloc_switch ( 'f', file_name));
  av[i] = NULL;
  if ( ( *log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
    return ( ERROR );
  /* if */
  /* *revision = '\0'; */
  while ( oxm_gets ( rcs_monitor, buffer, sizeof(buffer), log ) != NULL ) {
	ui_print (VALWAYS, "%s", buffer);
  } 

  if ( ( *log = oxm_endcmd ( rcs_monitor, &oxmstatus ) ) != OE_OK ) {
    return ( ERROR );
  } /* if */
  status = oxmstatus;
  leave ( );
  return ( status );
}

/*
 * Provide the revision of a group of  files 
 */
int src_ctl_fast_lookup_revision (
char * name,
char * rev_str,
BOOLEAN print_filename,
ERR_LOG * log)
{
  char tmp_str [PATH_LEN];
  int status;

  enter ( "src_ctl_fast_lookup_revision" );
  strcpy ( tmp_str, rev_str );

  status = rcsfaststat ( name, tmp_str, print_filename, log );
  leave ();
  return ( status );
}

int src_ctl_diff_rev_with_file( const char * rev, const char * filename,
                                const char * rcs_file,
                                int fd, BOOLEAN context, BOOLEAN whitespace,
                                ERR_LOG * log )
{
    int i;
    const char *av[16];
    char comma_v_file [ MAXPATHLEN ];
    char buf [ MAXPATHLEN ];
  int status;
  char ch2[2];
  int tempslot;

    if ((rev = alloc_switch('r', rev)) == NULL) {
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    }

    concat ( comma_v_file, sizeof ( comma_v_file ), rcs_file, ",v", NULL );
    i = 0;
    av[i++] = "-t1";
    tempslot = i++;
    av[i++] = "rcsdiff";
    if ( context )
      av[i++] = "-c";
    if ( whitespace )
      av[i++] = "-w";
    ch2[0] = '0' + i - 1; ch2[1] = '\0';
    av[tempslot] = strdup (ch2);
    av[i++] = rev;
    av[i++] = filename;
    av[i++] = comma_v_file;
    /* previously piped through more */
    *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
    while ( oxm_gets ( rcs_monitor, buf, sizeof(buf), log ) != NULL ) {
      if ( fd >= 0 ) {
        write ( fd, buf, strlen(buf) );
      } else {
        ui_print ( VALWAYS, "%s", buf );
      } /* if */
    } /* while */
    *log = oxm_endcmd ( rcs_monitor, &status );
    free( (char *)av[tempslot] );
    free( (char *) rev );
    return( OK );
}

int src_ctl_show_diffs( const char *rev, const char *altrev,
                        const char * rcs_file,
                        BOOLEAN context, BOOLEAN whitespace, ERR_LOG * log )
{
  int i;
  const char *av[16];
  int status;
  char comma_v_file [ MAXPATHLEN ];
  char buffer[MAXPATHLEN];
  char * r_rev;
  char * r_arev;
  if ((r_rev = alloc_switch('r', rev)) == NULL) {
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* if */
  if ((r_arev = alloc_switch('r', altrev)) == NULL) {
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* if */

  concat ( comma_v_file, sizeof ( comma_v_file ), rcs_file, ",v", NULL );
  i = 0;
  av[i++] = "rcsdiff";
  if ( context )
    av[i++] = "-c";
  if ( whitespace )
    av[i++] = "-w";
  av[i++] = r_rev;
  av[i++] = r_arev;
  av[i++] = comma_v_file;
  *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  while ( oxm_gets ( rcs_monitor, buffer, sizeof(buffer), log ) != NULL ) {
    ui_print ( VALWAYS, "%s", buffer );
  } /* while */
  *log = oxm_endcmd ( rcs_monitor, &status );
  return( status );
}

/*
 * Provide the revision of a particular file in a particular set
 */
int src_ctl_lookup_revision ( const char * name, const char * rev_str,
                              char * revision, ERR_LOG * log)
{
  char tmp_str [PATH_LEN];
  int status;

  enter ( "src_ctl_lookup_revision" );
  strcpy ( tmp_str, rev_str );
  status = rcsfullstat ( name, tmp_str, revision, log );
  leave ();
  return ( status );
}

/*
 * returns OK iff r2 is a proper ancestor of r1
 */
int checkancestor( char *r1, char *r2 )
{
    register char *s1, *s2;
    int n1[2], n2[2];

    s1 = r1;
    s2 = r2;
    for (;;) {
        ATOI(n1[0], s1)
        ATOI(n2[0], s2)
        /*
         * In the first pass through:
         * If the first rev number of s1 or s2 is not followed by a '.', then
         * return a 1.
         *
         * In the second pass through:
         * If the third rev number of s1 or s2 is not followed by a '.', then
         * return a 1.
         */
        if (*s1++ != '.')
            return(1);
        if (*s2++ != '.')
            return(1);
        ATOI(n1[1], s1)
        ATOI(n2[1], s2)

        /*
         * In the first pass through:
         * If the second rev number of s1 or s2 is not followed by a '.', and
         * it isn't because there isn't a third rev number, return a 1.
         *
         * In the second pass through:
         * If the fourth rev number of s1 or s2 is not followed by a '.', then
         * return a 1.
         */
        if (*s1 != '.' && *s1 != '\0')
            return(1);
        if (*s2 != '.' && *s2 != '\0')
            return(1);

        /*
         * In the first pass through:
         * If the first and second rev #'s aren't equal, break out.
         *
         * In the second pass through:
         * If the third and fourth rev #'s aren't equal, break out.
         * Also, since ODE rev strings should only have four rev #'s,
         * this routine should always break out at this point.
         * FIXME: this routine is too general.
         */
        if (n1[0] != n2[0] || n1[1] != n2[1] || *s1 == '\0' || *s2 == '\0')
            break;
        s1++;
        s2++;
    }
    /*
     * If the first rev #'s of s1 and s2 are not the same, return 1 if
     * the first rev # of s1 < first rev # of s2 or s1 and s2 have the
     * same number of rev #'s.
     */
    if (n1[0] != n2[0])
        return(n1[0] < n2[0] || (*s1 == '\0' && *s2 != '\0'));
    /*
     * If the second rev #'s of s1 and s2 are not the same, return 1 if
     * the second rev # of s1 < second rev # of s2 or s2 has more rev #'s
     * than s1
     */
    if (n1[1] != n2[1])
        return(n1[1] < n2[1] || (*s1 == '\0' && *s2 != '\0'));
    /*
     * The first two rev #'s of s1 and s2 are the same. Return a 1 if
     * s2 has more rev #'s than s1.
     */
    return(*s1 == '\0' && *s2 != '\0');
}

STATIC void
set_ancestor ( char * r3, const char * ancestor, int depth1, int depth2,
               int * min_depth )
{
  int new_min;

  if ( depth1 < depth2 ) {
    new_min = depth1;
  } else {
    new_min = depth2;
  } /* if */
  if ( new_min < *min_depth || *min_depth == 0 ) {
    *min_depth = new_min;
    strcpy ( r3, ancestor );
  } /* if */
} /* set_ancestor */

STATIC BOOLEAN
getancestor2 ( const char * ancestor, const char * ver_search,
               char * r3, const char * ancestry, int depth1, int depth2,
               int * min_depth )
{
  char * ancestor_branch;
  char * search_branch;
  BOOLEAN found;
  ERR_LOG log;

  enter ( "getancestor2" );
    if ( numfields ( ancestor, &log ) == 2 ) {
      if ( strcmp ( ancestor, ver_search ) == 0 ) {
        set_ancestor ( r3, ancestor, depth1, depth2, min_depth );
        leave();
        return ( TRUE );
      } else {
        leave();
        return ( FALSE );
      } /* if */
    } /* if */ 
    ancestor_branch = get_branch ( ancestor );
    search_branch = get_branch ( ver_search );
    if ( strcmp ( search_branch, ancestor_branch ) == 0 ) {
      free ( ancestor_branch );
      free ( search_branch );
      set_ancestor ( r3, ancestor, depth1, depth2, min_depth );
      leave();
      return ( TRUE );
    } else {
      free ( ancestor_branch );
      free ( search_branch );
      found = getancestor1 ( ancestor, ver_search, r3, ancestry, depth1,
                            depth2, min_depth );
      leave();
      return ( found );
    } /* if */
} /* end getancestor2 */

BOOLEAN getancestor1( const char * ver_merge, const char * ver_user, char * r3,
                     const char * ancestry, int depth1, int depth2,
                     int * min_depth )
{
  char * user_branch=NULL;
  char * a_ptr;
  char * successor;
  char * ancestor;
  char * tmp_ancestry;
  BOOLEAN found=FALSE;
  BOOLEAN found1=FALSE;
  const char * ver_search;
  char * merge_branch = NULL;
  char * successor_branch = NULL;
  BOOLEAN user_is_trunk = FALSE;
  BOOLEAN merge_is_trunk = FALSE;
  ERR_LOG log;
  int tip1;
  int tip2;
 
  enter ( "getancestor1" );
  if ( numfields ( ver_user, &log ) == 2 ) {
    user_is_trunk = TRUE;
  } else {
    user_branch = get_branch ( ver_user );
  } /* if */
  if ( numfields ( ver_merge, &log ) == 2 ) {
    merge_is_trunk = TRUE;
  } else {
    merge_branch = get_branch ( ver_merge );
  } /* if */
  if ( user_is_trunk || merge_is_trunk ) {
    if ( strcmp ( ver_merge, ver_user ) == 0 ) {
      set_ancestor ( r3, ver_merge, depth1, depth2, min_depth );
      leave();
      return ( TRUE );
    } /* if */
    if ( user_is_trunk && merge_is_trunk ) {
      return ( FALSE );
    } /* if */
  } else if ( strcmp ( user_branch, merge_branch ) == 0 ) {
    free ( merge_branch );
    tip1 = tip_revision ( ver_merge );
    tip2 = tip_revision ( ver_user );
    if ( tip1 < tip2 ) {
      set_ancestor ( r3, ver_merge, depth1, depth2, min_depth );
      leave();
      return ( TRUE );
    } else {
      set_ancestor ( r3, ver_user, depth1, depth2, min_depth );
      leave();
      return ( TRUE );
    } /* if */
  } else {
    free ( merge_branch );
  } /* if */
  if ( ! user_is_trunk ) {

    tmp_ancestry = strdup ( (char *)ancestry );
    a_ptr = tmp_ancestry;
    found = FALSE;
    for (;;) {
      successor = nxtarg (&a_ptr, ">" );
      if ( *successor == '\0' || successor == NULL ) {
        break;
      } /* if */
      ancestor = strdup (nxtarg (&a_ptr, ";" ));
      if ( *ancestor == '\0' || ancestor == NULL ) {
        break;
      } /* if */
      successor_branch = get_branch ( successor );
      if ( strcmp ( successor_branch, user_branch ) == 0 ) {
        found = TRUE;
        ver_search = ver_merge;
        if ( getancestor2 ( ancestor, ver_search, r3, ancestry, depth2+1,
                            depth1, min_depth ) ) { 
        } /* if */
      } /* if */
    } /* for */
    free ( user_branch );
    free ( tmp_ancestry );

  }

  if ( ! merge_is_trunk ) {

    tmp_ancestry = strdup ( (char *)ancestry );
    merge_branch = get_branch ( ver_merge );
    a_ptr = tmp_ancestry;
    found1 = found;
    found = FALSE;
    for (;;) {
      successor = nxtarg (&a_ptr, ">" );
      if ( *successor == '\0' || successor == NULL ) {
        break;
      } /* if */
      ancestor = strdup (nxtarg (&a_ptr, ";" ));
      if ( *ancestor == '\0' || ancestor == NULL ) {
        break;
      } /* if */
      successor_branch = get_branch ( successor );
      if ( strcmp ( successor_branch, merge_branch ) == 0 ) {
        found = TRUE;
        ver_search = ver_user;
        found = getancestor2 ( ancestor, ver_search, r3, ancestry, depth1+1,
                               depth2, min_depth );
      } /* if */
    } /* for */
    free ( user_branch );
    free ( tmp_ancestry );

  }

  leave();
  return ( found || found1 );
}

BOOLEAN getancestor( const char * ver_merge, const char * ver_user, char * r3,
                     char * ancestry )
{
  int min_depth = 0;

  return ( getancestor1( ver_merge, ver_user, r3, ancestry, 1, 1,
           &min_depth ) );
} /* end getancestor */

ERR_LOG
src_ctl_ancestor( const char * name, const char * ver_config, const char *rev1,
                  const char *rev2, char **rev3p, int *called_getancestor,
                  char * ancestry)
{
    char rev3[32];

  ui_print ( VDEBUG, "Entering src_ctl_ancestor\n" );
/*
  strcpy ( rev3, ver_config );
  status = checkancestor(rev2, rev3);
  if (status != 0) {
*/
      if (called_getancestor)
          *called_getancestor = TRUE;
      if (getancestor(rev1, rev2, rev3, ancestry ) != TRUE) {
          return( err_log ( OE_ANCESTOR, rev1, rev2, name ) );
      }
/*
  }
*/
  *rev3p = strdup(rev3);
  if (*rev3p == NULL) {
    return ( err_log ( OE_ALLOC ) );
  }
  ui_print ( VDEBUG, "Leaving src_ctl_ancestor\n" );
  return( OE_OK );
} /* src_ctl_ancestor */

int src_ctl_check_in ( const char * name, const char * rev,const char * logmsg,
                       const char * state, ERR_LOG * log )
{
  int i;
  char *p, *lastdot;
  const char *av[16];
  int tempslot=0;
  char ch2[2];
  char dbuf[MAXPATHLEN], fbuf[MAXPATHLEN];
  int status;
  char * rev_sw;
  char * logmsg_sw;
  char * state_sw;

  enter ( "src_ctl_check_in" );

  if ( sci_local ) {
/*
 * FIXME: what if defuncting?
 */
    if ((rev_sw = alloc_switch('u', rev)) == NULL) { /* ADDED */
      leave ( );
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    }
  } else 
    if ((rev_sw = alloc_switch('r', rev)) == NULL) {
      leave ( );
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    } /* if */
  /* if */


  if ((logmsg_sw = alloc_switch('m', logmsg)) == NULL) {
    leave ( );
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* end if */
  if ((state_sw = alloc_switch('s', state)) == NULL) {
    leave ( );
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* end if */

  path ( name, dbuf, fbuf );
  concat ( dbuf, sizeof ( dbuf ), name , ",v", NULL );
  i = 0;
  if ( !sci_local ) {
    av[i++] = "-t2";
    tempslot = i++;
  } /* if */
  av[i++] = "ci";
  av[i++] = "-f";
  av[i++] = "-q";
  av[i++] = state_sw;
  av[i++] = rev_sw;
  lastdot = NULL;
  for (p = rev_sw; *p != '\0'; p++)
    if (*p == '.')
      lastdot = p;
  if (lastdot != NULL)
    *lastdot = '\0';
  av[i++] = logmsg_sw;

  if ( sci_local )
    av[i++] = temp_working_file;
  else {
    ch2[0] = '0' + i - 2; ch2[1] = '\0';
    av[tempslot] = strdup ( ch2 );
    av[i++] = fbuf;
  } /* if */

  av[i++] = dbuf;
  av[i] = NULL;

  *log = oxm_runcmd ( rcs_monitor, i, av, BCSTEMP );
  if ( *log != OE_OK ) {
    free(rev_sw);
    free(state_sw);
    free(logmsg_sw);
    return ( ERROR );
  } /* if */
  *log = oxm_endcmd( rcs_monitor, &status );
  if ( *log != OE_OK ) {
    free(rev_sw);
    free(state_sw);
    free(logmsg_sw);
    return ( ERROR );
  } /* if */
  if ( !sci_local ) {
    free ( (char *)av[tempslot] );
  } /* if */
  free(rev_sw);
  free(state_sw);
  free(logmsg_sw);
  leave ( );
  return(status);
} /* end src_ctl_check_in */

int src_ctl_lock_revision(char * rev, char * file_name, ERR_LOG * log )
{
  int status;
  int i;
  const char *av[16];

  enter ( "src_ctl_lock_revision" );
  if ((rev = alloc_switch('l', rev)) == NULL) {
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  }

  i = 0;
  av[i++] = "rcs";
  av[i++] = "-q";
  av[i++] = rev;
  av[i++] = file_name;
  if ( ( *log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK ) {
    leave ();
    return ( ERROR );
  } /* if */
  *log = oxm_endcmd(rcs_monitor, &status );
  if (status != 0) {
    ui_print ( VFATAL, "rcs -l command failed\n" );
    ui_print ( VFATAL, "%s\n", err_str ( log ) );
    leave ();
    return ( ERROR );
  }
  leave ();
  return(status);
} /* end src_ctl_lock_revision */

int src_ctl_unlock_revision(
char * rev,
char * file_name,
ERR_LOG * log)
{
  int status;
  int i;
  const char *av[16];

  enter ( "src_ctl_unlock_revision" );
  if ((rev = alloc_switch('u', rev)) == NULL) {
    leave ();
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  }

  i = 0;
  av[i++] = "rcs";
  av[i++] = "-q";
  av[i++] = rev;
  av[i++] = file_name;
  if ( ( *log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK ) {
    leave ();
    return ( ERROR );
  } /* if */

  *log = oxm_endcmd( rcs_monitor, &status );
  if (status != 0) {
    ui_print ( VFATAL, "rcs -u command failed\n" );
    leave ();
    return ( ERROR );
  }
  leave ();
  return(status);
} /* end src_ctl_unlock_revision */

int
src_ctl_create_branch( const char * rcs_file, const char * rev,
                       const char * set_name, const char * ancestor,
                       const char * cmt_leader, ERR_LOG * log )
{
  int i;
  const char *av[16];
  char *symbol;
  char *leader=NULL;
  char comma_v_file [MAXPATHLEN];
  char * ibr_str;
  int status;
  char * a_switch=NULL;
  int args;
  char ascii_args[5];
  char *rev_sw;
  char *ascii_args_sw=NULL;

  if ( sci_local ) {
    if ((rev_sw = alloc_switch('u', rev)) == NULL) {
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    } /* end if */
  } else
    if ((rev_sw = alloc_switch('r', rev)) == NULL) {
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    } /* end if */
  /* if */
  if ((symbol = alloc_switch('N', set_name)) == NULL) {
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* end if */
  if (cmt_leader != NULL) { /* Allocate a switch for the comment leader */
  	if ((leader = alloc_switch('c', cmt_leader)) == NULL) {
    		*log = err_log ( OE_ALLOC );
    		return ( ERROR );
  	} 
  }

  if ((ibr_str = alloc_switch('m', IBR_MARKER )) == NULL) {
    *log = err_log ( OE_ALLOC );
    return ( ERROR );
  } /* end if */

  if ( ancestor != NULL )
    if ((a_switch = alloc_switch('a', ancestor )) == NULL) {
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    } /* end if */
  /* end if */

  args = 3;  
  concat ( comma_v_file, sizeof ( comma_v_file ), rcs_file, ",v", NULL );
  i = 0;
  if ( !sci_local ) {
    av[i++] = "-t1";     /* -t1 indicates that the file shouldn't be removed */
    if ( ancestor != NULL )
      args++;
    if ( cmt_leader != NULL )
      args++;
    sprintf (ascii_args, "%d", args);
    ascii_args_sw = strdup(ascii_args);
    av[i++] = ascii_args_sw;
    /* if */
  } /* if */
  av[i++] = "branch_ci";
  av[i++] = rev_sw;
  av[i++] = symbol;
  if (cmt_leader != NULL)
    av[i++] = leader;
  if ( ancestor != NULL )
    av[i++] = a_switch;
  /* if */
  if ( sci_local )
    av[i++] = temp_working_file;
  else
    av[i++] = working_file_tail;
  /* if */

  av[i++] = comma_v_file;
  av[i] = NULL;
  *log = oxm_runcmd ( rcs_monitor, i, av, BCSTEMP );
  if ( *log != OE_OK )
    return ( ERROR );
  *log = oxm_endcmd( rcs_monitor, &status );
  free(rev_sw);
  free(symbol);
  if (cmt_leader != NULL)
	free(leader);
  free(ibr_str);
  if ( ancestor != NULL )
  	free(a_switch);
  free(ascii_args_sw);
  return ( status );
} /* end src_ctl_create_branch */

STATIC void
get_ancestry ( char ** ancestry, ERR_LOG * log )
{
  int len;
  int anc_len;
  int anc_size;
  char buf[MAXPATHLEN];

  anc_len = 0;
  anc_size = MAXPATHLEN;
  *ancestry = (char *)malloc(anc_size);
  **ancestry = '\0';
  oxm_gets ( rcs_monitor, buf, sizeof(buf), log );
  if ( buf [0] != '-' ) {
    len = strlen(buf);
    memcpy ( *ancestry, buf, len );
    anc_len = len;
    while ( len+1 == sizeof(buf) ) {
      oxm_gets ( rcs_monitor, buf, sizeof(buf), log );
      len = strlen(buf);
      if ( anc_len+len > anc_size ) {
        *ancestry = (char *)realloc ( *ancestry, anc_len+len );
        anc_size = anc_len+len;
      } /* if */
      memcpy ( *ancestry+anc_len, buf, len );
      anc_len = anc_len + len;
    } /* while */
  } /* if */
  rm_newline ( *ancestry );
} /* end get_ancestry */

int src_ctl_create_branch2 ( 
const char * filename,
const char * comma_v_file,
BOOLEAN lock,
const char * rev,
const char * set_name,
const char * ancestor,
const char * cmt_leader,
const char * msg,
ERR_LOG * log )
{
  int   i;
  const char * av[16];
  int    status;
  char   branch_buf[MAXPATHLEN];
  char   * ancestry;
  char * new_ancestry;
  char * token;
  char   latest_branch[MAXPATHLEN];
  int    dots;
  char   bnum[MAXPATHLEN];
  int    newbnum;
  char * res;
#define revlength 30
  char   branch_rev[revlength];
  char * bufp;
  int    j;
  char * brev;
  char * br_symbol;
  char * symbol = (char *)NULL;
  char * leader = (char *)NULL;
  char c;
  char * urev;
  char * rrev;
  char * mmsg;
  int fd;

  for (;;) {
    if ( set_lock2 ( comma_v_file, &fd ) == WAITING ) {
      ui_print ( VNORMAL, "File locked. Waiting.\n" );
      sleep ( 5 );
    } else {
      break;
    } /* if */
  } /* for */
  rrev = alloc_switch ( 'r', rev );
  urev = alloc_switch ( 'u', rev );
  /* if */
  i = 0;
  av[i++] = "rlog";
  av[i++] = rrev;
  av[i++] = comma_v_file;
  av[i] = NULL;
  *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  if ( *log != OE_OK )
    return ( ERROR );
  /* if */
  while (oxm_gets ( rcs_monitor, branch_buf, sizeof(branch_buf), log ) != NULL)
    if (strncmp(branch_buf, "description:", 12) == 0)
      break;
    /* if */
  /* while */
  get_ancestry ( &ancestry, log );
 /*
  * If ancestry was found, then need to remove everything up to and including
  * '------...' line.
  */
  if ( *ancestry != '\0' ) {
    while (( res = oxm_gets( rcs_monitor, branch_buf, sizeof(branch_buf),
                             log )) != NULL) {
      if ( *branch_buf == '-' ) {
        break;
      } /* if */
    } /* while */
  }
  while (( res = oxm_gets( rcs_monitor, branch_buf, sizeof(branch_buf),
                           log )) != NULL) {
    if (strncmp(branch_buf, "branches:", 9) == 0) {
      break;
    } /* if */
   /*
    * Don't continue searching if '-' is found, because the 
    * branch information will be taken from another trunk revision!!
    */
    if ( *branch_buf == '-' ) {
      res = NULL;
      break;
    } /* if */
  } /* while */
  if ( res == NULL ) {
    concat ( branch_rev, sizeof(branch_rev ), rev, ".1", NULL );
  } else {
    bufp = branch_buf;
    /* have token point to the highest branch for the revision */
    while ((token = strtok(bufp, " ")) != NULL)  {
      strcpy (latest_branch, token);
      bufp = NULL;  /* tell strtok to keep going */
    }
    /* copy the branch number, i.e. the third number, into bnum */
    for (i=0, j=0, dots=0; latest_branch[i] != ';'; i++)  {
      if (dots >=2)
        bnum[j++] = latest_branch[i];
      if (latest_branch[i] == '.')
        dots++;
    }
    /* convert bnum string into a number and add one */
    newbnum = atoi(bnum)+1;
    i = 0;
    do  {
      bnum[i++] = newbnum % 10 + '0';
    }  while ((newbnum /= 10) > 0);
    bnum[i] = '\0';
    for (i=0, j = strlen(bnum)-1; i<j; i++, j--)  {
      c = bnum[i];
      bnum[i] = bnum[j];
      bnum[j] = c;
    } /* for */
    sprintf (branch_rev, "%s.%s", rev, bnum);
  } /* if */
  *log = oxm_endcmd( rcs_monitor, &status );

  /* check in the file to the correct branch */
  symbol = alloc_switch('N', set_name);
  if ( lock )
    brev = alloc_switch('r', branch_rev );
  else
    brev = alloc_switch('u', branch_rev );
  /* if */
  i = 0;
  av[i++] = "ci";
  av[i++] = brev;
  av[i++] = symbol;
  av[i++] = "-f";
  av[i++] = "-q";
  if ( msg == NULL )
    av[i++] = "-m*** Initial Branch Revision ***";
  else {
    mmsg = alloc_switch ( 'm', msg );
    av[i++] = mmsg;
  } /* if */
  av[i++] = comma_v_file;
  av[i++] = filename;
  av[i] = NULL;
  *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  if ( *log != OE_OK )
    return ( ERROR );
  /* if */
  *log = oxm_endcmd( rcs_monitor, &status );

  /* set the leader if needed
     and set the symbolic name correctly */
  i = 0;
 /*
  * The +8 is for the characters '-t-.1>;' . That's only 7, but you have
  * to add 1 for the \0 !!
  */
  if ( ancestor != NULL ) {
    new_ancestry = (char *)malloc(strlen(ancestry)+strlen(branch_rev)+
                                  strlen(ancestor)+8 );
    sprintf ( new_ancestry, "-t-%s.1>%s;%s", branch_rev, ancestor, ancestry );
  } else {
    new_ancestry = strdup( "-t-1.1.1.1>1.1;" );
  } /* if */
  av[i++] = "rcs";
  av[i++] = "-q";
  av[i++] = new_ancestry;
  if (cmt_leader != NULL) {       /* set the leader if needed */
    leader = alloc_switch('c', cmt_leader);
    av[i++] = leader;
  }
  br_symbol = (char *) malloc(strlen(symbol) + strlen(branch_rev) + 2);
  sprintf (br_symbol, "%s:%s", symbol, branch_rev);
  av[i++] = br_symbol;
  av[i++] = comma_v_file;
  av[i] = NULL;
  *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  if ( *log != OE_OK )
    return ( ERROR );
  /* if */
  *log = oxm_endcmd( rcs_monitor, &status );

  free(ancestry);
  free(new_ancestry);
  /* unlock the file */
  unset_lock2 ( comma_v_file, fd );
  return ( OK );
} /* end src_ctl_create_branch2 */

int
src_ctl_show_log( char * file_name, char *rev, int lock_users, int header,
                  int rcs_path, int long_format, ERR_LOG * log )
{
  int i;
  const char *av[16];
  BOOLEAN found_lock_line = FALSE;
  char buffer[MAXPATHLEN];
  BOOLEAN printed_file = FALSE;
  char rcs_file [MAXPATHLEN];
  int status;

  enter ( "src_ctl_show_log" );
  if (rev)
    if ((rev = alloc_switch('r', rev)) == NULL) {
      leave ();
      *log = err_log ( OE_ALLOC );
      return ( ERROR );
    } /* if */
  /* if */

  i = 0;
  av[i++] = "rlog";
  if ( lock_users )
    av[i++] = "-l";
  /* if */
  if ( long_format )
    av[i++] = "-L";
  /* if */
  if ( rcs_path )
    av[i++] = "R";
  /* if */
  if ( header )
    av[i++] = "-h";
  /* if */
  if ( rev != NULL )
    av[i++] = rev;
  concat ( rcs_file, sizeof (rcs_file), file_name, ",v" , NULL );
  av[i++] = rcs_file;
  av[i] = NULL;
  if ( ( *log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
    return ( ERROR );
  /* if */
  if ( lock_users ) {
    while ( oxm_gets ( rcs_monitor, buffer, sizeof(buffer), log ) != NULL ) {
      if ( strncmp ( buffer, "locks", 5 ) == 0 ) {
        found_lock_line = TRUE;
        break;
      } /* if */
    } /* while */
    if ( found_lock_line ) {
      while ( oxm_gets ( rcs_monitor, buffer, sizeof(buffer), log ) != NULL ) {
        if ( *buffer != '\t' )
          break;
        else {
          rm_newline ( buffer );
          if ( ! printed_file ) {
            printf ( "%s  ", rcs_file );
            printed_file = TRUE;
          } else
            printf ( " " );
          /* if */
          printf ( "%s;", buffer+1 );                     /* +1 to skip tab */
        } /* if */
      } /* while */
      printf ( "\n" );
    } /* if */
  } else {
    while ( oxm_gets ( rcs_monitor, buffer, sizeof(buffer), log ) != NULL ) {
      printf ( "%s", buffer );
    } /* while */
  } /* if */
  *log = oxm_endcmd( rcs_monitor, &status );
  leave ();
  return( status );
} /* end src_ctl_show_log */

/*
 * This chops the last revision component from a four component revision
 * string, leaving the branch.
 */
char *
get_branch ( const char * str )
{
  char * str_ptr;
  char * result;
  int i;

  result = strdup ( (char *)str );
  str_ptr = result;
  for ( i = 0;; ) {
    if ( *str_ptr == '.' ) {
      i++;
      if ( i == 3 ) {
        *str_ptr = '\0';
        break;
      } /* if */
    } else if ( *str_ptr == '\0' ) {
      break;
    } /* if */
    str_ptr++;
  } /* for */
  if ( i < 3 )
    return ( NULL );
  else
    return ( result );
  /* if */
} /* get_branch */

int
tip_revision ( const char * rev )
{
  char * p;
  int i;
  int tip;

  i = 0;
  for ( p = (char *)rev ; *p != NUL ; p++ ) {
    if ( *p == '.' ) {
      i++;
      if ( i == 3 ) {
        p++;
        break;
      }
      /* if */
    } /* if */
  } /* for */
  if ( i == 3 ) {
    ATOI ( tip, p );
    return ( tip );
  } else
    return ( ERROR );
  /* if */
} /* end tip_revision */

int
src_ctl_file_exists ( char *pathname, ERR_LOG * log )
{
    int status;
    int i;
    const char *av[16];
    char rcs_file[MAXPATHLEN];

    concat ( rcs_file, sizeof ( rcs_file ), pathname, ",v", NULL );
    i = 0;
    av[i++] = "exists";
    av[i++] = rcs_file;
    *log = oxm_runcmd ( rcs_monitor, i, av, NULL );
    if ( *log != OE_OK )
      return ( ERROR );
    *log = oxm_endcmd ( rcs_monitor, &status );
    return(status);
}

int
src_ctl_get_ancestry ( char * rcs_file, char ** ancestry )
{
    char buf[MAXPATHLEN];
    int found;
    int status;
    int i;
    const char *av[16];
    ERR_LOG log;

    i = 0;

    av[i++] = "rlog";
    av[i++] = rcs_file;
    av[i] = NULL;
    if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
      return ( ERROR );
    /* if */

    found = FALSE;
    while ( oxm_gets( rcs_monitor, buf, sizeof(buf), &log ) != NULL) {
        if (strncmp(buf, "description:", 12) == 0) {
            found = TRUE;
            break;
        }
    }
    get_ancestry ( ancestry, &log );
    log = oxm_endcmd( rcs_monitor, &status );

    return( OK );
} /* end src_ctl_get_ancestry */

ERR_LOG
src_ctl_add_ancestry ( char * rcs_file, char * ancestry )
{
    char buf[MAXPATHLEN];
    int i;
    const char *av[16];
    ERR_LOG log;
    int status;

    concat ( buf, sizeof ( buf ), "-a", ancestry, NULL );
    i = 0;
    av[i++] = "bmerge_s";
    av[i++] = buf;
    av[i++] = rcs_file;
    av[i] = NULL;
    if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
      return ( log );
    /* if */

    log = oxm_endcmd( rcs_monitor, &status );

    return( log );
} /* end src_ctl_add_ancestry */

ERR_LOG
src_ctl_add_ancestry2 ( char * rcs_file, char * new_ancestry )
{
  char comma_v_file [MAXPATHLEN];
  char * ancestry;
  ERR_LOG log;
  char buf[MAXPATHLEN];
  int i;
  const char * av[16];
  int status;
  int fd;
  char * final_ancestry;

  concat ( comma_v_file, sizeof ( comma_v_file ), rcs_file, ",v", NULL );
  for (;;) {
    if ( (status = set_lock2 ( comma_v_file, &fd ) == WAITING) ) {
      ui_print ( VALWAYS, "File locked. Waiting.\n" );
      sleep ( 5 );
    } else {
      break;
    } /* if */
  } /* for */
  i = 0;
  av[i++] = "rlog";
  av[i++] = "-r1.1";
  av[i++] = rcs_file;
  av[i] = NULL;
  if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
    return ( log );
  /* if */
  while ( oxm_gets ( rcs_monitor, buf, sizeof(buf), &log ) != NULL)
    if (strncmp(buf, "description:", 12) == 0)
      break;
    /* if */
  /* while */
  get_ancestry ( &ancestry, &log );
  log = oxm_endcmd ( rcs_monitor, &status );

  i = 0;
  av[i++] = "rcs";
  final_ancestry = (char *)malloc(strlen(ancestry)+strlen(new_ancestry)+5 );

  if ( ancestry == NULL || *ancestry == '\0' ) {
    concat ( final_ancestry, strlen(ancestry)+strlen(new_ancestry)+5,
                             "-t-",new_ancestry, ";", NULL );
  } else {
    concat ( final_ancestry, strlen(ancestry)+strlen(new_ancestry)+5,
                             "-t-",new_ancestry, ";", ancestry, NULL );
  } /* if */
  av[i++] = final_ancestry;
  av[i++] = rcs_file;
  av[i] = NULL;
  if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
    return ( log );
  /* if */
  log = oxm_endcmd ( rcs_monitor, &status );
  free(ancestry);
  free(final_ancestry);
  unset_lock2 ( comma_v_file, fd );
  return ( log );
} /* end src_ctl_add_ancestry2 */

STATIC
char *
get_ancestor ( char * ancestry, char * rev )
{
  char * a_ptr;
  char * successor;
  char * ancestor;
  char * tmp_ancestry;

  tmp_ancestry = strdup ( ancestry );
  a_ptr = tmp_ancestry;
  for (;;) {
    successor = nxtarg (&a_ptr, ">" );
    if ( successor[0] == '\0' ) {
      free ( tmp_ancestry );
      return ( NULL );
    } /* if */
    ancestor = nxtarg (&a_ptr, ";" );
    if ( strcmp ( successor, rev ) == 0 ) {
      free ( tmp_ancestry );
      return ( strdup ( ancestor ) );
    } /* if */
  } /* for */
} /* get_ancestor */

ERR_LOG
src_ctl_outdate ( char * name, char * rev, char * set_name, 
	BOOLEAN single_revision_outdate )
{
  const char *av[16];
  ERR_LOG log;
  int i;
  char * o_branch;
  char * a_ptr;
  char * successor;
  char * ancestor;
  int len;
  char * tmp_ancestry;
  char * new_ancestry;
  int status;
  char branch_buf[MAXPATHLEN];
  char * ancestry;
  char * orig_ancestry;
  char o_string[MAXPATHLEN];
  char * symbol;
  char comma_v_file[MAXPATHLEN];
  char * new_ancestor;
  int fd;
  int anc_len;

  concat ( comma_v_file, sizeof ( comma_v_file ), name, ",v", NULL );
  for (;;) {
    if ( set_lock2 ( comma_v_file, &fd ) == WAITING ) {
      ui_print ( VNORMAL, "File locked. Waiting.\n" );
      sleep ( 5 );
    } else {
      break;
    } /* if */
  } /* for */
  i = 0;
  av[i++] = "rlog";
  av[i++] = "-r1.1";
  av[i++] = name;
  av[i] = NULL;
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  if ( log != OE_OK )
    return ( log );
  /* if */
  while ( oxm_gets( rcs_monitor, branch_buf, sizeof(branch_buf), &log ) != NULL)
    if (strncmp(branch_buf, "description:", 12) == 0)
      break;
    /* if */
  /* while */
  get_ancestry ( &ancestry, &log );
  log = oxm_endcmd( rcs_monitor, &status );

  orig_ancestry = strdup ( ancestry );
  a_ptr = ancestry;
  o_branch = get_branch ( rev );
  len = strlen ( o_branch );
  anc_len = strlen(ancestry) + 3;
  new_ancestry = (char *)malloc(anc_len);
  tmp_ancestry = (char *)malloc(anc_len);
  *new_ancestry = '\0';
  for (;;) {
    successor = nxtarg (&a_ptr, ">" );
    if ( successor[0] == '\0' || successor == NULL )
      break;
    /* if */
    ancestor = nxtarg (&a_ptr, ";" );

    if (single_revision_outdate == FALSE) {
    	/* Check ancestry against the branch we'ew trying to outdate. */
    	if ( strncmp ( ancestor, o_branch, len ) == 0 ) {
      		new_ancestor = get_ancestor ( orig_ancestry, ancestor );
      		concat ( tmp_ancestry, anc_len, new_ancestry, successor,
               		">", new_ancestor, ";", NULL );
      		strcpy ( new_ancestry, tmp_ancestry );
      		free ( new_ancestor );
    	} else if ( strncmp ( successor, o_branch, len ) != 0 ) {
      		concat ( tmp_ancestry, anc_len, new_ancestry, successor,
               		">", ancestor, ";", NULL );
    		strcpy ( new_ancestry, tmp_ancestry );
    	} /* if */

    } else {
    	/* Check ancestry against the revision we're trying to outdate. */
    	if ( strncmp ( ancestor, rev, strlen(rev) ) == 0 ) {
      		new_ancestor = get_ancestor ( orig_ancestry, ancestor );
      		concat ( tmp_ancestry, anc_len, new_ancestry, successor,
                	">", new_ancestor, ";", NULL );
      		strcpy ( new_ancestry, tmp_ancestry );
      		free ( new_ancestor );
    	} else if ( strncmp ( successor, rev, strlen(rev) ) != 0 ) {
      		concat ( tmp_ancestry, anc_len, new_ancestry, successor,
               		">", ancestor, ";", NULL );
    		strcpy ( new_ancestry, tmp_ancestry );
	}

    } /* if */


  } /* for */

  concat ( tmp_ancestry, anc_len, "-t-", new_ancestry, NULL );

  if (single_revision_outdate == TRUE) {
  	concat ( o_string, sizeof ( o_string ), "-o", rev, NULL );
	symbol = NULL;
  } else {
  	concat ( o_string, sizeof ( o_string ), "-o:", rev, NULL );

  	if ((symbol = alloc_switch('n', set_name)) == NULL) {
    		free(orig_ancestry);
    		free(ancestry);
    		free(tmp_ancestry);
    		free(new_ancestry);
   		leave ();
   		return ( err_log ( OE_ALLOC ) );
  	} /* if */
  }
  /* Perform the outdate, and update the ancestry */
  i = 0;
  av[i++] = "rcs";
  av[i++] = "-q";
  av[i++] = tmp_ancestry;
  if ( symbol != NULL )
    av[i++] = symbol;
  /* if */
  av[i++] = o_string;
  av[i++] = name;
  av[i] = NULL;
  log = oxm_runcmd ( rcs_monitor, i, av, NULL );
  if ( log != OE_OK ) {
    free(orig_ancestry);
    free(ancestry);
    free(tmp_ancestry);
    free(new_ancestry);
    return ( log );
  } /* if */
  /* unlock the file */
  unset_lock2 ( comma_v_file, fd );
  log = oxm_endcmd( rcs_monitor, &status );
  free(orig_ancestry);
  free(ancestry);
  free(tmp_ancestry);
  free(new_ancestry);
  return ( log );
} /* end src_ctl_outdate */


BOOLEAN
is_revision ( char * revision )
{

char * ptr;

if (revision == NULL) return (FALSE);

if (strstr(revision, ".") == NULL) return (FALSE);

ptr = revision;

for (ptr = revision; *ptr != '.' && *ptr != '\0';) {
        if ((*ptr < '0') || (*ptr > '9'))
                return (FALSE);
	ptr++;
}
if (*ptr != '.') return (FALSE);

for (ptr++; *ptr != '.' && *ptr != '\0';ptr++) {
        if ((*ptr < '0') || (*ptr > '9'))
                return (FALSE);
}

if (*ptr == '\0') return (TRUE);  /* If it is a trunk revision return true.*/

/* More checks for branch revisions. */
if (*ptr != '.') return (FALSE);  

for (ptr++; *ptr != '.' && *ptr != '\0';ptr++) {
        if ((*ptr < '0') || (*ptr > '9'))
                return (FALSE);
}

if (*ptr != '.') return (FALSE);

for (ptr++; *ptr != '.' && *ptr != '\0';ptr++) {
        if ((*ptr < '0') || (*ptr > '9'))
                return (FALSE);
}

if (*ptr == '.') return (FALSE);

return (TRUE);
}

BOOLEAN
is_base_of_branch ( char * revision ) 
{
char * ptr;

if (revision == NULL) return (FALSE);

if (is_revision (revision) == FALSE) return (FALSE);

ptr = revision;
for (ptr = revision; *ptr != '.' && *ptr != '\0';ptr++);
if (*ptr != '.') return (FALSE);
ptr++;
for (; *ptr != '.' && *ptr != '\0';ptr++);
if (*ptr != '.') return (FALSE);
ptr++;
for (; *ptr != '.' && *ptr != '\0';ptr++);
if (*ptr != '.') return (FALSE);
ptr++;
if (atoi(ptr) != 1)
	return (FALSE);

return (TRUE);

}
