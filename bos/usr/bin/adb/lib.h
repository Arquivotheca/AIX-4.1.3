/* @(#)07	1.5  src/bos/usr/bin/adb/lib.h, cmdadb, bos411, 9428A410j 6/15/90 20:05:56 */
#ifndef  _H_LIB
#define  _H_LIB
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  Library call external declarations
 */

extern STRING *environ;
extern int    errno;

extern char *strncpy();
extern char *strchr();
#endif  /* _H_LIB */
