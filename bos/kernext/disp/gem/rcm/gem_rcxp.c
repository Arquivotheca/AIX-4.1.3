static char sccsid[] = "@(#)98	1.2.1.6  src/bos/kernext/disp/gem/rcm/gem_rcxp.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:21:16";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_associate_rcxp
 *		iggm_create_rcxp
 *		iggm_delete_rcxp
 *		iggm_disassociate_rcxp
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
 *;CHANGE HISTORY
 */

#include "gemincl.h"
#include "gemrincl.h"


/*
 * FUNCTION: iggm_create_rcxp
 *
 * DESCRIPTION: Allocate a slot on the adapter for the GM specific data
 *  needed for this rcxp and initialize it.  Return the offset on the adapter 
 *  where it was allocated and the number of creates done for a part with
 *  the name passed in the arg (count not inclusive of this one).
 */

iggm_create_rcxp(gdp, pRcxp, arg, datap)
gscDevPtr          gdp;
rcxpPtr            pRcxp;
create_rcxp        *arg;
rGemrcxPtr         datap;
{
  int		   i;
  rGemrcxPtr       pGemrcx;
  rGemRCMPrivPtr   pDevP;
  rcx_node	   *tpnode;
  int		   slot_num, rc;
  
#ifdef GEM_DBUG
  printf("In iggm_create_rcxp: gdp=0x%x pRcxp=0x%x arg=0x%x datap=0x%x\n",
	 gdp, pRcxp, arg, datap);
  printf(" arg->pData=%x\n", arg->pData);
#endif

  rc = 0;

    /*
     * Initialize pointer to RCM private area
     */
    pDevP = &( ((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  
  /*
   * Test if part has not already been created
   */
  if (pRcxp->pData == NULL) {
    
    
#ifdef GEM_DBUG
    printf("%s %d pDevP=0x%x\n",__FILE__,__LINE__,pDevP);
#endif
    
    /*
     * Alloc storage for device specific data (rGemrcx structure)
     */
    if((pGemrcx = (rGemrcxPtr) xmalloc(sizeof(rGemrcx),3,pinned_heap)) == NULL) {
      TRACE(("Error in gem_create_rcxp: malloc rGemrcx\n"));
      return(GM_NOMEMORY);
    }
    
#ifdef GEM_DBUG
    printf("%s %d Copying fields to pGemrcx=0x%x\n",__FILE__,__LINE__,pGemrcx);
#endif
    
    /*
     * Copy desired fields from the temporarily allocated storage for
     * device specific data passed into RCM
     */
    pRcxp->pData             = (genericPtr) pGemrcx;
    pGemrcx->cxt_type        = datap->cxt_type;
    pGemrcx->favored_proc  = datap->favored_proc;
    pGemrcx->pCxt            = NULL;
    pGemrcx->Gemrcxp.name   = arg->id;
    pGemrcx->Gemrcxp.create_count  = 0;
    
    /*
     * Initialize RCM fields
     */
    pGemrcx->status_flags    = PINNED;
    pGemrcx->num_slots       = datap->num_slots;
    
    /*
     * Allocate slot
     */
    
    LOCK_SLOTS();
    
#ifdef GEM_DBUG
    printf("%s %d Calling find_best_slot: cxt type = %d\n",
	   __FILE__,__LINE__,pGemrcx->cxt_type);
#endif
    
    slot_num      = iggm_find_best_cxt_slot
      (
       pDevP,
       pGemrcx->cxt_type,
       pGemrcx->num_slots,
       pGemrcx->favored_proc,
       pDevP->slots
       );

#ifdef GEM_DBUG
    printf("%s %d Slot number = %d\n",__FILE__,__LINE__,slot_num);
#endif
    
    if(slot_num >= 0) {		/* Slot alloc was successful		    */
      pGemrcx->start_slot = slot_num;		/* Store slot number        */

      for (i=slot_num; i<slot_num+pGemrcx->num_slots; i++)
      { pDevP->slots[i].num_rcx++;
	pDevP->slots[i].status_flags = PINNED | ON_ADAPT;
	pDevP->slots[i].pHead = (rcx_node *)rMalloc(sizeof(rcx_node));
	pDevP->slots[i].pHead->pRcx = (rcxPtr)pRcxp;
	pDevP->slots[i].pHead->pNext = NULL;
      }

#ifdef GEM_DBUG
      printf("%s %d Store address of slot\n",__FILE__,__LINE__);
#endif
      
      pGemrcx->pASL = (ulong *)
	GM_OFST(pDevP->slots[slot_num].slot_addr);	/* Store address*/
      pGemrcx->pASL = (ulong *)( (ulong)pGemrcx->pASL & 0x0fffffff );
    }
    else {			/* Slot alloc was unsuccessful		*/
      xmfree(pGemrcx, pinned_heap);
      pRcxp->pData = NULL;
      rc = slot_num;
    }
    UNLOCK_SLOTS();
  }  

  if(rc == 0) {
    /*
     * Part is created
     */
    pGemrcx = (rGemrcxPtr)(pRcxp->pData);

    /*
     * Put offset (in global memory) of allocated slot and create_count
     * into user structure.  create_count is not inclusive of this call
     */

#ifdef GEM_DBUG
    printf("%s %d Put address of slot=%x into user space\n",
	   __FILE__,__LINE__,pGemrcx->pASL);
#endif
    suword(
	   (&((rGemrcxPtr)arg->pData)->pASL),/*User address		*/
	   pGemrcx->pASL 			/* Kernel variable      */
	   );

    suword(
	   (&((rGemrcxPtr)arg->pData)->Gemrcxp.create_count),/*User address*/
	   pGemrcx->Gemrcxp.create_count		/* Kernel variable   */
	   );

    pGemrcx->Gemrcxp.create_count++;
  }

  return(rc);
}
  

  iggm_delete_rcxp(gdp, pRcxp)
struct _gscDev     *gdp;
rcxpPtr	pRcxp;
{

  rGemrcxPtr     pGemrcx;
  rGemRCMPrivPtr pDevP;
  rGemrcxPtr     private_rcxPtr;
  CxtSlot        *pCslot;
  int		 i;

  if (pRcxp->pData != NULL)
  { /*
     * Initialize local pointers
     */
    pDevP = &( ((rGemDataPtr) (gdp->devHead.vttld))->GemRCMPriv);
    pGemrcx = (rGemrcxPtr)pRcxp->pData;			/* Gemrcx structure */
    pCslot = &(pDevP->slots[pGemrcx->start_slot]);	/* Slot array elem  */
    
    pGemrcx->Gemrcxp.create_count--;

    /*
     * Test if all users are through with this part
     */
    if(pGemrcx->Gemrcxp.create_count == 0)
    { LOCK_CXTSLOT(pCslot);

      /*
       * Remove slot allocation for this rcxp from slot array structure
       */
      for (i=pGemrcx->start_slot; i<pGemrcx->start_slot+pGemrcx->num_slots;
	   i++)
      { pDevP->slots[i].num_rcx--;
	pDevP->slots[i].status_flags = 0;
      }

      UNLOCK_CXTSLOT(pCslot);

#ifdef GEM_DBUG
      printf("%s %d pGemrcx=0x%x\n",__FILE__,__LINE__,pGemrcx);
#endif

      /*
       * Free Gemrcx structure
       */
      xmfree(pGemrcx, pinned_heap);
#ifdef GEM_DBUG
      printf("%s %d \n",__FILE__,__LINE__);
#endif
      pRcxp->pData = NULL;
#ifdef GEM_DBUG
      printf("%s %d \n",__FILE__,__LINE__);
#endif

    }
  }

  return(0);


}

iggm_associate_rcxp() {printf("In stub iggm_associate_rcxp\n"); return(0); }
iggm_disassociate_rcxp() {printf("In stub iggm_disassociate_rcxp\n"); return(0); }
