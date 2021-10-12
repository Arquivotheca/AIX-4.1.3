/* @(#)53       1.1  src/bldenv/sbtools/include/ode/parse_rc_file.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:20
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: RC_HASH
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
 * $Log: parse_rc_file.h,v $
 * Revision 1.6.6.1  1993/11/03  20:40:07  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:37:56  damon]
 *
 * Revision 1.6.4.3  1993/04/28  18:06:17  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  18:06:05  damon]
 * 
 * Revision 1.6.4.2  1993/04/27  15:24:01  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:19:24  damon]
 * 
 * Revision 1.6.2.2  1992/12/03  19:14:09  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:43:16  damon]
 * 
 * Revision 1.6  1991/12/05  21:03:56  devrcs
 * 	Added _FREE_ to copyright marker
 * 	[91/08/01  08:10:21  mckeen]
 * 
 * 	rcsid/RCSfile header cleanup
 * 	[90/12/01  18:38:48  dwm]
 * 
 * Revision 1.4  90/10/07  20:35:30  devrcs
 * 	Added EndLog Marker.
 * 	[90/09/28  20:57:26  gm]
 * 
 * Revision 1.3  90/08/09  14:28:14  devrcs
 * 	Moved here from usr/local/sdm/include.
 * 	[90/08/05  12:35:36  gm]
 * 
 * Revision 1.2  90/05/24  23:12:22  devrcs
 * 	Created from old "project_db.h" file.
 * 	[90/05/03  15:05:28  randyb]
 * 
 * $EndLog$
 */
/* @(#)$RCSfile: parse_rc_file.h,v $ $Revision: 1.6.6.1 $ (OSF) $Date: 1993/11/03 20:40:07 $ */
/*
 * data structures used to hold parsed rc file for sandboxes
 */

#ifndef _PAR_RC_FILE_H
#define _PAR_RC_FILE_H
struct arg_list {
    struct arg_list *next;
    char **tokens;
    int ntokens;
    int maxtokens;
};

struct field {
    struct field *next;		/* next field */
    char *name;			/* name of this field */
    struct arg_list *args;	/* args for this field */
};

#define RC_HASHBITS 6		/* bits used in hash function */
#define RC_HASHSIZE (1<<RC_HASHBITS) /* range of hash function */
#define RC_HASHMASK (RC_HASHSIZE-1)	/* mask for hash function */
#define RC_HASH(i) ((i)&RC_HASHMASK)

struct hashent {
    struct hashent *next;
    struct field *field;
};

struct rcfile {
    struct hashent *hashtable[RC_HASHSIZE];
    struct field *list;
    struct field *last;
};

int
append_arg( char *, struct arg_list *);

int
create_arglist( struct field *, struct arg_list **);

int
find_field( struct rcfile *, const char *, struct field **, int );

int
init_rc_contents ( 
    struct      rcfile  * rc_contents,             /* structure to initialze */
    char      * sbrc );                      /* file with source information */

int
get_rc_value (
    const char * keyword,                                 /* word to look for */
    char     ** output,                         /* holds output from keyword */
    struct      rcfile  * contents,             /* holds contents of rc file */
    int         report_missing );                              /* error flag */

int
parse_rc_file( char *, struct rcfile *);
int
rci_expand_include( char **, char *);
#endif
