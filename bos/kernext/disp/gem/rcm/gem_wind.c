static char sccsid[] = "@(#)00	1.6.3.6  src/bos/kernext/disp/gem/rcm/gem_wind.c, sysxdispgem, bos411, 9428A410j 8/20/93 06:52:29";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_bltwinmask
 *		gem_chgm
 *		gem_chgr
 *		gem_drawwinreg
 *		gem_grp
 *		gem_upd_win
 *		iggm_update_win_attr
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



#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"
#include "gem_gai.h"
#include "gem_geom.h"

int  gem_upd_win();
int  gem_grp();
int  gem_chgm();
void gem_chgr();
int  gem_bltwinmask();
void gem_drawwinreg();

int iggm_update_win_attr(gdp, rp, newwap, changemask)
struct _gscDev	*gdp;
rcxPtr		rp;
struct _rcmWA	*newwap;
int		changemask;

{
  char    	se_buffer[8192];/* small private SE buffer          */
  int           rc;
  char          *pBuf;
  int		fifo_num;
  int		RCMChgMask;
  rGemRCMPrivPtr pDevP;

  ulong		seg_reg;

  if(rp == NULL) {
#ifdef GEOM_DBUG
    printf("NULL rcx. Defering update_win_attr: chgmask=0x%x\n",changemask);
#endif
    /*
     * This window has never been bound to an rcx.  Save the changemask
     * in the window's private area.  Use the wg_handle in the HFT's
     * temp copy of the new window (newwap) to get at the real window.
     */
    
    ((rWAPrivPtr) ((rcmWAPtr)(newwap->wa.wa_handle))->pPriv)->gWAChangeMask |=
      changemask;
    
  }							     
  else {
    /*
     * This routine is called for each rcx "linked" to the window.
     * We only want to do the update for the rcx that's current on its
     * domain.  Otherwise we defer the update by storing the changemask
     * in any rcx that's not the current one on its domain.
     * The update for that rcx will occur when it becomes the current one.
     */
    if(rp->pDomain->pCur != rp) {
      
#ifdef WIN_DBUG
      printf("Defering update_win_attr: chgmask=0x%x\n",changemask);
#endif 
      ((rGemrcxPtr) rp->pData)->gWAChangeMask |= changemask;
    }							     
    else {
      
#ifdef WIN_DBUG
      printf("Doing update_win_attr now: chgmask=0x%x\n",changemask);
#endif
      
      /*
       * Guard the domain do block context switch
       */
      GM_GUARD(rp->pDomain, rp->pProc);
      
      /*
       * Init local vars
       */
      pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
      pBuf = se_buffer;		/* ptr to SE tmp buffer     */

      if (((rGemrcxPtr)rp->pData)->cxt_type == IMM_RCXTYPE)
	fifo_num = ImmSeFifo;
      else
	fifo_num = TravSeFifo;

      RCMChgMask = 0;

      rc = gem_upd_win(gdp, rp, newwap, rp->pWG, changemask, RCMChgMask,
		       fifo_num, pBuf, &pBuf, 8192);

      /*
       * Flush Structure Element Buffer to fifo                            
       */
      if(pBuf - se_buffer) {
	seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
	
	WTFIFO( fifo_num, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
	
	BUSMEM_DET(seg_reg);
      }
      ((rGemrcxPtr) rp->pData)->gWAChangeMask = 0;

      /*
       * Unguard the domain do block context switch
       */
      GM_UNGUARD(rp->pDomain);

    }
  }

  return(0);
}

  int
    gem_upd_win(gdp, pRcx, pWA, real_wg, changemask, rcm_chg_mask,
		fifo_num, buf_start, pBuf, buf_size)
struct _gscDev	*gdp;
rcxPtr		pRcx;
struct _rcmWA	*pWA;
struct _rcmWG	*real_wg;
int		changemask;
int		rcm_chg_mask;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{  
  int           	buflen, client_clip_mask;
  rGemrcxPtr         	pGemrcx;
  rGemRCMPrivPtr	pDevP;

  pGemrcx = (rGemrcxPtr) (pRcx->pData) ;
  pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);

  if (*pBuf + 1024 > buf_start + buf_size)
    flush_buf(pDevP, fifo_num, buf_start, pBuf);

  if (changemask &
      (gCWdrawBuf | gCWdispBuf | gCWallgroups | gCWnumGroups | gCWpActiveGroup) ||
      rcm_chg_mask & rSetWinAttrs && pWA != pRcx->pWA)
  {
    /*
     * Process things related to groups
     */
    buflen=gem_grp( pRcx, pWA, rcm_chg_mask, *pBuf );/*@4*/
    *pBuf += buflen;
  }

 
  /***************************************************************************/
  /* if client clipping mask or regions  ...                                 */
  /***************************************************************************/

  client_clip_mask = 0;
  if ( pGemrcx->cxt_type == TRAV_RCXTYPE )
     iggm_do_client_clip(gdp, pRcx->pWA, pWA, pRcx->pWG, &client_clip_mask);

  if ( ( changemask & gCWpRegion || rcm_chg_mask & rSetWinAttrs && pWA != pRcx->pWA 
         && !pWA->wa.pMask) )
  {
    gem_chgr( gdp, pDevP, pRcx, pWA, real_wg, fifo_num, buf_start, pBuf,
	     buf_size, client_clip_mask );
  }
  else if ( changemask & gCWpMask ||
	    rcm_chg_mask & rSetWinAttrs && pWA != pRcx->pWA )
  {
    buflen = gem_chgm( pRcx, pWA, *pBuf );
    *pBuf += buflen;
  }

return(0);

}


/************************************/
/* Process things related to groups */
/************************************/

gem_grp( pRcx, pWA, rcm_chg_mask, bufstart )
rcxPtr		pRcx;
struct _rcmWA 	*pWA;
int		rcm_chg_mask;
char		*bufstart;
{
  rDefGroupsPtr	pActiveGroup;
  rWAPrivPtr	pWAPriv;

  typedef struct outbuf {
    windattrs    watt;
    scpm_se	 cpm;
  } outbuf;
  outbuf 	*pWindattr;

  pg_sebuf 	*pSDFB;       /* Address of where to put se's              */
  char		*pBuf = bufstart;
  
#ifdef GEM_DBUG
  printf("Enter gem_grp: pRcx=0x%x, pWA=0x%x, rcm_chg_mask=0x%x, bufstart=0x%x\n",
	 pRcx, pWA, rcm_chg_mask, bufstart );
#endif

  if (rcm_chg_mask & rSetWinAttrs)
    pWAPriv = (rWAPrivPtr)(pWA->pPriv);
  else
    pWAPriv = (rWAPrivPtr)(pRcx->pWA->pPriv);
  
  pSDFB = (pg_sebuf *)(pBuf);

  /* Init se length and opcode */
  pSDFB->sfb.len = sizeof(pg_sebuf);
  pSDFB->sfb.opcode = CE_SDFB;
  
  pBuf += pSDFB->sfb.len;
  
  /*
   * Set Window Attr & CPM according to active group depth
   */
  pWindattr = (outbuf *) pBuf;
  pWindattr->watt.len = sizeof(windattrs);
  pWindattr->watt.opcode = CE_WATT;
  pWindattr->watt.mask = CPM_FLAG;
  pBuf += pWindattr->watt.len;

  pWindattr->cpm.len = sizeof(scpm_se);
  if (((rGemrcxPtr)pRcx->pData)->cxt_type == IMM_RCXTYPE)
    pWindattr->cpm.opcode = CE_SCPM;
  else
    pWindattr->cpm.opcode = SE_NOOP;
  pWindattr->cpm.PadRsh = 0;
  pWindattr->cpm.GshBsh = 0;

  pBuf +=  pWindattr->cpm.len;
  
#ifdef GEM_DBUG
  printf("gem_grp: num_groups=0x%x, pWAPriv->active_group=0x%x\n",
	 pWAPriv->num_groups,pWAPriv->active_group);
#endif
  
  if(pWAPriv->num_groups) {
    pActiveGroup =
      (rDefGroupsPtr)(&pWAPriv->groups_used[pWAPriv->active_group]);
    
#ifdef GEM_DBUG
    printf("pActiveGroup=0x%x ActiveGroup.depth=0x%x,ActiveGroup.type=0x%x\n",
	   pActiveGroup,pActiveGroup->depth,pActiveGroup->type);
#endif
    
    if (pActiveGroup->depth == 24) 
      pWindattr->watt.flags = CPM24;
    else
      pWindattr->watt.flags = CPM8;

    /* Select current frame buffer based on group type */
    switch(pActiveGroup->type) {
      
    case gStandardGroup:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    case gOverlayGroup:
      pSDFB->sfb.data = OVERLAY_PLANES;
      break;
      
    case gUnderlayGroup:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    case gZBufferGroup:
      pSDFB->sfb.data = Z_BUFFER;
      break;
      
    case gClipGroup:
      pSDFB->sfb.data = WINDOW_PLANES;
      break;
      
    case gMaskGroup:
      pSDFB->sfb.data = WINDOW_PLANES;
      break;
      
    case gCursorGroup:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    case gDataGroup:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    case gFIFOGroup:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    case gOverUnderGroup:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    default:
      pSDFB->sfb.data = BASE_PLANES;
      break;
      
    }
    
  }
  else {
    /*
     * Set CPM to default when groups not defined
     */
    pWindattr->watt.flags = CPM8;
    /*
     * Set drawing frame buffer to base planes when groups not defined
     */
    pSDFB->sfb.data = BASE_PLANES;
  }    
  
#ifdef GEM_DBUG
  printf("Exit gem_grp\n");
#endif
  return (pBuf-bufstart);
  
  
}


/****************************************************************************/
/* set client clipping mask                                                 */
/****************************************************************************/
int  gem_chgm( pRcx, pWA, pBuf )
rcxPtr		pRcx;
struct _rcmWA 	*pWA;
char		*pBuf;
{

  gPixmapPtr    pGPMMask;
  char          *pBuf_start;
  long		hwid;
  rGemrcxPtr	pGemrcx;

  pGemrcx = (rGemrcxPtr)pRcx->pData;
  pBuf_start = pBuf;

#ifdef NOTREADY

  pGPMMask   = pWA->wa.pMask;               /* Global Plane Mask pixmap */

  if ( pGPMMask == NULL )
  {
    pGemrcx->win_comp_mask &= ~GAI_GPM_PLANE;

#ifdef GEM_DBUG
    printf("Setting window compare mask = 0x%x\n", pGemrcx->win_comp_mask);
#endif

    RESET_BIT                           /* Don't use Global Plane Mask for   */
      (                                 /* future rendering operations       */
       pBuf,
       GAI_GPM_PLANE
       );
    return(pBuf - pBuf_start);
  }

  hwid = ((rWGPrivPtr)pRcx->pWG->pPriv)->hwid;

  SELECT_WINDOW                     /* Draw in window plane              */
    (
     pBuf,                          /* SE buffer                         */
     GAI_GPM_PLANE,                 /* fb mask                           */
     GAI_GPM_PLANE,                 /* color                             */
     GAI_WIN_PLANES,		    /* compare mask			 */
     hwid			    /* compare value			 */
     );

  CLEAR                             /* Clear GAI_GPM_PLANE               */
    (
     pBuf
     );

  gem_bltwinmask                    /* Draw into GAI_GPM_PLANE with      */
    (                               /* "Window clipping Mask"            */
     pWA,                           /* pointer to window resource        */
     pGPMMask,                      /* client's clip mask bitmap         */
     &pBuf			    /* SE buffer			 */
     );

  RESTORE_REGS                      /* Restore adapter registers &       */
    (                               /* select base planes                */
     pBuf
     );

  pGemrcx->win_comp_mask |= GAI_GPM_PLANE;
  pGemrcx->win_comp_val  |= GAI_GPM_PLANE;

#ifdef GEM_DBUG
    printf("Setting window compare mask = 0x%x\n", pGemrcx->win_comp_mask);
    printf("Setting window compare value = 0x%x\n", pGemrcx->win_comp_val);
#endif

  SET_BIT                           /* Use Global Plane Mask for future  */
    (                               /* rendering operations              */
     pBuf,
     GAI_GPM_PLANE
     );
#endif

  return (pBuf - pBuf_start);
}

/*****************************************************************************/
/* set client clipping region(s)                                             */
/*****************************************************************************/
void  gem_chgr( gdp, pDevP, pRcx, pWA, real_wg, fifo_num, buf_start, pBuf,
	       buf_size, client_clip_mask )
struct _gscDev	*gdp;
rGemRCMPrivPtr	pDevP;
rcxPtr		pRcx;
struct _rcmWA	*pWA;
struct _rcmWG	*real_wg;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
int             client_clip_mask;
{
  int		i;
  int		len;
  gRegionPtr    pGPMRegion;
  long		hwid;
  rGemrcxPtr	pGemrcx;
  gWinGeomAttrPtr	pwg;
  TRAV_CONTEXT	*pRCMcxt;
  ac_se		*pACbuf;
  generic_vme	*pVme;
  generic_se	*pSe;
  frame_buf_cmp	*pFcmp;
  windattrs	*pWattr;
  struct _sgm_buf {
      sgm_se        sgm_xform;		/* Xform matrix			*/
      ulong         vtm;                /* View Transformation Matrix @3*/
    }		*sgm_buf;

  pGemrcx = (rGemrcxPtr)pRcx->pData;

  pGPMRegion = pWA->wa.pRegion;             /* Global Plane Mask Regions*/

#ifdef GEM_DBUG
  printf("%s %d Setting client clip region - pRegion=%x\n",__FILE__,
	 __LINE__,pGPMRegion);
  if (pGPMRegion != NULL)
  { printf("   Current number of boxes = %d:\n",pGPMRegion->numBoxes);   
    for (i=0; i<pGPMRegion->numBoxes; ++i)
    { printf("   (%d, %d) - (%d, %d)\n",pGPMRegion->pBox[i].ul.x,
	     pGPMRegion->pBox[i].ul.y, pGPMRegion->pBox[i].lr.x,
	     pGPMRegion->pBox[i].lr.y);
    }
  }
#endif

  if ( pGPMRegion == NULL )
  {
    pGemrcx->win_comp_mask &= ~GAI_GPM_PLANE;

    RESET_BIT                           /* Don't use Global Plane Mask for   */
      (                                 /* future rendering operations       */
       *pBuf,
       GAI_GPM_PLANE
       );
    return;
  }

  if ( client_clip_mask )
  {
    /* Use Global Plane Mask for future rendering operations */

     pGemrcx->win_comp_mask |= GAI_GPM_PLANE;
     pGemrcx->win_comp_val  |= GAI_GPM_PLANE;

     SET_BIT( *pBuf, GAI_GPM_PLANE );

     return;
  }

  hwid = ((rWGPrivPtr)real_wg->pPriv)->hwid;
  pwg = &real_wg->wg;

  if (fifo_num == ImmSeFifo)
  { 
#ifdef GEM_DBUG
    printf(" SEs are going down immediate fifo\n");
#endif
    SELECT_WINDOW		    /* Set window plane mask             */
      (
       *pBuf,			    /* SE buffer                         */
       GAI_GPM_PLANE,		    /* frame buffer mask                 */
       GAI_GPM_PLANE,		    /* color                             */
       (pwg->pClip ? GAI_WIN_PLANES : 0), /* compare mask		 */
       hwid			    /* compare value			 */
       );
  }
  else
  {
#ifdef GEM_DBUG
    printf(" SEs are going down traversal fifo\n");
#endif
    /*
     * SEs are going down traversal fifo.  Since we don't know the state
     * of the traversal context, we activate the RCM's context.
     */
    pRCMcxt = (TRAV_CONTEXT *)pDevP->pPrivCxt;
    len = store_dsv(gdp, pRcx, *pBuf);
    *pBuf += len;
    pACbuf = (ac_se *)*pBuf;
    pACbuf->len = sizeof(ac_se);
    pACbuf->opcode = CE_ACTC;
    pACbuf->adr = VME_ADR(&pRCMcxt->asl);
    *pBuf += sizeof(ac_se);
    pVme = (generic_vme *)*pBuf;
    pVme->len = sizeof(generic_vme);
    pVme->opcode = CE_LDSV;
    pVme->adr = VME_ADR(pRCMcxt + 1);
    *pBuf += sizeof(generic_vme);

    if (pGemrcx->cxt_type == TRAV_RCXTYPE)
    {
      STOP_OTHR_FIFO( *pBuf, 4, FALSE);
    }

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
  }

  /* Clear GAI_GAI_PLANE in exposed rgns*/
  CLEAR_AND_SET_COLOR( *pBuf, pwg->width, pwg->height, GAI_GPM_PLANE );

  gem_drawwinreg            	    /* Draw regions into global plane    */
      (                             /* mask based on "Window clipping    */
				    /* Regions"			         */
       pDevP,
       pGPMRegion,                  /* Regions to fill                   */
       fifo_num,
       buf_start,
       pBuf,                        /* SE buffer                         */
       buf_size
       );

  if (*pBuf + 1024 > buf_start + buf_size)
    flush_buf(pDevP, fifo_num, buf_start, pBuf);

  if (fifo_num == ImmSeFifo)
  { RESTORE_REGS		    /* Restore adapter registers &       */
      (				    /* select base planes                */
       *pBuf
       );
  }
  else
  { /*
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
     * Reset frame buffer mask back to window planes
     */
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_FMSK;
    pSe->data = GAI_WIN_WPMASK;
    *pBuf += sizeof(generic_se);

    /*
     * Make sure window compare value is set to private value
     */
    pFcmp = (frame_buf_cmp *)*pBuf;
    pFcmp->len = sizeof(frame_buf_cmp);
    pFcmp->opcode = CE_FBC;
    pFcmp->flags = 0;
    pFcmp->mask = 0;
    pFcmp->value = pDevP->priv_hwid;
    *pBuf += sizeof(frame_buf_cmp);

    if (pGemrcx->cxt_type == TRAV_RCXTYPE)
    {
      START_OTHR_FIFO( *pBuf, 4 );
    }

    /*
     * Reactivate traversal context
     */
    len = activate_cxt(gdp, pRcx, *pBuf);
    *pBuf += len;
    len = load_dsv(gdp, pRcx, *pBuf);
    *pBuf += len;
  }

  pGemrcx->win_comp_mask |= GAI_GPM_PLANE;
  pGemrcx->win_comp_val  |= GAI_GPM_PLANE;

#ifdef GEM_DBUG
    printf("Setting window compare mask = 0x%x\n", pGemrcx->win_comp_mask);
    printf("Setting window compare value = 0x%x\n", pGemrcx->win_comp_val);
#endif

  SET_BIT                           /* Use Global Plane Mask for future  */
    (                               /* rendering operations              */
     *pBuf,
     GAI_GPM_PLANE
     );


  return;

}       


#ifdef NOTREADY
/************************************************************************/
/*  FUNCTION NAME: gem_bltwinmask					*/
/*  									*/
/*  INPUT PARAMETERS:  							*/
/*  									*/
/*      pWA       - Current window attr					*/
/*      pGPMMask  - Pixmap to use for masking				*/
/*  	planes    - planes to blt bitmap to				*/
/*  									*/
/*  DESCRIPTION:  							*/
/*  									*/
/*   Blt pixmap to window planes					*/
/*     d) create blt pixmap			   			*/
/*     e) set REPLACE logical-op for writing			   	*/
/*     f) blt pix map to window plane(s) selected			*/
/*  									*/
/************************************************************************/

 void gem_bltwinmask(pWA, pGPMMask, planes, pDevP)
struct _rcmWA 	*pWA;
gPixmapPtr	pGPMMask;
ulong		planes;
rGemRCMPrivPtr	pDevP;
{
  save_buf	Wbuf;
  generic_se	Fbuf;
  bltPixmapRec 	BP;	                 	/* internal str for BLTing*/
  int		width;		         	/* pixmap width 	     */
  int		height;		         	/* pixmap height 	     */
  int		x;		         	/* blt origin    	     */
  int		y;		         	/* bly origin    	     */
  int		rc;
  int		h;
  int		w;
  char		*pBuf;
  char		*pBuf_start;
  ulong		buflen;
	
  pBuf       = (char *)(&(pDevP->se_buffer)); 	 /* ptr to SE tmp buffer     */
  pBuf_start = pBuf;
	
  /***************************************************************************/
  /* height and width is that of the pixmap		      		     */
  /***************************************************************************/
     w = pGPMMask->width;
     h = pGPMMask->height;

  /***************************************************************************/
  /* origin is that of the window's Mask origin			             */
  /***************************************************************************/
     x = pWA->maskOrg.x;
     y = pWA->maskOrg.y;

  /***************************************************************************/
  /* make blt pixmap                                                         */
  /***************************************************************************/
     rc = gem_mkBP(&BP, pGPMMask, pWA, 0, 0, w, h, x, y, planes);


  /***************************************************************************/
  /* Blt with replace Logical-operation                                      */
  /***************************************************************************/
     if(!rc) {
       BP.alu = GM_REPLACE;
       BP.fg_color = 0xff;
       BP.bg_color = 0x00;
       gem_ma(&BP);                                           /* Perform BLT*/
     }
}
#endif NOTREADY

/************************************************************************/
/*                                                                      */
/*  FUNCTION NAME: gem_drawwinreg                                       */
/*                                                                      */
/*  DESCRIPTION: This function draws clipping regions relative          */
/*      to the window origin.                                           */
/*                                                                      */
/*  INVOCATION:                                                         */
/*                                                                      */
/*      gem_drawwinreg ( pRegions, ppBuf )		                */
/*      gRegionPtr    pRegions;   clipping relative to ul screen        */
/*	char	      **ppBuf;	  pointer to pointer to SE buffer	*/
/*                                                                      */
/*  INPUT PARAMETERS:                                                   */
/*                                                                      */
/*  OUTPUT PARAMETERS:                                                  */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*  RETURN VALUES:                                                      */
/*                                                                      */
/*      void                                                            */
/*                                                                      */
/************************************************************************/
void gem_drawwinreg( pDevP, pRegions, fifo_num, buf_start, pBuf, buf_size)
rGemRCMPrivPtr	pDevP;
gRegionPtr	pRegions;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{
	gBox 		*boxptr;
	int		len;
	int		i;
	int		n;
	int		lim;
	int		x;
	int		y;
 
	rectangle 	*boxes;


	n = (pRegions->numBoxes);	    /* number of boxes to render     */
        if ( n == 0 ) return;               /* do nothing on zero boxes      */

	len = sizeof(rectangle);    	    /* number of bytes in SE's       */
 
	boxptr = (pRegions->pBox); 	    /* source box ul-lr              */

	lim = (buf_start + buf_size - *pBuf) / len;
	if (lim > n)
	  lim = n;

	while (n > 0)
	{ for ( i=0; i<lim; i++ )
	  { boxes = (rectangle *)(*pBuf);   /* alloc space for SE's to draw*/
	 			            /* a rectangle                 */

	    boxes->len    = len;            /* total bytes                 */
	    boxes->opcode = CE_IPLG;        /* integer polygon             */
	    boxes->flags  = CONVEX;         /* convex polygon              */
	    boxes->length = 0x18;           /* number of bytes in polygon  */

	    /***************************************************************/
	    /* form fill polygon Structure Element                         */
	    /*         rectangle coordinate pair order                     */
	    /*     			0  3				   */
	    /*     			1  2				   */
	    /* The lower-right corner of each box is actually 1 greater in */
	    /* the x and y direction than the region that should be clipped*/
	    /***************************************************************/
	    boxes->pt[0].x = boxptr->ul.x;           /* upper left corner  */
	    boxes->pt[0].y = boxptr->ul.y;           /* upper left corner  */
	    boxes->pt[1].x = boxptr->ul.x;           /* lower left corner  */
	    boxes->pt[1].y = boxptr->lr.y - 1;       /* lower left corner  */
	    boxes->pt[2].x = boxptr->lr.x - 1;       /* lower right corner */
	    boxes->pt[2].y = boxptr->lr.y - 1;       /* lower right corner */
	    boxes->pt[3].x = boxptr->lr.x - 1;       /* upper right corner */
	    boxes->pt[3].y = boxptr->ul.y;           /* upper right corner */
	    boxes->pt[4].x = boxptr->ul.x;           /* close polygon      */
	    boxes->pt[4].y = boxptr->ul.y;           /* close polygon      */

	    /***************************************************************/
	    /* move pointer to next source box                             */
	    /***************************************************************/
	    boxptr++;

	    /***************************************************************/
	    /* move pointer to next destination                            */
	    /***************************************************************/
	    *pBuf += len;	       /* pBuf point to next available spot*/
	  }

	  n -= lim;

	  if (n > 0)
	  { flush_buf(pDevP, fifo_num, buf_start, pBuf);
	    lim = buf_size / len;
	    if (lim > n)
	      lim = n;
	  }
	}

        return;
}
