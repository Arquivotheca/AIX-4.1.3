static char sccsid[] = "@(#)95	1.1  src/bos/usr/lpp/kls/dictutil/hudeadpr.c, cmdkr, bos411, 9428A410j 5/25/92 14:41:48";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudeadpr.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudeadpr.c
 *
 *  Description:  User dictionary Data Add
 *
 *  Functions:    hudeadpr()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*              Include Header.                                         */
/*----------------------------------------------------------------------*/

#include <stdio.h>   /* Standard I/O Package. */
#include <memory.h>
#include "hut.h"     /* Utility include File. */

/*----------------------------------------------------------------------*/
/*              Begining of hudcmrud.                                   */
/*		New Data add at memory.					*/
/*----------------------------------------------------------------------*/
void  hudeadpr ( mode, rbpt, rbpt1, length )
short    mode;                     /* Mode to Add type                       */
uchar   *rbpt;                     /* Pointer to Data (from Data) for RRN    */
uchar   *rbpt1;                    /* Pointer to Data (to   Data) for RRN    */
short    length;                   /* Move to Data Length                    */
{
   int           humvch();         /* Move Character function                */
   short        rl,               /* From Data Active Data's Size Save Area */
                 rl1;              /* To   Data Active Data's Size Save Area */
   int           rc,               /* Return Code from humvch                */
                 pos,              /* Start Position(top) Work               */
                 len,              /* Move Length(byte)   Work               */
                 i;                /* Counter Work                           */
   uchar        *st1,              /* Pointer to String 1                    */
                *st2,              /* Pointer to String 2                    */
                 clrstr[U_REC_L];  /* Clear Data String                      */

     /*********/
     /*	      */
     /* Init. */
     /*	      */
     /*********/
     rl = getrl(rbpt);
     rl1 = getrl(rbpt1);   
     if ( rl1 <= 0 )  rl1 = U_RLLEN;
     memset(clrstr, 0xff, U_REC_L);

     /***********************************************/
     /*						    */
     /* Data Add Process (mode==1) Data Add to Left */
     /*						    */
     /***********************************************/
     if ( mode == U_DADDLF )
     {
        st1 = rbpt1 + rl1;             /* Copy To Address Set                */
        st2 = rbpt  + U_RLLEN;         /* Copy From Address Set              */
        memcpy ( st1, st2, length );   /* Entry Copy From RRN -> To RRN      */
        pos = U_RLLEN + length;        /* Move Character Position Set        */
        len = rl - pos;                /* Move Character Length Set          */
                                       /* Character Move after Null padding  */
        rc  = humvch ( rbpt, pos, len, U_FORWD, length,
                             TRUE, clrstr, clrstr[0], len );

        rl1 += length;                 /* Update RRN Active Data's Area Size */
        rl  -= length;                 /* Update RRN Active Data's Area Size */
     }

     /**************************************************/
     /*	   					       */
     /* Data Add Process ( mode==2 ) Data Add to Right */
     /*	   					       */
     /**************************************************/
     if ( mode == U_DADDRT )
     {
        pos = U_RLLEN;                 /* Move Character Position Set        */
        len = rl1;                     /* Move Character Length Set          */
                                       /* Character Move after Null padding  */
        rc  = humvch ( rbpt1, pos, len, U_BACKWD, length,
                              TRUE, clrstr, clrstr[0], length );

        st1 = rbpt1 + U_RLLEN;         /* Copy To Data Address Set           */
        st2 = rbpt  + rl - length;     /* Copy From Data Address Set         */
        memcpy ( st1, st2, length );   /* Entry Copy From RRN -> To RRN      */

        rl1 += length;                 /* Update RRN Active Data's Area Size */
        rl  -= length;                 /* Update RRN Active Data's Area Size */
                                       /* Character Move after Null padding  */
        rc  = humvch ( rbpt, rl, length, U_BACKWD, length,
                             TRUE, clrstr, clrstr[0], length );
     }

     /************************************************/
     /*						     */
     /* Data Add Process ( mode==3 ) Data Add to New */
     /*						     */
     /************************************************/
     if ( mode == U_DADDNW )
     {
	/* Modify Comments: */
	memcpy ( rbpt1+U_RLLEN, rbpt+U_RLLEN+length, rl-length-U_RLLEN);
	memset ( rbpt +U_RLLEN+length, 0xff, rl-length-U_RLLEN);


        rl1  = rl-length ;             /* Update RRN Active Data's Area Size */
        rl   = U_RLLEN+length;         /* Update RRN Active Data's Area Size */
     }

     /*
      *      Return Code.
      */
     /* SET From RRN Active Data's Size    */
     setrl(rbpt, rl);
     /* SET To   RRN Active Data's Size    */
     setrl(rbpt1, rl1);
     return;
}
/*----------------------------------------------------------------------*/
/*              End of hudcmrud.                                        */
/*----------------------------------------------------------------------*/
