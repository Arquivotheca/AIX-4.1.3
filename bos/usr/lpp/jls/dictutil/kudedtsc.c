static char sccsid[] = "@(#)17	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudedtsc.c, cmdKJI, bos411, 9428A410j 7/23/92 01:01:01";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudedtsc
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
 * MODULE NAME:         kudedtsc
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
 * FUNCTION:            User Dictionary Data(Yomi & Kanji) Search Process
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        896 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudedtsc
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudedtsc ( dicdata, kanadata, kanalen, intkjdata,
 *                                   kjlen, datapos, datapos1 )
 *
 *  INPUT:              dicdata  :Pointer to User Dictionary
 *                      kanadata :Search to Yomi Data
 *                      kanalen  :Search to Yomi Data Length
 *                      intkjdata:Search to Kanji Data
 *                      kjlen    :Search to Kanji Data Length
 *
 *  OUTPUT:             datapos  :Search for Data Relative Byte -1
 *                      datapos1 :Search for Data Relative Byte -2
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NOKAN   :Yomi Data is None
 *                      NOKNJ   :Yomi Data is Exist & Kanji Data is None
 *                      KANKNJ  :Yomi Data is Exist & Kanji Data is Exist
 *
 * EXIT-ERROR:          Return Codes Returned to Caller.
 *                      IUFAIL  :Input User Dictionary is Empty
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcmp
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
 *                              CHPTTOSH:char to short int Data transfer
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
#include "kut.h"                        /* Utility File.                */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int  kudedtsc ( dicdata, kanadata, kanalen, intkjdata, kjlen,
                                            datapos, datapos1 )

uchar  *dicdata;       /* Pointer to RRN                                     */
uchar  *kanadata;      /* Pointer to Input Yomi Data                         */
short   kanalen;       /* Input Yomi Data Length                             */
uchar  *intkjdata;     /* Pointer to Kanji Data                              */
short   kjlen;         /* Input Kanji Data Length                            */
short  *datapos;       /* Pointer to Data Position 0                         */
short  *datapos1;      /* Pointer to Data Position 1                         */

{
   int       i,              /*  Counter Work 1                              */
             j,              /*  Counter Work 2                              */
             rc,             /*  Memory Compare return Code Area             */
             length;         /*  Yomi Data Compare Length Set Area           */
   ushort    rl,             /*  RRN Length                                  */
             prelen;         /*  Previous Length Set Area                    */
   uchar     knl,            /*  1 Entry Length                              */
             dl,             /*  Kanji Data Length                           */
             kjdt[U_KNJ_MX]; /*  Kanji Data Get Work                         */

  /*  1.1
   *      Initialize
   */
   *datapos  = NULL;                      /* Clear Relative Byte Address -1  */
   *datapos1 = NULL;                      /* Clear Relative Byte Address -2  */
   rl  = CHPTTOSH(dicdata);               /* GET RRN Active Data's Size      */
   if ( rl <= NULL )  return(IUFAIL);     /* This RRN is Empty               */

  /*  2.1
   *      Data Search Process
   */
   for ( *datapos1 = U_RLLEN; *datapos1 < rl;
                              *datapos1 = ( *datapos1 + knl + dl) )
   {
     /*  2.2
      *      Search Yomi Data
      */
      *datapos = *datapos1 + U_KNLLEN;      /* Set Yomi Data Address         */
      knl      = *(dicdata + *datapos1);    /* Set Yomi Data Length          */

                                            /* Set memcmp Length for Small   */
      length = ( (knl - U_KNLLEN) < kanalen ) ? ( knl - U_KNLLEN ) : kanalen;

                                            /* Yomi Data Compare             */
      rc = memcmp ( kanadata, (char *)(dicdata + *datapos), length);

      *datapos = *datapos1 + knl;           /* Set Kanji Data Header Address */
      dl       = *(dicdata + (*datapos));   /* Get Data Length in this Entry */

     /*  2.3
      *      Search Kanji Data
      */
      if ( rc == NULL  &&  kanalen == ( knl - U_KNLLEN ) )
      {
                                            /* Set Kanji Data First Address  */
         *datapos += U_DLLEN + U_RSVLEN;
         for ( i = dl - U_DLLEN - U_RSVLEN, j = 0; i > 0; i -= U_1KJ_L)
         {
                                            /* Set Kanji Data for Work Area  */
            kjdt[j]   = *(dicdata + (*datapos));
            (*datapos)++;
            kjdt[j+1] = *(dicdata + (*datapos));
            (*datapos)++;
            j += U_1KJ_L;                   /* Kanji Counter Count Up        */
                                            /* Kanji Data End Check (1 Data) */
            if ( (kjdt[j-U_1KJ_L] & U_MSB_M) == U_MSB_M )
            {
                                  /* Patern-3 Yomi Exist & Kanji Exist(RC=3) */
               if ( (memcmp(kjdt,intkjdata,kjlen)==NULL) && (j==kjlen) )
               {
                  *datapos -= j;            /* 1 Before Entry Address Set    */
                  return(U_KANKNJ);         /* Yomi & Kanji Exist            */
               }
               *datapos += U_RSVLEN;        /* Reserve Length Increment      */
               i        -= U_RSVLEN;        /* Reserve Length Decrement      */
               j         = 0;               /* 1 Kanji Length Clear          */
            }
         }
         *datapos -= U_RSVLEN;    /* Reserve Length Decrement                */
         return(U_NOKNJ);         /* Patern-2 Yomi Exist & Kanji None (RC=2) */
      }
      if ( rc < NULL || ( rc == NULL  &&  kanalen < ( knl - U_KNLLEN ) ) )
      {
         if ( *datapos1 == U_RLLEN )
            *datapos = U_RLLEN;          /* First Entry Address Set          */
         else
         {
            *datapos   = *datapos1;      /* Current Entry address set        */
            *datapos1 -= prelen;         /* Before Entry address set         */
         }
         return(U_NOKAN);                /* Patern-1 Yomi none(RC=0)         */
      }
      prelen = knl + dl ;                /* Previous Data Length Set         */
   }


   *datapos  = rl;                       /* SET RRN Deactive header address  */
   *datapos1 = rl - knl - dl;            /* SET RRN Deactive header address  */
   return(U_NOKAN);                      /* Patern-1 Yomi none(RC=0)         */
}
