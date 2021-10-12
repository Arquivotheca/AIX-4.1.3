static char sccsid[] = "@(#)86	1.5  src/bos/kernext/disp/gem/rcm/gem_client_clip.c, sysxdispgem, bos411, 9428A410j 5/28/93 15:08:43";

/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: iggm_do_client_clip
 *		iggm_same_clip_region
 *		iggm_update_clip_attr
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


  /**********************************************************************/
  /*     This file contains the required functions to implement the     */
  /*     performance enhancements for GTO's client clip function        */
  /*     (primarily for load3dm1/graPHIGS client clip implementation)   */
  /*     Please refer to ORBIT defect 78706 for more details.           */
  /**********************************************************************/

  /**********************************************************************/
  /*                                                                    */
  /*  FUNCTION NAME: iggm_do_client_clip                                */
  /*                                                                    */
  /*  DESCRIPTION:                                                      */
  /*                                                                    */
  /*    This routine will check whether it is necessary to perform     	*/
  /*	client clip:                                                    */
  /*    The current implementation of load3dm1 will send a NULL         */
  /*    client clip region whenever a user stops the cursor             */
  /*    movement while performing rubberbanding.  This function will    */
  /*    check for changes in either the window attributes or window     */
  /*    geometry to determine whether we need to reset the client       */
  /*    clip region for the current window.  This only addresses the    */
  /*    case when a sequential number of calls is made to the RCM       */
  /*    via an UPDATE_WIN_ATTR request from the analyze_window call     */
  /*    in the GTO RMS.                                                 */
  /*                                                                    */
  /*  INVOCATION: iggm_do_client_clip(curWA, newWA, curWG)              */
  /*                                                                    */
  /*                                                                    */
  /*  INPUT PARAMETERS:                                                 */
  /*                                                                    */
  /*    curWA - pointer to the window attributes for the current        */
  /*            rcx (i.e. pRcx->pWA)                                    */
  /*    newWA - pointer to the window attributes to be updated on       */
  /*            the current window                                      */
  /*    realWG - pointer to the window geometry for the current         */
  /*            rcx (i.e. pRcx->pWG)                                    */
  /*                                                                    */
  /*  RETURN VALUES:                                                    */
  /*                                                                    */
  /*            0 = DO NOT PERFORM CLIENT CLIP                          */
  /*            1 = DO CLIENT CLIP                                      */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/


#include <sys/sleep.h>
#include "gemincl.h"
#include "gemrincl.h"
#include "gem_gai.h"
#include "gem_geom.h"
#include "rcm_mac.h"

int iggm_do_client_clip();
int iggm_update_clip_attr();
int iggm_same_clip_region();


int iggm_do_client_clip(gdp, curWA, newWA, curWG, client_clip_mask)
struct _gscDev	*gdp;
struct _rcmWA   *curWA, *newWA;
struct _rcmWG   *curWG;
int             *client_clip_mask;

{

   create_win_attr *arg;
   int    sameWA = 0;
   int    sameWG = 0;

   if ( !curWA )
   {

#ifdef CHECK_CLIENT_CLIP
      printf("curWA = nothing\n");
#endif

      return(1);
   }
   else
      if ( !(((rWAPrivPtr)curWA->pPriv)) )
      {

#ifdef CHECK_CLIENT_CLIP
         printf("curWA->pPriv = nothing\n");
#endif

         iggm_create_win_attr(gdp, curWA, arg);
      }

#ifdef CHECK_CLIENT_CLIP
   printf("0: Current window ClientClipWA = 0x%x\n", ((rWAPrivPtr)curWA->pPriv)->ClientClipWA);
   printf("0: Current window groups_used  = 0x%x\n", ((rWAPrivPtr)curWA->pPriv)->groups_used);
#endif

   /***************************************************************/
   /* CASE 1:  The window attribute private client clip structure */
   /*          for the current context is NULL.  We will update   */
   /*          this structure with the data from the new window   */
   /*          attribute structure and proceed with the client    */
   /*          clip request.                                      */   
   /***************************************************************/

   if ( (((rWAPrivPtr)curWA->pPriv)->ClientClipWA == NULL) )
      if ( newWA && newWA->wa.pRegion && (newWA->wa.pRegion->numBoxes > 0) && newWA->wa.pRegion->pBox )
      {
         ((rWAPrivPtr)curWA->pPriv)->ClientClipWA = rMalloc( sizeof(GemWAClientClip) );
	 iggm_update_clip_attr( ((rWAPrivPtr)curWA->pPriv)->ClientClipWA, newWA, curWG, 1 );

#ifdef CHECK_CLIENT_CLIP
   printf("1: Current window ClientClipWA = 0x%x\n", ((rWAPrivPtr)curWA->pPriv)->ClientClipWA);
   printf("1: Current window groups_used  = 0x%x\n", ((rWAPrivPtr)curWA->pPriv)->groups_used);
#endif

	 return(1);
      }
      else
         return(1);
   

   /***************************************************************/
   /* CASE 2:  The window attributes needed to update the         */
   /*          current window have a NULL client clip region.     */
   /*          graPHIGS (load3dm1) may just be resetting the      */
   /*          region upon the user stopping to move the cursor.  */
   /*          We will check the window geometry in this case.    */
   /*          If it hasn't changed, we will not perform the      */
   /*          client clip.                                       */
   /***************************************************************/

   if ( !(newWA->wa.pRegion) || !(newWA) )
      return(1);


   /***************************************************************/
   /* CASE 3:  Default case.  We will make sure the geometry      */
   /*          of the window hasn't changed.  Then we will        */
   /*          check to make sure the client clip region          */
   /*          hasn't changed.                                    */
   /***************************************************************/

    sameWG = ( ((rWAPrivPtr)curWA->pPriv)->ClientClipWA->pWG == curWG );

#ifdef CHECK_CLIENT_CLIP
    printf("sameWG = 0x%x\n", sameWG);
#endif

    if ( !sameWG )
       ((rWAPrivPtr)curWA->pPriv)->ClientClipWA->pWG = curWG;


    if ( ((rWAPrivPtr)curWA->pPriv)->ClientClipWA->pRegion && (newWA->wa.pRegion->numBoxes > 0) && newWA->wa.pRegion->pBox )
       sameWA = iggm_same_clip_region( ((rWAPrivPtr)curWA->pPriv)->ClientClipWA->pRegion, 
                                       newWA->wa.pRegion);

#ifdef CHECK_CLIENT_CLIP
    printf("sameWA = 0x%x\n", sameWA);
#endif

    if ( sameWA && sameWG )
       *client_clip_mask = 1;
    else
       iggm_update_clip_attr( ((rWAPrivPtr)curWA->pPriv)->ClientClipWA, newWA, curWG, 0 );
       

     return( !sameWA );


}  /* end iggm_do_client_clip ... */



  /**********************************************************************/
  /*                                                                    */
  /*  FUNCTION NAME: iggm_update_clip_attr                              */
  /*                                                                    */
  /*  DESCRIPTION:                                                      */
  /*                                                                    */
  /*    This routine will update the private window attributes          */
  /*    "client clip area".  This is used to check for changes in       */
  /*    in the window attributes to determine whether to client clip    */
  /*    or not when a null client clip region is passed.                */
  /*                                                                    */
  /*  INVOCATION: iggm_update_clip_geom(curClientClipWA, curWA)         */
  /*                                                                    */
  /*                                                                    */
  /*  INPUT PARAMETERS:                                                 */
  /*                                                                    */
  /*    curClientClipWA - private window attributes                     */
  /*                                                                    */
  /*    curWA - pointer to the window attributes for the current        */
  /*            rcx (i.e. pRcx->pWA)                                    */
  /*                                                                    */
  /*  RETURN VALUES:                                                    */
  /*                                                                    */
  /*            0 = PRIVATE AREA IS NULL                                */
  /*            1 = PRIVATE AREA UPDATED WITH curWG VALUES              */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/

int iggm_update_clip_attr(curClientClipWA, newWA, curWG, first)
GemWAClientClipPtr    curClientClipWA;
struct _rcmWA         *newWA;
struct _rcmWG         *curWG;
int                   first;

{

   int i;


   if ( first )
   {
      curClientClipWA->pRegion = rMalloc( sizeof(gRegion) );
      curClientClipWA->pRegion->pBox = rMalloc(sizeof(gBox) * newWA->wa.pRegion->numBoxes);
   }

   curClientClipWA->pRegion->numBoxes = newWA->wa.pRegion->numBoxes;

   if ( !first )
   {
       rFree(curClientClipWA->pRegion->pBox);
       curClientClipWA->pRegion->pBox = rMalloc(sizeof(gBox) * newWA->wa.pRegion->numBoxes);
   }


   for ( i = 0; i < curClientClipWA->pRegion->numBoxes; i++ )
   {
       curClientClipWA->pRegion->pBox[i].ul.x = newWA->wa.pRegion->pBox[i].ul.x;
       curClientClipWA->pRegion->pBox[i].ul.y = newWA->wa.pRegion->pBox[i].ul.y;
       curClientClipWA->pRegion->pBox[i].lr.x = newWA->wa.pRegion->pBox[i].lr.x;
       curClientClipWA->pRegion->pBox[i].lr.y = newWA->wa.pRegion->pBox[i].lr.y;
   }

   curClientClipWA->pWG = curWG;


}  /* end iggm_update_clip_attr ... */



  /**********************************************************************/
  /*                                                                    */
  /*  FUNCTION NAME: iggm_same_clip_region                              */
  /*                                                                    */
  /*  DESCRIPTION:                                                      */
  /*                                                                    */
  /*    This routine will compare the private window client clip        */
  /*    region with that of the current window's.  If the regions       */
  /*    are the same, then there's no need to update the regions.       */
  /*                                                                    */
  /*  INVOCATION: iggm_same_client_clip_region                          */
  /*              (curClientClip, curReg)                               */
  /*                                                                    */
  /*                                                                    */
  /*  INPUT PARAMETERS:                                                 */
  /*                                                                    */
  /*    curClientClip   - private area  stored window                   */
  /*                                                                    */
  /*    curReg - current window region                                  */
  /*                                                                    */
  /*                                                                    */
  /*  RETURN VALUES:                                                    */
  /*                                                                    */
  /*            0 = DIFFERENT REGIONS                                   */
  /*            1 = SAME REGIONS                                        */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/

int iggm_same_clip_region(curClientClip, curReg)
gRegionPtr    curClientClip, curReg;

{

   int   i, sameRegion;

   sameRegion  = ( curClientClip->numBoxes == curReg->numBoxes );
   sameRegion &= ( curClientClip->numBoxes > 0 );

   if ( sameRegion )
      for ( i = 0; i < curClientClip->numBoxes; i++ )
        {
          sameRegion &= ( curClientClip->pBox[i].ul.x == curReg->pBox[i].ul.x );
	  sameRegion &= ( curClientClip->pBox[i].ul.y == curReg->pBox[i].ul.y );
	  sameRegion &= ( curClientClip->pBox[i].lr.x == curReg->pBox[i].lr.x );
	  sameRegion &= ( curClientClip->pBox[i].lr.y == curReg->pBox[i].lr.y );
        }

   return( sameRegion );


}  /* end iggm_same_clip_region ... */



