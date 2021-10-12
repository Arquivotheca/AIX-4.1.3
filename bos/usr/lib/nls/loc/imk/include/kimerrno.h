/* @(#)47	1.1  src/bos/usr/lib/nls/loc/imk/include/kimerrno.h, libkr, bos411, 9428A410j 5/25/92 15:37:48 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kimerrno.h
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

/******************** START OF MODULE SPECIFICATIONS ********************
*
* MODULE NAME:		kimerrno.h
*
* DESCRIPTIVE NAME:  
*
* MODULE TYPE:		C
*
* COMPILER:		AIX C
*
* CHANGE ACTIVITY:	010/10/89 - Modified
*
* STATUS:		korean Input Method Version 
*
* CATEGORY:		Header
*
******************** END OF SPECIFICATIONS *****************************/
 
#ifndef	_h_KIMERRNO
#define	_h_KIMERRNO

/***********************/
/* korean Input Method */
/***********************/
#define KIMDictError 	1  			/* dictionary access failure */
#define KIMEDError   (KIMDictError + 1)  	/* KIM editor error          */

#endif	_h_KIMERRNO
