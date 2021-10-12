static char sccsid[] = "@(#)36	1.2.1.3  src/bos/usr/lib/nls/loc/jim/jkkc/_Kclcadp.c, libKJI, bos411, 9428A410j 6/8/94 23:28:05";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kclcadp
 *
 * DESCRIPTIVE NAME:  User Dictionary Data Area Search
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)     : success
 *                    0x0310(USRDCT_OVER) : no free area in user dict
 *                    0x0a10(UPDATING)    : being updated
 *                    0x0b10(RQ_RECOVER)  : request recovery of user dict.
 *                    0x0582(USR_LSEEK)   : error of lseek()
 *                    0x0682(USR_READ)    : error of read()
 *                    0x0782(USR_WRITE)   : error of write()
 *                    0x0882(USR_LOCKF)   : error of lockf()
 *                    0x7fff(UERROR)      : unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
short   _Kclcadp(kcbptr, mode, kanadata, kanalen,kjdata,kjlen )
struct  KCB     *kcbptr;        /* pointer of kcb                       */
short   mode;                   /* Request Mode (Inquiry or Registration)*/
uschar  *kanadata;              /* Pointer to Yomi ( 7bit Code ) String */
short   kanalen;                /* Length of Yomi                       */
uschar  *kjdata;                /* Pointer to Kanji String Data         */
short   kjlen;                  /* Length of Kanji String Data          */

{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kclmvch();   /* Move Character in a Dimemsion*/
   extern short           _Kcldtsc();   /* User Dict. Data Area Search  */
   extern void            _Kcladpr();   /* Check User Dictionary and Add*/
   extern void            _Kclrepl();   /* Index Area Data Replace      */
   extern short           _Kczread();   /* Read The Dictionaries    (TS)*/
   extern short           _Kczwrit();   /* Write The Dictionaries   (TS)*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   short           z_rzread;    /* Define Area for Return of _Kczread   */
   short           z_rzwrit;    /* Define Area for Return of _Kczwrit   */

   uschar  intkjdat[U_KAN_MX],          /* User Dictionary Internal Code*/
           dicdata[U_REC_L+U_KAN_MX],   /* User Dictionary Read Area.   */
           dicdata1[U_REC_L],           /* User Dictionary Read Area.   */
           dicdata2[U_REC_L],           /* User Dictionary Read Area.   */
           dicdata3[U_REC_L],           /* User Dictionary Read Area.   */
           dicindex[U_INDX_A],          /* User Dictionary Read Area.   */
           buff;                        /* Work Character Area.         */
   long    recrrn[5];                   /* User Dictionary File Position*/
   int     writfg[5],                   /* Write Flag.                  */
           updtfg,                      /* Update Flag                  */
           indexlen;                    /* Index Length / 1024          */

   short   datapos,               /* Data  Position    Parm for _Kcldtsc*/
           datapos1,              /* Data  Position 1  Parm for _Kcldtsc*/
           inspos,                /* Insert Position.                   */
           inslen,                /* Insert Length.                     */
           indxpos,               /* Index Position    Parm for _Kclixsc*/
           indxpos1;              /* Index Position 1  Parm for _Kclixsc*/
   uschar  lastfg,                /* Last Flag         Parm for _Kclixsc*/
           dataarea[U_REC_L],     /* Insert Data Area.                  */
           data[U_KAN_MX],        /* Insert Index Data Area.            */
           clrstr[U_REC_L];       /* Clear Data Area.                   */
   usshort il;                   /* Work Active Index Area's Size.     */
   short   har,                   /* Work Highest Allocation RRN.       */
           nar,                   /* Work Next Available RRN.           */
           knl,                   /* Work Yomi Length Value.            */
           rrn,                   /* Work rrn Value.                    */
           rl,                    /* Work rl  Value.                    */
           rl1,                   /* Work rl  Value 1.                  */
           dl,                    /* Work dl  Value.                    */
           shiftlen,              /* Move Length.                       */
           wklen;                 /* Work Yomi Length Move Entry.       */
   uschar  wkana[U_KAN_MX];       /* Work Yomi for Move Entry.          */
   
   int     i,                     /* Work Integer Variable 1.           */
           j,                     /* Work Integer Variable 2.           */
           k;                     /* Work Integer Variable 3.           */
   short   rc;                    /* Return Code.                       */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = kcbptr;
                               /* Input Parameter Check & Initialize    */
   if ( kanalen >= U_KAN_MX )  return( UERROR );
   if ( kjlen   >= U_KNJ_MX )  return( UERROR );

   for ( i=0; i<5; i++ ) writfg[i] = U_FOF;/* Reset Write Flag          */
   memset ( clrstr,   NULL, U_REC_L );   /* Claer Data Area( NULL Set ) */
   memset ( dicdata,  NULL, U_REC_L );   /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata1, NULL, U_REC_L );   /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata2, NULL, U_REC_L );   /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata3, NULL, U_REC_L );   /* I/O Area Clear ( NULL Set ) */
   memset ( dicindex, NULL, U_INDX_A );  /* I/O Area Clear ( NULL Set ) */

   if ( *kanadata != U_OMIT_C )          /* Input Kanji is Mixed Mode   */
      {
      memcpy (intkjdat, kjdata, (int)kjlen);/* Copy input Kanji to Work */
                                         /* Kanji Data First Bit Off    */
      for ( i=0; i < ( kjlen - U_1KJ_L ); i += U_1KJ_L )
            intkjdat[i] = intkjdat[i] & U_MSB_O;
      }
                                        /* data read function           */
   z_rzread = _Kczread( kcbptr, UX, 0 , EXCLUSIVE, 0 );
                                        /* Memory Copy Index Data       */
   if (z_rzread != SUCCESS )            /* Error Process                */
      return(z_rzread);              /* Return with Error code       */


   memcpy ( dicindex, kcb.uxeuxe, (int)(U_INDX_A) );
   buff = dicindex[0];                  /* First 2byte Data Chenge      */
   dicindex[0] = dicindex[1];
   dicindex[1] = buff;

                                        /*  Check Index har Value       */
   har = dicindex[U_ILLEN + U_STSLEN];
   indexlen = 0;                        /* Initial Index Recode Length  */
                                     /* Index Recode is 1 Record(1Kbyte)*/
   if ( (har >= U_HAR_V1) && (har <= U_HAR_V2) )  indexlen = 1;
                                     /* Index Recode is 2 Record(2Kbyte) */
   if ( (har >= U_HAR_V3) && (har <= U_HAR_V4) )  indexlen = 2;
                                     /* Index Recode is 3 Record(3Kbyte) */
   if ( (har >= U_HAR_V5) && (har <= U_HAR_V6) )  indexlen = 3;
                                     /* Invalid har Value                */
   if ( indexlen == 0 )  return ( RQ_RECOVER );

   il = CHPTTOSH(dicindex);
   if ( il == U_IL_HED )
   {
      nar = U_BASNAR + indexlen;
   }
   else
   {
      nar =  dicindex[U_ILLEN + U_STSLEN + U_HARLEN];
   };

/*----------------------------------------------------------------------*  
 *      2.1 Data is Empty Process
 *----------------------------------------------------------------------*/ 
   if ( nar == (U_BASNAR + indexlen) )
   {
      nar++;                            /* nar Count Up & Set           */
      dicindex[U_ILLEN + U_STSLEN + U_HARLEN] = nar;
      knl = kanalen + U_KNLLEN;         /* Calculation of Yomi Length   */
      i   = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;
      dicindex[i] = knl;                /* Set Yomi Data Length         */
      i += U_KNLLEN;                    /* Memory Copy Yomi Data        */
      memcpy ( &dicindex[i], kanadata, (int)kanalen );
                                        /* Set RRN                      */
      dicindex[kanalen + i] = U_BASNAR + indexlen;
                                        /* Calc INDEX Active Data Size  */
      il = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN + knl + U_RRNLEN;
      SHTOCHPT(dicindex, il);           /* Set INDEX Active Data Size   */
      buff = dicindex[0];               /* First 2byte Data Chenge      */
      dicindex[0] = dicindex[1];
      dicindex[1] = buff;

      kcb.uxeuxe = (struct UXE *)&dicindex[0];
                                        /* Index data write       */
      z_rzwrit = _Kczwrit( kcbptr, UX, NULL, (short)EXCLUSIVE  );
                                        /* Memory Copy Index Data       */
      if (z_rzwrit != SUCCESS )         /* Error Process                */
         return(z_rzwrit);              /* Return with Error code       */

      knl = kanalen + U_KNLLEN;         /* Yomi Data Length Calc        */
      dl  = kjlen   + U_DLLEN + U_RSVLEN;/* Kanji Data Length Calc      */
      rl  = U_RLLEN + knl + dl;         /* Active Data Size Calc        */
      SHTOCHPT(dicdata, rl);            /* Set Active Data Size         */
      dicdata[U_RLLEN] = knl;           /* Yomi Data Length Set         */
                                        /* Yomi Data Memory Copy        */
      memcpy ( &dicdata[U_RLLEN + U_KNLLEN], kanadata, (int)kanalen );
      dicdata[U_RLLEN + knl] = dl;      /* Kanji Data Length Set        */
      i = U_RLLEN + knl + U_DLLEN;      /* Set Reserve Position         */
      dicdata[i] = U_UDCRSV;            /* Set Reserve Data             */
      i += U_RSVLEN;                    /* Add Reserve Length           */
                                        /* Kanji Data Memory Copy       */
      memcpy ( &dicdata[i], intkjdat, (int)kjlen );
      recrrn[0] = (U_BASNAR + indexlen);/* Write Record Number Set      */
      buff = dicdata[0];                /* First 2byte Data Chenge      */
      dicdata[0] = dicdata[1];
      dicdata[1] = buff;
      kcb.udeude = ( uschar *)&dicdata[0];
                                        /* control block data set       */
      z_rzwrit = _Kczwrit( kcbptr, UD, recrrn[0], (short)EXCLUSIVE );
                                        /* Memory Copy Index Data       */
      if (z_rzwrit != SUCCESS )         /* Error Process                */
         return(z_rzwrit);              /* Return with Error code       */

      return( SUCCESS );                /* Return Process Successful    */
   }

                                        /* Index Area Position Search   */
   _Kclixsc ( dicindex, kanadata, kanalen, &indxpos, &indxpos1, &lastfg );

                                        /* Read Data Area               */
   knl = dicindex[indxpos];             /* Yomi Data Length Set         */
   recrrn[0] = dicindex[indxpos + knl]; /* Set Record Number            */
                                        /* Data read function   */
   z_rzread = _Kczread( kcbptr, UD, recrrn[0], EXCLUSIVE, 0 );
   if (z_rzread != SUCCESS )            /* Error Process                */
      return(z_rzread);                 /* Return with Error code       */
                                        /* Dictionary Data Memory Copy  */
   memcpy ( dicdata, kcb.udeude, (int)(U_REC_L) );
   EMORAto7bit(dicdata);
   buff = dicdata[0];                   /* First 2byte Data Chenge      */
   dicdata[0] = dicdata[1];
   dicdata[1] = buff;

                                        /* Dictionary Data Entry Search */
   rc = _Kcldtsc ( dicdata, kanadata, kanalen,
                   intkjdat, kjlen, &datapos, &datapos1 );
                                        /* Get Insert Position          */
                                        /* & Insert Length              */
                                        /* & Set Insert String          */
   inspos = datapos;                    /* Insert Position Set          */
   switch ( rc ) 
   {
      case U_NOKAN :                    /* Yomi Data is None            */
                                        /* Insert Length Calculation    */
         inslen      = U_KNLLEN + kanalen + U_DLLEN + U_RSVLEN + kjlen;
         dataarea[0] = kanalen + U_KNLLEN;/* Set Yomi Data Length       */
                                         /* Set Yomi Data String        */
         memcpy( &(dataarea[U_KNLLEN]), kanadata, (int)kanalen );
         i = kanalen + U_KNLLEN;         /* Set Kanji Insert Position   */
                                         /* Set Kanji Data Length       */
         dataarea[i]           = kjlen + U_DLLEN + U_RSVLEN;
         dataarea[i + U_DLLEN] = U_UDCRSV;/* Set Reserve Data           */
                                         /* Set Kanji Data String       */
         memcpy( &dataarea[i + U_DLLEN + U_RSVLEN], intkjdat, (int)kjlen );
         break;

      case U_NOKNJ :                /* Yomi Data Exixt & Kanji is None  */
         inslen = U_RSVLEN + kjlen;     /* Insert Length Calculation    */
         knl    = dicdata[datapos1];    /* Get Yomi Data Length         */
         dl     = dicdata[datapos1 + knl];/* Get Data Length for Kanji  */
                                        /* Entry Over Renge             */
         if ( (dl + inslen) > U_KAN_MX )  return ( USRREC_OVER );
         dicdata[datapos1 + knl] += inslen;/*Update Data Length for Kanji*/
         dataarea[0] = U_UDCRSV;        /* Set Reserve Data             */
                                        /* Set Input Kanji Data String  */
         memcpy ( &(dataarea[U_RSVLEN]), intkjdat, (int)kjlen );
         break;

      case U_KANKNJ :                   /* Yomi & Kanji Data Exist      */
         return ( EXIST_USR );          /* Error Kanji is Already Exist */

      default       :
         return(rc);
   }

   rl  = CHPTTOSH(dicdata);             /* Get Active Data Area Size    */
   har = dicindex[3];                   /* Get Highest Alloc RRN        */
   nar = dicindex[4];                   /* Get Next Available RRN       */

   if (   (mode == U_MODINQ)            /* Mode is Inquiry              */
       && ( (har >= nar) || ( rl + inslen <= U_REC_L) )  )
            return( SUCCESS );          /* Data Regist is OK            */

                                        /* Insert Data Process          */
   if ( rl > datapos )
   {
                                        /* Insert Data Area Move        */
        _Kclmvch ( dicdata, (int)datapos, (int)(rl - datapos), U_BACKWD,
                     (int)inslen, TRUE, clrstr, NULL, (int)inslen );
                                        /* Copy Insert Data             */
        memcpy ( &dicdata[datapos], dataarea, (int)inslen );
   }
   else
                                        /* Insert Data is Last          */
        memcpy ( &dicdata[rl], dataarea, (int)inslen );
   rl += inslen;                        /* Update Active Data Area      */
   SHTOCHPT(dicdata, rl);               /* Set Active Data Area         */
   writfg[0] = U_FON;                   /* Write Flag On                */

                                    /* INDEX Update Process             */
   if ( lastfg == U_FON )
   {
      data[0] = kanalen + U_KNLLEN;     /* Set Yomi Data Length         */
      for ( i=0; i<kanalen; i++ )       /* Copy Yomi Data String        */
         data[i + U_KNLLEN] = *(kanadata + i);
      knl = dicindex[indxpos];          /* Get Old Yomi Data Length     */
                                        /* Replace Index Entry          */
      _Kclrepl( dicindex, indxpos, knl, data, kanalen + U_KNLLEN );
      writfg[4] = U_FON;                /* Write Flag On                */
   }

                                    /* Overflow Proccess                */
   if ( rl > U_REC_L )
   {
      updtfg = U_FOF;                   /* Update Flag Off             */
                                  /* Left Insert Proccess                  */
      if ( indxpos1 < indxpos )
      {
         shiftlen = 0;                 /* Shift Length Clear            */
         for ( i = U_RLLEN; i < rl; i += knl + dl )
         {
            knl = dicdata[i];           /* Get Yomi Data Length         */
            dl  = dicdata[i + knl];     /* Get This Entry Data Length   */
            shiftlen += knl + dl;       /* Update Shift Length          */
            if ( (rl - shiftlen) <  U_REC_L )
            {
               wklen = knl;             /* Set Last Yomi Data Length    */
                                        /* Set Last Yomi Data String    */
               memcpy ( wkana, &dicdata[i], (int)knl );
               break;
            }
         }

                                 /* Before Data Read                    */
         knl       = dicindex[indxpos1];/* Get Yomi Data Length         */
                                        /* Get This Entry Record No     */
         recrrn[1] = dicindex[indxpos1 + knl];
                                        /* User Dictionary Read         */
         z_rzread = _Kczread( kcbptr, UD, recrrn[1], EXCLUSIVE, 0 );
         if (z_rzread != SUCCESS )      /* Error Process                */
            return(z_rzread);           /* Return with Error code       */
                                        /* Copy User Dictionary         */
         memcpy ( dicdata1, kcb.udeude , (int)(U_REC_L) );
         buff = dicdata1[0];            /* First 2 Byte Chenge          */
         dicdata1[0] = dicdata1[1];
         dicdata1[1] = buff;

                                 /* Data Area Can Moved Check           */
         rl1 = CHPTTOSH(dicdata1);      /* Get Active Data Area Size    */
         if ( (rl1 + shiftlen) <= U_REC_L )
         {                              /* Move Data Area Size OK       */
                                        /* Entry Move for Left          */
            _Kcladpr ( U_DADDLF, dicdata, dicdata1, shiftlen );
            knl = dicindex[indxpos1];   /* Get Old Yomi Length          */
                                        /* Replace Index Entry          */
            _Kclrepl ( dicindex, indxpos1, knl, wkana, wklen );
            writfg[1] = U_FON;          /* Write Flag On                */
            writfg[4] = U_FON;          /* Write Flag On                */
            updtfg = U_FON;             /* Update Flag On               */
         }
      }

      knl = dicindex[indxpos];          /* Get Index Yomi Data Length   */
      il  = CHPTTOSH(dicindex);         /* Get Index Active Data Size   */

                                  /* Right Insert Process               */
      if ( (indxpos + knl + U_RRNLEN) < il && updtfg == U_FOF )
      {
         i = j = U_RLLEN;               /* Initialize Move Position     */
         while ( i < rl )
         {
            knl = dicdata[i];           /* Get Yomi Data Length         */
            dl  = dicdata[i + knl];     /* Get This Entry Data Length   */
            if ( ( i + knl + dl ) > U_REC_L )
            {
               shiftlen = rl - i;       /* Set Shift Length             */
               wklen = dicdata[j];      /* Get Last Yomi Data Length    */
                                        /* Get Last Yomi Data String    */
               memcpy ( wkana, &dicdata[j], (int)wklen );
               break;
            }
            j = i;                      /* Previous Position Set        */
            i += knl + dl;              /* 1 Entry Length Add           */
         }
                                 /* After Data Read                     */
         knl = dicindex[indxpos];       /* Get Yomi Data Length         */
         i   = indxpos + knl + U_RRNLEN;/* Next Entry Position Set      */
         knl = dicindex[i];             /* Get Next Yomi Data Length    */
         recrrn[2] = dicindex[i + knl]; /* Get Record Number            */
                                        /* User Dictionary Read         */
         z_rzread = _Kczread ( kcbptr, UD, recrrn[2], 0 );/* Data read  */
         if (z_rzread != SUCCESS)       /* Error Process                */
            return(z_rzread);           /* Return with Error code       */
                                        /* Copy User Dictionary         */
         memcpy ( dicdata2, kcb.udeude, (int)(U_REC_L) );
         buff = dicdata2[0];            /* First 2 Byte Chenge          */
         dicdata2[0] = dicdata2[1];
         dicdata2[1] = buff;
                                 /* Check Data Area Can be Moved        */
         rl1 = CHPTTOSH(dicdata2);
         if ( (rl1 + shiftlen) <= U_REC_L )
         {                     /* Move Data Area Size OK                */
                                        /* Entry Move for Right         */
            _Kcladpr ( U_DADDRT, dicdata, dicdata2, shiftlen );
            knl = dicindex[indxpos];    /* Get Yomi Data Length         */
                                        /* Replace Index Entry          */
            _Kclrepl ( dicindex, indxpos, knl, wkana, wklen );
            writfg[2] = U_FON;          /* Write Flag On                */
            writfg[4] = U_FON;          /* Write Flag On                */
            updtfg = U_FON;             /* Update Flag On               */
         }
      }

                                  /* Add New Data Record                */
      if ( nar <= har && updtfg == U_FOF )
      {
         rl = CHPTTOSH(dicdata);        /* Get Active Data Area Size    */
         shiftlen = 0;                  /* Shift Length Initialize      */
         for ( i = U_RLLEN; i < rl; i += knl + dl )
         {
            knl = dicdata[i];           /* Get Yomi Data Length         */
            dl  = dicdata[i + knl];     /* Get This Entry Data Length   */
            shiftlen += knl + dl;       /* Update Shift Data Length     */
            if ( shiftlen > ( rl * U_SPRF / U_SPRD ) )
            {
               wklen = knl;             /* Get Last Yomi Data Length    */
                                        /* Get Last Yomi Data String    */
               memcpy ( wkana, &dicdata[i], (int)knl );
               break;
            }
         }
                                        /* Entry Move for New           */
         _Kcladpr ( U_DADDNW, dicdata, dicdata3, shiftlen );
                                 /* Set Index Data                      */
         wkana[wklen] = nar;            /* Record No Set                */
         wklen += U_RRNLEN;             /* RRN Length Add               */
                                 /* Insert Index Data                   */
         il = CHPTTOSH(dicindex);       /* Get Index Active Length      */
         if ( il > indxpos )
         {                              /* Move Char Data Area          */
            _Kclmvch(dicindex, (int)indxpos, (int)(il - indxpos),
                    U_BACKWD, (int)wklen, TRUE, clrstr, NULL, (int)wklen );
                                        /* Data Copy Insert Entry       */
            memcpy ( &dicindex[indxpos], wkana, (int)wklen );
         }
         else
                                        /* Insert Entry is Last         */
            memcpy ( &dicindex[il], wkana, (int)wklen );

         recrrn[3] = nar;               /* Write Record No Set          */
         nar++;                         /* Update Next Available RRN    */
                                        /* Set Next Available RRN       */
         dicindex[U_ILLEN + U_STSLEN + U_HARLEN] = nar;
         il += wklen;                   /* Update Index Active Length   */
         SHTOCHPT(dicindex, il);        /* Set Index Active Length      */
         writfg[3] = U_FON;             /* Write Flag On                */
         writfg[4] = U_FON;             /* Write Flag On                */
         updtfg = U_FON;                /* Update Flag On               */
      }
                                /* No Data Add Space                    */
      if ( updtfg == U_FOF )  return( USRDCT_OVER );
   }
                                /* Write User Dictionary                */
   if ( mode != U_MODINQ )
   {                                    /* Mode is Registration         */
      if ( writfg[1] == U_FON )
      {
         buff = dicdata1[0];            /* First 2 Byte Chenge          */
         dicdata1[0] = dicdata1[1];
         dicdata1[1] = buff;
                                        /* User Dictionary Write        */
         kcb.udeude = ( uschar *)&dicdata1[0];/*       */
         z_rzwrit = _Kczwrit( kcbptr, UD, recrrn[1], (short)EXCLUSIVE );
                                        /* Memory Copy Index Data       */
         if (z_rzwrit != SUCCESS )      /* Error Process                */
            return(z_rzwrit);           /* Return with Error code       */
      }
      if ( writfg[2] == U_FON )
      {
         buff = dicdata2[0];            /* First 2 Byte Chenge          */
         dicdata2[0] = dicdata2[1];
         dicdata2[1] = buff;
                                        /* User Dictionary Write        */
         kcb.udeude = ( uschar *)&dicdata2[0];
         z_rzwrit = _Kczwrit( kcbptr, UD, recrrn[2], (short)EXCLUSIVE );
                                        /* Memory Copy Index Data       */
         if (z_rzwrit != SUCCESS )      /* Error Process                */
            return(z_rzwrit);           /* Return with Error code       */
      }
      if ( writfg[3] == U_FON )
      {
         buff = dicdata3[0];            /* First 2 Byte Chenge          */
         dicdata3[0] = dicdata3[1];
         dicdata3[1] = buff;
         kcb.udeude = ( uschar *)&dicdata3[0];
                                        /* User Dictionary Write        */
         z_rzwrit = _Kczwrit( kcbptr, UD, recrrn[3], (short)EXCLUSIVE );
                                        /* Memory Copy Index Data       */
         if (z_rzwrit != SUCCESS )      /* Error Process                */
            return(z_rzwrit);           /* Return with Error code       */
      }
      if ( writfg[0] == U_FON )
      {
         buff = dicdata[0];             /* First 2 Byte Chenge          */
         dicdata[0] = dicdata[1];
         dicdata[1] = buff;
         kcb.udeude = ( uschar *)&dicdata[0];
                                        /* User Dictionary Write        */
         z_rzwrit = _Kczwrit( kcbptr, UD, recrrn[0], (short)EXCLUSIVE );
                                        /* Memory Copy Index Data       */
         if (z_rzwrit != SUCCESS )      /* Error Process                */
            return(z_rzwrit);           /* Return with Error code       */
      }
      if ( writfg[4] == U_FON )
      {
         buff = dicindex[0];            /* First 2 Byte Chenge          */
         dicindex[0] = dicindex[1];
         dicindex[1] = buff;
         kcb.uxeuxe = ( struct UXE *)&dicindex[0];
                                        /* User Dictionary Index Write  */
         z_rzwrit = _Kczwrit( kcbptr, UX, NULL , (short)EXCLUSIVE );
                                        /* Memory Copy Index Data       */
         if (z_rzwrit != SUCCESS )      /* Error Process                */
            return(z_rzwrit);           /* Return with Error code       */
      }
   }

   return( SUCCESS );                   /* Process is Successful       */
}
