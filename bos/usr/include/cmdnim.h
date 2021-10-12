/* @(#)54	1.16  src/bos/usr/include/cmdnim.h, cmdnim, bos411, 9428A410j  2/19/94  18:04:52 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CMDNIM
#define _H_CMDNIM
/*******************************************************************************
*
*                             cmdnim.h
*
* common includes for all NIM commands
*******************************************************************************/

/*---------------------------- common includes     ---------------------------*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <regex.h>

#include "cmdnim_defines.h"	/* common NIM defines */

#include "cmdnim_msg.h"		/* message numbers generated from mkcatdefs */

#include "cmdnim_msgdefs.h"	/* attribute definitions from cmdnim.msg */

#include "cmdnim_structs.h"	/* common NIM structs */

/*---------------------------- global var externs  ---------------------------*/
/* README: any change here MUST be reflected in the cmdnim_cmd.h file */

extern struct nim_info	niminfo;
extern void *errsig[SIGMAX];
extern int protect_errstr;
extern void (*undo_on_interrupt)();
extern ATTR_ASS_LIST attr_ass;
extern char *reserved_words[];
extern struct nim_ere nimere[];
extern int force;
extern int verbose;
extern int ignore_lock;

#endif /* _H_CMDNIM */
