static char sccsid[] = "@(#)94	1.5.1.5  src/bos/kernext/disp/gem/rcm/gem_mvcxt.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:20:58";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		mvcxtoff
 *		mvcxton
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
/*;                                                                     */
/*; CHANGE HISTORY:                                                     */
/*;                                                                     */
/*;MC   09/05/89   Created                                            @#*/
/*;DM   09/05/89   Modified                                             */
/*;MC	09/14/89   Changed CURRENT to ON_ADAPT and SAVED to IN_KERNEL   */
/*;LW	09/15/89   Changed use of BUSY to lock/unlock CxtSlot           */
/*;LW 12/13/89  Restructured hfiles				        */
/*;LW 01/11/90  Set pCxt to NULL after free			        */
/*;                                                                     */
/*;**********************************************************************/
#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"		

/******************************************************************************/
/* FUNCTION: mvcxtoff                                                         */
/*                                                                            */
/* DESCRIPTION: 							      */
/*     move the contents of the adapter's context slot off the adapter into   */
/*    a kernel buffer.                                                        */
/******************************************************************************/
int
  mvcxtoff(pCslot, pRcx)
CxtSlot *pCslot;
rcxPtr	pRcx;
{
  
  rGemrcxPtr	pGemrcx;
  ulong		length;
  ulong		seg_reg;
  
  if(pRcx) {
  
    LOCK_CXTSLOT(pCslot);
    
    if (pCslot->status_flags & PINNED) {
      UNLOCK_CXTSLOT(pCslot);
      return(GM_PINNED);
    }
    
    if (! (pCslot->status_flags & ON_ADAPT)) {
      UNLOCK_CXTSLOT(pCslot);
      return(0);
    }
    
    pGemrcx = (rGemrcxPtr) pRcx->pData;
    
    pGemrcx->status_flags &= ~ON_ADAPT;
    
    length = pCslot->slot_len;				/* length of GM RCTX */
    pGemrcx->pCxt = (gem_cxt *)rMalloc(length);		/* get kernel buffer */

    seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

    pCslot->slot_addr &= 0x0fffffff;
    pCslot->slot_addr |= seg_reg;

#ifdef GEM_DBUG
printf("mvcxtoff: doing memcpy - target=%x  source=%x length=%x\n",
       pGemrcx->pCxt, pCslot->slot_addr, length);
#endif

    memcpy(pGemrcx->pCxt, pCslot->slot_addr, length);  /* save to kernel */

#ifdef GEM_DBUG
printf("mvcxtoff: finished memcpy\n");
#endif

    BUSMEM_DET(seg_reg);
    
    /*
     * update flags
     */

    pGemrcx->status_flags |= IN_KERNEL;
    pCslot->status_flags &= ~ON_ADAPT;
    
    UNLOCK_CXTSLOT(pCslot);
  }
  return(0);
}

/******************************************************************************/
/* FUNCTION: mvcxton                                                          */
/*                                                                            */
/* DESCRIPTION: 							      */
/*     Move an rcx's context onto the adapter.                                */
/******************************************************************************/
  int
  mvcxton(pCslot, pRcx)
CxtSlot *pCslot;
rcxPtr	pRcx;
{

  rGemrcxPtr 	 pGemrcx;
  rGemrcxPtr	 private_rcxPtr;
  int		 slot_num;
  int		 length;
  ulong		 seg_reg;


  LOCK_CXTSLOT(pCslot);

  pGemrcx  = (rGemrcxPtr) pRcx->pData;


  if (pCslot->status_flags & ON_ADAPT) {
    UNLOCK_CXTSLOT(pCslot);
    return(GM_ONADAPT);
  }

  if (pCslot->status_flags & PINNED) {
    UNLOCK_CXTSLOT(pCslot);
    return(GM_PINNED);
  }
  
  if ( ! (pGemrcx->status_flags & IN_KERNEL)) {
    UNLOCK_CXTSLOT(pCslot);
    return(GM_INKERNEL);
  }

  pGemrcx->status_flags &= ~IN_KERNEL;
  length = pCslot->slot_len;

  /*
   * Copy kernel copy of cxt onto adapter
   */

  if (pGemrcx->pCxt) {

    seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

    pCslot->slot_addr &= 0x0fffffff;
    pCslot->slot_addr |= seg_reg;

#ifdef GEM_DBUG
printf("mvcxton: doing memcpy - target=%x  source=%x length=%x\n",
       pCslot->slot_addr, pGemrcx->pCxt, length);
#endif

    memcpy(pCslot->slot_addr, pGemrcx->pCxt, length);

#ifdef GEM_DBUG
printf("mvcxton: finished memcpy\n");
#endif

    BUSMEM_DET(seg_reg);
    rFree(pGemrcx->pCxt);
    pGemrcx->pCxt = NULL;
  }

  pGemrcx->status_flags |= ON_ADAPT;
  pCslot->status_flags |= ON_ADAPT;

  UNLOCK_CXTSLOT(pCslot);

  return (0);
}
