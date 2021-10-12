static char sccsid[] = "@(#)56	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Traced.c, libKJI, bos411, 9428A410j 7/23/92 03:25:31";
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

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         _Traced
 *
 * DESCRIPTIVE NAME:    trace the real data of argument point.
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
 * FUNCTION:            put out the trace data to memory or disk.
 *
 * NOTES:               the system debug routine.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        968 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Traced
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Traced( troutp,trdp )
 *
 *  INPUT:              troutp  : pointer to trace block.
 *                      trdp    : pointer to trace data.
 *
 *  OUTPUT:             trace block. (TRB:memory)
 *                      trace file.  (disk)
 *
 * EXIT-NORMAL:         TRSUCC  : Success of Execution.(write to memory)
 *                              : Success of Execution.(write to disk)
 *
 *
 * EXIT-ERROR:          TRFWTE  : disk write error.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy() : Memory copy function.
 *                              write()  : Write a data to file.
 *
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
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              all.
 *
 *   OUTPUT:            DBCS Editor Control Block(DECB,DECB_FLD).
 *                              NA.
 *                      Extended Information Block(EXT).
 *                              NA.
 *                      Kanji Monitor Controle Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              all.
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
#include <memory.h>     /* Performs memory operations.                  */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "trb.h"        /* Magapel-Trace Block Define File.             */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Data Trace Program.
 */
_Traced( troutp1,trdp1 )

TRB     *troutp1;         /* Pointer to trace block.                    */
char    *trdp1;           /* Pointer to trace data.                     */
{
        /* Function definition.                                         */
        int     write();
        char    *memcpy();

        /* Trace block.                                                 */
        struct  _trbroot *troutp;

        /* Sorce trace data block.                                      */
        struct   tracex
           {
                short   length;     /* Trace data block length.         */
                short   index;      /* Trace ON/OFF discriminate        */
                                    /*                  switch or flag. */
                char    trdata[1];  /* Trace data                       */
           }    *trdp;

        /* Distination trace data block.                                */
        struct  trtophdr
           {
                unsigned long   serno;      /* Serial number of TRB.    */
                int             totlen;     /* Trace data block length. */
           }    *tophdr,topddd;
        int     trpreend;               /* Previouse end header address.*/

        /*
         *  Local parameter.
         */
        char    padchr[C_TRWORD];   /* Padding blank character area.    */
        int     write_rc;           /* Disk write function return code. */
        int     chklen;             /* Size of remaines trace out area. */
        int     padlen;             /* Padding blank character length.  */
        int     i,j,n;              /* Work parameter.                  */

/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_TRB  ,SNAP_Traced,"START TRACED");

        /* 1.1
         *      Get TRB from the "troutp".
         */
        troutp = &troutp1->trbblk;

        /* 1.2
         *      Get the trace data from the main program.
         */
        trdp    = (struct tracex *)trdp1;

        /*      Check the input data length error.                     */
        if (trdp->length <
                (int)(sizeof(trdp->length) + sizeof(trdp->index)))
           {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"ERROR 1.2 TRACED");

                return(TRSUCC);                         /* ERROR RETURN */
                }

        /*      Check the trace data length.                            */
        padlen = trdp->length % C_TRWORD;
        if ( padlen != C_INIT )
                {
                        /* Set the length for padding blank character.  */
                        padlen =  C_TRWORD - padlen;

                        /* Add the length of padding blank character
                                        to the output trace data length.*/
                        trdp->length += padlen;
                };

        /* 1.3
         *      Make the trace data block
         */
        /* Set internal trace data hedder address.                      */
        tophdr          = &topddd;

        /* Set seqence nomber of the trace output count.                */
        tophdr->serno   = troutp->troutsno;
        /*
         *      Set the trace data total length.
         */

        /* Set the size of input trace data
             + the size of padding blank character.                     */
        tophdr->totlen  = trdp->length;

        /* Add the size of serial number area.                          */
        tophdr->totlen += sizeof(tophdr->serno);

        /* Add the size of total length area.                           */
        tophdr->totlen += sizeof(tophdr->totlen);

        /* Add the size of previous trace data last pointer             */
        tophdr->totlen += sizeof(trpreend);

        /*
         *      Set the previous trace data last pointer.
         */
        trpreend   = troutp->trendadr;

        /* 1.4
         *      Check the distination flag.
         */

        if ( troutp->trflag != K_TALL )
             {

                 /* 1.5
                  *      Check the size of distination.
                  */

                 /*  If trace data size is more than total trace area size,
                         normal return without trace out.               */
                 if (tophdr->totlen >
                         (troutp->trblklen - (int)sizeof(troutp1->trbblk)) )
                        {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"ERROR 1.5 TRACED");

                        return(TRSUCC);                 /* ERROR RETURN */
                        }
                 /* If trace data size is more than remains trace area size,
                           output pointer is top address of trace area. */
                 chklen  =  troutp->trblklen - troutp->troutadr;
                 if ( chklen < tophdr->totlen )
                     {

                         /* 1.6
                          *      Renew the pointer of TRB distination.
                          */
                         /* Trace address is set the next one of
                                              the trace hedder area.    */
                         troutp->troutadr = sizeof(troutp1->trbblk);
                      };
                 /* 1.7
                  *      Write the trace data to TRB.
                  */

                 /* Get TRB header length                               */
                 i  =  sizeof(troutp1->trbblk);

                 /* Get trace data pointer.                             */
                 i  =  troutp->troutadr - i;

                 /* Write start header.                                 */
                 memcpy(&troutp1->trace[i],tophdr,sizeof(*tophdr));
                 i +=  sizeof(*tophdr);

                 /* Write trace data without padding data.              */
                 n = trdp->length - padlen;
                 memcpy(&troutp1->trace[i],trdp,n);
                 i +=  n;

                 /* If it have some padding blank character, write some.*/
                 if (padlen != C_INIT)
                         {
                         for(j=0;j<padlen;j++)
                             padchr[j] = ' '; /* set blank character.   */

                         /* Write pad blank data.                       */
                         memcpy(&troutp1->trace[i],padchr,padlen);
                         i += padlen;
                         };

                 /* Write end header.                                   */
                 memcpy(&troutp1->trace[i],&trpreend,sizeof(trpreend));

                 /* 1.8
                  *      Update TRB control data.
                  */

                 /* Increment trace data ID number.                     */
                 troutp->troutsno++;

                 /* Update write the block address.                     */
                 troutp->troutadr  +=  tophdr->totlen;

                 /* Update previouse end header.                        */
                 troutp->trendadr   =  troutp->troutadr - sizeof(trpreend);

                 /* RETURN                                              */
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"END 1.8 TRACED");

                 return( TRSUCC );                            /* RETURN */
             }

        else
             {
                /* 1.9  For Trace Disk.
                 *      Write the trace data to disk file & update TRB.
                 */

                /* Write start header                                   */
                write_rc = write(troutp->filds,tophdr,sizeof(*tophdr));

                /* If write error then error return                     */
                if (write_rc != sizeof(*tophdr))
                        {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"ERROR 1.9 TRACED");

                        return(TRFWTE);                  /* ERROR RETURN*/
                        }
                /* Write the trace data without padding data.           */
                n = trdp->length - padlen;
                write_rc = write(troutp->filds,trdp,n);

                /* If write error then error return.                    */
                if (n != write_rc)
                        {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"ERROR 1.9 TRACED");

                        return(TRFWTE);                  /* ERROR RETURN*/
                        }

                 /* If it have some padding blank character, write some.*/
                 if (padlen != C_INIT)
                        {
                        for(i=0;i<padlen;i++)
                                padchr[i] = ' '; /* Set blank character.*/

                        /* Write pad blank data.                        */
                        write_rc = write(troutp->filds,padchr,padlen);

                        /* If write error then error return.            */
                        if (write_rc != padlen)
                                {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"ERROR 1.9 TRACED");

                                return(TRFWTE);          /* ERROR RETURN*/
                                }
                        };

                /* Write end header.                                    */
                write_rc = write(troutp->filds,&trpreend,sizeof(trpreend));

                /* If write error then error return.                    */
                if(write_rc != sizeof(trpreend))
                        {
/*************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"ERROR 1.9  TRACED");

                        return(TRFWTE);                  /* ERROR RETURN*/
                        }
                /* Increment trace data ID number.                      */
                troutp->troutsno++;

                /*
                 *  Complete trace disk process.
                 */
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Traced,"END 1.9 TRACED");

                return( TRSUCC );                             /* RETURN */
             };
}
