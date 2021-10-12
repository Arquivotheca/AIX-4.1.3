/* @(#)47       1.1  src/bldenv/sbtools/include/ode/history.h, bldprocess, bos412, GOLDA411a 1/19/94 17:35:51
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: checklogstate
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
 * $Log: history.h,v $
 * Revision 1.1.4.2  1993/11/05  23:19:05  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  23:18:33  damon]
 *
 * Revision 1.1.4.1  1993/11/05  22:43:11  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  22:42:30  damon]
 * 
 * Revision 1.1.2.4  1993/04/28  20:44:43  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:12  damon]
 * 
 * Revision 1.1.2.3  1993/04/27  15:23:52  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:19:19  damon]
 * 
 * Revision 1.1.2.2  1993/03/17  20:38:48  damon
 * 	CR 446. Added forward decls.
 * 	[1993/03/17  20:38:18  damon]
 * 
 * $EndLog$
 */
#ifndef _HISTORY_H
#define _HISTORY_H
#include <ode/odedefs.h>
#include <ode/public/error.h>

/*
 * The following types are use for constructing and manipulating
 * the history of a file.
 */
struct comment_elem {
        char * comment;
        struct comment_elem * next;
};
struct header_info {
        char * revision;
        char * date;
        char * time;
        char * user;
};

/* The history of a file is a list of history elements where each
 * element has header info and a list of comments associated with
 * the entry.
 */
struct hist_elem {
        struct header_info header;
        int common_with_ancestor;
        struct hist_elem * next;
        struct comment_elem  * comments;
        struct comment_elem  * comment_tail;
        char *comment_leader;
};

typedef struct hist_elem * HIST_ELEM;

HIST_ELEM
hst_xtract_file ( char * path, char * comment_leader );
void
hst_insert_file ( char *path, HIST_ELEM history, char *comment_leader );
HIST_ELEM
new_hist_elem ( char * line, char * comment_leader);
HIST_ELEM
hst_alloc_entry ( char * comment_leader, char * logmsg, char * revision );
void
hst_freelist( HIST_ELEM node );
HIST_ELEM
hst_merge_lists ( HIST_ELEM ver_merge,  /*  The version to merge to. */
                  HIST_ELEM ver_user,   /*  The user's current version. */
                  HIST_ELEM ver_ancestor );
                                    /* The common ancestor of the above two. */
HIST_ELEM
hstdup( HIST_ELEM node);
ERR_LOG
hst_lookup_logmsg ( char *rev, char *leader,
                    BOOLEAN condense, char *xlog, int nxlog );
char *
hst_next_revision ( char * );

ERR_LOG
checklogstate( const char *file_name,
               const char *leader,   /* comment leader (if any) */
               BOOLEAN * improper ); /* bad copyright, leader line, etc. */
ERR_LOG
okmesg( char *leader, char *logbuf);
ERR_LOG
reinsert_log_messages( char *mesgfile );
ERR_LOG
save_log_message( char *mesgfile );
int
create_leaderless_log ( const char * name, const char * user_set,
                        char * logmsg, const char * mfile );
#endif
