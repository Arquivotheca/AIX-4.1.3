static char sccsid[] = "@(#)54	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_ME_0b.c, libKJI, bos411, 9428A410j 7/23/92 03:18:50";
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
 * MODULE NAME:         _ME_0b
 *
 * DESCRIPTIVE NAME:    Kanji No. input process.
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
 * FUNCTION:            Call numeric conversion routine.
 *                      Set converted data to KCB
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        996 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _ME_0b
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _ME_0b(pt)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      other   :return code from _Mecho().
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Msetch :set up convert position for display
 *                              _Mnumgt :Kanji monitor number get.
 *                              _MM_rtn :Mode conversion.
 *                      Standard Library.
 *                              memcpy  :
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
 *                              type    :input code type.
 *                              kjsvpt  :pointer to Kanji monitor save area.
 *                              curcol  :input field cursor colmn.
 *                              string  :pointer to input character string.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              chcodlen:changed character length.
 *                              chcode  :character code.
 *                              curright:limit of cursor position on right.
 *                              curleft :limit of cursor position on left.
 *                              knjnumfg:Kanji No. input area flag.
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              string  :pointer to input character string.
 *                              lastch  :input field last character
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              ax1lastc:Auxiliart area No.1 last character
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
#include <memory.h>     /* System memory operation uty.                 */

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
int  _ME_0b(pt)

KCB     *pt;            /* pointer KCB.                                 */

{
        int     _Mnumgt();      /* set up convert position for display. */
        int     _Msetch();      /* Mode conversion.                     */

        uchar   onum[C_DBCS];   /* double byte code.                    */
        KMISA   *kjsvpt;        /* pointer to KMISA.                    */
        int     ret_code;       /* return value.                        */
        uchar   *strptr;        /*                                      */
        int     brkflg;         /* work flag.                           */
        int     wi1;            /* work variavle integer.               */
        int     wi2;            /* work variavle integer.               */

        CPRINT(#### _ME_0b start ####);
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0b , "start    _ME_0b");

        ret_code = IMSUCC;      /* initialize return code.              */
        kjsvpt = pt->kjsvpt;    /* set pointer to KMISA.                */

        /* 1.
         *      call _Mnumgt.
         */
        ret_code = _Mnumgt( &(pt->code) , onum);
        brkflg = C_SWON;

        switch( ret_code )
          {
            /* 2.
             *      kmnumget sucsecc return.
             */
            case IMSUCC :

                switch( kjsvpt->knjnumfg )
                  {
                    /* 2. 2.
                     *      Kanji number input area flag = nomal area.
                     */
                    case C_INPFLD :

                        /* 2. 2. 1
                         *      set onum to string(KCB)
                         */
                if ( pt->curcol < M_KNMSCL ) {

                        strptr = pt->string + pt->curcol;
                        memcpy( strptr , onum , C_DBCS );

                        /* 2. 2. 2
                         *      set update character position and length.
                         */
                        ret_code = _Msetch( pt , pt->curcol , C_DBCS );
                        if( ret_code != IMSUCC )
                                {
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0b , "return.2.2.2 _ME_0b");
                                                  return( ret_code );
                                }

                        /* 2. 2. 3
                         *      set curright(cursor move left corner)
                         *      set lastch(input field last chara position)
                         *      set curcol(input field cursor column)
                         */

                        if ( pt->curcol >= pt->lastch ) {
                            pt->lastch = pt->lastch + C_DBCS;
                            if (pt->lastch > M_KNMSCL ){
                                pt->lastch = M_KNMSCL;
                            };
                        };

                        kjsvpt->curright = pt->lastch ;
                        pt->curcol = pt->curcol + C_DBCS;


                } else {

                   kjsvpt->actc3 = ( kjsvpt->actc3 | A_BEEP );

                };
                        break;

                    /* 2. 3.
                     *      Kanji number input field flag = auxiliary area
                     */
                    case C_AUX1   :

                        /* 2. 3. 1
                         *      set onum to aux1(auxiliary display point)
                         */

               if ( pt->cura1c < M_KNMSCL) {

                        strptr = pt->aux1 + pt->cura1c;
                        memcpy( strptr , onum , C_DBCS );

                        /* 2. 3. 2
                         *      set cura1c(position pf pseudo cursor
                         *                           of auxiliary area 1)
                         *      to  chpsa1(update character positon
                         *                           of auxiliary area 1. )
                         *      set chlna1(update character length)
                         *                           of auxiliary area 1. )
                         */
                        pt->chpsa1 = pt->cura1c;
                        pt->chlna1 = C_DBCS;



/* #(B)  1987.11.30. Flying Conversion Change */
                        /* 2. 3. 3
                         *      set curright(cursor move right corner)
                         *      set ax1lastc(auxiliary area No.1 last
                         *                   character position)
                         */
                        if ( pt->cura1c >= kjsvpt->ax1lastc ) {

                              kjsvpt->ax1lastc = kjsvpt->ax1lastc + C_DBCS;

                              if (kjsvpt->ax1lastc > M_KNMSCL) {
                                  kjsvpt->ax1lastc = M_KNMSCL;
                              };
                        };
                        kjsvpt->curright = kjsvpt->ax1lastc ;
/* #(E)  1987.11.30. Flying Conversion Change */


                        pt->cura1c = pt->cura1c + C_DBCS;

                  } else {
                    /* let beep.                                        */

                   kjsvpt->actc3 = ( kjsvpt->actc3 | A_BEEP );

                  };
                        break;

                  default :
                        brkflg = C_SWOFF;
                  };

                  if ( brkflg == C_SWON ) break;

            default :

                /* 3.
                 *      case RC != 0
                 */
                /* let beep.                                            */

                   kjsvpt->actc3 = ( kjsvpt->actc3 | A_BEEP );

        };

        /* 4.
         *      return.
         */
/*##*/  snap3( SNAP_KCB | SNAP_KMISA, SNAP_ME_0b , "return  _ME_0b");
        return( ret_code );

}
