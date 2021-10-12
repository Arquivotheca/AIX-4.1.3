static char sccsid[] = "@(#)15	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudcwrit.c, cmdKJI, bos411, 9428A410j 7/23/92 01:00:00";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudcwrit
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudcwrit
 *
 * DESCRIPTIVE NAME:    User Dictionary write
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            User Dictionary write Function
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        612 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudcwrit
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudcwrit ( cbptr, id, rrn )
 *
 *  INPUT:              cbptr   :Pointer to User Dictonary Control Block
 *                      id      :Process Identify
 *                      rrn     :relative record no
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              NA.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Kanji Project.
 */
#include "kut.h"                         /* Kanji Utility Define File.  */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void    kudcwrit ( cbptr, id, rrn )

UDCB          *cbptr;     /* Pointer to Control Block Buffer                 */
short          id;        /* Dictionary Identify                             */
short          rrn;       /* Read Relative Record Number                     */

{
  int   i;                /* Work Counter 1                                  */
  int   j;                /* Work Counter 2                                  */
  uchar har;              /* User Dictionary Active's Data Record No Area    */

  switch ((int)id)
    {
                          /* System Dictionary Index   ( Identify = 1 )      */
       case  1:  for ( i=0; i<U_REC_L; i++)
                   *(cbptr->dcptr + U_SHSPOS + U_SDVERL + i) 
                                                  = *(cbptr->wtptr + i);
                 break;

                          /* System Dictionary Data    ( Identify = 2 )      */
       case  2:  for ( i=0; i<U_SDBKLN; i++)
                   *(cbptr->dcptr + (rrn * U_SDBKLN) + i)
                                                  = *(cbptr->wtptr + i);
                 break;

                          /* User Dictionary Index     ( Identify = 3 )      */
       case  3:  j = U_REC_L;                            /* Set 1K byte      */
                                              /* Get User dictionary size    */
                 har = *(cbptr->dcptr + U_MRU_A + U_ILLEN + U_STSLEN);
                 if ( har > U_HAR_V3 )  j += U_REC_L;    /* Set 2K byte      */
                 if ( har > U_HAR_V5 )  j += U_REC_L;    /* Set 3K byte      */
                 for ( i=0; i<j; i++ )
                   *(cbptr->dcptr + U_MRU_A + i) = *(cbptr->wtptr + i);
                 break;

                          /* User Dictionary Data      ( Identify = 4 )      */
       case  4:  for ( i=0; i<U_REC_L; i++)
                   *(cbptr->dcptr + (rrn * U_REC_L) + i) = *(cbptr->wtptr + i);
                 break;

    }

  return;
   
}
