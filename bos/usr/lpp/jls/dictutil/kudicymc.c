static char sccsid[] = "@(#)37	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicymc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:25:25";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicymc
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
 * MODULE NAME:         kudicymc
 *
 * DESCRIPTIVE NAME:    Conversion Yomi data. (PC code -> 7bit code)
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
 * FUNCTION:            Convert to 7bit code data from PC code data .
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2392 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicymc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicymc(yomidata,yomilen,kanadata,kanalen,mflag)
 *
 *  INPUT:              kanadata:Pointer to yomidata(PC code) string.
 *                      kanalen :Length of yomidata.
 *
 *  OUTPUT:             moradata:Pointer to kanadata(7bit) string.
 *                      moralen :Length of kanadata.
 *                      mflag   :convert flag.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
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
#include "kudicymc.t"   /* conversion table                             */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void    kudicymc( yomidata,yomilen,kanadata,kanalen,mflag )

uchar   *yomidata;      /* PC code string area.                        */
short   yomilen;        /* PC code string lenght.                      */
uchar   *kanadata;      /* MKK 7bit code string area.                  */
short   *kanalen;       /* kanadata string lenght.                     */
short   *mflag;         /* PC code character type flag.                */
{

        int     i,j;            /* Loop Counter.                        */
        int     pc_ch;          /* a PC code character area.            */
        uchar   pc_hit;         /* a MKK code area.                     */
        uchar   chmode;         /* 7bit code conversion mode.           */
        int     tbl_idx;        /* MKK code conversion table pointer.   */
        char    asw;            /* ALPHABET switch flag                 */
        char    hsw;            /* HIRAGANA switch flag                 */
        char    dsw;            /* Default switch flag                  */
        char    ksw;            /* KATAKANA switch flag                 */
        char    assw;           /* Caps ALPHABET switch flag            */

        /*  1.
         *      Initialize the parameter area.
         */

        asw  = C_SWOFF;  /*  Initialize Alphabet flag         */
        hsw  = C_SWOFF;  /*  Initialize Hiragana flag         */
        dsw  = C_SWOFF;  /*  Initialize default flag          */
        ksw  = C_SWOFF;  /*  Initialize Katakana flag         */
        assw = C_SWOFF;  /*  Initialize Caps Alphabet flag    */

        j = yomilen;   /* DBCS character count set on j.      */

        for ( i = 0, *kanalen = 0; i < j; i++ )  {

        /*  2.1.
         *      Set PC code from input parameter(yomidata)
         *      to local variable(pc_ch).
         */
                pc_ch  = yomidata[0] * C_HIBYTE + yomidata[1];

                     /*  Add pointer to yomidata of input parameter */
                yomidata = yomidata + C_DBCS;

                     /*  Subtract yomilen of input parameter        */
                yomilen = yomilen - C_DBCS;

                     /*  Mask code for lower one byte         */
                pc_hit = C_BYTEM;

                     /*  Set invalid value to local variable  */
                tbl_idx = -1;

        /*  2.2.
         *      Get akind of PC Kanji code.
         */

        /*  Check Hiragana.                                        */
                if ( (pc_ch == 0x815b) || ((pc_ch >= 0x829f)
                    && (pc_ch <=0x82f1)) )  {

                        chmode = U_CHHIRA;   /*  Set Hiragana mode  */

                }

        /*  Check Katakana.                                        */
                else if ( (pc_ch >= 0x8340) && (pc_ch <= 0x8396) )  {

                        chmode = U_CHKATA;   /*  Set Katakana mode   */
                        ksw    = C_SWON;     /*  Set Katakana switch */

                }

        /*  Check Alphabet.                                        */
                else if ( (pc_ch == 0xfa56) || (pc_ch == 0xfa57) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }
                else if ( (pc_ch >= 0x8146) && (pc_ch <= 0x8149) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch == 0x815e) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch == 0x8169) || (pc_ch == 0x816a) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch == 0x817b) || (pc_ch == 0x817c) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch == 0x8181) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch == 0x8183) || (pc_ch == 0x8184) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch >= 0x8193) && (pc_ch <= 0x8197) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch >= 0x8260) && (pc_ch <= 0x8279) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode   */

                }

                else if ( (pc_ch >= 0x8281) && (pc_ch <= 0x829a) )  {

                        chmode = U_CHALPH;   /*  Set Alphabet mode    */
                        assw   = C_SWON;     /*  Set caps switch flag */

                }

        /* Check Numbre.                                                */
                else if ( (pc_ch >= 0x824f) && (pc_ch <= 0x8258) )  {

                        chmode = U_CHNUM;    /*  Set number mode     */

                }

        /* Check Blank.                                                 */
                else if (pc_ch == 0x8140)  {

                        chmode = U_CHBLNK;   /*  Set Blank mode      */

                }

        /* Another One.                                                 */
                else  {

                        chmode = U_CHKIGO;   /*  Set default mode    */

                };

        /*  2.3.
         *      Set the switch flag as the case may by.
         */

                          /*  Set the HIRAGANA switch flag  */
                if( (chmode == U_CHHIRA) || (chmode == U_CHKATA) )  {

                        hsw = C_SWON;    /*  Set Hiragana switch flag  */

                };

                          /*  Set the ALPHABET switch flag  */
                if( (chmode == U_CHALPH) || (chmode == U_CHNUM) )   {

                        asw = C_SWON;    /*  Set Alphabet switch flag  */

                };

                          /*  Set the BLANK switch flag  */
                if( (chmode == U_CHBLNK) || (chmode == U_CHKIGO) )  {

                        dsw = C_SWON;    /*  Set default switch flag  */

                };

        /* 3.1.1.1 Get the MKK table pointer. (1)
         *         (hiragana mode,katakana mode.)
         */
                     /*  Check chmode of local variable  */
                if ( (chmode == U_CHHIRA) || (chmode == U_CHKATA) )  {

                                  /*  Check PC code character area  */
                        if ( pc_ch < C_SPACE )   {

                                  /*  Set invalid value to local variable */
                                tbl_idx = -1;

                        };

                                  /*  Check PC code character area  */
                        if ( (C_SPACE <= pc_ch) && (pc_ch <= 0x83fc) ) {

                          /*  Set conversion table to local variable  */
                                tbl_idx  = (pc_ch / C_HIBYTE) - C_SPACEH;

                          /*  Set the conversion table to MKK code area  */
                                pc_hit =
                                kana_tbl[tbl_idx][ (pc_ch & C_BYTEM)
                                                            - C_SPACEL ];

                        };

                                  /*  Check PC code character area  */
                        if ( 0x83fc < pc_ch )  {

                                  /*  Set invalid value to local variable */
                                tbl_idx = -1;

                        };

                       /*  Check conversion table to local variable */
                        if ( tbl_idx == -1 )  {

                           /* local variable mask code for lower one byte */
                                pc_hit = C_BYTEM;

                        };

                };

        /* 3.1.1.2 Get the MKK table pointer. (2)
         *         (alphabet mode and number mode .)
         */
                     /*  Check chmode of local variable  */
                if ( ( chmode == U_CHALPH ) || (chmode == U_CHNUM ) ) {

                                  /*  Check PC code character area  */
                        if ( pc_ch < C_SPACE )  {

                                  /*  Set invalid value to local variable */
                                tbl_idx = -1;

                        };

                                  /*  Check PC code character area  */
                        if ( (C_SPACE <= pc_ch) && (pc_ch <= 0x82fc) ) {

                          /*  Set conversion table to local variable  */
                                tbl_idx = (pc_ch / C_HIBYTE) - C_SPACEH;

                          /*  Set the conversion table to MKK code area  */
                                pc_hit =
                                alph_tbl[tbl_idx][ (pc_ch & C_BYTEM)
                                                            - C_SPACEL ];

                        };

                                  /*  Check PC code character area  */
                        if ( 0x82fc < pc_ch )  {

                                  /*  Set invalid value to local variable */
                                tbl_idx = -1;

                        };

                       /*  Check conversion table to local variable */
                        if ( tbl_idx == -1 )  {

                           /* local variable mask code for lower one byte */
                                pc_hit = C_BYTEM;

                                        /*  Check PC code character area  */
                                if ( pc_ch == C_LIT )

                                        /*  Set PC code character area  */
                                    pc_hit = 0x27;

                                        /*  Check PC code character area  */
                                if ( pc_ch == C_QUOTO )

                                        /*  Set PC code character area  */
                                    pc_hit = 0x22;

                        };

                };

        /*   4.
         *      Put the MKK code.
         */
                         /*  Check MKK code area  */
                if ( pc_hit == C_BYTEM )  {

                        break;

                };

                       /*  Set MKK 7bit code string area  */
                kanadata[ *kanalen ] = pc_hit;

                       /*  Add lenght of kanadata string  */
                *kanalen = *kanalen + C_ADD;

        /*   5.
         *      Check yomile out of value.
         */
                if ( yomilen < C_DBCS )  {

                        break;

                };

        };   /*   end for loop   */

        /*   6.
         *      Set the mflag as the case may by.
         */
                  /*  Hiragana mode switch is ON  */
        if( (hsw == C_SWON) && (asw == C_SWOFF) && (dsw == C_SWOFF) )  {

            if(ksw == C_SWOFF) {   /*  katakana switch flag is OFF  */

                   *mflag = U_HIRA;  /* Set mflag of output parameter  */

            }
            else {                 /*  katakana switch flag is ON   */

                   *mflag = U_KATA;  /* Set mflag of output parameter  */

            };

        };

                        /*  Alphabet mode switch is ON  */
        if( (hsw == C_SWOFF) && (asw == C_SWON) && (dsw == C_SWOFF) )  {

                            /*  Shift 7bit code string area  */
                for(i = *kanalen ; i >= 0 ; i--)  {

                        *(kanadata + i + 1) = *(kanadata + i);

                };

                        /*  Insert '1D' to top of kanadata  */
                *kanadata       = HESC;

                        /*  Add lenght of kanadata  */
                *kanalen = *kanalen + 1;

                         /*  Upper Case Alphanumeric Mode  */
            if(assw == C_SWOFF)  {

                   *mflag = U_CAPON;   /* Set mflag of output parameter  */

                         /*  Lower Case Alphanumeric Mode  */
            }  else  {

                   *mflag = U_CAPOFF;  /* Set mflag of output parameter  */

            };

        };

                        /*  Alphabet mode and Hiragana mode switch is ON  */
        if( (hsw == C_SWON) && (asw == C_SWON) && (dsw == C_SWOFF) )  {

                *mflag = U_HEMIX;      /* Set mflag of output parameter  */

        };

                        /*  Default mode switch is ON  */
        if(dsw == C_SWON)  {

                *mflag = U_INVALD;     /* Set mflag of output parameter  */

        };

        /*   7.
         *      Return.
         */
        return;
}

