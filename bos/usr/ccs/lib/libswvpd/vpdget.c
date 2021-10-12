/* @(#)01       1.12  src/bos/usr/ccs/lib/libswvpd/vpdget.c, libswvpd, bos411, 9428A410j 6/11/93 11:53:58 */

/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: vpdget, vpdgetnxt, vpdchgget
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

/*
 * NAME: vpdget
 *
 * FUNCTION: Get single record in database table specified
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Locate the record in the database which matches the input buffer
 *      where specified.
 *      The table specified, modified by the current path setting, will
 *      be searched for a match as specified and the matching record
 *      will be returned in the caller's structure.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_NOMATCH - no matching record found.
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdget... */

int vpdget (int    tbl_id,    /* Table reference code value. */
            int    key_mask,  /* Field specification mask specifies fields which
                               * must match buffer. */
            void * tbl_ptr)   /* Pointer to table structure of type specified by
                               *  tbl_id. */

{                                       /* begin code for vpdchg        */

/* Local variables                                                      */


  int   rc ;                            /* Hold return code for testing */

  char  qstr[MAX_QUERY_LEN] ;           /* hold odm query string        */


/* Begin vpdget... */

  if (T_OPEN(tbl_id)!=TRUE)             /* if the table is not open, do */
    {                                   /* all required initialization  */
    if ( (rc = vpdinit(tbl_id)) != VPD_OK)
      { return(rc) ; }
    }

  if (vpdbldsql(tbl_id, key_mask, tbl_ptr, qstr) != VPD_OK)
    { return(VPD_SYS); }

  switch ( (int) odm_get_first(TABLE(tbl_id), qstr, tbl_ptr))
  {
  case 0:
    return(VPD_NOMATCH) ;               /* no more matching records     */
    break;
  case -1:
    return(VPD_SYS) ;                   /* error occurred               */
    break;
  default:
    return(VPD_OK) ;                    /* found another one            */
  } /* endswitch */

}



/*
 * NAME: vpdgetnxt
 *
 * FUNCTION: Get next record for table that matches prior criteria
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Find next record in the specified table, that match the search
 *      criteria specified earlier and return that record.
 *
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdgetnxt... */

int vpdgetnxt (int    tbl_id,   /* Table reference code value */
               void * tbl_ptr)  /* Pointer to table structure of type
                                 * specified by tbl_id. */

{                                       /* begin code for vpdchg        */

/* Local variables                                                      */


  int   rc ;                            /* Hold return code for testing */


/* Begin vpdgetnxt... */

  if (T_OPEN(tbl_id)!=TRUE)             /* if the table is not open, do */
    {                                   /* all required initialization  */
    if ( (rc = vpdinit(tbl_id)) != VPD_OK)
      { return(rc) ; }
    }

  rc = (int) odm_get_next(TABLE(tbl_id), tbl_ptr) ;
                                        /* get next record              */

  switch (rc) {
  case  0:
    return(VPD_NOMATCH) ;               /* no more matching records     */
    break;
  case -1:
    return(VPD_SYS) ;                   /* error occurred               */
    break;
  default:
    return(VPD_OK) ;                    /* found another one            */
  } /* endswitch */
}



/*
 * NAME: vpdchgget
 *
 * FUNCTION: Update record previously read.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The structure passed in must have been filled by a prior vpdget
 *      operation.  The corresponding record will be updated to reflect
 *      the current content of the structure.
 *
 * RETURNS: VPD_OK  - no errors, record added successfully
 *          VPD_SYS - some error occurred while dealing with odm.
 *
 */


/* Begin vpdchgget... */

int vpdchgget (int    tbl_id,   /* Table reference code value. */
               void * tbl_ptr)  /* Pointer to table structure of type
                                 * specified by tbl_id. */

{                                       /* begin code for vpdchg        */

/* Local variables                                                      */


  int   rc ;                            /* Hold return code for testing */


  if (T_OPEN(tbl_id)!=TRUE)             /* if the table is not open, do */
    {                                   /* all required initialization  */
    if ( (rc = vpdinit(tbl_id)) != VPD_OK)
      { return(rc) ; }
    }

  rc = odm_change_obj(TABLE(tbl_id), tbl_ptr) ;
                                        /* get next record              */

  if (rc == -1)                         /* if error during update       */
    { return(VPD_SYS) ;}                /* return error flag            */
  else
    { return(VPD_OK) ; }                /* else no error                */

} /* end vpdchgget */

