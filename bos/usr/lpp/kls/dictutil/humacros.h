/* @(#)23	1.1  src/bos/usr/lpp/kls/dictutil/humacros.h, cmdkr, bos411, 9428A410j 5/25/92 14:46:56 */
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		humacros.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       humacros.h
 *
 *  Description:  Defines User Dictionary Maintenance Macros.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

#ifndef _HUMACROS_H_
#define _HUMACROS_H_

/*    Types Define.  */
#include <sys/types.h>

/*    Which is Miminum Number Get.  */
#define MIN( a,b )  (( (a)<=(b) ) ? a:b)

/*    Which is Maximum Number Get.  */
#define MAX( a,b )  (( (a)> (b) ) ? a:b)

/*    Cursor Move.  */
#define CURSOR_MOVE(lin,col) putp(tparm(cursor_address,lin,col))
#endif
