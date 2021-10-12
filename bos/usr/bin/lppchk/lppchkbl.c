static char sccsid[] = "@(#)35  1.1.1.2  src/bos/usr/bin/lppchk/lppchkbl.c, cmdswvpd, bos411, 9428A410j 6/15/93 09:59:33";
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: lppchk (main)
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

/*
 * NAME: lppchkbl (build lpp_id list)
 *
 * FUNCTION: Build a list of lpp_id numbers that match the search
 *           criteria specified.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Accesses the LPP data base that is currently the 'local'
 *      data base (as set by vpdlocal) and sets the input structure
 *      to contain the count and address of the list of lpp_id's which
 *      match the input lpp_name pattern.
 *
 * INPUT:
 *      lppname - pointer to the lpp_name search string.  May include
 *                ODM wild card characters.
 *      idlist  - pointer to structure used to access the lpp_id list.
 *
 * OUTPUT:
 *      idlist  - idcnt will be set to reflect the number of id's which
 *                match the lpp_name.
 *                ida will point to the allocated array that contains those
 *                lpp_id's.
 *
 * RETURNS: 0 - no errors detected
 *          non-zero - processing errors found.
 *
 */

#include        "lppchk.h"      /* local and standard defines           */


/* Begin lppchkbl                                                       */

int lppchkbl(
  char  *lppname,               /* pointer to the lppname search arg    */
  struct lpp_ids *ids )         /* pointer to return structure          */


  {
  lpp_t m_lpp ;                 /* local lpp structure used for search  */

  int   vpd_rc ;                /* return code from vpd routines        */

  int   maxa ;                  /* hold maximum number of ids in array  */

#define IDS_NM 100              /* initial number of ids in array       */
#define IDS_IN 50               /* number of ids to increment if needed */

/*----------------------------------------------------------------------*/

  TRC(TRC_ENTRY,"entering lppchkbl - lppname = %s\n",lppname,0,0) ;

  if (ids->ida != NULL)         /* if there is an old array of ids      */
    {                           /* release that storage                 */
    free(ids->ida) ;
    }

  maxa = IDS_NM ;               /* set initial size for array           */
  ids->ida = (struct idl *) malloc(maxa*sizeof(int)) ;
                                /* get initial storage block            */

  ids->idcnt = 0 ;              /* initialize number of found entries   */

  strncpy(m_lpp.name, lppname, sizeof(m_lpp.name)) ;
                                /* copy name into search structure      */

  vpd_rc = vpdget(LPP_TABLE, LPP_NAME, &m_lpp) ;
                                /* issue first get request              */

  while (vpd_rc == VPD_OK)
    {
    if (ids->idcnt >= maxa)     /* if allocated space is filled         */
      {
      maxa += IDS_IN ;          /* step number of elements to allocate  */
      ids->ida = (struct idl *) realloc(ids->ida, maxa * sizeof(int)) ;
      }

    ids->ida->id[ids->idcnt] = m_lpp.lpp_id ;
                                /* copy lpp_id to array                 */
    ids->idcnt += 1 ;           /* increment count                      */

    lppck_free_lpp(&m_lpp) ;	/* free vchar data from lpp structure	*/
    vpd_rc = vpdgetnxt(LPP_TABLE,&m_lpp) ;
                                /* search for more matching lpp_names   */
    }                           /* end while more entries that match    */

  if (vpd_rc != VPD_NOMATCH)    /* nomatch is normal - any other return */
    {                           /* is an error - report it and return   */
    MSG_S(MSG_SEVERE, MSG_CHK_VPDERR, DEF_CHK_VPDERR, vpd_rc, odmerrno, 0) ;
                                /* report the condition                 */
    ids->idcnt = 0 ;            /* ensure table is not used             */
    }                           /* end unexpected return code           */

  TRC(TRC_EXIT,"leaving lppchkbl - id count = %d\n",ids->idcnt,0,0) ;

  return(ids->idcnt) ;          /* return number if ids found           */
  }                             /* end lppchkbl                         */


/*
 * NAME: lppchkil (in list check)
 *
 * FUNCTION: Check for the specified lpp_id in the list passed.  
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Search the specified lpp_id list for the specified lpp_id.
 *      If found, return true, else return false.
 *
 * INPUT:
 *      idlist  - pointer to structure used to access the lpp_id list.
 *	id	- lpp_id to be searched for
 *
 * RETURNS: 0 - FALSE - id not in the list
 *	    1 - TRUE  - id in the list
 *
 */

int lppchkil(
  struct lpp_ids *ids ,         /* pointer to return structure          */
  int	 sid ) 			/* id to be searched for		*/

  {
  int	i ;			/* index into array of ids		*/
  for (i=0 ; i < ids->idcnt ; i++ ) 
    { if ( ids->ida->id[i] == sid ) 
        { return(TRUE) ; } 	/* if match found return true		*/
    }				/* end for each entry in list		*/
  return(FALSE) ;
  }				/* end lppchkil				*/

/*
 *   lppck_free_lpp
 *   lppck_free_prod
 *   lppck_free_inv
 *   
 * These three routines will free the space associated with the 
 * vchar fields of the respective ODM data strucutres.  While these 
 * routines have really nothing to do with the 'build list' functions
 * in this file, they are here to minimize the impact on the build 
 * process.  In the long run they should be replaced by functionally
 * equivalent routines in the libswvpd package but those do not exist 
 * at this time (8/92).  
 *
 * Input - each receives a pointer to the corresponding type of 
 *	   odm structure.
 *
 * Output - Each vhar field in the structure will have its associated
 *          data space released (using free) and the pointer will be 
 * 	    cleared in the structure.
 *
 */

int lppck_free_lpp (			/* lpp - group, description	*/
	lpp_t *lp ) 
{
  if (lp->group != (char *)NULL)	/* free group info if needed	*/
    {
    free(lp->group) ;
    lp->group = (char *) NULL ;
    }
  if (lp->description != (char *)NULL)  /* free description info	*/
    {
    free (lp->description) ;
    lp->description = (char *) NULL ;
    }
}					/* end lppck_free_lpp ;		*/


int lppck_free_inv (			/* inventory - loc1, loc2	*/
	inv_t *in ) 
{
  if (in->loc1 != (char *)NULL)
    {
    free(in->loc1) ;
    in->loc1 = (char *)NULL ;
    } 
  if (in->loc2 != (char *)NULL)
    {
    free(in->loc2) ;
    in->loc2 = (char *)NULL ;
    } 
}					/* end lppck_free_inv ;		*/


int lppck_free_prod (			/* product - name, fixinfo, prereq*/
	prod_t *ip ) 
{
  if (ip->fixinfo != (char *)NULL)
    {
    free(ip->fixinfo) ;
    ip->fixinfo = (char *)NULL ;
    } 
  if (ip->prereq != (char *)NULL)
    {
    free(ip->prereq) ;
    ip->prereq = (char *)NULL ;
    } 
  if (ip->name != (char *)NULL)
    {
    free(ip->name) ;
    ip->name = (char *)NULL ;
    } 
}					/* end lppck_free_lpp ;		*/
