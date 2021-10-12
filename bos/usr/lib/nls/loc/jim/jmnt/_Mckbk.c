static char sccsid[] = "@(#)89	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mckbk.c, libKJI, bos411, 9428A410j 7/23/92 03:21:00";
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
 * MODULE NAME:         _Mckbk
 *
 * DESCRIPTIVE NAME:    Check a phrase
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
 * FUNCTION:            1.Check Kanji and conversion map in KMISA.
 *                      2.Return start position and end position of
 *                        phrase corresponds to next cursor position.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        892 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mckbk
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mckbk(pt,nxtcol,&ist,&ied,&iancf)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      nxtcol  :next cursor position
 *
 *  OUTPUT:             ist     :phrase start position.
 *                      ied     :phrase end position.
 *                      iancf   :attribute no change flag.
 *
 * EXIT-NORMAL:         IMSUCC :Success of Execution.
 *
 * EXIT-ERROR:          KMIVCPE  : Invalid conversion position.
 *                      KMIVKCME : Invalid code exists in kjcvmap.
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
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              *kjsvpt  : pointer to KMISA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convpos  : conversion position.
 *                              *kjcvmap : Kanji and conversion map.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
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
#include <memory.h>     /* System memory operation utylitye             */

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
 *  This module does,
 *      1.Check Kanji and conversion map in KMISA.
 *      2.Return start position and end position of
 *        phrase corresponds to next cursor position.
 */

int _Mckbk(pt,nxtcol,ist,ied,iancf)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
short   nxtcol;       /*  next cursor position                  */
short   *ist;         /*  phrase start position                 */
short   *ied;         /*  phrase end position                   */
short   *iancf;       /*  attribute no change flag              */
{
        KMISA   *kjsvpt;   /*   pointer to kanji moniter ISA           */
        int     rc ;       /*   return code                            */
        int     i       ;  /*   loop counter                           */
        short   ip ;       /*   Scanning position                      */
        uchar   ckcode ;   /*   Kanji map on scanning start position   */
        uchar   ckmap ;    /*   Kanji map on current scanning position */
        short   iwk1 ;     /*   Current scanning position(leftward)    */
        short   iwk2 ;     /*   Current scanning position(rightward)   */

/* ### */
        CPRINT(======== start _Mckbk ===========);
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mckbk , "start _Mckbk" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */

        /* 1.
         *  Check next cursor position.
         *  Get Kanji map on next cursor position.
         *  Set check code.
         */

        /* 1.1.
         *  set  !  return code.
         */
         rc = IMSUCC;

        /* 1.2.
         *  Check next cursor position
         */

         if(nxtcol < kjsvpt->convpos)  {
                rc = KMIVCPE;   /*  Set error return code  */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mckbk , "end ---No.1 ---- _Mckbk " );

                return(rc);
         };

        /* 1.3.
         *  set  !  scanning start position on Kanji and conversion map
         *          ckcode and get Kanji map.
         */
            /*  Set scanning start position
                in Kanji and conversion map      */
         ip = nxtcol - kjsvpt->convpos;
            /*  Get Kanji map      */
         ckcode = kjsvpt->kjcvmap[ip];

        /* 2.
         *  set  !  initial value of phrase start position(ist), phrase end
         *          position(ied).
         *  ist , ied and iancf are output parameters.
         */
                  /* Initialize output parameter  */

         *ist   = nxtcol;         /*  phrase start position     */
         *ied   = nxtcol + C_ANK; /*  phrase end position       */
         *iancf = C_SWOFF;        /*  attribute no change flag  */

        /* 3.
         *  Get  !  start and end position of a phrase by scanning.
         */
         switch(ckcode)  {
                case M_KJMCTN : /*  Constant for kjmap code.(Continuation) */
                case M_KJMJAN : /*  Constant for kjmap code.
                                         (Adjunct without homonym.) */

                         /*  position left of scanning start position  */
                   iwk1 = ip - C_DBCS;

                         /*  position right of scanning start position  */
                   iwk2 = ip + C_DBCS;

                   i = 0;    /*  initialize loop counter */

                   for( ; ; )  {               /* Scan leftward  */

                         /*
                          *  Check current scanning position is head of
                          *  Kanji and conversion map or not.
                          */
                        if(iwk1 <= 0)  {
                                     /* Set phrase start position  */
                                *ist = nxtcol - C_DBCS * (i + 1);
                                break;

                        }
                        else  {

                               /*
                                * Get Kanji map on current scanning position
                                */
                                ckmap = kjsvpt->kjcvmap[iwk1];

                           /*
                            *  Check Kanji map code is not
                            *  Continuation nor Adjunct without homonym.
                            */
                                if(ckmap != M_KJMCTN
                                  && ckmap != M_KJMJAN)  {

                                            /* Set phrase start position  */
                                        *ist = nxtcol - C_DBCS * (i + 1);
                                        break;

                                }
                                else  {

                         /*  Shift current scanning position leftward  */
                                        iwk1 = iwk1 - C_DBCS;

                                           /*  increase loop counter */
                                        i = i + 1;

                                };

                        };

                   };        /*  end of scanning  */
                   i = 0;    /*  Initialize loop counter */
                   for( ; ; )  {    /*  Scan rightward   */

                   /*
                    *  current scanning position is end of Kanji
                    *  and conversion map or not
                    */
                        if(kjsvpt->convlen <= iwk2)  {

                                            /* Set phrase end position  */
                                *ied = nxtcol + C_DBCS * i + C_ANK;
                                break;
                        }
                        else  {

                               /*
                                * Get Kanji map on current scanning position
                                */
                                ckmap = kjsvpt->kjcvmap[iwk2];

                           /*
                            *  Check Kanji map code is not
                            *  Continuation nor Adjunct without homonym.
                            */
                                if(ckmap != M_KJMCTN
                                  && ckmap != M_KJMJAN)  {

                                            /* Set phrase end position  */
                                        *ied = nxtcol + C_DBCS * i + C_ANK;
                                        break;

                                }
                                else  {

                         /*  Shift current scanning position rightward  */
                                        iwk2 = iwk2 + C_DBCS;

                                           /*  increase loop counter */
                                        i = i + 1;

                                };

                        };

                   };        /*  end of scanning  */
                   break;

                case M_KJMNCV : /*  Constant for kjmap code.(Yomi)  */

                           /* Check conversion status  */
                        switch(kjsvpt->kjcvmap[ip+1])  {

                        /*  Conversion status is "not converted" */
                                case M_KSNCNV :

                         /*  Shift scanning start position  */
                                   ip = ip - C_DBCS + 1;

                                   i = 0;   /*  Initialize loop counter */

                                   for( ; ; )  {  /* Scan leftward  */

                         /*
                          *  Check current scanning position is head of
                          *  Kanji and conversion map or not.
                          */
                                        if(ip <= 0)  {

                                     /* Set phrase start position  */
                                                *ist = nxtcol
                                                    - C_DBCS * i;
                                                break;

                                        }
                                        else  {

                                      /*
                                       * Get conversion status on current
                                       * scanning position
                                       */
                                                ckmap = kjsvpt->kjcvmap[ip];

                                      /*
                                       *  Check conversion status is not
                                       *  "Not converted".
                                       */
                                                if(ckmap != M_KSNCNV)  {

                                            /* Set phrase start position  */
                                                        *ist = nxtcol
                                                            - C_DBCS * i;
                                                        break;

                                                }
                                                else  {

                         /*  Shift current scanning position leftward  */
                                                        ip = ip - C_DBCS;

                                           /*  increase loop counter */
                                                        i = i + 1;

                                                };

                                        };

                                   };      /*  end of scanning  */

                                            /* Set phrase end position  */
                                   *ied = nxtcol + C_ANK;

                                            /* Set phrase start position  */
                                   if(*ist < 0)   *ist = 0;
                                   break;

                            /* Conversion status is "Not to be converted" */
                                case M_KSCNUM :

                                     /*  Set attribute no change flag  */
                                        *iancf = C_SWON;
                                        break;

                                default :
                                                /* Set error return code  */
                                        rc = KMIVKCME;

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mckbk , "end ---No.2 ---- _Mckbk " );

                                        return(rc);
                        };   /*  end switch(kjsvpt->kjcvmap[ip+1])  */
                        break;

                default :

                         /*  Shift scanning start position rightward  */
                   ip = ip + C_DBCS;

                   i = 0;    /*  Initialize loop counter  */

                   for( ; ; )  {    /* Scan rightward  */

                   /*
                    *  current scanning position is end of Kanji
                    *  and conversion map or not
                    */
                        if(kjsvpt->convlen <= ip)  {

                                            /* Set phrase end position  */
                                *ied = nxtcol + C_DBCS * i + C_ANK;
                                break;

                        }
                        else  {

                         /*  Shift current scanning position leftward  */
                                ckmap = kjsvpt->kjcvmap[ip];

                           /*
                            *  Check Kanji map code is not
                            *  Continuation nor Adjunct without homonym.
                            */
                                if(ckmap !=M_KJMCTN && ckmap != M_KJMJAN)  {

                                            /* Set phrase end position  */
                                        *ied = nxtcol + C_DBCS * i + C_ANK;
                                        break;

                                }
                                else  {

                         /*  Shift current scanning position rightward */
                                        ip = ip + C_DBCS;

                                           /*  increase loop counter  */
                                        i = i + 1;

                                };

                        };

                   };     /*  end of scanning  */

                     /* Set phrase start position  */
                   *ist = nxtcol;

         };    /*  end switch(ckcode)  */

        /* 4.
         *  Return Value
         */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_Mckbk , "end ---No.3 ---- _Mckbk " );

         return(rc);
}
