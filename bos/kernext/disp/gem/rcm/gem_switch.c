static char sccsid[] = "@(#)98	1.5.1.5  src/bos/kernext/disp/gem/rcm/gem_switch.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:21:35";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_start_switch
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
/*;LW 09/01/89  commented out ltt in asl init                         @1*/
/*;LW 09/14/89  ifdef'd out original end_switch                         */
/*;             Added SCCS tag line                                     */
/*;             Moved rcmfillcontext to gem_icxt and renamed            */
/*;LW 10/11/89  removed unused pData param                              */
/*;LW 12/13/89  Restructured hfiles				        */
/*;**********************************************************************/

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"

#define LIGHT_SWITCH    1
#define HEAVY_SWITCH    0

iggm_start_switch( gdp, old_rcx, new_rcx, seq_num )
struct _gscDev  *gdp;
rcxPtr          old_rcx;
rcxPtr          new_rcx;
int             *seq_num;
{
  struct phys_displays  *pdp = gdp->devHead.display;
  int                   faulting_domain_num = new_rcx->domain;
  int                   rc=HEAVY_SWITCH;
  
  switch( faulting_domain_num )
    {
    case TRAVERSAL_FIFO_DOMAIN:
    case IMMEDIATE_CREG_DOMAIN:
      rc = HEAVY_SWITCH;
#ifdef GEM_DBUG
      printf("In iggm_start_switch TRAVERSAL_FIFO_DOMAIN & IMMEDIATE_CREG_DOMAIN case: domnum=%d\n",
	     faulting_domain_num);
      printf("Returning HEAVY_SWITCH rc=%d\n",rc);
#endif GEM_DBUG
      break;
      
    case IMMEDIATE_FIFO_DOMAIN:
      rc = LIGHT_SWITCH;
#ifdef GEM_DBUG
      printf("In iggm_start_switch IMMEDIATE_FIFO_DOMAIN case:  domnum=%d\n",
	     faulting_domain_num);
      printf("Returning LIGHT_SWITCH rc=%d\n",rc);
#endif GEM_DBUG
      break;
      
    default:
      rc=HEAVY_SWITCH;
#ifdef GEM_DBUG
      printf("In iggm_start_switch default case:  domnum=%d\n",
	     faulting_domain_num);
      printf("Returning HEAVY_SWITCH rc=%d\n",rc);
#endif GEM_DBUG
      break;
    }
  
  return( rc );

}

