static char sccsid[] = "@(#)85	1.7.2.6  src/bos/kernext/disp/gem/rcm/gem_cxt.c, sysxdispgem, bos411, 9428A410j 6/23/93 09:04:06";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_create_rcx
 *		iggm_delete_rcx
 *		iggm_find_best_cxt_slot
 *		iggm_remove_from_linked_list
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
 *;MC   09/05/89   Created                                            @#
 *;DM   09/05/89   Modified                                             
 *;LW   09/13/89   Revised                                              
 *;LW   09/14/89   Changed function names, fixed compile errors         
 *;LW   09/19/89   Added setting of IN_KERNEL flag after init           
 *;JG   10/06/89   added ifdef kernel around assert calls             @1
 *;LW   10/25/89   removed setting of hwid                            @2
 *;CL   11/01/89   Fixed prolog
 *;MC   11/08/89   Fixed create_ and delete_rcx to handle null rcx's
 *;MC	11/29/89   Changed KERNEL to _KERNEL and removed KGNHAK
 *;LW	01/11/90   Get Gemrcx from pinned heap                   
 *;LW	01/18/90   Init new change masks in Gemrcx to force update geom/wind
 *;MC	01/22/90   Init comp mask and value fields
 *;MC   02/14/90   Made fix to find_best_cxt_slot		      @3
 *;LW   03/09/90   Removed call to check_rcxp_links 		      @4
 *;LW   03/16/90   Add code to put created rcx for trav fifo in proc  @5
 *;NGK  06/22/93   Free hwid in delete_rcx                        NGK 00
 */

#include "sys/sleep.h"
#include "gemincl.h"
#include "gemrincl.h"
#include "gem_geom.h"
#include "rcm_mac.h"

/*
 * FUNCTION: iggm_create_rcx                                              
 *
 * DESCRIPTION: Allocate a slot on the adapter for the GM specific data
 *  needed for this rcx and initialize it.
 */
iggm_create_rcx(gdp, rp, arg, datap)
struct _gscDev     *gdp;
struct _rcx        *rp;
create_rcx         *arg;
char               *datap;
{
  		
  rcmProcPtr	   pproc;
  rGemrcxPtr       pGemrcx;
  rGemRCMPrivPtr   pDevP;
  rGemrcxPtr       private_rcxPtr;
  CxtSlot          *CslotPtr;
  rcx_node	   *tpnode;
  int		   slot_num, rc;

#ifdef GEM_DBUG
  printf("In iggm_create_rcx: gdp=0x%x rp=0x%x arg=0x%x datap=0x%x\n",
	 gdp, rp, arg, datap);
  printf(" arg->pData=%x arg->pData->pASL=%x\n", arg->pData,
	 &((rGemrcxPtr)arg->pData)->pASL);
#endif
  rc = 0;

  /* if rcx is a null rcx */
  if (datap == NULL)
  {
    rp->pData = NULL;
    return(0);
  }

  /*
   * Initialize local vars
   */
  pDevP = &( ((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  
#ifdef GEM_DBUG
  printf("%s %d pDevP=0x%x gmbase=0x%x\n",__FILE__,__LINE__,pDevP,gmbase);
#endif

  /*
   * Alloc storage for device specific data (rGemrcx structure)
   */
  if((pGemrcx = (rGemrcxPtr) xmalloc(sizeof(rGemrcx),3,pinned_heap)) == NULL) {
    TRACE(("Error in gem_create_rcx: malloc rGemrcx\n"));
    return(GM_NOMEMORY);
  }

#ifdef GEM_DBUG
  printf("%s %d Copying fields to pGemrcx=0x%x\n",__FILE__,__LINE__,pGemrcx);
#endif

  /*
   * Copy desired fields from the temporarily allocated storage for
   * device specific data passed into RCM
   */
  private_rcxPtr        = (rGemrcxPtr) datap;   /* Get Ptr to temp copy */
  rp->pData             = (genericPtr) pGemrcx;
  pGemrcx->cxt_type        = private_rcxPtr->cxt_type & 0x0f;
  pGemrcx->favored_proc  = private_rcxPtr->favored_proc;
  pGemrcx->pCxt            = NULL;
  pGemrcx->GMChangeMask   = 0;
  pGemrcx->gWAChangeMask  = -1;
  pGemrcx->gWGChangeMask  = -1;

  /*
   * Initialize RCM fields
   */
  pGemrcx->status_flags    = 0x00000000;
  pGemrcx->num_slots       = 1;
  pGemrcx->win_comp_mask   = 0;
  pGemrcx->win_comp_val    = 0;

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

    CslotPtr = &pDevP->slots[slot_num];/* pointer to slot	            */
    ++CslotPtr->num_rcx;			/* increment use count	    */

#ifdef GEM_DBUG
    printf("%s %d Adding prcx to slot linked list\n",__FILE__,__LINE__);
#endif

    /*
     * add pointer to rcx to linked list of rcx's in slot
     */
    tpnode = CslotPtr->pHead;
    CslotPtr->pHead = (rcx_node *)rMalloc(sizeof(rcx_node));
    CslotPtr->pHead->pRcx = rp;
    CslotPtr->pHead->pNext = tpnode;

#ifdef GEM_DBUG
    printf("%s %d Store address of slot\n",__FILE__,__LINE__);
#endif

    pGemrcx->pASL  = (ulong *)
      GM_OFST(pDevP->slots[slot_num].slot_addr);	/* Store address*/
    pGemrcx->pASL = (ulong *)( (ulong)pGemrcx->pASL & 0x0fffffff );

#ifdef GEM_DBUG
    printf("%s %d Calling init_cxt\n",__FILE__,__LINE__);
#endif

    /*
     * initialize new context in kernel memory
     */
    if (pGemrcx->cxt_type == IMM_RCXTYPE)
      rc = gem_init_imm_cxt( pDevP, rp, datap );
    else if (pGemrcx->cxt_type == TRAV_RCXTYPE) 
      rc = gem_init_trav_cxt( pDevP, rp, datap );
    else
      rc = GM_BAD_RCXTYPE;
    if (rc == 0)
    {
#ifdef GEM_DBUG
      printf("%s %d Init_cxt returned successfully\n",__FILE__,__LINE__);
#endif
      pGemrcx->status_flags |= IN_KERNEL;
      pGemrcx->status_flags |= NEW_CXT;
#ifdef GEM_DBUG
      printf("%s %d Put address of slot=%x into user space\n",
	     __FILE__,__LINE__,pGemrcx->pASL);
#endif
      /*
       * Put offset (in global memory) of allocated slot into user structure
       */
      suword(
	     (&((rGemrcxPtr)arg->pData)->pASL),/*User address		*/
	     pGemrcx->pASL 			/* Kernel variable      */
	     );
    }
    else
    {
      xmfree(pGemrcx, pinned_heap);
      rp->pData = NULL;
    }
  }
  else {			/* Slot alloc was unsuccessful		*/
    xmfree(pGemrcx, pinned_heap);
    rp->pData = NULL;
    rc = slot_num;
  }

#if 0
  GM_UNLOCK_DEV(gdp);
#endif

#ifdef ONE_3D_WINDOW
      if (pGemrcx->cxt_type == TRAV_RCXTYPE) {
	if (pDevP->cur_3D_pid == NULL ||
	    pDevP->cur_3D_pid == pproc->procHead.pid)
	  pDevP->cur_3D_pid = pproc->procHead.pid;
	else 
	  rc = GM_MAX_3D;
      }
#endif

  return(rc);
}


/*
 * FUNCTION: iggm_delete_rcx                                                 
 *                                                                           
 * DESCRIPTION:                                                              
 *     delete device dependent buffers                                       
 *                                                                           
 */
iggm_delete_rcx(gdp, rp)
struct _gscDev    *gdp;
struct _rcx       *rp;
{
  rGemrcxPtr     pGemrcx;
  rGemRCMPrivPtr pDevP;
  rGemrcxPtr     private_rcxPtr;
  CxtSlot        *pCslot;
  disp_buf_sync	 *pDBS;
  char		 se_buffer[8192];
  char		 *pBuf = se_buffer;
  ulong		 seg_reg;
  rcmProcPtr	   pproc;

#if 0
  /*
   * Ensure other aixgsc calls that lock the device structure aren't running
   */
  FIND_GP(gdp,pproc);
  GM_LOCK_DEV(gdp,pproc);
#endif

  /* if not a null rcx */
  if (rp->pData != NULL)
  { 
    /*
     * Guard the domain do block context switch
     */
    GM_GUARD(rp->pDomain, rp->pProc);

    /*
     * Initialize local pointers
     */
    pDevP = &( ((rGemDataPtr) (gdp->devHead.vttld))->GemRCMPriv);
    pGemrcx = (rGemrcxPtr)rp->pData;			/* Gemrcx structure */
    pCslot = &(pDevP->slots[pGemrcx->start_slot]);	/* Slot array elem  */

    LOCK_CXTSLOT(pCslot);

    /*
     * Remove slot allocation for this rcx from slot array structure
     */
    if (pGemrcx->cxt_type == TRAV_RCXTYPE)
    { if (pDevP->trv_sync_cntr >= MAX_SYNC_CNTR)
	reset_sync_cntrs(gdp);
      else
      { pCslot->slot_lock = ++pDevP->trv_sync_cntr;
	if (rp->pWG && rp->pWG->pPriv)
	  ((rWGPrivPtr)rp->pWG->pPriv)->trv_sync_cntr =
	    pDevP->trv_sync_cntr;
	pDBS = (disp_buf_sync *)pBuf;
	pDBS->len = sizeof(disp_buf_sync);
	pDBS->opcode = CE_DBS;
	pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	pDBS->ofst = DISP_BUF_OFFSET;
	pDBS->flags = 0x2;
	pDBS->trv_cntr = pDevP->trv_sync_cntr;
	pBuf += sizeof(disp_buf_sync);

	if (pBuf - se_buffer)
	  { seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
	    WTFIFO( TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
	    BUSMEM_DET(seg_reg);
	  }
      }
    }

    if (pGemrcx->status_flags & ON_ADAPT)
      pCslot->status_flags = 0;	/* Make slot available		    */

    (pCslot->num_rcx)--;		/* Decrement use count		    */

    /* Maintain rcx list for this slot				    */
    iggm_remove_from_linked_list(pCslot,rp);

    UNLOCK_CXTSLOT(pCslot);

/* NGK 00 -- free the hwid */

    if (rp->pWG && rp->pWG->pPriv)
      free_hwid(gdp,((rWGPrivPtr)rp->pWG->pPriv)->hwid);

/* NGK 00 */

    /*
     * Free kernel copy of cxt if needed
     */
    if (pGemrcx->pCxt != NULL)
      rFree(pGemrcx->pCxt);	

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
    rp->pData = NULL;

#ifdef GEM_DBUG
    printf("%s %d \n",__FILE__,__LINE__);
#endif

  /*
   * Unguard the domain do allow context switch
   */
    GM_UNGUARD(rp->pDomain);

  }

#if 0
  GM_UNLOCK_DEV(gdp);
#endif
  return(0);
}


/*
 * FUNCTION: iggm_find_best_cxt_slot                                         
 *                                                                           
 * DESCRIPTION:                                                              
 *     find the best slot on the adapter to bind this particular rcx or rcx  
 *     part into							     
 *                                                                           
 */
int iggm_find_best_cxt_slot( pDevP, cxt_type, slots_needed, favored_proc,
			    slots )
rGemRCMPrivPtr	pDevP;
int     	cxt_type;
int     	slots_needed;
ulong   	favored_proc;
CxtSlot 	slots[];			/* GM slot information	*/
{
  int i;
  int slot;			/* currently best slot			   */
  int min_rcx;			/* number of rcx's bound to slot	   */
  ulong seg_reg;

  switch (cxt_type) {
    case IMM_RCXTYPE:
      /*
       * an immediate mode rendering context - find the slot with least
       * number of rcx's already bound to it, or the first slot with no
       * rcx's bound to it
       */
      slot = pDevP->first_imm_slot;
      min_rcx = slots[slot].num_rcx;
      i = slot + 1;

      while (i < pDevP->first_imm_slot + pDevP->num_imm_slots && min_rcx > 0)
      { if (slots[i].num_rcx < min_rcx)
        { slot = i;
          min_rcx = slots[i].num_rcx;
        }
        ++i;
      }

      break;
    case TRAV_RCXTYPE:
      /*
       * a traversal mode rendering context - find the slot with least
       * number of rcx's already bound to it, or the first slot with no
       * rcx's bound to it
       *
       * It is assumed that the calling routine will have done a
       * BEGIN_CRIT to prevent allocation of an rcx part to slot we just
       * decided is best before slot is bound
       */

      slot = pDevP->first_trav_slot;

      /*
       * initialize values
       */
      min_rcx = slots[slot].num_rcx;
      i = slot + 1;

      while (i < pDevP->first_trav_slot + pDevP->num_trav_slots && min_rcx > 0)
      { if (slots[i].num_rcx < min_rcx &&
	    !(slots[i].status_flags & PINNED))			    /*@3*/
	{ slot = i;
	  min_rcx = slots[i].num_rcx;
	}
	++i;
      }

      seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
      GMBASE_INIT(seg_reg, pDevP);
      while (*TRV_SYNC_CNTR < slots[slot].slot_lock)
	;
      BUSMEM_DET(seg_reg);

      break;
    case PART_RCXTYPE:
      /*
       * a context part - start at end of traversal/part slots and work
       * back until we find the number of empty contiguous slots
       * requested; always leave first slot for traversal contexts
       *
       * It is assumed that the calling routine will have done a
       * BEGIN_CRIT to prevent allocation of a trav cxt to a slot we
       * just decided is empty before slot is pinned
       */

      /*
       * all slots between i and slot are empty, slot = 0 if i is not
       * empty
       */
      slot = 0;
      i = pDevP->first_trav_slot + pDevP->num_trav_slots - 1;

      while (i > pDevP->first_trav_slot &&	/* still searching	*/
             (slot == 0 || slot - i < slots_needed))/* not enough slots */
      { if (slots[i].num_rcx > 0)		/* not empty		*/
	  /* reset the place holder					*/
          slot = 0;
        else if (slot == 0)
	  /* set new place holder					*/
          slot = i;
        --i;
      }

      if (slot - i < slots_needed)
      /* not enough contiguous empty slots to meet request		*/
        return(GM_NOSLOTS);
      else
      { /* successful							*/
	slot = i + 1;
	seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
        GMBASE_INIT(seg_reg, pDevP);
	for (i=slot; i<slot+slots_needed; ++i)
	  while (*TRV_SYNC_CNTR < slots[i].slot_lock)
	    ;
	BUSMEM_DET(seg_reg);
      }
      break;
    default:
      return(GM_BAD_RCXTYPE);
  }

  return(slot);
}


/*
 * FUNCTION: iggm_remove_from_linked_list                                    
 *                                                                           
 * DESCRIPTION:                                                              
 *     remove pointer to rp from linked list of rcx's in slot                
 *                                                                           
 */
iggm_remove_from_linked_list(SlotPtr,rp)
CxtSlot  *SlotPtr;
struct _rcx       *rp;
{
  rcx_node *pnode, *tp;

  assert(SlotPtr->pHead != NULL);
  
  if (SlotPtr->pHead->pRcx == rp)
  { tp = SlotPtr->pHead;
    SlotPtr->pHead = SlotPtr->pHead->pNext;
  }
  else
  { tp = NULL;

    /*
     * want pnode to be pointer to node before node which has pointer to rp
     */
    for (pnode = SlotPtr->pHead;
	 pnode->pNext != NULL;
	 pnode = pnode->pNext)
      if (pnode->pNext->pRcx == rp)	/* found node with pointer to rp */
      { tp = pnode->pNext;
	break;	/* out of for loop					*/
      }

    /*
     * make sure node was found
     */
    assert(tp != NULL);

    /*
     * set next pointer to point to node after one pointing to rp
     */
    pnode->pNext = pnode->pNext->pNext;
  }

  /*
   * free space allocated for node containing pointer to rp
   */
  rFree(tp);
}
