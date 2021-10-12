/* @(#)46	1.1  src/bos/usr/lib/nls/loc/imk/include/kimdict.h, libkr, bos411, 9428A410j 5/25/92 15:37:28 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kimdict.h
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
 *  Component:    Korean IM Dictionary 
 *
 *  Module:       kimdict.h 
 *
 *  Description:  korean Input Method dictionary routine header
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

#ifndef _KIMDICT_H_
#define _KIMDICT_H_
 
/*-----------------------------------------------------------------------*
*	constant definitions
*-----------------------------------------------------------------------*/
/************************/
/* user dictionary      */
/************************/
#define	UDICT_ENVVAR		"KIMUSRDICT"
#define	UDICT_FILENAME         	".usrdict"
#define	UDICT_DEFAULTFILE	"/usr/lpp/kls/dict/usrdict"
#define	UDICT_NUMPATHS	3

/************************/
/* system dictionary    */
/************************/
#define	SDICT_ENVVAR		"KIMSYSDICT"
#define	SDICT_FILENAME         	".sysdict"
#define	SDICT_DEFAULTFILE	"/usr/lpp/kls/dict/sysdict"
#define	SDICT_NUMPATHS	2

#endif _KIMDICT_H_
