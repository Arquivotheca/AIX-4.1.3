static char sccsid[] = "@(#)23	1.3  src/bos/usr/lib/nls/loc/imt/tfep/timproca.c, libtw, bos411, 9428A410j 9/17/93 09:20:32";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TIMProcessAuxiliary
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************************************************
*       Project Name : AIX-IM                           *
*       Program Name : TIMProcessAuxiliary()            *
*       Version      : Chinese AIX V 1.0                *
*       Date         : Dec. 05, 1990                    *
*       Designer     : Jim Roung                        *
*       Language     : C                                *
*       Compiler     : AIX C                            *
*       Feature      : TBD                              *
*       History      : TBD                              *
*********************************************************/

/* -----------------  Include Files  ------------------ */

#include "taiwan.h"
#include "stdio.h"

/* ----------------------------------------------------- */
/* **************  TIMProcessAuxiliary()  ************** */
/* ----------------------------------------------------- */

int             TIMProcessAuxiliary(im, aux_id, button, panel_row, panel_col,
                                                    item_row, item_col)
   IMObject        im;
   caddr_t         aux_id;
   uint            button, panel_row, panel_col, item_row, item_col;
{
   /* I didn't know how to implement this routine   */
   /* Meanwhile, Japan didn't implement it either   */
   /* So, this routine will be dummt temporarily    */
   /* Whenever, I get any further information about */
   /* it, I will implement it immediately           */

   return (IMError);

}
