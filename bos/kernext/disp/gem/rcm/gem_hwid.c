static char sccsid[] = "@(#)70	1.6.1.11  src/bos/kernext/disp/gem/rcm/gem_hwid.c, sysxdispgem, bos411, 9428A410j 9/3/93 08:39:25";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		free_hwid
 *		get_hwid
 *		make_hwid_head
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



/*
 *; CHANGE HISTORY:                                                     
 *;MC 10/23/89 Created                                                  
 *;CL 11/01/89 Fixed prolog
 *;MC 11/03/89 Added gem_gai.h include
 *;MC 11/29/89 Changed KERNEL to _KERNEL and removed KGNHAK
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gem_gai.h"
#include "gem_geom.h"
ulong itof();
void make_hwid_head();

/*
 * FUNCTION:  get_hwid                                                  
 *                                                                      
 * DESCRIPTION:                                                         
 *     Allocate an unused hardware ID.  If all ID's are used, find one
 *     that hasn't been used for a while, and return it as the chosen ID,
 *     indicating that it needs to be stolen from it's current process by
 *     subtracting NUM_HWIDS from it.  Thus if the ID, which is returned
 *     in the id parameter, is negative, that indicates that the ID needs
 *     to be stolen, and the chosen ID can be obtained by adding NUM_HWIDS
 *     to the id value returned.
 */
Bool get_hwid(gdp, pRcx, pWG, buf_start)
struct _gscDev	 *gdp;
struct _rcx	 *pRcx;
struct _rcmWG	 *pWG;
char		 *buf_start;
{
  int		 i, tmp;
  int		 new_id;
  int		 good;
  char		 *pBuf = buf_start;
  rGemRCMPrivPtr pDevP =		/* device private are in the RCM*/
    &(((rGemDataPtr)gdp->devHead.vttld)->GemRCMPriv);
  ulong          seg_reg;

  seg_reg=BUSMEM_ATT(BUS_ID, 0x00);

  /*
   * Check for an unused hardware ID
   */

  GMBASE_INIT(seg_reg, pDevP);

  if (pDevP->num_free_hwid > 0)
  { 
    for (i = pDevP->hwid_tail; i != -1; i = pDevP->hwid[i].prev )
        if ( pDevP->hwid[i].pwg == NULL ) 
        {
           /*************************************************************/
           /* RJE:  This if statement has been changed to prevent a DSI */
           /*       during adapter "bringup".  We used to check for a   */
           /*       NULL pointer and then would de-reference it.        */
           /*************************************************************/
           if ( pDevP->hwid[i].currentlyUsed == 1 )
              continue;

           pDevP->hwid[i].pwg = pWG;
	   pDevP->hwid[i].currentlyUsed=1;

	   make_hwid_head(pDevP, i);
	   --pDevP->num_free_hwid;
	   ((rWGPrivPtr)pWG->pPriv)->hwid = i;

	   BUSMEM_DET(seg_reg);
	   return(FALSE);
        }
  }

  /*
   * Find hwid that has been least recently used, but make sure that it
   * isn't a hwid that's being used by a process that's current on
   * either domain
   */

  while ((*IMM_SYNC_CNTR < ((rWGPrivPtr)(pDevP->hwid[pDevP->hwid_tail].pwg)->pPriv)->imm_sync_cntr)
         || (*TRV_SYNC_CNTR < ((rWGPrivPtr)(pDevP->hwid[pDevP->hwid_tail].pwg)->pPriv)->trv_sync_cntr)
         || pDevP->hwid[pDevP->hwid_tail].currentlyUsed==1 )



  {

    make_hwid_head(pDevP, pDevP->hwid_tail);

  }

  BUSMEM_DET(seg_reg);
  new_id = pDevP->hwid_tail;
  ((rWGPrivPtr)pWG->pPriv)->hwid = new_id;
  make_hwid_head(pDevP, new_id);
  pDevP->hwid[new_id].currentlyUsed=1;
  if (pDevP->hwid[new_id].pwg==NULL) {
    pDevP->hwid[new_id].pwg = pWG;
    pDevP->num_free_hwid--;
    return(FALSE);
  }
  return(TRUE);

}


/*
 * FUNCTION:  free_hwid                                                  
 *                                                                      
 * DESCRIPTION:                                                         
 *            Free up an allocated ID
 */
void free_hwid(gdp, id)
struct _gscDev	 *gdp;
ulong		 id;
{
  int i;
  rGemRCMPrivPtr pDevP =		/* device private are in the RCM*/
    &(((rGemDataPtr)gdp->devHead.vttld)->GemRCMPriv);

  for (i = pDevP->hwid_head; i != -1; i = pDevP->hwid[i].next)
    if (i == id)
    { pDevP->hwid[id].pwg = NULL;
      pDevP->hwid[id].currentlyUsed = 0;
      ++pDevP->num_free_hwid;
      break;
    }
}

void make_hwid_head(pDevP, i)
rGemRCMPrivPtr	pDevP;
int		i;
{
  if (pDevP->hwid[i].prev != -1)
  { pDevP->hwid[pDevP->hwid[i].prev].next = pDevP->hwid[i].next;
    if (pDevP->hwid[i].next != -1)
      pDevP->hwid[pDevP->hwid[i].next].prev = pDevP->hwid[i].prev;
    else
      pDevP->hwid_tail = pDevP->hwid[i].prev;
    pDevP->hwid[i].next = pDevP->hwid_head;
    pDevP->hwid[i].prev = -1;
    pDevP->hwid[pDevP->hwid_head].prev = i;
    pDevP->hwid_head = i;
  }
}
