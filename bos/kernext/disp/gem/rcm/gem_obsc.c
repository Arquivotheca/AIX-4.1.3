static char sccsid[] = "@(#)95	1.5.1.7  src/bos/kernext/disp/gem/rcm/gem_obsc.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:21:03";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_unobscured
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


/*;**********************************************************************/
/*;CHANGE HISTORY                                                       */
/*;                                                                     */
/*;CL 09/15/89  Created                                                 */
/*;LW 12/06/89  Added trace.  Added init of clip_flag                 @1*/
/*;                                                                     */
/*;**********************************************************************/
/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: iggm_unobscured                                      */
/*                                                                      */
/*  DESCRIPTION: This function determines if a window is currently      */
/*      unobscured.  By definition, a window is unobscured if it meets  */
/*      all of the following criteria:					*/
/*		   WinGeomAttr.pClip = NULL				*/
/*		              OR					*/
/*		   WinGeomAttr.pClip.numBoxes = 1  AND clip width &	*/
/*			 height equals WindowGeom width & height	*/
/*			      OR					*/
/*		   WinGeomAttr.pClip.numBoxes = 1 AND clip width &	*/
/*			 height equals all of WindowGeom width &	*/
/*			 height which is on the screen (ie: the clip	*/
/*			 region doesn't equal the window size ONLY	*/
/*			 because the window is partially off the screen)*/
/*									*/
/************************************************************************/


#include "gemincl.h"
#include "gemrincl.h"

  Bool iggm_unobscured(pWG)
gWinGeomAttrPtr	pWG;
{
  Bool			rc;
  ushort        	clip_width ;
  ushort        	clip_height ;
  gRegionPtr		pClip;
  gBoxPtr		pBox;

  pClip = pWG->pClip;

  if (pClip)
  {
    pBox = &pClip->pBox[0];
    clip_width = pBox->lr.x - pBox->ul.x ;
    clip_height = pBox->lr.y - pBox->ul.y ;

    if (pClip->numBoxes == 1 &&
	(clip_width == pWG->width ||
	 pBox->ul.x == 0 && pBox->lr.x == pWG->winOrg.x + pWG->width ||
	 pBox->ul.x == pWG->winOrg.x && pBox->lr.x == GM_WIDTH) &&
	(clip_height == pWG->height ||
	 pBox->ul.y == 0 && pBox->lr.y == pWG->winOrg.y + pWG->height ||
	 pBox->ul.y == pWG->winOrg.y && pBox->lr.y == GM_HEIGHT))
      rc = TRUE ;
    else
      rc = FALSE;
  } 
  else
    rc = TRUE;

#ifdef GEM_DBUG
 printf("iggm_unobscured: rc=0x%x\n", rc);				/*@1*/
#endif

  return(rc);

}
