static char sccsid[] = "@(#)46	1.1.1.16  src/bos/kernext/disp/gem/rcm/gem_uwg.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:21:55";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		activate_client_context
 *		activate_private_cxt
 *		change_colormap_only1134
 *		draw_client_clip
 *		draw_new_regions
 *		flush_buf
 *		gem_setwin
 *		set_FBC
 *		steal_wid
 *		upd_geom
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
 *; CHANGE HISTORY:
 *;
 *;MC   11/14/90   Created
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"
#include "gem_gai.h"
#include "gem_geom.h"

void activate_private_cxt();
void steal_wid();
void draw_new_regions();
void draw_client_clip();
void activate_client_context();
void set_FBC();
void gem_setwin();
void change_colormap_only();
void flush_buf();

/*
 * FUNCTION:  upd_geom
 *
 * DESCRIPTION:
 */
int  upd_geom(gdp, pRcx, pWG, pWA, real_wg, gai_chg_mask, rcm_chg_mask,
	      fifo_num, buf_start, pBuf, buf_size)
struct _gscDev	*gdp;
rcxPtr		pRcx;
struct _rcmWG 	*pWG;
struct _rcmWA	*pWA;
struct _rcmWG	*real_wg;
ulong		*gai_chg_mask;
ushort		rcm_chg_mask;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{
  int			i;
  long			hwid;
  Bool			react;
  rcmCmPtr		cm_handle;
  int			cm;
  rGemrcxPtr		pGemrcx;
  rWAPrivPtr		pwapriv;
  rWGPrivPtr            pwgpriv;
  rGemRCMPrivPtr	pDevP;
  TRAV_CONTEXT		*pRCMcxt;
  ulong			seg_reg;
  int			ntiks, count;
  volatile ulong	*dp, *freezeflag;
  ulong sync_element[100];
  int freezeAborted;
  generic_se            start_2D;
  
  pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  hwid = ((rWGPrivPtr)real_wg->pPriv)->hwid;


  if (pRcx)
    pGemrcx = (rGemrcxPtr)pRcx->pData;
  else
    pGemrcx = NULL;
  if (pWA)
    pwapriv = (rWAPrivPtr)pWA->pPriv;
  else
    pwapriv = NULL;

  pRCMcxt = (TRAV_CONTEXT *)pDevP->pPrivCxt;
  react = FALSE;
  
  /*
   * Find color table being used
   */
  cm_handle = (rcmCmPtr)pWG->wg.cm_handle;
  if (cm_handle != NULL)
    cm = cm_handle->hwd_map;
  else
    cm = NUM_VLTS - 1;

  /* 
   * Make sure there is enough room in se buffer for initial SE's
   */
  if (*pBuf + 1024 > buf_start + buf_size)
    flush_buf(pDevP, fifo_num, buf_start, pBuf);

  /*
   * Check to see if we need to write to the window planes - this is
   * true in four cases:
   *   a) the clipping region has changed
   *   b) the geometry is using a new window id, so it need to be drawn
   *   c) the geometry is currently using a holding id, but the colormap
   *      is changing, so we have to change which holding id is being used
   *   d) we're being called from bind, a new geometry is being bound, and
   *      client clip is active, so we need to redraw client clip
   */
  pwgpriv=(rWGPrivPtr) real_wg->pPriv;
  if (pwgpriv->deferredClearOverlays ||
      *gai_chg_mask & gCWclip || rcm_chg_mask & rNewHwid ||
      (*gai_chg_mask & gCWcolormap && hwid < 0) ||
      (rcm_chg_mask & rSetWinOrgSize && pWG != pRcx->pWG && pwapriv &&
       pWA->wa.pRegion))
  { /*
     * Since Clip regions are in screen coordinates, the window origin
     * and size must be set to full screen.  We do this by activating
     * the RCM's private context, which is set to be full screen and
     * an upper-left-hand coordinate system.
     */
    activate_private_cxt(gdp, pRcx, pGemrcx, pRCMcxt, &rcm_chg_mask, &react,
                    pBuf);
#ifdef FAST_LOCK
#else
    if (pGemrcx->cxt_type == TRAV_RCXTYPE)
    {
      STOP_OTHR_FIFO( *pBuf, 255, FALSE );
    }
#endif

  /* We used to clear the overlay planes in the old clipping region, but
   * that turned out to be a bad idea.  The new plan is to clear the
   * overlay planes in the new clipping region.  That will keep us from
   * tromping over other people's overlay planes.
   */
  pwgpriv=(rWGPrivPtr) real_wg->pPriv;
  if ((*gai_chg_mask & gCWclip) || pwgpriv->deferredClearOverlays) { 
    if (pGemrcx && pGemrcx->cxt_type == IMM_RCXTYPE) {
      if (!pwgpriv->deferredClearOverlays) {
        gem_clear_overlay(pDevP, pWG, fifo_num, buf_start, pBuf, buf_size);
      } else {
        gem_clear_overlay(pDevP, pWG, fifo_num, buf_start, pBuf, buf_size);
      }
    }
    if (!pGemrcx) pwgpriv->deferredClearOverlays=1;
    else pwgpriv->deferredClearOverlays=0;
  }


    /*
     * Check to see if we need to steal a WID from someone
     */
    if (rcm_chg_mask & rStealHwid)
      steal_wid(pDevP, real_wg, hwid, pBuf);

    /*
     * If case a), b), or c) above, then draw new regions
     */
    if (*gai_chg_mask & gCWclip || rcm_chg_mask & rNewHwid ||
	(*gai_chg_mask & gCWcolormap && hwid < 0))
      draw_new_regions(pDevP, pWG, real_wg, &hwid, cm, gai_chg_mask,
		       rcm_chg_mask, fifo_num, buf_start, pBuf, buf_size);

    /*
     * Make sure there is enough room in the buffer for any
     * remaing structure elements
     */
    if (*pBuf + 1024 > buf_start + buf_size)
      flush_buf(pDevP, fifo_num, buf_start, pBuf);

    /*
     * If case a) and client clip is active, or case d)
     */
    if ((*gai_chg_mask & gCWclip ||
	 rcm_chg_mask & rSetWinOrgSize && pWG != pRcx->pWG) &&
	pwapriv && pWA->wa.pRegion)
      draw_client_clip(pDevP, pWA, pWG, hwid, &pGemrcx->gWAChangeMask, pRCMcxt,
		       fifo_num, buf_start, pBuf, buf_size);

#ifdef FAST_LOCK
#else
    if (pGemrcx->cxt_type == TRAV_RCXTYPE)
    {
      START_OTHR_FIFO( *pBuf, 255 );
    }
#endif
  }	/* end: drawing to the window planes				*/

  /*
   * (Re-)activate client's context, if necessary
   */
    if (!(rcm_chg_mask & rCxtIsActive)) {
      activate_client_context(gdp, pRcx, pGemrcx, rcm_chg_mask, react, pBuf);
    }

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

  /*
     Wait to make sure traversal fifo is quiesced.  This is done by checking
     the value of the 3D fifo freeze flag managed by the GTO microcode.  It
     can take the following values:

             0 = microcode acknowledgement that 3D fifo is frozen
             1 = GTO request (via GCP interrupt) to freeze the 3D fifo
             2 = microcode refusal to freeze 3D fifo (2D is already frozen)

   */


  count = 1000;
  ntiks = HZ / 60;
  freezeAborted=1;
  GMBASE_INIT(seg_reg, pDevP);
  freezeflag = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.freeze_3d;
  /* printf("0freezeflag = %d\n", *freezeflag);
     printf("0pDevP->gmbase = 0x%x\n", pDevP->gmbase);
     printf("0seg_reg = 0x%x\n", seg_reg); */

  /* We will give the microcode ten seconds to process our request to freeze
     the 3D fifo.  If the 2D fifo is frozen, we will re-issue the request */
     
  while (freezeAborted) {
    freezeAborted=0;
    while ( *freezeflag == 1 && count )
    {
      --count;
      delay(ntiks);
    }
    if (*freezeflag == 2) {
      freezeAborted=1;
      delay(1);
      FREEZE_3D(pDevP);
      }
  }
  /* printf("Out of loop 1.  freezeflag = 0x%d\n", *freezeflag);
     printf("1pDevP->gmbase = 0x%x\n", pDevP->gmbase);
     printf("1seg_reg = 0x%x\n", seg_reg); */

  /* If we enter this loop, it means 10 seconds were not enough to freeze the
     3D fifo.  We will "trash" the 3D fifo, and make sure the 2D fifo is
     enabled before attempting to freeze the 3D fifo again */

  if (*freezeflag) { 
    dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.flush_drp;
    *dp = 1;
    dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.reset_3d;
    *dp = 1;
  
    start_2D.len = sizeof(generic_se);
    start_2D.opcode = CE_EDOF;
    start_2D.data   = ENABLE_FIFO;
    WTFIFO(TravSeFifo, &start_2D, start_2D.len, seg_reg, pDevP);
       
    count = 1000;
    freezeAborted=1;

  /* We will give the microcode ten seconds to process our request to freeze
     the 3D fifo.  If the 2D fifo is frozen, we will re-issue the request */

    while (freezeAborted) {
      freezeAborted=0;
      while ( (*freezeflag == 1) && (count > 0) ) 
      {
        --count;
        delay(ntiks);
      }
      if (*freezeflag == 2) {
        freezeAborted=1;
        delay(1);
        FREEZE_3D(pDevP);
      }
    }

  /* printf("Out of loop 2.  freezeflag = 0x%d\n", *freezeflag);
     printf("2pDevP->gmbase = 0x%x\n", pDevP->gmbase);
     printf("2seg_reg = 0x%x\n", seg_reg); */

    if (!count)
    { /* 
       * wait here, so that we don't lock the box up any further
       */
/*
    GM_UNLOCK_LDAT(pDevP);
    GM_UNGUARD(&(gdp->domain[0]));
*/

    freezeAborted=1;

    while (freezeAborted) {
      freezeAborted=0;
      while ( *freezeflag == 1 ) 
      {
        delay(ntiks);
      }
      if (*freezeflag == 2) {
        freezeAborted=1;
        delay(1);
        FREEZE_3D(pDevP);
      }
    }

  /* printf("Out of loop 3.  freezeflag = 0x%d\n", *freezeflag);
     printf("3pDevP->gmbase = 0x%x\n", pDevP->gmbase);
     printf("3seg_reg = 0x%x\n", seg_reg); */

/*
      GM_LOCK_LDAT(pDevP);
      GM_GUARD(&(gdp->domain[0]), gdp->domain[0].pCurProc);
*/
    }
    /* Send a sync down the traversal fifo because we may have flushed
     * a sync which was in the fifo.
     */
     sync_element[0]=0x001801e1; /* Move sync data to global memory */
     sync_element[1]=VME_ADR(IMM_SYNC_CNTR);
     sync_element[2]=DISP_BUF_OFFSET; sync_element[3]=0x00000002;
     sync_element[4]=0x00000000; /* don't care */
     sync_element[5]=pDevP->trv_sync_cntr;
     WTFIFO(TravSeFifo,sync_element,0x0018, seg_reg, pDevP);
  }

  /*
   * If the rcx bound to the geometry that we're updating has its context
   * active on the fifo, then we can process things that affect the
   * context
   */
  if (pGemrcx &&
      (((pGemrcx->cxt_type == IMM_RCXTYPE || fifo_num == TravSeFifo) &&
	pRcx == pRcx->pDomain->pCur) ||
       pGemrcx->cxt_type == TRAV_RCXTYPE && fifo_num == ImmSeFifo &&
       pGemrcx->status_flags & ON_ADAPT &&
       VME_ADR(&((TRAV_CONTEXT *)
		 pDevP->slots[pGemrcx->start_slot].slot_addr)->asl) ==
         ((GM_MMAP *)pDevP->gmbase)->gm_ucflags.trv_cxt_add))
       
  { BUSMEM_DET(seg_reg);
    if ((*gai_chg_mask & gCWclip || rcm_chg_mask & rZBuf)) {
      if (pDevP->num_zbuf_wind>1 && pGemrcx->cxt_type==TRAV_RCXTYPE) { 
         gem_protect_zbuffer(gdp, pWG->wg.pClip, fifo_num, buf_start, pBuf,
	  	  buf_size); 
      }
    }
    /*
     * Note that set_FBC must come before setwin so that if the WATT
     * structure element sets the color table, the correct WID will be used
     */
    set_FBC(pGemrcx, hwid, pWG, gai_chg_mask, rcm_chg_mask, pBuf);

    gem_setwin(pDevP, pRcx, pGemrcx, &pWG->wg, pWG->wg.winOrg.x,
	       pWG->wg.winOrg.y, pWG->wg.width, pWG->wg.height, cm,
	       gai_chg_mask, rcm_chg_mask, pBuf);
  }
  else if (*gai_chg_mask & gCWcolormap)
  { BUSMEM_DET(seg_reg);
    /*
     * We don't want to defer changing the colormap if we don't have to.
     * Otherwise, a window may appear to have the wrong colormap, even
     * after an update_geometry is done with the colormap change bit set.
     */
    change_colormap_only(gdp, hwid, cm, gai_chg_mask, pBuf);
  }
  else
    BUSMEM_DET(seg_reg);

  *gai_chg_mask &= ~gCWclip;

  return(0);
}

/*
 * FUNCTION:  activate_private_cxt
 *
 * DESCRIPTION:
 */
void activate_private_cxt(gdp, pRcx, pGemrcx, pRCMcxt, rcm_chg_mask, react,
			  pBuf)
struct _gscDev	*gdp;
rcxPtr		pRcx;
rGemrcxPtr	pGemrcx;
TRAV_CONTEXT	*pRCMcxt;
ushort		*rcm_chg_mask;
Bool		*react;
char		**pBuf;
{
  int			len;
  ac_se			*pACbuf;
  generic_vme		*pVme;

  /*
   * Before activating the RCM's private context, if the context we want
   * active when we're all done is currently active, we need to save its DSV.
   */

  if (*rcm_chg_mask & rCxtIsActive)
  { if (!pGemrcx || (pGemrcx->cxt_type == TRAV_RCXTYPE &&
		     !(*rcm_chg_mask & rTravFifo)))
      len = store_dsv(gdp, gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur, *pBuf);
    else
      len = store_dsv(gdp, pRcx, *pBuf);
    *pBuf += len;
    *rcm_chg_mask &= ~rCxtIsActive;
    *react = TRUE;
  }
  pACbuf = (ac_se *)*pBuf;
  pACbuf->len = sizeof(ac_se);
  pACbuf->opcode = CE_ACTF;
  pACbuf->adr = VME_ADR(&pRCMcxt->asl);
  *pBuf += sizeof(ac_se);
  pVme = (generic_vme *)*pBuf;
  pVme->len = sizeof(generic_vme);
  pVme->opcode = CE_LDSV;
  pVme->adr = VME_ADR(pRCMcxt + 1);
  *pBuf += sizeof(generic_vme);

}

/*
 * FUNCTION:  steal_wid
 *
 * DESCRIPTION:
 */
void steal_wid(pDevP, real_wg, hwid, pBuf)
rGemRCMPrivPtr	pDevP;
struct _rcmWG	*real_wg;
long		hwid;
char		**pBuf;
{
  int			cm, bdb, odb;
  int			prot_id;
  uchar			tmp;
  ulong			seg_reg;
  struct _rcmWG 	*old_wg;

  /*
   * Find the window geometry that we're stealing the wid from
   */
  old_wg = pDevP->hwid[hwid].pwg;

  if (old_wg->wg.cm_handle != NULL)
    cm = ((rcmCmPtr)old_wg->wg.cm_handle)->hwd_map;
  else
    cm = NUM_VLTS - 1;

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

  GMBASE_INIT(seg_reg, pDevP);

#ifdef GEM_DBUG_SYNC
  printf(" IMM_SYNC_CNTR = 0x%x\n", IMM_SYNC_CNTR);
  printf(" Waiting for sync counter to reach 0x%x\n",
	 ((rWGPrivPtr)old_wg->pPriv)->imm_sync_cntr);
#endif

  /* Find out which display buffers the window is using */

 /* 
  while (*IMM_SYNC_CNTR < ((rWGPrivPtr)old_wg->pPriv)->imm_sync_cntr)
#ifdef GEM_DBUG_SYNC
  { volatile ulong zyx;

    printf(" *IMM_SYNC_CNTR = 0x%x\n", *IMM_SYNC_CNTR);
    for (zyx=0; zyx<45000000; zyx++) ;
  }
#endif GEM_DBUG_SYNC
  ;
   */

#ifdef GEM_DBUG_SYNC
  printf(" TRV_SYNC_CNTR = 0x%x\n", TRV_SYNC_CNTR);
  printf(" Waiting for sync counter to reach 0x%x\n",
	 ((rWGPrivPtr)old_wg->pPriv)->trv_sync_cntr);
#endif

 /*
  while (*TRV_SYNC_CNTR < ((rWGPrivPtr)old_wg->pPriv)->trv_sync_cntr)
#ifdef GEM_DBUG_SYNC
  { volatile ulong zyx;

    printf(" *TRV_SYNC_CNTR = 0x%x\n", *TRV_SYNC_CNTR);
    for (zyx=0; zyx<45000000; zyx++) ;
  }
#endif GEM_DBUG_SYNC
  ;
   */

  /*
   * All SEs connected with this hwid have now been executed
   */
  tmp = DISP_BUFFS[(hwid/16)*4 + 3 - ((hwid%16) / 4)];

  if (tmp & (1 << hwid % 4 * 2))
    bdb = 1;
  else
    bdb = 0;
  if (tmp & (1 << hwid % 4 * 2 + 1))
    odb = 1;
  else
    odb = 0;

  BUSMEM_DET(seg_reg);

  prot_id = pDevP->prot_hwids[cm][bdb][odb];

#ifdef HWID_CHAIN
  printf("#%d->#%d", hwid, prot_id);
#endif

  ((rWGPrivPtr)old_wg->pPriv)->hwid = prot_id - NUM_HWIDS;
  pDevP->hwid[hwid].pwg = real_wg;

  /*
   * Note that UNDRAW will set the frame buffer comparison.  This is
   * reset to the rcm's private hwid in draw_new_regions: we can only
   * get here if StealHwid is set, and that is only set if NewHwid is set.
   */
  UNDRAW_HWID( *pBuf, old_wg, hwid, prot_id );

#ifdef HWID_CHAIN
  printf("!  ");
#endif
}

/*
 * FUNCTION:  draw_new_regions
 *
 * DESCRIPTION:
 */
void draw_new_regions(pDevP, pWG, real_wg, hwid, cm, gai_chg_mask,
		      rcm_chg_mask, fifo_num, buf_start, pBuf, buf_size)
rGemRCMPrivPtr	pDevP;
struct _rcmWG 	*pWG;
struct _rcmWG	*real_wg;
long		*hwid;
int		cm;
ulong		*gai_chg_mask;
ushort		rcm_chg_mask;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{
  int			bdb, odb;
  int			old_cm;
  Bool			found;
  int			prot_id;
  gRegionPtr		pWindowRegion;
  frame_buf_cmp		*pFcmp;
  windattrs		*pWattr;
  generic_se		*pSe;

  /*
   * If hwid is a holding id, find which one.
   */
  if (*hwid < 0)
  { *hwid += NUM_HWIDS;

    /*
     * If the colormap has changed, we need to compute the new holding
     * id based on the new color table.
     */
    if (*gai_chg_mask & gCWcolormap)
    { old_cm = 0;
      found = FALSE;
      while (old_cm < NUM_VLTS && !found)
      { bdb = 0;
	while (bdb < 2 && !found)
	{ odb = 0;
	  while (odb < 2 && !found)
	    if (pDevP->prot_hwids[old_cm][bdb][odb] == *hwid)
	      found = TRUE;
	    else
	      odb++;
	  if (!found)
	    bdb++;
	}
	if (!found)
	  old_cm++;
      }

      *hwid = pDevP->prot_hwids[cm][bdb][odb];
      ((rWGPrivPtr)real_wg->pPriv)->hwid = *hwid - NUM_HWIDS;
      *gai_chg_mask &= ~gCWcolormap;
    }
  }

  if (rcm_chg_mask & rNewHwid)
  {
    /*
     * We need to make sure that the proper values are set
     * in the window id look-up table.
     */

    prot_id = rcm_chg_mask >> 8;
    found = FALSE;
    if (*gai_chg_mask & gCWcolormap)
    { old_cm = 0;
      while (old_cm < NUM_VLTS && !found)
      { bdb = 0;
	while (bdb < 2 && !found)
	{ odb = 0;
	  while (odb < 2 && !found)
	    if (pDevP->prot_hwids[old_cm][bdb][odb] == prot_id)
	      found = TRUE;
	    else
	      odb++;
	  if (!found)
	    bdb++;
	}
	if (!found)
	  old_cm++;
      }
      *gai_chg_mask &= ~gCWcolormap;
    }
    else
    { bdb = 0;
      while (bdb < 2 && !found)
      { odb = 0;
	while (odb < 2 && !found)
	  if (pDevP->prot_hwids[cm][bdb][odb] == prot_id)
	    found = TRUE;
	  else
	    odb++;
	if (!found)
	  bdb++;
      }
    }

    /* Set window compare value						*/
    pFcmp = (frame_buf_cmp *)*pBuf;					
    pFcmp->len = sizeof(frame_buf_cmp);
    pFcmp->opcode = CE_FBC;
    pFcmp->flags = 0;
    pFcmp->mask = 0;
    pFcmp->value = *hwid;
    *pBuf += sizeof(frame_buf_cmp);
      
    /* Set Color table							*/
    pWattr = (windattrs *)*pBuf;
    pWattr->len = sizeof(windattrs);
    pWattr->opcode = CE_WATT;
    pWattr->mask = waColorMode | waObscurity | waOrigin | waSize;
    pWattr->ctid = cm;
    pWattr->flags = 0;
    *pBuf += sizeof(windattrs);

    /* Select base planes						*/
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_SDFB;
    pSe->data = BASE_PLANES;
    *pBuf += sizeof(generic_se);

    /* Set display buffer						*/
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_FCTL;
    pSe->data = bdb << 30;
    *pBuf += sizeof(generic_se);

    /* Select overlay planes						*/
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_SDFB;
    pSe->data = OVERLAY_PLANES;
    *pBuf += sizeof(generic_se);

    /* Set display buffer						*/
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_FCTL;
    pSe->data = odb << 30;
    *pBuf += sizeof(generic_se);

    /* Select window planes						*/
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_SDFB;
    pSe->data = WINDOW_PLANES;
    *pBuf += sizeof(generic_se);

    /* Reset window compare value to private value			*/
    pFcmp = (frame_buf_cmp *)*pBuf;					
    pFcmp->len = sizeof(frame_buf_cmp);
    pFcmp->opcode = CE_FBC;
    pFcmp->flags = 0;
    pFcmp->mask = 0;
    pFcmp->value = pDevP->priv_hwid;
    *pBuf += sizeof(frame_buf_cmp);
  }

  pWindowRegion = pWG->wg.pClip;

  if(pWindowRegion)
  { /*
     * Set int color
     */
    pSe = (generic_se *)*pBuf;
    pSe->len    = sizeof(generic_se);
    pSe->opcode = CE_INCI;
    pSe->data   = *hwid;
    *pBuf += sizeof(generic_se);

#ifdef GEM_DBUG
    { int i;
      printf("%s %d Drawing hwid %d into window planes\n", __FILE__, __LINE__,
	   *hwid);
      printf("   Current number of boxes = %d:\n",pWindowRegion->numBoxes);   
      for (i=0; i<pWindowRegion->numBoxes; ++i)
      { printf("   (%d, %d) - (%d, %d)\n", pWindowRegion->pBox[i].ul.x,
	       pWindowRegion->pBox[i].ul.y, pWindowRegion->pBox[i].lr.x,
	       pWindowRegion->pBox[i].lr.y);
      }
    }
#endif

    gem_drawwinreg(
		   pDevP,
		   pWindowRegion,		/* Regions to fill	*/
		   fifo_num,
		   buf_start,			/* SE buffer		*/
		   pBuf,			/* SE buffer pointer	*/
		   buf_size			/* SE buffer size	*/
		   );

  }
}

/*
 * FUNCTION:  draw_client_clip
 *
 * DESCRIPTION:
 */
void draw_client_clip(pDevP, pWA, pWG, hwid, changemask, pRCMcxt, fifo_num,
		      buf_start, pBuf, buf_size)
rGemRCMPrivPtr	pDevP;
struct _rcmWA	*pWA;
struct _rcmWG 	*pWG;
long		hwid;
long		*changemask;
TRAV_CONTEXT	*pRCMcxt;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{
  gWinGeomAttrPtr	pwg;
  generic_se		*pSe;
  frame_buf_cmp		*pFcmp;
  windattrs		*pWattr;
  struct _sgm_buf {
      sgm_se        sgm_xform;		/* Xform matrix			*/
      ulong         vtm;                /* View Transformation Matrix	*/
    }			*sgm_buf;

  /*
   * Set local variable(s)
   */
  pwg = &pWG->wg;

  /*
   * Set frame buffer mask and frame buffer comparison accordingly
   */
  pSe = (generic_se *)*pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_FMSK;
  pSe->data = GAI_GPM_WPMASK;
  *pBuf += sizeof(generic_se);
  pFcmp = (frame_buf_cmp *)*pBuf;
  pFcmp->len = sizeof(frame_buf_cmp);
  pFcmp->opcode = CE_FBC;
  pFcmp->flags = 0;
  if (pwg->pClip)
    pFcmp->mask = GAI_WIN_PLANES;
  else
    pFcmp->mask = 0;
  pFcmp->value = hwid;
  *pBuf += sizeof(frame_buf_cmp);

  /*
   * Since regions are relative to window, set window attributes
   * as per values in window geometry
   */
  pWattr = (windattrs *)*pBuf;
  pWattr->len = sizeof(windattrs);
  pWattr->opcode = CE_WATT;
  pWattr->mask = waColorTableID | waColorMode;
  pWattr->flags = 0;
  pWattr->x = pwg->winOrg.x;
  pWattr->y = GM_HEIGHT - pwg->winOrg.y - pwg->height;
  pWattr->width = pwg->width;
  pWattr->height = pwg->height;
  *pBuf += sizeof(windattrs);

  if (pWA->wa.RegOrigin != REG_ORG_LL)
  { /* Put height of window in transformation matrix			*/
    sgm_buf = (struct _sgm_buf *)*pBuf;
    sgm_buf->sgm_xform.len = sizeof(struct _sgm_buf);
    sgm_buf->sgm_xform.opcode = CE_GMEM;
    sgm_buf->sgm_xform.adr = VME_ADR(&pRCMcxt->vt.vtm[13]);
    sgm_buf->vtm = itof(pWattr->height - 1);
    *pBuf += sizeof(struct _sgm_buf);
  }
  else	/* Region Origin is Lower Left	*/
  { /* Set transformation matrix to default				*/
    sgm_buf = (struct _sgm_buf *)*pBuf;
    sgm_buf->sgm_xform.len = sizeof(struct _sgm_buf);
    sgm_buf->sgm_xform.opcode = CE_GMEM;
    sgm_buf->sgm_xform.adr = VME_ADR(&pRCMcxt->vt.vtm[5]);
    sgm_buf->vtm = itof(1);
    *pBuf += sizeof(struct _sgm_buf);

    sgm_buf = (struct _sgm_buf *)*pBuf;
    sgm_buf->sgm_xform.len = sizeof(struct _sgm_buf);
    sgm_buf->sgm_xform.opcode = CE_GMEM;
    sgm_buf->sgm_xform.adr = VME_ADR(&pRCMcxt->vt.vtm[13]);
    sgm_buf->vtm = 0;
    *pBuf += sizeof(struct _sgm_buf);
  }

  /* Clear GAI_GAI_PLANE in exposed rgns*/
  CLEAR_AND_SET_COLOR( *pBuf, pwg->width, pwg->height, GAI_GPM_PLANE);

  gem_drawwinreg		    /* Draw regions into global plane    */
    (				    /* mask based on "Window clipping    */
				    /* Regions"			         */
     pDevP,
     pWA->wa.pRegion,		    /* Regions to fill                   */
     fifo_num,
     buf_start,
     pBuf,			    /* SE buffer                         */
     buf_size
     );

  if (*pBuf + 1024 > buf_start + buf_size)
    flush_buf(pDevP, fifo_num, buf_start, pBuf);

  /*
   * Reset window attributes back to full screen
   */
  pWattr = (windattrs *)*pBuf;
  pWattr->len = sizeof(windattrs);
  pWattr->opcode = CE_WATT;
  pWattr->mask = waColorTableID | waColorMode;
  pWattr->flags = UNOBSCURED;
  pWattr->x = 0;
  pWattr->y = 0;
  pWattr->width = GM_WIDTH;
  pWattr->height = GM_HEIGHT;
  *pBuf += sizeof(windattrs);

  sgm_buf = (struct _sgm_buf *)*pBuf;
  sgm_buf->sgm_xform.len = sizeof(struct _sgm_buf);
  sgm_buf->sgm_xform.opcode = CE_GMEM;
  sgm_buf->sgm_xform.adr = VME_ADR(&pRCMcxt->vt.vtm[13]);
  sgm_buf->vtm = itof(GM_HEIGHT - 1);
  *pBuf += sizeof(struct _sgm_buf);

  if (pWA->wa.RegOrigin == REG_ORG_LL)
  { /* Reset transformation matrix back to upper left			*/
    sgm_buf = (struct _sgm_buf *)*pBuf;
    sgm_buf->sgm_xform.len = sizeof(struct _sgm_buf);
    sgm_buf->sgm_xform.opcode = CE_GMEM;
    sgm_buf->sgm_xform.adr = VME_ADR(&pRCMcxt->vt.vtm[5]);
    sgm_buf->vtm = itof(-1);
    *pBuf += sizeof(struct _sgm_buf);
  }

  /*
   * Reset frame buffer mask back to window planes, and window compare
   * value back to private value
   */
  pSe = (generic_se *)*pBuf;
  pSe->len = sizeof(generic_se);
  pSe->opcode = CE_FMSK;
  pSe->data = GAI_WIN_WPMASK;
  *pBuf += sizeof(generic_se);
  pFcmp = (frame_buf_cmp *)*pBuf;
  pFcmp->len = sizeof(frame_buf_cmp);
  pFcmp->opcode = CE_FBC;
  pFcmp->flags = 0;
  pFcmp->mask = 0;
  pFcmp->value = pDevP->priv_hwid;
  *pBuf += sizeof(frame_buf_cmp);

  /*
   * Reset flag in window attribute's changemask to indicate that
   * client clip has already been drawn
   */
  *changemask &= ~gCWpRegion;
}

/*
 * FUNCTION:  activate_client_context
 *
 * DESCRIPTION:
 */
void activate_client_context(gdp, pRcx, pGemrcx, rcm_chg_mask, react, pBuf)
struct _gscDev	*gdp;
rcxPtr		pRcx;
rGemrcxPtr	pGemrcx;
ushort		rcm_chg_mask;
Bool		react;
char		**pBuf;
{
  int			len;
  Bool			new;

  /*
   * (Re-)activate current process's context
   */
  if (!pGemrcx || (pGemrcx->cxt_type == TRAV_RCXTYPE &&
		   !(rcm_chg_mask & rTravFifo)))
  { len = activate_cxt_fast(gdp, gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur,
       *pBuf);
    *pBuf += len;
    len = load_dsv(gdp, gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur, *pBuf);
    *pBuf += len;
  }
  else
  { len = activate_cxt(gdp, pRcx, *pBuf);
    *pBuf += len;
    new = pGemrcx->status_flags & NEW_CXT;
    len = load_dsv(gdp, pRcx, *pBuf);
    *pBuf += len;
    /*
     * If we're activating a traversal context in the traversal fifo,
     * and we're not just REactivating it due to activating the RCM's
     * private context above, and it's not a new context, then the context
     * is not active because it was swapped off, and we must restore the
     * pick attribute stack that was saved when it was swapped off.
     */
    if (pGemrcx->cxt_type == TRAV_RCXTYPE && !react && !new)
    { len = restore_pick_stack(gdp, pRcx, *pBuf);
      *pBuf += len;
    }
  }
}

/*
 * FUNCTION:  set_FBC
 *
 * DESCRIPTION:
 */
void set_FBC(pGemrcx, hwid, pWG, gai_chg_mask, rcm_chg_mask, pBuf)
rGemrcxPtr	pGemrcx;
long		hwid;
struct _rcmWG 	*pWG;
ulong		*gai_chg_mask;
ushort		rcm_chg_mask;
char		**pBuf;
{
  /*
   * Need to set the window planes compare mask according to obscurity,
   * and set the compare value to the wid
   */

  /* Set private version of compare value				*/
  pGemrcx->win_comp_val  &= ~GAI_WIN_PLANES;
  pGemrcx->win_comp_val  |= hwid;

  /*
   * Turn on windowing appropriately
   */
  if ( iggm_unobscured(&pWG->wg) )
  {
#ifdef TRACKWIND
    printf("Window Planes OFF\n");
#endif

    pGemrcx->win_comp_mask &= ~GAI_WIN_PLANES;
  }
  else
  {
#ifdef TRACKWIND
    printf("Window Planes ON\n");
#endif

    pGemrcx->win_comp_mask |= GAI_WIN_PLANES;
  }

  SET_WINDOW_COMP(
		  *pBuf,			/* SE buffer		*/
		  pGemrcx->win_comp_mask,	/* compare mask		*/
		  pGemrcx->win_comp_val,	/* compare value	*/
		  pGemrcx->cxt_type,		/* context type		*/
		  rcm_chg_mask
		  );
}

/*
 *  FUNCTION NAME: gem_setwin			               
 *                                                                     
 * Generate & send window org and window clipping structure elements   
 */
void gem_setwin(pDevP, pRcx, pGemrcx, pwg, x, y, width, height, ctid,
		gai_chg_mask, rcm_chg_mask, ppbuf)
rGemRCMPrivPtr	pDevP;
rcxPtr		pRcx;
rGemrcxPtr	pGemrcx;
gWinGeomAttrPtr	pwg;
int		x, y;
ushort		width, height;
int		ctid;
ulong		*gai_chg_mask;
ushort		rcm_chg_mask;
char            **ppbuf;
{
  IMM_CONTEXT   *gmcxt;
  windattrs	*pOutbuf;
  ushort	obsc_flag;
  uchar		mask;
  int		rcxtype;
  struct _sgm_buf {
      sgm_se        sgm_xform;		/* Xform matrix			*/
      ulong         vtm;                /* View Transformation Matrix @3*/
    }		*sgm_buf;

  pOutbuf = (windattrs *)(*ppbuf);

#ifdef DBUGORG
  printf("setwin: x=%d y=%d w=%d h=%d\n",
	 x,y,width,height);
#endif

  rcxtype = pGemrcx->cxt_type;

  /*
   * Unless we determine otherwise, set everything except color mode
   */
  mask = waColorMode;

  /*
   * Since obscurity is mainly determined by clipping regions, and the
   * clipping regions might have been changed in an earlier call to upd_geom,
   * we must always check obscurity.
   */
  if ( iggm_unobscured(pwg) )
    obsc_flag = UNOBSCURED;
  else
    obsc_flag = 0;

  if (!(*gai_chg_mask & gCWcolormap || rcm_chg_mask & rNewHwid))
    mask |= waColorTableID;
  else
    *gai_chg_mask &= ~gCWcolormap;

  if (!(*gai_chg_mask & gCWwinOrg || *gai_chg_mask & gCWheight
	|| rcm_chg_mask & rSetWinOrgSize)) {
    mask |= waOrigin;
  } else {
    pOutbuf->x     = x;
    /*
     * Translate Window Origin to Lower Left Coord System
     * and lower left corner of window
     */
    pOutbuf->y     = ((GM_HEIGHT) - y) - height;
    *gai_chg_mask &= ~gCWwinOrg;
  }

  if (!(*gai_chg_mask & gCWwidth || *gai_chg_mask & gCWheight ||
	rcm_chg_mask & rSetWinOrgSize)) {
    mask |= waSize;
  } else {
    /*
     * Init width and height to new values
     */
    pOutbuf->width = width;
    pOutbuf->height = height;
    /*
     * Selectively set window width and height if not being called with
     * internal rcm changemask: rSetWinOrgSize
     */
    if(!(rcm_chg_mask & rSetWinOrgSize)) {

      if (!(*gai_chg_mask & gCWwidth)) /* Xserver didn't change width */
	pOutbuf->width = pRcx->pWG->wg.width;	/* Reset to current value */

      if (!(*gai_chg_mask & gCWheight)) /* Xserver didn't change height */
	pOutbuf->height = pRcx->pWG->wg.height;	/* Reset to current value */
    }
    *gai_chg_mask &= ~gCWwidth;
  }

  /*
   * Init Set Window Attribute se based on window geometry
   */
  pOutbuf->len = sizeof(windattrs);
  pOutbuf->opcode = CE_WATT;
  pOutbuf->mask = mask;
  pOutbuf->ctid = ctid;
  pOutbuf->flags = obsc_flag;	/* @2 */
  if (rcxtype == TRAV_RCXTYPE && !(rcm_chg_mask & rTravFifo))
    pOutbuf->flags |= 0x0001;


  /*
   * Update pointer
   */
  *ppbuf += sizeof(struct windattrs);

  /*
   * Set proper value in transformation matrix to get proper upper-left
   * coordinate window if 2d process
   * NB: When we implement 2D DWA clients, we will really want to check
   * to see if the rcx is X's rcx, not just if it's an immediate rcx.
   */
  if (rcxtype == IMM_RCXTYPE &&
      (*gai_chg_mask & gCWheight || rcm_chg_mask & rSetWinOrgSize))
  { sgm_buf = (struct _sgm_buf *)(*ppbuf);

    sgm_buf->sgm_xform.len = sizeof(sgm_se) + sizeof(sgm_buf->vtm);
    sgm_buf->sgm_xform.opcode = CE_GMEM;

    gmcxt  = (IMM_CONTEXT *)VME_ADR(pGemrcx->pASL);
    sgm_buf->sgm_xform.adr = VME_ADR(&gmcxt->vt.vtm[13]);

    /*
     * Since floating point is not permitted in the kernel, use conversion
     * routine.
     */
    sgm_buf->vtm = itof(height - 1);			    /*@3*/

    /*
     * Return updated pointer
     */
    *ppbuf += sizeof(struct _sgm_buf);
  }

  *gai_chg_mask &= ~gCWheight;

  return;

}

/*
 * FUNCTION:  change_colormap_only
 *
 * DESCRIPTION:
 */
void change_colormap_only(gdp, hwid, cm, gai_chg_mask, pBuf)
struct _gscDev	*gdp;
long		hwid;
int		cm;
ulong		*gai_chg_mask;
char		**pBuf;
{
  int			tmp_id;
  frame_buf_cmp		*pFcmp;
  windattrs		*pWattr;

  /*
   * Find and save the window id of the current context.  This must be
   * valid, since the current context always has a valid window id.
   */
  tmp_id = ((rWGPrivPtr)
	    gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur->pWG->pPriv)->hwid;

  /*
   * Change the window compare value to the hwid of the geometry being
   * updated.  Note that the window id must be valid, or we would have hit
   * case c) in upd_geom, and the colormap bit would no longer be set.  Also
   * note that we want to avoid the ucode doing a fifo switch while we have
   * a 3d process's window id set, but since we never defer doing this
   * change, we will always be doing it while called from update_window_
   * geometry, and so the traversal fifo will be stopped.
   */
  pFcmp = (frame_buf_cmp *)*pBuf;
  pFcmp->len = sizeof(frame_buf_cmp);
  pFcmp->opcode = CE_FBC;
  pFcmp->flags = 0x0000000c;
  pFcmp->mask = 0;
  pFcmp->value = hwid;
  *pBuf += sizeof(frame_buf_cmp);
      
  /* Set Color table							*/
  pWattr = (windattrs *)*pBuf;
  pWattr->len = sizeof(windattrs);
  pWattr->opcode = CE_WATT;
  pWattr->mask = waColorMode | waObscurity | waOrigin | waSize;
  pWattr->ctid = cm;
  pWattr->flags = 0;
  *pBuf += sizeof(windattrs);

  /* Reset the window compare value					*/
  pFcmp = (frame_buf_cmp *)*pBuf;
  pFcmp->len = sizeof(frame_buf_cmp);
  pFcmp->opcode = CE_FBC;
  pFcmp->flags = 0x0000000c;
  pFcmp->mask = 0;
  pFcmp->value = tmp_id;
  *pBuf += sizeof(frame_buf_cmp);

  /* Turn off colormap change flag					*/
  *gai_chg_mask &= ~gCWcolormap;
}

void flush_buf(pDevP, fifo_num, buf_start, ppBuf)
rGemRCMPrivPtr	pDevP;
int		fifo_num;
char		*buf_start;
char		**ppBuf;
{
  ulong		seg_reg;

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  WTFIFO( fifo_num, buf_start, *ppBuf - buf_start, seg_reg, pDevP);
  BUSMEM_DET(seg_reg);
  *ppBuf = buf_start;
}
