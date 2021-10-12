static char sccsid[] = "@(#)73  1.4  src/bos/usr/ccs/lib/libswvpd/vpd_free.c, libswvpd, bos411, 9428A410j 4/22/94 13:02:08";

/*
 *   COMPONENT_NAME: LIBSWVPD
 *
 * FUNCTIONS: vpd_free_vchars
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include <swvpd0.h>


/*--------------------------------------------------------------------------*
**
** NAME: vpd_free_vchars
**
** FUNCTION:  This routine frees every vchar field allocated by odmget_obj
**            for this product structure.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void vpd_free_vchars (int    tbl_id,
                      void * tbl_ptr)
 {
   lpp_t  * lpp;
   prod_t * product;
   hist_t * history;
   inv_t  * inventory;
   fix_t  * fix;


   switch (tbl_id)
   {
      case LPP_TABLE :
        lpp = (lpp_t *) tbl_ptr;
        if (lpp -> group != NULL )
        {
           free (lpp -> group);
           lpp -> group = NULL;
        }
        if (lpp -> description != NULL )
        {
           free (lpp -> description);
           lpp -> description = NULL;
        }
        break;

      case PRODUCT_TABLE :
        product = (prod_t *) tbl_ptr;
        if (product -> name != NULL )
        {
           free (product -> name);
           product -> name = NULL;
        }
        if (product -> fixinfo != NULL )
        {
           free (product -> fixinfo);
           product -> fixinfo = NULL;
        }
        if (product -> prereq != NULL )
        {
           free (product -> prereq);
           product -> prereq = NULL;
        }

        if (product -> description != NULL )
        {
           free (product -> description);
           product -> description = NULL;
        }
        if (product -> supersedes != NULL )
        {
           free (product -> supersedes);
           product -> supersedes = NULL;
        }

        break;

      case HISTORY_TABLE :
        history = (hist_t *) tbl_ptr;
        if (history -> comment != NULL )
        {
           free (history -> comment);
           history -> comment = NULL;
        }
        break;

      case INVENTORY_TABLE :
        inventory = (inv_t *) tbl_ptr;
        if (inventory -> loc1 != NULL )
        {
           free (inventory -> loc1);
           inventory -> loc1 = NULL;
        }
        if (inventory -> loc2 != NULL )
        {
           free (inventory -> loc1);
           inventory -> loc2 = NULL;
        }
        break;
      case FIX_TABLE :
	fix = (fix_t *) tbl_ptr;
	if (fix -> filesets != NULL )
	{
	   free (fix -> filesets);
	   fix -> filesets = NULL;
	}
	if (fix -> symptom != NULL )
	{
	   free (fix -> symptom);
	   fix -> symptom = NULL;
	}
   } /* end switch */

   return;
 } /* vpd_free_vchars */
