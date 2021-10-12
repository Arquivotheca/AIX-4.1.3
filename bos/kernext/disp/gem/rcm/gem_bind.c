static char sccsid[] = "@(#)81	1.5.3.8  src/bos/kernext/disp/gem/rcm/gem_bind.c, sysxdispgem, bos41J, 9521B_all 5/25/95 15:26:34";

/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: iggm_bind_window
 *		reset_sync_cntrs
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
 *;MC   11/22/89   Created                                           
 *;LW   11/27/89   Added Trace stuff                                   
 *;LW   12/14/89   Added support for "unbind" capability (null pwg,pwa)
 *;LW   01/18/90   Change to 8951 scheme for more discriminating upd_geom
 *;MC   01/25/90   Added color table if parameter to upd_geom
 *;LW   02/19/90   Changed clearing of geometry & window changemasks
 *;LW   02/27/90   Add rp to insert_imm interface
 *;LW   03/13/90   Removed 8943 code
 *;LWMC 04/24/90   Made changes for new update geometry
 *;NGK  06/15/93   Avoid hang in get_hwid...  NGK 00
 *;									
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gem_gai.h"
#include "gem_geom.h"
#include <rcm_mac.h>

Bool get_hwid();
void make_hwid_head();
void reset_sync_cntrs();

/*
 * NB: There are areas in our code which rely on the fact that when
 * the HFT calls our bind_window with a new window geometry and/or window
 * attributes, that the pointers to the geometry/attributes in the rcx
 * structure still point to the OLD geometry/attributes.  If this
 * changes in the future for any reason, it will cause serious problems
 * for us.
 */

int iggm_bind_window(gdp, rp, pwg, pwa)
struct _gscDev	*gdp;
struct _rcx	*rp;
struct _rcmWG	*pwg;
struct _rcmWA	*pwa;
{
  char			se_buffer[8192];
  int			buf_len, i, j;
  int			rc = 0;
  char			*pBuf, *buf_start;
  rGemRCMPrivPtr	pDevP;
  rWAPrivPtr		pwapriv;
  rWGPrivPtr		pwgpriv;
  rWAPrivPtr		tmppwapriv;
  rWGPrivPtr		tmppwgpriv;
  rGemrcxPtr		pGemrcx;
  disp_buf_sync		*pDBS;
  ulong			seg_reg;
  ulong			GeoMask;
  ushort 		RCMChgMask;
  ulong			WinMask;
  int			fifo_num;
  struct _sgca {
      ushort		len;
      ushort		opcode;
      ulong		offset;
      ulong		data;
    } *pSgca;
  rcxPtr                protectRcx;

  rcmProcPtr  cur_proc;

  /*  Sanity check  */
  if ((rp == NULL) || (gdp == NULL))
	return(-1);

  /*
   * If rcx is current on domain, then permission to the domain will
   * not be removed, and therefore no graphics fault will occur - therefore
   * we must do any deferred updates to the new window attributes and/or
   * geometries
   */
  
  /*
   * Guard the domain to block context switch
   */
  GM_GUARD(rp->pDomain, rp->pProc);
  pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  pBuf = buf_start = se_buffer;
  STOP_OTHR_FIFO(pBuf,115,FALSE);
  pGemrcx = (rGemrcxPtr)(rp->pData);
  GM_LOCK_LDAT(pDevP);

  if ( pwg )
    pwgpriv = (rWGPrivPtr)pwg->pPriv;
  else
    pwgpriv = NULL;

  if ( pwa )
  {
     if ( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA )
     {
#ifdef CHECK_CLIENT_CLIP
        printf("bind window problem\n");
#endif
        rFree( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA->pRegion->pBox );
        rFree( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA->pRegion );
        rFree( ((rWAPrivPtr)pwa->pPriv)->ClientClipWA );
        ((rWAPrivPtr)pwa->pPriv)->ClientClipWA = NULL;
     }

     pwapriv = (rWAPrivPtr)pwa->pPriv;
  }
  else /* if ( pwa ) */
     pwapriv = NULL;


  /*************************************************************************/
  /* RJE:  Additional checking has been added to the next two if blocks to */
  /*       prevent the GTO device driver from a DSI, particularly during   */
  /*       adapter bringup.                                                */
  /*************************************************************************/

  if (((rWGPrivPtr)rp->pWG) &&
          (((rWGPrivPtr)rp->pWG->pPriv)->hwid >= 0) &&
          (((rWGPrivPtr)rp->pWG->pPriv)->hwid < 64)) {
    if ((gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur==rp) ||
             (gdp->domain[TRAVERSAL_FIFO_DOMAIN].pCur==rp))
        pDevP->hwid[((rWGPrivPtr)rp->pWG->pPriv)->hwid].currentlyUsed=0;
    else {
        FIND_GP(gdp, cur_proc);
        if ((cur_proc->procHead.pid) == (rp->pProc->procHead.pid))
           pDevP->hwid[((rWGPrivPtr)rp->pWG->pPriv)->hwid].currentlyUsed=0;
    }
  }


  if (pGemrcx->cxt_type == IMM_RCXTYPE)
    fifo_num = ImmSeFifo;
  else
    fifo_num = TravSeFifo;

  if (pwg != NULL && pwa != NULL && !pwgpriv->zBufferWind &&
      pGemrcx->cxt_type == TRAV_RCXTYPE)
  { for (i=0; i<pwapriv->num_groups; ++i)
      if (pwapriv->groups_used[i].type == gZBufferGroup)
      { pwgpriv->zBufferWind = TRUE;
	if (++pDevP->num_zbuf_wind == 2)
	{ pSgca = (struct _sgca *)pBuf;
	  pSgca->len = sizeof(struct _sgca);
	  pSgca->opcode = CE_SGCA;
	  pSgca->offset = 0x90;
	  pSgca->data = 1;
	  pBuf += sizeof(struct _sgca);
          protectRcx=gdp->domain[TRAVERSAL_FIFO_DOMAIN].pCur;
          gem_protect_zbuffer(gdp,protectRcx->pWG->wg.pClip,fifo_num,
            se_buffer,&pBuf,1024); 
	}
      }
  }

  if (pwg != NULL)
  { /*
     * Combine geometry masks into Rcx
     */
    pGemrcx->gWGChangeMask |= pwgpriv->gWGChangeMask;
    pwgpriv->gWGChangeMask = 0;
  }

  if (pwa != NULL)
  { /*
     * Combine attribute masks into Rcx
     */
    WinMask = pGemrcx->gWAChangeMask |= pwapriv->gWAChangeMask;
    pwapriv->gWAChangeMask = 0;
  }
      
  if (rp == rp->pDomain->pCur)
  { 
    /*
     * If the process is through using this geometry, then we want to set
     * the sync counters so that we will know when all of the SEs related
     * to this geometry have finished executing
     */
    if (pwg != rp->pWG) { 
      if (pGemrcx->cxt_type == IMM_RCXTYPE) {
	if (pDevP->imm_sync_cntr >= MAX_SYNC_CNTR)
	  reset_sync_cntrs(gdp);
	else
	{ tmppwgpriv = (rWGPrivPtr)rp->pWG->pPriv;
	  tmppwgpriv->imm_sync_cntr = ++pDevP->imm_sync_cntr;
	  pDBS = (disp_buf_sync *)pBuf;
	  pDBS->len = sizeof(disp_buf_sync);
	  pDBS->opcode = CE_DBS;
	  pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	  pDBS->ofst = DISP_BUF_OFFSET;
	  pDBS->flags = 0x1;
	  pDBS->imm_cntr = pDevP->imm_sync_cntr;
	  pBuf += sizeof(disp_buf_sync);
          seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
          WTFIFO(fifo_num,buf_start,pBuf-buf_start,seg_reg,pDevP);
          BUSMEM_DET(seg_reg);
          pBuf=buf_start;
	}
      } else {
	if (pDevP->trv_sync_cntr >= MAX_SYNC_CNTR)
	  reset_sync_cntrs(gdp);
	else
	{ tmppwgpriv = (rWGPrivPtr)rp->pWG->pPriv;
	  tmppwgpriv->trv_sync_cntr = ++pDevP->trv_sync_cntr;
	  pDBS = (disp_buf_sync *)pBuf;
	  pDBS->len = sizeof(disp_buf_sync);
	  pDBS->opcode = CE_DBS;
	  pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	  pDBS->ofst = DISP_BUF_OFFSET;
	  pDBS->flags = 0x2;
	  pDBS->trv_cntr = pDevP->trv_sync_cntr;
	  pBuf += sizeof(disp_buf_sync);
          seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
          WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
          BUSMEM_DET(seg_reg);
          pBuf=buf_start;
	}
    }
  }
    if (
	pwg != NULL &&
	pwa != NULL
	)
    {
#ifdef GEM_DBUG2
      printf(" cm_handle 0x%x rcx:cm_handle=0x%x\n",
	     ((rcmCmPtr)pwg->wg.cm_handle),rp->pWG->wg.cm_handle);
#endif
      /*
       * Initialize local variables
       */
      RCMChgMask = 0;

      if (pGemrcx->cxt_type == IMM_RCXTYPE)
	RCMChgMask |= rImmFifo;
      else
      { RCMChgMask |= rTravFifo;
	if (pwg != rp->pWG)
	  RCMChgMask |= rZBuf;
      }

      /*
       * Make sure we have a valid hardware ID                            
       * Optimization: If the clipping region in the window geometry isn't
       * changing, and the window is unobscured, don't bother getting a
       * valid window id.
       */
#ifdef BIND_OPTIMIZE
      if (pGemrcx->gWGChangeMask & gCWclip || !iggm_unobscured(&pwg->wg))
      { 
#endif BIND_OPTIMIZE
	if (pwgpriv->hwid < 0)
	{ 
#ifdef GEM_DBUG
	  printf("%s %d Need new hardware ID: getting ",__FILE__,__LINE__);
#endif
	  RCMChgMask |= (pwgpriv->hwid + NUM_HWIDS) << 8;
	  if (get_hwid(gdp, rp, pwg, pBuf))
	  {
	    /* Set rcm flag to indicate that we need to steal the ID */
	    RCMChgMask |= rStealHwid;
	  }
	  RCMChgMask |= rNewHwid;

#ifdef GEM_DBUG
	  printf("0x%x as new ID\n",pwgpriv->hwid);
#endif
	}
        else
	{
#ifdef GEM_DBUG
	  printf("*Current hardware ID is 0x%x pClip=0x%x\n",
		 pwgpriv->hwid, pwg->wg.pClip);
#endif
	  make_hwid_head(pDevP, pwgpriv->hwid);
          pDevP->hwid[pwgpriv->hwid].currentlyUsed=1;
	}
#ifdef BIND_OPTIMIZE
      }
#endif BIND_OPTIMIZE
      /*
       * Process deferred update geometry 
       */
#ifdef GEM_DBUG
      printf("Doing defered upd_geo from bind_window rp=0x%x pwg=0x%x GeoMask=0x%x\n",
	     rp, pwg, pGemrcx->gWGChangeMask);
#endif
      rc = upd_geom(gdp, rp, pwg, pwa,
		    pwg,
		    &pGemrcx->gWGChangeMask,
		    RCMChgMask|rCxtIsActive|rSetWinOrgSize,
		    fifo_num, buf_start, &pBuf, 8192);
      if (rc < 0) {
		pBuf = buf_start;
		goto leave;
      }
      
#ifdef GEM_DBUG
      printf("%s %d\n",__FILE__,__LINE__);
#endif
      
      /*
       * Process deferred window updates 
       */
#ifdef GEM_DBUG
      printf("Doing defered upd_win from bind_window rp=0x%x pwa=0x%x WinMask=0x%x\n",
	     rp, pwa, WinMask);
#endif
      /*
       * Initialize local variables
       */
      RCMChgMask = rSetWinAttrs;

      rc = gem_upd_win(gdp, rp, pwa, pwg, WinMask, RCMChgMask,
		       fifo_num, buf_start, &pBuf, 8192);
      if (rc < 0) {
		pBuf = buf_start;
		goto leave;
      }
      pGemrcx->gWAChangeMask = 0;
    }
    
  }
leave:
  /*
   * send buffer down fifo
   */
      
#ifdef GEM_DBUG
  printf("%s %d buffer size=0x%x\n",__FILE__,__LINE__, pBuf - buf_start);
#endif
  START_OTHR_FIFO(pBuf,116);
  if (pBuf - buf_start) {
    seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
    WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
    BUSMEM_DET(seg_reg);
  }

  /*
   * Unguard the domain to block context switch
   */
  GM_UNGUARD(rp->pDomain);
  GM_UNLOCK_LDAT(pDevP);

  return(rc);
}


void reset_sync_cntrs(gdp)
struct _gscDev	*gdp;
{
  int		i;
  rcmWGPtr	pwg;
  rWGPrivPtr	pwgpriv;
  rGemRCMPrivPtr	pDevP;
  disp_buf_sync	DBS;
  ulong		seg_reg;
  rcm_wg_hash_t *pWG_hash_index;  /* use to traverse WG hash table */
  int           WG_table_index;   /* use to traverse WG hash table */

  pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);

  /*
   * Clear sync counters in all window geometry structures
   *
   *  Loop through hash table entries
   */
  for (WG_table_index = 0; WG_table_index < RCM_WG_HASH_SIZE; WG_table_index++)
  {

	pWG_hash_index = &(gdp->devHead.wg_hash_table->entry[WG_table_index]);

	pwg = ( rcmWGPtr )pWG_hash_index->pWG;

	/*
	*  Loop through linked list for a given hash entry
	*/

	while ( pwg != NULL )
	{
		pwgpriv = (rWGPrivPtr)pwg->pPriv;
		pwgpriv->imm_sync_cntr = 0;
		pwgpriv->trv_sync_cntr = 0;

		pwg = pwg->pNext;
	}

  }

  /*
   * Clear sync lock values in traversal slot structures
   */
  for (i=pDevP->first_trav_slot;
       i<pDevP->first_trav_slot + pDevP->num_trav_slots; ++i)
    pDevP->slots[i].slot_lock = 0;

  pDevP->imm_sync_cntr = 0;
  pDevP->trv_sync_cntr = 0;

  DBS.len = sizeof(disp_buf_sync);
  DBS.opcode = CE_DBS;
  DBS.adr = VME_ADR(IMM_SYNC_CNTR);
  DBS.ofst = DISP_BUF_OFFSET;
  DBS.flags = 0;	/* update both sync cntrs	*/
  DBS.imm_cntr = 0;
  DBS.trv_cntr = 0;

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

  WTFIFO(ImmSeFifo, &DBS, DBS.len, seg_reg, pDevP);

  FIFO_EMPTY(ImmSeFifo, seg_reg, pDevP);
  FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);

  BUSMEM_DET(seg_reg);
}
