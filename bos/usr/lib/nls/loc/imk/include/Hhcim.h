/* @(#)38	1.1  src/bos/usr/lib/nls/loc/imk/include/Hhcim.h, libkr, bos411, 9428A410j 5/25/92 15:35:58 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		Hhcim.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM HHC
 *
 *  Module:       Hhcim.c
 *
 *  Description:  Korean Im Hhc Header.
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

/********************/
/* HHC header.	    */
/********************/
#ifndef	_HHCIM_H
#define _HHCIM_H

#define		MSB			0x8000
#define		MSB_CLEAR		0x7fff
#define         CMP(x,y)        	((x) - (y))
#define         IXBLOCK_SIZE            (2*1024)
#define         DBLOCK_SIZE             (1*1024)
#define         MAX_LIST                  100
#define         MAXSTR                     50 
#define         FILE_NAME_LEN            MAXSTR
#define         SYS_DICTIONARY          "dict.ks"
#define		RESET_15_AND_7_BIT	0x7f7f
#define		SET_15_AND_7_BIT	0x8080
#define		IMACAND_MAXLEN		  40
#define		TRUE			   1
#define		FALSE			   0
#define		BufEmptyMask		0xffff
#define		IsBufEmpty(x)		(!((x) ^ BufEmptyMask))
typedef		unsigned short int 	KIMword;

#define	SET_MAKEKEY2RBN		0x80
#define	RESET_MAKEKEY2RBN	0x7f
#define	LastCharOfKey		0x8000
#define	IsLastCharOfKey(x)	((x) & LastCharOfKey)

#define	SET_MAKECAND2RBN	0x80
#define	RESET_MAKECAND2RBN	0x7f
#define	LastCharOfCand		0x0080
#define	IsLastCharOfCand(x)	((x) & LastCharOfCand)
#define	LastCandOfKey		0x8000
#define	IsLastCandOfKey(x)	((x) & LastCandOfKey)
#define	IsLastCCOfKey(x) 	(IsLastCandOfKey(x) && IsLastCharOfCand(x))

#define	IBMUnique		0x7f7f
#define	IsIBMUniqueCode(x)	(!((x) & IBMUnique))

#define UD_NORMAL	0x0000
#define UD_ORDONLY	0x000f
#define UD_ORDWR	0x00f0
#define UD_DICUTL	0x00ff

#define UDP_RDONLY	0
#define UDP_UPDATE	(UDP_RDONLY+1)

#define U_HAR_V1 (   6     )/* User Dictionary size HAR Data 1         */
#define U_HAR_V2 (   53    )/* User Dictionary size HAR Data 2         */

/***************************/
/*                         */
/* Define MRU Entry Status */
/*                         */
/***************************/
#define UD_MRU_OLD 0
#define UD_MRU_NEW (UD_MRU_OLD+1)

/*------------------------------------------------------------------------------*/
/*  Candidates Source.								*/
/*------------------------------------------------------------------------------*/

#define UD_FROM_NONE 0x0000
#define UD_FROM_MRU  0x00f0
#define UD_FROM_USR  0x0f00
#define UD_FROM_SYS  0xf000

/********************/
/* HHC header.	    */
/********************/
#endif
