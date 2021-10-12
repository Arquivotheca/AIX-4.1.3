static char sccsid[] = "@(#)90	1.8.3.6  src/bos/kernext/disp/gem/rcm/gem_geom.c, sysxdispgem, bos411, 9428A410j 9/1/93 12:58:19";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_update_win_geom
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
 *;LW   08/25/89   Created                                            @#
 *;JG   09/01/89   added undef TRACE, trace.h                         @1
 *;LW   09/12/89   remove setting of hwid from wg_handle
 *;lw   09/21/89   REDBYTE changes
 *;LW   10/12/89   Added pWG to gem_upd_geom routine
 *;LW   10/25/89   Made se_buffer local
 *;		   Changed where hwid is gotten from
 *;LW   10/26/89   Removed trace.h
 *;CL   11/01/89   Fixed prolog
 *;MC   11/03/89   Added gem_gai.h include
 *;MC	11/21/89   Added hwid to gem_upd_geom
 *;MC	11/29/89   Changed KERNEL to _KERNEL and removed KGNHAK
 *;MC   12/01/89   Modified for traversal and new ucode changes	      @2
 *;LW   12/13/89   Restructured hfiles	
 *;LW   01/22/90   Always do set win org and set compare value appropriately
 *;MC   01/25/90   Added support for color table id
 *;MC   02/13/90   Fixed reference of floating point value	      @3
 *;MC   02/19/90   Fixed obscurity test				      @4
 *;LW   02/27/90   Add rp to insert_imm interface
 *;LW   08/10/90   Add test of old_wg before doing undraw in upd_geom
 */


#include <sys/sleep.h>
#include "gemincl.h"
#include "gemrincl.h"
#include "gem_gai.h"
#include "gem_geom.h"
#include "rcm_mac.h"

/*
 * FUNCTION:  iggm_update_win_geom                                           
 *                                                                           
 * DESCRIPTION:                                                              
 */
iggm_update_win_geom(gdp, rp, newgp, changemask)
     struct _gscDev      *gdp;
     struct _rcx         *rp;
     struct _rcmWG       *newgp;
     int                 changemask;
{
  int			rc;
  char          	*pBuf;
  rGemRCMPrivPtr	pDevP;
  ulong			seg_reg;
  rGemrcxPtr		pGemrcx;
  rWGPrivPtr		pwgpriv;
  rWAPrivPtr		pwapriv;
  ushort 		RCMChgMask;
  struct _rcmWG		*real_wg;
  struct _rcmWA		*real_wa;
  int			*p_chg_mask;
  char    		se_buffer[8192];	/* private SE buffer	*/
  
#ifdef TRACKCTID
  if (changemask & 0x40){
    printf("UWG: ");
    printf("rp=%x chmsk=%x ", rp, changemask);}
#endif

#ifdef MEM_TRASH
  if ( pDevP->old_zbuffer->numBoxes > 3 )
     printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer->numBoxes);
  printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer);
#endif
  
  pDevP = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);



  /*
   * If rp is not NULL, then the actual window geometry being updated
   * can be obtained from rp->pWG; otherwise we need to do a FIND_WG
   * using the wg_handle copied in from the user's geometry attributes.
   */
  if (rp)
  { pGemrcx = (rGemrcxPtr)rp->pData;
    real_wa = rp->pWA;
    real_wg = rp->pWG;
  }
  else
  { pGemrcx = NULL;
    real_wa = NULL;

    FIND_WG(gdp, newgp->wg.wg_handle, real_wg);
    if (real_wg == NULL) {
      /*
       * Couldn't find real window geometry for some reason
       */
	pBuf = se_buffer;
	THAW_3D(pBuf);
	WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
	return(GM_NO_REAL);;
    }

  }

#ifdef MEM_TRASH
  if ( pDevP->old_zbuffer->numBoxes > 3 )
     printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer->numBoxes);
  printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer);
#endif  
  
  /*
   * Guard the domain to block context switch
   */
  GM_GUARD(&(gdp->domain[0]), gdp->domain[0].pCurProc);
  
  /*
   * Initialize local vars
   */
  pwgpriv = (rWGPrivPtr)real_wg->pPriv;
  
  /*
   * Setup Structure Element Buffer                                        
   */
  pBuf = se_buffer;			/* ptr to SE tmp buffer     */
  RCMChgMask = rCxtIsActive | rImmFifo;
  
  if (pGemrcx)
    p_chg_mask = (int *)&pGemrcx->gWGChangeMask;
  else
    p_chg_mask = (int *)&pwgpriv->gWGChangeMask;
  *p_chg_mask |= changemask;


  GM_LOCK_LDAT(pDevP);

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

  GMBASE_INIT(seg_reg, pDevP);

  FREEZE_3D(pDevP);      /* 3D fifo needs to be frozen before X updates
                            the geometry of a window */

  BUSMEM_DET(seg_reg);

#ifdef MEM_TRASH
  if ( pDevP->old_zbuffer->numBoxes > 3 )
     printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer->numBoxes);
  printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer);
#endif

  if (rp && *p_chg_mask & gCWclip && rp == rp->pDomain->pCur &&
      pwgpriv->hwid < 0)
  { 
#ifdef GEM_DBUG
    printf("%s %d Need new hardware ID: getting ",__FILE__,__LINE__);
#endif
    RCMChgMask |= (pwgpriv->hwid + NUM_HWIDS) << 8;
    if (get_hwid(gdp, rp, real_wg, pBuf))
    {
      /* Set rcm flag to indicate that we need to steal the ID */
      RCMChgMask |= rStealHwid;
    }
    RCMChgMask |= rNewHwid;

#ifdef GEM_DBUG
    printf("0x%x as new ID\n",pwgpriv->hwid);
#endif
  }

#ifdef MEM_TRASH
  if ( pDevP->old_zbuffer->numBoxes > 3 )
     printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer->numBoxes);
  printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer);
#endif

#ifdef  GEOM_DBUG
  printf("Doing update_win_geom Now: chgmask=0x%x\n", *p_chg_mask);
#endif 

  rc = upd_geom(gdp, rp, newgp,
		real_wa,
		real_wg,
		p_chg_mask,
		RCMChgMask,
		ImmSeFifo,
		se_buffer,
		&pBuf,
		8192);

#ifdef MEM_TRASH
  if ( pDevP->old_zbuffer->numBoxes > 3 )
     printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer->numBoxes);
  printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer);
#endif
  
  THAW_3D(pBuf);
  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  WTFIFO( ImmSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
  BUSMEM_DET(seg_reg);

  GM_UNLOCK_LDAT(pDevP);

  /*
   * Unguard the domain to allow context switch
   */
  
  GM_UNGUARD(&(gdp->domain[0]));

#ifdef MEM_TRASH
  if ( pDevP->old_zbuffer->numBoxes > 3 )
     printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer->numBoxes);
  printf("iggm_update_win_geom: 0x%x\n", pDevP->old_zbuffer);
#endif
  
#ifdef GEM_DBUG
  HERE_I_AM;
#endif
  
  return(0);   /* no value returned */
}


