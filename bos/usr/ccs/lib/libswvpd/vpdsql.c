/* @(#)29       1.6  src/bos/usr/ccs/lib/libswvpd/vpdsql.c, libswvpd, bos411, 9428A410j 5/24/94 14:07:33 */

/*
 *   COMPONENT_NAME: LIBSWVPD
 *
 * FUNCTIONS: vpdbldsql
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <swvpd0.h>                     /* swvpd structures and externs */
                                        /* also includes odm structures */
#include <string.h>

/*
 * NAME: vpdbldsql
 *
 * FUNCTION: Build search argument string for odm request.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The caller supplies a buffer to receive the odm search string
 *      specified by the field mask and data structure.  That string
 *      will then be used with an odm search request operation.
 *
 * (NOTES:) Assumes that the buffer area to receive the query string
 *      is as large as the maximum search string as defined by
 *      MAX_SQL_LEN.  If the constructed string would exceed that
 *      limit, an error will be returned.
 *
 * (DATA STRUCTURES:) Relies on vpd_ctl for access to the control structures
 *      that define the tables used for access to the swvpd
 *
 * RETURNS:
 *      VPD_OK  - no error
 *      VPD_SQL - constructed string would exceed MAX_SQL_LEN
 *      VPD_KEY - one or more key fields implied by the key_mask
 *                are not valid for the table specified
 *      VPD_VCH - the key_mask specified one or more fields defined as
 *                vchar fields or binary fields and thus cannot be
 *                part of a search criteria.
 */

int vpdbldsql (int    tbl_id,   /* Table identifier key. */
               int    key_mask, /* Bit mask where least significant bit implies
                                 * the first field in the table and adjacent
                                 * bits imply the subsequent fields in the
                                 * table. */
               void * tbl_ptr,  /* Pointer to a structure that corresponds to
                                 * the type indicated by tbl_id. */
               char * buffer)   /* Character array to receive the search
                                 * string.  Must be at least MAX_SQL_LEN in
                                 * size. */
{
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*  Local variables                                             */

  struct ClassElem   *C_Elem_ptr ;      /* pointer to the array of class      */
                                        /* elements for the table type spec   */
                                        /* by tbl_id                          */

  int           i ;                     /* current field number in table      */
                                        /* index into class element table     */

  int           mask ;                  /* single bit mask that corresponds   */
                                        /* to i, used to test key_mask to     */
                                        /* determine if current column is part*/
                                        /* of the search criteria             */

  int           Q_len ;                 /* current length of the query        */

  char          *buffp ;                /* pointer to next free location in   */
                                        /* callers buffer computed just before*/
                                        /* adding the next field              */

  int           namelen ;               /* hold field name length             */

  short         short_value ;           /* hold short value from structure    */

  long          long_value ;            /* hold long integer from structure   */

  int           Num_cols ;              /* number of columns/fields in the    */
                                        /* current table type                 */

  char          *fld_ptr ;              /* pointer to current field in the    */
                                        /* table structure                    */

  char          * relop;

  char          * vchar_val;

#define AND_STR  " AND "
#define LIKE_STR " LIKE '"


/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

* buffer = '\0' ;                       /* init query buffer            */
if (key_mask != VPD_ALL) {              /* if not all records do work   */
                                        /* null query string will select*/
                                        /* all records (thus is ok now) */

  C_Elem_ptr = vpd_ctl.table_elem[tbl_id] ;
                                        /* access pointer to array of   */
                                        /* element info for the table   */

  Num_cols = vpd_ctl.table_cols[tbl_id];
                                        /* get number of columns/fields */
                                        /* in the table                 */

  for (i=0, mask=1;                     /* i is col index, mask is bit  */
       i<Num_cols && mask <= key_mask ; /* that corresponds to that col */
       i++ , mask <<= 1 ) {             /* loops thru all valid columns */

    if ((key_mask & mask) != 0) {       /* if key mask indicates col i  */

      Q_len = strlen(buffer) ;          /* compute (safely) current leng*/


                                        /* If not start of string, add  */
                                        /* " AND " to the query         */
      if (Q_len !=0)
        {
        if (Q_len >=                    /* ensure room available for    */
              MAX_QUERY_LEN -           /* " AND "                      */
              sizeof(AND_STR) )
          { return VPD_SQLMAX ; }

        strcat(buffer,AND_STR) ;        /* if room available add " AND "*/
        Q_len += (sizeof(AND_STR) - 1) ;/* step Q_len for added chars   */
                                        /* do not count null char       */
        }                               /* end if not first field       */

      fld_ptr = (char *) tbl_ptr +      /* compute address of field     */
                C_Elem_ptr[i].offset ;  /* using offset info computed by*/
                                        /* odm create                   */

      namelen = strlen(C_Elem_ptr[i].elemname) ;
                                        /* compute length of name to be */
                                        /* used in limit check.         */

      buffp = (char *) buffer + Q_len ; /* compute point to add to buff */

      switch (C_Elem_ptr[i].type) {     /* based on data type switch    */
      case ODM_CHAR:
        if (strpbrk (fld_ptr, "*[?") == (char *) 0)
           relop = "= '";
         else
           relop = LIKE_STR;
        if (Q_len >=                    /* ensure room for data         */
                MAX_QUERY_LEN -
                ( strlen (relop)+1+
                 namelen +              /* Length of field name         */
                 strlen(fld_ptr)        /* field data length            */
                ))
          { return VPD_SQLMAX ; }       /* if no room exit now          */

                                        /* add "<name> LIKE '<value>'   */
                                        /* to search string in buffer   */
        strcat(buffp,C_Elem_ptr[i].elemname) ;
        strcat(buffp,relop);
        strcat(buffp,fld_ptr);
        strcat(buffp,"'") ;
        break;

      case ODM_VCHAR:     /* In 4.1, search can be done on vchar fields */
        vchar_val  = *(char **)fld_ptr ;
        fld_ptr=vchar_val;

        if (strpbrk (fld_ptr, "*[?") == (char *) 0)
           relop = "= '";
        else
           relop = LIKE_STR;
        if (Q_len >=                    /* ensure room for data         */
                MAX_QUERY_LEN -
                ( strlen (relop)+1+
                 namelen +              /* Length of field name         */
                 strlen(fld_ptr)        /* field data length            */
                ))
          { return VPD_SQLMAX ; }       /* if no room exit now          */

                                        /* add "<name> LIKE '<value>'   */
                                        /* to search string in buffer   */
        strcat(buffp,C_Elem_ptr[i].elemname) ;
        strcat(buffp,relop);
        strcat(buffp,fld_ptr);
        strcat(buffp,"'") ;
        break;

      case ODM_SHORT:
        if (Q_len >=                    /* ensure room for data         */
                MAX_QUERY_LEN -
                ( 1+                    /* "=" relational operator      */
                 namelen +              /* Length of field name         */
                 5                      /* largest short value (32767)  */
                ))
          { return VPD_SQLMAX ; }       /* if no room exit now          */
        short_value = *(short *)fld_ptr;/* get field short value        */
        sprintf( buffp ,                /* buffer location to add at    */
                 "%s=%d",               /* name=value is search string  */
                 C_Elem_ptr[i].elemname,/* field name                   */
                 short_value) ;         /* value to be matched          */
        break;
      case ODM_LONG:
        if (Q_len >=                    /* ensure room for data         */
                MAX_QUERY_LEN -
                ( 1+                    /* "=" relational operator      */
                 namelen +              /* Length of field name         */
                 10                     /* largest int value            */
                ))
          { return VPD_SQLMAX ; }       /* if no room exit now          */
        long_value = *(int *)fld_ptr;   /* get field long value         */
        sprintf( buffp ,                /* buffer location to add at    */
                 "%s=%ld",              /* name=value is search string  */
                 C_Elem_ptr[i].elemname,/* field name                   */
                 long_value) ;          /* value to be matched          */
        break;
      default:
        return VPD_BADCOL ;             /* error - column type is vchar */
                                        /* or binary and cannot be used */
                                        /* in a search string           */
      } /* endswitch */

    } /* endif */                       /* end if mask matches key_mask */
  } /* endfor */                        /* end loop thru mask           */
} /* endif */                           /* end if mask is not VPD_ALL   */

if (vpd_ctl.vpd_debug) {
  printf("vpdbldsql:\n  tbl_id=%d\n  key_mask=%X\n  search=%s\n",
         tbl_id,key_mask,buffer);
} /* endif */

return VPD_OK ;                         /* all is well, return          */
}                                       /* end procedure vpdbldsql      */
