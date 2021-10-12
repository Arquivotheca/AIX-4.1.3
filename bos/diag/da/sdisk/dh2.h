/* @(#)82	1.2  src/bos/diag/da/sdisk/dh2.h, dasdisk, bos411, 9428A410j 12/11/92 09:31:22 */
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_DH2SUB
#define _H_DH2SUB

#ifndef         TRUE
#define         TRUE 1
#endif
#ifndef         FALSE
#define         FALSE 0
#endif

#define         PASS_PARAM DKIOCMD            /* Pass-thru param needed by DD */
#define		SRCHSTR	"%s -N %s"	/* basic search string used in ELA */
#define		MAX_HRD_ERRS	3
#define		MAX_ERRS	6
#define		BUFFER_LENGTH	255	/* TU buffer length		   */
#define		DISK	1
#define		CONTROLLER 2
#define		ADAPTER	3
long	damode;				/* diagnostic mode		   */
int	menu_return;			/* return value from menu selection */

#endif
