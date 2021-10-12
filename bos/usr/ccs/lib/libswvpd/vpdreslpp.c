/* @(#)28       1.4  src/bos/usr/ccs/lib/libswvpd/vpdreslpp.c, libswvpd, bos411, 9428A410j 6/11/93 11:54:26 */

/*
 * COMPONENT_NAME: (LIBSWVPD) Software Vital Product Data Management
 *
 * FUNCTIONS: vpdreslpp_name vpdreslpp_id
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <swvpd0.h>                      /* get structs/defines for swvpd*/
#include <string.h>
#include <stdlib.h>

/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/
/*  Static structures used to cache lpp_name/lpp_id pairs       */
/*<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>*/

#define CACHE_SIZE 10

  typedef char lpp_name_string[MAX_LPP_NAME] ;
                                        /* hold an lpp_name value       */

static lpp_name_string cnm[CACHE_SIZE*2] ;
                                        /* the cache of lpp_names seen  */
typedef struct {
  char *c_name ;                        /* pointer to cached lpp_name   */
  short c_id   ;                        /* cached id                    */
  } cni_t ;

static struct {
  int used_slot ;
  int next_slot ;
  cni_t cni [CACHE_SIZE] ;
  } csh[2]= { 0, 0,                     /* initial used/next            */
              cnm[0] , 0,               /* init to local struct addrs   */
              cnm[1] , 0,               /* and null id                  */
              cnm[2] , 0,
              cnm[3] , 0,
              cnm[4] , 0,
              cnm[5] , 0,
              cnm[6] , 0,
              cnm[7] , 0,
              cnm[8] , 0,
              cnm[9] , 0,
              0, 0,                     /* used/next for remote         */
              cnm[10] , 0,              /* string ptrs and id's for rem */
              cnm[11] , 0,
              cnm[12] , 0,
              cnm[13] , 0,
              cnm[14] , 0,
              cnm[15] , 0,
              cnm[16] , 0,
              cnm[17] , 0,
              cnm[18] , 0,
              cnm[19] , 0 } ;

#define  CN(n) csh[vpd_ctl.cur_path].cni[n].c_name
#define  CI(n) csh[vpd_ctl.cur_path].cni[n].c_id
#define  CS    csh[vpd_ctl.cur_path]

/*
 * NAME: vpdreslpp_name
 *
 * FUNCTION: Resolve an LPP name to its corresponding lpp_id
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The LPP name indicated by the first parameter will be located
 *      in the LPP data base (or the local cache) and the corresponding
 *      LPP id will be placed in the location indicated by the second
 *      parameter.
 *
 * (DATA STRUCTURES:)
 *      The local static structures defined above provide a cache that
 *      is searched before accessing the database.
 *
 * RETURNS:
 *      VPD_OK  - no error
 *      VPD_NOID - The specified LPP name is not in the LPP data base
 */

int vpdreslpp_name (char  * n_ptr, /* Pointer to lpp_name to be resolved to an
                                    * lpp_id. */
                    short * i_ptr) /* Pointer to field to hold the id for the
                                    * name specified. */
{

  struct lpp mylpp ;                    /* local lpp struct used in     */
                                        /* odm search for match         */

  int   nx ;                            /* cache table index            */

  int   rc ;                            /* return code temp             */

  for (nx = 0;                          /* search the cache table for   */
       nx < CS.used_slot ;              /* a match, used_slot is index  */
       nx++ )
    {                                   /* of last used slot            */
    if (strcmp(n_ptr,CN(nx)) == 0)      /* if matching name found       */
      {
      *i_ptr = CI(nx) ;                 /* set return value             */
      return(VPD_OK) ;                  /* return found match           */
      } /* endif */
  } /* endfor */

  strcpy(mylpp.name,n_ptr) ;            /* move name to local structure */
  rc = vpdget(LPP_TABLE, LPP_NAME, (char *) &mylpp) ;
                                        /* get lpp entry that matches   */
                                        /* name passed in to this rtn   */

  if (rc == VPD_OK) {
    strncpy(CN(CS.next_slot),n_ptr,MAX_LPP_NAME) ;
                                        /* copy name to cache table     */
    CI(CS.next_slot) = mylpp.lpp_id ;   /* copy id into cache table     */
    *i_ptr = mylpp.lpp_id ;             /* set id into callers field too*/
    CS.next_slot = (CS.next_slot + 1) % CACHE_SIZE ;
                                        /* compute next slot to fill    */
    if (CS.used_slot != CACHE_SIZE) {
      CS.used_slot += 1 ;
    } /* endif */
    return(VPD_OK) ;                    /* return the lpp_id found      */
  } /* endif */
  return (VPD_NOID) ;                   /* indicate name not in lpp tbl */
}                                       /* end function vpdreslpp_name  */


/*
 * NAME: vpdreslpp_id
 *
 * FUNCTION: Resolve an LPP id to its corresponding LPP name
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The LPP id indicated by the first parameter will be located
 *      in the LPP data base (or the local cache) and the corresponding
 *      LPP name will be placed in the location indicated by the second
 *      parameter.
 *
 * (DATA STRUCTURES:)
 *      The local static structures defined above provide a cache that
 *      is searched before accessing the database.
 *
 * RETURNS:
 *      VPD_OK  - no error
 *      VPD_NOID - The specified LPP name is not in the LPP data base
 */

char * vpdreslpp_id (short   l_id,   /* Lpp_id to be matched. */
                     char  * n_ptr)  /* Caller area to receive name. */
{

  struct lpp mylpp ;                    /* local lpp struct used in     */
                                        /* odm search for match         */

  int   nx ;                            /* cache table index            */

  int   rc ;                            /* return code temp             */

  for (nx = 0;                          /* search the cache table for   */
       nx < CS.used_slot ;              /* a match, used_slot is index  */
       nx++ )                           /* of last used slot            */
    {
    if (CI(nx) == l_id) {               /* if matching id found in cache*/
      strncpy(n_ptr,CN(nx),MAX_LPP_NAME) ;
                                        /* copy string to caller area   */
      return(VPD_OK) ;                  /* return pointer to corr. name */
      } /* endif */
    } /* end for */                     /* end loop thru cached ids     */

  mylpp.lpp_id = l_id ;                 /* set lpp struct with lpp_id   */
  rc = vpdget(LPP_TABLE,LPP_LPPID, (char *) &mylpp) ;/* use to locate */
                                        /* record in the data base      */

  if (rc == VPD_OK) {                   /* if match - add to cache      */
    strncpy(CN(CS.next_slot),mylpp.name,MAX_LPP_NAME) ;
    strncpy(n_ptr                ,mylpp.name,MAX_LPP_NAME) ;
                                        /* copy to cache and caller     */
    CI(CS.next_slot) = l_id ;
    CS.next_slot = (CS.next_slot+1) % CACHE_SIZE ;
                                        /* compute next slot to fill    */
    if (CS.used_slot != CACHE_SIZE)
      { CS.used_slot += 1 ;}

    return ((char *) VPD_OK);           /* return ptr to cached name */
    } /* endif */
  return ((char *) VPD_NOID) ;          /* return error code */

} /* end vpdreslpp_id */

/*
 * NAME: vpdresclr
 *
 * FUNCTION: Clear the cache for the specified path
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Clear the cache entries that are associated with the specified
 *      vpd path.  This is necessary if the path used for either remote
 *      or local vpd access is modified.
 *      parameter.
 *
 * (DATA STRUCTURES:)
 *      The local static structures defined above provide a cache that
 *      is searched before accessing the database.
 *
 * RETURNS:
 *      VPD_OK  - no error
 */

int vpdresclr (int newpath)   /* Either VPD_LOCAL or VPD_REMOTE. */

{
  csh[newpath].used_slot = 0 ;          /* clear the used slots in cache*/
  csh[newpath].next_slot = 0 ;          /* reset where to start using it*/
  return (VPD_OK) ;                     /* no error, just return        */

} /* vpdresclr */
