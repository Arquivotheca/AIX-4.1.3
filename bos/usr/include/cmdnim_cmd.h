/* @(#)16	1.19  src/bos/usr/include/cmdnim_cmd.h, cmdnim, bos411, 9428A410j  5/10/94  15:42:43 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim_cmd.h
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

#ifndef _H_CMDNIM_CMD
#define _H_CMDNIM_CMD
/*******************************************************************************
*
*                             cmdnim_cmd.h
*
* common definitions for all NIM commands
*******************************************************************************/

#include "cmdnim.h"					/* common NIM definitions */


/*----------------------------- NIM globals       ----------------------------*/
/* README: if you change any of these, you must change the correspdoning */
/*		extern statement in cmdnim.h */

struct nim_info	niminfo;					/* info needed by all NIM commands */

void *errsig[SIGMAX];						/* ptrs to error signal handlers */

int protect_errstr=FALSE;					/* TRUE if current value of errstr */
													/*		should not be overwritten */

void (*undo_on_interrupt)()=NULL;		/* ptr to function which will perform */
													/*		undo operations when an */
													/*		interrupt occurs */

int force = 0;									/* >0 if ATTR_FORCE present */

int verbose = 0;								/* >0 if verbose msgs requested */

int ignore_lock = FALSE;					/* >0 if locking is to be bypassed */

char *reserved_words[] = {					/* NIM reserved words */
	ATTR_MASTER_T,								/* note: array MUST be NULL terminated */
	ATTR_BOOT_T,
	ATTR_NIM_SCRIPT_T,
	NULL
};

struct nim_ere nimere[] = {				/* NIM regular expressions */
	ERE_BAD_NAME, NULL,
	ERE_BAD_LOCATION, NULL,
	ERE_IF, NULL,
	ERE_IF_STANZA, NULL,
	ERE_HARD_ADDR, NULL,
	ERE_TWO_FIELDS, NULL,
	ERE_THREE_FIELDS, NULL,
	ERE_FOUR_FIELDS, NULL,
	ERE_IP_ADDR, NULL,
	ERE_HOSTNAME, NULL,
	ERE_FIRST_FIELD, NULL,
	ERE_ATTR_ASS, NULL,
	ERE_ATTR_NAME, NULL,
	ERE_SEQNO, NULL,
	ERE_WARNING_MSG, NULL,
	ERE_DEVICE, NULL,
	ERE_FILENAME, NULL,
	ERE_ATTR_TRANS, NULL,
	ERE_RC, NULL
};

#endif /* _H_CMDNIM_CMD */
