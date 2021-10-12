static char sccsid[] = "@(#)84	1.5.1.17  src/bos/kernext/disp/gem/rcm/gem_cwg.c, sysxdispgem, bos411, 9428A410j 5/28/93 15:08:54";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_create_win_geom
 *		iggm_delete_win_geom
 *		protect_wg
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
 *;CL   08/29/89   Created                                            @#
 *;MC   10/24/89   Initialized hwid field and added call to free_hwid @1
 *;MC   10/31/89   fixed call to free_hwid to pass gdp		      @2  
 *;CL   11/01/89   Fixed prolog
 *;MC   01/25/90   Added ctid initialization			      @3
 */

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"
#include "gem_geom.h"
#include "rcm_mac.h"

typedef struct _sgca {
    ushort		len;
    ushort		opcode;
    ulong		offset;
    ulong		data;
  } sgca;

iggm_create_win_geom (gdp, wgp, arg)
struct _gscDev  *gdp ;
struct _rcmWG   *wgp ;
create_win_geom *arg ;

{
  rWGPrivPtr	pwgp;

#ifdef GEM_DBUG
  printf("iggm_create_win_geom\n");
#endif

  /* allocate space for device specific private data area  */
  wgp->pPriv = (genericPtr)rMalloc(sizeof(rWGPriv));
  pwgp = (rWGPrivPtr)wgp->pPriv;
  pwgp->gWGChangeMask = -1;

  /*
   * Initialize hardware ID field to indicate   @1
   * that it needs a new hardware ID
   */
  pwgp->hwid = PROTECT_HWID - NUM_HWIDS;

  /*
   * Initialize the hwid sync counters to 0
   */
  pwgp->imm_sync_cntr = 0;
  pwgp->trv_sync_cntr = 0;

  /*
   * Initialize window as not using Z Buffer - this could be changed in bind
   */
  pwgp->zBufferWind = FALSE;

  return(0) ;
}


iggm_delete_win_geom(gdp, pwg)
struct _gscDev       *gdp ;
struct _rcmWG        *pwg;

{
  rGemRCMPrivPtr pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  struct _sgca {
      ushort		len;
      ushort		opcode;
      ulong		offset;
      ulong		data;
    } *pSgca;
  char			se_buffer[8192];
  char			*pBuf = se_buffer;
  TRAV_CONTEXT		*pRCMcxt;
  rWGPrivPtr		pwgpriv;
  int			len;
  ac_se			*pACbuf;
  generic_vme		*pVme;
  ulong			seg_reg;
  int                   cur_proc_id, guarded;
  rcmProcPtr            cur_proc;

  pwgpriv = (rWGPrivPtr)pwg->pPriv;

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

  GMBASE_INIT(seg_reg, pDevP);
  while ( *TRV_SYNC_CNTR < pwgpriv->trv_sync_cntr )
  ;

  BUSMEM_DET(seg_reg);
  
  /*
   * call routine to indicate that this ID is now available
   */
  if (pwgpriv->hwid >= 0)
    free_hwid(gdp, pwgpriv->hwid);

  /*
   * If current rcx on immediate domain is null, we assume that that is
   * because X is going away, and so clearing the overlay planes is a
   * moot point.  Besides which context would we reactivate?
   */
  if (pwg->wg.pClip != NULL && gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur)
  { pRCMcxt = (TRAV_CONTEXT *)pDevP->pPrivCxt;
    len = store_dsv(gdp, gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur, pBuf);
    pBuf += len;
    pACbuf = (ac_se *)pBuf;
    pACbuf->len = sizeof(ac_se);
    pACbuf->opcode = CE_ACTC;
    pACbuf->adr = VME_ADR(&pRCMcxt->asl);
    pBuf += sizeof(ac_se);
    pVme = (generic_vme *)pBuf;
    pVme->len = sizeof(generic_vme);
    pVme->opcode = CE_LDSV;
    pVme->adr = VME_ADR(pRCMcxt + 1);
    pBuf += sizeof(generic_vme);

    gem_clear_overlay(pDevP, pwg, ImmSeFifo, se_buffer, &pBuf, 8192);

    len = activate_cxt(gdp, gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur, pBuf);
    pBuf += len;
    len = load_dsv(gdp, gdp->domain[IMMEDIATE_CREG_DOMAIN].pCur, pBuf);
    pBuf += len;
  }
  
  if (pwgpriv->zBufferWind)
    if (--pDevP->num_zbuf_wind == 1)
    { pSgca = (struct _sgca *)pBuf;
      pSgca->len = sizeof(struct _sgca);
      pSgca->opcode = CE_SGCA;
      pSgca->offset = 0x90;
      pSgca->data = 0;
      pBuf += sizeof(struct _sgca); 
      /* Passing NULL as the clipping region is a convention which
       * means to use the full screen as a clipping region; that is,
       * clear the zbuffer for the entire screen when there's only one
       * zbuffer process left.
       */
      gem_protect_zbuffer(gdp,0,ImmSeFifo,
                          se_buffer,&pBuf,1024); 
    }

  /* Free up device specific private data area */
  rFree(pwg->pPriv) ;
  pwg->pPriv = NULL;

  if (pBuf - se_buffer)
  {
    seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

    WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );

    BUSMEM_DET(seg_reg);
  }

  return(0) ;

}               

protect_wg(gdp, pwg)
struct _gscDev       *gdp ;
struct _rcmWG        *pwg;

{
  rcxPtr	pCur;
  char			se_buffer[32];
  ulong			seg_reg;
  char			*pBuf;
  rGemRCMPrivPtr	pDevP;
  frame_buf_cmp		*pFcmp;

  /*
   * If a window is unobscured (windowing is off), it may have stuff in the
   * 3D fifo that has yet to be processed.  If so, the X-server can unmap
   * this window before it does get processed.  Since windowing was off
   * the stuff will be rendered and leave stuff on the screen.
   * In order to prevent this, this routine will force windowing to be 
   * turned on for the 3D fifo.  This is done by setting the 3D fifo's
   * compare mask to non-zero.  Care is taken to allow for the possiblity
   * that there isn't stuff left over.  This is done by checking if there
   * is another context on the 3D domain, if so, then mask must incorporate
   * the new context's use of the client clip plane.
   */
  pBuf = se_buffer;
  pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);

  pFcmp = (frame_buf_cmp *)pBuf;
  pFcmp->len = sizeof(frame_buf_cmp);
  pFcmp->opcode = CE_FBC;
  pFcmp->flags = 0x8 | 0x2 | 0x1; /* Win planes | Ignore Val | Othr FIFO */
  pBuf += sizeof(frame_buf_cmp);

  /*
   * If there is an rcx on the 3D domain AND it is not a null rcx 
   * then use compare mask of that window
   */
  pCur = gdp->domain[TRAVERSAL_FIFO_DOMAIN].pCur;
  if( pCur != NULL &&  !(pCur->flags & RCX_NULL))
    pFcmp->mask = (((rGemrcxPtr)(pCur->pData))->win_comp_mask &
		   GAI_GPM_PLANE) |  GAI_WIN_PLANES;
  else 
      pFcmp->mask = GAI_WIN_PLANES;
  

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  BUSMEM_DET(seg_reg);
}
