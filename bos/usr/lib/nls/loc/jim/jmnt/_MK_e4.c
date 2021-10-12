static char sccsid[] = "@(#)69	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_e4.c, libKJI, bos411, 9428A410j 7/23/92 03:19:43";
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
 * MODULE NAME:         _MK_e4
 *
 * DESCRIPTIVE NAME:    Display All candidates.
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
 * FUNCTION:            Check Object of conversion.
 *                      Display All candidatesin Input field
 *                      or Auxiliary area No.1
 *                      Check Yomi of Object of Conversion.
 *                      Single Kanji Candidates Open.
 *                      All Candidates Open.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3400 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MK_e4
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_e4( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Success of Execution.
 *
 * EXIT-ERROR:          IMNCVCHE:There is no converted characters.
 *                      IMNTOTHE:Another message has been already displaied.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Macaxst:Display All candidates
 *                                       in Auxiliary area No.1.
 *                              _Macifst:Display all candidates
 *                                       in Input field.
 *                              _Mifmst :Input field message set.
 *                              _Mkanagt:Get Kana data.
 *                              _Mkjgrst:Set Kana/Kanji data.
 *                              _Msglop :Singel Kanji candidates open.
 *                              _Kcallop:All candidates Open and Forward.
 *                              _Kcallfw:All candidates Forward.
 *                      Standard Library.
 *                              memcpy  :Copy characters from memory area
 *                                       A to B.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Controle Block(KCB).
 *                              kjsvpt          indlen          maxa1c
 *                              maxa1r
 *
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              kkcbsvpt        cconvlen        cconvpos
 *                              convpos         kanadata        kkmode1
 *                              kjcvmap         msetflg         realcol
 *                      Kana Kanji Controle Block(KKCB).
 *                              condbotm        grammap         grmapln
 *                              kjdata          kjlen           kjmap
 *                              kjmapln         maxkjlen        rsnumcnd
 *                              totalcan
 *
 *   OUTPUT:            Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3           alcancol        alcanrow
 *                              allcanfg        allcstge        allcstgs
 *                              iws1            kkcflag         kkcrc
 *                              kkmode2         nextact         tankan
 *                              alcnmdfg
 *                      Kana Kanji Controle Block(KKCB).
 *                              grammap         grmapln         kanadata
 *                              kanalen1        kanalen2        kjdata
 *                              kjlen           kjmap           kjmapln
 *                              reqcnt
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                              CHPTTOSH:Set short data from character
 *                                       pointed area.
 *                              SHTOCHPT:Set short data to character
 *                                       pointerd area.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Sept. 26 1988 Satoshi Higuchi
 *                      Bugs fix. The bugs are not display all candidates.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Perform Memory Operations.                   */

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
 *      Check of object of conversion.
 *      Check Single Kanji Candidates or All Candidates.
 *      Display All candidates in Input field or Axuiliary area No.1
 */
int     _MK_e4( pt )

register KCB    *pt;            /* Pointer to Kanji Control Block.      */
{
        register KMISA  *kjsvpt;/* Pointer to Kanji Monitor
                                   Internal Save Area.                  */
        register KKCB   *kkcbsvpt;
                                /* Pointer to Kana Kanji Control Block. */

        int     _Macaxst();     /* Display All candidates
                                   in Auxiliary area No.1.              */
        int     _Macifst();     /* Display all candidates
                                   in Input field.                      */
        int     _Mifmst();      /* Input field message set.             */
        int     _Mkanagt();     /* Get Kana data.                       */
        int     _Mkjgrst();     /* Set Kana/Kanji data.                 */
        int     _Msglop();      /* Single Kanji Candidates Open.        */

        int     _Kcallop();     /* All candidates Open and Forward.     */
        int     _Kcallfw();     /* All candidates Forward.              */

        char    *memcpy();      /* Copy characters from memory area
                                   A to B.                              */

        int     rc;             /* Return code.                         */

        int     i,j;            /* Loop counter.                        */
        int     loop;           /* Loop counter.                        */

        short   loop_flg = 1;   /* _Kcallfw loop flag.                  */

        short   pos;            /* Conversion position.                 */
        short   len;            /* Conversion length.                   */
        short   kanapos;        /* Position of Kana data.               */
        short   kanalen;        /* Length of Kana data.                 */
        short   jiritsu;        /* Length of Jiritsugo.                 */
        short   maplen;         /* Length of Goki.                      */

        short   allcstge;       /* All candidates first stage.          */

        short   maxlen;         /* Max kanji length.                    */

        short   totalcan;       /* Total candidates.                    */

        short   mappos;         /* Kanji map position.                  */
        short   stgcnt;         /* Candidates stage counter.            */
        short   cannum;         /* Candidates number.                   */
        short   kjlen;          /* Input field kanji data length.       */

        short   kjlenstg[256];  /* Kanji data length stage.             */
        short   candnum;        /* Candidates number.                   */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KMISA | SNAP_KKCB, SNAP_MK_e4, "Start" );



        kjsvpt   = pt->kjsvpt;
        kkcbsvpt = kjsvpt->kkcbsvpt;

        /*
         *      Initialize return code.
         */

        rc    = IMSUCC;



        /* 1.
         *      Check Object of conversion.
         */

        /*
         *      Set previous conversion mode.
         */
        kjsvpt->kkmode2 = kjsvpt->kkmode1;

        /*
         *      Check conversion length.
         *      Not Object of conversion string.
         */
        if ( !kjsvpt->cconvlen ) {

            /*
             *      Set action code No.3.
             */
            kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

            return( IMNCVCHE );
        };



        /* 2.
         *      Check an envilonment,whith field can display All candidates,
         *      Input field or Auxiliary area No.1.
         */

        /* 2.1.
         *      Check message set flag.
         */
        if ( (kjsvpt->msetflg & K_MSGOTH) == K_MSGOTH ) {

            /*
             *     Set action code No.3.
             */
            kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

            return( IMNTOTHE );
        };


        /* 2.2.
         *      Reset Tankan flag.
         */
        kjsvpt->tankan = C_SWOFF;



        /* 2.3.
         *      Check an envilonment,whith field can display All candidates,
         *      Input field or Auxiliary area No.1.
         */

        /*
         *      Display All candidates in Auxiliary area No.1
         *      with multi-rows.
         */
        if (  (pt->maxa1c >= ((int)sizeof(M_ACAXM1) - 1)) &&
              (pt->maxa1r >= M_ALAXDR)                )   {


            /* 2.4.
             *      Set  All candidates indication flag
             *          and All candidates indication col
             *          and All candidates indication row.
             */

            /*
             *      Set All candidates indication flag.
             */
            kjsvpt->allcanfg = M_ACAX1A;

            /*
             *      Set All candidates indication col.
             */
            kjsvpt->alcancol = (int)sizeof(M_ACAXM1) - 1;

            /*
             *      Set All candidates indication row.
             */
            if ( pt->maxa1r > M_ALAXRW ) {

                kjsvpt->alcanrow = M_ALAXRW;

            } else {

                kjsvpt->alcanrow = pt->maxa1r;
            };


        /*
         *      Display All candidates in Auxiliary area No.1
         *      with single-row.
         */
        } else if (  (pt->maxa1c >= M_ALIFCL) &&
                     (pt->maxa1r > 0)          ) {

            /* 2.4.
             *      Set  All candidates indication flag
             *          and All candidates indication col
             *          and All candidates indication row.
             */

            /*
             *      Set All candidates indication flag.
             */
            kjsvpt->allcanfg = M_ACAX1S;

            /*
             *      Set All candidates indication col.
             */
            kjsvpt->alcancol = pt->maxa1c - (sizeof(M_ACIFMG) - 1);

            /*
             *      Set All candidates indication row.
             */
            kjsvpt->alcanrow = M_ACAX1R;


        /*
         *      Display All candidates in Input field.
         */
        } else if ( (kjsvpt->realcol - pt->indlen) >= M_ALIFCL ) {

            /* 2.4.
             *      Set  All candidates indication flag
             *          and All candidates indication col
             *          and All candidates indication row.
             */

            /*
             *      Set All candidates indication flag.
             */
            kjsvpt->allcanfg = M_ACIF;

            /*
             *      Set All candidates indication col.
             */
            kjsvpt->alcancol = kjsvpt->realcol - pt->indlen -
                               (sizeof(M_ACIFMG) - 1);

            /*
             *      Set All candidates indication row.
             */
            kjsvpt->alcanrow = M_ACAX1R;


        /*
         *      Error message.
         */
        } else {

            /* 2.5.
             *      Set error message.
             */
            rc = _Mifmst( pt, K_MSGOTH, C_FAUL, C_FAUL, C_COL, C_DBCS,
                          sizeof(M_ACERMG)-1, M_ACERMG );

            /*
             *      Set next action address.
             *          (Message reset)
             */
            kjsvpt->nextact |= M_MGRSON;

            return( IMSUCC );
        };



/* #(B) 1987.11.30. Flying Conversion Add */

        /*
         *      Check Single Kanji Candidates or All Candidates.
         */

        kjsvpt->alcnmdfg =  C_SWON;     /* Set All Candidates Mode Flag. */

        /*
         *      Set Cobversion Position and Length.
         */
        pos = kjsvpt->cconvpos - kjsvpt->convpos;
        len = kjsvpt->cconvpos + kjsvpt->cconvlen;

        /*
         *      Check Mode of Object of Conversion.
         */
        for ( loop = kjsvpt->cconvpos ; loop < len ; loop += C_DBCS ) {

            /*
             *      Not Yomi.
             */
            if ( kjsvpt->kjcvmap[pos + C_ANK] != M_KSNCNV ) {

                /*
                 *      Set All Candidates Mode flag.
                 *              All Candidates Mode.
                 */
                kjsvpt->alcnmdfg = C_SWOFF;
                break;
            };
            pos += C_DBCS;      /* Conversion Position increase.        */
        };

        /*
         *      Check All Candidates Mode flag.
         */
        if ( kjsvpt->alcnmdfg ) {       /* Single Kanji Candidates Mode.*/

            rc = _Msglop( pt ); /* Single Kanji Candidates Open.        */

            return( IMSUCC );
        };
/* #(E) 1987.11.30. Flying Conversion Add */



        /* 3.
         *      Get Kana data.
         */

        /*
         *      Position of object of conversion.
         */
        pos = kjsvpt->cconvpos - kjsvpt->convpos;

        /*
         *      Get Kana data.
         */
        rc = _Mkanagt( pt, pos, &kanapos, &kanalen, &jiritsu, &maplen );

        /*
         *      Set Kana data.
         */
        memcpy( (char *)kkcbsvpt->kanadata,
                (char *)&kjsvpt->kanadata[kanapos],
                kanalen);

        /*
         *      Set Kana data length No.1.
         */
        kkcbsvpt->kanalen1 = kanalen;

        /*
         *      Set Kana data length No.2.
         */
        kkcbsvpt->kanalen2 = kanalen;



        /* 4.
         *      Set Single kanji candidates stage.
         */

        /*
         *      Check Kanji map code.
         */
        switch( kjsvpt->kjcvmap[pos] ) {

        case M_KJMALN : /* Abbreviation/Alphabet.                       */

        case M_KJMKNM : /* Kanji number.                                */

            /*
             *      Set Single kanji candidates stage.
             *          (First request number)
             */
            kjsvpt->allcstgs[0] = 0;

            /*
             *      Set Single kanji candidates stage.
             *          (Last request number)
             */
            kjsvpt->allcstgs[1] = 0;

            break;


        default :

            /*
             *      Set Single kanji candidates stage.
             *          (First request number)
             */
            kjsvpt->allcstgs[0] = -1;

            /*
             *      Set Single kanji candidates stage.
             *          (Last request number)
             */
            kjsvpt->allcstgs[1] = 0;

            break;
        };



        /* 5.
         *      Set Kana/Kanji data.
         */

        /* 5.1.
         *      Kanji Monitor Internal Save Area data copy to
         *          Kana Kanji Control Block.
         */
        rc = _Mkjgrst( pt, M_1SDATA );

        /*
         *      Shift Kanji data by Double byte.
         */
        memcpy( (char *)kjsvpt->iws1,
                (char *)kkcbsvpt->kjdata,
                kkcbsvpt->kjlen);
        SHTOCHPT( kkcbsvpt->kjdata, C_DBCS );
        memcpy( (char *)&kkcbsvpt->kjdata[C_DBCS],
                (char *)kjsvpt->iws1,
                kkcbsvpt->kjlen);

        /*
         *      Kanji data length increase.
         */
        kkcbsvpt->kjlen += C_DBCS;

        /*
         *      Shift Kanji map data by Double byte .
         */
        memcpy( (char *)kjsvpt->iws1,
                (char *)kkcbsvpt->kjmap,
                kkcbsvpt->kjmapln);
        SHTOCHPT( kkcbsvpt->kjmap , C_DBCS);
        memcpy( (char *)&kkcbsvpt->kjmap[C_DBCS],
                (char *)kjsvpt->iws1,
                kkcbsvpt->kjmapln);

        /*
         *      Kanji map length increase.
         */
        kkcbsvpt->kjmapln += C_DBCS;

        /*
         *      Shift Grammer map data by Single byte.
         */
        memcpy( (char *)kjsvpt->iws1,
                (char *)kkcbsvpt->grammap,
                kkcbsvpt->grmapln);
        kkcbsvpt->grammap[0] = C_ANK;
        memcpy( (char *)&kkcbsvpt->grammap[C_ANK],
                (char *)kjsvpt->iws1,
                kkcbsvpt->grmapln);

        /*
         *      Grammer map length increase.
         */
        kkcbsvpt->grmapln += C_ANK;



        /* 6.
         *      Set All candidates stage.
         */

        /* 6.1.
         *      Set All candidates request number.
         *      Check All candidates indication flag.
         */
        switch( kjsvpt->allcanfg ) {

        /*
         *      Display All candidates in Auxiliary area No.1
         *      with multi-rows.
         */
        case M_ACAX1A :

            /*
             *      Set All candidates request number.
             */
            kkcbsvpt->reqcnt = kjsvpt->alcanrow - M_ACMGFD;

            break;

        /*
         *      Display All candidates in Auxiliary area No.1
         *      with multi-row.
         */
        case M_ACAX1S :
        /*
         *      Display All candidates in Input field.
         */
        case M_ACIF :

            /*
             *      Set All candidates request number.
             */
            kkcbsvpt->reqcnt = 1;

            break;
        };


        /* 6.2.
         *      All candidates Open.
         */

        /*
         *      Set Kana Kanji Conversion calling flag.
         */
        kjsvpt->kkcflag = M_KKNOP;

        /*
         *      All candidates Open.
         */
        kjsvpt->kkcrc = _Kcallop( kkcbsvpt );


        /* 6.3.
         *      Check _Kcallop return code.
         */
        switch( kjsvpt->kkcrc ) {

        /* 6.3.1.
         *      All candidates success.
         */
        case K_KCSUCC :

            switch( kjsvpt->allcanfg ) {

            /*
             *      Display All candidates in Auxiliary area No.1
             *      with multi-rows.
             */
            case M_ACAX1A :

                /*
                 *      Set All candidates default request number.
                 */
/*======================================================================*/
/* #(B) Sept. 26 1988 Changed by S,Higuchi                              */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      allcstge = kjsvpt->alcanrow - M_ACMGFD;                         */
/* New source.                                                          */
/*      allcstge = kkcbsvpt->rsnumcnd;                                  */
/*======================================================================*/
		allcstge = kkcbsvpt->rsnumcnd;

                /*
                 *      Set Total candidates number.
                 */
                totalcan = kkcbsvpt->totalcan;

/*======================================================================*/
/* #(B) Sept. 26 1988 Changed by S,Higuchi                              */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      loop = ( totalcan - (totalcan % allcstge) ) / allcstge;         */
/*      if ( loop >= M_CANDNM ) {                                       */
/*          kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;                   */
/*          return( IMSUCC );                                           */
/*      };                                                              */
/*      for ( i=0 ; i<loop ; i++ ) {                                    */
/*          kjsvpt->allcstge[i] = allcstge;                             */
/*      };                                                              */
/*      kjsvpt->allcstge[loop] = totalcan % allcstge;                   */
/* New source.                                                          */
/*      for(i=0,j=0;j<totalcan;i++) {                                   */
/*          if(i >= M_CANDNM) {                                         */
/*              kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;               */
/*              return( IMSUCC );                                       */
/*          }                                                           */
/*          j += allcstge;                                              */
/*          kjsvpt->allcstge[i] = allcstge;                             */
/*          kjsvpt->kkcrc = _Kcallfw(kkcbsvpt);                         */
/*          allcstge = kkcbsvpt->rsnumcnd;                              */
/*          if(kjsvpt->kkcrc >= K_KCNFCA) {                             */
/*              kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;               */
/*              return(IMSUCC);                                         */
/*          }                                                           */
/*      }                                                               */
/*======================================================================*/
		for(i=0,j=0;j<totalcan;i++) {
		    /*
		     *      Out of All candidates.
		     */

		    if(i >= M_CANDNM) {
                    /*
                     *      Set action code No.3.
                     */
			kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;
			return( IMSUCC );
		    }
                    /*
                     *      Set All candidates request number stage.
                     */
		    j += allcstge;
		    kjsvpt->allcstge[i] = allcstge;
		    kjsvpt->kkcrc = _Kcallfw(kkcbsvpt);
		    allcstge = kkcbsvpt->rsnumcnd;

/*              Check _Kcallfw return code.                             */
		    if(kjsvpt->kkcrc >= K_KCNFCA) {
			kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;
			return(IMSUCC);
		    }
		}

                /*
                 *      Set Max Kanji data length.
                 */
                maxlen = kkcbsvpt->maxkjlen + M_KNJNOL;

                /*
                 *      Check max kanji data length.
                 */
                if ( maxlen <= ((int)sizeof(M_ACAXM1) - 1) ) {

                    /*
                     *      Set All candidates indication col.
                     */
                    kjsvpt->alcancol = sizeof(M_ACAXM1) - 1;

                } else if ( maxlen >= pt->maxa1c ){

                    /*
                     *      Set All candidates indication col.
                     */
                    kjsvpt->alcancol = pt->maxa1c;

                } else {

                    /*
                     *      Set All candidates indication col.
                     */
                    kjsvpt->alcancol = maxlen;
                };
                break;


            /*
             *      Display All candidates in Auxiliary area No.1
             *      with single-row.
             */
            case M_ACAX1S :
            /*
             *      Display All candidates in Input field.
             */
            case M_ACIF :

                mappos = 0;     /* Initialize Kanji map position.       */

                /*
                 *      Get Kanji map length.
                 */
                maplen = CHPTTOSH( &kkcbsvpt->kjmap[mappos] );

                jiritsu = 0;    /* Initialize Jiritsugo length.         */

                for ( j=C_DBCS ; j<maplen ; j++ ) {

                    /*
                     *      Check Kanji map code.
                     *          (Adjust)
                     */
                    if ( kkcbsvpt->kjmap[mappos+j] == M_KJMJAN ) {

                        break;
                    };

                    jiritsu++;  /* Jiritsugo length increase.           */
                };

                /*
                 *      Set Kanji data length.
                 */
                kjlenstg[0] = jiritsu * C_DBCS;

                candnum = 1;       /* Set Candidates number.            */


                while ( loop_flg ) {

                    /*
                     *      Set All candidates request number.
                     */
                    kkcbsvpt->reqcnt = M_DFLTRC;

                    /*
                     *      Set Kana Kanji Conversion calling flag.
                     */
                    kjsvpt->kkcflag = M_KKNOP;

                    /*
                     *      All candidates Forward.
                     */
                    kjsvpt->kkcrc = _Kcallfw( kkcbsvpt );

                    /*
                     *      Check _Kcallfw return code.
                     */
                    if ( kjsvpt->kkcrc >= K_KCNFCA ) {

                        /*
                         *      Set action code No.3.
                         */
                        kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

                        return( IMSUCC );

                    };

                    mappos = 0; /* Initialize Kanji map position.       */

                    for ( i=0 ; i<kkcbsvpt->rsnumcnd ; i++ ) {

                        /*
                         *      Get Kanji map length.
                         */
                        maplen = CHPTTOSH( &kkcbsvpt->kjmap[mappos] );

                        jiritsu = 0;    /* Initialize Jiritsugo length. */

                        for ( j=C_DBCS ; j<maplen ; j++ ) {

                            /*
                             *      Check Kanji map code.
                             *          (Adjust)
                             */
                            if ( kkcbsvpt->kjmap[mappos+j] == M_KJMJAN ) {

                                break;
                            };

                            jiritsu++;  /* Jiritsugo length increase.   */
                        };

                        /*
                         *      Set Kanji data length.
                         */
                        kjlenstg[candnum] = jiritsu * C_DBCS;

                        candnum++;      /* Candidates number increase.  */

                        /*
                         *      Kanji map position increase.
                         */
                        mappos += maplen;
                    };

                    /*
                     *      Page end of All candidates.
                     */
                    if ( kkcbsvpt->totalcan == kkcbsvpt->candbotm ) {

                        loop_flg = 0;   /* Loop flag reset.             */

                        break;
                    };
                };

                /*
                 *      Initialize counter of
                 *          All candidates request number stage.
                 */
                stgcnt = 0;

                cannum = 0;     /* Initialize Candidate number.         */

                kjlen = 0;      /* Initialize Kanji data length.        */

                for ( i=0 ; i<candnum ; i++ ) {

                    /*
                     *      Kanji data length increase.
                     */
                    kjlen += (kjlenstg[i] + M_KNJNOL);

                    /*
                     *      Check Kanji data length.
                     */
                    if ( kjlen > kjsvpt->alcancol ) {

                        if ( kjlen == (kjsvpt->alcancol + C_DBCS) ) {

                            /*
                             *      Set All candidates request number stage.
                             */
                            kjsvpt->allcstge[stgcnt] = cannum + 1;

                        } else if ( !cannum  ) {

                            /*
                             *      Set All candidates request number stage.
                             */
                            kjsvpt->allcstge[stgcnt] = 1;
                        } else {

                            /*
                             *      Set All candidates request number stage.
                             */
                            kjsvpt->allcstge[stgcnt] = cannum;

                            i--;        /* Loop position decrease.      */
                        };

                        /*
                         *      Increase counter of
                         *          All candidates request number stage.
                         */
                        stgcnt++;

                        /*
                         *      Out of All candidates.
                         */
			if ( stgcnt >= M_CANDNM ) {

                            /*
                             *      Set action code No.3.
                             */
                            kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

                            return( IMSUCC );
                        };

                        cannum = 0;     /* Initialize Candidate number. */

                        kjlen = 0;      /* Initialize Kanji data length.*/

                    } else if ( (cannum + 1) >= M_DFLTRC ) {

                        /*
                         *      Set All candidates request number stage.
                         */
                        kjsvpt->allcstge[stgcnt] = cannum + 1;

                        /*
                         *      Increase counter of
                         *          All candidates request number stage.
                         */
                        stgcnt++;

                        /*
                         *      Out of All candidates.
                         */
			if ( stgcnt >= M_CANDNM ) {

                            /*
                             *      Set action code No.3.
                             */
                            kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

                            return( IMSUCC );
                        };

                        cannum = 0;     /* Initialize Candidate number. */

                        kjlen = 0;      /* Initialize Kanji data length.*/

                    } else {

                        cannum++;       /* Candidate number increase.   */
                    };
                };

                if ( cannum ) {

                    /*
                     *      Set All candidates request number stage.
                     */
                    kjsvpt->allcstge[stgcnt] = cannum;

                    /*
                     *      Set All candidates request number stage.
                     *          (Last stage)
                     */
                    kjsvpt->allcstge[stgcnt+1] = 0;

                } else {

                    /*
                     *      Set All candidates request number stage.
                     *          (Last stage)
                     */
                    kjsvpt->allcstge[stgcnt] = 0;
                };

                /*
                 *      Set All candidates request number.
                 */
                kkcbsvpt->reqcnt = kjsvpt->allcstge[0];

                /*
                 *      Set Kana Kanji Conversion calling flag.
                 */
                kjsvpt->kkcflag = M_KKNOP;

                /*
                 *      All candidates Forward.
                 */
                kjsvpt->kkcrc = _Kcallfw( kkcbsvpt );

                /*
                 *      Check _Kcallfw return code.
                 */
                if (  (kjsvpt->kkcrc != K_KCSUCC) &&
                      (kjsvpt->kkcrc != K_KCNMCA)  ) {

                    /*
                     *      Check KKC return code.
                     *      Set action code No.3.
                     */
                    if ( kjsvpt->kkcrc == K_KCNFCA ) {

                        kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

                    } else {

                        kjsvpt->actc3 = kjsvpt->kkmode2;
                    };

                    return( IMSUCC );
                };
                break;
            };
            break;


        /*
         *      All candidates Page end.
         */
        case K_KCNMCA :

            /*
             *      Check All candidates indication flag.
             *          (Auxiliary area No.1 with multi-rows)
             *      Set All candidates indication col.
             */
            if ( kjsvpt->allcanfg == M_ACAX1A ) {

                /*
                 *      Set Max Kanji data length.
                 */
                maxlen = kkcbsvpt->maxkjlen + M_KNJNOL;

                /*
                 *      Check Max Kanji data length.
                 */
                if ( maxlen <= ((int)sizeof(M_ACAXM1) - 1) ) {

                    /*
                     *      Set All candidates indication col.
                     */
                    kjsvpt->alcancol = sizeof(M_ACAXM1) - 1;

                } else if ( maxlen >= pt->maxa1c ) {

                    /*
                     *      Set All candidates indication col.
                     */
                    kjsvpt->alcancol = pt->maxa1c;

                } else {

                    /*
                     *      Set All candidates indication col.
                     */
                    kjsvpt->alcancol = maxlen;
                };
            };

            /*
             *      Set All candidates request number stage.
             */
            kjsvpt->allcstge[0] = kkcbsvpt->totalcan;

            /*
             *      Set All candidates request number stage.
             *          (Last request number)
             */
            kjsvpt->allcstge[1] = 0;

            break;


        /*
         *      Candidate not found.
         *      Logical Error.
         *      Phygical Error.
         */
        default :

            /*
             *      Check KKC return code.
             *      Set action code No.3.
             */

            if ( kjsvpt->kkcrc == K_KCNFCA ) {

                kjsvpt->actc3 = kjsvpt->kkmode2 | A_BEEP;

            } else {

                kjsvpt->actc3 = kjsvpt->kkmode2;
            };

            return( IMSUCC );

        };



        /* 7.
         *      Display.
         *      Check All candidates indication flag.
         */
        switch( kjsvpt->allcanfg ) {

        /*
         *      Display is AUxiliary area No.1 with multi-rows.
         */
        case M_ACAX1A :

            /*
             *      Auxiliary area No.1 message set.
             */
            rc = _Macaxst( pt );

            break;

        /*
         *      Display is AUxiliary area No.1 with single-row.
         */
        case M_ACAX1S :
        /*
         *      Display is Input field.
         */
        case M_ACIF :

            /*
             *      Input field message set.
             */
            rc = _Macifst( pt );

            break;
        };



        /* 8.
         *      Return.
         */



/*********************** Control Block Snap. ****************************/
/*                                                                      */
/*      SNAP AREA:      Kanji Monitor Internal Save Area.               */
/*                      Kana Kanji Control Block.                       */
/*                                                                      */
/************************************************************************/

        snap3( SNAP_KCB | SNAP_KMISA | SNAP_KKCB, SNAP_MK_e4, "Return");

        return( IMSUCC );
}
