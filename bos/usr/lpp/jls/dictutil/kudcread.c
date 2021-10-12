static char sccsid[] = "@(#)14	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudcread.c, cmdKJI, bos411, 9428A410j 7/23/92 00:59:37";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudcread
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
 * MODULE NAME:         kudcread
 *
 * DESCRIPTIVE NAME:    User Dictionary read
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
 * FUNCTION:            User Dictionary read Function
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        496 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudcread
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudcread ( cbptr, id, rrn )
 *
 *  INPUT:              cbptr   :Pointer to User Dictonary Control Block
 *                      id      :Process Identify
 *                      rrn     :Relative Record No
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
void    kudcread ( cbptr, id, rrn )

UDCB          *cbptr;     /* Pointer to control block buffer                 */
short          id;        /* Dictionary Identify                             */
short          rrn;       /* Read Relative Record Number                     */

{

  switch (id)
    {
                          /* System Dictionary Index    ( Identify = 1 )     */
       case  1:  cbptr->rdptr = cbptr->dcptr + U_SHSPOS + U_SDVERL;
                 break;

                          /* System Dictionary Data     ( Identify = 2 )     */
       case  2:  cbptr->rdptr = cbptr->dcptr + U_SDBKLN + ( rrn * U_SDBKLN );
                 break;

                          /* User Dictionary Index      ( Identify = 3 )     */
       case  3:  cbptr->rdptr = cbptr->dcptr + U_MRU_A;
                 break;

                          /* User Dictionary Data       ( Identify = 4 )     */
       case  4:  cbptr->rdptr = cbptr->dcptr + ( rrn * U_REC_L );
                 break;
    }

  return;
   
}
