static char sccsid[] = "@(#)94	1.13  src/bos/kernext/diagex/diagex_pin.c, sysxdiag, bos41J, 9514A_all 3/31/95 09:34:26";
/* 
 * COMPONENT_NAME: sysxdiag
 *
 * FUNCTIONS: diag_intr, diag_intr_enable, diag_intr_disable
 *            diag_dma_complete, diag_dma_unmask, diag_dma_mask, 
 *            diag_io_write, diag_io_read, diag_mem_write,
 *            diag_mem_read, diag_pos_write, diag_pos_read,
 *            diag_io_wr_stream, diag_io_rd_stream,
 *            diag_mem_wr_stream, diag_mem_rd_stream
 *            diag_rw, get_ioatt_ptr, diag_watch4intr
 *	      diag_special_io_read, diag_special_io_write
 * 	      iodet_ptr diag_map_disable diag_map_enable
 *            diag_unmap_page diag_map_page diag_map_slave
 *            get_ISA_buffer free_ISA_buffer
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * MODULE NAME: diagex_pin.c
 *
 * This module contains a "generic" interrupt handler.  The purpose of
 * this routine is to provide a simple, limited way to verify that a
 * device is properly generating interrupts.
 * This module also contains any functions which could be called at
 * interrupt time and therefore need to be pinned.
 * (although currently all diagex code is pinned)
 *
 * It is not practical to support every possible device interrupt
 * handler that might be desired; for that purpose, a different
 * routine should be written for each device.
 * This is supported by the intr_func pointer, which is a pointer
 * to an optional interrupt handler assist routine. which takes the
 *
 * The code in this (and all) modules is pinned by diag_open().
 */

/* header files needed for compilation */
#include <sys/adspace.h>
#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/ioacc.h>
#include <sys/sleep.h>   /* for EVENT_SHORT (e_sleep) */
#include <sys/syspest.h>   /* for ASSERT */
#include <stddef.h>   /* for offsetof() macro */
#include <sys/diagex.h>
#include "diagex_dgx.h"

/*******************************************************************/
/* Global Variables */
/*******************************************************************/
extern diag_cntl_t diag_cntl;

/******************************************************************************
*
* NAME:  diag_intr
*
* FUNCTION:  Handle interrupts
*
* INPUT PARAMETERS:     None ( interrupt structure )
*
* EXECUTION ENVIRONMENT:  Interrupt
*
* RETURN VALUE DESCRIPTION: INTR_SUCC,	errno=<not set>
*                           INTR_FAIL,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: curtime, i_reset, e_wakeup, optional intr funct.
*
******************************************************************************/
int
diag_intr(struct intr *intr_ptr)
{
   diag_struc_t *handle;
   int ic=INTR_SUCC; /* interrupt return code (INTR_FAIL/INTR_SUCC) */
   int rc=DGX_OK;   /* return code */
   DGX_INTR_ROUTINE;
   TRACE_BOTH(DGX_OTHER,"IntB",(ulong)intr_ptr, 0, 0);
   /* handle is the upper portion of the intr_ptr */
   handle = (diag_struc_t *) intr_ptr;

   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      /* can't get to application's interrupt handler */
      TRACE_BOTH(DGX_OTHER,"Int1", (ulong)handle, 0, 0);
      return(INTR_FAIL);
   }

   /* copy user data to kernel data */
   rc=xmemin(handle->dds.data_ptr,handle->intr_data,handle->dds.d_count,
              &handle->udata_dp);
   if (rc) {
      eMSGn1(routine,"xmemin() Failed, RC",rc);
      TRACE_BOTH(DGX_OTHER,"Int2", (ulong)handle->dds.d_count, rc, 0);
   }
  
   if (handle->dds.kmid) {
      /* if there is an interrupt routine to run, run it */
      ic =(*handle->intr_func)(handle,handle->intr_data);
      /* ensure the user only returns valid interrupt codes */
      ASSERT((ic==INTR_SUCC) || (ic==INTR_FAIL));

   } else {
      /* if there is no interrupt routine,                               */ 
      /* ...must assume this is not our interrupt                        */
      ic=INTR_FAIL;
      TRACE_DBG(DGX_OTHER,"Inta", ic, rc, 0);
   }
  
   /* copy user data to kernel data */
   rc=xmemout(handle->intr_data,handle->dds.data_ptr,handle->dds.d_count,
             &handle->udata_dp);
   if (rc) {
      TRACE_BOTH(DGX_OTHER,"Int4", ic, rc, 0);
      eMSGn1(routine,"xmemout() Failed, RC",rc);
   }

   if (ic == INTR_SUCC) {
      i_reset(&handle->intr);
   }

   TRACE_DBG(DGX_OTHER,"IntE", ic, rc, 0);
   return(ic);
} /* end diag_intr() */

/******************************************************************************
*
* NAME:  diag_watch4intr
*
* FUNCTION:  disables interrupts
*            verify interrupt has not already occured
*            sleeps if interrupt has not occurred, until interrupt has occurred 
*            enables interrupts
*
* INPUT PARAMETERS:     handle : pointer to diagex instance context
*			flag_mask : flag to compare to flag set by intr_funct 
*                       timeout_sec : # of seconds before timing out
*				       (zero or negative value disables timeout)
*
* EXECUTION ENVIRONMENT:  Process only (calls i_enable/i_disable)
*
* RETURN VALUE DESCRIPTION: 0 if succesful,	errno=<not set>
*                           DGX_FAIL if timeout,errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: e_sleep, i_disable, i_enable
*
******************************************************************************/
int
diag_watch4intr(diag_struc_t *handle, int flag_mask, int timeout_sec)
{
   int rc=DGX_OK;
   int prev_priority;
   DGX_WATCH4INTR_ROUTINE;

   TRACE_DBG(DGX_OTHER,"WdtB", (ulong)handle,flag_mask, timeout_sec);
   /*----------------------------------------------------------------*/
   /* Detect Gross Errors */
   /*----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER,"Wdt1",(ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   /* Set the Watch Dog Timer for 'timeout_sec' */
   /* initialize the watch dog timer for the watch4intr() routine */
   /* The wdt structure is malloced and pinned with handle */
   diag_cntl.timeout = FALSE;  /* watchdog timer has not yet expired */
   handle->wdt.next = NULL;    /* no next watchdog timer */
   handle->wdt.prev = NULL;    /* no previous watchdog timer */
   handle->wdt.func = (void (*)())diag_watchdog;
   handle->wdt.count= 0;               /* inactivate the timer */
   handle->wdt.restart = timeout_sec;    /* set the timer */
   w_init(&handle->wdt);

   /* prevent interrupt from setting flags while processing this */
   prev_priority = i_disable(handle->dds.intr_priority);

   /* Wait for the correct interrupt to occur by testing flag_word */
   /* The application's interrupt routine should set handle->flag_word */
   while ( (! ((diag_cntl.timeout) || (handle->flag_word & flag_mask)) ) &&
	   (rc == DGX_OK)) {

      /* Start the Watch Dog Timer */
      w_start(&handle->wdt);

      /* Tell application's interrupt handler that we are sleeping */
      /* and wait for the interrupt to occur */
      handle->sleep_flag = TRUE;
      rc = e_sleep((int *) &handle->sleep_word, EVENT_SIGRET);
      /* e_sleep takes care of i_enable  before sleeping */
      /* e_sleep takes care of i_disable after  sleeping */

      /* Stop the watch dog timer */
      w_stop(&handle->wdt);

      /* assign the return from e_sleep to rc if it returned b/c of a signal*/
      if(rc == EVENT_SIG)
	rc = DGX_EVENT_SIG;
      else
	rc = DGX_OK;


   } /* end while (have not yet received the  correct interrupt flag_word) */

   /* Tell application's interrupt handler that we are no longer sleeping */
   handle->sleep_flag = FALSE;

   /* check to see if we timed out or got a matching interrupt */
   if (diag_cntl.timeout) {
      diag_cntl.timeout = FALSE;
      rc = DGX_FAIL;
   } else {
      /* clear the desired set of flag_word bits */
      handle->flag_word &= ~flag_mask;
   }

   i_enable(prev_priority);
  
   /* clear the watchdog timer */
   w_clear(&handle->wdt);

   TRACE_DBG(DGX_OTHER,"WdtE", rc,flag_mask, timeout_sec);
   return(rc);
} /* end diag_watch4intr() */

/******************************************************************************
*
* NAME:  diag_watchdog
*
* FUNCTION:  Envoked when watchdog timer expires
*            Sets Global variable diag_cntl.timeout
*            Wakes up sleeping watch4intr() routine.
*
* INPUT PARAMETERS:
*	     None.
*
* EXECUTION ENVIRONMENT:  Process/Interrupt
*
* RETURN VALUE DESCRIPTION: None.
*
* EXTERNAL PROCEDURES CALLED: offsetof
*
******************************************************************************/
void
diag_watchdog(struct watchdog *wdt)
{
   diag_struc_t *handle;

   TRACE_DBG(DGX_OTHER,"Wd_B",(ulong)wdt,0,0);
   /*----------------------------------------------------------------*
    * wdt is part of the handle, so find the handle's address and the 
    * sleepword's address based on wdt's address.
    *----------------------------------------------------------------*/
   handle = (diag_struc_t *)((ulong)wdt - (offsetof(diag_struc_t, wdt)));

   /*----------------------------------------------------------------
   * Set Flag so watch4intr() knows we timed out
   * Wakeup watch4intr()
   *----------------------------------------------------------------*/
   diag_cntl.timeout = TRUE;
   TRACE_DBG(DGX_OTHER,"Wd_E",(ulong)wdt,0,0);
   e_wakeup((int *) &handle->sleep_word);

} /* end diag_watchdog() */

/******************************************************************************
*
* NAME:  diag_dma_complete
*
* FUNCTION:  Cleans up after DMA transfer.  Unpins the user buffer
*            and detaches the cross-memory descriptor.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*                       daddr = phyical address for DMA master
*                               which is returned from master and slave
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: 0 if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*                           DGX_BADVAL_FAIL,	errno=<not set>
*                           DGX_DCOMPLETE_FAIL,	errno=d_complete return code
*                           DGX_UNPINU_FAIL,	errno=unpinu return code
*                           DGX_XMDETACH_FAIL,	errno=xmdetach return code
*
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int
diag_dma_complete(diag_struc_t *handle, int daddr)
{
   int rc;                  /* return code from called routines */
   int errcode=DGX_OK;     /* return code from this routine */
   int daddroff;            /* daddr - TCW base  ->  TCWoffset */
   int dma_index;
   DGX_COMPLETE_ROUTINE;

   TRACE_DBG(DGX_OTHER,"DmcB",(ulong)handle,daddr,0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER,"Dmc1",(ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   if ((daddr) == 0) {
      /* slave operations return daddr = 0 */
      if ( ! handle->dma_info[DGX_SLAVE_PG].in_use) {
         eMSG1(routine,"Slave DMA not in_use");
         TRACE_BOTH(DGX_OTHER,"Dmc2",0, 0, 0);
         return(DGX_BADVAL_FAIL);
      }
      dma_index = DGX_SLAVE_PG;
   } else {
      /* get starting index into the handle->dma[] array */
      /* (where d_master/slave info is saved)            */
      rc=find_dma_index(handle,daddr,&dma_index);
      if (rc) {
         TRACE_BOTH(DGX_OTHER,"Dmc3",rc, dma_index, 0);
         return(rc);
      }
   } 

   /* used only when diag_dma_complete() called from diag_dma_slave/master */
   if ((int) handle->dma_info[dma_index].daddr != DGX_UNSET) {
      /* daddr was set, we need to d_complete */

      if (handle->dds.bus_type == BUS_MICRO_CHANNEL) {
         /*----------------------------------------------------------------
         * Appears that we've got valid buffer information, Clean up the DMA
         *----------------------------------------------------------------*/

         if ((int) handle->dma_info[dma_index].daddr  != DGX_UNSET) {
            rc=d_complete(handle->dds.dma_chan_id,
                          handle->dma_info[dma_index].dma_flags,
                          handle->dma_info[dma_index].baddr,
                          handle->dma_info[dma_index].count,
                          &handle->dma_info[dma_index].dp,
                          (char *) daddr);
            if (rc != DMA_SUCC) {
               eMSGn1(routine,"D_Complete Failed, rc",rc);
               SETERR(DGX_DCOMPLETE_FAIL,rc);
               TRACE_BOTH(DGX_OTHER,"Dmc4",rc,handle->dds.dma_chan_id,dma_index);
               /* Do NOT return, Continue to detach and unpin */
            }
         } else {
            /* dma_info[].daddr is 'unset' in get_dma_handle and 'set'      */
            /* in diag_dma_master-After the the d_master                    */
            /* if we didn't set dma[i].daddr, we never did d_master/d_slave */
            /* successfully.  We don't need to d_complete */
            /* but we may need to unpin or detach */
         }
      } /* end if (bus_type) */
   } /* end if daddr was set */

   /* Detach the buffer used */
   if ( handle->dma_info[dma_index].xmattached) {
      rc=xmdetach(&handle->dma_info[dma_index].dp);
      if (rc != XMEM_SUCC) {
         eMSGn1(routine,"Detach Failed, rc",rc);
         SETERR(DGX_XMDETACH_FAIL,rc);
         TRACE_BOTH(DGX_OTHER,"Dmc5",rc,dma_index,0);
         /* Do NOT return, Continue to mark-free and unpin */
      } else {
         handle->dma_info[dma_index].xmattached = FALSE;
      }
   }

   /* Unpin the buffer used */
   if ( handle->dma_info[dma_index].pinned) {
      rc=unpinu(handle->dma_info[dma_index].baddr,
		handle->dma_info[dma_index].count,
                UIO_USERSPACE);
      if (rc) {
         eMSGn1(routine,"Unpin Failed, rc",rc);
         SETERR(DGX_UNPINU_FAIL,rc);
         TRACE_BOTH(DGX_OTHER,"Dmc6",rc,dma_index,0);
         /* Do NOT return, Continue to mark-free */
      } else {
         handle->dma_info[dma_index].pinned = FALSE;
      }
   }

   /* set all TCWs refered to by dma[] used to Available (in_use=FALSE) */
   if (dma_index == DGX_SLAVE_PG) {
      handle->dma_info[dma_index].in_use = FALSE;
   } else {
      free_dma_index(handle,dma_index);
   }

  
   TRACE_DBG(DGX_OTHER,"DmcE",(ulong)handle,daddr,errcode);
   return(errcode);

} /* end diag_dma_complete() */

/******************************************************************************
*
* NAME:  diag_dma_flush
*
* FUNCTION:  Flushes the IO and data cache.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*                       daddr = offset address for DMA master
*                               which is returned from master and slave
*
* EXECUTION ENVIRONMENT:  Process or Interrupt.
*
* RETURN VALUE DESCRIPTION: 0 if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*                           DGX_BADVAL_FAIL,	errno=<not set>
*                           DGX_FAIL,		errno=d_blush/d_cflush RC
*
* EXTERNAL PROCEDURES CALLED: d_cflush, d_bflush()
*
******************************************************************************/
int
diag_dma_flush(diag_struc_t *handle, int daddr)
{
   int errcode=DGX_OK;
   int rc,i;
   int dma_index;
   DGX_FLUSH_ROUTINE;

   TRACE_DBG(DGX_OTHER, "DmfB", (ulong)handle, daddr, 0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER,"Dmf1",(ulong)handle,0,0);
      return(DGX_INVALID_HANDLE);
   }

   if (handle->dds.bus_type != BUS_MICRO_CHANNEL) {
      /* MCA machines are the only ones which aren't cache consistent */
      TRACE_DBG(DGX_OTHER, "Dmfe", (ulong)handle, daddr, errcode);
      return(DGX_OK);
   } /* end if (bus_type) */

   rc = find_dma_index(handle,daddr, &dma_index);
   if (rc) return(rc);

   /*----------------------------------------------------------------
   * Appears that we've got valid buffer information, Flush the cache/buffer
   *----------------------------------------------------------------*/
   rc=d_cflush(handle->dds.dma_chan_id, handle->dma_info[dma_index].baddr,
               handle->dma_info[dma_index].count,daddr);
   if (rc != DMA_SUCC) {
      eMSGn1(routine,"d_cflush() Failed, RC",rc);
      SETERR(DGX_FAIL, rc);
      TRACE_BOTH(DGX_OTHER,"Dmf2",rc,dma_index,0);
      return(errcode);
   }
   rc=d_bflush(handle->dds.dma_chan_id,handle->dds.bus_id & 0x0FF00000,daddr);
   if (rc != DMA_SUCC) {
      eMSGn1(routine,"d_bflush() Failed, RC",rc);
      SETERR(DGX_FAIL, rc);
      TRACE_BOTH(DGX_OTHER,"Dmf3",rc,handle->dds.bus_id & 0x0FF00000,
				handle->dds.dma_chan_id);
      return(errcode);
   }

   TRACE_DBG(DGX_OTHER, "DmfE", (ulong)handle, daddr, errcode);
   return(errcode);

} /* end diag_dma_flush() */

/******************************************************************************
*
* NAME:  diag_dma_unmask
*
* FUNCTION:  Enable a DMA channel
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: 0 - Successful Operation,	errno=<not set>
*                           DGX_INVALID_HANDLE,		errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: d_unmask
*
******************************************************************************/
int
diag_dma_unmask(diag_struc_t *handle)
{
   int errcode=DGX_OK;
   DGX_UNMASK_ROUTINE;

   TRACE_DBG(DGX_OTHER, "DmuB", (ulong)handle, 0, 0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "Dmu1", (ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   /*----------------------------------------------------------------
   * Unmask the DMA Channel
   *----------------------------------------------------------------*/
   d_unmask(handle->dds.dma_chan_id);

   TRACE_DBG(DGX_OTHER, "DmuE", (ulong)handle, errcode, 0);
   return(errcode);

} /* end diag_dma_unmask() */

/******************************************************************************
*
* NAME:  diag_dma_mask
*
* FUNCTION:  Disable a DMA channel
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: 0 - Successful Operation,	errno=<not set>
*                           DGX_INVALID_HANDLE,		errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: d_mask
*
******************************************************************************/
int
diag_dma_mask(diag_struc_t *handle)
{
   int errcode=DGX_OK;
   DGX_MASK_ROUTINE;

   TRACE_DBG(DGX_OTHER, "DmmB", (ulong)handle, 0, 0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "Dmm1", (ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
      return(errcode);
   }

   /*----------------------------------------------------------------
   * Mask the DMA Channel
   *----------------------------------------------------------------*/
   d_mask(handle->dds.dma_chan_id);

   TRACE_DBG(DGX_OTHER, "DmmE", (ulong)handle, errcode, 0);
   return(errcode);

} /* end diag_dma_mask() */

/******************************************************************************
*
* NAME:  diag_io_write
*
* FUNCTION:  Write to an I/O register.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       data = data value to write from
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_PUT(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_PUTCX,BUS_PUTSX, BUS_PUTLX)
*
******************************************************************************/
int
diag_io_write(diag_struc_t *handle,int type,int offset,
              ulong dataval, struct timestruc_t *times, int intlev)
{
   int rc;

   ulong *data=&dataval;/* this allows the user to pass data, not an address */
                        /* but it also uses a KernelSpaceBuffer(change intlev)*/
   DGX_IOWR_ROUTINE;
   /* shift the data into the correct position(char shifts 24,short 16,long 0)*/

   TRACE_DBG(DGX_OTHER, "IowB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Iow+", dataval, (ulong)times, intlev);
   *data = (*data << ((IOLONG-type)<<3));

   intlev <<= 16;     /* Keep orig intlev in UpperWord for times data struct  */
   intlev |= INTRKMEM;/* since we are using a local buffer (data) not dataval */

   TRACE_DBG(DGX_OTHER, "Iowa",* (ulong *) data, intlev, 0);
   rc = diag_rw(handle,           /* pass thru */
                DGX_WRITEOP,      /* specify a write operation */
                DGX_IO_OP,        /* specify an I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "IowE", rc, 0, 0);
   return(rc);
} /* end diag_io_write() */

/******************************************************************************
*
* NAME:  diag_io_read
*
* FUNCTION:  Read from an I/O register.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       data = data buffer to read into
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  process or interrupt
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_GET(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_GETCX,BUS_GETSX, BUS_GETLX)
*
******************************************************************************/
int
diag_io_read(diag_struc_t *handle,int type,int offset,
             void *data, struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_IORD_ROUTINE;

   TRACE_DBG(DGX_OTHER, "IorB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Ior+", *(ulong *)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_READOP,       /* specify a read operation */
                DGX_IO_OP,        /* specify an I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "IorE", rc, *(ulong *)data, 0);
   return rc;
} /* end diag_io_read() */

/******************************************************************************
*
* NAME:  diag_mem_write
*
* FUNCTION:  Write to a memory register.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       data = data value to write from
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  process or interrupt
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_PUT(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_PUTCX,BUS_PUTSX, BUS_PUTLX)
*
******************************************************************************/
int
diag_mem_write(diag_struc_t *handle,int type,int offset,
               ulong dataval, struct timestruc_t *times, int intlev)
{
   int rc;
   ulong *data=&dataval; /* this allows the user to pass data, not an address */
                        /* but it also uses a KernelSpaceBuffer(change intlev)*/
   DGX_MEMWR_ROUTINE;

   TRACE_DBG(DGX_OTHER, "MewB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Mew+", dataval, (ulong)times, intlev);
   /* shift the data into the correct position(char shifts 24,short 16,long 0)*/
   *data = (*data << ((IOLONG-type)<<3));

   intlev <<= 16;     /* Keep orig intlev in UpperWord for times data struct  */
   intlev |= INTRKMEM;/* since we are using a local buffer (data) not dataval */

   TRACE_DBG(DGX_OTHER, "Mewa", *(ulong *) data, intlev, 0);
   rc = diag_rw(handle,           /* pass thru */
                DGX_WRITEOP,      /* specify a write operation */
                DGX_MEM_OP,       /* specify a memory operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "MewE", rc, 0, 0);
   return(rc);
} /* end diag_mem_write() */

/******************************************************************************
*
* NAME:  diag_mem_read
*
* FUNCTION:  Read from a memory register.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       data = data buffer to read to
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process only or Interrupt.
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_GET(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_GETCX,BUS_GETSX, BUS_GETLX)
*
******************************************************************************/
int
diag_mem_read(diag_struc_t *handle,int type,int offset,
              void *data, struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_MEMRD_ROUTINE;

   TRACE_DBG(DGX_OTHER, "MerB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Mer+", *(ulong *)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_READOP,       /* specify a read operation */
                DGX_MEM_OP,       /* specify a memory operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "MerE", *(ulong *)data, intlev, 0);
   return(rc);
} /* end diag_mem_read() */

/******************************************************************************
*
* NAME:  diag_pos_write
*
* FUNCTION:  Write a byte to a pos register.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       offset = offset of register
*                       data = data value to write from
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process only or Interrupt.
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_PUTCX return code
*           DGX_BOUND_FAIL     - offset out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - BUS_60X trying access POS regs
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime, BUS_PUTCX)
*
******************************************************************************/
int
diag_pos_write(diag_struc_t *handle,int offset,
               char dataval, struct timestruc_t *times, int intlev)
{
   int rc;
   char *data=&dataval; /* this allows the user to pass data, not an address */
                        /* but it also uses a KernelSpaceBuffer(change intlev)*/
   DGX_POSWR_ROUTINE;

   TRACE_DBG(DGX_OTHER, "PowB", (ulong)handle, 0, offset);
   TRACE_DBG(DGX_OTHER, "Pow+", dataval, (ulong)times, intlev);
   intlev <<= 16;     /* Keep orig intlev in UpperWord for times data struct  */
   intlev |= INTRKMEM;/* since we are using a local buffer (data) not dataval */

   TRACE_DBG(DGX_OTHER, "Powa", (ulong) data, intlev, 0);
   rc = diag_rw(handle,           /* pass thru */
                DGX_WRITEOP,      /* specify a write operation */
                DGX_POS_OP,       /* specify a POS operation  */
                IOCHAR,           /* specify 8 bit access */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "PowE", rc, 0, 0);
   return (rc);
} /* end diag_pos_write() */

/******************************************************************************
*
* NAME:  diag_pos_read
*
* FUNCTION:  Read a byte from a POS register.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       offset = offset of register
*                       data = data buffer to read to
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process or Interrupt.
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_GETCX return code
*           DGX_BOUND_FAIL     - offset out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - BUS_60X trying access POS regs
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime, BUS_GETCX)
*
******************************************************************************/
int
diag_pos_read(diag_struc_t *handle,int offset,
              char *data, struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_POSRD_ROUTINE;

   TRACE_DBG(DGX_OTHER, "PorB", (ulong)handle, 0, offset);
   TRACE_DBG(DGX_OTHER, "Por+", *(ulong *)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_READOP,       /* specify a read operation */
                DGX_POS_OP,       /* specify a POS operation  */
                IOCHAR,           /* specify 8 bit access */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "PorE", rc, *(ulong *)data, 0);
   return(rc);
} /* end diag_pos_read() */
/******************************************************************************
*
* NAME:  diag_io_wr_stream
*
* FUNCTION:  Write to a (sequence of) I/O register(s).
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       count  = the number of accesses to perform
*                       addr_incr_flag = 
*                          DGX_SING_LOC_HW  for Single Location HW     Access
*                                               Multiple Location Buff Access
*                          DGX_SING_LOC_BUF for Single Location Buffer Access
*                                               Multiple Location HW   Access
*                          DGX_SING_LOC_ACC for Multiple Location Access
*                                               (Hardware AND 'data' Buffer)
*                          DGX_MULT_LOC_ACC for Single Location Access
*                                               (Hardware AND 'data' Buffer)
*                       data    = data buffer to write from
*                       times   = address of time structure, NULL if don't care
*                       intrlev = PROCLEV  if calling from user process level
*                                 INTRKMEM if calling from interrupt level
*                                          (data buffer in kernel memory)
*                                 INTRPMEM if calling from user process level
*                                          (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process or Interrupt (see intrlev parameter).
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_PUT(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset or count out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_PUTCX,BUS_PUTSX, BUS_GETLX)
*
******************************************************************************/
int
diag_io_wr_stream(diag_struc_t *handle,int type,int offset,
                   int count, int addr_incr_flag, char *data,
                   struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_IOWRST_ROUTINE;

   TRACE_DBG(DGX_OTHER, "IwsB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Iws+", count, addr_incr_flag, 0);
   TRACE_DBG(DGX_OTHER, "Iws+", (ulong)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_WRITEOP,      /* specify a write operation */
                DGX_IO_OP,        /* specify an I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                count,            /* pass thru */
                addr_incr_flag,       /* pass thru */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "IwsE", rc, 0, 0);
   return(rc);
} /* end diag_io_wr_stream() */
/******************************************************************************
*
* NAME:  diag_io_rd_stream
*
* FUNCTION:  Read from a (sequence of) I/O register(s).
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       count  = the number of accesses to perform
*                       addr_incr_flag = 
*                          DGX_SING_LOC_HW  for Single Location HW     Access
*                                               Multiple Location Buff Access
*                          DGX_SING_LOC_BUF for Single Location Buffer Access
*                                               Multiple Location HW   Access
*                          DGX_SING_LOC_ACC for Multiple Location Access
*                                               (Hardware AND 'data' Buffer)
*                          DGX_MULT_LOC_ACC for Single Location Access
*                                               (Hardware AND 'data' Buffer)
*                       data    = data buffer to read into
*                       times   = address of time structure, NULL if don't care
*                       intrlev = PROCLEV  if calling from user process level
*                                 INTRKMEM if calling from interrupt level
*                                          (data buffer in kernel memory)
*                                 INTRPMEM if calling from user process level
*                                          (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process or Interrupt (see intrlev parameter).
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_GET(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset or count out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_GETCX,BUS_GETSX, BUS_GETLX)
*
******************************************************************************/
int
diag_io_rd_stream(diag_struc_t *handle,int type,int offset,
                   int count, int addr_incr_flag, char *data,
                   struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_IOWRST_ROUTINE;

   TRACE_DBG(DGX_OTHER, "IrsB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Irs+", count, addr_incr_flag, 0);
   TRACE_DBG(DGX_OTHER, "Irs+", (ulong)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_READOP,       /* specify a read operation */
                DGX_IO_OP,        /* specify an I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                count,            /* pass thru */
                addr_incr_flag,       /* pass thru */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "IrsE", rc, 0, 0);
   return(rc);
} /* end diag_io_rd_stream() */
/******************************************************************************
*
* NAME:  diag_mem_wr_stream
*
* FUNCTION:  Write to a (sequence of) Memory register(s).
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       count  = the number of accesses to perform
*                       addr_incr_flag = 
*                          DGX_SING_LOC_HW  for Single Location HW     Access
*                                               Multiple Location Buff Access
*                          DGX_SING_LOC_BUF for Single Location Buffer Access
*                                               Multiple Location HW   Access
*                          DGX_SING_LOC_ACC for Multiple Location Access
*                                               (Hardware AND 'data' Buffer)
*                          DGX_MULT_LOC_ACC for Single Location Access
*                                               (Hardware AND 'data' Buffer)
*                       data    = data buffer to write from
*                       times   = address of time structure, NULL if don't care
*                       intrlev = PROCLEV  if calling from user process level
*                                 INTRKMEM if calling from interrupt level
*                                          (data buffer in kernel memory)
*                                 INTRPMEM if calling from user process level
*                                          (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process or Interrupt (see intrlev parameter).
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_PUT(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset or count out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_PUTCX,BUS_PUTSX, BUS_PUTLX)
*
******************************************************************************/
int
diag_mem_wr_stream(diag_struc_t *handle,int type,int offset,
                   int count, int addr_incr_flag, char *data,
                   struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_MEMRDST_ROUTINE;

   TRACE_DBG(DGX_OTHER, "MwsB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Mws+", count, addr_incr_flag, 0);
   TRACE_DBG(DGX_OTHER, "Mws+", (ulong)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_WRITEOP,      /* specify a write operation */
                DGX_MEM_OP,       /* specify an I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                count,            /* pass thru */
                addr_incr_flag,       /* pass thru */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "MwsE", rc, 0, 0);
   return(rc);
} /* end diag_mem_wr_stream() */
/******************************************************************************
*
* NAME:  diag_mem_rd_stream
*
* FUNCTION:  Read from a (sequence of) Memory register(s).
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       count  = the number of accesses to perform
*                       addr_incr_flag = 
*                          DGX_SING_LOC_HW  for Single Location HW     Access
*                                               Multiple Location Buff Access
*                          DGX_SING_LOC_BUF for Single Location Buffer Access
*                                               Multiple Location HW   Access
*                          DGX_SING_LOC_ACC for Multiple Location Access
*                                               (Hardware AND 'data' Buffer)
*                          DGX_MULT_LOC_ACC for Single Location Access
*                                               (Hardware AND 'data' Buffer)
*                       data    = data buffer to read into
*                       times   = address of time structure, NULL if don't care
*                       intrlev = PROCLEV  if calling from user process level
*                                 INTRKMEM if calling from interrupt level
*                                          (data buffer in kernel memory)
*                                 INTRPMEM if calling from user process level
*                                          (data buffer in handle->intr_data)
*
* EXECUTION ENVIRONMENT:  Process or Interrupt (see intrlev parameter).
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_GET(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset or count out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
* EXTERNAL PROCEDURES CALLED: diag_rw (calls io_att, vm_det, io_det, curtime,
*                                      BUS_GETCX,BUS_GETSX, BUS_GETLX)
*
******************************************************************************/
int
diag_mem_rd_stream(diag_struc_t *handle,int type,int offset,
                   int count, int addr_incr_flag, char *data,
                   struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_MEMRDST_ROUTINE;

   TRACE_DBG(DGX_OTHER, "MrsB", (ulong)handle, type, offset);
   TRACE_DBG(DGX_OTHER, "Mrs+", count, addr_incr_flag, 0);
   TRACE_DBG(DGX_OTHER, "Mrs+", * (ulong *)data, (ulong)times, intlev);
   rc = diag_rw(handle,           /* pass thru */
                DGX_READOP,       /* specify a read operation */
                DGX_MEM_OP,       /* specify an I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                count,            /* pass thru */
                addr_incr_flag,       /* pass thru */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   TRACE_DBG(DGX_OTHER, "MrsE", rc, 0, 0);
   return(rc);
} /* end diag_mem_rd_stream() */
/******************************************************************************
*
* NAME:  add_handle
*
* FUNCTION:  Returns valid handle pointer or NULL if no valid handle is found
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  process or interrupt
*
* RETURN VALUE DESCRIPTION: Valid handle pointer if valid handle is found
*                           errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: None
*
******************************************************************************/

void
add_handle (diag_struc_t *handle)
{
   diag_struc_t *cur_handle;
   DGX_ADDHANDLE_ROUTINE;

    TRACE_DBG(DGX_OTHER, "ah_B", (ulong)handle, (ulong)diag_cntl.top_handle, 0);
    if (diag_cntl.top_handle == NULL) {
       /* start the list with this handle */
       diag_cntl.top_handle = handle;
    } else {
       /* add the handle to the end of a pre-existing list */
       cur_handle=diag_cntl.top_handle;
       while (cur_handle->next) cur_handle = cur_handle->next;
       cur_handle->next = handle;
    } 
    handle->next = NULL;
    handle->dma_info = NULL;
    handle->scratch_pad = NULL;
    handle->intr_data = NULL;

    TRACE_DBG(DGX_OTHER, "ah_E", (ulong)handle, (ulong)cur_handle, 0);

} /* end add_handle */

/******************************************************************************
*
* NAME:  find_handle
*
* FUNCTION:  Returns valid handle pointer or NULL if no valid handle is found
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  process or interrupt
*
* RETURN VALUE DESCRIPTION: Valid handle pointer if valid handle is found
*                           NULL if no valid handle is found
*                           errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: None
*
******************************************************************************/

diag_struc_t *find_handle (diag_struc_t *handle) {
   diag_struc_t *cur_handle;
   DGX_FINDHANDLE_ROUTINE;

    TRACE_DBG(DGX_OTHER, "fh_B", (ulong)handle, (ulong)diag_cntl.top_handle, 0);
    cur_handle=diag_cntl.top_handle;
    while (cur_handle) {
       if (handle == cur_handle) break;
       cur_handle = cur_handle->next;
    } /* end while  cur_handle */

    TRACE_DBG(DGX_OTHER, "fh_E", (ulong)handle, (ulong)cur_handle, 0);
    return(cur_handle);
} /* end find_handle */

/******************************************************************************
*
* NAME:  remove_handle
*
* FUNCTION:  Removes handle from the linked list. Does not free the memory.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  process or interrupt
*
* RETURN VALUE DESCRIPTION: Valid handle pointer if valid handle is found
*                           DGX_LISTFAIL if no valid handle is found
*                           errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: None
*
******************************************************************************/

int
remove_handle (diag_struc_t *handle)
{
   diag_struc_t *cur_handle;
   diag_struc_t *prv_handle;
   DGX_RMHANDLE_ROUTINE;


    TRACE_DBG(DGX_OTHER, "rh_B", (ulong)handle, (ulong)diag_cntl.top_handle, 0);
    if (diag_cntl.top_handle == NULL) {
       eMSGn1(routine,"Can't Remove Handle from an empty List",handle);
       TRACE_BOTH(DGX_OTHER, "rh_1", (ulong)diag_cntl.top_handle, 0, 0);
       return(DGX_LISTFAIL);
    } else {
       /* add the handle to the end of a pre-existing list */
       cur_handle=diag_cntl.top_handle;
       prv_handle=diag_cntl.top_handle;
       while (cur_handle) {
          if (handle == cur_handle) break;
          prv_handle = cur_handle;
          cur_handle = cur_handle->next;
       } /* end while  cur_handle */
       if ( cur_handle == NULL) {
          eMSGn1(routine,"Can't Find Handle in List",handle);
          TRACE_BOTH(DGX_OTHER, "rh_2", (ulong)diag_cntl.top_handle, 0,
					(ulong)prv_handle);
          return(DGX_LISTFAIL);
       } else {
          if (cur_handle == diag_cntl.top_handle) {
             diag_cntl.top_handle = cur_handle->next;
          } else {
             prv_handle->next = cur_handle->next;
          }
       }
    }

    TRACE_DBG(DGX_OTHER, "rh_E", (ulong)prv_handle, (ulong)cur_handle, 0);
    return(DGX_OK);
} /* end remove_handle */

/******************************************************************************
*
* NAME:  diag_rw
*
* FUNCTION:  Perform PIO accesses for all Diagex PIO functions
*
* INPUT PARAMETERS: handle = adapter handle
*                   wr_op= DGX_WRITEOP, DGX_READOP
*                   memio= DGX_MEM_OP, DGX_IO_OP, DGX_POS_OP, DGX_SPECIAL_IO_OP
*                   type = IOCHAR, IOSHORT , IOLONG
*                   offset = offset of register
*                   count= number of accesses to perform
*                   addr_incr_flag =
*                          DGX_SING_LOC_HW  for Single Location HW     Access
*                                               Multiple Location Buff Access
*                          DGX_SING_LOC_BUF for Single Location Buffer Access
*                                               Multiple Location HW   Access
*                          DGX_SING_LOC_ACC for Multiple Location Access
*                                               (Hardware AND 'data' Buffer)
*                          DGX_MULT_LOC_ACC for Single Location Access
*                                               (Hardware AND 'data' Buffer)
*                   data    = pointer to buffer of source/destination data
*                   times   = NULL for no timing info, timestruc otherwise
*                   intlev  = PROCLEV, INTRPMEM, INTRKMEM
*
* EXECUTION ENVIRONMENT:  Process or Interrupt.
*
* RETURN VALUE DESCRIPTION:
*           DGX_OK             - successful operation
*				 errno=<not set>
*           DGX_INVALID_HANDLE - Illegal handle or DDS 
*				 errno=<not set>
*           DGX_FAIL           - Unable to Read/Write Hardware
*				 errno=BUS_(GET/PUT)(L/S/C)X return code
*           DGX_BOUND_FAIL     - offset or count out of range
*                                of buffer or HW address range.
*                              - specified data buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*                              - specified times buffer not in the 
*                                interrupt buffer range specified
*                                in 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_BADVAL_FAIL    - Illegal 'type'
*                              - BUS_60X trying access POS regs
*				 errno=<not set>
*           DGX_XMATTACH_FAIL  - Interrupt buffer not XMATTACHED
*                              - by 'diag_open()' (INTRPMEM only)
*				 errno=<not set>
*           DGX_COPY_FAIL      - Unable to Copy User to/from Kernel Buffer
*				 errno=copy(in/out) or xmem(in/out) return code
*
******************************************************************************/
ulong
diag_rw(diag_struc_t *handle,int wr_op, int memio, int type,int offset,
        int count, int addr_incr_flag, void *data, struct timestruc_t *times,
        int intrlev)
{
   int errcode=DGX_OK;
   int  hwincrval = 0;   /* amount to increment hardware(addr_incr_flag dependent)*/
   int  bfincrval = 0;   /* amount to increment buffer  (addr_incr_flag dependent)*/
   int extent;           /* extent of access (count*size-1) */
   int num_dumps=1;      /* >1 if scratch pad buffer not big enough for 1xfer*/
   int cp_count=0;       /* >1 if read/writing mult times from/to 'data' buff*/
   struct timestruc_t deltatime;
   int timesintrlev;
   uchar *ptr;
   int prev_priority=-1;
   int rc=DGX_OK,i;
   DGX_RW_ROUTINE;
   

   TRACE_BOTH(DGX_OTHER, "rw_B", (ulong)handle, wr_op, memio);
   TRACE_BOTH(DGX_OTHER, "rw_+", type, offset, count);
   TRACE_BOTH(DGX_OTHER, "rw_+", addr_incr_flag, *(ulong *)data, intrlev);
   /* for single write routines                                         */
   /*    (i.e. diag_io_write, diag_mem_write, diag_pos_write),          */
   /*    the intrlev word is actually two shorts                        */
   /*    the most significant short is the intrlev for the times struct */
   /*        (the calling routine environment level)                    */
   /*    the least significant short is the intrlev for the data        */
   /*        (diagex uses its own INTRKMEM because it copies the        */
   /*          application's data to it's own variable)                 */
   /* for multiple write routines and all read routines,                */
   /*    (i.e. diag_*_read, diag_*_stream),                             */
   /*    the most significant short is 0,                               */
   /*     so the least significant short is copied to it                */
   timesintrlev = intrlev >> 16;
   intrlev      = intrlev &  0xFFFF;
   if (! timesintrlev) timesintrlev = intrlev;

   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/

   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      TRACE_BOTH(DGX_OTHER, "rw_1", (ulong)handle, 0, 0);
      eMSGn1(routine,"Unable to find handle",handle);
      return(DGX_INVALID_HANDLE);
   }

   /* Ensure handle->dds is valid */
   if ( ! handle->aioo.CopyDDS) {
      TRACE_BOTH(DGX_OTHER, "rw_2", (ulong)handle, 0, 0);
      eMSGn1(routine,"DDS not initialized in handle",handle);
      return(DGX_INVALID_HANDLE);
   }

   /* Ensure type is valid, initialize extent,bf/hwincrval */
   switch (type) {
      case IOCHAR  : /* type == 1 */
      case IOSHORT : /* type == 2 */
      case IOLONG  : /* type == 4 */
           extent = count*type;
           switch (addr_incr_flag) {
           case DGX_SING_LOC_ACC:
              bfincrval = 0; /* Never increment the pointer to the 'data' buff*/
              hwincrval = 0; /* Never increment the pointer to the hardware  */
              break;
           case DGX_SING_LOC_BUF:
              bfincrval = 0; /* Never increment the pointer to the 'data' buff*/
              hwincrval = 1; /* Always increment the pointer to the hardware */
              break;
           case DGX_SING_LOC_HW :
              bfincrval = 1; /* Always increment the pointer to the 'data' buf*/
              hwincrval = 0; /* Never increment the pointer to the hardware  */
              break;
           case DGX_MULT_LOC_ACC:
              bfincrval = 1; /* Always increment the pointer to the 'data' buf*/
              hwincrval = 1; /* Always increment the pointer to the hardware */
              break;
           default : eMSGn1(routine,"Invalid 'addr_incr_flag'",addr_incr_flag);
              TRACE_BOTH(DGX_OTHER, "rw_3", extent, type, addr_incr_flag);
              return(DGX_BADVAL_FAIL);
           }
           TRACE_DBG(DGX_OTHER, "rw_a", extent, bfincrval, hwincrval);
           break;
      default : eMSGn1(routine,"Invalid 'type'",type);
                TRACE_BOTH(DGX_OTHER, "rw30", type, 0, 0);
                return(DGX_BADVAL_FAIL);
   } /* end switch (type) */

   /*----------------------------------------------------------------
   * Get Bus Mem Attach
   *    if (hwincrval==0) only one HW address is accessed..extent is type
   *    if (hwincrval==1) many HW addresses are accessed..extent is type*count
   *----------------------------------------------------------------*/
   errcode = get_ioatt_ptr (handle, memio, offset,
             ((hwincrval)?(count*type):(type)), &ptr);
   if (errcode != DGX_OK) {
        TRACE_BOTH(DGX_OTHER, "rw_4", errcode, count, type);
	return(errcode);
   }


   if (bfincrval == 0) {
      /* if bfincrval==0, we will only use 1 scratch_pad, 'data' location */
      num_dumps   = 1;
      cp_count    = 1;  /* only copy/xmem/bcopy in/out 1 location */
      /* count = count; */
   }
   else
   if ((DGX_SCRPADSZ) < extent) {
      /* there is not enough space in scratch pad */
      /* we have to use the given space multiple times      */
      num_dumps   = extent%(DGX_SCRPADSZ) 
                  ? extent/(DGX_SCRPADSZ) + 1
                  : extent/(DGX_SCRPADSZ);     /* # times to use given space */
      count       = (DGX_SCRPADSZ)/type;       /* count should match g space */
      cp_count    = count;  /* copy/xmem/bcopy in/out count location */
   } else {
      /* there is enough space in scratch pad currently */
      /* we will use the given space one time           */
      num_dumps   = 1;
      cp_count    = count;  /* copy/xmem/bcopy in/out count location */
      /* count = count; */
   } /* end if (how we use the scratch pad) */
   TRACE_DBG(DGX_OTHER, "rw_b", num_dumps, count, cp_count);

   if ((count>1) && (timesintrlev == PROCLEV)) {
      /* prevent interrupt from overwriting process level scratch pad */
      /* NOTE: when count is 1, no scratch pad is used */
      prev_priority = i_disable(handle->dds.intr_priority);
   }


   /* log the start time */
   curtime(&handle->itime);

   if (((addr_incr_flag == DGX_MULT_LOC_ACC) || (count == 1)) &&
        (handle->dds.bus_type == BUS_MICRO_CHANNEL) && 
	(memio != DGX_SPECIAL_IO_OP)) 
   {

      if (wr_op == DGX_WRITEOP) {
         /*-------------------------------------------*/
         /*  uCHANNEL Bus  Write Operation  No ScratchPad  */
         /*-------------------------------------------*/
         switch (intrlev) {
            case PROCLEV:
                        rc=copyin(data,ptr,cp_count*type);
                        break;
            case INTRKMEM:
			rc=BUS_PUTSTRX(ptr,data,count*type); 
                        break;
            case INTRPMEM:
                        rc=xmemin(data,ptr,cp_count*type,handle->udata_dp);
             break;
         } /* end switch (intrlev) */
      } /* end if uCHANNEL write accesses */
      else
      if (wr_op == DGX_READOP) {
         /*-------------------------------------------*/
         /*  uCHANNEL Bus   Read Operation  No ScratchPad  */
         /*-------------------------------------------*/
          switch (intrlev) {
             case PROCLEV:
                        rc=copyout(ptr,data,cp_count*type);
                        break;
             case INTRKMEM:
                        rc=BUS_GETSTRX(ptr,data,cp_count*type);
                        break;
             case INTRPMEM:
                        rc=xmemout(ptr,data,cp_count*type,handle->udata_dp);
             break;
          } /* end switch (intrlev) */
      } /*end uCHANNEL READ accesses */
      if (rc) {
         TRACE_BOTH(DGX_OTHER, "rw_5", rc, 0, 0);
         eMSGn5(routine,"<COPY> OUT/IN RC",rc,"intrlev",intrlev,
                        "size",cp_count*type,"kaddr",ptr,"uaddr",data);
         SETERR(DGX_COPY_FAIL,rc);
   	 iodet_ptr(handle, ptr);
         return(errcode);
      }
   } /*end uCHANNEL NoScratchPad accesses */
   else
   if ((wr_op == DGX_WRITEOP) && (handle->dds.bus_type == BUS_MICRO_CHANNEL)) {
      /*-------------------------------------------*/
      /*  Micro Channel Write Operation            */
      /*-------------------------------------------*/
      switch (type) {
                      /*___________________________________________*/
         case IOCHAR:/*__________________________________IOCHAR___*/
         {  char *dst=ptr;
            char *src=(char *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               switch (intrlev) {
                  case PROCLEV:
                     rc=copyin(data,(handle->scratch_pad),cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_PUTSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemin(data,(handle->scratch_pad),cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw_6",rc,num_dumps,0);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> IN RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  io_det(ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }


               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  rc=BUS_PUTCX(dst, *src);
                  if (rc) {
                     TRACE_BOTH(DGX_OTHER,"rw_7",rc,i,count);
                     TRACE_BOTH(DGX_OTHER,"rw_+",(ulong) dst,(ulong)src,0);
                     SETERR(DGX_FAIL,rc);
                     io_det(ptr);
                     if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                     return(errcode);
                  }
                  dst += hwincrval;
                  src += bfincrval;
               } /* end for(i) */

               /* reset the pointers for the next buffer dump */
               data = (char*) ((int)data + (int) (DGX_SCRPADSZ));
	       src=(char *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOCHAR case */
        }
                      /*___________________________________________*/
         case IOSHORT:/*__________________________________IOSHORT__*/
         {  short *dst=(short *)ptr;
            short *src=(short *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               switch (intrlev) {
                  case PROCLEV:
                     rc=copyin(data,(handle->scratch_pad),cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_PUTSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemin(data,(handle->scratch_pad),cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw_8",rc,num_dumps,(ulong) handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> IN RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",(handle->scratch_pad),"uaddr",data);
                  io_det(ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }


               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  rc=BUS_PUTSX(dst, *src);
                  if (rc) {
                     TRACE_BOTH(DGX_OTHER,"rw_9",rc,i,count);
                     TRACE_BOTH(DGX_OTHER,"rw_+",(ulong)dst,(ulong)src,0);
                     SETERR(DGX_FAIL,rc);
                     io_det(ptr);
                     if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                     return(errcode);
                  }
                  dst += hwincrval;
                  src += bfincrval;
               } /* end for(i) */

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
  	       src=(short *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break;  /* IOSHORT case */
        }
                     /*___________________________________________*/
         case IOLONG:/*__________________________________IOLONG__*/
         {  long *dst=(long *)ptr;
            long *src=(long *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               switch (intrlev) {
                  case PROCLEV:
                     rc=copyin(data,(handle->scratch_pad),cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_PUTSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemin(data,(handle->scratch_pad),cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw10",rc,num_dumps,(ulong) handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> IN RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",(handle->scratch_pad),"uaddr",data);
   	  	  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }

               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  rc=BUS_PUTLX(dst, *src);
                  if (rc) {
                     TRACE_BOTH(DGX_OTHER,"rw11",rc,i,count);
                     TRACE_BOTH(DGX_OTHER,"rw_+",(ulong) dst,(ulong) src,0);
                     SETERR(DGX_FAIL,rc);
                     io_det(ptr);
                     if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                     return(errcode);
                  }
                  dst += hwincrval;
                  src += bfincrval;
               } /* end for(i) */

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
	       src=(long *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOLONG case */
         }
      } /* end switch (type) */
      
   } /* end if uChannel write accesses */
   else
   if ((wr_op == DGX_READOP) && (handle->dds.bus_type == BUS_MICRO_CHANNEL)) {
      /*-------------------------------------------*/
      /*  Micro Channel Read Operation             */
      /*-------------------------------------------*/
      switch (type) {
                      /*___________________________________________*/
         case IOCHAR:/*__________________________________IOCHAR___*/
         {  char *src=ptr;
            char *dst=(char *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               /* write from source to scratch pad */
               for (i=0; i<count; i++) {
                  rc=BUS_GETCX(src, dst);
                  if (rc) {
                     TRACE_BOTH(DGX_OTHER,"rw12",rc,i,count);
                     TRACE_BOTH(DGX_OTHER,"rw_+",(ulong) dst,(ulong) src,0);
                     SETERR(DGX_FAIL,rc);
                     io_det(ptr);
                     if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                     return(errcode);
                  }
                  src += hwincrval;
                  dst += bfincrval;
               } /* end for(i) */

               switch (intrlev) {
                  case PROCLEV:
                     rc=copyout((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_GETSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemout((handle->scratch_pad),data,cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw13",rc,num_dumps,(ulong)handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  io_det(ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }


               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
			   dst=(char *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOCHAR case */
        }
                      /*___________________________________________*/
         case IOSHORT:/*__________________________________IOSHORT__*/
         {  short *src=(short *)ptr;
            short *dst=(short *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {

               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  rc=BUS_GETSX(src, dst);
                  if (rc) {
                     TRACE_BOTH(DGX_OTHER,"rw14",rc,i,count);
                     TRACE_BOTH(DGX_OTHER,"rw_+",(ulong)dst,(ulong)src,0);
                     SETERR(DGX_FAIL,rc);
                     io_det(ptr);
                     if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                     return(errcode);
                  }
                  src += hwincrval;
                  dst += bfincrval;
               } /* end for(i) */

               switch (intrlev) {
                  case PROCLEV:
                     rc=copyout((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_GETSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemout((handle->scratch_pad),data,cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw15",rc,num_dumps,(ulong)handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  io_det(ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               dst=(short *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break;  /* IOSHORT case */
        }
                     /*___________________________________________*/
         case IOLONG:/*__________________________________IOLONG__*/
         {  long *src=(long *)ptr;
            long *dst=(long *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {

               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  rc=BUS_GETLX(src, dst);
                  if (rc) {
                     TRACE_BOTH(DGX_OTHER,"rw16",rc,i,count);
                     TRACE_BOTH(DGX_OTHER,"rw_+",(ulong)dst,(ulong)src,0);
                     SETERR(DGX_FAIL,rc);
                     io_det(ptr);
                     if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                     return(errcode);
                  }
                  src += hwincrval;
                  dst += bfincrval;
               } /* end for(i) */

               switch (intrlev) {
                  case PROCLEV:
                     rc=copyout((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_GETSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemout((handle->scratch_pad),data,cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw17",rc,num_dumps,(ulong)handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  io_det(ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
	       dst=(long *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOLONG case */
         }
      } /* end switch (type) */
      
   } /*end uChannel READ accesses */
   else
   if (((addr_incr_flag == DGX_MULT_LOC_ACC) || (count == 1)) )/* &&
        (handle->dds.bus_type == BUS_60X))*/ {
      if (wr_op == DGX_WRITEOP) {
         /*-------------------------------------------*/
         /*  6XX Bus  Write Operation  No ScratchPad  */
         /*-------------------------------------------*/
         switch (intrlev) {
            case PROCLEV:
                        rc=copyin(data,ptr,cp_count*type);
                        break;
            case INTRKMEM:
                     	rc = BUS_PUTSTRX(ptr,data,cp_count*type);
                        break;
            case INTRPMEM:
                        rc=xmemin(data,ptr,cp_count*type,handle->udata_dp);
             break;
         } /* end switch (intrlev) */
      } /* end if 6XX write accesses */
      else
      if (wr_op == DGX_READOP) {
         /*-------------------------------------------*/
         /*  6XX Bus   Read Operation  No ScratchPad  */
         /*-------------------------------------------*/
          switch (intrlev) {
             case PROCLEV:
                        rc=copyout(ptr,data,cp_count*type);
                        break;
             case INTRKMEM:
                     	rc = BUS_GETSTRX(ptr,data,cp_count*type);
                        break;
             case INTRPMEM:
                        rc=xmemout(ptr,data,cp_count*type,handle->udata_dp);
             break;
          } /* end switch (intrlev) */
      } /*end 6XX READ accesses */
      if (rc) {
         TRACE_BOTH(DGX_OTHER,"rw18",rc,num_dumps,(ulong)handle->scratch_pad);
         eMSGn5(routine,"<COPY> OUT/IN RC",rc,"intrlev",intrlev,
                        "size",cp_count*type,"kaddr",ptr,"uaddr",data);
         SETERR(DGX_COPY_FAIL,rc);
         iodet_ptr(handle, ptr);
         if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
		{ i_enable(prev_priority); }
         return(errcode);
      }
   } /*end 6XX NoScratchPad accesses */
   else
   if ((wr_op == DGX_WRITEOP) /*&& (handle->dds.bus_type == BUS_60X)*/ ) {
      /*-------------------------------------------*/
      /*  6XX Bus   Write Operation   w/ScratchPad */
      /*-------------------------------------------*/
      switch (type) {
                      /*___________________________________________*/
         case IOCHAR:/*__________________________________IOCHAR___*/
         {  char *dst=ptr;
            char *src=(char *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               switch (intrlev) {
                  case PROCLEV:
                     rc=copyin(data,(handle->scratch_pad),cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_PUTSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemin(data,(handle->scratch_pad),cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw19",rc,num_dumps,(ulong)handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> IN RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
   		  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }


               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  *dst = *src;
                  dst += hwincrval;
                  src += bfincrval;
               } /* end for(i) */

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               src=(char *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOCHAR case */
        }
                      /*___________________________________________*/
         case IOSHORT:/*__________________________________IOSHORT__*/
         {  short *dst=(short *)ptr;
            short *src=(short *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               switch (intrlev) {
                  case PROCLEV:
                     rc=copyin(data,(handle->scratch_pad),cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_PUTSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemin(data,(handle->scratch_pad),cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw20",rc,num_dumps,(ulong)handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> IN RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
   		  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }


               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  *dst = *src;
                  dst += hwincrval;
                  src += bfincrval;
               } /* end for(i) */

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               src=(short *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break;  /* IOSHORT case */
        }
                     /*___________________________________________*/
         case IOLONG:/*__________________________________IOLONG__*/
         {  long *dst=(long *)ptr;
            long *src=(long *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {
               switch (intrlev) {
                  case PROCLEV:
                     rc=copyin(data,(handle->scratch_pad),cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_PUTSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemin(data,(handle->scratch_pad),cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw21",rc,num_dumps,(ulong)handle->scratch_pad);
                  SETERR(DGX_COPY_FAIL,rc);
                 eMSGn5(routine,"<COPY> IN RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
   		  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }

               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  *dst = *src;
                  dst += hwincrval;
                  src += bfincrval;
               } /* end for(i) */

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               src=(long *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOLONG case */
         }
      } /* end switch (type) */
      
   } /* end if 6XX write accesses */
   else
   if ((wr_op == DGX_READOP)/* && (handle->dds.bus_type == BUS_60X)*/ ) {
      /*-------------------------------------------*/
      /*  6XX Bus    Read Operation   w/ScratchPad */
      /*-------------------------------------------*/
      switch (type) {
                      /*___________________________________________*/
         case IOCHAR:/*__________________________________IOCHAR___*/
         {  char *src=ptr;
            char *dst=(char *)(handle->scratch_pad);
            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {

               /* write from source to scratch pad */
               for (i=0; i<count; i++) {
                  *dst = *src;
                  src += hwincrval;
                  dst += bfincrval;
               } /* end for(i) */

               switch (intrlev) {
                  case PROCLEV:
                     rc=copyout((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_GETSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemout((handle->scratch_pad),data,cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw22",rc,num_dumps,(ulong) handle->scratch_pad);
                 eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  SETERR(DGX_COPY_FAIL,rc);

   		  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }


               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               dst=(char *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOCHAR case */
        }
                      /*___________________________________________*/
         case IOSHORT:/*__________________________________IOSHORT__*/
         {  short *src=(short *)ptr;
            short *dst=(short *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {

               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  *dst = *src;
                  src += hwincrval;
                  dst += bfincrval;
               } /* end for(i) */

               switch (intrlev) {
                  case PROCLEV:
                     rc=copyout((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_GETSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemout((handle->scratch_pad),data,cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw23",rc,num_dumps,(ulong)handle->scratch_pad);
                 eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  SETERR(DGX_COPY_FAIL,rc);
   		  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               dst=(short *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break;  /* IOSHORT case */
        }
                     /*___________________________________________*/
         case IOLONG:/*__________________________________IOLONG__*/
         {  long *src=(long *)ptr;
            long *dst=(long *)(handle->scratch_pad);

            /* write from source to scratch pad */
            for ( ; num_dumps>0; num_dumps--) {

               /* write from scratch pad to destination */
               for (i=0; i<count; i++) {
                  *dst = *src;
                  src += hwincrval;
                  dst += bfincrval;
               } /* end for(i) */

               switch (intrlev) {
                  case PROCLEV:
                     rc=copyout((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRKMEM:
                     rc = BUS_GETSTRX((handle->scratch_pad),data,cp_count*type);
                     break;
                  case INTRPMEM:
                     rc=xmemout((handle->scratch_pad),data,cp_count*type,handle->udata_dp);
                     break;
               } /* end switch (intrlev) */
               if (rc) {
                  TRACE_BOTH(DGX_OTHER,"rw24",rc,num_dumps,(ulong)handle->scratch_pad);
                 eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
                 "size",cp_count*type,"kaddr",handle->scratch_pad,"uaddr",data);
                  SETERR(DGX_COPY_FAIL,rc);
   		  iodet_ptr(handle, ptr);
                  if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
			{ i_enable(prev_priority); }
                  return(errcode);
               }

               /* reset the pointers for the next buffer dump */
               data = (char *) ((int)data + (DGX_SCRPADSZ));
               dst=(long *)(handle->scratch_pad);
               if (num_dumps==2) {
                  /* the next pass will be the last, set for remainder */
                  count = extent % (DGX_SCRPADSZ);
                  if (count == 0) count = (DGX_SCRPADSZ);
                  count /= type;
                  cp_count = count;  /* copy in/out count values */
               }

            } /* end for (num_dumps) */
            break; /* IOLONG case */
         }
      } /* end switch (type) */
      
   } /*end 6XX READ accesses */

   /* log the stop time */
   curtime(&handle->ntime);
   if ((!errcode) && times) {
      deltatime.tv_sec  = handle->ntime.tv_sec  - handle->itime.tv_sec;
      deltatime.tv_nsec = handle->ntime.tv_nsec - handle->itime.tv_nsec;
      switch (timesintrlev) {
         case PROCLEV:
            rc=copyout(&deltatime,times,sizeof(struct timestruc_t));
            break;
         case INTRKMEM:
            rc=BUS_GETSTRX(&deltatime,times,sizeof(struct timestruc_t));  
            break;
         case INTRPMEM:
            rc=xmemout(&deltatime,times,sizeof(struct timestruc_t),handle->udata_dp);
            break;
      } /* end switch (intrlev) */
      if (rc) {
         TRACE_BOTH(DGX_OTHER,"rw25",rc,deltatime.tv_sec,(ulong)times->tv_sec);
         TRACE_BOTH(DGX_OTHER,"rw25",rc,deltatime.tv_nsec,(ulong)times->tv_nsec);
         eMSGn5(routine,"<COPY> OUT RC",rc,"intrlev",intrlev,
         "size",sizeof(struct timestruc_t),"kaddr",&deltatime,"uaddr",times);
         SETERR(DGX_COPY_FAIL,rc);
   	 iodet_ptr(handle, ptr);
         return(errcode);
      }
   } /* end if (times) */

   /* release the I/O attached pointer */
   iodet_ptr(handle, ptr);

   /* reenable interrupts if turned off */
   if ((timesintrlev == PROCLEV) && (prev_priority != -1)) 
	{ i_enable(prev_priority); }

   TRACE_DBG(DGX_OTHER,"rw_E",errcode,0,0);
   return (errcode);

} /* end diag_rw() */

/******************************************************************************
*
* NAME:  get_ioatt_ptr
*
* FUNCTION: Get I/O Access for specified region 
*
* INPUT PARAMETERS:     handle = adapter handle
*                       memio= DGX_MEM_OP, DGX_IO_OP, DGX_POS_OP, DGX_SPECIAL_IO_OP
*                       offset = offset of register
*                       extent = # of bytes from offset that may be accessed
*                       addr   = location to put I/O attached address
*
* EXECUTION ENVIRONMENT:   process or interrupt level
*
* RETURN VALUE DESCRIPTION: DGX_OK,		errno=<not set>
*                           DGX_BOUND_FAIL,	errno=<not set>
*                           DGX_BADVAL_FAIL,	errno=<not set>
*                           DGX_INTERNAL_FAIL,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: io_att
*                             
******************************************************************************/
int
get_ioatt_ptr(diag_struc_t *handle, int memio, int offset,
              int extent, caddr_t *addr)
{
   
   struct io_map iomap;
   int errcode=DGX_OK;
   int bid=handle->dds.bus_id;
   DGX_IOATT_ROUTINE;

   TRACE_DBG(DGX_OTHER,"gipB",offset,extent,(ulong)addr);
   if (handle->dds.bus_type == BUS_BID) {
	/* initialize bus attach structure for PCI I/O space */
	switch(memio)
	{
		/* this is for getting bus memory */
      		case DGX_MEM_OP: 
			iomap.bid   = BID_VAL(IO_PCI, PCI_BUSMEM, BID_NUM(handle->dds.bus_id));
			break;
		/* default is io space */
		default:
			iomap.bid   = BID_VAL(IO_PCI, PCI_IOMEM, BID_NUM(handle->dds.bus_id));
			break;
			
	}
   	iomap.key   = IO_MEM_MAP;
	iomap.flags = 0;          /* read/write */
	iomap.size  = 0xff;    /* get 256byte */
	iomap.busaddr = (long long)(ulong)handle->dds.bus_io_addr;

	*addr = iomem_att(&iomap) + offset;

   	TRACE_BOTH(DGX_OTHER,"gipd",bid,(ulong)handle->dds.bus_mem_addr,
				(ulong)*addr);

	
   }
   else if (handle->dds.bus_type == BUS_60X) {
         if ((offset+extent) > handle->dds.bus_mem_length) {
   	    TRACE_BOTH(DGX_OTHER,"gip1",offset,extent,
				handle->dds.bus_mem_length);
            eMSGn3(routine,"Requested offset",offset,
                   "and extent",extent,
                   "larger than bus Memory length",handle->dds.bus_mem_length);
            return(DGX_BOUND_FAIL);
         }
         /* for 601 BUS, replace buid with 7F to get access to real memory */
         bid  &= (DGX_NO_BUID);
         bid  |= (DGX_REALMEM_BUID);
         bid  &= (0xFFFFFFF0);
         bid  |= (handle->dds.bus_mem_addr>>28);
         bid  |= 0x80000000;
         *addr = (uchar *) ((int)vm_att((ulong)bid,
                              (handle->dds.bus_mem_addr+offset) ));
   	 TRACE_BOTH(DGX_OTHER,"gipa",bid,(ulong)handle->dds.bus_mem_addr,
				(ulong)*addr);
   } 
   else switch (memio) {
      /*----------------------------------------------------
      *   Get IO Attach for I/O Operations
      *----------------------------------------------------*/
      case DGX_IO_OP: 
         if ((offset+extent) > handle->dds.bus_io_length) {
   	    TRACE_BOTH(DGX_OTHER,"gip3",offset,extent,
				handle->dds.bus_io_length);
            eMSGn3(routine,"Requested offset",offset,
                   "and extent",extent,
                   "larger than bus I/O length",handle->dds.bus_io_length);
            return(DGX_BOUND_FAIL);
         }
         *addr = (uchar *) ((int)BUSIO_ATT((ulong)handle->dds.bus_id,
                              (handle->dds.bus_io_addr+offset) ));
   	 TRACE_DBG(DGX_OTHER,"gipb",(ulong)handle->dds.bus_id,
				    (ulong)handle->dds.bus_io_addr,
				    (ulong)*addr);
         break;
      /*------------------------------------------------------
       *   Get IO Attach for Special I/O Operations, but use 'io_att'
       *   instead of 'BUSIO_ATT' so that an unmodified bus_id
       *   value is used to attach I/O space.
       *-----------------------------------------------------*/
      case DGX_SPECIAL_IO_OP:
         if ((offset+extent) > handle->dds.bus_io_length) {
            eMSGn3(routine,"Requested offset",offset,
                   "and extent",extent,
                   "larger than bus I/O length",handle->dds.bus_io_length);
            return(DGX_BOUND_FAIL);
         }

         *addr = (uchar *) ((int)io_att((ulong)handle->dds.bus_id,
                              (handle->dds.bus_io_addr+offset) ));

         break;

      /*----------------------------------------------------
      *   Get IO Attach for Memory Operations
      *----------------------------------------------------*/
      case DGX_MEM_OP: 
         if ((offset+extent) > handle->dds.bus_mem_length) {
   	    TRACE_BOTH(DGX_OTHER,"gip5",offset,extent,
				handle->dds.bus_mem_length);
            eMSGn3(routine,"Requested offset",offset,
                   "and extent",extent,
                   "larger than bus memory length",handle->dds.bus_mem_length);
            return(DGX_BOUND_FAIL);
         }
         *addr = (uchar *) ((int)BUSMEM_ATT((ulong)(bid | (DGX_BYPASS)),
                              (handle->dds.bus_mem_addr+offset) ));
   	 TRACE_BOTH(DGX_OTHER,"gipc",(ulong)(bid | (DGX_BYPASS)),
				    (ulong)handle->dds.bus_mem_addr,
				    (ulong)*addr);
         break;
      /*----------------------------------------------------
      *   Get IO Attach for POS Operations
      *----------------------------------------------------*/
      case DGX_POS_OP: 
         if (offset > DGX_POS_LENGTH-IOCHAR) {
   	    TRACE_BOTH(DGX_OTHER,"gip7",offset,extent,
				DGX_POS_LENGTH-IOCHAR);
            eMSGn2(routine,"Requested offset",offset,
                   "larger than bus POS length", DGX_POS_LENGTH);
            return(DGX_BOUND_FAIL);
         }
         *addr = IOCC_ATT(handle->dds.bus_id,
                        IO_IOCC + (handle->dds.slot_num<<16) + offset);
   	 TRACE_BOTH(DGX_OTHER,"gipd",(ulong)handle->dds.bus_id,
				    (ulong)(handle->dds.slot_num<<16),
				    (ulong)*addr);
         break;
      /*----------------------------------------------------
      *   Illegal Operation
      *----------------------------------------------------*/
      default:
         *addr=0;
   	 TRACE_BOTH(DGX_OTHER,"gip9",(ulong)memio,0,0);
         eMSGn1(routine,"Illegal 'memio' specified",memio);
         return(DGX_INTERNAL_FAIL);
         break;
   } /* end switch(memio) */

    TRACE_DBG(DGX_OTHER,"gipE",(ulong)errcode,*(ulong *)addr,0);
   return(errcode);

} /* end get_ioatt_ptr() */

/******************************************************************************
*
* NAME:  find_dma_index
*
* FUNCTION:  
*        search thru the 'in_use' inner array linked list until a match
*           is made with daddr, and remove the associated member from
*           the inner array linked list.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       daddr  = DGX_UNSET or valid daddr
*
* EXECUTION ENVIRONMENT:  Process or Interrupt environment
*
* RETURN VALUE DESCRIPTION: 0 if successful,		errno=<not set>
*                           DGX_BADVAL_FAIL		errno=<not set>
*                           DGX_FAIL			errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: none
*                             
*
******************************************************************************/
int
find_dma_index(diag_struc_t *handle, int daddr, int *dma_index)
{
   dma_info_t  *curr_p=NULL;
   dma_info_t  *prev_p=NULL;
   DGX_FINDINDX_ROUTINE;


   TRACE_DBG(DGX_OTHER,"fdiB",daddr,*dma_index,0);
   /* Search the 'in_use' dma_info[] member list */
   if ( handle->dma_info == NULL) {
      eMSG1(routine,"Can't Parse NULL dma_info list!!");
      TRACE_DBG(DGX_OTHER,"fdi1",(ulong)handle,(ulong)handle->dma_info,0);
      return(DGX_FAIL);
   }


   /* the DGX_SLAVE_PG's next always points to the top of the in_use list */
   curr_p = handle->dma_info[DGX_SLAVE_PG].next;
   prev_p = &handle->dma_info[DGX_SLAVE_PG];

   while (curr_p) {      /* while in the 'in_use' list */

      /* find daddr */
      if ((int) curr_p->daddr == daddr) break;
      prev_p = curr_p;
      curr_p = curr_p->next;

   } /* end while (curr_p) <while in the 'in_use' list> */

   if (curr_p && ((int) curr_p->daddr == daddr)) {
      *dma_index = ((int)curr_p - (int)(handle->dma_info))
                 / sizeof(dma_info_t);
      TRACE_DBG(DGX_OTHER,"fdiE",daddr,*dma_index,0);
      return(DGX_OK);
   } else {
      /* couldn't find daddr */
      *dma_index=DGX_UNSET;
      eMSG1(routine, "Couldn't Find DMA INDEX");
      TRACE_BOTH(DGX_OTHER,"fdi2",daddr,*dma_index,0);
      return(DGX_BADVAL_FAIL);
   }
} /* end find_dma_index() */

/******************************************************************************
*
* NAME:  free_dma_index
*
* FUNCTION:  
*        use 'index' to determine the the dma_info[] member to
*           remove from the inner array linked list.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       index  = valid index (from get_dma_index()
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: 0 if successful,		errno=<not set>
*                           DGX_BADVAL_FAIL		errno=<not set>
*                           DGX_FAIL			errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: none
*                             
*
******************************************************************************/
int
free_dma_index(diag_struc_t *handle, int dma_index)
{
   dma_info_t  *curr_p=NULL;
   dma_info_t  *prev_p=NULL;
   DGX_FREEINDX_ROUTINE;


   TRACE_DBG(DGX_OTHER,"rdiB",(ulong)handle,dma_index,0);
   /* Search the 'in_use' dma_info[] member list */
   if ( handle->dma_info == NULL) {
      TRACE_BOTH(DGX_OTHER,"rdi1",(ulong)handle,dma_index,0);
      eMSG1(routine,"Can't Parse NULL dma_info list!!");
      return(DGX_FAIL);
   }

   if ((dma_index>DGX_SLAVE_PG) && (dma_index<=handle->dds.maxmaster)) {
      curr_p = &handle->dma_info[dma_index  ];
   }

   /* the DGX_SLAVE_PG's next always points to the top of the in_use list */
   prev_p = &handle->dma_info[DGX_SLAVE_PG];

   /* find prev_p just before curr_p */
   while (prev_p->next) {      /* while in the 'in_use' list */
      if (prev_p->next == curr_p) break;
      prev_p = prev_p->next;
   } /* end while (prev_p->next) <while in the 'in_use' list> */


   if (curr_p) {
      /* this member is no longer 'in_use'                             */
      /* remove the dma_info[] member from the inner-array linked list */
      prev_p->next     = curr_p->next;
      curr_p->in_use   = FALSE;
      curr_p->next     = NULL;
      curr_p->firsttcw = DGX_UNSET;
      curr_p->last_tcw = DGX_UNSET;
      TRACE_DBG(DGX_OTHER,"rdiE",(ulong)handle,dma_index,0);
      return(DGX_OK);
   } else {
      /* couldn't find daddr */
      eMSG1(routine, "Couldn't Free DMA INDEX");
      TRACE_BOTH(DGX_OTHER,"rdi2",(ulong)handle,dma_index,0);
      return(DGX_BADVAL_FAIL);
   }
} /* end free_dma_index() */



/*****************************************************************************
*
* NAME:     diag_trace
*
* FUNCTION: Put a trace into the internal trace table and the external
*       system trace.
*
* EXECUTION ENVIRONMENT: process or interrupt
*
* NOTES:
*  
*	The Trace table will look like:
*
*                          Most of the time                Just before Wrap
*                         --------------------            --------------------
*                         |diagextrcTOP!!!!  |            |diagextrcTOP!!!!  |
*                         |         :        |      |---> |diagextrcCUR!!!!  |
*                         |         :        |      |     |    :             |
*  most recent entry--->  |tag arg1 arg2 arg3| --|  |     |    :             |
*  is just above this-->  |diagextrcCUR!!!!  |---+--|     |    :             |
*                         |         :        |   |        |    :             |
*                         |         :        |   |------> |tag arg1 arg2 arg3|
*                         |diagextrcBOT!!!!  |            |diagextrcBOT!!!!  |
*                         --------------------            --------------------
*
*	The Trace table is found/used by:
*		 1) run the code which in some way invokes diagex
*		 2) break into the lowlevel debugger
*			(bosboot -D -a -d/dev/ipldevice must have been run)
*			(use Alt-Ctl-Keypad4 from hft/lft)
*			(use Ctl-\ from tty)
*			(via kernel breakpoint,assert, or DSI)
*		 3) "map diag_open" (or the current entry point(see Makefile)
*			this gives you an address 'near' the trace table
*			(if address is 01234567, use 1230000 for <addr> in (4) )
*		 4) "find diagextrcTOP <addr>"	to find the top of trace table
*		 5) "set top fx"		to have a variable for 1st addr
*		 6) "find diagextrcBOT top+7"	to find the bottom of table
*		 8) "set bot fx"		to have a variable for last addr
*		 9) "find diagextrcCUR top+4"	to find that current position
*		10) "d fx-150 160"		to see lastest 15 trace points
*						(assuming no wrap in latest 16)
*	            "d bot-150 160"		to see 15 trace points before
*							the wrap
*		11) locate the latest trace points' tags in the source code
*			to determine the meaning of the args and latests code
*			path.  (see comments around the XXXX_TRACE macro
*			defines for tag naming conventions)
*
* CALLED FROM: all diagex routines
*
* INPUT:
*  hook		- trace hook 
*  tag			- four letter trace ID
*  arg1 to arg 3	- arguments to trace
*
* RETURNS:  none.
*
******************************************************************************/
diag_trace(
ulong  hook,	/* trace hook */
char  	*tag,	/* trace ID */
ulong  arg1,	/* 1st argument */
ulong  arg2,	/* 2nd argument */
ulong  arg3)	/* 3rd argument */

{

  int i;


  /*
   *
   *	I don't think it's that important to disable interrupts here,
   *	It takes time, we have to pass in 'handle', and we should
   *	still be able to make sense of the trace table.
   *	If you DO want to use it, see the i_disable/i_enable calls below.
   *
   *    int ipri;
   *	ipri = i_disable(handle->dds.intr_priority);
   */

  /*
   * Copies the trace point into the internal trace table
   */
  i = diag_cntl.trace.next_entry;
  diag_cntl.trace.table[i] = *(ulong *)tag;
  diag_cntl.trace.table[i+1] = arg1;
  diag_cntl.trace.table[i+2] = arg2;
  diag_cntl.trace.table[i+3] = arg3;

  /* Increment for the next entry (wrap if at end) */
  if ((i += 4 ) >= ((DGX_TRACE_SIZE)-4)) {
        i = 4;
  }
  diag_cntl.trace.next_entry = i;

  /*
   * Calls the external trace routine
   */
   TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);

  /* mark the latest postion (this will be overwritten on next call) */
  strncpy(((char *)(&diag_cntl.trace.table[i])), "dgxtraceCUR!!!!!",16);

  /*
   *i_enable(ipri);
   */

  /*
   * Calls the external trace routine
   */
   TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);


} /* diag_trace */


/******************************************************************************
*
* NAME:  diag_special_io_read
*
* FUNCTION:  Read from an I/O register using the busid provided by user
*            diagex_dds structure without any modification to the busid.
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       data = data buffer to read into
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
******************************************************************************/

int
diag_special_io_read(diag_struc_t *handle,int type,int offset,
             void *data, struct timestruc_t *times, int intlev)
{
   int rc;
   DGX_SPECIAL_IO_RD_ROUTINE;

   TRACE_BOTH(DGX_OTHER,"SpRd",0,0,0);
   rc = diag_rw(handle,           /* pass thru */
                DGX_READOP,       /* specify a read operation */
                DGX_SPECIAL_IO_OP, /* specify a special I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */

    return(rc);

} /* end diag_special_io_read() */

/******************************************************************************
*
* NAME:  diag_special_io_write
*
* FUNCTION:  Read from an I/O register using the busid provided by user
*            diagex_dds structure without any modification to the busid.
*
*
* INPUT PARAMETERS:     handle = adapter handle
*                       type = IOCHAR, IOSHORT , IOLONG
*                       offset = offset of register
*                       data = data value to write from
*                       times = address of time structure, NULL if don't care
*                       intlev = PROCLEV  if calling from user process level
*                                INTRKMEM if calling from interrupt level
*                                         (data buffer in kernel memory)
*                                INTRPMEM if calling from user process level
*                                         (data buffer in handle->intr_data)
******************************************************************************/

int
diag_special_io_write(diag_struc_t *handle,int type,int offset,
              ulong dataval, struct timestruc_t *times, int intlev)
{
   int rc;
   ulong *data=&dataval; /* this allows the user to pass data, not an address */
                        /* but it also uses a KernelSpaceBuffer(change intlev)*/
   DGX_SPECIAL_IO_WR_ROUTINE;

   TRACE_BOTH(DGX_OTHER,"SpWr",0,0,0);
   /* shift the data into the correct position(char shifts 24,short 16,long 0)*/
   *data = (*data << ((IOLONG-type)<<3));

   intlev <<= 16;     /* Keep orig intlev in UpperWord for times data struct  */
   intlev |= INTRKMEM;/* since we are using a local buffer (data) not dataval */

   rc = diag_rw(handle,           /* pass thru */
                DGX_WRITEOP,      /* specify a write operation */
                DGX_SPECIAL_IO_OP, /* specify a special I/O operation  */
                type,             /* pass thru */
                offset,           /* pass thru */
                1,                /* Only do one access */
                DGX_SING_LOC_ACC, /* specify access to ONE location only */
                data,             /* pass thru */
                times,            /* pass thru */
                intlev);          /* pass thru */
   return(rc);
} /* end diag_special_io_write() */

/******************************************************************************
*
* NAME:  iodet_ptr
*
* FUNCTION:  Clean up on exit from pio.  Figure out which type of detach 
*            should be done.
*
*
* INPUT PARAMETERS:     handle = adapter handle
*                       addr = 
******************************************************************************/
iodet_ptr(diag_struc_t *handle, uchar * addr)
{

	switch(handle->dds.bus_type)
	{
         	case BUS_60X:  
			vm_det(addr); 
			break;
		case BUS_BID:
			iomem_det(addr);
			break;
		default:
         		io_det(addr);  
			break;
	}
}
/******************************************************************************
*
* NAME:  diag_map_disable
*
* FUNCTION: disables the DMA channel for this handle. 
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
diag_map_disable(diag_struc_t * handle)
{
   int rc=DGX_OK;

   TRACE_DBG(DGX_OTHER, "dmdB", (ulong)handle, 0, 0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "dmd2", (ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   /*----------------------------------------------------------------
   * disable the DMA Channel
   *----------------------------------------------------------------*/
   rc = D_MAP_DISABLE(handle->dhandle);
   if(rc != DMA_SUCC)
   {
      eMSGn1(routine,"Unable to disable DMA channel",handle);
      TRACE_BOTH(DGX_OTHER, "dmd3", (ulong)handle, 0, 0);
      return(DGX_DISABLE_FAIL);
   }

   TRACE_DBG(DGX_OTHER, "dmdE", 0, 0, 0);
   return(rc);
} /* end of diag_map_disable() */

/******************************************************************************
*
* NAME:  diag_map_enable
*
* FUNCTION: Enables the DMA channel for this handle. 
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
diag_map_enable(diag_struc_t * handle)
{
   int rc=DGX_OK;

   TRACE_DBG(DGX_OTHER, "dmeB", (ulong)handle, 0, 0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "dme2", (ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   /*----------------------------------------------------------------
   * enable the DMA Channel
   *----------------------------------------------------------------*/
   rc = D_MAP_ENABLE(handle->dhandle);
   if(rc != DMA_SUCC)
   {
      eMSGn1(routine,"Unable to enable DMA channel",handle);
      TRACE_BOTH(DGX_OTHER, "dme1", (ulong)handle->dhandle, 0, 0);
      return(DGX_ENABLE_FAIL);
   }
   TRACE_DBG(DGX_OTHER, "dmeE", (ulong)handle, rc, 0);

   return(rc);
} /* end of diag_map_enable() */
/******************************************************************************
*
* NAME:  diag_map_page
*
* FUNCTION: This function initializes, pins, and cross-memory attaches the user
*	    buffer by using the DMA_MAP_PAGE macro.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
* 			int dma_flags = Use the DMA_READ flag for transferring
*				data form the system to the adapter.  See 
*				<sys/dma.h>  more info.
* 			caddr_t baddr = Points to user's read/write buffer
* 			uint * daddr  = Points to an integer to be filled 
*				with the physical memory address of baddr 
*				upon successful completion of this call.
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*			    DGX_BOUND_FAIL,	errno=<not set>
*                           DGX_BADVAL_FAIL,	errno=<not set>
*                           DGX_PINU_FAIL,	errno=<not set>
*                           DGX_XMATTACH_FAIL,	errno=<not set>
*                      
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int
diag_map_page(diag_struc_t * handle,int dma_flags, caddr_t baddr, uint * users_daddr,int count)
{
   int i,offset,rc,daddr=DGX_UNSET;
   int errcode=DGX_OK;
   int dma_index;  /* index into dma_info[] */
   struct xmem dp; 
   int prev_priority;
   DGX_MASTER_ROUTINE;

   TRACE_DBG(DGX_OTHER, "dmpB", dma_flags, (ulong) baddr, count);
    rc=suword((int *)users_daddr, (int)DGX_UNSET);
    if (rc) {
       eMSGn1(routine,"Invalid daddr Pointer",users_daddr);
       SETERR(DGX_BADVAL_FAIL,rc);
       TRACE_BOTH(DGX_OTHER, "dmp1", errcode, 0, 0);
       return(errcode);
    }
   
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER, "dmp2", (ulong)handle, DGX_INVALID_HANDLE, 0);
      return(DGX_INVALID_HANDLE);
   }

   if (handle->dds.bus_type != BUS_BID) {
      eMSGn1(routine,"invalid type",handle);
      TRACE_BOTH(DGX_OTHER, "dmp2", (ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   offset = (int)baddr & 0xFFF;
   /* disallow DMA if count too big, count 0, or maxmaster = 0 */
   if (((offset+count) > PAGESIZE) || (count < 1) || (handle->dds.maxmaster==0)) {
      eMSGn5(routine,
         "No count or DMA Buffer smaller than count for handle",handle,
         "DMA transfer byte count",count,
         "DMA Buffer size",handle->dds.dma_bus_length,
         "DMA Buffer Offset",offset,
         "Maximum #of DMAs",handle->dds.maxmaster);
      TRACE_BOTH(DGX_OTHER, "dmp3", offset, count, i);
      TRACE_BOTH(DGX_OTHER, "dmp+", handle->dds.maxmaster,
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
      TRACE_BOTH(DGX_OTHER, "dmp4", count, offset, rc);
      return(rc);
   }

   /*----------------------------------------------------------------
   * Prepare Memory for DMA
   *----------------------------------------------------------------*/
   rc=pinu(baddr, count, UIO_USERSPACE);
   if (rc) {
      eMSGn1(routine,"Couldn't Pin DMA Buffer: size ",count);
      SETERR(DGX_PINU_FAIL,rc);
      free_dma_index(handle,dma_index);
      TRACE_BOTH(DGX_OTHER, "dmp5", rc, errcode, 0);
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
      TRACE_BOTH(DGX_OTHER, "dmp6", rc, errcode, 0);
      return(errcode);
   } else {
      handle->dma_info[dma_index].xmattached = TRUE;
   }

   /*----------------------------------------------------------------
   * Get the physical address of baddr (daddr)
   * Prepare a single page for DMA
   *----------------------------------------------------------------*/
   rc = D_MAP_PAGE(handle->dhandle,dma_flags,baddr,users_daddr,&handle->dma_info[dma_index].dp);

   if (rc != DMA_SUCC) {
      eMSG1(routine,"Couldn't CrossMemDMA");
      SETERR(DGX_FAIL,rc);
      diag_dma_complete(handle,(int) handle->dma_info[dma_index].daddr);/*cleanu
p*/
      TRACE_BOTH(DGX_OTHER, "dmp7", rc, errcode, 0);
      return (errcode);
   }


   /* save d_master/xmemdma info for future use */
   handle->dma_info[dma_index].daddr = (uchar *) *users_daddr;
   handle->dma_info[dma_index].dma_flags = dma_flags;
   handle->dma_info[dma_index].in_use = TRUE;

   /* make physical address available so adapter may be programmed */
   /*rc=suword((int *)users_daddr, (int)daddr);  */
   if (rc) {
      eMSGn1(routine,"Invalid daddr Pointer",users_daddr);
      SETERR(DGX_BADVAL_FAIL,rc);
      TRACE_BOTH(DGX_OTHER, "dmp8", *users_daddr, errcode, 0);
      return(errcode);
   }

   TRACE_DBG(DGX_OTHER, "dmpE", (ulong )*users_daddr, 0, 0);
   return (errcode);

} /* end of diag_map_page */



/******************************************************************************
*
* NAME:  diag_unmap_page
*
* FUNCTION: This function initializes, pins, and cross-memory attaches the user
*	    buffer by using the DMA_MAP_PAGE macro.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
* 			int dma_flags = Use the DMA_READ flag for transferring
*				data form the system to the adapter.  See 
*				<sys/dma.h>  more info.
* 			caddr_t baddr = Points to user's read/write buffer
* 			uint * daddr  = Points to an integer to be filled 
*				with the physical memory address of baddr 
*				upon successful completion of this call.
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*			    DGX_BOUND_FAIL,	errno=<not set>
*                           DGX_BADVAL_FAIL,	errno=<not set>
*                           DGX_PINU_FAIL,	errno=<not set>
*                           DGX_XMATTACH_FAIL,	errno=<not set>
*                      
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int
diag_unmap_page(diag_struc_t * handle, int * daddr)
{
   int rc;                  /* return code from called routines */
   int errcode=DGX_OK;     /* return code from this routine */
   int dma_index;

   TRACE_DBG(DGX_OTHER,"DupB",(ulong)handle,(ulong)*daddr,0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER,"Dup1",(ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   rc = find_dma_index(handle,* daddr, &dma_index);
   if (rc) return(rc);

   D_UNMAP_PAGE(handle->dhandle,daddr);

   /* Detach the buffer used */
   if ( handle->dma_info[dma_index].xmattached) {
      rc=xmdetach(&handle->dma_info[dma_index].dp);
      if (rc != XMEM_SUCC) {
         eMSGn1(routine,"Detach Failed, rc",rc);
         SETERR(DGX_XMDETACH_FAIL,rc);
         TRACE_BOTH(DGX_OTHER,"Dup5",rc,dma_index,0);
         /* Do NOT return, Continue to mark-free and unpin */
      } else {
         handle->dma_info[dma_index].xmattached = FALSE;
      }
   }

   /* Unpin the buffer used */
   if ( handle->dma_info[dma_index].pinned) {
      rc=unpinu(handle->dma_info[dma_index].baddr,
		handle->dma_info[dma_index].count,
                UIO_USERSPACE);
      if (rc) {
         eMSGn1(routine,"Unpin Failed, rc",rc);
         SETERR(DGX_UNPINU_FAIL,rc);
         TRACE_BOTH(DGX_OTHER,"Dup6",rc,dma_index,0);
         /* Do NOT return, Continue to mark-free */
      } else {
         handle->dma_info[dma_index].pinned = FALSE;
      }
   }

   handle->dma_info[dma_index].in_use = FALSE;

   TRACE_DBG(DGX_OTHER,"DupE",(ulong)handle,(ulong)errcode,0);
   return(errcode);

} /* end diag_unmap_page() */
/******************************************************************************
*
* NAME:  diag_map_slave
*
* FUNCTION: This function initializes, pins, and cross-memory attaches the user
*	    buffer by using the D_MAP_SLAVE macro.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
* 			dma_flags = Use the DMA_READ flag for transferring
*				data form the system to the adapter.  See 
*				<sys/dma.h>  more info.
* 			baddr  = Points to user's buffer
*                       count = number of bytes to transfer
*			minxfer = minimum transfer device/system allows.
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*                           DGX_MAP_SLAVE_FAIL, errno=<not set>
*                      
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int
diag_map_slave(diag_struc_t * handle,int dma_flags, caddr_t baddr, int count,int minxfer)
{
	int rc;
	int errcode;
      	TRACE_BOTH(DGX_OTHER,"SlvB",(ulong)handle,0,0);

	/* 
	 * get dio structure.  pass pointer to dio struct and number of io 
	 * vecs.
	 */
	if(handle->dio_st->dioinit != TRUE)
	{
		DIO_INIT(handle->dio_st->vlist,count);
		handle->dio_st->dioinit = TRUE;
	}

   	/*----------------------------------------------------------------
   	* Detect Gross Errors
   	*----------------------------------------------------------------*/
   	/* Ensure handle is valid */
   	if ( ! find_handle(handle) ) {
	      	eMSGn1(routine,"Unable to find handle",handle);
      		TRACE_BOTH(DGX_OTHER,"Slv1",(ulong)handle,0,0);
      		return(DGX_INVALID_HANDLE);
   	}

   	if (count <= 0) {
	      eMSGn1(routine,"Invalid count", count);
	      TRACE_BOTH(DGX_OTHER,"Slv2",count,0,0);
	      return(DGX_BOUND_FAIL);
	} /* end if (count <=0) */


	handle->dio_st->vlist->used_iovecs = 1;
	handle->dio_st->vlist->dvec->iov_base = baddr;
	handle->dio_st->vlist->dvec->iov_len = count;
	handle->dio_st->vlist->dvec->xmp = xmem_global;

	rc = D_MAP_SLAVE(handle->dhandle,dma_flags,minxfer,
			handle->dio_st->vlist,handle->dio_st->chan_flags);

   	if( rc != DMA_SUCC)
   	{
       		eMSGn1(routine,"d_map_slave Failed, rc",rc);
        	SETERR(DGX_MAP_SLAVE_FAIL,rc);
        	TRACE_BOTH(DGX_OTHER,"Slv3",rc,0,0);
 		return(errcode);
   	}

      	TRACE_BOTH(DGX_OTHER,"SlvE",(ulong)handle,0,0);
	return(errcode);

}/* end of diag_map_slave */

/******************************************************************************
*
* NAME:  diag_unmap_slave
*
* FUNCTION: This function cleans up after you have finished doing a DMA slave
*	    operation.
*
* INPUT PARAMETERS:     handle = handle returned by diag_open
* 			uint * daddr  = Points to an integer to be filled 
*				with the physical memory address of baddr 
*				upon successful completion of this call.
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*                           DGX_INVALID_HANDLE,	errno=<not set>
*                           DGX_UMAP_SLAVE_FAIL, errno=<not set>
*                      
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int
diag_unmap_slave(diag_struc_t * handle)
{
   int rc;                  /* return code from called routines */
   int errcode=DGX_OK;     /* return code from this routine */
   int dma_index;

   TRACE_DBG(DGX_OTHER,"DusB",(ulong)handle,0,0);
   /*----------------------------------------------------------------
   * Detect Gross Errors
   *----------------------------------------------------------------*/
   /* Ensure handle is valid */
   if ( ! find_handle(handle) ) {
      eMSGn1(routine,"Unable to find handle",handle);
      TRACE_BOTH(DGX_OTHER,"Dus1",(ulong)handle, 0, 0);
      return(DGX_INVALID_HANDLE);
   }

   rc = D_UNMAP_SLAVE(handle->dhandle);

   if( rc != DMA_SUCC)
   {
        eMSGn1(routine,"D_UNMAP_SLAVE Failed, rc",rc);
        SETERR(DGX_UMAP_SLAVE_FAIL,rc);
        TRACE_BOTH(DGX_OTHER,"Dus2",rc,0,0);
 	return(errcode);
   }

   TRACE_DBG(DGX_OTHER,"DusE",(ulong)handle,(ulong)errcode,0);
   return(errcode);

} /* end diag_unmap_slave() */

/******************************************************************************
*
* NAME:  get_ISA_buffer
*
* FUNCTION: This function mallocs the memory in the lower 16 MB of real memory
*	    space since ISA can only access this area.  Run the free_ISA_buffer
* 	    to free this memory.
*
* INPUT PARAMETERS:     baddr = buffer address obtained from get_ISA_buffer
*			size  = size of the buffer
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*			    DGX_RMALLOC_FAIL if not successful, errno=<not set>
*                      
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int get_ISA_buffer(caddr_t baddr,int size)
{
	int errcode=DGX_OK;

	baddr = (caddr_t )rmalloc(size, UWSHIFT);
       	if (baddr == NULL) {
          eMSGn2(routine,"Can't rmalloc buffer",(int *)baddr,"size",sizeof(diag_struc_t));
          SETERR(DGX_RMALLOC_FAIL,baddr); 
	  return(errcode);
	}

	return(errcode);
}

/******************************************************************************
*
* NAME:  free_ISA_buffer
*
* FUNCTION: This function free's the memory that was obtained in get_ISA_buffer
*
* INPUT PARAMETERS:     baddr = buffer address obtained from get_ISA_buffer
*			size  = size of the buffer
*
* EXECUTION ENVIRONMENT:  Process or interrupt.
*
* RETURN VALUE DESCRIPTION: DGX_OK if successful,	errno=<not set>
*			    DGX_ISA_FREE_FAIL if not successful, errno=<not set>
*                      
* EXTERNAL PROCEDURES CALLED: d_complete, unpinu, xmdetach
*
******************************************************************************/
int free_ISA_buffer(caddr_t  baddr,int size)
{
	int rc;
	int errcode=DGX_OK;

	rc = rmfree(baddr,size);
       	if (rc == NULL) {
          eMSGn2(routine,"Can't rmfree buffer",baddr,"size",sizeof(diag_struc_t));
          SETERR(DGX_RMFREE_FAIL,rc); 
	  return(errcode);
	}

	return(errcode);
}
