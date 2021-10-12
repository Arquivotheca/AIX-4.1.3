static char sccsid[] = "@(#)25	1.1  src/bos/usr/lpp/kls/dictutil/humvch.c, cmdkr, bos411, 9428A410j 5/25/92 14:47:24";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		humvch.c
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
/************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       humvch.c
 *
 *  Description:  Character movement in a offset, then clear
 *                remaining area.
 *
 *  Functions:    humvch()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ************************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.                        */
#include "hut.h"        /* Dictionary Utility Define File.              */

/*----------------------------------------------------------------------*/
/*                      Begining of humvch.                             */
/*----------------------------------------------------------------------*/
int humvch(str, pos, len, dir, dist,clearp, clrstr, clrpos, clrlen)
uchar *str;              /* Pointer to character dimension.             */
int   pos;              /* Start position.                              */
int   len;              /* Character length you want move.              */
int   dir;              /* Move Direction. M_FORWD or M_BACKWD          */
int   dist;             /* Move Distance.                               */
int   clearp;            /* Crear flag (You want clear remaining data)  */
                        /* specify TRUE or FALSE.                       */
uchar *clrstr;           /* Pointer to string contain clear data.       */
int   clrpos;           /* Clear data start position in clrstr.         */
int   clrlen;           /* Clear data length in clrstr(not in str).     */
{
        register uchar   *org;           /* Oreginal character address.  */
        register uchar   *dest;          /* Destination address.         */
        uchar            *stop;          /* Stop address.                */
        uchar            *clrstart;      /* clear data start address.    */
        int             times;           /* Clear loop counter.          */
        int             rem;             /* remainder of Clear loop.     */
        uchar            *memcpy();      /* memory copy function         */

        /* 1.
         *      Check input parameter.
         */

        if ((pos < 0) ||
            (len <= 0) ||
	    ((clearp == TRUE) && ((clrpos < 0) || (clrlen <= 0))) ||
            ((dir == U_FORWD) && (pos < dist))                     ) {

                return(IUFAIL);
        }

        /* 2.
         *      Branch by parameter -dir-.
         */


        if (dir == U_FORWD) {   /* Move foreawd */

                org  = str + pos;   /* Calculate sorce start address        */
/***Nov28
                stop = org + len;   **/ /* Calculate move stop address          */
                stop = org + len -1;    /* Calculate move stop address          */
                dest = org - dist;  /* Calculate destination start address  */
                while(org <= stop) {
                        *dest++ = *org++; /* Copying data */
                }
		if (clearp) { /* if clear == TRUE */
                        times    = dist / clrlen;
                                            /* memcpy() Execution times     */
                        rem      = dist % clrlen;
                                            /* remainder after memcpy loop  */
                        dest     = str + pos + len - dist;
                                            /* destination address.         */
                        clrstart = clrstr + clrpos;
                                            /* Calculate sorce start address*/
                        while (times > 0) {
                                memcpy(dest, clrstart, clrlen);
                                            /* copying data                 */
                                dest += clrlen;
                                            /* New destination address      */
                                times--;
                                            /* Loop counter decrement       */
                        }
                        memcpy(dest, clrstart, rem);
                                            /* fill in remaining area       */
                }
        }
        else {                  /* Move backward */
                org  = str + pos + len - 1;
                                    /* Calculate sorce start address        */
                stop = str + pos;
                                    /* Calculate move stop address          */
                dest = org + dist;
                                    /* Calculate destination start address  */
                while(org >= stop) {
                        *dest-- = *org--;  /* copying data */
                }
		if (clearp) {
                        times    = dist / clrlen;
                                            /* memcpy() Execution times     */
                        rem      = dist % clrlen;
                                            /* remainder after memcpy loop  */
                        dest     = str + pos;
                                            /* destination address.         */
                        clrstart = clrstr + clrpos;
                                            /* calculate sorce start address*/
                        while (times > 0) {
                                memcpy(dest, clrstart, clrlen);
                                                /* fill in specified data   */
                                dest += clrlen; /* New destination address  */
                                times--;
                        }
                        memcpy(dest, clrstart, rem); /* fill remaining area */
                }
        }

        return(IUSUCC);
}
/*----------------------------------------------------------------------*/
/*                      End of humvch.                             	*/
/*----------------------------------------------------------------------*/
