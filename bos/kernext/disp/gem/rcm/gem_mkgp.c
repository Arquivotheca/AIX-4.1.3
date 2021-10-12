static char sccsid[] = "@(#)93	1.7.1.8  src/bos/kernext/disp/gem/rcm/gem_mkgp.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:20:53";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		iggm_make_gp
 *		iggm_unmake_gp
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
/*;CL 08/31/89	Created							*/
/*;LW 09/01/89	Restructured and trimmed				*/
/*;DM 09/01/89  Reviewed and fixed                                      */
/*;CL 09/10/89  Assign gm_base to datap                              @1 */
/*;MC 09/13/89  Changed to pass shmem to make_gp		      @2*/
/*;LW 09/15/89  Added pointers to gcp read pointers	 	      @3*/
/*;MC 09/15/89  Added malloc for proc private space		      @4*/
/*;DM 09/20/89  Added xmem routines for fifo pointers                 @5*/
/*;MC 09/22/89  Moved gmbase & position_ofst init in pDevP to gem_ircx	*/
/*;		and changed returned value from gmbase to pos offset  @6*/
/*;DM 09/27/89  Filled in gm card slot and domain information         @7*/
/*;CL 10/09/89  Casted bus_mem_start_ram to unsigned long to avoid      */
/*;             compile warning                                       @8*/
/*;LW 11/09/89  Remove Excp and change to new mkgp interface          @9*/
/*;LW 11/20/89  Added malloc of xmem structure                        @A*/
/*;MC 11/29/89  Changed KERNEL to _KERNEL and removed KGNHAK		*/
/*;LW 12/13/89  Restructured hfiles				        */
/*;LW 01/11/90  Changed xs to structure					*/
/*;LW 01/11/90  Add unmake_gp          					*/
/*;CL 01/19/90  Add gemini hardware features to datap		      @B*/
/*;LW 03/05/90  Add xmattach for fifo locks                             */
/*;LW 03/28/90  Changes for 3 domain                                    */
/*;**********************************************************************/

/*****  System Include Files  *******************************************/

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"
#include "rcm_mac.h"

int iggm_make_gp(gdp, pproc, datap, length) 			/* @9 */
struct _gscDev   *gdp;
rcmProcPtr	 pproc;			/* current rcm process pointer    @9 */
rMakegpdd        *datap;				            /*@2*/
int              length;

{
  int		 i;
  int		 rc;			/* return code		      */
  
  struct phys_displays
    *pd = gdp->devHead.display;
  rGemDataPtr	 pgd	  = (rGemDataPtr)gdp->devHead.vttld;
  rGemRCMPrivPtr pDevP	  = &(pgd->GemRCMPriv);
  rGemprocPtr	pGemProcP;						/*@A*/
  
  datap->position_ofst    = pDevP->position_ofst;                    /*@1@2@6*/
  
  /* initialize gemini backplane configuration information                   @7*/
  datap->gcardslots.magic = pgd->gm_crdslots.magic;
  datap->gcardslots.gcp   = pgd->gm_crdslots.gcp;
  datap->gcardslots.drp   = pgd->gm_crdslots.drp;
  datap->gcardslots.shp   = pgd->gm_crdslots.shp;
  datap->gcardslots.imp   = pgd->gm_crdslots.imp;
  datap->screen_width_mm  = pd->display_info.screen_width_mm;
  datap->screen_height_mm  = pd->display_info.screen_height_mm;
  
  /* initialize domain addresses and lengths			    @7*/
  
  i = IMMEDIATE_CREG_DOMAIN;
  datap->domain_addr[i].addr =
    (unsigned long) pd->busmemr[i].bus_mem_start_ram;               /*@8*/
  datap->domain_addr[i].length =
    pd->busmemr[i].bus_mem_end_ram -
      pd->busmemr[i].bus_mem_start_ram + 1;
  
  i = TRAVERSAL_FIFO_DOMAIN;
  datap->domain_addr[i].addr =
    (unsigned long) pd->busmemr[i].bus_mem_start_ram;               /*@8*/
  datap->domain_addr[i].length =
    pd->busmemr[i].bus_mem_end_ram -
      pd->busmemr[i].bus_mem_start_ram + 1;

  i =  IMMEDIATE_FIFO_DOMAIN;
  datap->domain_addr[i].addr =
    (unsigned long) pd->busmemr[i].bus_mem_start_ram;               /*@8*/
  datap->domain_addr[i].length =
    pd->busmemr[i].bus_mem_end_ram -
      pd->busmemr[i].bus_mem_start_ram + 1;
  
  
#ifdef GEM_DBUG
  printf("odmdds = %x\n", pd->odmdds) ;
#endif GEM_DBUG
  
  datap->features = ((struct gem_dds *)pd->odmdds)->features ;           /*@B*/
  
  
  /*
   * xmalloc rGemproc, pin rGemproc
   */
  
#ifdef GEM_DBUG
  printf("pproc: %x,  prochead: %x\n",pproc,gdp->devHead.pProc);
#endif GEM_DBUG
  
  if (pproc == NULL)
    return(GM_NOT_GP);

  
  if ( pproc->procHead.pPriv == NULL ) {
    if((pproc->procHead.pPriv = xmalloc(sizeof(rGemproc),3,pinned_heap))==NULL)
      {
	
#ifdef GEM_DBUG
	printf("No memory for rGemProc\n");
#endif GEM_DBUG
	
	gemlog(NULL,"hispd3d","iggm_make_gp","malloc",NULL,NULL,UNIQUE_1);
	return(ERROR);
      }
#ifdef LOCK_GAI
    
    /*
     * Init rGemproc
     */
    pGemProcP = (rGemprocPtr)(pproc->procHead.pPriv);		/*@A*/

    pGemProcP->shmem = datap->shmem;
    pGemProcP->pLocks = datap->pFifoLock;

    /*
     * Use Unused field in kernel shmFifo structure to
     * maintain flag used to determine first time mkgp has been called
     */
    if(pDevP->shmem->rcm_lock == FALSE) {
      pDevP->shmem->rcm_lock = TRUE;
      datap->pFifoLock = NULL;
    }
    
#ifdef GEM_DBUG
    printf("pGemProcP->shmem=0x%x, pGemProcP->pLocks=0x%x\n",
	   pGemProcP->shmem, pGemProcP->pLocks);
    printf("features = %d, pd->odmdds->features = %d\n",
	   datap->features, ((struct gem_dds *)pd->odmdds)->features) ;
#endif GEM_DBUG
    

    /*
     * User process passes in a pointer to its fifo pointers
     * We attach the memory to kernel memory space   
     */
    pGemProcP->xs.aspace_id = XMEM_INVAL;
    if((xmattach(                      /* initialize xmem descriptor       */
		 pGemProcP->shmem,             /* user buffer                      */
		 sizeof(shmFifo),              /* buf length                       */
		 &pGemProcP->xs,               /* put xmem descriptor in proc struct */
		 USER_ADSPACE
		 )) != XMEM_SUCC ){
      pGemProcP->shmem = NULL;
      gemlog(NULL,"hispd3d","iggm_make_gp",
	     "attach fifo ptrs",NULL,GM_NOMEMORY,UNIQUE_1);
      return(ERROR);
    }
    
    
    /*
     * User process passes in a pointer to its fifo locks
     * We attach the memory to kernel memory space   
     */
    pGemProcP->xs_fifolocks.aspace_id = XMEM_INVAL;
    if((xmattach(                      /* initialize xmem descriptor       */
		 pGemProcP->pLocks,            /* user buffer                      */
		 sizeof(FifoLock),            /* buf length                       */
		 &pGemProcP->xs_fifolocks,     /* put xmem descriptor in proc struct */
		 USER_ADSPACE
		 )) != XMEM_SUCC ){
      pGemProcP->pLocks = NULL;
      gemlog(NULL,"hispd3d","iggm_make_gp",
	     "attach fifo locks",NULL,GM_NOMEMORY,UNIQUE_1);
      return(ERROR);
    }
#endif

  }   
  return(0);
}

  int
  iggm_unmake_gp( gdp, pproc ) 
struct _gscDev *gdp;
rcmProcPtr	 pproc;			/* current rcm process pointer    @9 */

{
  int rc;
  struct phys_displays
    *pd = gdp->devHead.display;
  rGemDataPtr	 pgd	  = (rGemDataPtr)gdp->devHead.vttld;
  rGemRCMPrivPtr pDevP	  = &(pgd->GemRCMPriv);
  
#ifdef ONE_3D_WINDOW
  /*
   * Clear pid of one allowable 3d process
   */
  if(pproc->procHead.pid == pDevP->cur_3D_pid)
    pDevP->cur_3D_pid = NULL;
#endif


#ifdef GEM_DBUG
  printf("Entering iggm_unmake_gp\n");
#endif GEM_DBUG
  
  xmfree( pproc->procHead.pPriv, pinned_heap);

#ifdef GEM_DBUG
HERE_I_AM;
#endif GEM_DBUG

  pproc->procHead.pPriv = NULL;
  
#ifdef GEM_DBUG
  printf("Exiting iggm_unmake_gp \n");
#endif GEM_DBUG

  return(rc);
}
