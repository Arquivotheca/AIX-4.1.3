static char sccsid[] = "@(#)89	1.8.3.5  src/bos/kernext/disp/gem/rcm/gem_eswtch.c, sysxdispgem, bos411, 9428A410j 9/10/93 17:29:09";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		Scenario_a
 *		Scenario_b
 *		Scenario_c
 *		gem_end_switch
 *		if
 *		switch
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
 *;MC   09/05/89   Created                                            @#
 *;DM   09/05/89   Modified                                             
 *;LW   09/14/89   Corrected DPM'ism (changed function name to iggm_ )  
 *;                Changed declaration of SE buffers                    
 *;                Moved subroutines to gem_esfun.c                     
 *;DM   09/14/89   changed pSlots to pCslot in mvcxtoff call            
 *;DM   09/14/89   lint fixes                                           
 *;DM   09/19/89   pass old_rcx to insert_fifo                          
 *;MC	09/22/89   changed function name from iggm_ to gem_	      @1
 *;LW   10/12/89   Added pWA to gem_upd_win routine                     
 *;LW   10/12/89   Added pWG to gem_upd_geom routine                    
 *;MC	10/24/89   Added hwid management, and removed call to set cmp	
 *		   mask and value, as it's not needed			
 *;CL   11/01/89   Fixed prolog
 *;LW   11/09/89   Add tests for old_rcx->pData being null 
 *;MC   11/21/89   Added hwid to gem_upd_geom
 *;MC	11/29/89   Changed KERNEL to _KERNEL and removed KGNHAK
 *;MC   01/25/89   Added color table id parameter to upd_geom
 *;LW   02/27/90   Add rp to insert_imm interface
 *;LW   03/13/90   Change interface to gem_upd_geom
 *;MC   03/23/90   Made changes for Gemini II
 *;LWMC 04/24/90   Made changes for new update geometry
 *;LW   05/02/90   Added test for old_rcx == new_rcx    
 *;LW   05/31/90   Removed pointer chasing to get old/new rcx from proc 
 *;LW   08/17/90   Fixed setting of RCMChgMask in domain case statement scen A
 */

#include <sys/sleep.h>
#include "gemincl.h"
#include "gemrincl.h"
#include "gem_geom.h"
#include "rcm_mac.h"
#include "gmasl.h"

Bool get_hwid();
void make_hwid_head();

gem_end_switch( gdp, old_rcx, new_rcx, seq_num )			/*@1*/
struct _gscDev  *gdp;           /* device dependent information             */
rcxPtr          old_rcx;        /* rendering context active on adapter      */
rcxPtr          new_rcx;        /* context to be made active on adapter     */
int             seq_num;
{
  int		 i;
  int            rc=0;			/* return code                      */
  rGemprocPtr	 pProcP;
  CxtSlot        *pSlots;		/* array of gemini slot information */
  int            faulting_domain_num;   /* the domain number of the fault   */
  rGemrcxPtr     pOldGemRcx;		/* ptr to old gemini rcx information*/
  rGemrcxPtr     pNewGemRcx;		/* ptr to new gemini rcx information*/
  int            old_cxt_slot_num;     /* slot number of previous ative slot*/
  int            new_cxt_slot_num;     /* slot number of previous ative slot*/
  struct gem_ddf *ddf;
  ulong          seg_reg;
  volatile ulong *dp;
  volatile ulong *p;
  ulong sync_element[100];


  rGemRCMPrivPtr pDevP =		/* device private area in the RCM   */
    &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
  GM_LOCK_LDAT(pDevP);
  ddf = (struct gem_ddf *) gdp->devHead.display->free_area;
  faulting_domain_num = new_rcx->domain;/* domain causing this fault        */

  /*
   * slot information                                                        
   */
  pSlots              = pDevP->slots;                 /* ptr to slot array  */

  if (old_rcx && old_rcx->pData!=NULL && old_rcx->pWG && old_rcx->pWG->pPriv) {
    pDevP->hwid[((rWGPrivPtr)old_rcx->pWG->pPriv)->hwid].currentlyUsed=0;
  }
  /*
   * If new_rcx is a 'null' rcx and faulting domain is the traversal domain,
   * that is a signal that old_rcx is being deleted.  The following code should
   * take care of four potential problems: 1) a process writing to the screen
   * after it's window is gone; 2) process B getting it's context into the same
   * slot as old_rcx's before all of old_rcx's structure elements have been
   * executed in the fifo; 3) a process being abnormally killed and leaving
   * a partial structure element in the fifo; and 4) we trash the traversal
   * fifo with a sync element in it
   */

  if (new_rcx->pData == NULL || new_rcx==NULL)
  {

#ifdef NEW_UCODE

    if (faulting_domain_num == TRAVERSAL_FIFO_DOMAIN)
    { 
      seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
      GMBASE_INIT(seg_reg, pDevP);
      dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.flush_drp;
      *dp = 1;
      dp = (volatile ulong *)&((GM_MMAP *)pDevP->gmbase)->gm_ucflags.reset_3d;
      *dp = 1;

      while (*dp) ;

      /* Send a sync down the traversal fifo because we may have flushed
       * a sync which was in the fifo.
       */
      sync_element[0]=0x001801e1; /* Move sync data to global memory */
      sync_element[1]=VME_ADR(IMM_SYNC_CNTR);
      sync_element[2]=DISP_BUF_OFFSET; sync_element[3]=0x00000002;
      sync_element[4]=0x00000000; /* don't care */
      sync_element[5]=pDevP->trv_sync_cntr;
      WTFIFO(TravSeFifo,sync_element,0x0018, seg_reg, pDevP);
       
      BUSMEM_DET(seg_reg);
    }

#endif

    GM_UNLOCK_LDAT(pDevP);
    return(0);
  }

 /*
  * get information about the new rcx                                       
  */
  pNewGemRcx          = (rGemrcxPtr) new_rcx->pData;/* gemini RCX structure */
  new_cxt_slot_num    = pNewGemRcx->start_slot;     /* fixed at create time */

 /*
  * get information about the old rcx, if it exists, and if it's not 'null'
  */
  if ( old_rcx != NULL && old_rcx->pData != NULL )
  {
    pOldGemRcx       = (rGemrcxPtr)old_rcx->pData;  /* gemini RCX structure */
    old_cxt_slot_num = pOldGemRcx->start_slot;
  }
  else
  {
    pOldGemRcx       = NULL;
    old_cxt_slot_num = -1;
  }

  /*
   * Make sure domain num is valid
   */
#ifdef GEM_DBUG
  printf("  domnum=%d\n", faulting_domain_num);
#endif
  assert(faulting_domain_num == TRAVERSAL_FIFO_DOMAIN ||
	 faulting_domain_num == IMMEDIATE_CREG_DOMAIN);

  /*
   * Process the graphics fault
   */
  if(old_rcx != new_rcx) {

      /*
       * Make faulting process the current process for threshold pacing
       * when it writes to the FIFO
       */
#ifdef TRACK_PID
      printf("%s %d ddf=0x%x pid=0x%x on domain %d\n",
	     __FILE__,__LINE__,ddf,new_rcx->pProc->procHead.pid,
	     faulting_domain_num );
#endif
      if (faulting_domain_num == TRAVERSAL_FIFO_DOMAIN) {
	ddf->trav_pid = new_rcx->pProc->procHead.pid;
	ddf->travpid_flags = AWAKE;
      }
      else {
	ddf->imm_pid =  new_rcx->pProc->procHead.pid;
	ddf->immpid_flags = AWAKE;
      }

      /*
       * If new rcx belongs in the slot that is currently active
       */
      if ( new_cxt_slot_num == old_cxt_slot_num )
      {
#ifdef GEM_DBUG
	printf("  Calling scenario_a \n");
#endif
                rc = Scenario_a(
			gdp,		         /* device specific info    */
			old_rcx,
			new_rcx,
			pSlots,
			faulting_domain_num
		     );
      } 
      else 
      /*
       * The new context is not bound to the same slot that is currently
       * active.  If the new context is on the adapter then the switch is
       * bantam weight.
       */
        if ( pNewGemRcx->status_flags & ON_ADAPT )
        {

#ifdef GEM_DBUG
	  printf("  Calling scenario_b \n");
#endif
                rc = Scenario_b(
			gdp,			/* device specific info    */
			old_rcx,
			new_rcx,
			faulting_domain_num
		     );
        } 
        else 
	/*
	 * The faulting rcx does not belong in the slot that is currently
	 * active and the faulting context is not on the adapter
	 */
        {
#ifdef GEM_DBUG
	  printf("  Calling scenario_c \n");
#endif
                rc = Scenario_c(
			gdp,			/* device specific info    */
			old_rcx,
			new_rcx,
			pSlots,
			faulting_domain_num
		     );
        }
      
      seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
      GMBASE_INIT(seg_reg, pDevP);
      
#ifndef POLL2D
  TER_INIT(0, pDevP);
  p = TER_P[0]; *p++ = ENABLE_THRESH; *p = ENABLE_THRESH;
  TER_INIT(1, pDevP);
  p = TER_P[1]; *p++ = ENABLE_THRESH; *p = ENABLE_THRESH;
#endif

#ifndef POLL3D
  TER_INIT(2, pDevP);
  p = TER_P[2]; *p++ = ENABLE_THRESH; *p = ENABLE_THRESH;
  TER_INIT(3, pDevP);
  p = TER_P[3]; *p++ = ENABLE_THRESH; *p = ENABLE_THRESH;
#endif
	  BUSMEM_DET(seg_reg);

#ifdef TRACK_THRESH
      seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

      for (i=0; i<4 ; i++) 
      {
         GMBASE_INIT(seg_reg, pDevP);
         TER_INIT(i, pDevP);
         p = TER_P[i];
         printf("%d: %d %d %x %x ",
	 i,
	 0x00000001 & p[0],
	 0x00000001 & p[1],
	 0x0000ffff & p[2], 0x0000ffff & p[3]);
      }
      printf("\n");
      BUSMEM_DET(seg_reg);
#endif
      
    } else {
#ifdef GEM_DBUG
      printf("Uneeded switch on %d\n",faulting_domain_num);
#endif
    }
#ifdef GEM_DBUG
	  printf("  Exit end switch \n");
#endif

  GM_UNLOCK_LDAT(pDevP);
  return(rc);
}

#define	SET_ALL_CHNGIND	0x0006B7FE


/*
 * FUNCTION: Scenario_a()                                                     
 *                                                                            
 * If new rcx belongs in the slot that is currently active, then the          
 * following must be done.                                                    
 *   a)   store_dsv                                                           
 *   b)   drain fifo                                                          
 *   c)   mvcxtoff( current )                                                 
 *   d)   mvcxton ( new     )                                                 
 *   e)   activate context and restore_dsv
 *                                                                            
 */
Scenario_a(gdp, old_rcx, new_rcx, pSlots, faulting_domain_num)
struct _gscDev  *gdp;           /* device dependent information            */
rcxPtr          old_rcx;        /* rendering context active on adapter     */
rcxPtr          new_rcx;        /* context to be made active on adapter    */
CxtSlot         *pSlots;	/* information about all gem rcx slots     */
int   	      	faulting_domain_num; /* domain that caused the fault       */
{
  	int   		n;			/* number of bytes in pBuf */
  	int   		rc=0;			/* return code             */
	char		se_buffer[8192];

  	int   	      	fifo_num;      		/* ImmSeFifo or TravSeFifo */
	int		slot_num;		/* slot that needs swapping*/
	CxtSlot		*pCslot;		/* ptr to gemini slot info */
  	rGemRCMPrivPtr 	pDevP; 			/* device private area     */

	char		*pBuf = &se_buffer[0];
	char		*buf_start = pBuf;
	ulong		seg_reg;
	ulong		*changemask;
	ushort		RCMChgMask = 0;	/* upd_geom's private change mask */
	rWGPrivPtr	pwgpriv;
	disp_buf_sync	*pDBS;
	Bool		new;
	ulong		*pmask;
	ulong		save_mask;
	sgm1		*psgm;
	rGemrcxPtr      rPtr;
	ASL             *aslPtr;
        RMINFO          *rminfoPtr;
        create_win_geom *arg;

#ifdef GEM_DBUG
printf("Entering scenario_a gdp=0x%x, old_rcx=0x%x, new_rcx=0x%x, pSlots=0x%x, faulting_domain_num=0x%x\n",gdp, old_rcx, new_rcx, pSlots, faulting_domain_num);
#endif

	pDevP	  = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
	slot_num  = ((rGemrcxPtr)new_rcx->pData)->start_slot;
  	pCslot   = &(pSlots[slot_num]);

	seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

        /*
         * prevent others from operating on this slot while we are             
         */
	LOCK_CXTSLOT(pCslot);

        /*
         * find fifo number belonging to the domain number                     
         */
	switch ( faulting_domain_num )
	{
	case TRAVERSAL_FIFO_DOMAIN:
	  RCMChgMask |= rTravFifo | rZBuf;
	  fifo_num = TravSeFifo;
	  break;
 
	case IMMEDIATE_CREG_DOMAIN:
	  RCMChgMask |= rImmFifo;
	  fifo_num = ImmSeFifo;
	  break;
	  
	default:					  /* error        */
	  fifo_num = GM_DOMAIN;
	  break;
	}
	if ( 0 > fifo_num ){
	  UNLOCK_CXTSLOT(pCslot);
	  BUSMEM_DET(seg_reg);
	  return (fifo_num);
	}

	/***********************************************************/
	/* Set sync counters for old geometry, so we know when all */
	/* of the SEs related to it have finished executing        */
	/***********************************************************/

	if ( old_rcx && old_rcx->pWG && (old_rcx->pWG != new_rcx->pWG) )
	{ 
          if (((rGemrcxPtr)old_rcx->pData)->cxt_type == IMM_RCXTYPE)
	    if (pDevP->imm_sync_cntr >= MAX_SYNC_CNTR)
 	       reset_sync_cntrs(gdp);
	    else
	    {
               if ( !((rWGPrivPtr)old_rcx->pWG->pPriv) )
                  iggm_create_win_geom(gdp, (rWGPrivPtr)old_rcx->pWG, arg);
               pwgpriv = (rWGPrivPtr)old_rcx->pWG->pPriv;
	       pwgpriv->imm_sync_cntr = ++pDevP->imm_sync_cntr;
	       pDBS = (disp_buf_sync *)pBuf;
	       pDBS->len = sizeof(disp_buf_sync);
	       pDBS->opcode = CE_DBS;
	       pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	       pDBS->ofst = DISP_BUF_OFFSET;
	       pDBS->flags = 0x1;
	       pDBS->imm_cntr = pDevP->imm_sync_cntr;
	       pBuf += sizeof(disp_buf_sync);
	       WTFIFO( fifo_num, buf_start, pBuf-buf_start, seg_reg, pDevP );
	       pBuf=buf_start;
	    }
	  else
	    if (pDevP->trv_sync_cntr >= MAX_SYNC_CNTR)
	       reset_sync_cntrs(gdp);
	    else
	    {
               if ( !((rWGPrivPtr)old_rcx->pWG->pPriv) )
                  iggm_create_win_geom(gdp, (rWGPrivPtr)old_rcx->pWG, arg);
               pwgpriv = (rWGPrivPtr)old_rcx->pWG->pPriv;
	       pwgpriv->trv_sync_cntr = ++pDevP->trv_sync_cntr;
	       pDBS = (disp_buf_sync *)pBuf;
	       pDBS->len = sizeof(disp_buf_sync);
	       pDBS->opcode = CE_DBS;
	       pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	       pDBS->ofst = DISP_BUF_OFFSET;
	       pDBS->flags = 0x2;
	       pDBS->trv_cntr = pDevP->trv_sync_cntr;
	       pBuf += sizeof(disp_buf_sync);
	       WTFIFO( fifo_num,buf_start,pBuf-buf_start, seg_reg, pDevP );
	       pBuf=buf_start;
	    }
	}

        /* 
         * issue the save drawing state vector command                         
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if(old_rcx != NULL && old_rcx->pData != NULL) {
	  n = store_dsv
	    (
	     gdp,                     /* device info                      */
	     old_rcx,                 /* rcx to save                      */
	     pBuf                     /* SE buffer                        */
	     );
	  if ( 0 > n ){
	    UNLOCK_CXTSLOT(pCslot);
	    BUSMEM_DET(seg_reg);
	    return(n);
	  }
	  pBuf += n;

	  if (((rGemrcxPtr)old_rcx->pData)->cxt_type == TRAV_RCXTYPE)
	  {
	    n = save_pick_stack(gdp, old_rcx, pBuf);
	    if (0 > n)
	    { UNLOCK_CXTSLOT(pCslot);
	      BUSMEM_DET(seg_reg);
	      return(n);
	    }
	    pBuf += n;
	  }

#ifdef GEM_DBUG
HERE_I_AM;
#endif
	  /* put SE's into adapter's fifo     */
	  WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );

	  /* reset pointer into SE buffer */
	  pBuf = buf_start;
	} 
#ifdef GEM_DBUG
HERE_I_AM;
#endif
        /*
         * wait for graphics processors to finish before taking the context    
         * off the adapter                                                     
         */
        FIFO_EMPTY(fifo_num, seg_reg, pDevP);   /* empty SE fifo                  */
        FIFO_EMPTY(fifo_num+1, seg_reg, pDevP); /* and corresponding blt data fifo*/
 
        /*
         * save the context currently on the adapter to a kernel buffer        
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	rc = mvcxtoff
	(
	     pCslot,		        /* slot to save                   */
	     old_rcx  	             /* move off context that was running */
	);
	if (rc)
	{
	      UNLOCK_CXTSLOT(pCslot);
	      BUSMEM_DET(seg_reg);
	      return(rc);
	}
 
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	/*
	 * Since the address of the context we're activating is the same as
	 * the old context, the microcode will only update those things
	 * indicated in the change mask.  Therefore, we want to set all of
	 * the bits in the change mask, but first we save the old value, so
	 * we can restore it later.
	 * NB: This code assumes that the ASL is the first thing in the slot.
	 */
	pmask = (ulong *)&((ASL *)((rGemrcxPtr)new_rcx->pData)->pCxt)->chngind;
	save_mask = *pmask;
	*pmask = SET_ALL_CHNGIND;

        rc = mvcxton                    /* move context onto the adapter  */
        (
           pCslot,                      /* slot to place context into     */
           new_rcx                      /* rcx to move onto the adapter   */
        );
        if (rc)
        {
	   UNLOCK_CXTSLOT(pCslot);
	   BUSMEM_DET(seg_reg);
	   return(rc);
        }

        /*
         * Make sure we have a valid hardware ID                            
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid < 0)
	{ 
#ifdef GEM_DBUG
	  printf("  *Need new hardware ID: getting ");
#endif
	  RCMChgMask |= (((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid + NUM_HWIDS)
	    		  << 8;
	  if (get_hwid(gdp, new_rcx, new_rcx->pWG, pBuf))
	  {
	    /* Set rcm flag to indicate that we need to steal the ID */
	    RCMChgMask |= rStealHwid;
	  }
	  RCMChgMask |= rNewHwid;
#ifdef GEM_DBUG
	  printf("%d as new ID\n",((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid);
#endif
	}
	else
	{
#ifdef GEM_DBUG
	  printf("  *Current hardware ID is %d pClip=0x%x\n",
		 ((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid,
		  new_rcx->pWG->wg.pClip);
#endif
	  make_hwid_head(pDevP, ((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid);
          pDevP->hwid[((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid].currentlyUsed=1;
	}

        /*
         * Perform deferred update window geometry 
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (new_rcx->pWG &&
	    new_rcx->pWG->pPriv) {

	  if (((rGemrcxPtr)new_rcx->pData)->cxt_type == TRAV_RCXTYPE)
	  { /*
	     * We need to close a hole where end_switch may call upd_geom,
	     * put the activate context for the new rcx into the buffer,
	     * but before the activate context is actually executed, a call
	     * to update_geometry is done for the new rcx's geometry.  Since
	     * the context is not active on the fifo, context-specific
	     * items are not done, expecting that they will be done at end_
	     * switch time, but end_switch has already run.  So to close this
	     * hole, we are activating the new rcx's context BEFORE calling
	     * upd_geom and making sure it takes effect.
	     */

	    /* Consider the following situation:  a process is writing
	     * to the traversal domain, displaying buffer B.  It loses its
	     * time slice, and its context is saved on the adapter.  Later,
	     * its window ID is stolen by someone else who needs it.  When
	     * we reactivate this process' context, we want it to have the
	     * right window ID.  If it doesn't, it puts all of its saved
	     * attributes onto the window which stole its ID.  This is not
	     * a good plan.
	     *
	     * To correct this problem, we find the address of the context
	     * on the adapter and update the window ID of that saved
	     * context to match the new window ID that this context is
	     * about to be given.
	     */

	    /* The address of the private context information */
            rPtr = (rGemrcxPtr)new_rcx->pData;

	    /* The offset of the ASL in adapter memory */
            aslPtr = (ASL *)(rPtr->pASL);
            aslPtr = (ASL *)( (ulong)aslPtr & 0x0fffffff );

	    /* Add the start of the adapter memory to that for an address
	     * we can really use.
	     */
            GMBASE_INIT(seg_reg, pDevP);
            aslPtr = ((ASL*)((ulong)aslPtr + (ulong)pDevP->gmbase));

	    /* Find the start of the rminfo section. */
            rminfoPtr = (RMINFO*)((ulong)aslPtr->prminfo & 0x00ffffff);
	    rminfoPtr = (RMINFO*)((ulong)rminfoPtr + (ulong)pDevP->gmbase);

	    /* The value is stored in the top 16 bits, not the bottom 16 */
            rminfoPtr->wcmpval=0x2b<<16;
            rminfoPtr->rmchngind |= (1<<14);
    
	    n = activate_cxt(gdp, new_rcx, pBuf);
	    pBuf += n;
	    new = ((rGemrcxPtr)new_rcx->pData)->status_flags & NEW_CXT;
	    n = load_dsv(gdp, new_rcx, pBuf);
	    pBuf += n;
	    if (!new)
	    { n = restore_pick_stack(gdp, new_rcx, pBuf);
	      pBuf += n;
	    }
	    WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
	    pBuf = buf_start;
	    RCMChgMask |= rCxtIsActive;
	    FIFO_EMPTY(fifo_num, seg_reg, pDevP);
	  }

	  changemask = &((rGemrcxPtr)new_rcx->pData)->gWGChangeMask;

	  /*
	   * Lock device to ensure that we are not in upd_geom multiple
	   * times at the same time
	   */

	  rc = upd_geom(gdp,
			new_rcx,
			new_rcx->pWG,
			new_rcx->pWA,
			new_rcx->pWG,
			changemask,
			RCMChgMask,
			fifo_num,
			buf_start,
			&pBuf,
			8192);


	  if ( 0 > rc ){
	    UNLOCK_CXTSLOT(pCslot);
	    BUSMEM_DET(seg_reg);
	    return(rc);
	  }
	}

	/*
	 * Now that we've activated the context, reset the change bit
	 * indicators in ASL
	 */
	pmask = (ulong *)&((ASL *)pCslot->slot_addr)->chngind;
	psgm = (sgm1 *)pBuf;
	psgm->sgm.len = sizeof(sgm1);
	psgm->sgm.opcode = CE_GMEM;
	psgm->sgm.adr = VME_ADR(pmask);
	psgm->data = save_mask;
	pBuf += sizeof(sgm1);
 
        /*
         * Perform deferred update window attributes                         
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (new_rcx->pWA &&
	    new_rcx->pWA->pPriv)
	{
	  changemask = &((rGemrcxPtr)new_rcx->pData)->gWAChangeMask;
	  RCMChgMask = 0;

	  rc = gem_upd_win(gdp,
			   new_rcx,
			   new_rcx->pWA,
			   new_rcx->pWG,
			   *changemask,
			   RCMChgMask,
			   fifo_num,
			   buf_start,
			   &pBuf,
			   8192);
	  if ( 0 > rc ){
	      UNLOCK_CXTSLOT(pCslot);
	      BUSMEM_DET(seg_reg);
	      return(rc);
	    }
	  ((rGemrcxPtr)new_rcx->pData)->gWAChangeMask = 0;
	}
 
	/*
	 * put SE's into fifo
	 */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (pBuf - buf_start)
	{ WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
	}

#ifdef GEM_DBUG
HERE_I_AM;
#endif
	UNLOCK_CXTSLOT(pCslot);
	BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
	printf("Exiting scenario_a \n");
#endif

	return(0);
      }                                                /* end Scenario_a () */


/*
 * FUNCTION: Scenario_b()                                                     
 *                                                                            
 * If the new context is on the adapter and not in the same slot as the       
 * previous active context then the switch is bantam weight                   
 * and we do the following:                                                   
 *                                                                            
 *  a) Set up necessary Structure Elements
 *  c) put SE's into the fifo
 *                                                                            
 */
Scenario_b(gdp, old_rcx, new_rcx, faulting_domain_num)
struct _gscDev  *gdp;           /* device dependent information             */
rcxPtr          old_rcx;        /* rendering context active on adapter      */
rcxPtr          new_rcx;        /* context to be made active on adapter     */
int             faulting_domain_num;	/* domain causing the fault	    */
{
    	int     n;			/* number of bytes in SE buffer     */
	char	se_buffer[8192];
    	int     rc=0;			/* return code( 0 is good )         */
    	int     fifo_num;		/* ImmSeFifo or TravSeFifo          */
    	rGemRCMPrivPtr pDevP =		/* device private area              */
      		&(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);

	char		*pBuf = &se_buffer[0];
	char		*buf_start = pBuf;
	ulong		seg_reg;
	ulong		*changemask;
	ushort		RCMChgMask = 0;	/* upd_geom's private change mask */
	rWGPrivPtr	pwgpriv;
	disp_buf_sync	*pDBS;
	Bool		new;
	rGemrcxPtr      rPtr;
	ASL             *aslPtr;
        RMINFO          *rminfoPtr;
        create_win_geom *arg;

#ifdef GEM_DBUG
printf("Entering scenario_b gdp=0x%x, old_rcx=0x%x, new_rcx=0x%x, faulting_domain_num=0x%x\n",gdp, old_rcx, new_rcx, faulting_domain_num);
#endif

	seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
        /*
         * find fifo number belonging to the domain number                 
         */
        switch ( faulting_domain_num )
	  {
	  case TRAVERSAL_FIFO_DOMAIN:
	    RCMChgMask |= rTravFifo | rZBuf;
	    fifo_num = TravSeFifo;
	    break;
	    
	  case IMMEDIATE_CREG_DOMAIN:
	    RCMChgMask |= rImmFifo;
	    fifo_num = ImmSeFifo;
	    break;
	    
          default:
	    fifo_num = GM_DOMAIN;
	    break;
	  }

	if ( 0 > fifo_num ) 
           return (fifo_num);

	/***********************************************************/
	/* Set sync counters for old geometry, so we know when all */
	/* of the SEs related to it have finished executing        */
	/***********************************************************/

	if ( old_rcx && old_rcx->pWG && (old_rcx->pWG != new_rcx->pWG) )
	{ 
          if (((rGemrcxPtr)old_rcx->pData)->cxt_type == IMM_RCXTYPE)
	    if (pDevP->imm_sync_cntr >= MAX_SYNC_CNTR)
	       reset_sync_cntrs(gdp);
	    else
	    {
               if ( !((rWGPrivPtr)old_rcx->pWG->pPriv) )
                  iggm_create_win_geom(gdp, (rWGPrivPtr)old_rcx->pWG, arg);
               pwgpriv = (rWGPrivPtr)old_rcx->pWG->pPriv;
	       pwgpriv->imm_sync_cntr = ++pDevP->imm_sync_cntr;
	       pDBS = (disp_buf_sync *)pBuf;
	       pDBS->len = sizeof(disp_buf_sync);
	       pDBS->opcode = CE_DBS;
	       pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	       pDBS->ofst = DISP_BUF_OFFSET;
	       pDBS->flags = 0x1;
	       pDBS->imm_cntr = pDevP->imm_sync_cntr;
	       pBuf += sizeof(disp_buf_sync);
	       WTFIFO(fifo_num,buf_start,pBuf-buf_start, seg_reg, pDevP);
	       pBuf=buf_start;
	    }
	  else
	    if (pDevP->trv_sync_cntr >= MAX_SYNC_CNTR)
	       reset_sync_cntrs(gdp);
	    else
	    {
               if ( !((rWGPrivPtr)old_rcx->pWG->pPriv) )
                  iggm_create_win_geom(gdp, (rWGPrivPtr)old_rcx->pWG, arg);
               pwgpriv = (rWGPrivPtr)old_rcx->pWG->pPriv;
	       pwgpriv->trv_sync_cntr = ++pDevP->trv_sync_cntr;
	       pDBS = (disp_buf_sync *)pBuf;
	       pDBS->len = sizeof(disp_buf_sync);
	       pDBS->opcode = CE_DBS;
	       pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	       pDBS->ofst = DISP_BUF_OFFSET;
	       pDBS->flags = 0x2;
	       pDBS->trv_cntr = pDevP->trv_sync_cntr;
	       pBuf += sizeof(disp_buf_sync);
	       WTFIFO(fifo_num,buf_start,pBuf-buf_start, seg_reg, pDevP);
	       pBuf=buf_start;
	    }
	}

        /*
         * store state vector of drawing engine                         
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if(old_rcx != NULL && old_rcx->pData != NULL) {
	  n = store_dsv                /* Save drawing state vector to      */
	    (                          /* global memory.                    */
	     gdp,                      /* device info                       */
	     old_rcx,                  /* rcx to save                       */
	     pBuf                      /* SE buffer                         */
	     );
	  if ( 0 > n ){
	    BUSMEM_DET(seg_reg);
	    return(n);
	  }
	  pBuf += n;

	  if (((rGemrcxPtr)old_rcx->pData)->cxt_type == TRAV_RCXTYPE)
	  {
	    n = save_pick_stack(gdp, old_rcx, pBuf);
	    if (0 > n)
	    { BUSMEM_DET(seg_reg);
	      return(n);
	    }
	    pBuf += n;
	  }
        } 

        /*
         * Make sure we have a valid hardware ID   
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid < 0)
	{ 
#ifdef GEM_DBUG
	  printf("  *Need new hardware ID: getting ");
#endif
	  RCMChgMask |= (((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid + NUM_HWIDS)
	    		  << 8;
	  if (get_hwid(gdp, new_rcx, new_rcx->pWG, pBuf))
	  {
	    /* Set rcm flag to indicate that we need to steal the ID */
	    RCMChgMask |= rStealHwid;
	  }
	  RCMChgMask |= rNewHwid;
#ifdef GEM_DBUG
	  printf("%d as new ID\n",((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid);
#endif
	}
	else
	{
#ifdef GEM_DBUG
	  printf("  *Current hardware ID is %d pClip=0x%x\n",
		 ((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid,
		  new_rcx->pWG->wg.pClip);
#endif
	  make_hwid_head(pDevP, ((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid);
          pDevP->hwid[((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid].currentlyUsed=1;
	}

        /*
         * Perform deferred update window geometry                         
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
        STOP_OTHR_FIFO(pBuf,117,FALSE);
	if (new_rcx->pWG &&
	    new_rcx->pWG->pPriv) {

	  if (((rGemrcxPtr)new_rcx->pData)->cxt_type == TRAV_RCXTYPE)
	  { /*
	     * We need to close a hole where end_switch may call upd_geom,
	     * put the activate context for the new rcx into the buffer,
	     * but before the activate context is actually executed, a call
	     * to update_geometry is done for the new rcx's geometry.  Since
	     * the context is not active on the fifo, context-specific
	     * items are not done, expecting that they will be done at end_
	     * switch time, but end_switch has already run.  So to close this
	     * hole, we are activating the new rcx's context BEFORE calling
	     * upd_geom and making sure it takes effect.
	     */

	    /* Consider the following situation:  a process is writing
	     * to the traversal domain, displaying buffer B.  It loses its
	     * time slice, and its context is saved on the adapter.  Later,
	     * its window ID is stolen by someone else who needs it.  When
	     * we reactivate this process' context, we want it to have the
	     * right window ID.  If it doesn't, it puts all of its saved
	     * attributes onto the window which stole its ID.  This is not
	     * a good plan.
	     *
	     * To correct this problem, we find the address of the context
	     * on the adapter and update the window ID of that saved
	     * context to match the new window ID that this context is
	     * about to be given.
	     */

	    /* The address of the private context information */
            rPtr = (rGemrcxPtr)new_rcx->pData;

	    /* The offset of the ASL in adapter memory */
            aslPtr = (ASL *)(rPtr->pASL);
            aslPtr = (ASL *)( (ulong)aslPtr & 0x0fffffff );

	    /* Add the start of the adapter memory to that for an address
	     * we can really use.
	     */
            GMBASE_INIT(seg_reg, pDevP);
            aslPtr = ((ASL*)((ulong)aslPtr + (ulong)pDevP->gmbase));

	    /* Find the start of the rminfo section. */
            rminfoPtr = (RMINFO*)((ulong)aslPtr->prminfo & 0x00ffffff);
	    rminfoPtr = (RMINFO*)((ulong)rminfoPtr + (ulong)pDevP->gmbase);

	    /* The value is stored in the top 16 bits, not the bottom 16 */
            rminfoPtr->wcmpval=0x2b<<16;
            rminfoPtr->rmchngind |= (1<<14);

	    n = activate_cxt(gdp, new_rcx, pBuf);
	    pBuf += n;
	    new = ((rGemrcxPtr)new_rcx->pData)->status_flags & NEW_CXT;
	    n = load_dsv(gdp, new_rcx, pBuf);
	    pBuf += n;
	    if (!new)
	    { n = restore_pick_stack(gdp, new_rcx, pBuf);
	      pBuf += n;
	    }
	    WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
	    pBuf = buf_start;
	    RCMChgMask |= rCxtIsActive;
	    FIFO_EMPTY(fifo_num, seg_reg, pDevP);
	  }

	  changemask = &((rGemrcxPtr)new_rcx->pData)->gWGChangeMask;

	  /*
	   * Lock device to ensure that we are not in upd_geom multiple
	   * times at the same time
	   */

	  rc = upd_geom(gdp,
			new_rcx,
			new_rcx->pWG,
			new_rcx->pWA,
			new_rcx->pWG,
			changemask,
			RCMChgMask,
			fifo_num,
			buf_start,
			&pBuf,
			8192);


	  if ( 0 > rc ){
	    BUSMEM_DET(seg_reg);
	    return(rc);
	  }
	}

        /*
         * Perform deferred update window attributes     
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (new_rcx->pWA &&
	    new_rcx->pWA->pPriv)
	{
	  changemask = &((rGemrcxPtr)new_rcx->pData)->gWAChangeMask;
	  RCMChgMask = 0;

	  rc = gem_upd_win(gdp,
			   new_rcx,
			   new_rcx->pWA,
			   new_rcx->pWG,
			   *changemask,
			   RCMChgMask,
			   fifo_num,
			   buf_start,
			   &pBuf,
			   8192);
	  if ( 0 > rc ){
	    BUSMEM_DET(seg_reg);
	    return(rc);
	  }
	  ((rGemrcxPtr)new_rcx->pData)->gWAChangeMask = 0;
	}

	/*
	 * insert SE's into immediate fifo
	 */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
        START_OTHR_FIFO(pBuf,118);
	if (pBuf - buf_start)
	{ WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
	}

	BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
	printf("Exiting scenario_b \n");
#endif

        return(0);
}                                                      /* end scenario_b () */


/*
 * FUNCTION: Scenario_c()                                                     
 * If faulting rcx does not belong in the slot that is currently active and   
 * the faulting context is not on the adapter, then:                          
 *                                                                            
 *  a) save_dsv ( currently active context )                                  
 *  b) mvcxtoff ( pSlots[new_cxt_slot_num].rcx-node->pRcx );                  
 *  c) mvcxton  ( new_rcx );                                                  
 *  d) set up rest of necessary Structure Elements
 *  e) put the SE's into the fifo                                     
 *                                                                            
 */
Scenario_c(gdp, old_rcx, new_rcx, pSlots, faulting_domain_num) 
struct _gscDev  *gdp;           /* device dependent information             */
rcxPtr          old_rcx;        /* rendering context active on adapter      */
rcxPtr          new_rcx;        /* context to be made active on adapter     */
CxtSlot         *pSlots;	/* information about all gem rcx slots      */
int             faulting_domain_num;	/* domain causing the fault	    */
{
    	int     	n, i;		/* number of bytes in SE buffer     */

    	char    	se_buffer[8192];/* small private SE buffer          */
    	int     	rc=0;		/* return code                      */
    	int     	fifo_num;      	/* ImmSeFifo or TravSeFifo          */
	int		slot_num;	/* slot number being swapped        */
  	CxtSlot 	*pCslot;	/* ptr to gemini context slot info  */
	rcx_node	*tpnode;	/* temp ptr to cxt slot		    */
	rcxPtr		tprcx;		/* temp ptr to rcx		    */
  	rGemRCMPrivPtr  pDevP;		/* pointer to device private area   */

	char		*pBuf = &se_buffer[0];
	char		*buf_start = pBuf;
	ulong		seg_reg;
	ulong		*changemask, *p;
	ushort		RCMChgMask = 0;	/* upd_geom's private change mask */
	rWGPrivPtr	pwgpriv;
	disp_buf_sync	*pDBS;
	Bool		new;
	rGemrcxPtr      rPtr;
	ASL             *aslPtr;
        RMINFO          *rminfoPtr;
        create_win_geom *arg;

#ifdef GEM_DBUG
       printf("Entering scenario_c gdp=0x%x, old_rcx=0x%x, new_rcx=0x%x, pSlots=0x%x, faulting_domain_num=0x%x\n",gdp, old_rcx, new_rcx, pSlots, faulting_domain_num);
#endif


	pDevP     = &(((rGemDataPtr)(gdp->devHead.vttld))->GemRCMPriv);
	slot_num  = ((rGemrcxPtr)new_rcx->pData)->start_slot;
	pCslot	  = &(pSlots[slot_num]);
	
	seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
        GMBASE_INIT(seg_reg, pDevP);

#ifdef THRESH_DBUG
printf("Threshold regs:\n");
for (i=0; i<4 ; i++) {
  TER_INIT(i, pDevP);
  p = TER_P[i];
  printf("FIFO %d REGS @ 0x%x: ENA_H %x ENA_L %x THRESH_H %x THRESH_L %x\n",
	 i,p,p[0],p[1],p[2],p[3]);
}
#endif

        /*
         * prevent others from operating on this slot while we are 
         */
	LOCK_CXTSLOT(pCslot);

        /*
         * find fifo number belonging to the domain number        
         */
        switch ( faulting_domain_num )
	  {
	  case TRAVERSAL_FIFO_DOMAIN:
	    RCMChgMask |= rTravFifo | rZBuf;
	    fifo_num = TravSeFifo;
	    break;
	    
	  case IMMEDIATE_CREG_DOMAIN:
	    RCMChgMask |= rImmFifo;
	    fifo_num = ImmSeFifo;
	    break;
	    
	  default:
	    fifo_num = GM_DOMAIN;
	    break;
	  }

	if ( 0 > fifo_num )
        {
	  UNLOCK_CXTSLOT(pCslot);
	  BUSMEM_DET(seg_reg);
	  return (fifo_num);
	}

	/***********************************************************/
	/* Set sync counters for old geometry, so we know when all */
	/* of the SEs related to it have finished executing        */
	/***********************************************************/

	if ( old_rcx && old_rcx->pWG && (old_rcx->pWG != new_rcx->pWG) ) 
        {
	  if (((rGemrcxPtr)old_rcx->pData)->cxt_type == IMM_RCXTYPE)
	    if (pDevP->imm_sync_cntr >= MAX_SYNC_CNTR)
	        reset_sync_cntrs(gdp);
	    else
	    { 
               if ( !((rWGPrivPtr)old_rcx->pWG->pPriv) )
                  iggm_create_win_geom(gdp, (rWGPrivPtr)old_rcx->pWG, arg);
               pwgpriv = (rWGPrivPtr)old_rcx->pWG->pPriv;
	       pwgpriv->imm_sync_cntr = ++pDevP->imm_sync_cntr;
	       pDBS = (disp_buf_sync *)pBuf;
	       pDBS->len = sizeof(disp_buf_sync);
	       pDBS->opcode = CE_DBS;
	       pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	       pDBS->ofst = DISP_BUF_OFFSET;
	       pDBS->flags = 0x1;
	       pDBS->imm_cntr = pDevP->imm_sync_cntr;
	       pBuf += sizeof(disp_buf_sync);
	       WTFIFO(fifo_num,buf_start,pBuf-buf_start, seg_reg, pDevP);
	       pBuf=buf_start;
	    }
	  else
	    if (pDevP->trv_sync_cntr >= MAX_SYNC_CNTR)
	       reset_sync_cntrs(gdp);
	    else
	    {
               if ( !((rWGPrivPtr)old_rcx->pWG->pPriv) )
                  iggm_create_win_geom(gdp, (rWGPrivPtr)old_rcx->pWG, arg);
               pwgpriv = (rWGPrivPtr)old_rcx->pWG->pPriv;
	       pwgpriv->trv_sync_cntr = ++pDevP->trv_sync_cntr;
	       pDBS = (disp_buf_sync *)pBuf;
	       pDBS->len = sizeof(disp_buf_sync);
	       pDBS->opcode = CE_DBS;
	       pDBS->adr = VME_ADR(IMM_SYNC_CNTR);
	       pDBS->ofst = DISP_BUF_OFFSET;
	       pDBS->flags = 0x2;
	       pDBS->trv_cntr = pDevP->trv_sync_cntr;
	       pBuf += sizeof(disp_buf_sync);
	       WTFIFO(fifo_num,buf_start,pBuf-buf_start, seg_reg, pDevP);
	       pBuf=buf_start;
	    }
	}

        /*
         * store state vector of drawing engine                        
         */
	if(old_rcx != NULL && old_rcx->pData != NULL) {
	  n = store_dsv                  /* Save drawing state vector to   */
	    (                            /* global memory.                 */
	     gdp,                        /* device info                    */
	     old_rcx,                    /* rcx to save                    */
	     pBuf	 		 /* SE buffer                      */
	     );
	  if ( 0 > n )
	    {
	      UNLOCK_CXTSLOT(pCslot);
	      BUSMEM_DET(seg_reg);
	      return(n);
	    }
	  pBuf += n;

	  if (((rGemrcxPtr)old_rcx->pData)->cxt_type == TRAV_RCXTYPE)
	  {
	    n = save_pick_stack(gdp, old_rcx, pBuf);
	    if (0 > n)
	    { UNLOCK_CXTSLOT(pCslot);
	      BUSMEM_DET(seg_reg);
	      return(n);
	    }
	    pBuf += n;
	  }

#ifdef GEM_DBUG
HERE_I_AM;
#endif
	  /* put SE's into adapter's fifo   */
	  WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );

	  /* reset pointer into SE buffer */
	  pBuf = buf_start;
	}

	/*
	 * swap context in new_rcx's slot if necessary
	 * we must drain the fifos first, to ensure that there
	 * are no structure elements in the fifo for this context
	 */
	if (pCslot->status_flags & ON_ADAPT)
        { /*
	   * find rcx currently in new_rcx's slot
	   */
	  tprcx = NULL;
	  for (tpnode=pCslot->pHead; tpnode != NULL; tpnode = tpnode->pNext)
	    if (((rGemrcxPtr)tpnode->pRcx->pData)->status_flags & ON_ADAPT)
	    { tprcx = tpnode->pRcx;
	      break;
	    }
	  assert(tprcx);

	  /*
	   * drain fifos
	   */
	  FIFO_EMPTY(fifo_num, seg_reg, pDevP);
	  FIFO_EMPTY(fifo_num + 1, seg_reg, pDevP);

	  /*
	   * save the context currently on the adapter to a kernel buffer 
	   */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	  rc = mvcxtoff
	    (
	     pCslot,
	     tprcx
	     );  	             /* move off context that was running  */
	  if ( rc )
	  {
	    UNLOCK_CXTSLOT(pCslot);
	    BUSMEM_DET(seg_reg);
	    return(rc);
	  }
	}
	  
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	rc = mvcxton                 /* move context onto the adapter      */
	  ( 
	   pCslot,                 /* device info                        */
	   new_rcx                 /* rcx to move onto the adapter       */
	   );
	if ( rc )
	{
	  UNLOCK_CXTSLOT(pCslot);
	  BUSMEM_DET(seg_reg);
	  return(rc);
	}

        /*
         * Make sure we have a valid hardware ID                      
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid < 0)
	{ 
#ifdef GEM_DBUG
	  printf("  *Need new hardware ID: getting ");
#endif
	  RCMChgMask |= (((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid + NUM_HWIDS)
	    		  << 8;
	  if( get_hwid(gdp, new_rcx, new_rcx->pWG, pBuf))
	  {
	    /* Set rcm flag to indicate that we need to steal the ID */
	    RCMChgMask |= rStealHwid;
	  }
	  RCMChgMask |= rNewHwid;
#ifdef GEM_DBUG
	  printf("%d as new ID\n",((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid);
#endif
	}
	else
	{
#ifdef GEM_DBUG
	  printf("  *Current hardware ID is %d pClip=0x%x \n",
		 ((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid,
		 new_rcx->pWG->wg.pClip);
#endif
	  make_hwid_head(pDevP, ((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid);
          pDevP->hwid[((rWGPrivPtr)new_rcx->pWG->pPriv)->hwid].currentlyUsed=1;
	}

        /*
         * Perform deferred update window geometry          
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
        STOP_OTHR_FIFO(pBuf,119,FALSE);
	if (new_rcx->pWG &&
	    new_rcx->pWG->pPriv) {

	  if (((rGemrcxPtr)new_rcx->pData)->cxt_type == TRAV_RCXTYPE)
	  { /*
	     * We need to close a hole where end_switch may call upd_geom,
	     * put the activate context for the new rcx into the buffer,
	     * but before the activate context is actually executed, a call
	     * to update_geometry is done for the new rcx's geometry.  Since
	     * the context is not active on the fifo, context-specific
	     * items are not done, expecting that they will be done at end_
	     * switch time, but end_switch has already run.  So to close this
	     * hole, we are activating the new rcx's context BEFORE calling
	     * upd_geom and making sure it takes effect.
	     */

	    /* Consider the following situation:  a process is writing
	     * to the traversal domain, displaying buffer B.  It loses its
	     * time slice, and its context is saved on the adapter.  Later,
	     * its window ID is stolen by someone else who needs it.  When
	     * we reactivate this process' context, we want it to have the
	     * right window ID.  If it doesn't, it puts all of its saved
	     * attributes onto the window which stole its ID.  This is not
	     * a good plan.
	     *
	     * To correct this problem, we find the address of the context
	     * on the adapter and update the window ID of that saved
	     * context to match the new window ID that this context is
	     * about to be given.
	     */

	    /* The address of the private context information */
            rPtr=(rGemrcxPtr)new_rcx->pData;

	    /* The offset of the ASL in adapter memory */
            aslPtr = (ASL *)(rPtr->pASL);
            aslPtr = (ASL *)( (ulong)aslPtr & 0x0fffffff );

	    /* Add the start of the adapter memory to that for an address
	     * we can really use.
	     */
            GMBASE_INIT(seg_reg, pDevP);
            aslPtr = ((ASL*)((ulong)aslPtr + (ulong)pDevP->gmbase));

	    /* Find the start of the rminfo section. */
            rminfoPtr = (RMINFO*)((ulong)aslPtr->prminfo & 0x00ffffff);
	    rminfoPtr = (RMINFO*)((ulong)rminfoPtr + (ulong)pDevP->gmbase);

	    /* The value is stored in the top 16 bits, not the bottom 16 */
            rminfoPtr->wcmpval = 0x2b<<16;
            rminfoPtr->rmchngind |= (1<<14);

	    n = activate_cxt(gdp, new_rcx, pBuf);
	    pBuf += n;
	    new = ((rGemrcxPtr)new_rcx->pData)->status_flags & NEW_CXT;
	    n = load_dsv(gdp, new_rcx, pBuf);
	    pBuf += n;
	    if (!new)
	    { n = restore_pick_stack(gdp, new_rcx, pBuf);
	      pBuf += n;
	    }
	    WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
	    pBuf = buf_start;
	    RCMChgMask |= rCxtIsActive;
	    FIFO_EMPTY(fifo_num, seg_reg, pDevP);
	  }

	  changemask = &((rGemrcxPtr)new_rcx->pData)->gWGChangeMask;

	  /*
	   * Lock device to ensure that we are not in upd_geom multiple
	   * times at the same time
	   */

	  rc = upd_geom(gdp,
			new_rcx,
			new_rcx->pWG,
			new_rcx->pWA,
			new_rcx->pWG,
			changemask,
			RCMChgMask,
			fifo_num,
			buf_start,
			&pBuf,
			8192);


	  if ( 0 > rc ){
	    UNLOCK_CXTSLOT(pCslot);
	    BUSMEM_DET(seg_reg);
	    return(rc);
	  }
	}

        /*
         * Perform deferred update window attributes                 
         */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
	if (new_rcx->pWA &&
	    new_rcx->pWA->pPriv) {

	  changemask = &((rGemrcxPtr)new_rcx->pData)->gWAChangeMask;
	  RCMChgMask = 0;

	  rc = gem_upd_win(gdp,
			   new_rcx,
			   new_rcx->pWA,
			   new_rcx->pWG,
			   *changemask,
			   RCMChgMask,
			   fifo_num,
			   buf_start,
			   &pBuf,
			   8192);
	  if ( 0 > rc ){
	    UNLOCK_CXTSLOT(pCslot);
	    BUSMEM_DET(seg_reg);
	    return(rc);
	  }
	  ((rGemrcxPtr)new_rcx->pData)->gWAChangeMask = 0;
	}
 
	/*
	 * put SE's into fifo
	 */
#ifdef GEM_DBUG
HERE_I_AM;
#endif
        START_OTHR_FIFO(pBuf,120);
	if (pBuf - buf_start)
	{ WTFIFO( fifo_num, buf_start, pBuf - buf_start, seg_reg, pDevP );
	}

#ifdef GEM_DBUG
HERE_I_AM;
#endif
	UNLOCK_CXTSLOT(pCslot);
	BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
	printf("Exiting scenario_c \n");
#endif

        return(0);
}                                                       /* end Senario_c() */
