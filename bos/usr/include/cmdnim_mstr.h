/* @(#)47	1.4  src/bos/usr/include/cmdnim_mstr.h, cmdnim, bos411, 9428A410j  10/15/93  14:29:23 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim_mstr.h
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

#ifndef _H_CMDNIM_MSTR
#define _H_CMDNIM_MSTR
/*******************************************************************************
*
*                             cmdnim_mstr.h
*
* includes for all NIM master commands
*******************************************************************************/

#include "cmdnim_cmd.h"				/* common includes for all NIM commands */

#include "cmdnim_obj.h"				/* NIM object manipulation */

/*******************************************************************************
*********************** libmstr globals                *************************
*******************************************************************************/
struct nim_net *net_info;					/* caches network information */
int num_net_info=-1;							/* num of net_info structs */
struct nim_client_if *head_client_if;	/* ptr to start of linked list */

LIST backups = {								/* LIST of OBJ_BACKUP structs */
	NULL,											/* used in restore_backups */
	0,												/* VERY IMPORTANT that LIST.num */
	0,												/* be initialized to 0 here - */
	DEFAULT_CHUNK_SIZE						/* we will check it in mstr_exit */
};

#endif /* _H_CMDNIM_MSTR */


