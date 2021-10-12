/* @(#)10        1.2  src/bos/kernel/sys/POWER/idecdrom.h, idecd, bos41J, 9510A_all 2/16/95 14:33:27 */
#ifndef _H_IDECDROM
#define _H_IDECDROM
/*
 * COMPONENT_NAME: (IDECD) IDE CD-ROM Device Driver Include File
 *
 * FUNCTIONS:  NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/* idecdrom.h header dependencies                                       */
/************************************************************************/

#include "scdisk.h"

/************************************************************************/
/* IDE CD-ROM Ioctls                            			*/
/************************************************************************/

#define IDE_CDIORDSE	DKIORDSE        /* Read and return sense data on*/
					/* error.  			*/
#define IDE_CDEJECT	DKEJECT		/* Eject Media from drive      	*/
#define IDE_CDPMR	DKPMR		/* Prevent media removal 	*/
#define IDE_CDAMR	DKAMR		/* Allow media removal		*/
#define IDE_CDAUDIO	DKAUDIO		/* CD-ROM Play Audio Operations	*/
#define IDE_CDMODE	DK_CD_MODE	/* Get or Change CD-ROM data 	*/
					/* modes 			*/


/************************************************************************/
/* other IDE #defines                                  			*/
/************************************************************************/

/* Modes for OPENX */
#define IDE_SINGLE	SC_SINGLE	/* Single Open for audio and  */
					/* changing modes             */

/************************************************************************/
/* Structure for Play Audio Operations on CD-ROM drives 		*/
/************************************************************************/

/* see scdisk.h */
#endif /* _H_IDECDROM */
