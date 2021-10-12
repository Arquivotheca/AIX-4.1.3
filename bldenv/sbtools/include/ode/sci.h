/* @(#)64       1.1  src/bldenv/sbtools/include/ode/sci.h, bldprocess, bos412, GOLDA411a 1/19/94 17:37:01
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
 * $Log: sci.h,v $
 * Revision 1.2.8.6  1993/11/09  16:53:38  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:38  damon]
 *
 * Revision 1.2.8.5  1993/11/08  17:58:31  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:22  damon]
 * 
 * Revision 1.2.8.4  1993/11/05  20:34:14  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:31  damon]
 * 
 * Revision 1.2.8.3  1993/11/03  22:23:05  marty
 * 	CR # 463 - Include declarations of functions
 * 	[1993/11/03  22:22:37  marty]
 * 
 * Revision 1.2.8.2  1993/11/03  21:54:56  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  21:54:46  damon]
 * 
 * Revision 1.2.8.1  1993/11/03  20:40:14  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:37:58  damon]
 * 
 * Revision 1.2.6.1  1993/09/07  16:34:18  damon
 * 	CR 625. Fixed bsubmit -info
 * 	[1993/09/07  16:33:27  damon]
 * 
 * Revision 1.2.4.11  1993/05/27  19:43:22  marty
 * 	CR # 558 - Clean up for rios_aix build
 * 	[1993/05/27  19:43:10  marty]
 * 
 * Revision 1.2.4.10  1993/05/05  18:37:12  damon
 * 	CR 489. Added sci_select_not_exist
 * 	[1993/05/05  18:37:06  damon]
 * 
 * Revision 1.2.4.9  1993/05/05  13:04:37  damon
 * 	CR 491. Added sci_elem_cnt()
 * 	[1993/05/05  13:04:29  damon]
 * 
 * Revision 1.2.4.8  1993/05/04  17:05:51  damon
 * 	CR 463. More pedantic changes
 * 	[1993/05/04  17:05:41  damon]
 * 
 * Revision 1.2.4.7  1993/04/28  20:44:41  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:11  damon]
 * 
 * Revision 1.2.4.6  1993/04/27  15:24:03  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:19:26  damon]
 * 
 * Revision 1.2.4.5  1993/03/17  20:38:44  damon
 * 	CR 446. Added forward decls.
 * 	[1993/03/17  20:38:20  damon]
 * 
 * Revision 1.2.4.4  1993/03/04  20:19:26  marty
 * 	Add history type definitions.
 * 	[1993/03/04  20:07:32  marty]
 * 
 * Revision 1.2.4.3  1993/02/19  17:35:00  damon
 * 	CR 376. Moved sci_rcs.h to sci.h
 * 	[1993/02/19  17:32:59  damon]
 * 
 * Revision 1.2.4.2  1993/02/10  18:36:24  damon
 * 	CR 432. Tracks simple ancestry
 * 	[1993/02/10  18:35:55  damon]
 * 
 * Revision 1.2.2.8  1992/12/03  19:14:19  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:26  damon]
 * 
 * Revision 1.2.2.7  1992/11/25  19:36:22  damon
 * 	CR 331. Added REV_ALL
 * 	[1992/11/25  19:35:53  damon]
 * 
 * Revision 1.2.2.6  1992/11/06  19:02:19  damon
 * 	CR 296. Added include of error.h
 * 	[1992/11/06  19:02:06  damon]
 * 
 * Revision 1.2.2.5  1992/11/05  20:21:37  damon
 * 	CR 296. Added forward decs for some sci_* procs
 * 	[1992/11/05  20:21:11  damon]
 * 
 * Revision 1.2.2.4  1992/09/28  19:45:36  damon
 * 	CR 238. Added versions for generic 1 & 2
 * 	[1992/09/28  19:45:19  damon]
 * 
 * Revision 1.2.2.3  1992/09/24  18:41:46  gm
 * 	CR238: Added alloc_comment_leader declaration.
 * 	[1992/09/23  18:20:29  gm]
 * 
 * Revision 1.2.2.2  1992/08/06  22:05:05  damon
 * 	CR 238. Added defines for file mode and rev choice
 * 	[1992/08/06  22:04:47  damon]
 * 
 * Revision 1.2  1991/12/05  21:04:03  devrcs
 * 	Fixed bug 84: wrong # of files in log file
 * 	[91/10/29  17:06:17  damon]
 * 
 * 	Shared sandbox and comment leader NONE support
 * 	[91/08/07  12:27:47  damon]
 * 
 * 	Now using SCI_LIST/SCI_ELEM
 * 	[91/08/05  15:31:46  damon]
 * 
 * 	First version using library version of SCAPI
 * 	[91/07/31  20:13:24  damon]
 * 
 * $EndLog$
 */

#ifndef _SCI_RCS_H
#define _SCI_RCS_H
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/public/error.h>
# define STATIC static

/*
 * The innards of this abstract data type are subject to change.
 */
struct sci_elem {
  char * name;
  char * set;
  char * ver_merge;
  char * ver_user;
  char * ver_config;
  char * ver_ancestor;
  char * ver_latest;
  char * ver_generic1;
  char * ver_generic2;
  char * leader;
  char * ancestry;
  BOOLEAN same13;                                /* ver_merge = ver_ancestor */
  BOOLEAN same23;                                 /* ver_user = ver_ancestor */
  BOOLEAN defunct;
  BOOLEAN skip;                           /* file was in resub tracking file */
  BOOLEAN locked;
  BOOLEAN need_merge;
  BOOLEAN merged_up;                      /* did the user merge-up the file? */
  BOOLEAN has_user_branch;
  BOOLEAN has_merge_branch;
  BOOLEAN exists;
  int called_getancestor;
  int status;
  struct sci_elem *next;
};

struct sci_list {
  int serial_num;             /* Used to confirm that this is a valid object */
  int elem_cnt;                        /* The number of elements in the list */
  struct sci_elem * head;
  struct sci_elem * tail;
};

typedef struct sci_list * SCI_LIST;
typedef struct sci_elem * SCI_ELEM;

/*
 * File mode to use for add_to_list. Used when calling sci_init
 */
#define FILES_GIVEN	0
#define FILES_CHANGED	1
#define FILES_SAVED	2

/*
 * Which rev of an sci struct to use for an operation.
 */
#define REV_MERGE	1
#define REV_USER	2
#define REV_LATEST	3
#define REV_GENERIC1	4
#define REV_GENERIC2	5
#define REV_ALL		6

/*
 * Prototypes
 */
int
sci_init (
struct rcfile * contents,
char ** sb,
char ** sb_base,
char ** set,
char ** submit_set,
char ** sbrc_file,
int f,
BOOLEAN s );
int
sci_init2 ( char * submit_set );
int
sci_init3 ( const char * submit_set, struct rcfile * contents );
char *
alloc_comment_leader ( char * file, struct rcfile rc_info );
int 
getenv_user ( char ** user );
int
match_comment_leader ( char * comment_leader, struct rcfile rc_info );
int
sci_new_list ( SCI_LIST * sl );
int
sci_all_list ( SCI_LIST * , char *);
int
sci_add_to_list_as_is ( SCI_LIST sl, char * file_name );
int
sci_add_to_list ( SCI_LIST , char * );
SCI_ELEM
sci_first ( SCI_LIST );
SCI_ELEM
sci_next ( SCI_ELEM );
int
sci_elem_cnt ( SCI_LIST );
int
sci_lookup_leader_list ( SCI_LIST );
int
sci_lookup_merge_rev_list ( SCI_LIST sl, char * rev_str,
                                char * config_str );
int
sci_ancestor_list ( SCI_LIST sl );
int
sci_lookup_ancestor_rev_list ( SCI_LIST sl, char * rev_str,
                                   int * missing_revs );
int
sci_config_lookup_list ( SCI_LIST sl );
int
sci_merge_list ( SCI_LIST sl, BOOLEAN no_log );
ERR_LOG
sci_select_not_exist ( SCI_LIST, SCI_LIST * );
ERR_LOG
sci_ancestor_update_list( SCI_LIST );
ERR_LOG
sci_ancestor_update_list2 ( SCI_LIST sl );
int
sci_lookup_rev_list ( SCI_LIST sl, char * rev_str, BOOLEAN which_rev,
                          int * missing_revs );
int
sci_lookup_user_rev_list ( SCI_LIST , char * , int * );
int
sci_lookup_merge_rev_list ( SCI_LIST , char * ,
                                char * );
int sci_lookup_latest_rev_list ( SCI_LIST sl, char * set_name,
                                 int * missing_revs);
int
sci_lookup_ancestor_rev_list ( SCI_LIST , char * ,
                                   int * );
int
sci_ancestor_list ( SCI_LIST );
int
sci_merge_list ( SCI_LIST , BOOLEAN );
int
sci_create_files ( SCI_LIST , const char * , BOOLEAN );
ERR_LOG
sci_delete_files ( SCI_LIST sl, char * set_name );
int
sci_check_in_file ( SCI_ELEM , char * , char * ,
                    BOOLEAN );
ERR_LOG
sci_check_in_list ( SCI_LIST sl, char * build_set, char * user_set,
                    char * state, BOOLEAN no_log );
int
sci_check_in_list2 ( SCI_LIST , char * , char * , BOOLEAN );
int
sci_outdate_list_p1 ( SCI_LIST sl, char * set_name );
int
sci_outdate_list_p2 ( SCI_LIST sl, char * set_name );
int
sci_read_files ( SCI_LIST , char * );
int
sci_edit_files ( SCI_LIST , char * , char * );
ERR_LOG
sci_select_not_exists ( SCI_LIST , SCI_LIST * );
ERR_LOG
sci_show_log_list ( SCI_LIST sl, BOOLEAN rev, BOOLEAN lock_users, BOOLEAN header,
                    BOOLEAN rcs_path, BOOLEAN long_format );
ERR_LOG
sci_diff_rev_with_file ( SCI_LIST sl, BOOLEAN context, BOOLEAN whitespace );
ERR_LOG
sci_diff_rev_with_rev ( SCI_LIST sl, BOOLEAN context, BOOLEAN whitespace );
int
sci_set_comment_leader ( SCI_ELEM sci_ptr, char * leader );
int sci_set_cmt_leader_list ( SCI_LIST sl, char * leader );
ERR_LOG
sci_set_symbol_list ( SCI_LIST sl, char * sym_str, BOOLEAN override );
ERR_LOG
sci_submit ( SCI_LIST, char *, char *, BOOLEAN );
ERR_LOG
sci_rm_submit ( SCI_LIST file_set, char * set_name );
int sci_real_fast_lookup_user_rev_list ( SCI_LIST sl , char * set_name ,
                                         int * missing_revs );
int
sci_real_fast_lookup_latest_rev_list ( SCI_LIST sl , char * set_name ,
                                       int * missing_revs );
void
sci_trackfile ( char * file_name, char * log_file );
int
sci_is_branch ( SCI_LIST sl, int * bad_branches );
void
log_error ( void );
BOOLEAN is_in_error ( void );
#endif
