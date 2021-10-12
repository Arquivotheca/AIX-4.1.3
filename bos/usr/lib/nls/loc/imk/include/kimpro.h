/* @(#)48	1.1  src/bos/usr/lib/nls/loc/imk/include/kimpro.h, libkr, bos411, 9428A410j 5/25/92 15:38:04 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kimpro.h
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
 *  Component:    Korean IM Profile
 *
 *  Module:       kimpro.h
 *
 *  Description:  korean Input Method profile routine header 
 *
 *  History:      5/22/90  Initial Creation.     
 * 
 ******************************************************************/

#ifndef _KIMPRO_H_
#define _KIMPRO_H_

/*-----------------------------------------------------------------------*
*	constant definitions
*-----------------------------------------------------------------------*/
#define	ENVVAR		"KIMPROFILE"
#define	FILENAME	".kimrc"
#define	DEFAULTFILE	"/usr/lpp/kls/defaults/kimrc"
#define	NUMPATHS	3
#define MAXWORDSIZE     256
#define KP_OK 0				/* return code for internal use */
#define KP_MAKEERR 1			/* return code for internal use */

#endif _KIMPRO_H_
