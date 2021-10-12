/* @(#)85	1.1  src/bos/usr/lpp/kls/dictutil/hhccomm.h, cmdkr, bos411, 9428A410j 5/25/92 14:40:10 */
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hhccomm.h
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
 *  Module:       hhccom.h
 *
 *  Description:  hangul / hanja conversion include file.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ********************************************************************************/

#ifndef	_HHCCOMM_H
#define _HHCCOMM_H

/*------------------------------------------------------------------------------*/	
/* 		Utility and HHC Common Header file.				*/
/*------------------------------------------------------------------------------*/	

#define CMP(x,y)        	((x) - (y))
#define	BufEmptyMask		0xffff
#define	IsBufEmpty(x)		(!((x) ^ BufEmptyMask))
#define	LastCharOfKey		0x8000
#define	IsLastCharOfKey(x)	((x) & LastCharOfKey)
#define	LastCharOfCand		0x0080
#define	IsLastCharOfCand(x)	((x) & LastCharOfCand)
#define	LastCandOfKey		0x8000
#define	IsLastCandOfKey(x)	((x) & LastCandOfKey)
#define	IsLastCCOfKey(x) 	(IsLastCandOfKey(x) && IsLastCharOfCand(x))
#define	IBMUnique		0x7f7f
#define	IsIBMUniqueCode(x)	(!((x) & IBMUnique))

/*------------------------------------------------------------------------------*/	
/* 		Utility and HHC Common Header file.				*/
/*------------------------------------------------------------------------------*/	
#endif
