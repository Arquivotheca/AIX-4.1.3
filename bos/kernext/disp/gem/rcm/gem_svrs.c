static char sccsid[] = "@(#)21	1.1.2.12  src/bos/kernext/disp/gem/rcm/gem_svrs.c, sysxdispgem, bos411, 9428A410j 1/28/93 14:15:22";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_restore_state
 *		gem_save_state
 *		set_WLUT
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
 *;CHANGE HISTORY                                                       
 *;                                                                     
 *;MC 04/05/90  Created                                                 
 *;MC 04/18/90  Modified for Gemini II
 *;                                                                     
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"
#include "gem_geom.h"
#include "rcm_mac.h"

#define TAG_ofst	0x78
#define Curs_Lock_ofst	0x1c8
#define GCP_Fifo_ofst	0x78
#define DRP_Fifo_ofst	0x80
#define CTR_REG_CLEAR	3
#define STRUCT_LEV_OFST	860

void set_WLUT();

/* Typedef for communications save area					*/
typedef struct _commSave {
  ulong		gcpCommSave[GMCommAreaSize/4];
  ulong		shpCommSave[GMCommAreaSize/4];
  ulong		drpCommSave[GMCommAreaSize/4];
  char		cursorSave[NUMCSRPAT][CURSPMSIZ];
  char		rcmPrivSave[RCMPrivCxtSize];
  int		numdsv;
  char		*dsvptr;
  int		sefifosize;
  char		*sefifodata;
  char		datafifo[FifoLen];
  ulong		dataReadPtr;
  ulong		dataInUseCount;
  ulong		trav_rcx;
  rcxPtr	*slot_cxts;
} commSave, *commSavePtr;

/* Typedef for conditional return structure element			*/
typedef struct _cond_ret {
	ushort	len;
	ushort	opcode;
	ulong	mask;
	ulong	cond;
} cond_ret;

/* Typedef for execute structure element				*/
typedef struct _execut {
	ushort	len;
	ushort	opcode;
	ulong	strid;
	ulong	rsvd1;
	ulong	rsvd2;
} execut;

/*
 * FUNCTION NAME: gem_save_state
 *                                                                     
 * DESCRIPTION: Saves any contexts or RCX parts currently on the adapter to
 *		kernel memory.
 *		   NB: This function assumes that the server has done a
 *		lock_adapter and is not itself writing to the adapter.
 */
gem_save_state(gdp)
struct _gscDev	*gdp;
{
  int		i, rc;		/* loop counter				*/
  int		offset;
  int		buf_len;	/* buffer length			*/
  int		length;		/* slot length				*/
  volatile ulong *dp;		/* control register			*/
  ulong		wp;		/* write pointer			*/
  ulong		*garp;		/* geographical address register pointer*/
  ulong		*tserpp;	/* ptr to trav se fifo read ptr		*/
  ulong		*crp;		/* control reg pointer			*/
  ulong		*tdp;		/* ptr to trav data fifo		*/
  ulong		*dsvp;		/* drawing state cector ptr		*/
  ulong		*pcur;		/* pointer to curser			*/
  ulong         features;       /* check if SHP exists                  */
  ulong		seg_reg;
  uint		oldlevel;
  devDomainPtr	pDom;		/* pointer to domain			*/
  rcxPtr	rp;		/* pointer to rcx			*/
  rGemrcxPtr	pGemrcx;	/* pointer to rcx private area		*/
  rcx_node	*pnode;		/* pointer to rcx node			*/
  char		se_buffer[1024];/* small temporary buffer		*/
  char		*pBuf;
  char		*pdsv;
  cond_ret	*pCr;
  commSavePtr	pSave;		/* pointer to comm save area		*/
  ulong		*plong;
  rcmProcPtr	pProc;
  struct phys_displays *pd = gdp->devHead.display;
  rGemDataPtr	pgd 	= (rGemDataPtr)(gdp->devHead.vttld);
  rGemRCMPrivPtr pDevP  = &pgd->GemRCMPriv;

#ifdef GEM_DBUG
  printf("Enter gem_save_state: gdp = 0x%x\n", gdp);
#endif

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  GMBASE_INIT(seg_reg, pDevP);

  features = ((struct gem_dds *)pd->odmdds)->features; /* to check for SHP */

  /*
   * Ensure that no one else changes slot information while we're
   * saving the contents
   */
  LOCK_SLOTS();

#ifdef NEW_HOTKEY

  /*
   * The following code is currently unnecessary, as the RMS lock_adapter
   * routine is calling the AIXGSC lock adapter.  (It used to just disable
   * the 3D fifo.)
   */

  GMBASE_INIT(seg_reg, pDevP);
  dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.reset_3d;
  while (*dp) ;
  dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.flush_drp;
  *dp = 1;

  pBuf = se_buffer;
  STOP_OTHR_FIFO( pBuf, 3, TRUE );
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );

  while (*dp) ;

#endif

  pDevP->pCommSave = rMalloc(sizeof(commSave));
  if (pDevP->pCommSave == NULL)
  { UNLOCK_SLOTS();
    BUSMEM_DET(seg_reg);
    return(GM_NOMEMORY);
  }
  pSave = (commSavePtr)pDevP->pCommSave;

  /* 
   * Find current immediate context and save its DSV.
   */
  rp = gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur;

#ifdef GEM_DBUG
  printf(" Current immedaite rcx = 0x%x\n", rp);
#endif

  if (rp)
  { pBuf = se_buffer;
    buf_len = store_dsv(gdp, rp, pBuf);
    if (buf_len < 0)
    { UNLOCK_SLOTS();
      BUSMEM_DET(seg_reg);
      rFree(pSave);
      return(buf_len);
    }
    pBuf += buf_len;

#ifndef NEW_HOTKEY
    STOP_OTHR_FIFO( pBuf, 252, FALSE );
#endif

    WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  }

#ifdef GEM_DBUG
  printf("Waiting for IMM fifo - stop other fifo\n");
#endif

  /*
   * Wait for immediate fifos to drain
   */
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  FIFO_EMPTY(ImmDataFifo, seg_reg, pDevP);

#ifdef GEM_DBUG
  printf("IMM fifo empty\n");
#endif

  /*
   * Save any remaining data in traversal fifo
   */

  /* set dp to point to the geographical address register		*/

  GMBASE_INIT(seg_reg, pDevP);
  garp = (ulong *)(pDevP->gmbase + GAR_ofst);

  /* Select the GCP processor						*/

  *garp = pDevP->gcp_geo;
  IUR_INIT(TravSeFifo, pDevP);
  pSave->sefifosize = *IUR_P[TravSeFifo] & IURMASK;

#ifdef GEM_DBUG
  printf("Trav se in use count = 0x%x\n", pSave->sefifosize);
#endif

  if (pSave->sefifosize > 0)
  { pSave->sefifodata = (char *)rMalloc(pSave->sefifosize);
    GMBASE_INIT(seg_reg, pDevP);
    tserpp = (ulong *)(pDevP->gmbase + GCP_TRP_ofst);
    offset = (*tserpp & 0x0000ffff);

#ifdef GEM_DBUG
    printf("trav se read ptr offset = 0x%x\n", offset);
#endif

    if (offset + pSave->sefifosize > FifoLen)
    { i = FifoLen - offset;
      pBuf = pSave->sefifodata;
      GMBASE_INIT(seg_reg, pDevP);
      /* printf("memcpy 1\n"); */
      memcpy(pBuf, pDevP->gmbase + (*tserpp & 0x001fffff), i);
      /* printf("memcpy 1\n"); */
      pBuf += i;
      i = pSave->sefifosize - i;
      /* printf("memcpy 2\n"); */
      memcpy(pBuf, pDevP->gmbase + (*(tserpp-1) & 0x001fffff), i);
      /* printf("memcpy 2\n"); */
    }
    else
    { 
      GMBASE_INIT(seg_reg, pDevP);
      /* printf("memcpy 3\n"); */
      memcpy(pSave->sefifodata, pDevP->gmbase + (*tserpp & 0x001fffff),
	     pSave->sefifosize);
      /* printf("memcpy 3\n"); */
    }

#ifdef GEM_DBUG
    printf("Done saving trav fifo\n");
#endif

    /* reset traversal se fifo in use count, and read/write pointers	*/

    GMBASE_INIT(seg_reg, pDevP);
    IUR_INIT(TravSeFifo, pDevP);
    *IUR_P[TravSeFifo] &= ~IURMASK;
    crp = (ulong *)(pDevP->gmbase + CTR2_ofst);
    *crp = CTR_REG_CLEAR;
    *tserpp = *(tserpp - 1);

#ifdef GEM_DBUG
    printf("IUR = 0x%x   read ptr = 0x%x\n", *IUR_P[TravSeFifo], *tserpp);
#endif

  }

#ifdef NEW_HOTKEY

  GMBASE_INIT(seg_reg, pDevP);

  /*
   * Save traversal data fifo 
   *
   * Since we don't know whether a read or a write is being done, the in
   * use count doesn't help us determine how much is in the fifo, so we
   * save the whole thing.  Also, since the fifo write pointer is not
   * writable, and there is no convenient method of changing it, we start
   * saving the fifo at the write pointer, reset the write pointer to 0
   * when we come back, and restore the fifo at 0.  Note that we must
   * adjust the read pointer accordingly.
   */

  for (pProc=gdp->devHead.pProc; pProc; pProc=pProc->procHead.pNext)
  { 
    for (rp=pProc->procHead.pRcx; rp; rp=rp->pNext)
    {
        if (VME_ADR(&((TRAV_CONTEXT *)
		    pDevP->slots[((rGemrcxPtr)rp->pData)->start_slot].slot_addr)->asl)
	== ((GM_MMAP *)pDevP->gmbase)->gm_ucflags.trv_cxt_add)

           break;
     }

    if (rp)
      break;

  }

  pSave->trav_rcx = (ulong)rp;

#ifdef GEM_DBUG
  printf(" Current traversal rcx = 0x%x\n", rp);
  printf("  rcx.asl=0x%x\n", VME_ADR(&((TRAV_CONTEXT *)
		 pDevP->slots[((rGemrcxPtr)rp->pData)->start_slot].slot_addr)->asl));
  printf("  real asl =0x%x\n", ((GM_MMAP *)pDevP->gmbase)->gm_ucflags.trv_cxt_add);
#endif

  GMBASE_INIT(seg_reg, pDevP);

  if (rp && !(rp->flags & RCX_NULL))
  { *garp = pDevP->drp_geo;

    /* save in use count						*/

    IUR_INIT(TravDataFifo, pDevP);
    pSave->dataInUseCount = *IUR_P[TravDataFifo] & 0x1ffff;

    /* find adress in fifo of write pointer				*/

    IP_INIT(3, pDevP);
    wp = *pDevP->ip_reg[3] & 0xffff;
    pSave->dataReadPtr = *(ulong *)(pDevP->gmbase + DRP_TRP_ofst) & 0xffff;

    /* adjust read pointer according to write pointer			*/

    if (wp <= pSave->dataReadPtr)
      pSave->dataReadPtr -= wp;
    else
      pSave->dataReadPtr += FifoLen - wp;

    tdp = (ulong *)(pDevP->gmbase +
		    (*(ulong *)(pDevP->gmbase + DRP_TRP_ofst - 4) & 0x001fffff));
    /* printf("memcpy 4\n"); */
    memcpy(pSave->datafifo, tdp + wp/4, FifoLen - wp);
    /* printf("memcpy 4\n"); */

    /* printf("memcpy 5\n"); */
    if (wp > 0)
      memcpy(&pSave->datafifo[FifoLen-wp], tdp, wp);
    /* printf("memcpy 5\n"); */

    /* reset traversal data fifo in use count, and read/write pointers	*/

    crp = (ulong *)(pDevP->gmbase + CTR3_ofst);
    *crp = CTR_REG_CLEAR;
    *(tdp + 1) = *tdp;

    *garp = pDevP->gcp_geo;
  }
#endif

  /*
   * Restart traversal fifo
   * NB: We need to do 2 starts, because there have been 2 stops: one
   * during lock_adapter called by X before calling us, and another
   * one above to ask for acknowledgment.
   */

  pBuf = se_buffer;
  START_OTHR_FIFO( pBuf, 3 );
  START_OTHR_FIFO( pBuf, 3 );

  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );

  /* 
   * Find current traversal context and save its DSV(s) and pick
   * attribute stack.
   */

#ifndef NEW_HOTKEY
  rp = gdp->domain[TRAVERSAL_GLOBAL_DOMAIN].pCur;

#ifdef GEM_DBUG
  printf(" Current traversal rcx = 0x%x\n", rp);
#endif

#endif

  if (rp && !(rp->flags & RCX_NULL))
  { pBuf = se_buffer;
    buf_len = save_pick_stack(gdp, rp, pBuf);
    if (buf_len < 0)
    { UNLOCK_SLOTS();
      BUSMEM_DET(seg_reg);
      return(buf_len);
    }
    pBuf += buf_len;

    buf_len = store_dsv(gdp, rp, pBuf);
    if (buf_len < 0)
    { UNLOCK_SLOTS();
      BUSMEM_DET(seg_reg);
      return(buf_len);
    }
    pBuf += buf_len;

    /* get pointer to beginning of DSV					*/
    dsvp = pDevP->gmbase + (*(((ulong *)pBuf)-1) & 0x001fffff);

#ifdef GEM_DBUG
    printf("pointer to dsv = 0x%x\n", dsvp);
#endif

#ifdef GEM_DBUG
    printf("Writing to trav se fifo\n");
#endif

    WTFIFO( TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP);

#ifdef GEM_DBUG
    printf("Waiting for trav se fifo - store dsv\n");
#endif

    FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);

#ifdef GEM_DBUG
    printf("trav se fifo empty\n");
#endif

    /* find how many more DSV's need to be saved			*/
    pSave->numdsv = *(dsvp + STRUCT_LEV_OFST/4);

#ifdef GEM_DBUG
    printf("numdsv = %d\n", pSave->numdsv);
#endif

    if (pSave->numdsv > 1)
    { pSave->dsvptr = (char *)rMalloc(TRVDSVSIZ * (pSave->numdsv - 1));
      for (i=1, pdsv=pSave->dsvptr; i<pSave->numdsv; ++i, pdsv+=TRVDSVSIZ)
      { 
        /* printf("memcpy 6\n"); */
        memcpy(pdsv, dsvp, TRVDSVSIZ);
        /* printf("memcpy 6\n"); */

	pCr = (cond_ret *)(pBuf = se_buffer);
	pCr->len = sizeof(cond_ret);
	pCr->opcode = SE_CRET;
	pCr->mask = 0;
	pCr->cond = 2;
	pBuf += sizeof(cond_ret);

	buf_len = store_dsv(gdp, rp, pBuf);
	pBuf += buf_len;

#ifdef GEM_DBUG
	printf("Waiting for trav se fifo - return & store dsv\n");
#endif

	WTFIFO( TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
	FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);
#ifdef GEM_DBUG
	printf("trav se fifo empty\n");
#endif
      }
    }
  }

  /*
   * Save information on adapter
   */
  
  GMBASE_INIT(seg_reg, pDevP);

  /*
   * Save communication area
   */
  /* Select the GCP processor						*/
  *garp = pDevP->gcp_geo;


  /* Copy the comm area to malloc'd storage */

  /* printf("memcpy 7\n"); */
  memcpy(pSave->gcpCommSave, pDevP->gmbase, GMCommAreaSize);
  /* printf("memcpy 7\n"); */

  /* Set GCP fifo read ptrs equal to fifo base adresses			*/
  plong = &pSave->gcpCommSave[GCP_Fifo_ofst/4];
  *(plong + 1) = *plong;
  plong += 2;
  *(plong + 1) = *plong;

  /* Select the shading processor (if one exists) */
     if ( !(features & NO_SHP) )
     {
        *garp=pDevP->shp_geo;	

        /* printf("memcpy 8\n"); */
        memcpy(pSave->shpCommSave, pDevP->gmbase, GMCommAreaSize);
        /* printf("memcpy 8\n"); */
     }

  /* Select the drawing processor */
  *garp=pDevP->drp_geo;	

  /* Copy the comm area to malloc'd storage */

  /* printf("memcpy 9\n"); */
  memcpy(pSave->drpCommSave, pDevP->gmbase, GMCommAreaSize);
  /* printf("memcpy 9\n"); */

  /* Set DRP fifo read ptrs equal to fifo base adresses			*/
  plong = &pSave->drpCommSave[DRP_Fifo_ofst/4];
  *(plong + 1) = *plong;
  plong += 2;
  *(plong + 1) = *plong;

#ifdef GEM_DBUG
  printf(" Finished copying comm areas\n");
#endif

  /*
   * Save cursor pattern
   */

    pDevP->pCursplanes = &(((GM_MMAP *)pDevP->gmbase)->cursplane[0][0]);
    pcur = (ulong *)(pDevP->pCursplanes);
    /* printf("memcpy 10\n"); */
    memcpy(pSave->cursorSave, pcur, NUMCSRPAT*CURSPMSIZ);
    /* printf("memcpy 10\n"); */

#ifdef GEM_DBUG
  printf(" Done copying cursor pattern\n");
#endif

  /*
   * Save RCM private context
   */

  pDevP->pPrivCxt = ((GM_MMAP *)pDevP->gmbase)->rcm_priv_cxt;
  /* printf("memcpy 11\n"); */
  memcpy(pSave->rcmPrivSave, pDevP->pPrivCxt, RCMPrivCxtSize);
  /* printf("memcpy 11\n"); */

#ifdef GEM_DBUG
  printf(" Done copying RCM private context\n");
#endif

  /*
   * Save slot information
   */

  for (i=pDevP->first_imm_slot; i<pDevP->first_imm_slot+pDevP->num_imm_slots;
       ++i)
    /*
     * if the cxt is on the adapter, move it to kernel memory
     */

    if (pDevP->slots[i].status_flags & ON_ADAPT)
    { 
       /*
       * Find current RCX for slot
       */

#ifdef GEM_DBUG
      printf(" Saving slot #%d\n", i);
#endif

      pnode = pDevP->slots[i].pHead;
      while (!(((rGemrcxPtr)pnode->pRcx->pData)->status_flags & ON_ADAPT) &&
	     pnode != NULL)
	pnode = pnode->pNext;
      assert(pnode);
      pGemrcx = (rGemrcxPtr)pnode->pRcx->pData;
      pGemrcx->status_flags &= ~ON_ADAPT;
      length = pDevP->slots[i].slot_len;
      pGemrcx->pCxt = (gem_cxt *)rMalloc(length);

      if (pGemrcx->pCxt == NULL)
      { UNLOCK_SLOTS();
	BUSMEM_DET(seg_reg);
	return(GM_NOMEMORY);
      }

      pDevP->slots[i].slot_addr &= 0x0fffffff;
      pDevP->slots[i].slot_addr |= seg_reg;
      /* printf("seg_reg = 0x%x\n", seg_reg);
         printf("pDevP->slots[i].slot_addr = 0x%x\n", pDevP->slots[i].slot_addr);
         printf("pGemrcx->pCxt = 0x%x\n", pGemrcx->pCxt); */

      /* printf("memcpy 12\n"); */
      memcpy(pGemrcx->pCxt, pDevP->slots[i].slot_addr, length);
      /* printf("memcpy 12\n"); */

      pGemrcx->status_flags |= IN_KERNEL;
      pDevP->slots[i].status_flags &= ~ON_ADAPT;
    }
    else
    {
#ifdef GEM_DBUG
      printf(" Slot #%d is empty\n", i);
#endif
    }

  pSave->slot_cxts = (rcxPtr *)rMalloc(4*pDevP->num_trav_slots);
  for (i=0; i<pDevP->num_trav_slots; ++i)
    pSave->slot_cxts[i] = NULL;

  i = pDevP->first_trav_slot;
  while (i<pDevP->first_trav_slot+pDevP->num_trav_slots)
  { /*
     * if the cxt or rcx part is on the adapter, move it to
     * kernel memory
     */
    if (pDevP->slots[i].status_flags & ON_ADAPT)
    { if (pDevP->slots[i].status_flags & PINNED)
      { /* Slot has RCX part in it					*/

#ifdef GEM_DBUG
	printf(" Saving RCX part in slot #%d\n", i);
#endif

	pGemrcx = (rGemrcxPtr)((rcxpPtr)pDevP->slots[i].pHead->pRcx)->pData;
	length = pDevP->slots[i].slot_len * pGemrcx->num_slots;
      }
      else
      { /* Slot has context in it					*/

#ifdef GEM_DBUG
	printf(" Saving context in slot #%d\n", i);
#endif

	/*
	 * Find current RCX for slot
	 */

	pnode = pDevP->slots[i].pHead;
	while (!(((rGemrcxPtr)pnode->pRcx->pData)->status_flags & ON_ADAPT) &&
	       pnode != NULL)
	  pnode = pnode->pNext;
	assert(pnode);
	pSave->slot_cxts[i - pDevP->first_trav_slot] = pnode->pRcx;
#ifdef GEM_DBUG
	printf(" cxt in slot %d = 0x%x\n", i,
	       pSave->slot_cxts[i - pDevP->first_trav_slot]);
#endif
	pGemrcx = (rGemrcxPtr)pnode->pRcx->pData;
	pGemrcx->status_flags &= ~ON_ADAPT;
	length = pDevP->slots[i].slot_len;
      }
      pGemrcx->pCxt = (gem_cxt *)rMalloc(length);
      if (pGemrcx->pCxt == NULL)
      { UNLOCK_SLOTS();
	BUSMEM_DET(seg_reg);
	return(GM_NOMEMORY);
      }

      pDevP->slots[i].slot_addr &= 0x0fffffff;
      pDevP->slots[i].slot_addr |= seg_reg;

      /* printf("seg_reg = 0x%x\n", seg_reg);
         printf("pDevP->slots[i].slot_addr = 0x%x\n", pDevP->slots[i].slot_addr);
         printf("pGemrcx->pCxt = 0x%x\n", pGemrcx->pCxt); */

      /* printf("memcpy 13\n"); */
      memcpy(pGemrcx->pCxt, pDevP->slots[i].slot_addr, length);
      /* printf("memcpy 13\n"); */
      pGemrcx->status_flags |= IN_KERNEL;
      pDevP->slots[i].status_flags &= ~ON_ADAPT;
      i += pGemrcx->num_slots;
    }
    else
    {
#ifdef GEM_DBUG
      printf(" Slot #%d is empty\n", i);
#endif
      ++i;
    }
  }

#ifdef GEM_DBUG
  printf(" Finished saving slots\n");
#endif

  UNLOCK_SLOTS();
  BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
  printf("   Exit gem_save_state\n");
#endif

  return(0);
}

/*
 * FUNCTION NAME: gem_restore_state
 *                                                                     
 * DESCRIPTION: Restore any saved information back onto the adapter and
 *		switch on current contexts.
 *		   NB: This function assumes that no one else is trying
 *		to access the adapter while it is running.
 */
gem_restore_state(gdp)
struct _gscDev	*gdp;
{
  int		i, j, rc;
  ulong		seg_reg;
  uint		oldlevel;
  volatile ulong *dp;
  ulong		*pcur;
  ulong		*garp;
  ulong		*crp;
  ulong		*dsvp;
  ulong		*trpp;
  ulong		tdfo;
  ulong		*tdp;
  ulong         features;       /* check if SHP exists */
  ulong         *CommSave;
  commSavePtr	pSave;
  devDomainPtr	pDom;
  rcxPtr	rp;
  rGemrcxPtr	pGemrcx;
  int		length;
  CxtSlot	*pCslot;
  char		se_buffer[8192];
  char		*pBuf = se_buffer;
  char		*pdsv;
  generic_vme	*pVme;
  generic_se	*pSe;
  execut	*pEx;
  ushort	buf_len;
  char		*tmp1, *tmp2;
  char		*dbp;
  char		disp_info;
  int		bdb, odb;
  int		cm;
  TRAV_CONTEXT	*pRCMcxt;
  struct _rcmWG	*pwg;
  rcmProcPtr	pProc;
  struct phys_displays *pd = gdp->devHead.display;
  rGemDataPtr	pgd 	= (rGemDataPtr)(gdp->devHead.vttld);
  rGemRCMPrivPtr pDevP  = &pgd->GemRCMPriv;

#ifdef GEM_DBUG
  printf("Enter gem_restore_state: gdp = 0x%x\n", gdp);
#endif

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  GMBASE_INIT(seg_reg, pDevP);

  features = ((struct gem_dds *)pd->odmdds)->features; /* to check for SHP */

  /* 
   * Make sure that fifos are empty
   */
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  FIFO_EMPTY(ImmDataFifo, seg_reg, pDevP);
  FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);
  FIFO_EMPTY(TravDataFifo, seg_reg, pDevP);

  /*
   * Ensure that no one else changes slot information while we're
   * restoring the RCX parts
   */
  LOCK_SLOTS();

  /* set pointer to communications save area				*/
  pSave = (commSavePtr)pDevP->pCommSave;

  /*
   * Move any traversal RCX's and RCX parts back onto the adapter
   */

  i=pDevP->first_trav_slot;
  while (i<pDevP->first_trav_slot+pDevP->num_trav_slots)
    if (pDevP->slots[i].status_flags & PINNED)
    { /* Slot is allocated to an RCX part				*/

#ifdef GEM_DBUG
      printf(" Moving RCX part in slot #%d back onto adapter\n", i);
#endif

      pGemrcx = (rGemrcxPtr)((rcxpPtr)pDevP->slots[i].pHead->pRcx)->pData;
      pGemrcx->status_flags &= ~IN_KERNEL;
      pDevP->slots[i].slot_addr &= 0x0fffffff;   
      pDevP->slots[i].slot_addr |= seg_reg;
      /* printf("memcpy 1\n"); */
      memcpy(pDevP->slots[i].slot_addr, pGemrcx->pCxt,
	     pDevP->slots[i].slot_len * pGemrcx->num_slots);
      /* printf("memcpy 1\n"); */

      for (j=i; j<i+pGemrcx->num_slots; ++j)
	pDevP->slots[j].status_flags |= ON_ADAPT;
      rFree(pGemrcx->pCxt);
      pGemrcx->pCxt = NULL;
      i += pGemrcx->num_slots;
    }
    else
    { if (pSave->slot_cxts[i - pDevP->first_trav_slot])
      {	/*
	 * Copy context onto adapter
	 */

#ifdef GEM_DBUG
	printf("Moving RCX      in slot %d (0x%x) back onto adapter\n", i,
	       pSave->slot_cxts[i - pDevP->first_trav_slot]);
#endif

	pGemrcx = (rGemrcxPtr)pSave->slot_cxts[i - pDevP->first_trav_slot]->pData;
	pCslot = &pDevP->slots[pGemrcx->start_slot];
	length = pCslot->slot_len;
	pGemrcx->status_flags &= ~IN_KERNEL;
        pDevP->slots[i].slot_addr &= 0x0fffffff;
        pDevP->slots[i].slot_addr |= seg_reg;
        pCslot->slot_addr &= 0x0fffffff;
        pCslot->slot_addr |= seg_reg;
        /* printf("memcpy 2\n"); */
	memcpy(pCslot->slot_addr, pGemrcx->pCxt, length);
        /* printf("memcpy 2\n"); */
	pGemrcx->status_flags |= ON_ADAPT;
	pCslot->status_flags |= ON_ADAPT;
	rFree(pGemrcx->pCxt);
	pGemrcx->pCxt = NULL;
      }

      ++i;
    }

  /* free data for storing slot context information	*/
  rFree(pSave->slot_cxts);

#ifdef GEM_DBUG
  printf(" Done restoring traversal RCX's and RCX parts\n");
#endif

  /*
   * Restore RCM private context
   */

  GMBASE_INIT(seg_reg, pDevP);

  pDevP->pPrivCxt = ((GM_MMAP *)pDevP->gmbase)->rcm_priv_cxt;
  /* printf("memcpy 3\n"); */
  memcpy(pDevP->pPrivCxt, pSave->rcmPrivSave, RCMPrivCxtSize);
  /* printf("memcpy 3\n"); */

#ifdef GEM_DBUG
  printf(" Finished restoring RCM private context\n");
#endif

  /*
   * Restore cursor pattern
   */

  pDevP->pCursplanes = &(((GM_MMAP *)pDevP->gmbase)->cursplane[0][0]);
  pcur = (ulong *)(pDevP->pCursplanes);
  /* printf("memcpy 4\n"); */
  memcpy(pcur, pSave->cursorSave, NUMCSRPAT*CURSPMSIZ);
  /* printf("memcpy 4\n"); */

#ifdef GEM_DBUG
  printf(" Finished restoring cursor pattern\n");
#endif

  /*
   * Restore communication area
   */

  /* set dp to point to the geographical address register		*/
  garp = (ulong *)(pDevP->gmbase + GAR_ofst);

  /* Select the GCP processor						*/
  *garp = pDevP->gcp_geo;

  /* Copy the comm area from malloc'd storage				*/
  /* printf("memcpy 5\n"); */
  memcpy(pDevP->gmbase, pSave->gcpCommSave, GMCommAreaSize);
  /* printf("memcpy 5\n"); */

  /* Select the shading processor					*/
  *garp=pDevP->shp_geo;

  /* Select the shading processor (if one exists) & copy comm area from
     malloc'd storage, skipping tag field */

     if ( !(features & NO_SHP) )
     {
        *garp=pDevP->shp_geo;
        /* printf("memcpy 6\n"); */
        memcpy(pDevP->gmbase, pSave->shpCommSave, TAG_ofst);
        /* printf("memcpy 6\n"); */
        tmp1 = (char *)pDevP->gmbase;
        tmp1 += TAG_ofst + 4;
        tmp2 = (char *)pSave->shpCommSave;
        tmp2 += TAG_ofst + 4;
        /* printf("memcpy 7\n"); */
	memcpy(tmp1, tmp2, GMCommAreaSize - TAG_ofst - 4);
        /* printf("memcpy 7\n"); */
     }
     else
     {
        tmp1 = (char *)pDevP->gmbase;
        tmp1 += TAG_ofst + 4;
     }

  /* Select the drawing processor					*/
  *garp=pDevP->drp_geo;

  /* Copy the comm area from malloc'd storage, skipping tag field and	*/
  /* cursor lock work							*/
  if (TAG_ofst < Curs_Lock_ofst)
  { 
    /* printf("memcpy 8\n"); */
    memcpy(pDevP->gmbase, pSave->drpCommSave, TAG_ofst);
    /* printf("memcpy 8\n"); */
    tmp2 = (char *)pSave->drpCommSave;
    tmp2 += TAG_ofst + 4;
    /* printf("memcpy 9\n"); */
    memcpy(tmp1, tmp2, Curs_Lock_ofst - TAG_ofst - 4);
    /* printf("memcpy 9\n"); */
    tmp1 += Curs_Lock_ofst - TAG_ofst;
    tmp2 += Curs_Lock_ofst - TAG_ofst;
    /* printf("memcpy 10\n"); */
    memcpy(tmp1, tmp2, GMCommAreaSize - Curs_Lock_ofst - 4);
    /* printf("memcpy 10\n"); */
  }
  else
  { 
    /* printf("memcpy 11\n"); */
    memcpy(pDevP->gmbase, pSave->drpCommSave, Curs_Lock_ofst);
    /* printf("memcpy 11\n"); */
    tmp1 = (char *)pDevP->gmbase + Curs_Lock_ofst + 4;
    tmp2 = (char *)pSave->drpCommSave;
    tmp2 += Curs_Lock_ofst + 4;
    /* printf("memcpy 12\n"); */
    memcpy(tmp1, tmp2, TAG_ofst - Curs_Lock_ofst - 4);
    /* printf("memcpy 12\n"); */
    tmp1 += TAG_ofst - Curs_Lock_ofst;
    tmp2 += TAG_ofst - Curs_Lock_ofst;
    /* printf("memcpy 13\n"); */
    memcpy(tmp1, tmp2, GMCommAreaSize - TAG_ofst - 4);
    /* printf("memcpy 13\n"); */
  }

  /* Reselect the GCP processor						*/
  *garp = pDevP->gcp_geo;

  /* Reset hardware's fifo write ptrs					*/
  crp = (ulong *)(pDevP->gmbase + CTR0_ofst);
  *crp = CTR_REG_CLEAR;
  crp = (ulong *)(pDevP->gmbase + CTR1_ofst);
  *crp = CTR_REG_CLEAR;
  crp = (ulong *)(pDevP->gmbase + CTR2_ofst);
  *crp = CTR_REG_CLEAR;
  crp = (ulong *)(pDevP->gmbase + CTR3_ofst);
  *crp = CTR_REG_CLEAR;

#ifdef GEM_DBUG
  printf(" IUR=0x%x   read ptr = 0x%x\n", *IUR_P[ImmSeFifo],
	 *((uint *)(pDevP->gmbase + GCP_IRP_ofst)));
#endif

  /* Reset 3D fifo semaphore value					*/
  dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.freeze_3d;
  *dp = 0;

  pSe = (generic_se *)pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_TS;
  pSe->data = 0;
  pBuf += sizeof(generic_se);

#ifdef WRITE_EVERY
  printf("Writing to Imm fifo - test set\n");
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  printf("Waiting for fifo to empty\n");
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  printf("Fifo is empty\n");
  pBuf = se_buffer;
#endif

#ifdef GEM_DBUG
  printf(" Done restoring communications area\n");
#endif

  /*
   * Restore window ID information
   */

  /* Activate RCM's private context					*/

  pRCMcxt = (TRAV_CONTEXT *)pDevP->pPrivCxt;
  pVme = (generic_vme *)pBuf;
  pVme->len = sizeof(generic_vme);
  pVme->opcode = CE_ACTC;
  pVme->adr = VME_ADR(&pRCMcxt->asl);
  pBuf += sizeof(generic_vme);
  pVme = (generic_vme *)pBuf;
  pVme->len = sizeof(generic_vme);
  pVme->opcode = CE_LDSV;
  pVme->adr = VME_ADR(pRCMcxt + 1);
  pBuf += sizeof(generic_vme);

#ifdef WRITE_EVERY
  printf("Writing to Imm fifo - act cxt & load dsv\n");
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  printf("Waiting for fifo to empty\n");
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  printf("Fifo is empty\n");
  pBuf = se_buffer;
#endif

  GMBASE_INIT(seg_reg, pDevP);
  dbp = (char *)(pDevP->gmbase + DISP_BUF_OFFSET);

  for (i=pDevP->hwid_head; i >= 0; i=pDevP->hwid[i].next)
    if (pwg = pDevP->hwid[i].pwg)
    { /* set correct values in window look-up table			*/

#ifdef GEM_DBUG
      printf(" Resetting WLUT entry #%d\n", i);
#endif

      /* find color map for this window ID				*/
      if (pDevP->hwid[i].pwg->wg.cm_handle)
	cm = ((rcmCmPtr)pDevP->hwid[i].pwg->wg.cm_handle)->hwd_map;
      else
	cm = NUM_VLTS - 1;

      disp_info = dbp[(i/16)*4 + 3 - ((i%16) / 4)];
      if (disp_info & (1 << i % 4 * 2))
	bdb = 1;
      else
	bdb = 0;
      if (disp_info & (1 << i % 4 * 2 + 1))
	odb = 1;
      else
	odb = 0;

      set_WLUT(i, cm, bdb, odb, &pBuf);

#ifdef WRITE_EVERY
  printf("Writing to Imm fifo - set WLUT\n");
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  printf("Waiting for fifo to empty\n");
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  printf("Fifo is empty\n");
  pBuf = se_buffer;
#endif

    }

  for (cm=0; cm<NUM_VLTS; cm++)
    for (bdb=0; bdb<2; bdb++)
      for (odb=0; odb<2; odb++)
	if (pDevP->prot_hwids[cm][bdb][odb] >= 0)
	{

#ifdef GEM_DBUG
	  printf(" Resetting protect WLUT entry #%d\n",
		 pDevP->prot_hwids[cm][bdb][odb]);
#endif

	  set_WLUT(pDevP->prot_hwids[cm][bdb][odb], cm, bdb, odb, &pBuf);

#ifdef WRITE_EVERY
  printf("Writing to Imm fifo - set WLUT2\n");
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  printf("Waiting for fifo to empty\n");
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  printf("Fifo is empty\n");
  pBuf = se_buffer;
#endif

	}
	  
  /* Make sure window planes are selected				*/
  pSe = (generic_se *)pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_SDFB;
  pSe->data = WINDOW_PLANES;
  pBuf += sizeof(generic_se);

#ifdef WRITE_EVERY
  printf("Writing to Imm fifo - select window planes\n");
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  printf("Waiting for fifo to empty\n");
  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  printf("Fifo is empty\n");
  pBuf = se_buffer;
#endif

#ifdef GEM_DBUG
  printf("Done resetting WLUT\n");
#endif

  reset_sync_cntrs(gdp);
#ifdef GEM_DBUG
  printf(" Done resetting display buffer sync counters\n");
#endif GEM_DBUG

  /*
   * Find the current immediate context and switch it on
   */
  rp = gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur;
#ifdef GEM_DBUG
  printf(" Restoring immediate rcx = 0x%x\n", rp);
  GMBASE_INIT(seg_reg, pDevP);
  IUR_INIT(ImmSeFifo, pDevP);
  printf(" IUR=0x%x   read ptr = 0x%x\n", *IUR_P[ImmSeFifo],
	 *((uint *)(pDevP->gmbase + GCP_IRP_ofst)));
#endif

  if (rp)
  { /*
     * Copy context onto adapter
     */
    pGemrcx = (rGemrcxPtr)rp->pData;
    pCslot = &pDevP->slots[pGemrcx->start_slot];
    length = pCslot->slot_len;
    pGemrcx->status_flags &= ~IN_KERNEL;
    pCslot->slot_addr &= 0x0fffffff;
    pCslot->slot_addr |= seg_reg;
    /* printf("memcpy 14\n");
       printf("pCslot->slot_addr = 0x%x\n", pCslot->slot_addr);
       printf("seg_reg = 0x%x\n", seg_reg);
       printf("pGemrcx->pCxt = 0x%x\n", pGemrcx->pCxt); */
    memcpy(pCslot->slot_addr, pGemrcx->pCxt, length);
    /* printf("memcpy 14\n"); */
    pGemrcx->status_flags |= ON_ADAPT;
    pCslot->status_flags |= ON_ADAPT;
    rFree(pGemrcx->pCxt);
    pGemrcx->pCxt = NULL;

    /*
     * Set activate context SE
     */
    pVme = (generic_vme *)pBuf;
    pVme->len = sizeof(generic_vme);
    pVme->opcode = CE_ACTC;
    pGemrcx->pASL = (ulong *)( (ulong)pGemrcx->pASL & 0x0fffffff );
    pGemrcx->pASL = (ulong *)( (ulong)pGemrcx->pASL | seg_reg );
    pVme->adr = VME_ADR(&((IMM_CONTEXT *)pGemrcx->pASL)->asl);
    pBuf += sizeof(generic_vme);

    /*
     * Set load DSV SE
     */
    buf_len = load_dsv(gdp, rp, pBuf);
    pBuf += buf_len;

    /*
     * Write buffer to fifo.
     */
#ifdef GEM_DBUG
    printf(" Writing activate and load DSV\n");
#endif

#ifdef GEM_DETAIL
  { ulong *p;
    int j;
    printf(" Buffer = 0x%x\n", pBuf - se_buffer);
    p = (ulong *)se_buffer;
    for (i=0; i<(pBuf - se_buffer)/4; ++i, ++p)
    { printf("%08x ", *p);
      if (i % 8 == 7)
	printf("\n");
      if (i % 192 == 191)
      { printf("pausing .");
	for (j=0; j<40000000; ++j)
	if (j % 5000000 == 0)
	  printf(".");
	printf("\n");
      }
    }
    if (i % 8 != 0)
      printf("\n");
  }
#endif

    WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );

#ifdef GEM_DBUG
    printf(" Waiting for Imm Se Fifo to empty\n");
#endif

    FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);

#ifdef GEM_DBUG
    printf(" Fifo is now empty\n");
#endif
  }

  /*
   * Find the current traversal context and switch it on
   */

  rp = (rcxPtr)pSave->trav_rcx;

#ifdef GEM_DBUG
  printf(" Restoring traversal rcx = 0x%x\n", rp);
#endif

  if (rp && !(rp->flags & RCX_NULL))
  {

     /*
     * Set activate context SE
     */

    pGemrcx = (rGemrcxPtr)rp->pData;
    pBuf = se_buffer;
    pVme = (generic_vme *)pBuf;
    pVme->len = sizeof(generic_vme);
    pVme->opcode = CE_ACTC;

    pGemrcx->pASL = (ulong *)( (ulong)pGemrcx->pASL & 0x0fffffff );
    pGemrcx->pASL = (ulong *)( (ulong)pGemrcx->pASL | seg_reg );
    pVme->adr = VME_ADR(&((TRAV_CONTEXT *)pGemrcx->pASL)->asl);
    pBuf += sizeof(generic_vme);

    /*
     * Set load DSV SE
     */
    buf_len = load_dsv(gdp, rp, pBuf);
    pBuf += buf_len;

    /* get address of dsv						*/
    GMBASE_INIT(seg_reg, pDevP);
    dsvp = pDevP->gmbase + (*(((ulong *)pBuf) - 1) & 0x001fffff);

#ifdef GEM_DBUG
    printf("  dsv ptr = 0x%x\n", dsvp);
    printf("  num dsv's saved = %d\n", pSave->numdsv);
#endif

    if (pSave->numdsv > 1)
    { for (i=1, pdsv=pSave->dsvptr+TRVDSVSIZ*(pSave->numdsv-2);
	   i<pSave->numdsv; ++i, pdsv-=TRVDSVSIZ)
      { 
	WTFIFO(TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP);
	FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);

	pSe = (generic_se *)(pBuf = se_buffer);
	pSe->len = sizeof(generic_se);
	pSe->opcode = CE_AECT;
	pSe->data = -1;
	pBuf += sizeof(generic_se);

	pEx = (execut *)pBuf;
	pEx->len = sizeof(execut);
	pEx->opcode = SE_EXST;
	pEx->strid = 0;
	pBuf += sizeof(execut);

        /* printf("memcpy 15\n"); */
	memcpy(dsvp, pdsv, TRVDSVSIZ);
        /* printf("memcpy 15\n"); */

	buf_len = load_dsv(gdp, rp, pBuf);
	pBuf += buf_len;
      }
      rFree(pSave->dsvptr);
    }

#ifdef GEM_DBUG
    printf(" Done restoring DSV's\n");
#endif

    /*
     * Set restore pick stack SE
     */
    buf_len = restore_pick_stack(gdp, rp, pBuf);
    pBuf += buf_len;

#ifdef GEM_DBUG
    printf(" Done restoring pick stack\n");
#endif

#ifdef NEW_HOTKEY
    /* Select the drawing processor					*/
    *garp=pDevP->drp_geo;

    GMBASE_INIT(seg_reg, pDevP);
    tdp = (ulong *)(pDevP->gmbase + BltTravFifo);

#ifdef GEM_DBUG
    printf("traversal fifo = 0x%x\n", tdp);
#endif

    GMBASE_INIT(seg_reg, pDevP);
    /* printf("memcpy 16\n"); */
    memcpy(tdp, pSave->datafifo, FifoLen);
    /* printf("memcpy 16\n"); */
    ADD_INIT(TravDataFifo, pDevP);
    *ADD_P[TravDataFifo] = pSave->dataInUseCount;
    *(ulong *)(pDevP->gmbase + DRP_TRP_ofst) |= pSave->dataReadPtr;

    /* Reselect the GCP processor					*/
    *garp = pDevP->gcp_geo;

    /*
     * Set Resume from Heavy Context Switch SE
     */
    *(ulong *)pBuf = 0x00040000 | CE_RSHVY;
    pBuf += 4;
#endif

    /*
     * Write buffer to fifo.
     */
#ifdef GEM_DBUG
    printf(" Writing activate and load DSV\n");
#endif
    WTFIFO( TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );

    /* Wait for Fifo to drain						*/
    FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);
  }
  
  if (pSave->sefifosize > 0)
  { 
    WTFIFO( TravSeFifo, pSave->sefifodata, pSave->sefifosize, seg_reg, pDevP );


    rFree(pSave->sefifodata);
  }

  /* Free space for save areas						*/
  rFree(pDevP->pCommSave);

#ifdef NEW_HOTKEY
  /*
   * The following code is currently unnecessary, as we are no longer guarding
   * the domain.  (See comment at beginning of save_state.)
   */
#if 0
  /*
   * Release the 3D domain to allow 3D processes to access the
   * adapter again.
   */
  pDom = &(gdp->domain[TRAVERSAL_GLOBAL_DOMAIN]);
  GM_UNGUARD(pDom);

#endif
#endif

  UNLOCK_SLOTS();
  BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
  printf("   Exit gem_restore_state\n");
#endif

  return(0);
}

void set_WLUT(wid, cm, bdb, odb, ppbuf)
int wid, cm, bdb, odb;
char **ppbuf;
{
  char		*pBuf = *ppbuf;
  frame_buf_cmp	*pFcmp;
  windattrs	*pWatt;
  generic_se	*pSe;

#ifdef WLUT_DETAIL
  printf("set_WLUT: id #%d = [%d %d %d]\n", wid, cm, bdb, odb);
#endif

  /* set window planes compare value					*/
  pFcmp = (frame_buf_cmp *)pBuf;
  pFcmp->len = sizeof(frame_buf_cmp);
  pFcmp->opcode = CE_FBC;
  pFcmp->flags = 8;	
  pFcmp->mask = 0;
  pFcmp->value = wid;
  pBuf += sizeof(frame_buf_cmp);

  /* set color map							*/
  pWatt = (windattrs *)pBuf;
  pWatt->len = sizeof(windattrs);
  pWatt->opcode = CE_WATT;
  pWatt->mask = waColorMode | waObscurity | waOrigin | waSize;
  pWatt->ctid = cm;
  pWatt->flags = 0;
  pBuf += sizeof(windattrs);

  /* Select base planes							*/
  pSe = (generic_se *)pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_SDFB;
  pSe->data = BASE_PLANES;
  pBuf += sizeof(generic_se);

  /* Set display buffer							*/
  pSe = (generic_se *)pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_FCTL;
  pSe->data = bdb << 30;
  pBuf += sizeof(generic_se);

  /* Select overlay planes						*/
  pSe = (generic_se *)pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_SDFB;
  pSe->data = OVERLAY_PLANES;
  pBuf += sizeof(generic_se);

  /* Set display buffer							*/
  pSe = (generic_se *)pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_FCTL;
  pSe->data = odb << 30;
  pBuf += sizeof(generic_se);

  *ppbuf = pBuf;
}
