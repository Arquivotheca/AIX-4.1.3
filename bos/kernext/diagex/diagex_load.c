static char sccsid[] = "@(#)93	1.9  src/bos/kernext/diagex/diagex_load.c, sysxdiag, bos41J, 9513A_all 3/17/95 15:17:46";
/*
 *
 * COMPONENT_NAME: sysxdiag
 *
 * FUNCTIONS: diag_open, diag_close, diag_dma_master, diag_dma_slave,
 *            diag_read_handle, diag_cleanup
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
 /*
 *
 * This kernel extension allows a diagnostic application program to
 * exercise hardware.
 *
 * The kernel extension is divided into 2 modules:
 *   diagex_load.c 
 *            code that may be loaded and not pinned
 *            (although it currently is pinned)
 *   diagex_pin.c 
 *            code that must be loaded and pinned
 *
 */

/* header files needed for compilation */
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/sleep.h>   /* for EVENT_NULL (e_sleep) */
#include <sys/diagex.h>
#include "diagex_dgx.h"

/*******************************************************************
* Global Variables
*******************************************************************/
diag_cntl_t diag_cntl = { LOCK_AVAIL, LOCK_AVAIL, NULL };


/******************************************************************************
*
* NAME:  diag_open
*
* FUNCTION:  Get memory for this instance and Initialize a DMA channel
*            set up the adapters interrupt handler.
*            And pin diag extensions interrupt routines.
*
* INPUT PARAMETERS:     dds_ptr = pointer to the adapters dds.
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: if successful returns 0, handle set to valid handle 
*                           DGX_BOUND_FAIL,	errno=<not set>
*                           DGX_BADVAL_FAIL,	errno=<not set>
*                           DGX_KMOD_FAIL,	errno=kmod_entrypt rc
*                           DGX_IINIT_FAIL,	errno=i_init rc
*                           DGX_DINIT_FAIL,	errno=d_init rc
*                           DGX_COPYDDS_FAIL,	errno=copyin rc
*                           DGX_PINCODE_FAIL,	errno=pincode rc
*                           DGX_PINU_FAIL,	errno=pinu rc
*                           DGX_XMATTACH_FAIL,	errno=xmattch rc
*                           DGX_XMALLOC_FAIL,	errno=xmalloc rc
*
* EXTERNAL PROCEDURES CALLED: pincode, pinu, copyin, copyout kmod_entrypt,
*                             d_init, i_init, xmalloc, xmattach
*                             lockl, unlockl, suword
*                             diag_cleanup calls:
*                                 unpincode, unpinu,
*                                 d_clear, i_clear, xmfree, xmdetach
*
******************************************************************************/
int
diag_open (diagex_dds_t *dds_ptr, diag_struc_t **usershandle)
{
    
    int rc,i;
    int errcode=DGX_OK;
    diag_struc_t *handle=NULL; /* pointer to new handle */
    diagex_dds_t tmp_dds;
    int dma_info_size=0;  /* size of the dma_info[] */
    DGX_OPEN_ROUTINE;

    /*----------------------------------------------------------------*
    * Do Control Setups 
    *----------------------------------------------------------------*/
    /* prevent simultaneous opens or closes from using same resouces  */
    /* by locking an open control                                     */
    /* second+ processes will go to sleep until 1st process exits open() */
    if (diag_cntl.openlock != LOCK_AVAIL) {
       eMSG1(routine,"About to sleep...waiting for lock");
    }
    lockl( (lock_t *)&diag_cntl.openlock, LOCK_SHORT);

    /*
     * Use atomic operation to make sure that the initialization is only
     * done for the first device.
     */
    if (diag_cntl.top_handle == NULL) {
        diag_cntl.trace.next_entry = 4;
	strncpy(((char *)(&diag_cntl.trace.table[0])), "dgxtraceTOP!!!!",16);
	strncpy(((char *)(&diag_cntl.trace.table[4])), "dgxtraceCUR!!!!",16);
	strncpy(((char *)(&diag_cntl.trace.table[DGX_TRACE_SIZE-4])),
						       "dgxtraceBOT!!!!",16);
    	/* log beginning of external trace */
	TRACE_DBG(DGX_OTHER, "BEG1", 0, 0, 0);
    }
    TRACE_DBG(DGX_OTHER, "OpnB", (ulong)dds_ptr, *(ulong *)usershandle, 0);

    /* copy dds passed into diag structure */
    rc = copyin(dds_ptr,&tmp_dds,sizeof(diagex_dds_t));
    if (rc) {
       TRACE_BOTH(DGX_OTHER,"Opn1",rc,0,0);
       eMSGn1(routine,"Can't copyin DDS, copyin rc",rc);
       SETERR(DGX_COPYDDS_FAIL,rc);
       unlockl(&diag_cntl.openlock);
       return(errcode);
    }

    /*----------------------------------------------------------------
    * Detect Gross errors before resources are allocated/initialized
    *----------------------------------------------------------------*/

    /* set usershandle to NULL incase of error, test usershandle for validity */
    rc=suword(usershandle, NULL);
    if (rc) {
       TRACE_BOTH(DGX_OTHER,"Opn2",rc,0,0);
       eMSGn1(routine,"Invalid Handle Pointer",*usershandle);
       SETERR(DGX_INVALID_HANDLE,rc);
       unlockl(&diag_cntl.openlock);
       return(errcode);
    }

    if (tmp_dds.bus_type == BUS_MICRO_CHANNEL) {
       /* bus length is allowed to be a multiple of PAGESIZE or 0 */
       rc = (tmp_dds.dma_bus_length>>(PGSHIFT))<<(PGSHIFT);
       if (tmp_dds.dma_bus_length != rc) {
          TRACE_BOTH(DGX_OTHER,"Opn3",rc,tmp_dds.dma_bus_length,0);
          eMSGn1(routine,"DMA bus length Error",tmp_dds.dma_bus_length);
          unlockl(&diag_cntl.openlock);
          return(DGX_BOUND_FAIL);
       }
    } else
    if ((tmp_dds.bus_type != BUS_60X) && (tmp_dds.bus_type != BUS_BID) ) 
    {
       /* only bus_types BUS_60X,BUS_BID, and BUS_MICRO_CHANNEL are valid */
       TRACE_BOTH(DGX_OTHER,"Opn4",tmp_dds.bus_type,BUS_60X,BUS_MICRO_CHANNEL);
       eMSGn3(routine,"Illegal dds.bus_type",tmp_dds.bus_type,
                     "Try BUS_MICRO_CHANNEL",BUS_MICRO_CHANNEL,
                     "or  BUS_60X          ",BUS_60X          );
       unlockl(&diag_cntl.openlock);
       return(DGX_BADVAL_FAIL);
     }  /* end if (bus_type) */

    do { /* once, breaking on error */
   
       /*----------------------------------------------------------------
       * Ensure there is enough memory for the opeation
       *----------------------------------------------------------------*/
       /* get diag structure (handle) memory */
       handle=(diag_struc_t *)xmalloc(sizeof(diag_struc_t),UWSHIFT,pinned_heap);
       if (handle == NULL) {
          eMSGn2(routine,"Can't xmalloc Hndl",handle,"size",sizeof(diag_struc_t));
          SETERR(DGX_XMALLOC_FAIL,handle);
          break; /* from do...while (FALSE) */
       } else {

          /* Add handle to linked list of Valid handles */
          add_handle(handle);

          /* init Allocation/Initialization/OutstandingOperations flags */
          /* and clear out rest of handle, NULLing all pointers         */
          bzero((char *)(handle), sizeof(diag_struc_t));

       } /* end else (initialize new handle structure) */

       /* Ensure that if dma_bus_length = 0, maxmaster =0 */
       if (tmp_dds.bus_type == BUS_MICRO_CHANNEL) {
          if (tmp_dds.dma_bus_length == 0) {
             tmp_dds.maxmaster=0;
          } 
       } /* end if (bus_type) */

       /* If maxmaster =0, don't malloc memory to manage DMAs */
       /* (i.e. no slave DMAs allowed either                  */

       if (tmp_dds.maxmaster != 0) {
          /* dma_info[] refers to the active DMAs list */
          /* the (+1) will supply a space for dma_slave info */
          dma_info_size = (tmp_dds.maxmaster+1)*sizeof(dma_info_t);

          /* get DMA structure memory */
          handle->dma_info=xmalloc(dma_info_size, UWSHIFT, pinned_heap);
          if (handle->dma_info == NULL){
             eMSGn2(routine,"Can't xmalloc dma struct",handle->dma_info,
                    "size", dma_info_size);
             SETERR(DGX_XMALLOC_FAIL,handle->dma_info);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.AllocDmaAreaMem = TRUE;
             /* Clear all dma_info availability flags (in_use=FALSE) */
             bzero((char *)(handle->dma_info), dma_info_size);
          }
       } /* end if (dma_len AND maxmaster != 0) */

       /* get Interrupt Data memory */
       if (tmp_dds.kmid) { /* interrupt function specified */
          handle->intr_data = xmalloc(tmp_dds.d_count, UWSHIFT,pinned_heap);
          if (handle->intr_data == NULL) {
             eMSGn2(routine,"Can't xmalloc intrpt data area",handle->intr_data,
                   "size",tmp_dds.d_count);
             SETERR(DGX_XMALLOC_FAIL,handle->intr_data);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.AllocIntrptDataMem = TRUE;
          }
       }
       /* get PIO Scratch Pad memory ( see diag_rw() ) */
       handle->scratch_pad=xmalloc((DGX_SCRPADSZ),(PGSHIFT),pinned_heap);
       if (handle->scratch_pad == NULL) {
             eMSG1(routine,"Scratch Pad not allocated");
             SETERR(DGX_XMALLOC_FAIL,handle->scratch_pad);
             break; /* from do...while (FALSE) */
       }


       /*----------------------------------------------------------------
       * Initialize/Utilize the Successfully Allocated Memory
       *----------------------------------------------------------------*/

       /* copy dds passed into diag structure */
       rc = copyin(dds_ptr,&handle->dds,sizeof(diagex_dds_t));
       if (rc) {
          eMSGn1(routine,"Can't copyin DDS, copyin rc",rc);
          SETERR(DGX_COPYDDS_FAIL,rc);
          break; /* from do...while (FALSE) */
       } else {
          handle->aioo.CopyDDS = TRUE;
          handle->dds.maxmaster=tmp_dds.maxmaster; /*may have been set to zero*/
       }
   
       /* pin DIAGEX routines */
       rc = pincode(diag_intr);
       if (rc) {
          eMSGn1(routine,"Can't pin Diag extentions, pincode rc",rc);
          SETERR(DGX_PINCODE_FAIL,rc);
          break; /* from do...while (FALSE) */
       } else {
          handle->aioo.PinDiagExt = TRUE;
       }
   
       if (handle->dds.kmid) { /* interrupt function specified */

          handle->intr_func = (int (*)())kmod_entrypt(handle->dds.kmid,0);
          if (NULL == handle->intr_func) {
             eMSGn1(routine,"Can't find Intrpt Entry Pt, kmod rc",
                                              (handle->intr_func));
             SETERR(DGX_KMOD_FAIL,handle->intr_func);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.SetIntrptEntPt = TRUE;
          }
   
          /* pin the interrrupt handler */
          rc = pincode(handle->intr_func);
          if (rc) {
             eMSG1(routine,"Can't pin intrpt funct");
             SETERR(DGX_PINCODE_FAIL,rc);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.PinIntrptFunct = TRUE;
          }

          /* pin the user's interrupt data area */
          rc=pinu(handle->dds.data_ptr,handle->dds.d_count, UIO_USERSPACE);
          if (rc) {
             eMSGn1(routine,"Can't pin user interrupt data area, pinu rc",rc);
             SETERR(DGX_PINU_FAIL,rc);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.PinUIntrptData = TRUE;
          }

          /* cross mem attach the user's intr data area for use in the kernel */
          handle->udata_dp.aspace_id = XMEM_INVAL;
          rc=xmattach(handle->dds.data_ptr, handle->dds.d_count,
                     &handle->udata_dp,     USER_ADSPACE);
          if (rc != XMEM_SUCC) {
             eMSGn1(routine,"Can't attach interrupt data area, xmat rc", rc);
             SETERR(DGX_XMATTACH_FAIL,rc);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.XmatUIntrptData = TRUE;
          }

          /* initialize this interrupt e_sleep/e_wake word and other flags */
          handle->sleep_word = EVENT_NULL;
          handle->sleep_flag = FALSE;/* set by watch4intr,tested by IntrHndlr */
          handle->flag_word  = FALSE;/* set by IntrHndlr, tested by watch4intr*/

          /* initialize interrupt structure */
          handle->intr.handler = diag_intr;
          handle->intr.bus_type = handle->dds.bus_type;
          handle->intr.flags = handle->dds.intr_flags;
          handle->intr.level = handle->dds.bus_intr_lvl;
          handle->intr.priority = handle->dds.intr_priority;
          handle->intr.bid = handle->dds.bus_id;
 	  if(handle->dds.bus_type == BUS_BID)
 		handle->intr.bid = BID_VAL(IO_PCI, PCI_IOMEM, 
 				BID_NUM(handle->dds.bus_id));

          /* initialize this interrupt channel */
          rc=i_init(&handle->intr);
          if (INTR_SUCC != rc) {
             eMSGn1(routine,"Can't Init Interrupt Chan, iinit rc",rc);
             SETERR(DGX_IINIT_FAIL,rc);
             break; /* from do...while (FALSE) */
          } else {
             handle->aioo.InitIntrptChan = TRUE;
          }
   
       } /* endif there is an interrupt function supplied */

       if (handle->dds.bus_type == BUS_MICRO_CHANNEL) {
          if (handle->dds.maxmaster != 0) {
             /* Initialize the dma channel for its use */
             handle->dds.dma_chan_id=d_init(handle->dds.dma_lvl,
                                       handle->dds.dma_flags,
                                       (handle->dds.bus_id+DGX_BYPASS));
             if (handle->dds.dma_chan_id == DMA_FAIL) {
                eMSGn3(routine,
                       "Can't Init DMA Chan, d_init rc",handle->dds.dma_chan_id,
                       "handle->dds.dma_bus_length",handle->dds.dma_bus_length,
                       "handle->dds.dma_lvl",handle->dds.dma_lvl);
                SETERR(DGX_DINIT_FAIL,handle->dds.dma_chan_id);
                   break; /* from do...while (FALSE) */
             } else {
                /* the user will need this value for POS reg programming */
                dds_ptr->dma_chan_id = handle->dds.dma_chan_id;
                handle->aioo.InitDmaChan = TRUE;
             }
          } /* end if (dma maxmaster != 0) */
       } /* end if (BUS_MICRO_CHANNEL) */
       else if (handle->dds.bus_type == BUS_60X) {
          /* not used on BUS_60X adapters */
          handle->dds.dma_chan_id = DGX_UNSET;
       }
       else {
  
	/* This is to do a DMA initialization.  The dma_bus_mem variable
	 * should contain any bus specific flag information.
	 */
          if (handle->dds.maxmaster != 0) 
	  {
	        handle->dhandle = (struct d_handle *) D_MAP_INIT(
			BID_VAL(IO_PCI, PCI_IOMEM, BID_NUM(handle->dds.bus_id)),
					handle->dds.dma_flags,
					handle->dds.dma_bus_mem,
					handle->dds.dma_chan_id);
             	 if ((int) handle->dhandle == DMA_FAIL) 
		 {
                	eMSGn3(routine,
                       "Can't Init dhandle , d_map_init\
		       rc",handle->dhandle,
                       "handle->dds.dma_bus_length",handle->dds.dma_bus_length,
                       "handle->dds.dma_lvl",handle->dds.dma_lvl);
                	SETERR(DGX_DINIT_FAIL,handle->dds.dma_chan_id);
                   	break; /* from do...while (FALSE) */
			
		}
		/* set this so we know to run the d_map_clear in cleanup */
                handle->aioo.InitDmaChan = TRUE;
          } /* end if (dma maxmaster != 0) */
	}
       rc=suword((int *)usershandle, (int)handle);
       if (rc) {
          eMSGn1(routine,"Invalid Handle Pointer",*usershandle);
          SETERR(DGX_INVALID_HANDLE,rc);
          break;
       }
       rc=copyout(&handle->dds, dds_ptr, sizeof(int));
       if (rc) {
          eMSGn1(routine,"Can't copyout DDS, copyout rc",rc);
          SETERR(DGX_COPYDDS_FAIL,rc);
          break;
       }
    } while (FALSE); /* done with open setup */

    /*----------------------------------------------------------------*
    * Cleanup Errors
    *----------------------------------------------------------------*/
    if (errcode != DGX_OK) {
       eMSGn1(routine,"Unsuccessful Open, cleaning Up",errcode);
       diag_cleanup(&handle);
    }

    /*----------------------------------------------------------------
    * Do Control Setups 
    *----------------------------------------------------------------*/
    /* Allow other opens access to available resources   */
    /* second+ processes should now wake up from sleep   */
    unlockl(&diag_cntl.openlock);

    /*----------------------------------------------------------------
    * Return
    *----------------------------------------------------------------*/
    TRACE_DBG(DGX_OTHER, "OpnE", (ulong)dds_ptr, *(ulong *)usershandle, errcode);
    return(errcode);

} /* end diag_open() */


/******************************************************************************
*
* NAME:  diag_close
*
* FUNCTION:  Free an adapter handle and dma channel
*
* INPUT PARAMETERS:     handle = adapter handle
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: 0 if successful,		errno=<not set>
*                           DGX_INVALID_HANDLE,		errno=<not set>
*                           DGX_OUTSTANDINGDMA_FAIL,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: lockl, unlockl
*                             diag_cleanup calls:
*                                 unpincode, unpinu,
*                                 d_clear, i_clear, xmfree, xmdetach
*
******************************************************************************/
int
diag_close(diag_struc_t *handle )
{
   int errcode=DGX_OK;
   DGX_CLOSE_ROUTINE;

   /*----------------------------------------------------------------
   * Do Control Setups 
   *----------------------------------------------------------------
   * prevent removing handle list while another process
   * in the diag_open() routine (and my have alrady checked to ensure they 
   * exist), by getting the open control lock
   * second+ processes will go to sleep until 1st process releases lock */
   if (diag_cntl.openlock != LOCK_AVAIL) {
      eMSG1(routine,"About to sleep...waiting for lock");
   }
   lockl( (lock_t *)&diag_cntl.openlock, LOCK_SHORT);
   TRACE_DBG(DGX_OTHER, "ClsB", (ulong)handle, 0, 0);

   errcode = diag_cleanup(&handle);

   /*----------------------------------------------------------------
   * Do Control Setups 
   *----------------------------------------------------------------*/
   /* Allow other opens/closes access to available resources   */
   /* second+ processes should now wake up from sleep   */
   TRACE_DBG(DGX_OTHER, "ClsE", (ulong)handle, errcode, 0);
   unlockl(&diag_cntl.openlock);

   return(errcode);

} /* end diag_close() */

/******************************************************************************
*
* NAME:  diag_dma_master
*
* FUNCTION:  Initialize DMA transfer for a master.  Attach to the
*            user buffer with a cross-memory descriptor and pin it.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*                       dma_flags = DMA transfer direction see dma.h
*                       baddr = address of user memory buffer
*                       count = number of bytes to transfer
*                       users_daddr= Pointer to daddr (daddr to be set here)
*
* EXECUTION ENVIRONMENT:  Process only
*
* RETURN VALUE DESCRIPTION: 0 - if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*                           DGX_BOUND_FAIL,	errno=<not set>
*                           DGX_PINU_FAIL,	errno=pinu() return code
*                           DGX_XMATTACH_FAIL,	errno=xmattch() return code
*
* EXTERNAL PROCEDURES CALLED: pinu, xmattach, d_master, xmemdma, suword
*                             lockl, unlockl
*
******************************************************************************/
int
diag_dma_master
  (diag_struc_t *handle,int dma_flags,caddr_t baddr,int count,int *users_daddr)
{
   int i,offset,rc,daddr=DGX_UNSET;
   int errcode=DGX_OK;
   int dma_index;  /* index into dma_info[] */
   struct xmem dp; 
   int prev_priority;
   DGX_MASTER_ROUTINE;

   TRACE_DBG(DGX_OTHER, "DmaB", dma_flags, (ulong) baddr, count);
    rc=suword((int *)users_daddr, (int)DGX_UNSET);
    if (rc) {
       eMSGn1(routine,"Invalid daddr Pointer",users_daddr);
       SETERR(DGX_BADVAL_FAIL,rc);
       TRACE_BOTH(DGX_OTHER, "Dma1", errcode, 0, 0);
       return(errcode);
    }
   
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "Dma2", (ulong)handle, DGX_INVALID_HANDLE, 0);
      return(DGX_INVALID_HANDLE);
   }

   if (handle->dds.bus_type == BUS_MICRO_CHANNEL) {
      i = handle->dds.dma_bus_length; /* maximum DMA size */
   } else /* if (bus_type == BUS_60X) */ {
      i = PAGESIZE;                   /* maximum DMA size */
   }
   offset = (int)baddr & 0xFFF;
   /* disallow DMA if count too big, count 0, or maxmaster = 0 */
   if (((offset+count) > i) || (count < 1) || (handle->dds.maxmaster==0)) {
      eMSGn5(routine,
         "No count or DMA Buffer smaller than count for handle",handle,
         "DMA transfer byte count",count,
         "DMA Buffer size",handle->dds.dma_bus_length,
         "DMA Buffer Offset",offset,
         "Maximum #of DMAs",handle->dds.maxmaster);
      TRACE_BOTH(DGX_OTHER, "Dma3", offset, count, i);
      TRACE_BOTH(DGX_OTHER, "Dma+", handle->dds.maxmaster,
				   handle->dds.dma_bus_length,DGX_BOUND_FAIL);
      return(DGX_BOUND_FAIL);
   } /* end if bad DMA size */

   /*----------------------------------------------------------------
   * Find a Valid (free) DMA buffer in handle's DMA area
   *----------------------------------------------------------------*/
   if (diag_cntl.dmaindexlock != LOCK_AVAIL) {
      eMSG1(routine,"About to sleep...waiting for lock");
   }
   lockl( (lock_t *)&diag_cntl.dmaindexlock, LOCK_SHORT);
   rc = get_dma_index(handle,count+offset, &dma_index);
   unlockl(&diag_cntl.dmaindexlock);

   if (rc) {
      eMSG1(routine, "Can't Get DMA Index");
      TRACE_BOTH(DGX_OTHER, "Dma4", count, offset, rc);
      return(rc);
   }

   /*----------------------------------------------------------------
   * Prepare Memory for DMA
   *----------------------------------------------------------------*/
   if (handle->dds.bus_type == BUS_MICRO_CHANNEL) {
      /* set page offset into DMA bus memory */
      daddr  = handle->dma_info[dma_index].firsttcw;
      daddr += ((int)baddr & 0xFFF);        /* Add the offset into the PAGE */
   } /* end if (bus_type) */

   rc=pinu(baddr, count, UIO_USERSPACE);
   if (rc) {
      eMSGn1(routine,"Couldn't Pin DMA Buffer: size ",count);
      SETERR(DGX_PINU_FAIL,rc);
      free_dma_index(handle,dma_index);
      TRACE_BOTH(DGX_OTHER, "Dma5", rc, errcode, 0);
      return (errcode);
   } else {
      handle->dma_info[dma_index].pinned = TRUE;
      handle->dma_info[dma_index].count=count;
      handle->dma_info[dma_index].baddr=baddr;
   }

   handle->dma_info[dma_index].dp.aspace_id = XMEM_INVAL;
   rc=xmattach(baddr,count,&handle->dma_info[dma_index].dp, USER_ADSPACE);
   if (rc != XMEM_SUCC) {
      eMSGn1(routine,"Couldn't CrossMemAttach DMA Buffer: size ",count);
      SETERR(DGX_XMATTACH_FAIL,rc);
      diag_dma_complete(handle, (int) handle->dma_info[dma_index].daddr);/* cleanup */
      TRACE_BOTH(DGX_OTHER, "Dma6", rc, errcode, 0);
      return(errcode);
   } else {
      handle->dma_info[dma_index].xmattached = TRUE;
   }

   if (handle->dds.bus_type == BUS_MICRO_CHANNEL) {

      d_master(handle->dds.dma_chan_id, dma_flags, baddr, count,
                &handle->dma_info[dma_index].dp, (uchar *)daddr);

   } else {

      /*----------------------------------------------------------------
      * Get the physical address of baddr (daddr)
      * Prepare a single page for DMA
      *----------------------------------------------------------------*/
      /* always do access checking on the page */
      /* always XMEM_HIDE the page */
      /*   This prepares the page for DMA. (XMEM_UNHIDE is for post DMA) */
      /*   On the 60X bus, this does not really hide the page            */
      /*   so there will be no xmemdma(XMEM_UNHIDE) in diag_complete()   */
      dma_flags = XMEM_HIDE | XMEM_ACC_CHK;

      daddr = xmemdma(&handle->dma_info[dma_index].dp, baddr, dma_flags);
      if (daddr == XMEM_FAIL) {
         eMSG1(routine,"Couldn't CrossMemDMA");
         SETERR(DGX_FAIL,daddr);
         diag_dma_complete(handle,(int) handle->dma_info[dma_index].daddr);/*cleanup*/
         TRACE_BOTH(DGX_OTHER, "Dma7", daddr, errcode, 0);
         return (errcode);
      }

   } /* end if (bus_type) */

   /* save d_master/xmemdma info for future use */
   handle->dma_info[dma_index].daddr = (uchar *) daddr;
   handle->dma_info[dma_index].dma_flags = dma_flags;

   /* make physical address available so adapter may be programmed */
   rc=suword((int *)users_daddr, (int)daddr);
   if (rc) {
      eMSGn1(routine,"Invalid daddr Pointer",users_daddr);
      SETERR(DGX_BADVAL_FAIL,rc);
      diag_dma_complete(handle, daddr); /* clean up */
      TRACE_BOTH(DGX_OTHER, "Dma8", daddr, errcode, 0);
      return(errcode);
   }

   TRACE_DBG(DGX_OTHER, "DmaE", *users_daddr, errcode, 0);
   return (errcode);

} /* end diag_dma_master() */

/******************************************************************************
*
* NAME:  diag_dma_slave
*
* FUNCTION:  Initialize DMA transfer for a slave.  Attach to the
*            user buffer with a cross-memory descriptor and pin it.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*                       dma_flags = DMA transfer direction see dma.h
*                       baddr = address of user memory buffer
*                       count = number of bytes to transfer
*
* EXECUTION ENVIRONMENT:  Process only
*
* RETURN VALUE DESCRIPTION: 0 if successful,	errcode=<not set>
*                           DGX_INVALID_HANDLE,	errcode=<not set>
*                           DGX_BOUND_FAIL,	errno=<not set>
*                           DGX_PINU_FAIL,	errcode=pinu return code
*                           DGX_XMATTACH_FAIL,	errcode=xmattch return code
*
* EXTERNAL PROCEDURES CALLED: pinu, xmattach, d_slave
*                             lockl, unlockl
*
******************************************************************************/
int
diag_dma_slave(diag_struc_t *handle,int dma_flags,caddr_t baddr,int count)
{
   int i,rc;
   int prev_priority;
   int errcode=DGX_OK;
   DGX_SLAVE_ROUTINE;

   TRACE_DBG(DGX_OTHER, "SlvB", dma_flags, (ulong) baddr, count);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER,"Slv1",(ulong)handle,0,0);
      return(DGX_INVALID_HANDLE);
   }
   if (handle->dds.bus_type != BUS_MICRO_CHANNEL) {
      eMSGn1(routine,"Slave DMA not supported on non Micro Channel Devices",
             handle->dds.bus_type);
      TRACE_BOTH(DGX_OTHER,"Slv2",handle->dds.bus_type,0,0);
      return(DGX_BADVAL_FAIL);
   } /* end if (bus_type) */
   if (count <= 0) {
      eMSGn1(routine,"Invalid count", count);
      TRACE_BOTH(DGX_OTHER,"Slv3",count,0,0);
      return(DGX_BOUND_FAIL);
   } /* end if (count <=0) */

   /*----------------------------------------------------------------
   * diag_open() created a handle->dma_info[] array with 1 more element than
   * there are dma_bus_memory pages.  This extra array element (the first
   * element) will contain the d_slave() data.                         
   *----------------------------------------------------------------*/

   if (diag_cntl.dmaindexlock != LOCK_AVAIL) {
      eMSG1(routine,"About to sleep...waiting for lock");
   }
   lockl( (lock_t *)&diag_cntl.dmaindexlock, LOCK_SHORT);

   if (handle->dma_info[DGX_SLAVE_PG].in_use == TRUE) {
      eMSGn2(routine,"Slave DMA member is not Free, RC",DGX_BOUND_FAIL,
             "handle->dds.dma_bus_length", handle->dds.dma_bus_length);
      unlockl(&diag_cntl.dmaindexlock);
      TRACE_BOTH(DGX_OTHER,"Slv4",handle->dds.dma_bus_length,0,0);
      return(DGX_BOUND_FAIL);
   }
   else
   {
      handle->dma_info[DGX_SLAVE_PG].in_use = TRUE;
      handle->dma_info[DGX_SLAVE_PG].daddr=(uchar *)DGX_UNSET; /*tested in dma_complete*/
   }

   unlockl(&diag_cntl.dmaindexlock);

   rc=pinu(baddr, count, UIO_USERSPACE);
   if (rc) {
      eMSGn1(routine,"Couldn't Pin DMA Buffer: size ",count);
      handle->dma_info[DGX_SLAVE_PG].in_use = FALSE;
      SETERR(DGX_PINU_FAIL,rc);
      TRACE_BOTH(DGX_OTHER,"Slv5",rc,count,0);
      return (errcode);
   } else {
      handle->dma_info[DGX_SLAVE_PG].pinned = TRUE;
      handle->dma_info[DGX_SLAVE_PG].count=count;
      handle->dma_info[DGX_SLAVE_PG].baddr=baddr;
   }

   handle->dma_info[DGX_SLAVE_PG].dp.aspace_id = XMEM_INVAL;
   rc=xmattach(baddr,count,&handle->dma_info[DGX_SLAVE_PG].dp, USER_ADSPACE);
   if (rc != XMEM_SUCC) {
      eMSGn1(routine,"Couldn't xmattach DMA Buffer: size ",count);
      SETERR(DGX_XMATTACH_FAIL,rc);
      diag_dma_complete(handle, 0); /* clean up */
      TRACE_BOTH(DGX_OTHER,"Slv6",rc,count,0);
      return (errcode);
   } else {
      handle->dma_info[DGX_SLAVE_PG].xmattached = TRUE;
   }

   d_slave(handle->dds.dma_chan_id,dma_flags,baddr,count,
           &handle->dma_info[DGX_SLAVE_PG].dp);
  
   /* save d_slave info for future use */
   handle->dma_info[DGX_SLAVE_PG].in_use = TRUE;
   handle->dma_info[DGX_SLAVE_PG].daddr=0;
   handle->dma_info[DGX_SLAVE_PG].dma_flags=dma_flags;
   handle->dma_info[DGX_SLAVE_PG].firsttcw = DGX_UNSET;
   handle->dma_info[DGX_SLAVE_PG].last_tcw = DGX_UNSET;

   TRACE_DBG(DGX_OTHER, "SlvE", errcode,0,0);
   return (errcode);

} /* end diag_dma_slave() */

/******************************************************************************
*
* NAME:  diag_read_handle
*
* FUNCTION:  Returns interrupt handler control structure
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: 0 if copyout successful
*
* EXTERNAL PROCEDURES CALLED: copyout
*
******************************************************************************/
int
diag_read_handle(diag_struc_t *handle,diag_struc_t *hdlr)
{
    int rc;

   TRACE_DBG(DGX_OTHER, "RdhB", (ulong) handle,0,0);
   rc=copyout(handle,hdlr, sizeof(diag_struc_t));
   if (rc) {
      setuerror(rc);
      TRACE_BOTH(DGX_OTHER, "Rdh1", rc,DGX_FAIL,0);
      return(DGX_FAIL);
   }
   TRACE_DBG(DGX_OTHER, "RdhE", (ulong)handle,0,0);
   return(DGX_OK);
}


/******************************************************************************
*
* NAME:  diag_cleanup
*
* FUNCTION:  Free an adapter handle and its associated resources.
*            This routine allows both open and close to clean up without
*            interferring with each other's locking mechanisms.
*
* INPUT PARAMETERS:     handle = adapter handle
*
* EXECUTION ENVIRONMENT:  Process only.
*
* RETURN VALUE DESCRIPTION: 0 if successful,		errno=<not set>
*                           DGX_INVALID_HANDLE,		errno=<not set>
*                           DGX_OUTSTANDINGDMA_FAIL,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: unpincode, unpinu,
*                             d_clear, i_clear, xmfree, xmdetach
*
******************************************************************************/
int
diag_cleanup(diag_struc_t **diag_handle )
{
   int rc;
   int dma_index;
   diag_struc_t *handle;
   int errcode=DGX_OK;
   DGX_CLEAN_ROUTINE;

    handle = (diag_struc_t *) *diag_handle;

   TRACE_DBG(DGX_OTHER, "ClnB", (ulong)handle,0,0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "Cln1", (ulong)handle,0,0);
      return(DGX_INVALID_HANDLE);
   }
   /*----------------------------------------------------------------
   * Clean Up Outstanding Operations
   *   (DMA operations are not tracked for 60X bus devices)
   *----------------------------------------------------------------*/
   if (handle->dma_info) {
      for (dma_index=0;
           dma_index <= handle->dds.maxmaster;
           dma_index++) {
         if (handle->dma_info[dma_index].in_use) {
             /* Application should diag_dma_complete before diag_close()ing */
             /* Failing Closure due to potential DMA Transaction in Progress */
            eMSGn2(routine,
             "dma_index",dma_index,
             "in_use", handle->dma_info[dma_index].in_use); 
            errcode = (DGX_OUTSTANDINGDMA_FAIL);
         } /* end if (in_use) */
      } /* end for(dma_index) */
      if (errcode) {
         TRACE_BOTH(DGX_OTHER, "Cln2", errcode,0,0);
         return(errcode);
      }
   }

   /*----------------------------------------------------------------
   * Ensure the DMA channel is Free
   *   (associated flags are 'cleared' when handle is removed)
   *----------------------------------------------------------------*/
   if (handle->aioo.InitDmaChan) {
      if(handle->dds.bus_type == BUS_MICRO_CHANNEL)
      	d_clear(handle->dds.dma_chan_id);
      else
	D_MAP_CLEAR(handle->dhandle);

      if(handle->dio_st->dioinit)
      {
	DIO_FREE(handle->dio_st->vlist);
	DIO_FREE(handle->dio_st->blist);
      }
   }

   /*----------------------------------------------------------------
   * Free up the interrupt handler, if needed
   *   (associated flags are 'cleared' when handle is removed)
   *----------------------------------------------------------------*/
   if (handle->aioo.SetIntrptEntPt) {

      if (handle->aioo.InitIntrptChan) {
         i_clear(&handle->intr);
      }

      if (handle->aioo.PinIntrptFunct) {
         unpincode(handle->intr_func);
      }

      if (handle->aioo.XmatUIntrptData) {
         xmdetach(&handle->udata_dp);
      }

      if (handle->aioo.PinUIntrptData) {
         unpinu(handle->dds.data_ptr,handle->dds.d_count, UIO_USERSPACE);
      }

   } /* end if an interrupt function was loaded */

   /* unpin the diagex kernel extention */
   if (handle->aioo.PinDiagExt) {
      unpincode(diag_intr);
   }

   /*----------------------------------------------------------------
   * Free Allocated Memory
   *   (associated flags are 'cleared' when handle is removed)
   *----------------------------------------------------------------*/
   if (handle->aioo.AllocDmaAreaMem) {
      xmfree(handle->dma_info,pinned_heap);
   }

   if (handle->aioo.AllocIntrptDataMem) {
      xmfree(handle->intr_data,pinned_heap);
   }

   /* Get rid of PIO scratch pad (since no one is using it)            */
   if (handle->scratch_pad != NULL) {
      xmfree(handle->scratch_pad,pinned_heap);
   }

   /* free up the handle for this adapter */
   remove_handle(handle);
   xmfree(handle,pinned_heap);
   handle=NULL;
   *diag_handle=handle;

   TRACE_DBG(DGX_OTHER, "ClnE", (ulong) handle,0,0);
   return(errcode);

} /* end diag_cleanup() */

/******************************************************************************
*
* NAME:  get_dma_index
*
* FUNCTION:  
*        Search the 'in_use' dma_info[] members
*           These are in a linked list starting at the start of the dma_info[] 
*        BUS_MICRO_CHANNEL adapters:                                           
*        As 'in_use' members are found, the first and last TCWs used are noted 
*            in order to calculate the 'tcw' gap between each pair of dma_info 
*            members (micro channel only).                                     
*            If a sufficent 'gap'is found AND a NOT 'in_use' member is available
*            the NOT 'in_use' member is added to the inner-array linked list
*            in the 'gap'.
*            PGSHIFT is used to force partial pages to take up a whole page.   
*        BUS_60X adapters need only find a NOT 'in_use' member and add it to
*            the inner-array linked list.
*            
*            
*
* INPUT PARAMETERS:     handle = adapter handle
*                       count  = size of DMA   
*                       dma_index
*                              = set by this routine to available member index
*
* EXECUTION ENVIRONMENT:  Process
*
* RETURN VALUE DESCRIPTION: 0 if successful,		errno=<not set>
*                           DGX_BOUND_FAIL		errno=<not set>
*                           DGX_FAIL			errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: none
*                             
*
******************************************************************************/
int
get_dma_index(diag_struc_t *handle, int count, int *dma_index)
{
   int          gap_start;
   int          gap_end  ;
   int          index;
   dma_info_t  *curr_p=NULL;
   dma_info_t  *prev_p=NULL;
   DGX_GETINDX_ROUTINE;

   TRACE_DBG(DGX_OTHER, "GdiB", (ulong) handle,count,(ulong) dma_index);
   /* Search the 'in_use' dma_info[] member list */
   if ( handle->dma_info == NULL) {
      eMSG1(routine,"Can't Parse NULL dma_info list!!");
      TRACE_BOTH(DGX_OTHER, "Gdi1", (ulong) handle,count,DGX_FAIL);
      return(DGX_FAIL);
   }

   if (count <= 0) {
      eMSGn1(routine,"Invalid Count",count);
      TRACE_BOTH(DGX_OTHER, "Gdi2", (ulong)handle,count,DGX_FAIL);
      return(DGX_BOUND_FAIL);
   }

   /* the DGX_SLAVE_PG's next always points to the top of the in_use list */
   curr_p = handle->dma_info[DGX_SLAVE_PG].next;
   prev_p = &handle->dma_info[DGX_SLAVE_PG];

   if (handle->dds.bus_type == BUS_MICRO_CHANNEL) {

      /* initialize the 'gap' to be the entire dma_bus_length */
      gap_start   = handle->dds.dma_bus_mem;
      gap_end     = handle->dds.dma_bus_length + gap_start;
      gap_start >>= (PGSHIFT);
      gap_end   >>= (PGSHIFT);

      while (curr_p) {      /* while in the 'in_use' list */
         gap_end  = curr_p->firsttcw >> (PGSHIFT);

         if (((gap_end - gap_start) << PGSHIFT)  >= count) {
            /* the 'gap' is big enough for the DMA size of 'count' */
            /* prev_p points to member before the 'gap' */
            /* curr_p points to member after  the 'gap' */
            break;
         } /* end if (gap is big enough) */

         gap_start  = curr_p->last_tcw >> (PGSHIFT);
         gap_start += 1; /* puts us on the page AFTER the one being used */

         prev_p = curr_p;
         curr_p = curr_p->next;
      } /* end while (curr_p) <while in the 'in_use' list> */

      /* Here because :                                              */
      /*  A)  found 'gap' between prev_p and curr_p                  */
      /*  B)  end of 'in_use' list, curr_p is NULL                   */
      /*                            prev_p is at next to last member */

      if (curr_p && prev_p) {
         /*  A)  found 'gap' between prev_p and curr_p           */
         /* continue on......                                    */
      } /* endif found 'gap' */
      else
      {
         /*  B)  end of 'in_use' list, curr_p is NULL            */
         /*  see if there is room after the last list entry's    */
         /*  gap_start (already set)                             */
         gap_end = handle->dds.dma_bus_mem + handle->dds.dma_bus_length - 1;
         gap_end >>= (PGSHIFT);
         if (((gap_end - gap_start) << PGSHIFT)  < count) {
            /* (gap is NOT big enough) */
            *dma_index=DGX_UNSET;
            eMSGn5(routine, "gap is too small at end of TCW list",0,
                            "gap_end",gap_end, "gap_start",gap_start,
                            "gap_delta",((gap_end - gap_start) << PGSHIFT),
                            "count",count);
            TRACE_BOTH(DGX_OTHER, "Gdi3", gap_end,gap_start,PGSHIFT);
            return(DGX_BOUND_FAIL);
         } /* end if (gap is NOT big enough) */
         else {
            /* found 'gap' between prev_p and end of TCWs */
            /* already set up prev_p and curr_p like in case (A)  */
            /* prev_p = pointer to member with highest TCW used */
            /* curr_p = NULL;                                   */
         }
      } /* end if (end of in_use list, with no gap found) */

      /* Cases A,B are set up as if we found a TCW 'gap' between */
      /* prev_p and curr_p.  Now we just need a dma_info[] member  */
      /* to insert between prev_p and curr_p                       */

      /* search for a NOT 'in_use' dma_info[] member */
      /* (not including the slave member, index=maxmaster) */
      for (index=DGX_SLAVE_PG+1; index<=handle->dds.maxmaster; index++) {
         if (handle->dma_info[index].in_use == FALSE) break;
      } 
      if (index > handle->dds.maxmaster) {
         /* there are no NOT in_use dma_info[] members */
         *dma_index=DGX_UNSET;
         eMSGn4(routine, "there are no NOT in_use dma_info[] members",0,
                        "index",index,"in_use",handle->dma_info[index].in_use,
                        "maxmaster",handle->dds.maxmaster);
         TRACE_BOTH(DGX_OTHER,"Gdi4",index,handle->dds.maxmaster,0);
         return(DGX_BOUND_FAIL);
      } else {
        /* reset count to be full page multiple */
        count = (count % (PAGESIZE))
              ? (count >> (PGSHIFT)) + 1
              : (count >> (PGSHIFT));
        count <<= (PGSHIFT);
         /* index refers to a NOT in_use dma_info[] member */
         /* insert the dma_info[] member into the inner-array linked list */
         gap_start                      <<= PGSHIFT;
         prev_p->next                     = &handle->dma_info[index];
         handle->dma_info[index].next     = curr_p;
         handle->dma_info[index].firsttcw =gap_start;
         handle->dma_info[index].last_tcw =gap_start+count;
         handle->dma_info[index].daddr    = (char *)DGX_UNSET;/*tested in dma_complete*/
         handle->dma_info[index].in_use   = TRUE;
         *dma_index                       = index;
         TRACE_DBG(DGX_OTHER, "GdiE", gap_start,count,*dma_index);
         return(DGX_OK);
      }

   } /* end if (bus_type == BUS_MICRO_CHANNEL) */

   else /* if (bus_type == BUS_60X) */
   {
      while (curr_p) {      /* while in the 'in_use' list */
          /* get to the end of the list */
          prev_p = curr_p;
          curr_p = curr_p->next;
      } /* end while (curr_p) <while in the 'in_use' list> */

      /* prev_p = pointer to member with highest TCW used */
      /* curr_p = NULL;                                   */
      /* Now we just need a dma_info[] member  */
      /* to insert after prev_p                */

      /* search for a NOT 'in_use' dma_info[] member */
      /* (not including the slave member, index=maxmaster) */
      for (index=DGX_SLAVE_PG+1; index<=handle->dds.maxmaster; index++) {
         if (handle->dma_info[index].in_use == FALSE) break;
      } 
      if (index > handle->dds.maxmaster) {
         /* there are no NOT in_use dma_info[] members */
         *dma_index=DGX_UNSET;
         eMSGn4(routine, "there are no NOT in_use dma_info[] members",0,
                         "index",index,"in_use",handle->dma_info[index].in_use,
                          "maxmaster",handle->dds.maxmaster);
         TRACE_BOTH(DGX_OTHER,"Gdi5",handle->dds.maxmaster,index,DGX_BOUND_FAIL);
         return(DGX_BOUND_FAIL);
      } else {
         /* index refers to a NOT in_use dma_info[] member */
         /* insert the dma_info[] member into the inner-array linked list */
         prev_p->next                     = &handle->dma_info[index];
         handle->dma_info[index].next     = NULL;
         handle->dma_info[index].firsttcw = DGX_UNSET;
         handle->dma_info[index].last_tcw = DGX_UNSET;
         handle->dma_info[index].daddr    = (uchar *)DGX_UNSET;/*tested in dma_complete*/
         handle->dma_info[index].in_use   = TRUE;
         *dma_index                       = index;
         TRACE_BOTH(DGX_OTHER, "Gdie", (ulong) handle,count,*dma_index);
         return(DGX_OK);
      }
   } /* end if (bus_type == BUS_60X) */
} /* end get_dma_index() */

