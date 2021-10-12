/* @(#)23       1.18  src/bos/kernel/sys/POWER/cdrom.h, sysxcdrm, bos411, 9428A410j 2/24/93 18:15:57 */
#ifndef  _H_CDROM
#define _H_CDROM
/*
 * COMPONENT_NAME: (SYSXCDRM) SCSI CD-ROM Device Driver Include File
 *
 * FUNCTIONS:	NONE
 * 
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*********************************************************************/
/* cdrom.h header dependencies					     */
/* This is file is provided for backward compatibility with previous */
/* releases of AIX.  All new code should now use scdisk.h.           */
/*********************************************************************/
#include <sys/scsi.h>
#include <sys/scdisk.h>
#include <sys/types.h>

/************************************************************************/
/* Ioctl Command defines                                                */
/************************************************************************/
#define CDIORDSE                DKIORDSE  /* Read with sense on error     */
#define CDIOCMD                 0x02      /* Pass-through command         */

 
#endif
