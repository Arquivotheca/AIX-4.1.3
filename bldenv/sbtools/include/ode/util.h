/* @(#)68       1.1  src/bldenv/sbtools/include/ode/util.h, bldprocess, bos412, GOLDA411a 1/19/94 17:37:17
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: searchp
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
 * $Log: util.h,v $
 * Revision 1.1.6.5  1993/11/10  15:35:25  damon
 * 	CR 463. Changed unsigned types to const char
 * 	[1993/11/10  15:35:12  damon]
 *
 * Revision 1.1.6.4  1993/11/09  16:53:42  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/09  16:52:40  damon]
 * 
 * Revision 1.1.6.3  1993/11/08  20:18:00  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:20  damon]
 * 
 * Revision 1.1.6.2  1993/11/08  17:58:35  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  17:58:01  damon]
 * 
 * Revision 1.1.6.1  1993/11/03  20:40:17  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:39:15  damon]
 * 
 * Revision 1.1.4.1  1993/08/19  16:40:28  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  16:39:59  damon]
 * 
 * Revision 1.1.2.10  1993/05/05  18:49:48  marty
 * 	Put back old definition of opentemp().
 * 	[1993/05/05  18:49:29  marty]
 * 
 * Revision 1.1.2.9  1993/05/05  14:34:29  marty
 * 	Change prototyping of opentemp() from const char * to
 * 	char *.
 * 	[1993/05/05  14:34:19  marty]
 * 
 * Revision 1.1.2.8  1993/05/04  17:05:53  damon
 * 	CR 463. More pedantic changes
 * 	[1993/05/04  17:05:42  damon]
 * 
 * Revision 1.1.2.7  1993/04/29  17:30:07  damon
 * 	CR 463. Fixed fprstab prototype
 * 	[1993/04/29  17:30:01  damon]
 * 
 * Revision 1.1.2.6  1993/04/29  15:44:48  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:43:55  damon]
 * 
 * Revision 1.1.2.5  1993/04/28  20:44:47  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:15  damon]
 * 
 * Revision 1.1.2.4  1993/04/28  15:29:28  damon
 * 	CR 463. More protos
 * 	[1993/04/28  15:29:21  damon]
 * 
 * Revision 1.1.2.3  1993/04/28  13:00:03  damon
 * 	CR 463. Lots of prototypes added
 * 	[1993/04/28  12:59:57  damon]
 * 
 * Revision 1.1.2.2  1993/04/27  20:42:43  damon
 * 	CR 463. Added util.h
 * 	[1993/04/27  20:42:23  damon]
 * 
 * $EndLog$
 */
#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <time.h>

int
abspath ( char * name, char * result );
int
amatch(const char *, const char * );
char *
#ifdef __STDC__
concat(char *buf, int buflen, ...);
#else
concat(va_alist);
#endif
int
editor( const char *, const char *);
char *
fdate ( char *, const char *, struct tm *);
int
filecopy ( int , int );
int
ffilecopy ( FILE *here, FILE *there );
char *
folddown ( char *, char *);
char *
foldup ( char *, char * );
void
fprstab ( FILE *, const char *[]);
int
free_disk_space( int , char *);
int
getbool ( const char *, int );
int
getstab ( const char *, const char *table[], const char *defalt );
char *
getstr ( const char *, const char *defalt, char *answer );
int
gmatch (const char *, const char * );
int
makepath( char *, char *, int , int );
char *
nxtarg ( char **, const char * );
void
path ( const char *, char *, char *);
int
opentemp( const char *, char *);
void
prstab ( const char *[]);
void
#ifdef __STDC__
quit (int , const char *, ...);
#else
quit (va_alist);
#endif
void
rm_newline ( char * string );
int
searchp ( const char *, const char *, const char *, int (*func)( const char * ) );
char *
skipover ( const char *string, const char *charset );
char *
skipto ( const char *string, const char *charset );
int
stablk ( const char *arg, const char *table[], int quiet );
int
stlmatch ( const char *small, const char *big );
int
umatch(const char *, const char * );
void
#ifdef __STDC__
uquit ( int status, ... );
#else
uquit ( va_alist );
#endif
#endif
