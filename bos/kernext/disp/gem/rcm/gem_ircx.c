static char sccsid[] = "@(#)92	1.7.3.6  src/bos/kernext/disp/gem/rcm/gem_ircx.c, sysxdispgem, bos41J, 9521B_all 5/25/95 15:26:26";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_iact
 *		iggm_init_rcx
 *		init_rcm_cxt
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
/*;CHANGE HISTORY                                                       */
/*;lw 09/22/89 Created							*/
/*;MC 09/22/89 Removed unneeded malloc for RCM Private area		*/
/*;DM 09/28/89 Moved phys_displays initialization to gem_open           */
/*;MC 10/24/89 Added initialization of hwid fields in RCM Priv area   @1*/
/*;MC 11/29/89 Changed KERNEL to _KERNEL and removed KGNHAK		*/
/*;LW 12/13/89  Restructured hfiles				        */
/*;LW 05/11/90 Added gem_iact routine				        */
/*;**********************************************************************/
#define  XMAP_NMULTIMAP 16
#include <sys/syspest.h>
#include <sys/sleep.h>

#include "gemincl.h"
#include "gemrincl.h"
#include "gmasl.h"		

void init_rcm_cxt();

int iggm_init_rcx(pd, pgd)
struct phys_displays *pd;			/* read only 		*/
rGemDataPtr pgd;				/* vtt local data 	*/
{
  int 			i, j, k;
  ulong			auth;
  CxtSlot		*pSlots;
  ulong			free_mem;
  ulong			rem_mem;
  ulong			pslot;
  ulong			seg_reg;
  ulong			*dp;
  shmFifoPtr		pShmem;
  frame_buf_cmp		*pFbc;
  windattrs		*pWat;
  DRSTVC		*pDSV;
  rGemRCMPrivPtr 	pDevP	= &pgd->GemRCMPriv;

  ulong  gm_base = 0x00000000 | ((struct gem_dds *)pd->odmdds)->io_bus_mem_start;

#ifdef GEM_DBUG
  printf("Enter iggm_init_rcx: pd = 0x%x  pgd = 0x%x\n", pd, pgd);
  printf(" gm_base = 0x%x\n", gm_base);
#endif

  /*
   * Initialize RCM's private  fields
   */
  pDevP->pPrivCxt = ((GM_MMAP *)gm_base)->rcm_priv_cxt;

  /* Init cursor pointer based on amt of memory on adapter */
  if( ((struct gem_dds *)pd->odmdds)->features & MEMORY_4_MEG) {
    pDevP->pCursplanes = &(((GM_MMAP4 *)gm_base)->cursplane[0][0]);
    pgd->free_store = (uint)&((GM_MMAP4 *)gm_base)->mem[0];
    pgd->free_store_len = GMMaxGblMem4Meg - 
	((pgd->free_store + GMMaxGblMem4Meg) & 0xfff);
  } else {
    pDevP->pCursplanes = &(((GM_MMAP *)gm_base)->cursplane[0][0]);
    pgd->free_store = (uint)&((GM_MMAP *)gm_base)->mem[0];
    pgd->free_store_len = GMMaxGblMem - 
	((pgd->free_store + GMMaxGblMem) & 0xfff);
  }

  pDevP->gmbase 	        = gm_base;	/* adapter base address */
  pDevP->position_ofst          = gm_base & 0x0FFFFFFF;  

  /*
   * Init array of CxtSlot structures for managing GM Global memory
   */
  pSlots	   	        = pDevP->slots;
  free_mem = pgd->free_store_len;
  pDevP->num_trav_slots = free_mem / sizeof(TRAV_SLOT);
  rem_mem = free_mem - pDevP->num_trav_slots * sizeof(TRAV_SLOT);
  pDevP->num_imm_slots = rem_mem / sizeof(IMM_SLOT);
  while (pDevP->num_imm_slots < 16)
  { --pDevP->num_trav_slots;
    rem_mem = free_mem - pDevP->num_trav_slots * sizeof(TRAV_SLOT);
    pDevP->num_imm_slots = rem_mem / sizeof(IMM_SLOT);
  }
  pDevP->first_trav_slot = 0;
  pDevP->first_imm_slot = pDevP->num_trav_slots;

#ifdef GEM_DBUG
  printf(" num_trav_slots=%d num_imm_slots=%d\n", pDevP->num_trav_slots,
	 pDevP->num_imm_slots);
#endif

  pslot = pgd->free_store;
  for (i=pDevP->first_trav_slot;
       i<pDevP->first_trav_slot + pDevP->num_trav_slots; ++i)
  { 
    pSlots[i].slot_addr		= pslot;
#ifdef GEM_DBUG_SLOTS
    printf("   trav slot %d addr = 0x%x\n",i,pSlots[i].slot_addr);
#endif
    pslot += sizeof(TRAV_SLOT);
    pSlots[i].slot_len		= sizeof(TRAV_SLOT);
    pSlots[i].type		= TRAV_RCXTYPE ;
    pSlots[i].status_flags	= 0 ;
    pSlots[i].slot_lock		= 0 ;
    pSlots[i].num_rcx		= 0 ;
    pSlots[i].pHead		= NULL ;
  }

  for (i=pDevP->first_imm_slot; i<pDevP->first_imm_slot + pDevP->num_imm_slots;
       ++i)
  { 
    pSlots[i].slot_addr		= pslot;
#ifdef GEM_DBUG_SLOTS
    printf("   imm slot %d addr = 0x%x\n",i,pSlots[i].slot_addr);
#endif
    pslot += sizeof(IMM_SLOT);
    pSlots[i].slot_len		= sizeof(IMM_SLOT);
    pSlots[i].type		= IMM_RCXTYPE ;
    pSlots[i].status_flags	= 0 ;
    pSlots[i].slot_lock		= 0 ;
    pSlots[i].num_rcx		= 0 ;
    pSlots[i].pHead		= NULL ;
  }

  /*
   * Initialize old clip region ptrs
   */
  pDevP->old_zbuffer = NULL;


  /*
   * Initialize vars in rGemRCMPriv needed by code ported from RMS
   */
  
  pDevP->gcp_geo       =  pgd->gm_crdslots.gcp;
  pDevP->drp_geo       =  pgd->gm_crdslots.drp;
  pDevP->shp_geo       =  pgd->gm_crdslots.shp;
  
  pDevP ->gar_reg      =  (volatile ulong  *) (gm_base + GAR_ofst);
  pDevP ->gcr_reg      =  (volatile ulong  *) (gm_base + GCR_ofst);
  pDevP ->gsr_reg      =  (volatile ulong  *) (gm_base + GSR_ofst);
  pDevP ->ipr_reg      =  (volatile ulong  *) (gm_base + IPR_ofst);
  pDevP ->idc_reg      =  (volatile ulong  *) (gm_base + IDC_ofst);
  pDevP ->iur_reg[0]   =  (volatile ulong  *) (gm_base + IUR0_ofst);
  pDevP ->add_reg[0]   =  (volatile ulong  *) (gm_base + ADD0_ofst);
  pDevP ->sub_reg[0]   =  (volatile ulong  *) (gm_base + SUB0_ofst);
  pDevP ->thres_reg[0] =  (volatile ulong  *) (gm_base + THR0_ofst);
  pDevP ->iur_reg[1]   =  (volatile ulong  *) (gm_base + IUR1_ofst);
  pDevP ->add_reg[1]   =  (volatile ulong  *) (gm_base + ADD1_ofst);
  pDevP ->sub_reg[1]   =  (volatile ulong  *) (gm_base + SUB1_ofst);
  pDevP ->thres_reg[1] =  (volatile ulong  *) (gm_base + THR1_ofst);
  pDevP ->iur_reg[2]   =  (volatile ulong  *) (gm_base + IUR2_ofst);
  pDevP ->add_reg[2]   =  (volatile ulong  *) (gm_base + ADD2_ofst);
  pDevP ->sub_reg[2]   =  (volatile ulong  *) (gm_base + SUB2_ofst);
  pDevP ->thres_reg[2] =  (volatile ulong  *) (gm_base + THR2_ofst);
  pDevP ->iur_reg[3]   =  (volatile ulong  *) (gm_base + IUR3_ofst);
  pDevP ->add_reg[3]   =  (volatile ulong  *) (gm_base + ADD3_ofst);
  pDevP ->sub_reg[3]   =  (volatile ulong  *) (gm_base + SUB3_ofst);
  pDevP ->thres_reg[3] =  (volatile ulong  *) (gm_base + THR3_ofst);
  pDevP ->ser_reg[0]   =  (volatile ulong  *) (gm_base + SER0_ofst);
  pDevP ->ser_reg[1]   =  (volatile ulong  *) (gm_base + SER1_ofst);
  pDevP ->ser_reg[2]   =  (volatile ulong  *) (gm_base + SER2_ofst);
  pDevP ->ser_reg[3]   =  (volatile ulong  *) (gm_base + SER3_ofst);
  pDevP ->ter_reg[0]   =  (volatile ulong  *) (gm_base + TER0_ofst);
  pDevP ->ter_reg[1]   =  (volatile ulong  *) (gm_base + TER1_ofst);
  pDevP ->ter_reg[2]   =  (volatile ulong  *) (gm_base + TER2_ofst);
  pDevP ->ter_reg[3]   =  (volatile ulong  *) (gm_base + TER3_ofst);
  pDevP ->pFunnel[0]   =  (volatile char   *) (gm_base + FNL0_ofst);
  pDevP ->pFunnel[1]   =  (volatile char   *) (gm_base + FNL1_ofst);
  pDevP ->pFunnel[2]   =  (volatile char   *) (gm_base + FNL2_ofst);
  pDevP ->pFunnel[3]   =  (volatile char   *) (gm_base + FNL3_ofst);

  pDevP ->ip_reg[0]   =  (volatile ulong  *) (gm_base + IP0_ofst);
  pDevP ->ip_reg[1]   =  (volatile ulong  *) (gm_base + IP1_ofst);
  pDevP ->ip_reg[2]   =  (volatile ulong  *) (gm_base + IP2_ofst);
  pDevP ->ip_reg[3]   =  (volatile ulong  *) (gm_base + IP3_ofst);

  /*
  * Init RCM's pointers to the fifo's
  */
  pDevP->shmem = (shmFifoPtr)rMalloc(sizeof(shmFifo));
  if(pDevP->shmem <= 0) {
	gemlog(NULL,"hispd3d","iggm_init_rcx","iggm_init_rcx",
					   NULL,NULL,UNIQUE_1);
	return(ERROR);
  }
  pShmem 	     = pDevP->shmem;
  pShmem->fi[0]      = 0;
  pShmem->fi[1]      = 0;
  pShmem->fi[2]      = 0;
  pShmem->fi[3]      = 0;
  pShmem->fifo[0].cp = (char *)(((GM_MMAP *)gm_base)->ise_fifo);
  pShmem->fifo[1].cp = (char *)(((GM_MMAP *)gm_base)->ipd_fifo);
  pShmem->fifo[2].cp = (char *)(((GM_MMAP *)gm_base)->tse_fifo);
  pShmem->fifo[3].cp = (char *)(((GM_MMAP *)gm_base)->tpd_fifo);
  pShmem->rcm_lock   = FALSE;

  /*
   * Initialize structures for managing hardware IDs		      @1
   */
  pDevP->num_free_hwid = DWA_HWID_RESRVD;
  pDevP->priv_hwid = NUM_HWIDS - NUM_VLTS*4-1;
  pDevP->hwid_head = DWA_LAST_HWID;
  pDevP->hwid_tail = DWA_FIRST_HWID;

  for (i=0; i<NUM_HWIDS; ++i)
  { pDevP->hwid[i].pwg = NULL;

    if (i < NUM_HWIDS-NUM_VLTS*4)

    { pDevP->hwid[i].next = i - 1;
      pDevP->hwid[i].prev = i + 1;
      pDevP->hwid[i].currentlyUsed=0;
    }
  }

  pDevP->hwid[DWA_LAST_HWID].prev = -1;
  pDevP->hwid[DWA_FIRST_HWID].next = -1;

  pDevP->prot_hwids[NUM_VLTS-1][0][0] = PROTECT_HWID;

  for (i=0, j=NUM_HWIDS-2; i<NUM_VLTS; ++i)
  { if (i != NUM_VLTS-1)
      pDevP->prot_hwids[i][0][0] = j--;
    pDevP->prot_hwids[i][0][1] = j--;
    pDevP->prot_hwids[i][1][0] = j--;
    pDevP->prot_hwids[i][1][1] = j--;
  }

  pDevP->imm_sync_cntr = 0;
  pDevP->trv_sync_cntr = 0;

  pDevP->fifo_sem = 0;

  pDevP->num_zbuf_wind = 0;

  pDevP->lock = LOCK_AVAIL;	/* Var for lockl system call */

  /*
   * Initialize count of 3D processes
   */
  pDevP->cur_3D_pid = NULL;

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  GMBASE_INIT(seg_reg, pDevP);

  /*
   * Initialize Z Buffer Clear Flag in GCP Comm Area
   */
  /* Select the GCP Comm Area						*/
  dp = (ulong *)(pDevP->gmbase + GAR_ofst);
  *dp = pDevP->gcp_geo;

  dp = (ulong *)(pDevP->gmbase + 0x90);
  *dp = 0;

  BUSMEM_DET(seg_reg);

#ifdef FIFO_SYNC
  pDevP->generic1 = 0x03100000;
#endif

return(SUCCESS);

}

/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: gem_iact                                         */
/*                                                                  */
/* DESCRIPTIVE Name:  Initialize adapter for MOM use for VTTACT     */
/*                                                                  */
/* FUNCTION:                                                        */
/*           Initialize overlay color table                         */
/*           Initialize Window ID table                             */
/*                                                                  */
/* INPUTS:   Pointer to Virtual Terminal data structure             */
/*                                                                  */
/* OUTPUTS:                                                         */
/*                                                                  */
/* CALLED BY:                                                       */
/*                                                                  */
/* CALLS:                                                           */
/*                                                                  */
/********************************************************************/
  int gem_iact(ld)
struct gemini_data *ld;
{
  ulong			seg_reg;
  long                  i, j, k, len;
  volatile ulong	*p;
  uint                  str_elem_buff[2048];
  char			*pBuf;
  ulong			*pl;
  generic_vme		*pVme;
  
  typedef struct lopct_se {
    ushort len;
    ushort opcode;
    ulong flags;
    ulong adr;
  } lopct_se;
  
  typedef	struct sgm_bufr_struct {
    ushort    len;
    ushort    opcode;
    ulong     adr;
    ulong     colors[16];
  } opct_sgm;
  
  typedef	struct output_buf {
    opct_sgm		sgm;
    lopct_se		opct;
    frame_buf_cmp	fbc;
    windattrs		watt;
    generic_se	sel_olay;
    generic_se	set_olay_disp;
    generic_se	sel_base;
    generic_se	set_base_disp;
  } output_buf;
  
  output_buf		*pOut;
  
  generic_se 		*pSE;
  windattrs 		*pWat;
  frame_buf_cmp 	*pFbc;
  lopct_se    		*lopctPtr;
  opct_sgm		*sgmPtr;
  rGemRCMPrivPtr	pDevP = &(ld->GemRCMPriv);
  
#ifdef GEM_DBUG
  printf("gem_iact entry\n");
#endif

  pOut = (output_buf *)str_elem_buff;
  
  /*
   * Set overlay planes color table                      
   */
  pOut->sgm.len = sizeof(struct sgm_bufr_struct);
  pOut->sgm.opcode = CE_GMEM;
  pOut->sgm.adr = VME_ADR(&(((GM_MMAP *)pDevP->gmbase)->bpct[0][0]));
  pOut->sgm.colors[0]  = 0x00000000;
  pOut->sgm.colors[1]  = 0x00ff0000;
  pOut->sgm.colors[2]  = 0x0000ff00;
  pOut->sgm.colors[3]  = 0x00ffffff;
  pOut->sgm.colors[4]  = 0x000000ff;
  pOut->sgm.colors[5]  = 0x00800000;
  pOut->sgm.colors[6]  = 0x00ffff00;
  pOut->sgm.colors[7]  = 0x00008080;
  pOut->sgm.colors[8]  = 0x00800080;
  pOut->sgm.colors[9]  = 0x00808000;
  pOut->sgm.colors[10] = 0x0080ffff;
  pOut->sgm.colors[11] = 0x00ff80ff;
  pOut->sgm.colors[12] = 0x00ffff80;
  pOut->sgm.colors[13] = 0x00808080;
  pOut->sgm.colors[14] = 0x00ff00ff;
  pOut->sgm.colors[15] = 0x00ffffff;
  
  pOut->opct.len = sizeof(lopct_se);
  pOut->opct.opcode = CE_LOPC;
  pOut->opct.flags = 0;
  pOut->opct.adr = pOut->sgm.adr;
  
  /*  
    wrfifo(ImmFIFO,str_elem_buff,pOut->sgm.len+pOut->opct.len,ld);
    */  
  
  /*
   * Initialize an entry in the adapter's window look-up 
   * table for the RCM                                  
   */
  
  /* Set compare value to PROTECT_HWID                    */
  pOut->fbc.len = sizeof(frame_buf_cmp);
  pOut->fbc.opcode = CE_FBC;
  pOut->fbc.flags = 0x08;
  pOut->fbc.mask = 0;
  pOut->fbc.value = PROTECT_HWID;
  
  /* Set color table to be X's                            */
  pOut->watt.len = sizeof(windattrs);
  pOut->watt.opcode = CE_WATT;
  pOut->watt.mask = 0x0f;
  pOut->watt.ctid = NUM_VLTS-1;
  pOut->watt.flags = 0;
  
  /* Select overlay planes                                */
  pOut->sel_olay.len = sizeof(generic_se);
  pOut->sel_olay.opcode = CE_SDFB;
  pOut->sel_olay.data = OVERLAY_PLANES;
  
  /* Set display buffer to be A                           */
  pOut->set_olay_disp.len = sizeof(generic_se);
  pOut->set_olay_disp.opcode = CE_FCTL;
  pOut->set_olay_disp.data = 0;
  
  /* Select base planes                                   */
  pOut->sel_base.len = sizeof(generic_se);
  pOut->sel_base.opcode = CE_SDFB;
  pOut->sel_base.data = BASE_PLANES;
  
  /* Set display buffer to be A                           */
  pOut->set_base_disp.len = sizeof(generic_se);
  pOut->set_base_disp.opcode = CE_FCTL;
  pOut->set_base_disp.data = 0;
  
  len = sizeof(output_buf);
  
#ifdef GEM_DBUG
  printf("Writing 0x%x bytes to fifo\n", len);
#endif
  
  wrfifo(ImmFIFO, str_elem_buff, len , ld);
  
  
  
  for (i=0; i<NUM_VLTS; ++i)
    for (j=0; j<2; ++j)
      for (k=0; k<2; ++k)
	if (i != NUM_VLTS-1 || j != 0 || k != 0)
	  { len = 0;
	    pFbc = (frame_buf_cmp *)str_elem_buff;
	    /* Set compare value to appropraie window id    */
	    pFbc->len = sizeof(frame_buf_cmp);
	    pFbc->opcode = CE_FBC;
	    pFbc->flags = 0x08;
	    pFbc->mask = 0;
	    pFbc->value = ld->GemRCMPriv.prot_hwids[i][j][k];
#ifdef INITWLT
	    printf("I%x CBO %d %d %d ",pFbc->value,i,j,k);
	    printf("\n");
#endif
	    len += sizeof(frame_buf_cmp);
	    /* Set appropriate color table                  */
	    pWat = (windattrs *)(pFbc + 1);
	    pWat->len = sizeof(windattrs);
	    pWat->opcode = CE_WATT;
	    pWat->mask = 0x0f;
	    pWat->ctid = i;
	    pWat->flags = 0;
	    len += sizeof(windattrs);
	    /* Select overlay planes                        */
	    pSE = (generic_se *)(pWat + 1);
	    pSE->len = sizeof(generic_se);
	    pSE->opcode = CE_SDFB;
	    pSE->data = OVERLAY_PLANES;
	    len += sizeof(generic_se);
	    /* Set appropriate display buffer               */
	    pSE++;
	    pSE->len = sizeof(generic_se);
	    pSE->opcode = CE_FCTL;
	    pSE->data = k << 30;
	    len += sizeof(generic_se);
	    /* Select base planes                           */
	    pSE++;
	    pSE->len = sizeof(generic_se);
	    pSE->opcode = CE_SDFB;
	    pSE->data = BASE_PLANES;
	    len += sizeof(generic_se);
	    /* Set appropriate display buffer               */
	    pSE++;
	    pSE->len = sizeof(generic_se);
	    pSE->opcode = CE_FCTL;
	    pSE->data = j << 30;
	    len += sizeof(generic_se);
#ifdef GEM_DBUG
	    printf("Writing 0x%x bytes to fifo\n", len);
#endif
	    wrfifo(ImmFIFO, str_elem_buff, len, ld);
	  }
  
  
  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  GMBASE_INIT(seg_reg, pDevP);
  
  /*
   * Init threshold values
   */
#define HIGH2D	128
#define HIGH3D	HIGH2D

#define LO2D	(HIGH2D+512)
#define LO3D	LO2D

#define HIGH2DBLT	(512)
#define HIGH3DBLT	HIGH2DBLT

#define LO2DBLT	(HIGH2DBLT+(1024*10))
#define LO3DBLT	LO2DBLT

  for ( i=0; i<4; i++)
     THRES_INIT(i, pDevP);

  p = THRES_P[0]; *p++ = FifoLen - (HIGH2D);    *p = FifoLen - (LO2D);
  p = THRES_P[1]; *p++ = FifoLen - (HIGH2DBLT); *p = FifoLen - (LO2DBLT);
  p = THRES_P[2]; *p++ = FifoLen - (HIGH3D);    *p = FifoLen - (LO3D);
  p = THRES_P[3]; *p++ = FifoLen - (HIGH3DBLT); *p = FifoLen - (LO3DBLT);

#ifdef TRACK_THRESH

printf("Threshold regs:\n");

for (i=0; i<4; i++)
   TER_INIT(i, pDevP);
   
for (i=0; i<4 ; i++) {
  p = TER_P[i];
  printf("FIFO %d: ENA_H %d ENA_L %d THRESH_H %x THRESH_L %x\n",
	 i,0x00000001 & p[0],0x00000001&p[1],0x0000ffff&p[2],0x0000ffff&p[3]);
}
#endif

  BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
  printf(" About to do RCM private context initialization\n");
#endif

  init_rcm_cxt(ld);

#ifdef GEM_DBUG
  printf("gem_iact exit\n");
#endif

  return;
}

/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: init_rcm_cxt                                     */
/*                                                                  */
/* DESCRIPTIVE Name:  Initialize RCM's private context		    */
/*                                                                  */
/* FUNCTION:                                                        */
/*           Initialize rminfo, asl, view table, view addressing    */
/*           table, and dsv					    */
/*                                                                  */
/* INPUTS:   Pointer to Virtual Terminal data structure             */
/*                                                                  */
/* OUTPUTS:                                                         */
/*                                                                  */
/* CALLED BY:                                                       */
/*                                                                  */
/* CALLS:                                                           */
/*                                                                  */
/********************************************************************/
void init_rcm_cxt(ld)
struct gemini_data *ld;
{
  rGemRCMPrivPtr	pDevP = &(ld->GemRCMPriv);
  TRAV_CONTEXT		*pCxt;
  TRAV_CONTEXT		*gmcxt;
  ulong			seg_reg;
  char			se_buffer[4096];
  char			*pBuf = se_buffer;
  ulong			*pl;
  generic_se		*pSE;
  generic_vme		*pVme;
  ac_se			*pACbuf;

#ifdef GEM_DBUG
  printf("Enter init_rcm_cxt\n");
#endif

  /*
   * Initialize the RCM's private context
   */
  if ((pCxt = (TRAV_CONTEXT *)rMalloc(sizeof(TRAV_CONTEXT))) == NULL)
    return;
  gmcxt = (TRAV_CONTEXT *)pDevP->pPrivCxt;

					/*------------------------------*/
					/* RMINFO initialization	*/
					/*------------------------------*/
  pCxt->rminfo.rmchngind	= 0;		/* change indicator	*/
  pCxt->rminfo.wfrmbufmsk	= (~GAI_WIN_PLANES & 0xff) << 16;
						/* wndw plne frm bfr msk*/
  pCxt->rminfo.wcmpmask		= 0;		/* wndw plne compare msk*/
  pCxt->rminfo.wcmpval		= 0;		/* wndw plne compare val*/

  pCxt->rminfo.polylclr		= GMDefaultColor; /* polyline color	*/
  pCxt->rminfo.intclr		= GMDefaultColor; /* interior color	*/
  pCxt->rminfo.wlogop		= 0;		/* logical op		*/
  pCxt->rminfo.windorgx		= 0;		/* window origin (x)	*/
  pCxt->rminfo.windorgy		= 0;		/* window origin (y)	*/
  pCxt->rminfo.windwidth	= itof(GM_WIDTH - 1); /* window width-1	*/
  pCxt->rminfo.windheight	= itof(GM_HEIGHT - 1);/* window height-1*/
  pCxt->rminfo.woflgs		= 0x00000028;	/* flags		*/
						/* (unobsc & window)	*/
  pCxt->rminfo.numpikatst	= 0;

					/*------------------------------*/
					/* ASL initialization		*/
					/* (adapter state list)		*/
					/*------------------------------*/
  pCxt->asl.dsvo	= VME_ADR(gmcxt+1);	/* offset to dsv	*/
  pCxt->asl.nviewtab	= 1;			/* # view tables	*/
  pCxt->asl.viewato	= VME_ADR(&gmcxt->vat[0]); /* offset to vat	*/

  pCxt->asl.npylnbtb	= 0;			/* # Polyline bundle tbl*/
  pCxt->asl.pylnbtbo	= 0;			/* Offset to Pyln bun tb*/
  pCxt->asl.npymkbtb	= 0;			/* # Polymarker bun tbl */
  pCxt->asl.pymkbtbo	= 0;			/* Offset to Pymk bun tb*/
  pCxt->asl.ntextbtb	= 0;			/* # Text bundle table	*/
  pCxt->asl.textbtbo	= 0;			/* Offset to Text bun tb*/
  pCxt->asl.nintrbtb	= 0;			/* # Interior bun tbl	*/
  pCxt->asl.intrbtbo	= 0;			/* Offset to Intr bun tb*/
  pCxt->asl.nedgebtb	= 0;			/* # Edge bundle table	*/
  pCxt->asl.edgebtbo	= 0;			/* Offset to Edge bun tb*/
  pCxt->asl.npattbl	= 0;			/* # pattern tables	*/
  pCxt->asl.pattao	= 0;			/* offset to pt		*/
  pCxt->asl.nhattbl	= 0;			/* # hatch tables	*/
  pCxt->asl.htblo	= 0;			/* offset to ht		*/
  pCxt->asl.nlntbl	= 0;			/* # line type tables	*/
  pCxt->asl.lnptbo	= 0;			/* offset to ltt	*/
  pCxt->asl.lnprtbo     = 0;                    /* offset to line type
                                                   rendering table      */

  pCxt->asl.nmktytb		= 0;		/* # Marker type tbl	*/ 
  pCxt->asl.mktytbo		= 0;		/* Offset to Mk type tb */
  pCxt->asl.ncullsztb		= 0;		/* # Cull size table	*/
  pCxt->asl.cullsztbo		= 0;		/* Offset to Cull sz tb */
  pCxt->asl.ndepthcutb		= 0;		/* # Depth cue table	*/ 
  pCxt->asl.depthcutbo		= 0;		/* Offset to Depth cu tb*/
  pCxt->asl.nlghtsrctb		= 0;		/* # Light src table	*/
  pCxt->asl.lghtsrctbo		= 0;		/* Offset to Lght src tb*/
  pCxt->asl.nclrproctbnrm	= 0;		/* # Color proc tbl	*/
  pCxt->asl.clrproctbonrm	= 0;		/* Offset to Clr proc tb*/
  pCxt->asl.nclrproctbech	= 0;		/* # Color Proc tbl	*/
  pCxt->asl.clrproctboech	= 0;		/* Offset to Clr Proc tb*/
  pCxt->asl.nlogclrtb		= 0;		/* # Logical color tbl	*/
  pCxt->asl.logclrtbo		= 0;		/* Offset to Log clr tb */ 
  pCxt->asl. pkcorcntlblk	= 0;		/* Offset to Pick Correl*/
  pCxt->asl.pkinvhlghtfl	= 0;		/* Offset to Pick, Invis*/
						/* and Hghlgh Filter tb */
  pCxt->asl.hatwkao		= 0;		/* offset to hwa	*/

  pCxt->asl.trsurwkao	= 0;			/* Offset to Trim Surf	*/
  pCxt->asl.shpwkao	= 0;			/* Offset to Shp Wk Area*/
  pCxt->asl.rsvd1	= 0;			/* Rsvd for Image Cd Sup*/
  pCxt->asl.rsvd2	= 0;			/* Rsvd for Image Cd Sup*/
  pCxt->asl.rsvd3	= 0;			/* Rsvd for Image Cd Sup*/
  pCxt->asl.langlktbo	= 0;			/* Offset to Language Lk*/   
  pCxt->asl.nalang	= 0;			/* National Language(Def*/
  pCxt->asl.nechvwtb	= 0;			/* # Echo view table	*/
  pCxt->asl.echvwtbo	= 0;			/* Offset to Echo vw tb */

  pCxt->asl.chngind	= 0;			/* change indicators	*/
  pCxt->asl.clrtblbg	= 0;			/* clr tbl begin change */ 
  pCxt->asl.clrtblend	= 0;			/* clr table end change */

  pCxt->asl.lntblchngbm		= 0;		/* line tbl change bits */
  pCxt->asl.pattblchngbm	= 0;		/* pat tbl change bits	*/
  pCxt->asl.hattblchngbm	= 0;		/* hat tbl change bits	*/

						/* Overlay Plane Masks	*/
  pCxt->asl.ofrmbufmsk	= 0;			/* ovrl plne frm bfr msk*/
  pCxt->asl.ocmpmsk	= 0;			/* ovrl plne compare msk*/
  pCxt->asl.ocmpval	= 0;			/* ovrl plne compare val*/

  pCxt->asl.dispmsk	= GMOverlayDisplayMask;	/* display mask		*/

  pCxt->asl.ologop	= 0;			/* ovrl plne logical op	*/

  pCxt->asl.woanntxtratio	= 0;
  pCxt->asl.vwtbsgmdo		= 0;	/* Offset to View table for   @A*/
  pCxt->asl.lwdcwcrat		= itof(1);	/* line wid wind ratio	*/
  pCxt->asl.prminfo		= VME_ADR(&gmcxt->rminfo); /* offset to dsv*/
  pCxt->asl.gtxtculdspmth	= 1;		/* cull display method	*/
  pCxt->asl.gtxtculhgt		= 0;		/* culling height	*/


					/*------------------------------*/
					/* VT  initialization		*/
					/* (view table)			*/
					/*------------------------------*/
  pCxt->vt.vwtindx	= 1;			/* View Table Index	*/
  pCxt->vt.vopflg	= 0x000E;		/* View Operation Flag	*/

  pCxt->vt.vtm[0]	= itof(1);		/* View Trans Matrix	*/
  pCxt->vt.vtm[1]	= 0;			/*    device coords -	*/
  pCxt->vt.vtm[2]	= 0;			/*	 upper left org */
  pCxt->vt.vtm[3]	= 0;			/*			*/
  pCxt->vt.vtm[4]	= 0;			/*			*/
  pCxt->vt.vtm[5]	= itof(-1);		/*			*/
  pCxt->vt.vtm[6]	= 0;			/*			*/
  pCxt->vt.vtm[7]	= 0;			/*			*/
  pCxt->vt.vtm[8]	= 0;			/*			*/
  pCxt->vt.vtm[9]	= 0;			/*			*/
  pCxt->vt.vtm[10]	= itof(1);		/*			*/
  pCxt->vt.vtm[11]	= 0;			/*			*/
  pCxt->vt.vtm[12]	= 0;			/*			*/
  pCxt->vt.vtm[13]	= itof(1023);		/*			*/
  pCxt->vt.vtm[14]	= 0;			/*			*/
  pCxt->vt.vtm[15]	= itof(1);		/*			*/

  pCxt->vt.prp[0]	= 0;			/* Proj Ref Pt (x)	*/
  pCxt->vt.prp[1]	= 0;			/* Proj Ref Pt (y)	*/
  pCxt->vt.prp[2]	= itof(1);		/* Proj Ref Pt (z)	*/

  pCxt->vt.vpd		= 0;			/* View Plane Distance	*/

  pCxt->vt.truvp[0]	= 0;			/* True Vwport (pixel)	*/
  pCxt->vt.truvp[1]	= itof(400*1279);	/* True Vwport (pixel)	*/
  pCxt->vt.truvp[2]	= 0;			/* True Vwport (pixel)	*/
  pCxt->vt.truvp[3]	= itof(400*1023);	/* True Vwport (pixel)	*/
  pCxt->vt.truvp[4]	= 0;			/* True Vwport (pixel)	*/
  pCxt->vt.truvp[5]	= 0;			/* True Vwport (pixel)	*/
  pCxt->vt.truw2vr[0]	= itof(1);		/* True WtoVwport Ratio */
  pCxt->vt.truw2vr[1]	= itof(1);		/* True WtoVwport Ratio */
  pCxt->vt.truw2vr[2]	= itof(1);		/* True WtoVwport Ratio */
  pCxt->vt.truiw2vr[0]	= itof(1);		/* True Invrse WtoVwport*/
  pCxt->vt.truiw2vr[1]	= itof(1);		/* True Invrse WtoVwport*/
  pCxt->vt.truiw2vr[2]	= itof(1);		/* True Invrse WtoVwport*/
  pCxt->vt.truwvc[0]	= 0;			/* True Window in VC	*/
  pCxt->vt.truwvc[1]	= itof(400*1279);	/* True Window in VC	*/
  pCxt->vt.truwvc[2]	= 0;			/* True Window in VC	*/
  pCxt->vt.truwvc[3]	= itof(400*1023);	/* True Window in VC	*/
  pCxt->vt.truwvc[4]	= 0;			/* True Window in VC	*/
  pCxt->vt.truwvc[5]	= itof(1);		/* True Window in VC	*/
  pCxt->vt.npczclip[0]	= 0;			/* Z clip bound in NPC	*/
  pCxt->vt.npczclip[1]	= itof(1);		/* Z clip bound in NPC	*/
  pCxt->vt.shearparm[0]	= itof(1279);		/* Shearing Parameters	*/
  pCxt->vt.shearparm[1]	= itof(1023);		/* Shearing Parameters	*/
  pCxt->vt.shearparm[2]	= itof(-1);		/* Shearing Parameters	*/

					/*------------------------------*/
					/* View Address Table		*/
					/*	initialization		*/
					/*------------------------------*/

  pCxt->vat[0]		= VME_ADR(&gmcxt->vt);	/* offset to vt		*/

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);

  GMBASE_INIT(seg_reg, pDevP);

  pDevP->pPrivCxt = ((GM_MMAP *)pDevP->gmbase)->rcm_priv_cxt;
  memcpy(pDevP->pPrivCxt, pCxt, sizeof(TRAV_CONTEXT));
  rFree(pCxt);

  BUSMEM_DET(seg_reg);

  /*
   * Set up RCM's private DSV by setting a default DSV, sending down
   * a set interior style SE to set the interior style to solid, then
   * doing a store DSV to where we want the DSV.
   */
  pACbuf = (ac_se *)pBuf;
  pACbuf->len = sizeof(ac_se);
  pACbuf->opcode = CE_ACTC;
  pACbuf->adr = VME_ADR(&((TRAV_CONTEXT *)pDevP->pPrivCxt)->asl);
  pBuf += sizeof(ac_se);
  pl = (ulong *)pBuf;
  *pl = 0x00040000 | CE_SDDS;
  pBuf += sizeof(ulong);
  pSE = (generic_se *)pBuf;
  pSE->len = sizeof(generic_se);
  pSE->opcode = SE_IS;
  pSE->data = GM_SOLID;
  pBuf += sizeof(generic_se);
  pVme = (generic_vme *)pBuf;
  pVme->len = sizeof(generic_vme);
  pVme->opcode = CE_SDSV;
  pVme->adr = VME_ADR(gmcxt+1);
  pBuf += sizeof(generic_vme);

#ifdef GEM_DBUG
  printf(" gem_ircx: writing to Trav Se Fifo - buffer=0x%x len=0x%x\n",
	 se_buffer, pBuf - se_buffer);
#endif

  seg_reg = BUSMEM_ATT(BUS_ID, 0x00);
  WTFIFO(TravSeFifo, se_buffer, pBuf - se_buffer, seg_reg, pDevP);
  /*  Wait for fifo to drain... X may use DSV.  */
  FIFO_EMPTY(TravSeFifo, seg_reg, pDevP);
  BUSMEM_DET(seg_reg);

#ifdef GEM_DBUG
  printf(" Done initializing context\n");
#endif

#ifdef GEM_DBUG
  printf("   Exit init_rcm_cxt\n");
#endif

}
