static char sccsid[] = "@(#)26	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicdlp.c, cmdKJI, bos411, 9428A410j 7/23/92 01:09:58";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicdlp
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

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kudicdlp
 *
 * DESCRIPTIVE NAME:    Kanji User Dictionary Delete Process
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
 * FUNCTION:            Kanji User Dictionary Delete Process
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        2388 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicdlp
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicdlp( mode, kanadata, kanalen, pos, udcbptr )
 *
 *  INPUT:              mode     :  Delete Mode (Yomi Delete or Kanji Delete)
 *                      kanadata :  Pointer to Yomi Data
 *                      kanalen  :  Yomi Data Length
 *                      pos      :  Kanji Position
 *                      udcbptr  :  Pointer to User Dictionary Control Block
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kumvch,   kudeixsc, kudedtsc,
 *                              kuderepl, kudcread, kudcwrit
 *
 *                      Standard Liblary.
 *                              memcmp, memcpy
 *
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
 *                              CHPTTOSH:char to short int transfer.
 *                              SHTOCHPT:short int to char transfer
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************/

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /*                                              */
/*
 *      include Kanji Project.
 */
#include "kut.h"        /* Kanji user dictionary utility                */
/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
void    kudicdlp ( mode, kanadata, kanalen, pos, udcbptr )

short    mode;          /* Delete Mode                                       */
uchar   *kanadata;      /* Yomi Data                                         */
short    kanalen;       /* Yomi Data Length                                  */
uchar    pos;           /* Kanji Position                                    */
UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block          */

{
    void    kumvch();               /* Character Move           Function     */
    void    kudeixsc();             /* Dictionary Index Search  Function     */
    void    kudedtsc();             /* Dictionary Data  Search  Function     */
    void    kuderepl();             /* Dictionary Index Replace Function     */
    void    kudcread();             /* Dictionary Data Read     Function     */
    void    kudcwrit();             /* Dictionary Data Write    Function     */

    uchar   dicindex[U_INDX_A],     /* Dictionary Index Save Area            */
            dicdata[U_REC_L],       /* Dictionary Data Save Area             */
            clrstr[U_REC_L],        /* Clear String Data Area                */
            buff,                   /* Work Buffer Area                      */
            kjdata,                 /* Kanji Data ( Dummy )                  */
            *kjdt,                  /* Pointer to Kanji Data ( Dummy )       */
            lastfg;                 /* Last Entry Flag (ON==Last or OFF)     */
    short   il,                     /* Active Index Area's Size.             */
            nar,                    /* Next Available RRN.                   */
            rrn,                    /* Relative Record Number.               */
            wk_rrn,                 /* Relative Record Number.               */
            indxpos,                /* Index Entry Position                  */
            indxpos1,               /* Index Entry Position (Previous)       */
            rl,                     /* Active Data's Size.                   */
            knl,                    /* Length of Yomi                        */
            dl,                     /* Data Length                           */
            kjlen,                  /* Kanji Data Length                     */
            datapos,                /* Kanji Entry Position                  */
            datapos1,               /* Kanji Entry Position (Previous)       */
            del_f;                  /* Kanji Entry Delete Flag               */
    int     i,                      /* Loop Counter.                         */
            dlen,                   /* Data Delete Length                    */
            dpos,                   /* Data Delete Position                  */
            ddst;                   /* Data Delete Distance                  */

    /*
     *      Initialize
     */
    for ( i=0; i < U_REC_L; i++ )  clrstr[i] = NULL;

    /*
     *      Dictinary Index & Data Read Process
     */
    kudcread ( udcbptr, 3, NULL );            /* Read Dictionary Index       */
    memcpy   ( dicindex, udcbptr->rdptr, U_INDX_A );
    buff = dicindex[0];                       /* First 2byte Data Chenge     */
    dicindex[0] = dicindex[1];
    dicindex[1] = buff;
    il = CHPTTOSH(dicindex);                  /* Get Active Index Area Size  */
                                              /* Get Next Available RRN      */
    nar = dicindex[U_ILLEN + U_STSLEN + U_HARLEN];
                                    /* Search for Yomi in Dictionary Index   */
    kudeixsc( dicindex, kanadata, kanalen, &indxpos, &indxpos1, &lastfg );
    knl = dicindex[indxpos];                  /* Get Yomi Data Length        */
    rrn = dicindex[indxpos + knl];            /* Get Target Record Number    */

    kudcread ( udcbptr, 4, rrn );             /* Read Target RRN Data        */
    memcpy   ( dicdata, udcbptr->rdptr, U_REC_L );
    buff = dicdata[0];                        /* First 2byte Data Chenge     */
    dicdata[0] = dicdata[1];
    dicdata[1] = buff;
    rl = CHPTTOSH(dicdata);                   /* Get Active Data Area Size   */
                                              /* Search Entry in Dictionary  */
    kjdata = NULL;                            /* Dummy Data                  */
    kjlen = 1;                                /* Dummy Data                  */
    kudedtsc( dicdata, kanadata, kanalen, &kjdata, kjlen, &datapos, &datapos1 );
    /*
     *     Katakana or Kanji Delete
     */
    knl = dicdata[datapos1];                  /* Set Yomi Data Length        */
    dl  = dicdata[datapos1 + knl];            /* Set Kanji Data Length       */
    del_f = TRUE;                             /* Set Delete Flag             */
    if ( mode != U_S_YOMD )
        {
        del_f = FALSE;                        /* Reset Delete Flag           */
        dpos = datapos1 + knl + U_DLLEN;      /* Initial Delete Position Set */
        ddst = U_RSVLEN + U_1KJ_L;            /* Initail Delete Distance Set */
        for ( i = 1 ; i <= pos ; i++ )
            {
            dpos += U_RSVLEN + U_1KJ_L;       /* Reserve Length Add          */
                                              /* Kanji End Code Check        */
            while ( ( dicdata[dpos - U_1KJ_L] & U_MSB_M ) != U_MSB_M )
                {
                dpos += U_1KJ_L;                    /* 1 Kanji Length Add    */
                if ( i == pos )  ddst += U_1KJ_L;   /* 1 Kanji Length Add    */
                }
            }
                                              /* Set MRU Delete Kanji Data   */
        kjdt   = &dicdata[dpos - ddst + U_RSVLEN];
        kjlen  = ddst - U_RSVLEN;
                                              /* MRU Data Delete             */
        kudcmrud ( mode, kanadata, kanalen, kjdt, kjlen, udcbptr );

        dlen = rl - dpos;                     /* Set Delete Length           */
                                              /* Set Delete Flag             */
        if ( pos == 1  &&  dpos == datapos )  del_f = TRUE;
        }

    if ( del_f == TRUE )
        {                           /* 1 Entry Delete Postion & Length Set   */
        dpos = datapos;                       /* Set Delete Position         */
        dlen = rl - datapos;                  /* Set Delete Length           */
        ddst = knl + dl;                      /* Set Delete Distance         */
        }
    else
        {
        dl -= ddst;                           /* Update Kanji Data Length    */
        dicdata[datapos1 + knl] = dl;         /* Set Kanji Data Length       */
        }
                                   /* Target Data Delete ( Data is Last ) */
    if ( rl == datapos  &&  del_f == TRUE )
        for ( i=datapos1; i < U_REC_L; i++ )  dicdata[i] = NULL;
    else                           /* Target Data Delete ( Entry or Data )*/
        kumvch ( dicdata, dpos, dlen, U_FORWD, ddst,
                 TRUE, clrstr, clrstr[0], ddst );
    rl -= ddst;                               /* Update Active Data Size     */
    SHTOCHPT(dicdata, rl);                    /* Set Active Data Size        */
    /*
     *      Dictionary Index Update
     */
    if ( del_f == TRUE )
        if ( rl == U_RLLEN )
           {                                /* This Record is 1 Entry        */
           knl  = dicindex[indxpos];             /* Get Yomi Data Length     */
           dpos = indxpos + knl + U_RRNLEN;      /* Set Delete Position      */
           dlen = il - dpos;                     /* Set Delete Length        */
           ddst = knl + U_RRNLEN;                /* Set Delete Distans       */
           if ( dpos == il )                /* Dictionary Index Entry Delete */
               for ( i=indxpos; i < il; i++ )  dicindex[i] = NULL;
           else
               kumvch ( dicindex, dpos, dlen, U_FORWD, ddst,
                        TRUE, clrstr, clrstr[0], ddst );

           il -= ddst;                      /* Update Active Index Area Size */
           SHTOCHPT(dicindex, il);          /* Set Active Index Area Size    */
           nar--;                           /* Update Next Available RRN     */
                                            /* Set Next Available RRN        */
           dicindex[U_ILLEN + U_STSLEN + U_HARLEN] = nar;
           if ( nar !=  rrn )
              {                             /* Set Index First Entry Position*/
              i = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;
              for ( ; i < il; i += knl + U_RRNLEN )
                  {
                  knl    = dicindex[i];          /* Get Yomi Data Length     */
                  wk_rrn = dicindex[i + knl];    /* Get Record Number        */
                  if ( wk_rrn == nar )
                     dicindex[i + knl] = rrn;    /* Set New Record Number    */
                  }
              kudcread ( udcbptr, 4, nar );      /* Max Data Record Read     */
                                                 /* Copy nar Record Data     */
              memcpy ( dicdata, udcbptr->rdptr, U_REC_L );
              buff = dicdata[0];                 /* First 2Byte Data Chenge  */
              dicdata[0] = dicdata[1];
              dicdata[1] = buff;
                                                 /* Clear Old nar Record     */
              memcpy ( udcbptr->rdptr, clrstr, U_REC_L );
              }
           }
        else
           {
                                    /* Compare of Input Yomi & index Yomi    */
           if ( memcmp( &dicindex[indxpos + U_KNLLEN], kanadata, kanalen )
                 == NULL )
              {
              kjdata = NULL;        /* Dummy Data                            */
              kjlen  = 1;           /* Dummy Data                            */
                                    /* Search Index Entry Yomi Data          */
              kudedtsc( dicdata, kanadata, kanalen,
                        &kjdata, kjlen, &datapos, &datapos1 );
                                    /* Index Entry Replace                   */
              kuderepl( dicindex, indxpos,  (short)dicindex[indxpos],
                        &dicdata[datapos1], (short)dicdata[datapos1] );
              }
           }
        /*
         *     Dictionary Data Write
         */
        buff       = dicdata[0];           /* First 2Byte Data Chenge        */
        dicdata[0] = dicdata[1];
        dicdata[1] = buff;
        udcbptr->wtptr = dicdata;          /* Dictionary Data Write          */
        kudcwrit ( udcbptr, 4, rrn );
        /*
         *     Dictionary index write
         */
        buff        = dicindex[0];         /* First 2Byte Data Chenge        */
        dicindex[0] = dicindex[1];
        dicindex[1] = buff;
        udcbptr->wtptr = dicindex;         /* Index Data Write               */
        kudcwrit ( udcbptr, 3, NULL );

        return;
}
