static char sccsid[] = "@(#)27	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudiceng.c, cmdKJI, bos411, 9428A410j 7/23/92 01:10:21";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudiceng
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
 * MODULE NAME:         kudiceng
 *
 * DESCRIPTIVE NAME:    System Dictionary Entry Position or Length Search.
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
 * FUNCTION:            Get position and length of system dictionary data
 *                      by moradata.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1236 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudiceng
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudiceng( moralen, moradata, dicblkpt, enren, enpos )
 *
 *  INPUT:              moralen :Moradata Length.
 *                      moradata:Moradata
 *                      dicblkpt:Pointer to System Dictionary Data Block.
 *
 *  OUTPUT:             enren   :System Dictionary Entry Length.
 *                      enpos   :System Dictionary Entry Position.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
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
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */

/*
 *      include Kanji Project.
 */
#include "kut.h"        /* Kanji Utility Define File.                   */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void kudiceng( moralen, moradata, dicblkpt, enlen, enpos )

short   moralen;        /* Mora Length                                  */
uchar   *moradata;      /* Mora Data                                    */
uchar   *dicblkpt;      /* System Dictionary Pointer                    */
short   *enlen;         /* Entry Length                                 */
short   *enpos;         /* Entry Position                               */
{
        int     entry_ps;       /* Work Entry Position                  */
        ushort  offset;         /* Offset                               */
        short   data_ln;        /* Next Position Length                 */
        uchar   wk_yomi;        /* Work Yomi                            */
        char    mora_flg;       /* Last Yomi Flag                       */

        int     i;              /* Loop Counter                         */
        int     j;              /* Loop Counter                         */

        /*
         *      Input Parameter Initialize      ( Length, Position )
         */

        *enlen = 0;             /* Entry Length Initialize              */

        *enpos = 0;             /* Entry Position Initialize            */

        /*
         *      System Dictionary Ver. Skip     ( Position )
         */

        entry_ps = 2;

        /*
         * Entry Position and Length Search
         */

        /*
         * 1 Mora Word Check
         */

        for ( ;; )
          {
            if ( dicblkpt[entry_ps] > moradata[0] )
                 break;

            if ( dicblkpt[entry_ps] == moradata[0] )
               {
                /*
                 * Entry Length or Next Mora Position Set
                 */

                entry_ps += 4;

                /*
                 * Entry Position or Next Mora Position Length Set
                 */

                if ( (dicblkpt[entry_ps-1] & C_SB80) == C_SB80 )
                   {
                    /*
                     * Length Conversion Set (MSB ON)
                     */

                    (void)memcpy( (int *)&offset, &dicblkpt[entry_ps-1], 2 );

                    offset -= 0x8000;

                    entry_ps++;
                   }
                else
                   {
                    /*
                     * Length Set (MSB OFF)
                     */

                    offset = dicblkpt[entry_ps-1];
                   };

                if ( moralen == 1 )
                   {
                    /*
                     * Entry Length and Entry Postion Set
                     *       (Mora Code Only 1 Byte)
                     */

                    *enlen = offset;
                    *enpos = entry_ps;
                    return;
                   }
                else
                   {
                        /*
                         * Next Mora Position Set
                         */

                        entry_ps += offset;
                   };

                /*
                 * 2 Mora Word Check
                 */

                for ( ;; )
                  {
                   if ( dicblkpt[entry_ps] > moradata[1] )
                        return;

                    if ( dicblkpt[entry_ps] == moradata[1] )
                       {
                        /*
                         * Entry Length or Next Mora Position Set
                         */

                        entry_ps += 4;


                        if ( (dicblkpt[entry_ps-1] & C_SB80) == C_SB80 )
                           {
                            /*
                             * Length Conversion Set (MSB ON)
                             */

                            (void)memcpy( (int *)&offset,
                                           &dicblkpt[entry_ps-1], 2 );
                           }
                        else
                           {
                            /*
                             * Length Set (MSB OFF)
                             */

                            offset = dicblkpt[entry_ps-1];
                           };

                        if ( moralen == 2 )
                           {
                            /*
                             * Entry Length Set (Mora Code 2 Bytes)
                             */

                            if ( (offset & C_DB8000) == C_DB8000 )
                               {
                                offset -= 0x8000;
                                entry_ps++;
                               };

                            /*
                             * Entry Length and Entry Postion Set
                             *       (Mora Code Only 1 Byte)
                             */

                            *enlen = offset;
                            *enpos = entry_ps;
                            return;
                           }
                        else
                           {
                            /*
                             * 3 Mora Code Length
                             */

                            if ( dicblkpt[entry_ps-3] == 0xff &&
                                 dicblkpt[entry_ps-2] == 0xff )
                               {
                                if ( (offset & C_DB8000) == C_DB8000 )
                                   {
                                    offset -= 0x8000;
                                    entry_ps++;
                                   };

                                data_ln = 2048 - entry_ps - offset;
                               }
                            else
                               {
                                (void)memcpy( (int *)&data_ln,
                                               &dicblkpt[entry_ps-3], 2 );

                                if ( (offset & C_DB8000) == C_DB8000 )
                                   {
                                    offset -= 0x8000;
                                    entry_ps++;
                                   };

                                 data_ln -= offset;
                               };

                            /*
                             * Next Mora Position Set
                             */

                            entry_ps += offset;

                           };

                        /*
                         * 3 Mora Word Check
                         */

                        for ( i = 0; i < data_ln; i += offset + 1 )
                          {
                            /*
                             * Data Length Set
                             */

                            if ( (dicblkpt[entry_ps] & C_SB80) == C_SB80 )
                               {
                                /*
                                 * Length Conversion Set (MSB ON)
                                 */

                                (void)memcpy( (int *)&offset,
                                               &dicblkpt[entry_ps], 2 );

                                offset -= 0x8000;

                                entry_ps++;
                               }
                            else
                               {
                                /*
                                 * Length Set (MSB OFF)
                                 */

                                offset = dicblkpt[entry_ps];
                               };

                            /*
                             * 3 More Than Yomi
                             */

                            entry_ps++;

                            /*
                             * Last Yomi Flag
                             */

                            mora_flg = C_SWOFF;

                            for ( j = 0; j < moralen - 2; j++ )
                              {
                               /*
                                * Yomi MSB Check
                                */

                               wk_yomi = dicblkpt[entry_ps+j];

                               if ( (wk_yomi & C_SB80 ) == C_SB80 )
                                  {
                                   /*
                                    * Last Yomi Flag Set
                                    */

                                   mora_flg = C_SWON;

                                   /*
                                    * Last Yomi Conversion
                                    */

                                   wk_yomi -= 0x80;
                                  };

                                /*
                                 * Mora Word Check
                                 */

                                if ( wk_yomi == moradata[j+2] )
                                   {
                                    /*
                                     * Last Yomi Flag Check
                                     */

                                    if ( mora_flg == C_SWOFF )
                                         continue;

                                    /*
                                     * Mora Length Check
                                     */

                                    if ( moralen == j + 3 )
                                       {
                                        /*
                                         * Entry Length Set
                                         */

                                        *enlen = offset - j - 1;

                                        /*
                                         * Entry Position Set
                                         */

                                        *enpos = entry_ps + j + 1;
                                        return;
                                       };
                                   };
                                /*
                                 * Offset Position Add
                                 */

                                entry_ps += offset;

                                break;
                              };
                          };

                        /*
                         * No Search
                         */

                        return;

                       };

                    /*
                     * 2 PTR Check
                     */

                    if ( dicblkpt[entry_ps+1] == 0xff &&
                         dicblkpt[entry_ps+2] == 0xff )
                       {

                        /*
                         * No Search
                         */

                        return;
                       };

                    /*
                     * Offset Conversion (2 Mora Position)
                     */

                    (void)memcpy( (int *)&offset,
                                  &dicblkpt[entry_ps+1], 2 );

                    /*
                     * Offset Position Add
                     */

                    entry_ps += offset;
                  };
               };

            /*
             * 1 PTR Check
             */

            if ( dicblkpt[entry_ps+1] == 0xff &&
                 dicblkpt[entry_ps+2] == 0xff )
               {
                break;
               };

            /*
             * Offset Conversion (1Mora Position)
             */

            (void)memcpy( (int *)&offset, &dicblkpt[entry_ps+1], 2 );

            /*
             * Offset Position Add
             */

            entry_ps += offset;
          };

        /*
         *      Return Code.
         */

        return;
}
