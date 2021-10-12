/* @(#)65       1.1  src/bldenv/sbtools/include/ode/sci_client.h, bldprocess, bos412, GOLDA411a 1/19/94 17:37:05
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
 * Copyright (c) 1993, 1992, 1991, 1990 Open Software Foundation, Inc.
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
 */
/*
 * HISTORY
 * $Log: sci_client.h,v $
 * Revision 1.1.2.5  1993/11/03  23:43:01  damon
 * 	Merged with changes from 1.1.2.4
 * 	[1993/11/03  23:42:38  damon]
 *
 * 	CR 463. More pedantic
 * 	[1993/11/03  23:40:53  damon]
 *
 * Revision 1.1.2.4  1993/11/03  22:23:08  marty
 * 	CR # 463 - Include declarations of functions
 * 	[1993/11/03  22:22:39  marty]
 * 
 * Revision 1.1.2.3  1993/11/03  22:02:44  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  22:02:35  damon]
 * 
 * Revision 1.1.2.2  1993/11/03  21:55:28  marty
 * 	CR # 463 - Type cast sci_show_log_list_client()
 * 	[1993/11/03  21:55:08  marty]
 * 
 * Revision 1.1.2.1  1993/11/03  20:58:13  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:57:43  damon]
 * 
 * $EndLog$
 */
#ifndef _SCI_CLIENT_H
#define _SCI_CLIENT_H
#include <ode/sci.h>
int
sci_check_out_client ( SCI_LIST , const char * , BOOLEAN ,
                       const char * , ERR_LOG * );
ERR_LOG
sci_show_log_list_client ( SCI_LIST file_set, char * rev, BOOLEAN all_revs,
                           BOOLEAN lock_users, BOOLEAN header, BOOLEAN rcs_path,
                           BOOLEAN long_format );
ERR_LOG
sci_client_latest_rev_list ( SCI_LIST file_set, BOOLEAN rev_option,
                             const char * rev_label, BOOLEAN wantfilename );
ERR_LOG
sci_server_bcs_opts ( SCI_LIST file_set, BOOLEAN revision_option,
                      BOOLEAN outdate_option,
                      const char * symbolic_name, const char * submit_set,
                      BOOLEAN leader_option,
                      const char * leader, BOOLEAN big_symname_option,
                      const char * big_symname, BOOLEAN small_symname_option,
                      const char * small_symname );
ERR_LOG
sci_submit_client ( SCI_LIST file_set, SCI_LIST * file_set2,
                    const char * user_set, const char * user_sandbox,
                    const char * submit_set, const char * expanded_config,
                    const char * submit_user_name, const char * defect_number,
                    BOOLEAN already_held, BOOLEAN info, const char * tag,
                    BOOLEAN resub, BOOLEAN rmove,
                    BOOLEAN outdate, const char * stime, const char * sdate,
                    BOOLEAN * in_submit_stage, char * rtime, char * rdate );
#endif
