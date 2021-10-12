static char sccsid[] = "@(#)88	1.8.1.9  src/bos/kernext/disp/gem/rcm/gem_esfun.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:19:21";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		activate_cxt
 *		activate_cxt_fast
 *		load_dsv
 *		lock_imm_fifo_dom
 *		restore_pick_stack
 *		save_pick_stack
 *		store_dsv
 *		unlock_imm_fifo_dom
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
 *;LW 09/18/89 Inspection changes                                       
 *;MC 09/19/89 Added additional test for setting drp fifo ptrs	      @1
 *;            and added ifdef for checking fuword return code		
 *;lw 09/20/89 Changed load/store to always specify addr of dsv		
 *;lw 10/24/89 Added debug printfs                             		
 *;CL 11/01/89 Fixed prolog                                             
 *;LW 11/09/89 Add debug output to save dsv                             
 *;MC 11/29/89  Changed KERNEL to _KERNEL and removed KGNHAK		
 *;LW 12/13/89  Restructured hfiles				      @2
 *;LW 02/27/90   Add rp to insert_imm interface
 *;LW 02/28/90  Use pLocks in gemproc for @ of fifolocks
 *;LW 03/02/90  Fix gemlogs
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"

/*
 * FUNCTION: store_dsv                                                        
 *                                                                            
 * DESCRIPTION:                                                               
 *     put save_drawing_state_vector into buffer                              
 *                                                                            
 */
store_dsv(gdp, pRcx, pBuf)
struct _gscDev  *gdp;
rcxPtr          pRcx;
generic_vme	*pBuf;
{
  rGemRCMPrivPtr pDevP =		/* device private area in the RCM  */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  CxtSlot        *pCxtSlot;
  rGemrcxPtr	 pGemrcx;

#ifdef GEM_DBUG
  printf("%s @ %d: gdp=0x%x, pRcx=0x%x, pBuf=0x%x\n",
	 __FILE__,__LINE__,gdp, pRcx, pBuf);
#endif

  if (!pRcx || pRcx->flags & RCX_NULL)
    return(0);

  pGemrcx = (rGemrcxPtr)pRcx->pData;
  pCxtSlot = &(pDevP->slots[pGemrcx->start_slot]);

  pBuf->opcode = CE_SDSV;

  pBuf->len = sizeof(generic_vme);
  if (pGemrcx->cxt_type == IMM_RCXTYPE)
    pBuf->adr = VME_ADR(
			&(( (IMM_CONTEXT *)pCxtSlot->slot_addr)->dsv)
			);
  else
    pBuf->adr = VME_ADR(
			pCxtSlot->slot_addr + sizeof(TRAV_SLOT) - TRVDSVSIZ
			);
#ifdef GEM_DBUG
  printf("%s @ %d: Store dsv at: @=0x%x slot=0x%x bytes added to pBuf:0x%x\n",
	 __FILE__,__LINE__, pBuf->adr,((rGemrcxPtr)pRcx->pData)->start_slot,
  pBuf->len);
#endif

  return(  pBuf->len );
}


/*
 * FUNCTION: load_dsv                                                         
 *                                                                            
 * DESCRIPTION:                                                               
 *     put restore_drawing_state_vector structure element into fifo           
 */
load_dsv(gdp, pRcx, pBuf)
struct _gscDev  *gdp;
rcxPtr          pRcx;
generic_vme	*pBuf;
{
  rGemRCMPrivPtr pDevP =		/* device private area in the RCM   */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);

  CxtSlot        *pCxtSlot;
  rGemrcxPtr    pGemrcx;

#ifdef GEM_DBUG
  printf("%s @ %d: gdp=0x%x, pRcx=0x%x, pBuf=0x%x\n",
	 __FILE__,__LINE__,gdp, pRcx, pBuf);
#endif
  
  if (!pRcx || pRcx->flags & RCX_NULL)
    return(0);

  pGemrcx  = (rGemrcxPtr) pRcx->pData;
  pCxtSlot = &(pDevP->slots[pGemrcx->start_slot]);

  if(pGemrcx->status_flags & NEW_CXT) {
    pGemrcx->status_flags &= ~NEW_CXT;
    pBuf->len    = sizeof(long);
    pBuf->opcode = CE_SDDS;
  }
  else {
    
    pBuf->opcode = CE_LDSV;
    pBuf->len = sizeof(generic_vme);
    if (pGemrcx->cxt_type == IMM_RCXTYPE)
      pBuf->adr  = VME_ADR(
			   &(( (IMM_CONTEXT *)pCxtSlot->slot_addr)->dsv)
			   );
    else
      pBuf->adr = VME_ADR(
			  pCxtSlot->slot_addr + sizeof(TRAV_SLOT) - TRVDSVSIZ
			  );
  }    
#ifdef GEM_DBUG
  printf("%s @ %d: bytes added to pBuf:0x%x\n",
	 __FILE__,__LINE__, pBuf->len);
#endif
  return(pBuf->len);
}


/*
 * FUNCTION: save_pick_stack
 *
 * DESCRIPTION:
 *     put save_pick_attribute_stack into buffer
 *
 */
save_pick_stack(gdp, pRcx, pBuf)
struct _gscDev  *gdp;
rcxPtr          pRcx;
generic_vme	*pBuf;
{
  rGemRCMPrivPtr pDevP =		/* device private area in the RCM  */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  CxtSlot        *pCxtSlot;
  rGemrcxPtr	 pGemrcx;

#ifdef GEM_DBUG
  printf("%s @ %d: gdp=0x%x, pRcx=0x%x, pBuf=0x%x\n",
	 __FILE__,__LINE__,gdp, pRcx, pBuf);
#endif

  if (!pRcx || pRcx->flags & RCX_NULL)
    return(0);

  pGemrcx = (rGemrcxPtr)pRcx->pData;
  pCxtSlot = &(pDevP->slots[pGemrcx->start_slot]);

  pBuf->opcode = CE_SPSTK;

  pBuf->len = sizeof(generic_vme);
  pBuf->adr = VME_ADR(
		      ((TRAV_SLOT *)pCxtSlot->slot_addr)->pick_buffer
		      );

  return(  pBuf->len );
}


/*
 * FUNCTION: restore_pick_stack
 *
 * DESCRIPTION:
 *     put restore_pick_attribute_stack into buffer
 *
 */
restore_pick_stack(gdp, pRcx, pBuf)
struct _gscDev  *gdp;
rcxPtr          pRcx;
generic_vme	*pBuf;
{
  rGemRCMPrivPtr pDevP =		/* device private area in the RCM  */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  CxtSlot        *pCxtSlot;
  rGemrcxPtr	 pGemrcx;

#ifdef GEM_DBUG
  printf("%s @ %d: gdp=0x%x, pRcx=0x%x, pBuf=0x%x\n",
	 __FILE__,__LINE__,gdp, pRcx, pBuf);
#endif

  if (!pRcx || pRcx->flags & RCX_NULL)
    return(0);

  pGemrcx = (rGemrcxPtr)pRcx->pData;
  pCxtSlot = &(pDevP->slots[pGemrcx->start_slot]);

  pBuf->opcode = CE_RPSTK;

  pBuf->len = sizeof(generic_vme);
  pBuf->adr = VME_ADR(
		      ((TRAV_SLOT *)pCxtSlot->slot_addr)->pick_buffer
		      );

  return(  pBuf->len );
}


/*
 *  FUNCTION:	activate_cxt                                                  
 *                                                                            
 *  DESCRIPTION:                                                              
 *     put activate context structure element into fifo                       
 *                                                                            
 */
activate_cxt(gdp, pRcx, pBuf)
struct _gscDev  *gdp;
rcxPtr          pRcx;
generic_vme	*pBuf;
{
  rGemRCMPrivPtr pDevP =		/* device private area in the RCM   */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  CxtSlot        *pCxtSlot;

#ifdef GEM_DBUG
  printf("%s @ %d: gdp=0x%x, pRcx=0x%x, pBuf=0x%x\n",
	 __FILE__,__LINE__,gdp, pRcx, pBuf);
#endif
  
  if (!pRcx || pRcx->flags & RCX_NULL)
    return(0);

  pCxtSlot = &(pDevP->slots[((rGemrcxPtr)pRcx->pData)->start_slot]);

  pBuf->len = sizeof(generic_vme);
  pBuf->opcode = CE_ACTC;
  if (((rGemrcxPtr)pRcx->pData)->cxt_type == IMM_RCXTYPE)
    pBuf->adr  = VME_ADR(&((IMM_CONTEXT *)pCxtSlot->slot_addr)->asl);
  else
    pBuf->adr  = VME_ADR(&((TRAV_CONTEXT *)pCxtSlot->slot_addr)->asl);

#ifdef GEM_DBUG
  printf("%s @ %d: bytes added to pBuf:0x%x\n",
	 __FILE__,__LINE__, pBuf->len);
#endif

  return (pBuf->len); 
}

/*
 *  FUNCTION:	activate_cxt                                                  
 *                                                                            
 *  DESCRIPTION:                                                              
 *     put activate context FAST structure element into fifo               
 *                                                                            
 */
activate_cxt_fast(gdp, pRcx, pBuf)
struct _gscDev  *gdp;
rcxPtr          pRcx;
generic_vme	*pBuf;
{
  rGemRCMPrivPtr pDevP =		/* device private area in the RCM   */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  CxtSlot        *pCxtSlot;

#ifdef GEM_DBUG
  printf("%s @ %d: gdp=0x%x, pRcx=0x%x, pBuf=0x%x\n",
	 __FILE__,__LINE__,gdp, pRcx, pBuf);
#endif
  
  if (!pRcx || pRcx->flags & RCX_NULL)
    return(0);

  pCxtSlot = &(pDevP->slots[((rGemrcxPtr)pRcx->pData)->start_slot]);

  pBuf->len = sizeof(generic_vme);
  pBuf->opcode = CE_ACTF;
  if (((rGemrcxPtr)pRcx->pData)->cxt_type == IMM_RCXTYPE)
    pBuf->adr  = VME_ADR(&((IMM_CONTEXT *)pCxtSlot->slot_addr)->asl);
  else
    pBuf->adr  = VME_ADR(&((TRAV_CONTEXT *)pCxtSlot->slot_addr)->asl);

#ifdef GEM_DBUG
  printf("%s @ %d: bytes added to pBuf:0x%x\n",
	 __FILE__,__LINE__, pBuf->len);
#endif

  return (pBuf->len); 
}

/*
 *  FUNCTION:	lock_imm_fifo_dom
 *
 *  DESCRIPTION:
 *     Ensure that we're contedning with a 2d process for use of the fifo
 *
 */
  lock_imm_fifo_dom(pProc,pDevP)
rcmProc	*pProc;
rGemRCMPrivPtr	pDevP;
{
  static ulong false = FALSE;
  static ulong true  = TRUE;
  static ulong their_flag = FALSE;

  shmFifoPtr	pShmem;
  rGemprocPtr	pGemProcP;	/* gemini process private area              */
  int		rc = 0;
  pGemProcP = (rGemprocPtr)(pProc->procHead.pPriv);
  pShmem = pDevP->shmem;
  
#ifdef GEM_DBUG
HERE_I_AM;
#endif

  /*
   * Set our flag on
   */
  if( (rc = xmemout(&true,
	      &(pGemProcP->pLocks->rcm_lock),
	      sizeof(pGemProcP->pLocks->rcm_lock),
	      &pGemProcP->xs_fifolocks) )
     != XMEM_SUCC )
    {
      /* Write to user's space unsuccessful                        */
      END_CRIT();
      gemlog(NULL,"hispd3d","lock_imm_fifo_dom","xmemout",
	     rc,GM_MEMCPY,UNIQUE_1);
      return(ERROR);
    }

#ifdef GEM_DBUG
HERE_I_AM;
#endif

  do {
    /*
     * Get their flag
     */
    if( (rc = xmemin(&pGemProcP->pLocks->user_lock, &their_flag,
	       sizeof(their_flag), &pGemProcP->xs_fifolocks)) 
       != XMEM_SUCC ){
      /* transfer from user's space unsuccessfull                          */
      END_CRIT();
      gemlog(NULL,"hispd3d","lock_imm_fifo_dom","xmemin",
				    rc,GM_MEMCPY,UNIQUE_1);
      return(ERROR);
    }
  }
  while (their_flag == TRUE) ;
#ifdef GEM_DBUG
HERE_I_AM;
#endif


  return(0);
}

/*
 *  FUNCTION:	unlock_imm_fifo_dom
 *
 *  DESCRIPTION:
 *     Allow a 2d process access to the fifo
 *
 */
unlock_imm_fifo_dom(pProc)
rcmProc	*pProc;
{
  static ulong false = FALSE;
  static ulong true  = TRUE;
  int rc = 0;
  rGemprocPtr	pGemProcP;	/* gemini process private area              */
  pGemProcP = (rGemprocPtr)(pProc->procHead.pPriv);
  /*
   * turn our flag off
   */
#ifdef GEM_DBUG
HERE_I_AM;
#endif


  if( (rc = xmemout(&false,
	      &(pGemProcP->pLocks->rcm_lock),
	      sizeof(pGemProcP->pLocks->rcm_lock),
	      &pGemProcP->xs_fifolocks) )
     != XMEM_SUCC )
    {
      /* Write to user's space unsuccessful                        */
      END_CRIT();

     gemlog(NULL,"hispd3d","unlock_imm_fifo_dom","xmemout",
	     rc,GM_MEMCPY,UNIQUE_1);
      return(ERROR);
    }
#ifdef GEM_DBUG
HERE_I_AM;
#endif

  return(0);
}
