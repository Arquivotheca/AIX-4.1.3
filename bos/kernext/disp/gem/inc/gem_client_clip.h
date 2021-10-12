/* @(#)88	1.1  src/bos/kernext/disp/gem/inc/gem_client_clip.h, sysxdispgem, bos411, 9428A410j 3/9/93 17:48:42 */

/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: 
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


/******************************************************************/
/* This file contains the required structures to implement the    */
/* performance enhancements for GTO's client clip function        */
/* (primarily for load3dm1/graPHIGS client clip implementation).  */
/* Please refer to ORBIT defect 78706 for more details.           */
/******************************************************************/

extern int iggm_do_client_clip();
extern int iggm_update_clip_attr();
extern int iggm_same_clip_region();

#include <sys/rcm.h>
#include <sys/rcm_win.h>

/**************************************/
/* Used to save window attribute data */
/**************************************/

typedef struct _GemWAClientClip
{
  rcmWG         *pWG;
  gRegionPtr    pRegion;
} GemWAClientClip, *GemWAClientClipPtr;

