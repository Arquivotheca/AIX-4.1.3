static char sccsid[] = "@(#)83	1.5.1.12  src/bos/kernext/disp/gem/rcm/gem_cwat.c, sysxdispgem, bos411, 9428A410j 4/26/94 18:24:03";


/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: iggm_create_win_attr
 *		iggm_delete_win_attr
 *		
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



/*;
 *;                                                                     
 *; CHANGE HISTORY:                                                     
 *;                                                                     
 *;CL   08/29/89   Created                                            @#
 *;MC   10/27/89   Changed malloc to rMalloc and free to rFree	      @1
 *;CL   11/01/89   Fixed prolog
 */

#include "gemincl.h"
#include "gemrincl.h"


iggm_create_win_attr(gdp, wap, arg)
struct _gscDev  *gdp ;
struct _rcmWA   *wap ;
create_win_attr *arg ;

{
  /* allocate space for device specific private data area */
  wap->pPriv = (genericPtr) rMalloc (sizeof (rWAPriv)) ;
  ((rWAPrivPtr)wap->pPriv)->num_groups    = 0;
  ((rWAPrivPtr)wap->pPriv)->active_group  = 0;
  ((rWAPrivPtr)wap->pPriv)->groups_used   = NULL;
  ((rWAPrivPtr)wap->pPriv)->gWAChangeMask = -1;
  ((rWAPrivPtr)wap->pPriv)->ClientClipWA  = NULL;

  return(0) ;
}






iggm_delete_win_attr(gdp, pwa)
struct _gscDev       *gdp ;
struct _rcmWA	     *pwa;

{

#ifdef GEM_DBUG
  printf("Entering iggm_delete_win_attr\n");
#endif

  /* free up device specific private data area */

  if ( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA )
  {

     if ( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA->pRegion->pBox )
        rFree( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA->pRegion->pBox );

     if ( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA->pRegion )
        rFree( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA->pRegion );

     rFree( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA );
     ((rWAPrivPtr)pwa->pPriv)->ClientClipWA = NULL;
  }

#if 0
  if ( ((rWAPrivPtr)pwa->pPriv)->groups_used )
     rFree( ((rWAPrivPtr)pwa->pPriv)->groups_used );
#endif

  rFree( pwa->pPriv );
  pwa->pPriv = NULL;

#ifdef GEM_DBUG
  printf("Leaving iggm_delete_win_attr\n");
#endif

  return(0) ;
}                  


