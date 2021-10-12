static char sccsid[] = "@(#)86	1.5.2.10  src/bos/kernext/disp/gem/rcm/gem_ddfun.c, sysxdispgem, bos411, 9428A410j 5/18/94 10:28:20";

/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_def_groups
 *		gem_load_olay_ct
 *		gem_wake_switch
 *		iggm_ddf
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
 *;CL 09/15/89  Created                                                 
 *;CL 11/01/89  Fixed prolog
 *;LW 11/20/89  Fixed xmemat for 8943
 *;MC 11/29/89  Changed KERNEL to _KERNEL and removed KGNHAK
 *;LW 01/11/90  Changed use of xs
 *;LW 01/11/90  Copy WAPriv before freeing it
 *;                                                                     
 */

#include <sys/sleep.h>
#include "gemincl.h"
#include "gemrincl.h"
#include "gem_gai.h"
#include "gem_geom.h"
#include "rcm_mac.h"

#define gWakeSwitchPtr   ulong *

int iggm_ddf(gdp, cmd, ddi, ddi_len)
struct _gscDev   *gdp;                    /* virtual terminal structure   */
int              cmd;                     /* specific dev-dep-fun to do   */
caddr_t          ddi;                     /* ptr to struct with arguments */
int              ddi_len;                 /* len of structure             */
{
	int rc;		                  /* return code                  */



	switch( cmd ) 
        { 
	   case DEF_GROUPS:
              rc = gem_def_groups(gdp, ddi) ;    
              break ;

	   case WAKEUP_SWITCH:
	      rc = gem_wake_switch(gdp, ddi);
	      break;

	   case SAVE_STATE:
	      rc = gem_save_state(gdp, ddi);
	      break;

	   case RSTR_STATE:
	      rc = gem_restore_state(gdp, ddi);
	      break;

	    case LOAD_OLAY_CT:
	      rc = gem_load_olay_ct(gdp, ddi);
	      break;

	   default:
	      rc = GM_BAD_DDF_CMD;
	      break;
	}


	return(rc);

}



/*
 * FUNCTION NAME: gem_load_olay_ct
 *                                                                     
 * DESCRIPTION: Loads the overlay planes via the 2D or 3D fifo
 */
gem_load_olay_ct(gdp, ddi)
struct _gscDev  *gdp ;
gOlay_ColorsPtr   ddi ;
{ 
  int		i;
  ulong         seg_reg;
  rcmProcPtr	pproc;
  rGemprocPtr	 pProcP;
  rcxPtr	pRcx2D, pRcx3D;
  rGemDataPtr	 pgd 	= (rGemDataPtr)(gdp->devHead.vttld);
  rGemRCMPrivPtr pDevP  = &pgd->GemRCMPriv;
  int		firstindex,lastindex;

  typedef struct lopct_se {
    ushort len;
    ushort opcode;
    ulong flags;
    ulong adr;
  } lopct_se;
  lopct_se  	*pLopct;
  
  typedef	struct sgm_bufr_struct {
    generic_vme  sgm;
    ulong     colors[16];
  } opct_sgm;
  opct_sgm	*pSgm;

  char		*pOut, *bufstart;
  uint str_elem_buff[24], numcolors;
  
#ifdef GEM_DBUG
  printf("Enter gem_load_olay_ct\n");
#endif
  
  bufstart = pOut = (char *)str_elem_buff;

  firstindex = ddi->firstindex;
  lastindex = ddi->lastindex;

  FIND_GP(gdp,pproc);
  
  pProcP = (rGemprocPtr)pproc->procHead.pPriv;
  pRcx2D = pproc->pDomainCur[IMMEDIATE_CREG_DOMAIN];
  pRcx3D = pproc->pDomainCur[TRAVERSAL_FIFO_DOMAIN];
  
    numcolors=ddi->lastindex-ddi->firstindex+1;
  /*
   * Test for 2D client
   */
  if( pRcx2D != NULL && ((rGemrcxPtr)pRcx2D->pData) != NULL)
    {
      /*
       * Set overlay planes color table                      
       */
      pSgm = (opct_sgm *)pOut;
      pSgm->sgm.len = sizeof(generic_se) + numcolors*4;
      pSgm->sgm.opcode = CE_GMEM;
      pSgm->sgm.adr =
	VME_ADR(&(((GM_MMAP *)pDevP->gmbase)->olay_ct_2D[firstindex]));
      for(i=0; i<numcolors; i++)
	pSgm->colors[i]  = ddi->colors[i];

      pOut += pSgm->sgm.len ;
      
      pLopct = (lopct_se *)pOut;
      pLopct->len = sizeof(lopct_se);
      pLopct->opcode = CE_LOPC;
      pLopct->flags = CT_LOAD_ASYNC;
      pLopct->adr = VME_ADR(&(((GM_MMAP *)pDevP->gmbase)->olay_ct_2D[0]));
      pOut += sizeof(lopct_se);

      seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
      WTFIFO(ImmFIFO, str_elem_buff, pOut - bufstart, seg_reg, pDevP );
      BUSMEM_DET(seg_reg);
      
    }

  /*
   * Test for 3D client
   */
  if( pRcx3D != NULL && ((rGemrcxPtr)pRcx3D->pData) != NULL)
    {
      pSgm = (opct_sgm *)pOut;
      pSgm->sgm.len = sizeof(generic_vme) + numcolors*4;
      pSgm->sgm.opcode = CE_GMEM;
      pSgm->sgm.adr =
	VME_ADR(&(((GM_MMAP *)pDevP->gmbase)->olay_ct_3D[firstindex]));
      for(i=0; i<numcolors; i++)
	pSgm->colors[i]  = ddi->colors[i];

      pOut += pSgm->sgm.len ;
      
      pLopct = (lopct_se *)pOut;
      pLopct->len = sizeof(lopct_se);
      pLopct->opcode = CE_LOPC;
      pLopct->flags = CT_LOAD_ASYNC;
      pLopct->adr = VME_ADR(&(((GM_MMAP *)pDevP->gmbase)->olay_ct_3D[0]));
      pOut += sizeof(lopct_se);

      seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
      WTFIFO(ImmFIFO, str_elem_buff, pOut - bufstart, seg_reg, pDevP );
      BUSMEM_DET(seg_reg);
    }

#ifdef GEM_DBUG
printf("Exit gem_load_olay_ct\n");
#endif

  return(0);

}

/*
 * FUNCTION NAME: gem_def_groups                                       
 *                                                                     
 * DESCRIPTION: Notifies the RCM which groups are being used by a
 *		particular window
 */

gem_def_groups(gdp, ddi)
struct _gscDev  *gdp ;
gDef_GroupPtr   ddi ;
{ 
  rGemDataPtr	 pgd 	= (rGemDataPtr)(gdp->devHead.vttld);
  rGemRCMPrivPtr pDevP  = &pgd->GemRCMPriv;
  rcmWAPtr       pWA;
  rWAPrivPtr     pWAPriv;
  rWGPrivPtr     pWGPriv;
  int            num, size , i;
  ulong          seg_reg;
  rcxPtr	 pRcx;
  rcxPtr         protectRcx;
  char		 se_buffer[1024];
  char		 *pBuf = se_buffer;
  long		 save_bus;
  struct _sgca {
      ushort		len;
      ushort		opcode;
      ulong		offset;
      ulong		data;
    } *pSgca;

#ifdef GEM_DBUG
printf("Enter gem_def_groups\n");
#endif

  pWA = (rcmWAPtr)ddi->wa;

  num = ddi->num_groups;
  
  size = (sizeof(rWAPriv)) + 
         (sizeof(*ddi->groups_used) * num );

  pWAPriv =(rWAPrivPtr) rMalloc(size) ; 
  
  pWAPriv->active_group = ddi->active_group ;
  pWAPriv->num_groups = ddi->num_groups ;
  pWAPriv->groups_used = ((rDefGroupsPtr)(pWAPriv + 1));

#ifdef GEM_DBUG
printf("size=%d, sizeof(gDef_Group)=%d,sizeof(*ddi->groups_used)=%d, sizeof(rWAPriv)=%d\n",
       size, sizeof(gDef_Groups),sizeof(*ddi->groups_used),sizeof(rWAPriv));
printf("gem_def_groups: active_group=0x%x, num_groups=0x%x, groups_used=0x%x\n",
       pWAPriv->active_group,pWAPriv->num_groups,pWAPriv->groups_used);
#endif

  /*
   * Copy groups_used array from ddi storage
   */

 /* The pointer 'group_used' is an address in user space. If it is
  * accessed it could cause a crash. Since the data we are interested
  * in is immediately following the this pointer, it will be modified
  * here to use a kernel address See Defect 137696
  */
  ddi->groups_used = (ddi + 1);

  for(i=0; i<num; i++){
#ifdef GEM_DBUG
  printf("group[%d] = 0x%x\n",i,ddi->groups_used[i]);
#endif
    pWAPriv->groups_used[i] = ddi->groups_used[i];
    if (pWAPriv->groups_used[i].type == gZBufferGroup)
    { for (pRcx=pWA->pHead; pRcx != NULL; pRcx=pRcx->pLinkWA)
      { if (pRcx->pWG && (pWGPriv = pRcx->pWG->pPriv) &&
	    !pWGPriv->zBufferWind)
	{ GM_GUARD(pRcx->pDomain, pRcx->pProc);
	  pWGPriv->zBufferWind = TRUE;
	  if (++pDevP->num_zbuf_wind == 2)
	  { 
	    pSgca = (struct _sgca *)pBuf;
	    pSgca->len = sizeof(struct _sgca);
	    pSgca->opcode = CE_SGCA;
	    pSgca->offset = 0x90;
	    pSgca->data = 1;
	    pBuf += sizeof(struct _sgca);
            protectRcx=gdp->domain[TRAVERSAL_FIFO_DOMAIN].pCur;
            gem_protect_zbuffer(gdp,protectRcx->pWG->wg.pClip,TravSeFifo,
                   se_buffer,&pBuf,1024); 
	  }
	  if (pBuf - se_buffer)
	  { seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
	    WTFIFO(TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP );
	    BUSMEM_DET(seg_reg);
	  }
	  GM_UNGUARD(pRcx->pDomain);
	}
      }
    }
  }

#ifdef CHECK_CLIENT_CLIP
  printf("0: pWA->pPriv = 0x%x \n", pWA->pPriv);
  printf("0: ((rWAPrivPtr)(pWA->pPriv))->ClientClipWA = 0x%x \n", ((rWAPrivPtr)(pWA->pPriv))->ClientClipWA);
#endif

  if ( pWA->pPriv ) 
  {
    pWAPriv->ClientClipWA  = ((rWAPrivPtr)(pWA->pPriv))->ClientClipWA;
    pWAPriv->gWAChangeMask = ((rWAPrivPtr)(pWA->pPriv))->gWAChangeMask;
    rFree(pWA->pPriv);
  }
  else
  {
    pWAPriv->ClientClipWA  = NULL;
    pWAPriv->gWAChangeMask = -1; 
  }

  pWA->pPriv = (genericPtr) pWAPriv;

#ifdef CHECK_CLIENT_CLIP
  printf("1: pWA->pPriv = 0x%x \n", pWA->pPriv);
  printf("1: ((rWAPrivPtr)(pWA->pPriv))->ClientClipWA = 0x%x \n", ((rWAPrivPtr)(pWA->pPriv))->ClientClipWA);
#endif

#ifdef GEM_DBUG
printf("Exit gem_def_groups\n");
#endif

  return(0);

}

/*
 * FUNCTION NAME: gem_wake_switch                                      
 *                                                                     
 * DESCRIPTION: executes an e_wakeup to wakeup any switch that's sleeping 
 *              while the immediate fifo is being used
 */
gem_wake_switch(gdp, ddi)
struct _gscDev	*gdp;
gWakeSwitchPtr	ddi;
{
  e_wakeup(&((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv.immfifo_busy);

  return(0) ;
}
