static char sccsid[] = "@(#)21	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicadp.c, cmdKJI, bos411, 9428A410j 7/23/92 01:05:45";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicadp
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
 * MODULE NAME:         kudicadp
 *
 * DESCRIPTIVE NAME:    User Dictionary Data Area Search
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
 * FUNCTION:            User Dictionary Data(Kana & Kanji) Additional function
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        4132 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicadp
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicadp ( mode, kanadata, kanalen,
 *                                 kjdata, kjlen, udcbptr )
 *
 *  INPUT:              mode     :Request Mode ( Inquiry or Registration )
 *                      kanadata :Pointer to Yomi Data (7Bit Code) String
 *                      kanalen  :Length of Yomi Data String
 *                      kjdata   :Pointer to Kanji Data String
 *                      kjlen    :Length of Kanji Data String
 *                      udcbptr  :Pointer to Dictionary Control Block
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IUSUCC  :User Dictionary Add is Successful
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      UDIVPARE:Input Parameter Error
 *                      UDIVHARE:User Dictonary har is Invalid Error
 *                      UDOVFDLE:Insert Entry is Full
 *                      UDDCEXTE:Kanji is Exist Error 
 *                      UDDCFULE:User Dictionary is Full
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              kumvch
 *                              kudeixsc
 *                              kudedtsc
 *                              kuderepl
 *                              kudeadpr
 *                              kudcread
 *                              kudcwrit
 *                      Standard Liblary.
 *                              memcpy
 *                      Advanced Display Graphics Support Liblary(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                              CHPTTOSH:char to short int Data transfer
 *                              SGTOCHPT:char to short int Data transfer
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
/*
 *      include Standard.
 */
#include <stdio.h>           /* Standard I/O Package.                        */
/*#include <memory.h>*/      /* System memory operation uty.                 */

/*
 *      include Kanji Project.
 */
#include "kut.h"                             /* Kanji Utility Define File.   */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int  kudicadp ( mode, kanadata, kanalen, kjdata, kjlen, udcbptr )

short   mode;                /* Request Mode (Inquiry or Registration)       */
uchar   *kanadata;           /* Pointer to Yomi ( 7bit Code ) String         */
short   kanalen;             /* Length of Yomi                               */
uchar   *kjdata;             /* Pointer to Kanji String Data                 */
short   kjlen;               /* Length of Kanji String Data                  */
UDCB    *udcbptr;            /* Pointer to User Dictionary Control Block     */

{
   int     kumvch(),         /* Character Move Function                      */
           kudedtsc();       /* User Dictionary Data Search   Function       */
   void    kudeadpr(),       /* User Dictionary Data Add      Function       */
           kuderepl(),       /* User Dictionary Index Replace Function       */
           kudcread(),       /* User Dictionary File Read     Function       */
           kudcwrit();       /* User Dictionary File Write    Function       */

   uchar   intkjdat[U_KAN_MX],           /* User Dictionary Internal Code.   */
           dicdata[U_REC_L+U_KAN_MX],    /* User Dictionary Read Area.       */
           dicdata1[U_REC_L],            /* User Dictionary Read Area.       */
           dicdata2[U_REC_L],            /* User Dictionary Read Area.       */
           dicdata3[U_REC_L],            /* User Dictionary Read Area.       */
           dicindex[U_INDX_A],           /* User Dictionary Read Area.       */
           buff;                         /* Work Character Area.             */
   long    recrrn[5];                    /* User Dictionary File Position.   */
   int     writfg[5],                    /* Write Flag.                      */
           updtfg,                       /* Update Flag                      */
           indexlen;                     /* Index Length / 1024              */

   short   datapos,                 /* Data  Position    Parm for kudedtsc.  */
           datapos1,                /* Data  Position 1  Parm for kudedtsc.  */
           inspos,                  /* Insert Position.                      */
           inslen,                  /* Insert Length.                        */
           indxpos,                 /* Index Position    Parm for kudeixsc.  */
           indxpos1;                /* Index Position 1  Parm for kudeixsc.  */
   uchar   lastfg,                  /* Last Flag         Parm for kudeixsc.  */
           dataarea[U_REC_L],       /* Insert Data Area.                     */
           data[U_KAN_MX],          /* Insert Index Data Area.               */
           clrstr[U_REC_L];         /* Clear Data Area.                      */
   ushort  il;                      /* Work Active Index Area's Size.        */
   short   har,                     /* Work Highest Allocation RRN.          */
           nar,                     /* Work Next Available RRN.              */
           knl,                     /* Work Yomi Length Value.               */
           rl,                      /* Work rl  Value.                       */
           rl1,                     /* Work rl  Value 1.                     */
           dl,                      /* Work dl  Value.                       */
           shiftlen,                /* Move Length.                          */
           wklen;                   /* Work Yomi Length Move Entry.          */
   uchar   wkana[U_KAN_MX];         /* Work Yomi for Move Entry.             */
   
   int     i,                       /* Work Integer Variable 1.              */
           j,                       /* Work Integer Variable 2.              */
           rc;                      /* Return Code.                          */


                                    /* Input Parameter Check & Initialize    */
   if ( kanalen >= U_KAN_MX )  return( UDIVPARE );
   if ( kjlen   >= U_KNJ_MX )  return( UDIVPARE );

   for ( i=0; i<5; i++ ) writfg[i] = U_FOF;   /* Reset Write Flag            */
   memset ( clrstr,   NULL, U_REC_L );        /* Claer Data Area( NULL Set ) */
   memset ( dicdata,  NULL, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata1, NULL, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata2, NULL, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata3, NULL, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicindex, NULL, U_INDX_A );       /* I/O Area Clear ( NULL Set ) */

   if ( *kanadata != U_OMIT_C )               /* Input Kanji is Mixed Mode   */
      {
      memcpy (intkjdat, kjdata, (int)kjlen);  /* Copy input Kanji to Work    */
                                              /* Kanji Data First Bit Off    */
      for ( i=0; i < ( kjlen - U_1KJ_L ); i += U_1KJ_L )
            intkjdat[i] = intkjdat[i] & U_MSB_O;
      }

   kudcread( udcbptr, 3, 0 );                 /* Data Read Function          */
                                              /* Memory Copy Index Data      */
   memcpy ( dicindex, udcbptr->rdptr, U_INDX_A );
   buff = dicindex[0];                        /* First 2byte Data Chenge     */
   dicindex[0] = dicindex[1];
   dicindex[1] = buff;

                                    /*  Check Index har Value                */
   har = dicindex[U_ILLEN + U_STSLEN];
   indexlen = 0;                    /* Initial Index Recode Length           */
                                    /* Index Recode is 1 Record ( 1 Kbyte )  */
   if ( (har >= U_HAR_V1) && (har <= U_HAR_V2) )  indexlen = 1;
                                    /* Index Recode is 2 Record ( 2 Kbyte )  */
   if ( (har >= U_HAR_V3) && (har <= U_HAR_V4) )  indexlen = 2;
                                    /* Index Recode is 3 Record ( 3 Kbyte )  */
   if ( (har >= U_HAR_V5) && (har <= U_HAR_V6) )  indexlen = 3;
                                    /* Invalid har Value                     */
   if ( indexlen == 0 )  return ( UDIVHARE );

   il = CHPTTOSH(dicindex);
   if ( il == U_IL_HED )
     {
       nar = U_BASNAR + indexlen;
     }
    else {
       nar =  dicindex[U_ILLEN + U_STSLEN + U_HARLEN];
     };

   /*  2.1 Data is Empty Process                                        */
   if ( nar == (U_BASNAR + indexlen) )
     {
        nar++;                                /* nar Count Up & Set          */
        dicindex[U_ILLEN + U_STSLEN + U_HARLEN] = nar;
        knl = kanalen + U_KNLLEN;             /* Calculation of Yomi Length  */
        i   = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;
        dicindex[i] = knl;                    /* Set Yomi Data Length        */
        i += U_KNLLEN;                        /* Memory Copy Yomi Data       */
        memcpy ( &dicindex[i], kanadata, (int)kanalen );
                                              /* Set RRN                     */
        dicindex[kanalen + i] = U_BASNAR + indexlen;
                                              /* Calc INDEX Active Data Size */
        il = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN + knl + U_RRNLEN;
        SHTOCHPT(dicindex, il);               /* Set INDEX Active Data Size  */
        buff = dicindex[0];                   /* First 2byte Data Chenge     */
        dicindex[0] = dicindex[1];
        dicindex[1] = buff;
        udcbptr->wtptr = &dicindex[0];        /* Control Block Data Set      */
        kudcwrit ( udcbptr, 3, NULL );        /* Index Data Write            */

        knl = kanalen + U_KNLLEN;             /* Yomi Data Length Calc       */
        dl  = kjlen   + U_DLLEN + U_RSVLEN;   /* Kanji Data Length Calc      */
        rl  = U_RLLEN + knl + dl;             /* Active Data Size Calc       */
        SHTOCHPT(dicdata, rl);                /* Set Active Data Size        */
        dicdata[U_RLLEN] = knl;               /* Yomi Data Length Set        */
                                              /* Yomi Data Memory Copy       */
        memcpy ( &dicdata[U_RLLEN + U_KNLLEN], kanadata, (int)kanalen );
        dicdata[U_RLLEN + knl] = dl;          /* Kanji Data Length Set       */
        i = U_RLLEN + knl + U_DLLEN;          /* Set Reserve Position        */
        dicdata[i] = U_UDCRSV;                /* Set Reserve Data            */
        i += U_RSVLEN;                        /* Add Reserve Length          */
                                              /* Kanji Data Memory Copy      */
        memcpy ( &dicdata[i], intkjdat, (int)kjlen );
        recrrn[0] = (U_BASNAR + indexlen);    /* Write Record Number Set     */
        buff = dicdata[0];                    /* First 2byte Data Chenge     */
        dicdata[0] = dicdata[1];
        dicdata[1] = buff;
        udcbptr->wtptr = &dicdata[0];         /* Control Block Data Set      */
        kudcwrit ( udcbptr, 4, recrrn[0] );   /* Dictionary Data Write       */

        return( IUSUCC );                     /* Return Process Successful   */
     }

                                    /* Index Area Position Search            */
   kudeixsc ( dicindex, kanadata, kanalen, &indxpos, &indxpos1, &lastfg );

                                    /* Read Data Area                        */
   knl = dicindex[indxpos];                   /* Yomi Data Length Set        */
   recrrn[0] = dicindex[indxpos + knl];       /* Set Record Number           */
   kudcread( udcbptr, 4, recrrn[0] );         /* Dictionary Data Read        */
                                              /* Dictionary Data Memory Copy */
   memcpy ( dicdata, udcbptr->rdptr, U_REC_L );
   buff = dicdata[0];                         /* First 2byte Data Chenge     */
   dicdata[0] = dicdata[1];
   dicdata[1] = buff;
                                    /* Dictionary Data Entry Search          */
   rc = kudedtsc ( dicdata, kanadata, kanalen,
                   intkjdat, kjlen, &datapos, &datapos1 );

                                    /* Get Insert Position                   */
                                    /*   & Insert Length & Set Insert String */
   inspos = datapos;                          /* Insert Position Set         */
   switch ( rc ) 
      {
      case U_NOKAN :                /* Yomi Data is None                     */
                                              /* Insert Length Calculation   */
         inslen      = U_KNLLEN + kanalen + U_DLLEN + U_RSVLEN + kjlen;
         dataarea[0] = kanalen + U_KNLLEN;    /* Set Yomi Data Length        */
                                              /* Set Yomi Data String        */
         memcpy( &(dataarea[U_KNLLEN]), kanadata, (int)kanalen );
         i = kanalen + U_KNLLEN;              /* Set Kanji Insert Position   */
                                              /* Set Kanji Data Length       */
         dataarea[i]           = kjlen + U_DLLEN + U_RSVLEN;
         dataarea[i + U_DLLEN] = U_UDCRSV;    /* Set Reserve Data            */
                                              /* Set Kanji Data String       */
         memcpy( &dataarea[i + U_DLLEN + U_RSVLEN], intkjdat, (int)kjlen );
         break;

      case U_NOKNJ :                /* Yomi Data Exixt & Kanji is None       */
         inslen = U_RSVLEN + kjlen;           /* Insert Length Calculation   */
         knl    = dicdata[datapos1];          /* Get Yomi Data Length        */
         dl     = dicdata[datapos1 + knl];    /* Get Data Length for Kanji   */
                                              /* Entry Over Renge            */
         if ( (dl + inslen) > U_KAN_MX )  return ( UDOVFDLE );
         dicdata[datapos1 + knl] += inslen;   /* Update Data Length for Kanji*/
         dataarea[0] = U_UDCRSV;              /* Set Reserve Data            */
                                              /* Set Input Kanji Data String */
         memcpy ( &(dataarea[U_RSVLEN]), intkjdat, (int)kjlen );
         break;

      case U_KANKNJ :               /* Yomi & Kanji Data Exist               */
         return ( UDDCEXTE );                 /* Error Kanji is Already Exist*/
      }

   rl  = CHPTTOSH(dicdata);                   /* Get Active Data Area Size   */
   har = dicindex[3];                         /* Get Highest Alloc RRN       */
   nar = dicindex[4];                         /* Get Next Available RRN      */

   if (   (mode == U_MODINQ)                  /* Mode is Inquiry             */
       && ( (har >= nar) || ( rl + inslen <= U_REC_L) )  )
            return( IUSUCC );                 /* Data Regist is OK           */

                                    /* Insert Data Process                   */
   if ( rl > datapos )
     {
                                              /* Insert Data Area Move       */
        rc = kumvch ( dicdata, (int)datapos, (int)(rl - datapos), U_BACKWD,
                     (int)inslen, TRUE, clrstr, NULL, (int)inslen );
                                              /* Copy Insert Data            */
        memcpy ( &dicdata[datapos], dataarea, (int)inslen );
     }
   else
                                              /* Insert Data is Last         */
        memcpy ( &dicdata[rl], dataarea, (int)inslen );

   rl += inslen;                              /* Update Active Data Area     */
   SHTOCHPT(dicdata, rl);                     /* Set Active Data Area        */
   writfg[0] = U_FON;                         /* Write Flag On               */

                                    /* INDEX Update Process                  */
   if ( lastfg == U_FON )
     {
        data[0] = kanalen + U_KNLLEN;         /* Set Yomi Data Length        */
        for ( i=0; i<kanalen; i++ )           /* Copy Yomi Data String       */
           data[i + U_KNLLEN] = *(kanadata + i);
        knl = dicindex[indxpos];              /* Get Old Yomi Data Length    */
                                              /* Replace Index Entry         */
        kuderepl( dicindex, indxpos, knl, data, kanalen + U_KNLLEN );
        writfg[4] = U_FON;                    /* Write Flag On               */
     }

                                    /* Overflow Proccess                     */
   if ( rl > U_REC_L )
     {   
        updtfg = U_FOF;                       /* Update Flag Off             */
                                    /* Left Insert Proccess                  */
        if ( indxpos1 < indxpos )
          {
            shiftlen = 0;                     /* Shift Length Clear          */
            for ( i = U_RLLEN; i < rl; i += knl + dl )
              {
                knl = dicdata[i];             /* Get Yomi Data Length        */
                dl  = dicdata[i + knl];       /* Get This Entry Data Length  */
                shiftlen += knl + dl;         /* Update Shift Length         */
                if ( (rl - shiftlen) <  U_REC_L )
                  {
                    wklen = knl;              /* Set Last Yomi Data Length   */
                                              /* Set Last Yomi Data String   */
                    memcpy ( wkana, &dicdata[i], (int)knl );
                    break;
                  }
              }

                                    /* Before Data Read                      */
            knl       = dicindex[indxpos1];   /* Get Yomi Data Length        */
                                              /* Get This Entry Record No    */
            recrrn[1] = dicindex[indxpos1 + knl];
                                              /* User Dictionary Read        */
            kudcread ( udcbptr, 4, recrrn[1] );
                                              /* Copy User Dictionary        */
            memcpy ( dicdata1, udcbptr->rdptr, U_REC_L );
            buff = dicdata1[0];               /* First 2 Byte Chenge         */
            dicdata1[0] = dicdata1[1];
            dicdata1[1] = buff;

                                    /* Data Area Can Moved Check             */
            rl1 = CHPTTOSH(dicdata1);         /* Get Active Data Area Size   */
            if ( (rl1 + shiftlen) <= U_REC_L )
              {                     /* Move Data Area Size OK                */
                                              /* Entry Move for Left         */
                kudeadpr ( U_DADDLF, dicdata, dicdata1, shiftlen );
                knl = dicindex[indxpos1];     /* Get Old Yomi Length         */
                                              /* Replace Index Entry         */
                kuderepl ( dicindex, indxpos1, knl, wkana, wklen );
                writfg[1] = U_FON;            /* Write Flag On               */
                writfg[4] = U_FON;            /* Write Flag On               */
                updtfg = U_FON;               /* Update Flag On              */
              }
          }

        knl = dicindex[indxpos];              /* Get Index Yomi Data Length  */
        il  = CHPTTOSH(dicindex);             /* Get Index Active Data Size  */

                                    /* Right Insert Process                  */
        if ( (indxpos + knl + U_RRNLEN) < il && updtfg == U_FOF )
          {
            i = j = U_RLLEN;                  /* Initialize Move Position    */
            while ( i < rl )
              {
                knl = dicdata[i];             /* Get Yomi Data Length        */
                dl  = dicdata[i + knl];       /* Get This Entry Data Length  */
                if ( ( i + knl + dl ) > U_REC_L )
                  {
                    shiftlen = rl - i;        /* Set Shift Length            */
                    wklen = dicdata[j];       /* Get Last Yomi Data Length   */
                                              /* Get Last Yomi Data String   */
                    memcpy ( wkana, &dicdata[j], (int)wklen );
                    break;
                  }
                 j = i;                       /* Previous Position Set       */
                 i += knl + dl;               /* 1 Entry Length Add          */
              }
                                    /* After Data Read                       */
            knl = dicindex[indxpos];          /* Get Yomi Data Length        */
            i   = indxpos + knl + U_RRNLEN;   /* Next Entry Position Set     */
            knl = dicindex[i];                /* Get Next Yomi Data Length   */
            recrrn[2] = dicindex[i + knl];    /* Get Record Number           */
                                              /* User Dictionary Read        */
            kudcread ( udcbptr, 4, recrrn[2] );
                                              /* Copy User Dictionary        */
            memcpy ( dicdata2, udcbptr->rdptr, U_REC_L );
            buff = dicdata2[0];               /* First 2 Byte Chenge         */
            dicdata2[0] = dicdata2[1];
            dicdata2[1] = buff;
                                    /* Check Data Area Can be Moved          */
            rl1 = CHPTTOSH(dicdata2);
            if ( (rl1 + shiftlen) <= U_REC_L )
              {                     /* Move Data Area Size OK                */
                                              /* Entry Move for Right        */
                kudeadpr ( U_DADDRT, dicdata, dicdata2, shiftlen );
                knl = dicindex[indxpos];      /* Get Yomi Data Length        */
                                              /* Replace Index Entry         */
                kuderepl ( dicindex, indxpos, knl, wkana, wklen );
                writfg[2] = U_FON;            /* Write Flag On               */
                writfg[4] = U_FON;            /* Write Flag On               */
                updtfg = U_FON;               /* Update Flag On              */
              }
          }

                                    /* Add New Data Record                   */
        if ( nar <= har && updtfg == U_FOF )
          {
            rl = CHPTTOSH(dicdata);           /* Get Active Data Area Size   */
            shiftlen = 0;                     /* Shift Length Initialize     */
            for ( i = U_RLLEN; i < rl; i += knl + dl )
              {
                knl = dicdata[i];             /* Get Yomi Data Length        */
                dl  = dicdata[i + knl];       /* Get This Entry Data Length  */
                shiftlen += knl + dl;         /* Update Shift Data Length    */
                if ( shiftlen > ( rl * U_SPRF / U_SPRD ) )
                  {
                    wklen = knl;              /* Get Last Yomi Data Length   */
                                              /* Get Last Yomi Data String   */
                    memcpy ( wkana, &dicdata[i], (int)knl );
                    break;
                  }
              }
                                              /* Entry Move for New          */
            kudeadpr ( U_DADDNW, dicdata, dicdata3, shiftlen );
                                    /* Set Index Data                        */
            wkana[wklen] = nar;               /* Record No Set               */
            wklen += U_RRNLEN;                /* RRN Length Add              */
                                    /* Insert Index Data                     */
            il = CHPTTOSH(dicindex);          /* Get Index Active Length     */
            if ( il > indxpos )
              {                               /* Move Char Data Area         */
                rc = kumvch(dicindex, (int)indxpos, (int)(il - indxpos),
                        U_BACKWD, (int)wklen, TRUE, clrstr, NULL, (int)wklen );
                                              /* Data Copy Insert Entry      */
               memcpy ( &dicindex[indxpos], wkana, (int)wklen );
              }
            else
                                              /* Insert Entry is Last        */
               memcpy ( &dicindex[il], wkana, (int)wklen );

            recrrn[3] = nar;                  /* Write Record No Set         */
            nar++;                            /* Update Next Available RRN   */
                                              /* Set Next Available RRN      */
            dicindex[U_ILLEN + U_STSLEN + U_HARLEN] = nar;
            il += wklen;                      /* Update Index Active Length  */
            SHTOCHPT(dicindex, il);           /* Set Index Active Length     */
            writfg[3] = U_FON;                /* Write Flag On               */
            writfg[4] = U_FON;                /* Write Flag On               */
            updtfg = U_FON;                   /* Update Flag On              */
         }
                                    /* No Data Add Space                     */
       if ( updtfg == U_FOF )  return( UDDCFULE );
     }
                                    /* Write User Dictionary                 */
   if ( mode != U_MODINQ )
     {                                        /* Mode is Registration        */
       if ( writfg[1] == U_FON )
         {
            buff = dicdata1[0];               /* First 2 Byte Chenge         */
            dicdata1[0] = dicdata1[1];
            dicdata1[1] = buff;
            udcbptr->wtptr = &dicdata1[0];    /* Set Write Data Address      */
                                              /* User Dictionary Write       */
            kudcwrit ( udcbptr, 4, recrrn[1] );
         }
       if ( writfg[2] == U_FON )
         {
           buff = dicdata2[0];                /* First 2 Byte Chenge         */
           dicdata2[0] = dicdata2[1];
           dicdata2[1] = buff;
           udcbptr->wtptr = &dicdata2[0];     /* Set Write Data Address      */
                                              /* User Dictionary Write       */
           kudcwrit ( udcbptr, 4, recrrn[2] );
         }
       if ( writfg[3] == U_FON )
         {
           buff = dicdata3[0];                /* First 2 Byte Chenge         */
           dicdata3[0] = dicdata3[1];
           dicdata3[1] = buff;
           udcbptr->wtptr = &dicdata3[0];     /* Set Write Data Address      */
                                              /* User Dictionary Write       */
           kudcwrit ( udcbptr, 4, recrrn[3] );
         }
       if ( writfg[0] == U_FON )
         {
           buff = dicdata[0];                 /* First 2 Byte Chenge         */
           dicdata[0] = dicdata[1];
           dicdata[1] = buff;
           udcbptr->wtptr = &dicdata[0];      /* Set Write Data Address      */
                                              /* User Dictionary Write       */
           kudcwrit ( udcbptr, 4, recrrn[0] );
         }
       if ( writfg[4] == U_FON )
         {
           buff = dicindex[0];                /* First 2 Byte Chenge         */
           dicindex[0] = dicindex[1];
           dicindex[1] = buff;
           udcbptr->wtptr = &dicindex[0];     /* Set Write Data Address      */
                                              /* User Dictionary Index Write */
           kudcwrit ( udcbptr, 3, NULL );
         }
     }

   return( IUSUCC );                          /* Process is Successful       */
}
