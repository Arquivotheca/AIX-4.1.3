static char sccsid[] = "@(#)98	1.1  src/bos/usr/lpp/kls/dictutil/huderepl.c, cmdkr, bos411, 9428A410j 5/25/92 14:42:17";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		huderepl.c
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
 *  Module:       huderepl.c
 *
 *  Description:  Index Area Data Replace
 *
 *  Functions:    huderepl()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>                 /* Standard I/O Package. */
#include "hut.h"                   /* Utility Define File.  */

/*----------------------------------------------------------------------*/
/*                      Begining of huderepl				*/
/*		Replaces a old Index Block entry into new.		*/
/*----------------------------------------------------------------------*/
void  huderepl ( rbpt, rbreppos, rbreplen, repdata, replen )
uchar  *rbpt;                       /* Pointer to INDEX Area     (to   Data) */
short   rbreppos;                   /* Replace Start Postion     (to   Data) */
short   rbreplen;                   /* Replace Length            (to   Data) */
uchar  *repdata;                    /* Pointer to Replace Data   (from Data) */
short   replen;                     /* Replace Data Length       (from Data) */
{
   short        il;                /* IL Length for Work                    */
   int           rc,                /* return code Work                      */
                 pos,               /* Start Position(top) Work              */
                 len,               /* Move Length(byte)   Work              */
                 dist,              /* Move Distance (byte)                  */
                 i;                 /* Counter Work                          */
   uchar        *st1,               /* Pointer to String 1                   */
                 clrstr[U_REC_L];   /* Clear Data String                     */

        /*  1.1
         *      initialize
         */
        /* Active INDEX Area's Size Get          */
	il = getil(rbpt);
   	memset(clrstr, 0xff, U_REC_L);

        /*  2.1
         *      Move Data Index Area
         */
        dist = replen - rbreplen;        /* Calculation Move Distance        */
        pos  = rbreppos + rbreplen;     /* Calculation Move Position        */
/**** Nov27
        pos  = rbreppos + rbreplen + 2; **/     /* Calculation Move Position        */
        len  = il - pos;                 /* Calculation Move Length          */
        if ( dist > 0 )
                                         /* INDEX Area BACKWARD Move         */
          rc = humvch ( rbpt, pos, len, U_BACKWD, dist,
                              TRUE, clrstr, clrstr[0], len );

        if ( dist < 0 )
                                         /* INDEX Area FORWARD Move          */
          rc = humvch ( rbpt, pos, len, U_FORWD, -dist,
                              TRUE, clrstr, clrstr[0], len );

        /*  2.2
         *      Replace Data Index Area
         */

        st1  = rbpt + rbreppos;          /* To Data Address Set              */
                                         /* INDEX Area Data Replace          */
        memcpy ( st1, repdata, replen );

        /*
         *      Return & Value set
         */
        il += replen - rbreplen;         /* Active INDEX Area's Size Calc.   */
        /* Set Active INDEX Area's Size     */
	setil(rbpt, il);
        return;
}
/*----------------------------------------------------------------------*/
/*                      End of huderepl				        */
/*----------------------------------------------------------------------*/
