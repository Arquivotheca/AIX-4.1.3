/* @(#)63       1.1  src/bldenv/sbtools/include/ode/sandboxes.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:57
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
 * $Log: sandboxes.h,v $
 * Revision 1.1.4.5  1993/11/12  18:31:53  damon
 * 	CR 780. resb now handles subproject sb.conf files
 * 	[1993/11/12  18:31:42  damon]
 *
 * Revision 1.1.4.4  1993/11/04  20:39:08  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  20:38:56  damon]
 * 
 * Revision 1.1.4.3  1993/11/04  19:52:09  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  19:48:50  damon]
 * 
 * Revision 1.1.4.2  1993/11/04  00:03:20  damon
 * 	CR 463. More pedantic
 * 	[1993/11/04  00:02:50  damon]
 * 
 * Revision 1.1.4.1  1993/11/03  20:40:12  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:39:13  damon]
 * 
 * Revision 1.1.2.6  1993/06/02  13:52:00  damon
 * 	CR 517. Cleaned up subprojects wrt sb.conf and sc.conf
 * 	[1993/06/02  13:50:47  damon]
 * 
 * Revision 1.1.2.5  1993/05/27  15:17:02  damon
 * 	CR 548. Change sb_conf_read to use sb_path instead of basedir+sb
 * 	[1993/05/27  15:16:54  damon]
 * 
 * Revision 1.1.2.4  1993/05/14  16:50:23  damon
 * 	CR 518. Added sb_full_path
 * 	[1993/05/14  16:48:22  damon]
 * 
 * Revision 1.1.2.3  1993/04/28  20:12:15  damon
 * 	CR 463. Added include of parse_rc_file.h
 * 	[1993/04/28  20:12:09  damon]
 * 
 * Revision 1.1.2.2  1993/04/28  20:06:46  damon
 * 	CR 463. Prototypes
 * 	[1993/04/28  20:06:32  damon]
 * 
 * $EndLog$
 */

#include <ode/odedefs.h>
#include <ode/public/error.h>
#include <ode/parse_rc_file.h>

BOOLEAN
lock_sb ( const char * base_dir, const char * lock_file, const char * user );

int
current_sb (
    char     ** sb,                                       /* name of sandbox */
    char     ** basedir,                           /* name of base directory */
    char     ** sb_rcfile,                       /* name of sandbox rc file. */
    char     ** usr_rcfile );                   /* name and path to rc file. */

void
sb_conf_read ( struct rcfile *, const char *, char *, char * );

void
sb_conf_read_chain ( struct rcfile *rc, const char * sb_path,
                     char * project, const char * sub_project );

ERR_LOG
sb_conf_std ( struct rcfile * rc, char ** backing_project,
              char ** backing_build, BOOLEAN * ode_sc,
              BOOLEAN * ode_build_env );

ERR_LOG
sb_conf_write ( char *, char *, char *, char *, char *, BOOLEAN, BOOLEAN );

ERR_LOG
sb_conf_resb ( const char * , const char * , const char * ,
                const char *, const char * );

char *
sb_full_path ( const char *, const char * );

BOOLEAN
sb_current_dir ( const char * sbname, const char * basedir, char ** dir );

BOOLEAN in_sandbox (void);
BOOLEAN get_default_usr_rcfile ( char ** usr_rcfile, BOOLEAN report );
