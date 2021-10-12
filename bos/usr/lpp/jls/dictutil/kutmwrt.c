static char sccsid[] = "@(#)59	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kutmwrt.c, cmdKJI, bos411, 9428A410j 7/23/92 01:31:05";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kutmwrt
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 * MODULE NAME:         kutmwrt
 *
 * DESCRIPTIVE NAME:    write temporaly file
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
 *  MODULE SIZE:        412 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kutmwrt
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kutmwrt( flname, dcptr, fsize )
 *
 *  INPUT:              flname          : pointer to file name
 *                      dcptr           : pointer UDCB
 *                      fsize           : size of usr dictionary
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
 *                              NA.
 *                      Standard Liblary.
 *                              creat
 *                              write
 *                              close
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

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>          /* File Control Package.                	*/
#include <unistd.h>
#include "kut.h"            /* Utility Define File                      */

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

#define  STSLEN   (  1   )  /* file status   data lenght                */

int kutmwrt( flfd, dcptr, fsize )
int     flfd;     /* user ductionary file discripter        		*/
uchar   *dcptr;   /* pointer to memory of user dictionary 		*/
long    fsize;    /* user Dictionary file size    			*/

{
  int           de_ret; 	/* return value 			*/
  int           st_ret; 	/* return value 			*/
  int           i;
  uchar         faldata = 0xf0;
  uchar         rundata = 0xf1;
  struct        flock flck;     /* flock structure for fcntl()          */

  /*--------------------------------------------------------------------*
   * Lock user dictionary file
   *--------------------------------------------------------------------*/
  flck.l_type = F_WRLCK;
  flck.l_whence = flck.l_start = flck.l_len = 0;
  for ( i=0; i<U_TRYLOK; i++) {
     if ( (de_ret = fcntl( flfd, F_SETLK, &flck )) != -1 )
             break;
  }
  if ( de_ret == -1 ) {
    return( -1 );	/* lock error		*/
  }

  /*--------------------------------------------------------------------*
   * Write to status data on file to 0xf0
   *--------------------------------------------------------------------*/
  rundata = *( dcptr + U_MRU_A+U_ILLEN );
  lseek( flfd, (U_MRU_A+U_ILLEN), 0 );
  de_ret = write( flfd, &faldata, STSLEN );
  if( de_ret < STSLEN ) {
       return( -2 );
  }

  /*--------------------------------------------------------------------*
   * Write to status data on memory to 0xf0
   *--------------------------------------------------------------------*/
  *( dcptr + U_MRU_A + U_ILLEN ) = faldata;

  /*--------------------------------------------------------------------*
   * Write user dictionary image
   *--------------------------------------------------------------------*/
  lseek( flfd, 0, 0 );
  de_ret = write( flfd, dcptr, (unsigned)fsize );
  if ( de_ret != fsize ) {
    return( -3 );	/* write error		*/
  }

  /*--------------------------------------------------------------------*
   * Write previous status data to memory
   *--------------------------------------------------------------------*/
  *( dcptr + U_MRU_A+U_ILLEN ) = rundata;

  /*--------------------------------------------------------------------*
   * Write previous status data to file
   *--------------------------------------------------------------------*/
  lseek( flfd, (U_MRU_A+U_ILLEN), 0 );
  de_ret = write( flfd, &rundata, STSLEN );
  if( de_ret < STSLEN ) {
       return( -4 );
  }

  /*--------------------------------------------------------------------*
   * Unlock temporaly file
   *--------------------------------------------------------------------*/
  flck.l_type = F_UNLCK;
  for ( i=0; i<U_TRYLOK; i++ ) {
     if ( (de_ret = fcntl( flfd, F_SETLK, &flck )) != -1 )
             break;
  }
  if ( de_ret == -1 ) {
    return( -5 );	/* unlock error		*/
  }

  /*--------------------------------------------------------------------*
   * Success return
   *--------------------------------------------------------------------*/
  return( IUSUCC );

}

