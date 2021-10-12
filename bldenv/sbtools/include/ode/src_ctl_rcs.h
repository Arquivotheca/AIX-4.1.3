/* @(#)67       1.1  src/bldenv/sbtools/include/ode/src_ctl_rcs.h, bldprocess, bos412, GOLDA411a 1/19/94 17:37:13
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
 * $Log: src_ctl_rcs.h,v $
 * Revision 1.1.4.3  1993/11/09  16:53:40  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:39  damon]
 *
 * Revision 1.1.4.2  1993/11/08  17:58:33  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:23  damon]
 * 
 * Revision 1.1.4.1  1993/11/05  22:43:15  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  22:42:31  damon]
 * 
 * Revision 1.1.2.7  1993/05/12  19:43:14  marty
 * 	CR # 480 - Change call to src_Ctl_outdate().
 * 	[1993/05/12  16:27:28  marty]
 * 
 * Revision 1.1.2.6  1993/05/04  21:01:03  damon
 * 	CR 483. Added name parameter to src_ctl_ancestor
 * 	[1993/05/04  21:00:53  damon]
 * 
 * Revision 1.1.2.5  1993/04/29  15:44:46  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:43:54  damon]
 * 
 * Revision 1.1.2.4  1993/04/28  20:44:45  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:14  damon]
 * 
 * Revision 1.1.2.3  1993/04/27  15:24:05  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:19:28  damon]
 * 
 * Revision 1.1.2.2  1993/03/17  20:38:51  damon
 * 	CR 446. Added forward decls.
 * 	[1993/03/17  20:38:23  damon]
 * 
 * $EndLog$
 */
#include <ode/odedefs.h>

char *
get_branch ( const char * str );
ERR_LOG
src_ctl_undo_create( const char *, const char *, BOOLEAN );
ERR_LOG
src_ctl_add_symbol( const char *, const char *, BOOLEAN );
ERR_LOG
src_ctl_remove_symbol( const char *, const char * );
int
rcs_cmp_func( char *, char *, int, int * );
int
src_ctl_check_out_with_fd( const char * file_name, const char *rev, int fd,
                           const char *leader, ERR_LOG * log);
int
src_ctl_check_out ( const char * file_name, const char * rev,
                    const char * leader, ERR_LOG * log);
int
src_ctl_diff_rev_with_file( const char * rev, const char * filename,
                            const char * rcs_file, int fd, BOOLEAN context,
                            BOOLEAN whitespace, ERR_LOG * log );
int
src_ctl_show_diffs( const char *rev, const char *altrev,
                    const char * rcs_file,
                    BOOLEAN context, BOOLEAN whitespace, ERR_LOG * log );
int
src_ctl_lookup_revision ( const char * name, const char * rev_str,
                          char * revision, ERR_LOG * log);
ERR_LOG
src_ctl_ancestor( const char *, const char * ver_config, const char *rev1,
                  const char *rev2, char **rev3p,
                  int *called_getancestor, char * ancestry);
int
src_ctl_check_in ( const char * name, const char * rev,const char * logmsg,
                   const char * state, ERR_LOG * log );
int
src_ctl_create_branch( const char * rcs_file, const char * rev,
                       const char * set_name, const char * ancestor,
                       const char * cmt_leader, ERR_LOG * log );
int src_ctl_create_branch2 (
const char * filename,
const char * comma_v_file,
BOOLEAN lock,
const char * rev,
const char * set_name,
const char * ancestor,
const char * cmt_leader,
const char * msg,
ERR_LOG * log );
int
src_ctl_show_log( char * file_name, char *rev, int lock_users, int header,
                  int rcs_path, int long_format, ERR_LOG * log );
int
src_ctl_file_exists ( char *pathname, ERR_LOG * log );
int
src_ctl_get_ancestry ( char * rcs_file, char ** ancestry );
ERR_LOG
src_ctl_add_ancestry ( char *, char *);
ERR_LOG
src_ctl_add_ancestry2 ( char * rcs_file, char * new_ancestry );
ERR_LOG
src_ctl_outdate ( char * name, char * rev, char * set_name, BOOLEAN single_revision_outdate );
ERR_LOG
src_ctl_remove_file( const char * name );
int
src_ctl_lock_revision(char * , char * , ERR_LOG * );
int
numfields( const char *rev, ERR_LOG * log );
BOOLEAN
is_base_of_branch ( char * revision );
BOOLEAN
is_revision ( char * revision );
