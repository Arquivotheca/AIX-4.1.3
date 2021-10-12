static char sccsid[] = "@(#)61	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_a2.c, libKJI, bos411, 9428A410j 7/23/92 03:19:12";
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* System memory operation utility              */

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

static  void _M2n1n(); /*  convert DBCS of Kanji No. into PC code */

/*
 *   This module does,
 *       1.Check PROFILE.
 *       2.Get Kanji number.
 *       3.Convert Kanji No. into Kanji character.
 *       4.Reset message.
 *       5.Set converted character to string in KCB.
 */

int _MK_a2(pt)
KCB     *pt ;         /*  pointer to Kanji Control Block        */
{
        char    *memcpy();         /*  copies a specified # of character  */
        char    *memset();         /*  change first characters            */
        KMISA   *kjsvpt;           /*  pointer to kanji moniter ISA       */
        extern  int     _Mktnc();  /*  Restore saved field.               */
        extern  int     _Mecho();  /*  Set a character to KCB for echo.   */
        extern  int     _Mreset(); /*  Reset mode.                        */
        int     rc ;                /* return value                       */
        char    spcflg ;            /* space check flag                   */
        char    lpsw   ;            /* loop switch flag                   */
        char    modflg ;            /* beep flag                          */
        uchar   *strptr ;           /* pointer to kanji number            */
        int     knjnumln;           /* kanji number length                */
        char    knjnum[M_MAXKJN];   /* internal area for kanji number     */
        char    spc[2];             /* character string                   */
        char    jisnum[8] ;         /* PC code                            */
        int     i;                  /* loop counter                       */

/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_MK_a2 , "start _MK_a2" );

         kjsvpt = pt->kjsvpt;  /*  set pointer to kanji moniter ISA  */
         rc = IMSUCC;          /*  set return value                  */

        /* 1.
         *   Initialize  !  flag.
         */
         spcflg = C_SWOFF;    /* Initialize space check flag         */
         lpsw   = C_SWOFF;    /* Initialize loop switch              */
         modflg = 0;          /* Initialize beep flag                */

        /* 2.
         *   Get  !   kanji number.
         */
         while(lpsw == C_SWOFF)  {

                             /*  check type of input code  */
                             /*  Not pseudo code           */
             if(pt->type != K_INESCF)  {

                 modflg = A_BEEP;    /*  set beep  */
                 break;

             };



/* #(B)  1987.11.30. Flying Conversion Change */
                            /*  input field     */
            if(kjsvpt->knjnumfg == C_INPFLD) {

                            /*  Set pointer to Kanji number   */
                strptr = pt->string + (int )kjsvpt->curleft;

                            /*  Get length of Kanji number   */
                knjnumln = pt->lastch - kjsvpt->curleft;

            } else {           /*  auxiliary area  */

                            /*  Set pointer to Kanji number   */
                strptr = pt->aux1 + (int )kjsvpt->curleft;

                            /*  Get length of Kanji number   */
                knjnumln = kjsvpt->ax1lastc - kjsvpt->curleft;

            };
/* #(E)  1987.11.30. Flying Conversion Change */



            if(knjnumln > 0) {

                            /*  Copy Kanji number to internal area  */
                (void)  memcpy(knjnum, strptr, knjnumln);

                            /*
                             *  Add NULL-code to the end of Kanji number.
                             */
                (void)  memset(&knjnum[knjnumln], 0x00, 1);

            };

                    /*  Set high byte of space code to local valiable  */
            spc[0] = HIGH(C_SPACE);

                    /*  Set low byte of space code to local valiable  */
            spc[1] = LOW(C_SPACE);


                         /*  Check first character of Kanji number */
            if ( strncmp(strptr, spc, C_DBCS) == 0 )  {

                spcflg = C_SWON;    /*  Set space check flag  */

            };

           if ( spcflg == C_SWOFF ) {  /*  Check space check flag  */

            switch(pt->code)  {      /* Check depressed key  */

                  case  P_ACTION :   /* Case Action  */

                                     /*  Check input key in PROFILE */
                            if((kjsvpt->kmpf[0].kjin & K_DAACT)
                              != K_DAACT) {

                                modflg = A_BEEP;    /* Set beep        */
                                lpsw   = C_SWON;    /* Set loop switch */

                            };

                            break;

                  case  P_ENTER :    /*  Case Enter   */

                                     /*  Check input key in PROFILE */
                            if((kjsvpt->kmpf[0].kjin & K_DAENT)
                                != K_DAENT)  {

                                modflg = A_BEEP;    /* Set beep        */
                                lpsw   = C_SWON;    /* Set loop switch */

                            };

                            break;

                  case  P_CR :       /*  Case CR  */

                                     /*  Check input key in PROFILE */
                            if((kjsvpt->kmpf[0].kjin & K_DACR)
                              != K_DACR) {

                                modflg = A_BEEP;    /* Set beep        */
                                lpsw   = C_SWON;    /* Set loop switch */

                            };

                            break;

                  default :          /* Case default  */

                            modflg = A_BEEP;        /* Set beep        */
                            lpsw   = C_SWON;        /* Set loop switch */

                            break;
            };

          } else {

            switch(pt->code)  {    /*  Check input code */

                  case  P_ACTION :  /*  Case Action  */

                           /*
                            *  Check input key in PROFILE
                            *  and reset key in PROFILE
                            */
                            if( ((kjsvpt->kmpf[0].kjin & K_DAACT)
                                  != K_DAACT) &&
                                ((kjsvpt->kmpf[0].reset & K_REACT)
                                  != K_REACT) )  {

                               modflg = A_BEEP;   /*  Set beep   */

                            };

                            lpsw  = C_SWON;       /* Set loop switch  */

                            break;

                  case  P_ENTER : /*  Case Enter   */

                           /*
                            *  Check input key in PROFILE
                            *  and reset key in PROFILE
                            */
                            if( ((kjsvpt->kmpf[0].kjin & K_DAENT)
                                != K_DAENT) &&
                                ((kjsvpt->kmpf[0].reset & K_REENT)
                                  != K_REENT) ) {

                               modflg = A_BEEP;   /*  Set beep   */

                            };

                            lpsw  = C_SWON;       /* Set loop switch  */

                            break;

                  case  P_CR :   /*  Case CR  */

                           /*
                            *  Check input key in PROFILE
                            *  and reset key in PROFILE
                            */
                            if ( ((kjsvpt->kmpf[0].kjin & K_DACR)
                              != K_DACR) &&
                                ((kjsvpt->kmpf[0].reset & K_RECR)
                                  != K_RECR) ) {

                                modflg = A_BEEP;    /* set beep */
                            };

                            lpsw  = C_SWON;         /* Set loop switch  */

                            break;

                  default:     /* Case default  */

                                spcflg = C_SWOFF;  /* Set space check flag */
                                lpsw = C_SWON;     /* Set loop switch  */

                            break;

            };

          };

            if(lpsw == C_SWON)   /*  Check loop switch  */

                break;

                                 /*  call internal subroutine  */
            (void)_M2n1n(knjnum, knjnumln, jisnum);

                     /*
                      *  JIS Kuten code
                      *  to PC kanji code conversion.
		      *  Add JIS78 or JIS83 Mode switch 
                      */
            if ( kjsvpt->kmpf[0].kjno == K_KJIS ) {
                rc = _Mktnc(jisnum, &(kjsvpt->chcode[0]),
                                               kjsvpt->kmpf[0].kuten);
            } else {
                unsigned char buff[12];

                if( (rc = ConvAtoI(jisnum, buff)) == IMSUCC ) {
                    rc = ConvItoS(buff, &(kjsvpt->chcode[0]));
                }
            }

            if(rc != IMSUCC)  {  /*  Check return code  */

                            /*  Set length of changed character in KMISA  */
                kjsvpt->chcodlen = C_DBCS;

                if ( kjsvpt->knjnumfg == C_INPFLD ) {

                        pt->curcol = kjsvpt->curleft;
                } else {
                        pt->cura1c = kjsvpt->curleft;
                };

                            /*  Set beep set flag  */
                modflg = ((modflg & 0xf0) | A_BEEP);

                break;

            };

            if(kjsvpt->knjnumfg == C_INPFLD)  {  /*  input field  */

                     /*
                      *  Restore Saved field Information.
                      */
                _Mfmrst(pt, K_MSGOTH);

                            /*  Set beep flag  */
                modflg = ((modflg & 0xf0) | A_1STINP);

            }
            else {                               /*  auxiliary area  */

                     /*
                      *  Restore Saved field Information.
                      */
                _Mfmrst(pt, K_MSGOTH);

                            /*  Set beep flag  */
                modflg = ((modflg & 0xf0) | A_KNJNOM);

            };

                        /*  Set character to KCB for echo */
            rc = _Mecho(pt, K_HLAT0, M_NORMAL);

            if(rc == IMSUCC){     /*  Check return code  */

                                  /*  Set KMAT Action code 2 */
                kjsvpt->actc2 = A_CRNA;

            };

                                  /*  Check Keyboard Lock Flag.    */
            if ( pt->kbdlok == K_KBLON ) {

                                  /*  Set current conversion mode  */
                kjsvpt->kkmode1 = kjsvpt->kkmode2;

            };

            lpsw = C_SWON;   /*  Set loop switch  */

         };   /*  end while(lpsw == C_SWOFF)  */

        /* 3.
         *   change  !  mode
         */

         if(spcflg == C_SWON)  {   /*  Check space check flag  */

                            /*  Check beep flag  */
                if ( (modflg & 0xf0) != A_BEEP ) {

                     /*
                      *  Restore Saved field Information.
                      */
                  _Mfmrst(pt, K_MSGOTH);

                            /*  Set beep flag  */
                  modflg = ((modflg & 0xf0) | A_1STINP);

                };
         };
         if(modflg != 0) {    /*  Check beep flag  */

            kjsvpt->actc3 = modflg; /*  Set KMAT Action code 3 */

         };
        /* 4.
         *   Return Value.
         */


/* ### */
snap3( SNAP_KCB | SNAP_KMISA , SNAP_MK_a2 , "end _MK_a2 --- No.2 --- " );

         return(rc);
}
/*  define subroutine for */
/*  convert DBCS of Kanji No. into PC code */
static  void  _M2n1n(strptr, knjnumln, jisnum)
        char  *strptr;      /* pointer to kanji number(DBCS 2Byte*/
        int   knjnumln;     /* kanji number length               */
        char  *jisnum;      /* pointer to PC code string(1Byte)  */
{
        char  chr2[C_DBCS]; /* buffer area      */
        int   i;            /* loop counter     */

        for(i = 0; i < knjnumln / C_DBCS; i++)  {

                /*  Copy Kanji number to buffer area  */
                memcpy(chr2, strptr + i * C_DBCS, C_DBCS);

                /*  Check code of Kanji number */
                if(chr2[0] != 0x82 || chr2[1] > 0x59 || chr2[1] < 0x4f)  {
                        /* Set space code to PC code string */
                        jisnum[i] = 0x20;
                        continue;
                };

                /* Set PC code string */
                jisnum[i] = chr2[1] - 0x1f;
        };

        /******************************************************************/
        /*#(B) Bug Fix.  Mon Nov 09,1987                                  */
        /*     Modify Reazon.                                             */
        /*              C String Must Be Terminate By Null Character      */
        /*              ,But Now Not Terminate By Null.                   */
        /*     Add Source Code.                                           */
        /*             jisnum[i] = '\0';                                  */
        /******************************************************************/
        /*
         *      Set NULL Character.
         */
        jisnum[i] = '\0';
}
