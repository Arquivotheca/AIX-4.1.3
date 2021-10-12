static char sccsid[] = "@(#)52	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kumsg.c, cmdKJI, bos411, 9428A410j 7/23/92 01:28:44";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kumsg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************  *
 * MODULE NAME:         kumsg
 *
 * DESCRIPTIVE NAME:    display message
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        548 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kumsg
 *
 *  PURPOSE:            See Function.
 
 *  LINKAGE:            int kumsg( udcb , line , col , msgid )
 *
 *  INPUT:              udcb            : pointer to UDCB
 *                      line            : display line
 *                      col             : display colmn
 *                      msgid           : message ID
 *
 *  OUTPUT:             NA.
 *
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              kudisply
 *                      Standard Liblary.
 *                              memset
 *                              fflush
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include "kut.h"
/*#include <memory.h>*/

#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

#include "kje.h"


/*
 * Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

#if defined(MSGFILE)
int kumsg( udcb , line , col , msgid )

UDCB    *udcb;          /* pointer to UDCB.                             */
short   line;           /* start line.                                  */
short   col;            /* start column.                                */
short   msgid;          /* message ID.                                  */

{
	int     kudisply();
	UECB    *uecb;  /* pointer to UECB.                             */
	char    blkmsg[256];
	char    errmsg[256];
	int     wrc;
	short   data_length = 54;

	/* display error message.                                       */
	memset(blkmsg,0x20,(int)(data_length));
	memset(errmsg,0x20,(int)(data_length));

	switch ( msgid ) {
	    case U_AMSGN :
		(void)strcpy(errmsg, CU_AMSGN);
                break;
	    case U_BMSGN :
		(void)strcpy(errmsg, CU_BMSGN);
		break;
	    case U_CMSGN :
		(void)strcpy(errmsg, CU_CMSGN);
		break;
	    case U_DMSGN :
		(void)strcpy(errmsg, CU_DMSGN);
		break;
	    case U_EMSGN :
		(void)strcpy(errmsg, CU_EMSGN);
		break;
	    case U_FMSGN :
		(void)strcpy(errmsg, CU_FMSGN);
		break;
	    case U_GMSGN :
		(void)strcpy(errmsg, CU_GMSGN);
		break;
	    case U_HMSGN :
		(void)strcpy(errmsg, CU_HMSGN);
		break;
	    case U_IMSGN :
		(void)strcpy(errmsg, CU_IMSGN);
		break;
	    case U_JMSGN :
		(void)strcpy(errmsg, CU_JMSGN);
		break;
	    case U_KMSGN :
		(void)strcpy(errmsg, CU_KMSGN);
		break;
	    case U_LMSGN :
		(void)strcpy(errmsg, CU_LMSGN);
		break;
	    case U_MMSGN :
		(void)strcpy(errmsg, CU_MMSGN);
		break;
	    case U_NMSGN :
		(void)strcpy(errmsg, CU_NMSGN);
		break;
	    case U_OMSGN :
		(void)strcpy(errmsg, CU_OMSGN);
		break;
	    case U_PMSGN :
		(void)strcpy(errmsg, CU_PMSGN);
		break;
	    case U_QMSGN :
		(void)strcpy(errmsg, CU_QMSGN);
		break;
	    case U_RMSGN :
		(void)strcpy(errmsg, CU_RMSGN);
		break;
	    case U_SMSGN :
		(void)strcpy(errmsg, CU_SMSGN);
		break;
	    case U_TMSGN :
		(void)strcpy(errmsg, CU_TMSGN);
		break;
	    case U_UMSGN :
		(void)strcpy(errmsg, CU_UMSGN);
		break;
	    case U_VMSGN :
		(void)strcpy(errmsg, CU_VMSGN);
		break;
	    case U_WMSGN :
		(void)strcpy(errmsg, CU_WMSGN);
		break;
	    case U_XMSGN :
		(void)strcpy(errmsg, CU_XMSGN);
		break;
	    case U_YMSGN :
		(void)strcpy(errmsg, CU_YMSGN);
		break;
	    case U_ZMSGN :
		(void)strcpy(errmsg, CU_ZMSGN);
		break;
	    case U_AAMSGN :
		(void)strcpy(errmsg, CU_AAMSGN);
		break;
	    case U_ABMSGN :
		(void)strcpy(errmsg, CU_ABMSGN);
		break;
	    case U_ACMSGN :
		(void)strcpy(errmsg, CU_ACMSGN);
		break;
	    case U_ADMSGN :
		(void)strcpy(errmsg, CU_ADMSGN);
		break;
	    case U_AEMSGN :
		(void)strcpy(errmsg, CU_AEMSGN);
		break;
	    case U_AFMSGN :
		(void)strcpy(errmsg, CU_AFMSGN);
		break;
	    case U_AGMSGN :
		(void)strcpy(errmsg, CU_AGMSGN);
		break;
	    case U_AHMSGN :
		(void)strcpy(errmsg, CU_AHMSGN);
		break;
	    case U_AIMSGN :
		(void)strcpy(errmsg, CU_AIMSGN);
		break;
	    case U_AJMSGN :
		(void)strcpy(errmsg, CU_AJMSGN);
		break;
	    case U_AKMSGN :
		(void)strcpy(errmsg, CU_AKMSGN);
		break;
	    case U_ALMSGN :
		(void)strcpy(errmsg, CU_ALMSGN);
		break;
	    case U_AMMSGN :
		(void)strcpy(errmsg, CU_AMMSGN);
		break;
	    case U_ANMSGN :
		(void)strcpy(errmsg, CU_ANMSGN);
		break;
	    case U_AOMSGN :
		(void)strcpy(errmsg, CU_AOMSGN);
		break;
	    case U_APMSGN :
		(void)strcpy(errmsg, CU_APMSGN);
		break;
	    case U_AQMSGN :
		(void)strcpy(errmsg, CU_AQMSGN);
		break;
	    case U_ARMSGN :
		(void)strcpy(errmsg, CU_ARMSGN);
		break;
	    case U_ASMSGN :
		(void)strcpy(errmsg, CU_ASMSGN);
		break;
	    case U_ATMSGN :
		(void)strcpy(errmsg, CU_ATMSGN);
		break;
	    case U_AUMSGN :
		(void)strcpy(errmsg, CU_AUMSGN);
		break;
	    case U_AVMSGN :
		(void)strcpy(errmsg, CU_AVMSGN);
		break;
	    case U_AWMSGN :
		(void)strcpy(errmsg, CU_AWMSGN);
		break;
	    case U_AXMSGN :
		(void)strcpy(errmsg, CU_AXMSGN);
		break;
	    case U_AYMSGN :
		(void)strcpy(errmsg, CU_AYMSGN);
		break;
	    case U_AZMSGN :
		(void)strcpy(errmsg, CU_AZMSGN);
		break;
	    case U_BAMSGN :
		(void)strcpy(errmsg, CU_BAMSGN);
		break;
	    case U_BBMSGN :
		(void)strcpy(errmsg, CU_BBMSGN);
		break;
	    case U_BCMSGN :
		(void)strcpy(errmsg, CU_BCMSGN);
		break;
	    case U_BDMSGN :
		(void)strcpy(errmsg, CU_BDMSGN);
		break;
	    case U_BEMSGN :
		(void)strcpy(errmsg, CU_BEMSGN);
		break;
	    case U_BFMSGN :
		(void)strcpy(errmsg, CU_BFMSGN);
		break;
	    case U_BGMSGN :
		(void)strcpy(errmsg, CU_BGMSGN);
		break;
	    case U_ADDING :
		(void)strcpy(errmsg, CU_ADDING);
		break;
	    case U_UPDATING :
		(void)strcpy(errmsg, CU_UPDATING);
		break;
	    case U_WRERROR :
		(void)strcpy(errmsg, CU_WRERROR);
		break;
	    case U_WRFORCE :
		(void)strcpy(errmsg, CU_WRFORCE);
		break;
	    default :
		break;
	};

	wrc =  kudisply(udcb,line,col,errmsg,data_length);
	fflush(stdout);
	return;
}

#else
int kumsg( udcb , line , col , msgid )

UDCB    *udcb;          /* pointer to UDCB.                             */
short   line;           /* start line.                                  */
short   col;            /* start column.                                */
short   msgid;          /* message ID.                                  */

{
	int     kudisply();
	UECB    *uecb;  /* pointer to UECB.                             */
	char    blkmsg[256];
	int     wrc;
	short   data_length = 54;

	/* display error message.                                       */
	memset(blkmsg,0x20,(int)(data_length));
	uecb = udcb->erptr;
	while(TRUE)  {
	  if( uecb->id == U_ENDID ) {
	    wrc =  kudisply(udcb,line,col,blkmsg,data_length);
	    break;
	  } else if( uecb->id == msgid )  {
	    wrc =  kudisply(udcb,line,col,(char *)(uecb->msg),data_length);
	    break;
	  }
	  uecb++;
	};
	fflush(stdout);
	return;
};
#endif
