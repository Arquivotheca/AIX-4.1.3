/* @(#)64	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jdict/jimdict.h, libKJI, bos411, 9428A410j 7/23/92 01:43:47	*/
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_jimdict_h
#define	_jimdict_h

/*-----------------------------------------------------------------------*
*	constant definitions
*-----------------------------------------------------------------------*/

/* user dictionary */
#define	UDICT_ENVVAR		"JIMUSRDICT"
#define	UDICT_FILENAME         	".usrdict"
#define	UDICT_DEFAULTFILE	"/usr/lpp/jls/dict/usrdict"
#define	UDICT_NUMPATHS	3
/* adjunct dictionary */
#define	ADICT_ENVVAR		"JIMADJDICT"
#define	ADICT_FILENAME         	".adjdict"
#define	ADICT_DEFAULTFILE	"/usr/lpp/jls/dict/adjdict"
#define	ADICT_NUMPATHS	3
/* system dictionary */
#define	SDICT_ENVVAR		"JIMMULDICT"
#define	SDICT_DEFAULTFILE	"/usr/lpp/jls/dict/ibmbase"
#define	SDICT_NUMPATHS	2
#define SDICT_NUM               16
#define SDICT_LEN               80
#define SDICT_SUCCESS           0
#define SDICT_ERROR             (-1)

#define FALSE  			0
#define TRUE   			1

#endif	/* _jimdict_h */
