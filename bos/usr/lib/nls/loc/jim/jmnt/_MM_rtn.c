static char sccsid[] = "@(#)73	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MM_rtn.c, libKJI, bos411, 9428A410j 7/23/92 03:19:58";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _MM_rtn
 *
 * DESCRIPTIVE NAME:    Kanji monitor mode switching routine.
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
 * FUNCTION:            Change Knaji Monitor according to input parameter
 *                      and current status.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3200 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _MM_rtn
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MM_rtn( pt, m_code )
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      m_code  :Kanji Monitor Action Table Code 3.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IMSUCC  :Successful.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              _Mindset():Set Shift Indicator.
 *                              _Msetch() :Set Changed Position and
 *                                         Changed Length for Display.
 *                              _Maxmst() :Set Auxiliary Area Message.
 *                              _Mifmst() :Set Input Field Message.
 *                              _Mfmrst() :Restore Saved Field.
 *                              _MM_09()  :Mode Switching for
 *                                         Dictionary Registration.
 *
 *                      Standard Library.
 *                              memset()
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
 *                              kjsvpt  beep    curcol  axuse1  maxa1c
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              convpos convlen cconvpos        cconvlen
 *                              realcol
 *                      Trace Block(TRB).
 *                              NA.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Control Block(KCB).
 *                              discrd  hlatst  chpos   chlen
 *                              chpsa1  chlna1
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              beep    nextact kkmode1 kkmode2 kjcvmap
 *                              hkmode  knjnumfg        cvmdsw
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
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Added pt->indlen in if statement and
 *                      Changed the value in if statement from 4 to 5
 *                      See problem collection sheet P-8 and P-9.
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
#include "kcb.h"        /* Kanji Control Brock Structure.               */


/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      This module dose,
 *              1. Change Knaji Monitor according to input parameter
 *                 and current status.
 *              2. Display field message if necessary.
 */
int  _MM_rtn( pt, m_code )

KCB     *pt;            /* Pointer to Kanji Control Block               */
uchar   m_code;         /* KMAT Action Code 3                           */
{
        register        KMISA   *kjsvpt;        /* Pointer to KMISA     */

        extern  int     _Mindset();     /* Set Shift Indicator          */
        extern  int     _Msetch();      /* Set Changed Position and     */
                                        /* Changed Length for Display   */
        extern  int     _Maxmst();      /* Set Auxiliary Area Message   */
        extern  int     _Mifmst();      /* Set Input Field Message      */
        extern  int     _Mfmrst();      /* Restore Saved Field          */
        extern  int     _MM_09();       /* Mode Switching for           */
                                        /* Dictionary Registration      */
        extern  char    *GetHostCodeConverter();  /* Get iconv pointer  */

        uchar   *kjcvmap;       /* Work Pointer to Kanji and            */
                                /* Conversion Map                       */
        uchar   m_codeh;        /* KMAT Action Code 3 Higher Half Byte  */
        uchar   m_codel;        /* KMAT Action Code 3 Lower Half Byte   */

        uchar   *hlatst;        /* Pointer to Highlight Attribute       */
        uchar   conv_st;        /* Conversion Status                    */
        uchar   conv_stb;       /* Conversion Status (Before)           */

        uchar   msg_typ;        /* Message Type                         */
        short   area_num;       /* Auxiliary Area Number                */
        short   scr_col;        /* Message Column                       */
        short   scr_row;        /* Message Row                          */
        short   csr_col;        /* Cursor Column                        */
        short   csr_row;        /* Cursor Row                           */
        short   csr_lft;        /* Cursor Left Limit                    */
        short   csr_rgt;        /* Cursor Right Limit                   */
        short   str_len;        /* Indicator Length                     */
        uchar   *str_ptr;       /* Pointer to Message String            */

        int     i;              /* Loop Counter                         */
        int     j;              /* Loop Counter                         */

        int     rc;             /* Return Code.                         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MM_rtn,"start _MM_rtn");

        /*
         *      Pointer to Kanji Monitor Internar Save Area
         */

        kjsvpt = pt->kjsvpt;

        /*
         *      Set Successful Resturn Code
         */

        rc = IMSUCC;

        /* 1. 1
         *      Get Higher Half Byte of The Action Code
         */

        m_codeh = m_code & C_SBF0;

        /*
         *      Process by Higher Half Byte
         */

        switch ( m_codeh )
               {
                /*
                 *      No Operation
                 */
                case A_NOP:
                        break;

                /*
                 *      Beep
                 */
                case A_BEEP:
                        /*
                         *      Check Beep Flag
                         */
                        if ( kjsvpt->kmpf[0].beep == K_BEEPON )
                                pt->beep = K_BEEPON;    /* Set Beep Flag */
                        break;

                /*
                 *      Return to Application
                 */
                case A_DICOFF:
                        pt->discrd = K_DISOFF;  /* Set Discard Flag      */
                        break;

                /*
                 *      Conversion Impossible Indicator or Beep
                 */
                case A_INPCON:
                        /*
                         *      Check Beep Flag
                         */
                        if ( kjsvpt->kmpf[0].beep == K_BEEPON )
                                pt->beep = K_BEEPON;    /* Set Beep Flag  */

                        kjsvpt->convimp = K_CVIPON;     /* Convrsion      */
                                                        /* Impossible Flag*/

                        rc = _Mindset( pt, M_INDL );    /* Set Indicator  */

                        /* Set Next Action Code to Reset Indicator */
                        kjsvpt->nextact = kjsvpt->nextact | M_CNRSON;
                        break;

                /*
                 * No Character to be Converted
                 */
                case A_MNOCNV:
                        /*
                         *      Check Conversion Length
                         */
                        if ( kjsvpt->convlen == 0 )
                                /* Set First Yomi Input Mode */
                                kjsvpt->kkmode1 = A_1STINP;
                        else
                                /* Edit Mode (E) */
                                m_code = A_EDTMOD;
                        break;
               };

        /* 1. 2
         *      Get Lower Half Byte of Action Code
         */

        m_codel = m_code & C_SB0F;

        /*
         *      Process by Lower Half Byte
         */
        switch ( m_codel )
               {
                /*
                 *      No Operation
                 */
                case A_NOP:
                        break;

                /*
                 *      Set First Yomi Input Mode
                 */
                case A_1STINP:
                        kjsvpt->kkmode1 = A_1STINP;     /* 1st. Yomi Set*/
                        break;

                /*
                 *      Set Continuous Yomi Input Mode
                 */
                case A_CONINP:
                        kjsvpt->kkmode1 = A_CONINP;     /* Cont. Yomi   */
                        break;

                /*
                 *      Set Conversion Mode
                 */
                case A_CNVMOD:
                        kjsvpt->kkmode1 = A_CNVMOD;     /* Conv. Mode   */
                        break;

                /*
                 *      Set Edit Mode (A,B,C,D,E)
                 */
                case A_EDTMOA:
                case A_EDTMOB:
                case A_EDTMOC:
                case A_EDTMOD:
                case A_EDTMOE:

                        /* (a)
                         *      Check Conversion Length
                         */
                        if ( kjsvpt->convlen == 0 )
                           {
                                /* First Yomi Input Mode */
                                kjsvpt->kkmode1 = A_1STINP;
                                break;
                           };

                        /*  (b)
                         *      If Cursor Position is
                         *         Outside The Conversion Area
                         */
                        if ( pt->curcol < kjsvpt->convpos ||
                             pt->curcol > kjsvpt->convpos + kjsvpt->convlen)
                           {
                                /* Set Edit Mode C */
                                kjsvpt->kkmode1 = A_EDTMOC;
                                break;
                           };

                        /*
                         *      Position to Kanji and Conversion Map
                         *              for Cursor Position
                         */
                        kjcvmap = kjsvpt->kjcvmap + pt->curcol
                                                  - kjsvpt->convpos;

                        /*
                         *      Get of Conversion Status of Cursor Position
                         */
                        conv_st  = *(kjcvmap + 1) & C_SBF0;

                        /*
                         *      Get Conversion Status
                         *              Before  Cursor Position
                         */
                        conv_stb = *(kjcvmap - 1) & C_SBF0;

                        /*  (c)
                         *      Check Cursor Position == Conversion Position
                         */
                        if ( pt->curcol ==  kjsvpt->convpos )
                           {
                                /* (c-1)
                                 *      Check Conversion Status == Kanji
                                 *              or
                                 *      Yomi Once Converted
                                 */
                               if ( conv_st == M_KSCNVK ||
                                    conv_st == M_KSCNVY )

                                  {
                                        /* Set Edit Mode A */
                                        kjsvpt->kkmode1 = A_EDTMOA;
                                        break;
                                  }  ;
                                /*  (c-2)
                                 *      Check Conversion Status = Kanji
                                 *              or
                                 *      Yomi Not Once Converted
                                 */
                               if ( conv_st == M_KSNCNV )

                                  {
                                        /* Set Edit Mode B */
                                        kjsvpt->kkmode1 = A_EDTMOB;
                                        break;
                                  }
                               else
                                  {
                                        /* (c-3) Set Edit Mode C */
                                        kjsvpt->kkmode1 = A_EDTMOC;
                                        break;
                                  };
                           };

                        /*  (d)
                         *      Check Cursor Position ==
                         *              The End of Conversion Length
                         */
                        if ( pt->curcol ==  kjsvpt->convpos +
                                            kjsvpt->convlen )
                           {
                                /*  (d-1)
                                 *      Check Conversion Status ==
                                 *              Not Converted
                                 */
                                if ( conv_stb == M_KSNCNV )
                                   {


/* #(B)  1987.11.30. Flying Conversion Change */
                                      if ( kjsvpt->convlen ==
                                           kjsvpt->cconvlen )
                                         {
                                            /* Set Continuous
                                               Yomi Input Mode. */
                                            kjsvpt->kkmode1 = A_CONINP;
                                         }
                                      else
                                         {
                                            /* Set Edit Mode E */
                                            kjsvpt->kkmode1 = A_EDTMOE;
                                         };
/* #(E)  1987.11.30. Flying Conversion Change */


                                      break;
                                   };

                                /*   (d-2)
                                 *      Conversion Status before
                                 *              Cursor Position ==
                                 *      Kanji or Yomi Once Converted
                                 *              and
                                 *      Current Conversion Length Exists
                                 */
                                if ( (conv_stb == M_KSCNVK ||
                                      conv_stb == M_KSCNVY) &&
                                     (kjsvpt->cconvlen > 0) )
                                   {
                                        /* Conversion Mode */
                                        kjsvpt->kkmode1 = A_CNVMOD ;
                                        break;
                                   };

                                /*  (d-3)
                                 *      Otherwise
                                 *      Set Edit Mode C
                                 */
                                kjsvpt->kkmode1 = A_EDTMOC ;
                                break;
                           };


                        /*  (e)
                         *      Conversion Length == 0
                         */
                        if ( kjsvpt->cconvlen == 0 )
                           {
                                /* Set Edit Mode C */
                                kjsvpt->kkmode1 = A_EDTMOC;
                                break;
                           };

                        /*  (f)
                         *      Conversion Status of Cursor Position ==
                         *              Not Converted
                         */
                        if ( conv_st  == M_KSNCNV )
                           {
                                /* Set Edit Mode B */
                                kjsvpt->kkmode1 = A_EDTMOB;
                                break;
                           };

                        /*  (g)
                         *      Conversion Status before Cursor Position ==
                         *                      Not Converted
                         */
                        if ( conv_stb == M_KSNCNV )
                           {
                                /* (g-1)
                                 *      Cursor Position ==
                                 *              Current Conversion Position
                                 *              and
                                 *      Current Conversion Length Exists
                                 */
                                if ( pt->curcol == kjsvpt->cconvpos )
                                   {
                                        /* (g-1) Set Edit Mode D */
                                        kjsvpt->kkmode1 = A_EDTMOD;
                                        break;
                                   }
                                else
                                   {
                                        /* (g-2) Set Edit Mode E */
                                        kjsvpt->kkmode1 = A_EDTMOE;
                                        break;
                                   };
                           };

                        /*   (h)
                         *      Conversion Status of Cursor Position
                         *      Kanji or Yomi Once Converted
                         */
                        if ( conv_st  == M_KSCNVK ||
                             conv_st  == M_KSCNVY )
                           {
                                /* Set Edit Mode A */
                                kjsvpt->kkmode1 = A_EDTMOA;
                                break;
                            };

                        /*
                         *      Default
                         *      Set Edit mode C
                         */
                        kjsvpt->kkmode1 = A_EDTMOC;
                        break;

                /*
                 *      Set Hiragana / Katakana Conversion Mode
                 */
                case A_HIRKAT:
                        /* Set Hiragana/Katakana Conversion Mode */
                        kjsvpt->kkmode1 = A_HIRKAT;

                        /* Set Hiragana/Katakana Mode to "Reset" */
                        kjsvpt->hkmode = K_HKRES;

                        /* Set Work Pointer to Highlight Attribute */
                        hlatst  = pt->hlatst;

                        /* Move Pointer to Highlight Attribute */
                        hlatst += kjsvpt->cconvpos;

                        /* Set Highlight Attribute to "Normal" */
                        for ( i = 0; i < kjsvpt->cconvlen; i++ )
                                *hlatst++ = K_HLAT0;

                        /*
                         *      Set Changed Position and Length
                         */
                        rc = _Msetch( pt, kjsvpt->cconvpos,
                                          kjsvpt->cconvlen );
                        break;

                /*
                 *      All Candidates Mode
                 */
                case A_ALCADM:
                        /* Store Current Mode */
                        kjsvpt->kkmode2 = kjsvpt->kkmode1;

                        /* Set All Candidates Mode */
                        kjsvpt->kkmode1 = A_ALCADM;
                        break;

                /*
                 *      Kanji Number Mode
                 */
                case A_KNJNOM:
                        /*
                         *      Check Current Mode == Kanji Number Mode
                         */
                        if ( kjsvpt->kkmode1 == A_KNJNOM )
                           {
                                /* KANJI no. input key to change code  */
                                /* P_KANJI == PKJNUM in jexm/exmdefs.h */
                                if ( pt->code == P_KANJI )
                                   {
                                        if ( kjsvpt->kmpf[0].kjno == K_KJIS &&
                                             GetHostCodeConverter() != NULL )
                                           {
                                                kjsvpt->kmpf[0].kjno = K_KNO;
                                                memcpy(pt->aux1, M_KNMSGI, 10);
                                           }
                                        else
                                           {
                                                kjsvpt->kmpf[0].kjno = K_KJIS;
                                                memcpy(pt->aux1, M_KNMSG, 10);
                                           }
                                        pt->chpsa1 = 0;
                                        pt->chlna1 = pt->cura1c;
                                        break;
                                   }
                                /* Reset Field Message */
                                rc = _Mfmrst( pt, K_MSGOTH );
                           };

                        /*
                         *      Auxiliary Area is Not Used
                         *              and
                         *      Auxiliary Area is Wide Enough
                         */
                        if ( pt->axuse1 == K_ANOUSE &&
                             pt->maxa1c >= ((int)sizeof(M_KNMSG) - 1) )
                           {
                                msg_typ  = K_MSGOTH;    /* Message Type */
                                area_num = C_AUX1;      /* Aux No.1     */
                                scr_col  = M_KNMSCL;    /* Indicator Col*/
                                scr_row  = M_KNMSRL;    /* Indicator Row*/
                                csr_col  = M_KNMCSX;    /* Cursor Col   */
                                csr_row  = M_KNMCSY;    /* Cursor Row   */
                                csr_lft  = M_KNMCSL;    /* Cur. Range L */
                                csr_rgt  = M_KNMCSR;    /* Cur. Range R */
                                                        /* Message Len. */
                                str_len  = sizeof(M_KNMSG) - 1;
                                                        /* Message Str  */
                                if ( kjsvpt->kmpf[0].kjno == K_KJIS )
                                   {
                                         str_ptr = (uchar *)M_KNMSG;
                                   }
                                else if ( GetHostCodeConverter() == NULL )
                                   {
                                         kjsvpt->kmpf[0].kjno = K_KJIS;
                                         str_ptr = (uchar *)M_KNMSG;
                                   }
                                else
                                   {
                                         str_ptr = (uchar *)M_KNMSGI;
                                   }

                                /*
                                 *      Display Auxiliary Area Message
                                 */
                                rc = _Maxmst( pt, msg_typ, area_num,
                                              scr_col, scr_row,
                                              csr_col, csr_row,
                                              csr_lft, csr_rgt,
                                              str_len, str_ptr );

                                /* Set Last Character Position */


/* #(B)  1987.11.30. Flying Conversion Change */
                                kjsvpt->ax1lastc = kjsvpt->curleft;
/* #(E)  1987.11.30. Flying Conversion Change */


                                kjsvpt->knjnumfg = C_SWON;
                           }
                        else
                           {
                                /*
                                 *      Check Input Field is Wide Enough
                                 */
/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     if(kjsvpt->realcol >= ...                               */
/*      NEW     if(kjsvpt->realcol - pt->indlen >= ...                  */
/*----------------------------------------------------------------------*/
				if ( kjsvpt->realcol - pt->indlen >=
                                             ((int)sizeof(M_KNMSG) - 1) )
                                   {
                                        /* Message Type */
                                        msg_typ = K_MSGOTH;
                                        /* Cursor Column */
                                        csr_col = M_KNMCSX;
                                        /* Cursor Row */
                                        csr_row = M_KNMCSY;
                                        /* Cursor Move Left Limit */
                                        csr_lft = M_KNMCSL;
                                        /* Cursor Move Right Limit */
                                        csr_rgt = M_KNMCSR;
                                        /* Message Length */
                                        str_len = sizeof(M_KNMSG) - 1;
                                        /* Message String */
                                        str_ptr = (uchar *)M_KNMSG;

                                        /*
                                         *      Display Input Field Message
                                         */
                                        rc = _Mifmst( pt, msg_typ,
                                                      csr_col, csr_row,
                                                      csr_lft, csr_rgt,
                                                      str_len, str_ptr );

                                        kjsvpt->knjnumfg = C_SWOFF;
                                   }
                                else
                                   {
                                        /*
                                         *      Input Field is Short
                                         */

                                        /* Message Type */
                                        msg_typ = K_MSGOTH;
                                        /* Cursor (Not to be Display) */
                                        csr_col = C_FAUL;
                                        /* Cursor (Not to be Display) */
                                        csr_row = C_FAUL;
                                        csr_lft = K_AUXNEA;
                                        csr_rgt = K_AUXNEA;
                                        /* Message Length */
                                        str_len = sizeof(M_KNMSGE) - 1;
                                        /* Message String */
                                        str_ptr = (uchar *)M_KNMSGE;

                                        /*
                                         *      Display Input Field Message
                                         *      (Inadequate Length of
                                         *              Input Fielde)
                                         */
                                        rc = _Mifmst( pt, msg_typ,
                                                      csr_col, csr_row,
                                                      csr_lft, csr_rgt,
                                                      str_len, str_ptr );

                                        /*
                                         *      Set Next Action Code
                                         *              to Reset Message
                                         */
                                        kjsvpt->nextact = kjsvpt->nextact
                                                          | M_MGRSON;
                                   };
                           };

                        /*
                         *      Check Current Mode == Kanji Number Mode
                         */
                        if ( kjsvpt->kkmode1 == A_KNJNOM )
                           {
                                /*
                                 *      Check Input Field
                                 */
                                if ( kjsvpt->knjnumfg == C_INPFLD )
                                   {
                                        /* Set Chenged Position */
                                        pt->chpos = M_KNMCSL;
                                        /* Set Chenged Length */
                                        pt->chlen = M_KNMSCL - M_KNMCSL;
                                   }
                                else
                                   {
                                        /*
                                         *      Set Changed Position in
                                         *              Auxiliary Area
                                         */
                                        pt->chpsa1 = M_KNMCSL;
                                        /*
                                         *      Set Changed Length in
                                         *              Auxiliary Area
                                         */
                                        pt->chlna1 = M_KNMSCL - M_KNMCSL;
                                        break;
                                   };
                           };

                        /* Store First Yomi Input Mode for Resetting */
                        kjsvpt->kkmode2 = A_1STINP;

                        /* Set Kanji Number Mode */
                        kjsvpt->kkmode1 = A_KNJNOM;

                        break;

                /*
                 *      Conversion Mode Switching Mode
                 */
                case A_MODESW:
                        /*
                         *      Check Current Mode
                         */
                        if ( kjsvpt->kkmode1 == A_MODESW )
                           {
                                /*
                                 *      Reset Field Message
                                 */
                                rc = _Mfmrst( pt, K_MSGOTH );
                           };

                        /* Store First Yomi Input Mode for Resetting */
                        kjsvpt->kkmode2 = A_1STINP;

                        /* Set Conversion Switching Mode */
                        kjsvpt->kkmode1 = A_MODESW;

                        /*
                         *      Auxiliary Area is Not Used
                         *              or
                         *      Auxiliary Area is Wide Enough
                         */

/*----------------------------------------------------------------------*/
/*      #(B) Changed by S,Higuchi on Aug. 23 1988                       */
/*      OLD     if ( pt->axuse1 == K_ANOUSE &&                          */
/*                   pt->maxa1c >= (((int)sizeof(M_MSWMSG)-1)/4) &&     */
/*                   pt->maxa1r >= 4 )                                  */
/*                 {                                                    */
/*                                                                      */
/*      NEW     if ( pt->axuse1 == K_ANOUSE &&                          */
/*                   pt->maxa1c >= (((int)sizeof(M_MSWMSG)-1)/5) &&     */
/*                   pt->maxa1r >= 5 )                                  */
/*                 {                                                    */
/*----------------------------------------------------------------------*/
                        if ( pt->axuse1 == K_ANOUSE &&
			     pt->maxa1c >= (((int)sizeof(M_MSWMSG)-1)/5) &&
			     pt->maxa1r >= 5 )
                           {
                                /*
                                 *      Aux. Area Indicate Parameter Set
                                 */
                                msg_typ  = K_MSGOTH;    /* Message Type */
                                area_num = C_AUX1;      /* Aux No.1     */
                                scr_col  = M_MSWMSC;    /* Indicator Col*/
                                scr_row  = M_MSWMSR;    /* Indicator Row*/
                                csr_col  = C_FAUL;      /* Cursor Col   */
                                csr_row  = C_FAUL;      /* Cursor Row   */
                                csr_lft  = K_AUXNEA;    /* Cur. Range L */
                                csr_rgt  = K_AUXNEA;    /* Cur. Range R */
                                                        /* Message Len. */
                                str_len  = sizeof(M_MSWMSG) - 1;
                                                        /* Message Str  */
                                str_ptr  = (uchar *)M_MSWMSG;

                                /*
                                 *      Display Auxiliary Area Message
                                 */
                                rc = _Maxmst( pt, msg_typ, area_num,
                                              scr_col, scr_row,
                                              csr_col, csr_row,
                                              csr_lft, csr_rgt,
                                              str_len, str_ptr );

                                /* Pointer to Auxiliary Highlight Attr. */
                                hlatst = pt->hlata1;

                                /*
                                 *      Set "Reverse"
                                 */
                                switch ( kjsvpt->kmpf[0].conversn )
                                       {
                                        /*
                                         *      Word
                                         */
                                        case K_FUKUGO:
                                                /* Set Highlight Pos. */
                                                hlatst += M_FUKPOS;
                                                break;

                                        /*
                                         *      Single Phrase
                                         */
                                        case K_TANBUN:
                                                /* Set Highlight Pos. */
                                                hlatst += M_TANPOS;
                                                break;

                                        /*
                                         *      Multi Phrase
                                         */
                                        case K_RENBUN:

/* #(B) 1988.01.12. Flying Conversion Change */
                        if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                                /* Set Highlight Pos. */
                                hlatst += M_FLYPOS;

                        } else {

                                /* Set Highlight Pos. */
                                hlatst += M_RENPOS;
                        };

                        break;
/* #(E) 1988.01.12. Flying Conversion Change */


                                       };

                                /*
                                 *      Set Aux. Area Highlight Attribute
                                 *                              (Reverse)
                                 */
                                memset( hlatst, K_HLAT1, M_MSWATR );

                                kjsvpt->cvmdsw = C_SWON;

                           }
                        else
                           {
                                /*
                                 *      Input Field is Wide Enough
                                 */
                                if ( (kjsvpt->realcol - pt->indlen)  >=
                                             ((int)sizeof(M_MSWMSG)-1) )
                                   {
                                        /* Message Type */
                                        msg_typ = K_MSGOTH;
                                        /* Cursor Not to be Display */
                                        csr_col = C_FAUL;
                                        /* Cursor Not to be Display */
                                        csr_row = C_FAUL;
                                        csr_lft = K_AUXNEA;
                                        csr_rgt = K_AUXNEA;
                                        /* Message Length */
                                        str_len = sizeof(M_MSWMSG) - 1;
                                        /* Message String */
                                        str_ptr = (uchar *)M_MSWMSG;

                                        /*
                                         *      Display Input Field Message
                                         */
                                        rc = _Mifmst( pt, msg_typ,
                                                      csr_col, csr_row,
                                                      csr_lft, csr_rgt,
                                                      str_len, str_ptr );

                                        /* Pointer to Aux. Highlight Attr.*/
                                         hlatst = pt->hlatst;

                                        /*
                                         *      Set "Reverse"
                                         */
                                        switch ( kjsvpt->kmpf[0].conversn )
                                               {
                                                /*
                                                 *      Word
                                                 */
                                                case K_FUKUGO:
                                                        /* Highlight Pos.*/
                                                        hlatst += M_FUKPOS;
                                                        break;

                                                /*
                                                 *      Single Phrase
                                                 */
                                                case K_TANBUN:
                                                        /* Highlight Pos. */
                                                        hlatst += M_TANPOS;
                                                        break;

                                                /*
                                                 *      Multi Phrase
                                                 */
                                                case K_RENBUN:


/* #(B) 1988.01.12. Flying Conversion Change. */

                        if ( kjsvpt->kmpf[0].convtype == K_CIKUJI ) {

                                /* Highlight Pos. */
                                hlatst += M_FLYPOS;

                        } else {

                                /* Highlight Pos. */
                                hlatst += M_RENPOS;
                        };
                        break;


/* #(E) 1988.01.12. Flying Conversion Change. */


                                               };

                                        /*
                                         *      Set Aux. Area Highlight
                                         *              Attribute (Reverse)
                                         */
                                        memset( hlatst, K_HLAT1, M_MSWATR );

                                        kjsvpt->cvmdsw = C_SWOFF;

                                   }
                                else
                                   {
                                        /*
                                         *      Input Field is Short
                                         */

                                        /* Message Type */
                                        msg_typ = K_MSGOTH;
                                        /* Cursor Not to be Display */
                                        csr_col = C_FAUL;
                                        /* Cursor Not to be Display */
                                        csr_row = C_FAUL;
                                        csr_lft = K_AUXNEA;
                                        csr_rgt = K_AUXNEA;
                                        /* Message Length */
                                        str_len = sizeof(M_KNMSGE) - 1;
                                        /* Message String */
                                        str_ptr = (uchar *)M_KNMSGE;

                                        /*
                                         *      Display Input Field Message
                                         *      (Inadequate Length of
                                         *              Input Field)
                                         */
                                        rc = _Mifmst( pt, msg_typ,
                                                      csr_col, csr_row,
                                                      csr_lft, csr_rgt,
                                                      str_len, str_ptr );

                                        /*
                                         *      Set Next Action Code
                                         *              to Reset Message
                                         */
                                        kjsvpt->nextact = kjsvpt->nextact
                                                          | M_MGRSON;
                                   };
                           };
                        break;

                /*
                 *      Dictionary Registration Mode
                 */
                case A_DCREGA:
                        /*
                         *      Check Current Mode
                         */
                        if ( kjsvpt->kkcrmode == K_NODIC )
                           {
                                /*
                                 *      Call Mode Switching Routine
                                 *              (Dictionary Registration)
                                 */
                                rc = _MM_09( pt );
                           }
                        else if ( pt->code == P_REG )
                           {
                                /*
                                 *      Check Beep Flag
                                 */
                                if ( kjsvpt->kmpf[0].beep == K_BEEPON )
                                        /*
                                         *      Beep Flag Set
                                         */
                                        pt->beep = K_BEEPON;
                           };
                        break;

               };

        /* 1. 3
         *      Return Code.
         */

snap3(SNAP_KCB | SNAP_KMISA,SNAP_MM_rtn,"return _MM_rtn");

        return( rc );
}
