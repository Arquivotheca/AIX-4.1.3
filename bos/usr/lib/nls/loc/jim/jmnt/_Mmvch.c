static char sccsid[] = "@(#)30	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mmvch.c, libKJI, bos411, 9428A410j 7/23/92 03:23:58";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Mmvch
 *
 * DESCRIPTIVE NAME:    MoVe CHaracter in a dimemsion.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            Character movement in a dimemsion, then clear
 *                      remaining area.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        896 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mmvch()
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mmvch( str, pos, len, dir, dist,
 *                              clear, clrstr, clrpos, clrlen)
 *
 *  INPUT:              str     :Pointer to character dimension.
 *                      pos     :Start position.
 *                      len     :Character length you want move.
 *                      dir     :Move Direction. M_FORWD or M_BACKWD
 *                                              (forward or backward)
 *                      dist    :Move Distance.
 *                      clear   :Crear flag (You want clear remaining data)
 *                               specify TRUE or FALSE.
 *                      clrstr  :Pointer to string contain clear data.
 *                      clrpos  :Clear data start position in clrstr.
 *                      clrlen  :Clear data length in clrstr(not in str).
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      IMFAIL :Failure of Execution.(Invalid input data)
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy()        :Copy n character from memory
 *                                               areaA to areaB
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
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
 *      include Standard.
 */

#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */

#include "kj.h"         /* Kanji Project Define File.                   */
#include "kcb.h"        /* Kanji Control Block Structure.               */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */

int _Mmvch(str, pos, len, dir, dist, clear, clrstr, clrpos, clrlen)
char *str;              /* Pointer to character dimension.              */
int   pos;              /* Start position.                              */
int   len;              /* Character length you want move.              */
int   dir;              /* Move Direction. M_FORWD or M_BACKWD          */
int   dist;             /* Move Distance.                               */
int   clear;            /* Crear flag (You want clear remaining data)   */
                        /* specify TRUE or FALSE.                       */
char *clrstr;           /* Pointer to string contain clear data.        */
int   clrpos;           /* Clear data start position in clrstr.         */
int   clrlen;           /* Clear data length in clrstr(not in str).     */
{
        register char   *org;           /* Oreginal character address.  */
        register char   *dest;          /* Destination address.         */
        char            *stop;          /* Stop address.                */
        char            *clrstart;      /* clear data start address.    */
        int             times;          /* Clear loop counter.          */
        int             rem;            /* remainder of Clear loop.     */
        char            *memcpy();      /* memory copy function         */

        /* 1.
         *      Check input parameter.
         */

        if ((pos < 0) ||
            (len <= 0) ||
            ((clear == TRUE) && ((clrpos < 0) || (clrlen <= 0))) ||
            ((dir == M_FORWD) && (pos < dist))                     ) {

                return(IMFAIL);
        }

        /* 2.
         *      Branch by parameter -dir-.
         */

        if (dir == M_FORWD) {   /* Move foreawd */
                org  = str + pos;   /* Calculate sorce start address        */
                stop = org + len;   /* Calculate move stop address          */
                dest = org - dist;  /* Calculate destination start address  */
                while(org <= stop) {
                        *dest++ = *org++; /* Copying data */
                }
                if (clear) { /* if clear == TRUE */
                        times    = dist / clrlen;
                                            /* memcpy() Execution times     */
                        rem      = dist % clrlen;
                                            /* remainder after memcpy loop  */
                        dest     = str + pos + len - dist;
                                            /* destination address.         */
                        clrstart = clrstr + clrpos;
                                            /* Calculate sorce start address*/
                        while (times > 0) {
                                memcpy(dest, clrstart, clrlen);
                                            /* copying data                 */
                                dest += clrlen;
                                            /* New destination address      */
                                times--;
                                            /* Loop counter decrement       */
                        }
                        memcpy(dest, clrstart, rem);
                                            /* fill in remaining area       */
                }
        }
        else {                  /* Move backward */
                org  = str + pos + len - 1;
                                    /* Calculate sorce start address        */
                stop = str + pos;
                                    /* Calculate move stop address          */
                dest = org + dist;
                                    /* Calculate destination start address  */
                while(org >= stop) {
                        *dest-- = *org--;  /* copying data */
                }
                if (clear) {
                        times    = dist / clrlen;
                                            /* memcpy() Execution times     */
                        rem      = dist % clrlen;
                                            /* remainder after memcpy loop  */
                        dest     = str + pos;
                                            /* destination address.         */
                        clrstart = clrstr + clrpos;
                                            /* calculate sorce start address*/
                        while (times > 0) {
                                memcpy(dest, clrstart, clrlen);
                                                /* fill in specified data   */
                                dest += clrlen; /* New destination address  */
                                times--;
                        }
                        memcpy(dest, clrstart, rem); /* fill remaining area */
                }
        }

        return(IMSUCC);
}

