/* @(#)87	1.1  src/bos/usr/include/cmdnim_obj.h, cmdnim, bos411, 9428A410j  10/15/93  14:30:53 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/include/cmdnim_obj.h
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

#ifndef _H_CMDNIM_OBJ
#define _H_CMDNIM_OBJ
/*******************************************************************************
*
*                             cmdnim_obj.h
*
* includes for NIM object manipulation
*******************************************************************************/

#include "cmdnim_db.h"				/* NIM database definitions */

/*******************************************************************************
*********************** libmstr structs                *************************
*******************************************************************************/

/*---------------------------- obj_backup          --------------------------*/
/* used when backing up multiple objects */
typedef struct obj_backup {
	struct nim_object *obj;
	struct listinfo info;
} OBJ_BACKUP;
#define OBJ_BACKUP_SIZE					sizeof( OBJ_BACKUP )

/*******************************************************************************
*********************** libmstr externs                *************************
*******************************************************************************/
extern LIST backups;

#endif /* _H_CMDNIM_OBJ */

