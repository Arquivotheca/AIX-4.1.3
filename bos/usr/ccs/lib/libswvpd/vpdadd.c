/* @(#)99       1.12  src/bos/usr/ccs/lib/libswvpd/vpdadd.c, libswvpd, bos411, 9428A410j 3/26/94 17:26:14 */

/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: vpdadd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <swvpd0.h>                     /* declarations for swvpd rtns  */
#include <stdlib.h>
#include <string.h>

/*
 * NAME: vpdadd
 *
 * FUNCTION: Add record to the table specified from buffer
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Add the record in the buffer area to the table specified.  The
 *      data in the buffer must match the structure required for the
 *      table addressed. The table addressed will be controlled by the
 *      current setting of the vpd_path.
 *
 * (NOTES:) Add the specified record to the table.  Special handling is
 *      needed for the lpp table. We will locate the VPD control record
 *      in the LPP table and use the value in that record to set the
 *      lpp_id for the record being added to the LPP table.  For all
 *      other tables no such resolution is done.
 *
 * (DATA STRUCTURES:) If the table addressed is the LPP table, the entry
 *      in that table used to control lpp_id's will be updated to the
 *      latest used lpp_id.  If the table is not open on entry to this
 *      routine, it will be opened and the global structure tracking that
 *      state will be updated.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdadd... */

int vpdadd (int    tbl_id,   /* Table ident code value. */
            void * tbl_ptr)  /* Pointer to table of the type specified. */
{

  int   rc ;                            /* Hold return code for testing */
  int   suppress_autonumbering = 0;	/* Auto number the LPP table ids */

  struct lpp mylpp ;                    /* local working lpp structure  */


/* Begin vpdadd... */
  if (tbl_id == MERGING_LPP_TABLE) {
     tbl_id = LPP_TABLE;
     suppress_autonumbering = 1;
  }

  if (T_OPEN(tbl_id)!=TRUE)             /* if the table is not open, do */
    {                                   /* all required initialization  */
    if ( (rc = vpdinit(tbl_id)) != VPD_OK)
      { return(rc) ; }
    }

  /* If adding to LPP table must set the lpp_id value based   */
  /* on value in control record  			      */
  if ((tbl_id == LPP_TABLE) && (!suppress_autonumbering)) {


    memset(&mylpp, 0, sizeof (mylpp)) ; /* initialize local lpp struct  */
    strcpy(mylpp.name,LPP_CTL_NAME) ;   /* set up local struct w/ ctl nm*/

    if (vpdget(LPP_TABLE, LPP_NAME, &mylpp) == VPD_OK)
      {
      mylpp.lpp_id += 1 ;               /* increment lpp_id in ctl rec  */
      rc = odm_change_obj(TABLE(LPP_TABLE),&mylpp) ;
                                        /* put updated version into tbl */
      }
    else
      {
      mylpp.lpp_id = 2;                 /* set first lpp_id value       */
                                        /* init to 2 so first value used*/
                                        /* is 1, see below              */

      rc = odm_add_obj(TABLE(LPP_TABLE),&mylpp) ;
                                        /* add lpp_id ctl rec to table  */
      } /* endif */

    if (rc == -1)
      { return(VPD_SYS) ; }             /* could not add/update ctl rec */

                                        /* NOTE: reserved record has the*/
                                        /* id number to be used NEXT    */
                                        /* since we have incremented it */
                                        /* above, need to decrement it  */
                                        /* before using it.  By keeping */
                                        /* this value 1 beyond the value*/
                                        /* in use, avoid possibly       */
                                        /* resolving an active id to the*/                                              /* reserved record name         */
    ((struct lpp *) tbl_ptr)->lpp_id = mylpp.lpp_id-1 ;
                                        /* now have resolved lpp_id     */
    }                                   /* end if adding to LPP table   */

  if (odm_add_obj(TABLE(tbl_id),tbl_ptr)/* add the data to the data base*/
      == -1 )
    { return(VPD_SYS); };               /* if error in add, say so      */

  return(VPD_OK) ;                      /* else all is well.            */

} /* end vpdadd */
