/* @(#)50       1.1  src/bldenv/sbtools/include/ode/misc.h, bldprocess, bos412, GOLDA411a 1/19/94 17:36:05
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: insert_line_in_sorted_file
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
 */
/*
 * HISTORY
 * $Log: misc.h,v $
 * Revision 1.1.2.3  1993/04/27  15:23:56  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:19:23  damon]
 *
 * Revision 1.1.2.2  1993/04/26  21:59:22  damon
 * 	CR 446. New declarations for misc.c
 * 	[1993/04/26  21:59:12  damon]
 * 
 * 	CR 446. New declarations for misc.c
 * 
 * $EndLog$
 */

#include <stdarg.h>
#include <ode/odedefs.h>

char * alloc_switch ( char, const char *);
void enter ( const char * );
void leave ( void );
void report_current_function ( void );
void atomic_init ( void );
void begin_atomic ( void );
void end_atomic ( void );
int exists ( const char * );
BOOLEAN isdir ( const char * );
void get_date ( char *, char * );
int insert_line_in_sorted_file ( char *, char *, int (*cmp_func)(char *, ...),
                                 int );
int canonicalize ( const char *, const char *, char *, int );
BOOLEAN str_to_BOOLEAN ( const char * );
