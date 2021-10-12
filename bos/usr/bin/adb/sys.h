/* @(#)18	1.10  src/bos/usr/bin/adb/sys.h, cmdadb, bos411, 9428A410j 3/10/94 11:02:05 */
#ifndef  _H_SYS
#define  _H_SYS
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  No functions.
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  External declarations for system call library routines
 */

#include <unistd.h> 
#include <stdlib.h>
#include <sys/wait.h>

extern int  ptrace();

#endif  /*  _H_SYS */
