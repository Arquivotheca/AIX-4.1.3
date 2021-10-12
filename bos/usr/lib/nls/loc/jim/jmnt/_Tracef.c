static char sccsid[] = "@(#)57	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Tracef.c, libKJI, bos411, 9428A410j 7/23/92 03:25:36";
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
 * MODULE NAME:         _Tracef
 *
 * DESCRIPTIVE NAME:    Trace data file control.
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
 * FUNCTION:            Making the trace data file and write the TRB.
 *                      switch the trace memory mode or the trace disk mode.
 *
 * NOTES:               the system debug routine.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        904 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Traced
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Tracef( troutp,opcode )
 *
 *  INPUT:              troutp  : pointer to trace block.
 *                      opcode  : operation code.( 0:open 1:close )
 *
 *  OUTPUT:             trace block.(TRB      : memory)
 *                      trace file. (DBCSTRAC : disk)
 *
 * EXIT-NORMAL:         TRSUCC  : Success of Execution.(write to memory)
 *                              : Success of Execution.(write to disk)
 *
 *
 * EXIT-ERROR:          TRFWTE  : disk file write error.
 *                      TRFOPE  : disk file open  error.
 *                      TRFCLE  : disk file close error.
 *
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              NA.
 *                      Standard Library.
 *                              memcpy() : Memory copy function.
 *                              write()  : Write to a file.
 *                                         (memory is operate as a file.)
 *                              open()   : open a file.
 *                              close()  : close a file.
 *                              unlink() : Unlink a file.
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
#include <memory.h>     /* Set memory copy memory.                      */
#include <fcntl.h>

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
 *      Trace file Program.
 */
_Tracef( troutp1,opcode )

TRB *troutp1;               /* Pointer to trace block.                  */
short   opcode;             /* Operation code.( 0:open 1:close )        */
{
        /* Function Definition.                                         */
        extern  int     write();        /* Write the devide.            */
        extern  int     unlink();       /* Unlink the devide.           */
        extern  int     open();         /* Open the devide.             */
        extern  int     close();        /* Close the devide.            */

        extern  int     _Traced();       /* Trace data.                  */

        /* Trace block                                                  */
        struct  _trbroot *troutp;

        /* Sorce trace data block.                                      */
        struct   tracex
           {
                short   length;     /* Trace data block length.         */
                short   index;      /* Trace ON/OFF discriminate.       */
                char    trdata[4];  /* Trace data.                      */
           } message;

        /* Local parameter.                                             */
        char    trfname[16];              /* Trace File Name.           */
        char   *path;                     /* Func. Para. File unlink.   */
        char   *trdp;                     /* Func. Para. TRB Traced.    */
        char   *troutp_;                  /* Func. Para. TRB Traced.    */
        int     oflag;                    /* Func. Para. File open.     */
        int     modeii;                   /* Func. Para. File open.     */
        int     fildes;                   /* Func. Para. File close.    */
        int     rc;                       /* Return code.(internal)     */
        int     write_rc;                 /* Return code.(Disk)         */

/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_TRB  ,SNAP_Tracef,"START TRACEF");

        /* 1.1
         *      Get TRB pointer address.
         */
        troutp = &troutp1->trbblk;

        /* 1.2
         *      Check the process code.
         */
        /* Set trace file name.                                         */
        sprintf(trfname,"%s",DBCSTRAC);

        /* If operation code is not file close code then
                        trace file open , chenge trace disk set.        */
        if (opcode != C_TRFCL )
             {

                /* 1.3
                 *      Delete the trace file.
                 */
                path     =  trfname;        /* Set delete file name.    */
                write_rc =  unlink( path ); /* Delete file.             */

                /* 1.4
                 *      Open the trace file.
                 */
                path     =  trfname;              /* Set open file name.*/
                oflag    = ( O_CREAT | O_WRONLY );/* write only.        */
                modeii   =  0;                    /* mode = NULL.       */
                write_rc = open(path,oflag,modeii); /* Open file.       */

                /* 1.5
                 *      Check Open Error.
                 */
                if (write_rc == C_FAUL)
                    {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Tracef,"ERROR 1.5 TRACEF");

                        return(TRFOPE);                 /* Error RETURN */
                    }
                else
                    {
                        /* Set file descripter.                         */
                        troutp->filds = write_rc;
                    }
                /* 1.6
                 *      Write TRB To disk file for the descripter.
                 */
                write_rc = write(troutp->filds,troutp1,troutp->trblklen);

                /* 1.7
                 *      Check On Error.(Write Disk)
                 */
                if (write_rc != troutp->trblklen)
                    {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Tracef,"ERROR 1.7 TRACEF");

                        /* disk file write error.                       */
                        return(TRFWTE);
                    }
                else
                    {
                        /* Set the disk out message on TRB. By _Traced() */

                        message.length     = 8;
                        message.index      = 1;
                        message.trdata[0]  = 'd';
                        message.trdata[1]  = 'i';
                        message.trdata[2]  = 's';
                        message.trdata[3]  = 'k';

                        /* Set the message top address.                 */
                        trdp    = (char *)&message;

                        /* Set TRB top address.                         */
                        troutp_ = (char *)troutp1;

                        /* Write the message to TRB on memory */
                        rc      = _Traced(troutp_,trdp);

                        /* Chenge the distination frlag on TRB.          */
                        troutp->trflag  = K_TALL;

                        /* RETURN                                        */
/*************************************************************************
 *      debug process routine.                                           */
snap3(SNAP_TRB  ,SNAP_Tracef,"END 1.7 TRACEF");

                        return(rc);                           /* RETURN  */
                    };
             }
        else
             {
                /* 1.8
                 *      Close the file for TRB descripter.
                 */
                /* Set the disk file descripter.                        */
                fildes   = troutp->filds;

                /* Close the trace file.                                */
                write_rc = close(fildes);

                /* 1.9
                 *      Check on error.
                 */
                if ( write_rc == C_FAUL )
                    {
/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Tracef,"ERROR 1.9 TRACEF");

                        /* The trace disk file close error.             */
                        return(TRFCLE);                 /* Error RETURN */
                    }
                else
                    {
                        /* trace flag set on the memory out.            */
                        troutp->trflag  = K_TLIMIT;

/************************************************************************
 *      debug process routine.                                          */
snap3(SNAP_TRB  ,SNAP_Tracef,"END 1.9 TRACEF");

                        /* trace file closed , normal return.           */
                        return(TRSUCC);                      /* RETURN  */
                    };
             };
}
