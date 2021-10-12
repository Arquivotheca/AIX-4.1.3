static char     sccsid[] = "@(#)86      1.2  src/bos/usr/ccs/lib/libswvpd/init_vchars.c, libswvpd, bos411, 9428A410j 1/27/94 15:26:25";
/*
 *   COMPONENT_NAME: libswvpd
 *
 *   FUNCTIONS: init_vchars
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include <swvpd0.h>

/*--------------------------------------------------------------------------*
**
** NAME: init_vchars
**
** FUNCTION:  This routine initializes every vchar field associated with 
**            this VPD structure.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void init_vchars     (int    tbl_id,
                      void * tbl_ptr)
{
   lpp_t  * lpp;
   prod_t * product;
   hist_t * history;
   inv_t  * inventory;


   switch (tbl_id)
   {
      case LPP_TABLE :
        lpp = (lpp_t *) tbl_ptr;
        lpp -> group = NULL;
        lpp -> description = NULL; 
        break;

      case PRODUCT_TABLE :
        product = (prod_t *) tbl_ptr;
        product -> name = NULL; 
        product -> fixinfo = NULL; 
        product -> prereq = NULL;
        product -> description = NULL;
        product -> supersedes = NULL;
        break;

      case HISTORY_TABLE :
        history = (hist_t *) tbl_ptr;
        history -> comment = NULL; 
        break;

      case INVENTORY_TABLE :
        inventory = (inv_t *) tbl_ptr;
        inventory -> loc1 = NULL; 
        inventory -> loc2 = NULL; 
        break;

      default:
        break;

   } /* end switch */

} /* init_vchars */
