/* @(#)89       1.1  src/bos/usr/lpp/kls/dictutil/huconst.h, cmdkr, bos411, 9428A410j 5/25/92 14:40:48 */
/*
 * COMPONENT_NAME :	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS :		huconst.h
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
/********************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       huconst.h
 *
 *  Description:  header file.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ********************************************************************************/

#ifndef _HUCONST_H_ 
#define _HUCONST_H_

/*------------------------------------------------------------------------------*/
/*              	Include Heaers.						*/
/*------------------------------------------------------------------------------*/
#include <sys/types.h>

/*------------------------------------------------------------------------------*/
/*              	Define Utility Constants.				*/
/*------------------------------------------------------------------------------*/

#define U_KEY_MX	20			/* Maximum Length of a Key data */
#define U_CAN_MX	20		  /* Maximum Length of a candidate data */
#define U_MN_KEY	2			/* Minimum Length of a Key data */
#define U_MX_KEY	U_KEY_MX	        /* Maximum Length of a Key data */
#define U_MN_CAN	0	          /* Maximum Length of a candidate data */
#define U_MX_CAN	U_CAN_MX
#define U_STSLEN	2		/* User Dictionary Status Field.	*/	
#define U_MRULEN	2		/* User Dictionary mru length.		*/
#define U_MRU_MX	2048		/* Maimum Length of a use MRU.		*/
#define U_MRU_A		2048		/* A MRU AREA length.			*/
#define U_REC_L		1024		/* A data block.			*/
#define U_RLLEN		2		/* Actual Data Block Length 		*/
#define U_ILLEN		2		/* Actual Index Block Length.		*/
#define U_HARLEN	2
#define U_NARLEN	2
#define U_RRNLEN	2
#define U_SIX_A		3072
#define U_UIX_A		1024
#define U_MRU_A		2048
#define U_BASNAR	3
#define U_STSPOS	0
#define U_MRUPOS	1
#define U_ILPOS		0
#define U_HARPOS	1
#define U_NARPOS	2
#define U_RLPOS		0
#define U_SUCC		1
#define U_FAIL		-1
#define USRDICT		0
#define SYSDICT		(USRDICT+1)
#define U_KEYCAN	0
#define U_NOCAN		(U_KEYCAN+1)
#define U_NOKEY		(U_NOCAN+1)
#define U_CB_MN		2
#define U_CB_MX		1020
#define U_REC_L1  ( 1024 )          /*    1 recorde size ( 1K )         */
#define U_REC_L2  ( 2048 )          /*    2 recorde size ( 2K )         */
#define U_REC_L3  ( 3072 )          /*    3 recorde size ( 3K )         */
#define HUDICADH        ( 0 )
#define HUDICUPH        ( 1 )
#define HUDICCOM        ( 2 )
#define HUTABLE         ( 3 )
#define HUSHOW          ( 4 )

/*------------------------------------------------------------------------------*/
/*              	Define Utility Constants.				*/
/*------------------------------------------------------------------------------*/
#endif 
