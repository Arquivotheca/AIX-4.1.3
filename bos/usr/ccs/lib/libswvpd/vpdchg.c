/* @(#)00       1.10  src/bos/usr/ccs/lib/libswvpd/vpdchg.c, libswvpd, bos411, 9428A410j 6/11/93 11:53:31 */

/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: vpdchg, vpdchgall, vpdchgadd
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
#include <string.h>
#include <stdlib.h>

/*
 * NAME: vpdchg
 *
 * FUNCTION: Change single record in database table specified
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Change the record in the database which matches the input buffer
 *      where specified, to match the complete table entry in the buffer.
 *      The table specified, modified by the current path setting, will
 *      be searched for a match as specified and the matching record
 *      will be updated to the content of the buffer.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdchg... */

int vpdchg (int    tbl_id,     /* Table reference code value. */
            int    key_mask,   /* Field specification mask.  Specifies fields
                                * which must match buffer. */
            void * tbl_ptr)    /* Pointer to table structure of type
                                * specified by tbl_id. */
{                                       /* begin code for vpdchg        */

/* Local variables                                                      */


  int   rc ;                            /* Hold return code for testing */

  char  qstr[MAX_QUERY_LEN] ;           /* hold odm query string        */

  char  wktbl[MAX_TBL_SIZE] ;           /* hold returned record from get*/


/* Begin vpdchg... */

  if (T_OPEN(tbl_id)!=TRUE)             /* if the table is not open, do */
    {                                   /* all required initialization  */
    if ( (rc = vpdinit(tbl_id)) != VPD_OK)
      { return(rc) ; }
    }

  if (vpdbldsql(tbl_id, key_mask, tbl_ptr, qstr) != VPD_OK)
    { return(VPD_SYS); }

  switch ((int)odm_get_first(TABLE(tbl_id), qstr, wktbl))
  {
  case 0:
    return(VPD_NOMATCH) ;               /* no matching record - ok      */
    break;
  case -1:
    return(VPD_SYS) ;                   /* odm error - return that      */
    break;
  default:
    memcpy(tbl_ptr, wktbl, sizeof(struct ClassHdr)) ;
                                        /* move ODM control info from   */
                                        /* record returned to users data*/
    if (odm_change_obj(TABLE(tbl_id), tbl_ptr) == -1 )
      { return(VPD_SYS) ; }

    return (VPD_OK) ;

    break;
  }                                     /* end switch                   */
}



/*
 * NAME: vpdchgall
 *
 * FUNCTION: Change all records that match the search criteria
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Find any records in the specified table, that match the search
 *      criteria and change all of them to match the record input.
 *
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdchgall... */

int vpdchgall (int    tbl_id,     /* Table reference code value. */
               int    key_mask,   /* Field specification mask.  Specifies fields
                                   * which must match buffer. */
               void * tbl_ptr)    /* Pointer to table structure of type
                                   * specified by tbl_id. */
                                        /* of type specified by tbl_id  */

{                                       /* begin code for vpdchg        */

/* Local variables                                                      */


  int   rc ;                            /* Hold return code for testing */

  char  qstr[MAX_QUERY_LEN] ;           /* hold odm query string        */

  char  wktbl[MAX_TBL_SIZE] ;           /* hold returned record from get*/


/* Begin vpdchg... */

  if (T_OPEN(tbl_id)!=TRUE)             /* if the table is not open, do */
    {                                   /* all required initialization  */
    if ( (rc = vpdinit(tbl_id)) != VPD_OK)
      { return(rc) ; }
    }

  if (vpdbldsql(tbl_id, key_mask, tbl_ptr, qstr) != VPD_OK)
    { return(VPD_SYS); }

  rc = (int) odm_get_first(TABLE(tbl_id), qstr, wktbl) ;
                                        /* get first record             */

  while ((rc != 0) && (rc != -1))
    {
    memcpy(tbl_ptr, wktbl, sizeof(struct ClassHdr)) ;
                                        /* move ODM control info from   */
                                        /* record returned to users data*/
    if (odm_change_obj(TABLE(tbl_id), tbl_ptr) == -1 )
      { return(VPD_SYS) ; }             /* update the record            */

    rc = (int) odm_get_next(TABLE(tbl_id),wktbl) ;
                                        /* get next matching record     */
    }                                   /* endwhile */

  if (rc != 0)                       /* if end is - not no more data */
    { return(VPD_SYS) ; }               /* indicate error               */
  else
    { return(VPD_OK) ; }                /* end or no matching data = ok */
}

/*
 * NAME: vpdchgadd
 *
 * FUNCTION: Change single record in database table specified add if needed
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Change the record in the database which matches the input buffer
 *      where specified, to match the complete table entry in the buffer.
 *      The table specified, modified by the current path setting, will
 *      be searched for a match as specified and the matching record
 *      will be updated to the content of the buffer. If no matching record
 *      is found, the input record will be added to the table.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdchgadd... */

int vpdchgadd (int    tbl_id,   /* Table reference code value. */
               int    key_mask, /* Field specification mask.  Specifies fields
                                 * which must match buffer. */
               void * tbl_ptr)  /* Pointer to table structure of type specified
                                 * by tbl_id. */

{                                       /* begin code for vpdchg        */

/* Local variables                                                      */


  int   rc ;                            /* Hold return code for testing */

/* Begin vpdchgadd... */

  if ((rc=vpdchg(tbl_id, key_mask, tbl_ptr)) == VPD_NOMATCH)
    {                                   /* if no match found - add      */
    rc = vpdadd(tbl_id, tbl_ptr) ;      /* then add to table            */
    } /* endif */

  return(rc) ;

} /* end vpdchgadd */
