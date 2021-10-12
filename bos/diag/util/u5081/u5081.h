/* @(#)12	1.2.4.3  src/bos/diag/util/u5081/u5081.h, dsau5081, bos41J, 9513A_all 3/9/95 10:55:51 */
/*
 *   COMPONENT_NAME: DSAU5081
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _dkbd
#define _dkbd
#ifndef	CATD_ERR
#define	CATD_ERR	-1
#endif
#define ERR_FILE_OPEN	-1

#define MAXTEMP 	40 

#define MAX_BUF 	1024 
#define MAX_ERROR 	4

#define	GENERICTITLE		1
#define	NODISPLAYSFOUND		2
#define	DISPLAYSELECTION	3
#define	COLORPATTERNS		4
#define	TESTDESCRIPTION		5


#define	SOFTWARE_ERR		6
#define	MONOPATTERNS		7
#define	X_WARNING		8
#define	NO_SA			9
#define     SGACOLORPATTERNS  11
#define     WGACOLORPATTERNS  11
#define	EXIT			10

#endif 
