/* @(#)04       1.10  src/bos/usr/ccs/lib/libswvpd/vpddel.c, libswvpd, bos411, 9428A410j 10/28/93 14:28:20 */

/*
 * COMPONENT_NAME: LIBSWVPD
 *
 * FUNCTIONS: vpddel
 *            vpddelall
 *            vpddel_rec
 *
 * ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <swvpd0.h>


/*-----------------------------------------------------------------------
 * NAME: vpddel
 *
 * FUNCTION: Remove the first occurence of a single record in database
 *           table specified.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Remove the first occurence of a record from the database (specified
 *      by tbl_id) which matches certain search criteria.  The search cri-
 *      teria are determined by the key mask (which determines the key 
 *      search fields) applied to the record pointed to by rec_ptr.
 *      The table specified, modified by the current path setting, will
 *      be searched for a match as specified and the matching record
 *      will be removed.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *----------------------------------------------------------------------*/

int vpddel (int    tbl_id,   /* table reference code value */
            int    key_mask, /* field specification mask */
            void * rec_ptr)  /* pointer to record containing search criteria */ 
{
   int   rc;                            /* Hold return code for testing */
   char  qstr[MAX_QUERY_LEN];           /* hold odm query string        */
   char  wktbl[MAX_TBL_SIZE];           /* hold returned record from get*/
 
   /* open the table if its not already */
   if (T_OPEN(tbl_id) != TRUE)
   {
      /* initialize the table */
      if ((rc = vpdinit(tbl_id)) != VPD_OK)
         return (rc);
   }

   if (vpdbldsql(tbl_id, key_mask, rec_ptr, qstr) != VPD_OK)
      return (VPD_SYS);

   switch ((int)odm_get_first(TABLE(tbl_id), qstr, wktbl))
   {
      case 0:
           return (VPD_OK);    /* no matching record - ok */
           break;

      case -1:
           return (VPD_SYS);   /* odm error - return that */
           break;

      default:                 /* remove record just found */
           if (odm_rm_by_id (TABLE(tbl_id), ((lpp_t *)wktbl)->_id) == -1 )
              return (VPD_SYS);
           return (VPD_OK);
           break;
   } /* switch */

} /* vpddel */



/*------------------------------------------------------------------------
 * NAME: vpddelall
 *
 * FUNCTION: Delete all records that match the search criteria
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Find any records in the specified table, that match the search
 *      criteria and remove all of them.
 *
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 *------------------------------------------------------------------------*/

int vpddelall (int    tbl_id,    /* table reference code value */
               int    key_mask,  /* field specification mask */
               void * rec_ptr)   /* pointer to record containing search 
                                    criteria */
{
   int   rc;
   char  qstr[MAX_QUERY_LEN];    /* hold odm query string */

   /* open the table if its not already */
   if (T_OPEN (tbl_id) != TRUE)
   {
      /* initialize the table */
      if ((rc = vpdinit(tbl_id)) != VPD_OK)
         return (rc);
   }

   if (vpdbldsql(tbl_id, key_mask, rec_ptr, qstr) != VPD_OK)
      return (VPD_SYS);

   /* remove all matching records */
   rc = odm_rm_obj (TABLE (tbl_id), qstr);

   if (rc == -1)
      return (VPD_SYS);
   else
      return(VPD_OK);

} /* vpddelall */



/*-----------------------------------------------------------------------
 * NAME: vpddel_rec
 *
 * FUNCTION: Remove the record pointed to by rec_ptr in table specified
 *           tbl_id.
 *
 * EXECUTION ENVIRONMENT:
 *
 *          The record (pointed to by rec_ptr) has already been located in
 *          the table (specified by tbl_id) by a prior call to vpdget or
 *          vpdgetnxt.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *----------------------------------------------------------------------*/

int vpddel_rec (int    tbl_id,   /* Table reference code value. */
                void * rec_ptr)  /* Pointer to table struct of type specified
                                  * by tbl_id. */
{
   int   rc;

   /* open table if not already */
   if (T_OPEN(tbl_id) != TRUE)
   {
      /* initialize the table */
      if ( (rc = vpdinit(tbl_id)) != VPD_OK)
         return (rc);
   }

   /* remove the record pointed to be rec_ptr */
   if (odm_rm_by_id(TABLE(tbl_id), ((lpp_t *)rec_ptr)->_id) == -1 )
      return(VPD_SYS);

    return (VPD_OK);

} /* vpddel_rec */

