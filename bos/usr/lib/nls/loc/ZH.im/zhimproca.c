static char sccsid[] = "@(#)72	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhimproca.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:39:13";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHIMProcessAuxiliary
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* -----------------  Include Files  ------------------ */

#include "chinese.h"
#include <stdio.h>

/* ----------------------------------------------------- */
/* **************  ZHIMProcessAuxiliary()  ************** */
/* ----------------------------------------------------- */

int             ZHIMProcessAuxiliary(im, aux_id, button, panel_row, panel_col,
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
