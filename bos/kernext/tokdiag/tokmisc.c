static char sccsid[] = "@(#)74	1.12  src/bos/kernext/tokdiag/tokmisc.c, diagddtok, bos411, 9428A410j 2/17/94 18:47:40";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  save_trace(), cio_gen_crc(),
 *             remove_netid_for_chan(),
 *             sll_init_list(), sll_alloc_elem(), sll_free_elem(),
 *             sll_link_last(), sll_unlink_first(),
 *             que_stat_blk(), report_status(),
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/tokuser.h>

#include "tokddi.h"
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

extern tracetable_t    tracetable;
extern cdt_t   ciocdt;
extern dd_ctrl_t   dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     save_trace
 *
 * FUNCTION: enter trace data in the trace table and call AIX trace
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    Do not call directly -- use macros defined in cioddhi.h so that
 *           tracing can be converted to a zero-run-time-cost option by
 *           simply re-defining the macros.
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void save_trace (num,str1,arg2,arg3,arg4,arg5)
   register ulong  num;     /* number of additional arguments (1-5)        */
   register char   str1[];  /* string of 0 to 4 characters (best to use 4) */
            ulong  arg2;    /* optional data word                          */
            ulong  arg3;    /* optional data word                          */
            ulong  arg4;    /* optional data word                          */
            ulong  arg5;    /* optional data word                          */
{
#define     MAXTRACEARGS  5
   register char  *s;
   register int    ndx;
            ulong  trcdata[MAXTRACEARGS];

   trcdata[4] = trcdata[3] = trcdata[2] = trcdata[1] = 0; /* [0] is ok */
   num = MIN (num, MAXTRACEARGS);
   switch (num)
   {
      case 5:  trcdata[4] = arg5;
      case 4:  trcdata[3] = arg4;
      case 3:  trcdata[2] = arg3;
      case 2:  trcdata[1] = arg2;
      case 1:  trcdata[0] = 0x20202020;               /* spaces */
         /* first argument is supposed to be a character string */
         for (ndx=0, s=(char *)trcdata;
              ((ndx<4) && (str1[ndx] != 0)); ndx++,s++)
            *s = str1[ndx];

         /* now, for in-memory trace table, move the data */
         for (ndx = 0; ndx < num; ndx++)
         {
            tracetable.table[tracetable.nexthole] = trcdata[ndx];
            if (++tracetable.nexthole >= TRACE_TABLE_SIZE)
               tracetable.nexthole = 0;
         }
         /* mark end of table data */
         tracetable.table[tracetable.nexthole] = 0x60606060; /* "ZZZZ" */
   }

   /* for AIX trace, make the call */
   TRCHKGT (TRACE_HOOKWORD | HKTY_GT | num, trcdata[0],
            trcdata[1], trcdata[2], trcdata[3], trcdata[4]);

   return;
} /* end save_trace */


/*****************************************************************************/
/*
 * NAME:     cio_gen_crc
 *
 * FUNCTION: generate a 16-bit crc value (used to check VPD)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  16-bit crc value per specs for MCA bus VPD area crc
 *
 */
/*****************************************************************************/
ushort cio_gen_crc (
   register uchar *buf,  /* area with data whose crc value is to be computed */
   register int   len)   /* number of bytes is data area */
{
   register uchar work_msb;
   register uchar work_lsb;
   register uchar value_msb;
   register uchar value_lsb;
   ushort tempshort;

   DEBUGTRACE3 ("CRCb", (ulong)buf, (ulong)len); /* cio_gen_crc begin */

   /* step through the caller's buffer */
   for (value_msb = 0xFF, value_lsb = 0xFF; len > 0; len--)
   {
      value_lsb ^= *buf++;
      value_lsb ^= (value_lsb << 4);

      work_msb = value_lsb >> 1;
      work_lsb = (value_lsb << 7) ^ value_lsb;

      work_lsb = (work_msb << 4) | (work_lsb >> 4);
      work_msb = ((work_msb >> 4) & 0x07) ^ value_lsb;

      value_lsb = work_lsb ^ value_msb;
      value_msb = work_msb;

   } /* end loop to step through the caller's buffer */

   tempshort = ((ushort)value_msb << 8) | value_lsb;
   DEBUGTRACE2 ("CRCe", (ulong)tempshort); /* cio_gen_crc end */
   return(tempshort);
} /* end cio_gen_crc */

/*****************************************************************************/
/*
 * NAME:     remove_netid_for_chan
 *
 * FUNCTION: remove last netid for specified channel from table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  TRUE iff item was in table (else FALSE)
 *
 */
/*****************************************************************************/
int 
remove_netid_for_chan (
register dds_t   *p_dds,  /* pointer to dds structure */
register chan_t   chan)     /* channel for which last netid is to be remvd*/
{
	int rc;
	int ndx;
	int saved_intr_level;

	DEBUGTRACE3 ("RMCb",(ulong)p_dds,(ulong)chan);
	DISABLE_INTERRUPTS (saved_intr_level);

	rc = FALSE;
	/* find the netid element in the array */
	for (ndx = 0; ndx < TOK_MAX_NETIDS ; ndx++) {
		if ( (chan == CIO.netid_table[ndx].chan) &&
		     (CIO.netid_table[ndx].inuse == TRUE) )
		{
			CIO.netid_table[ndx].inuse = FALSE;
			CIO.num_netids--;
			rc = TRUE;
			break;
		}
	}

	ENABLE_INTERRUPTS (saved_intr_level);
	DEBUGTRACE2 ("RMCe", (ulong)rc); /* remove_netid_for_chan end */
	return (rc);
}

/*****************************************************************************/
/*
 * NAME:     sll_init_list
 *
 * FUNCTION: initialize a singly-linked list structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void sll_init_list (
   s_link_list_t *ll_ptr,      /* ptr to control structure                   */
   sll_elem_ptr_t space_ptr,   /* ptr to area for list (max_elem * elem_size)*/
   int            max_elem,    /* number of elements in list                 */
   int            elem_size,   /* size of each element                       */
   ulong         *hwm_ptr)     /* ptr to high-water-mark for queue size      */
{
   int num_elem;
   sll_elem_ptr_t  ptr;
   sll_elem_ptr_t *prev_next_addr;

   ll_ptr->num_free = max_elem;   /* initially there will be this many free */
   ll_ptr->num_elem = 0;          /* and none will be on the active list */
   ll_ptr->max_elem = max_elem;   /* we need this only for consistency check */
   ll_ptr->elem_size = elem_size; /* keep size of an element */
   ll_ptr->hwm_ptr = hwm_ptr;     /* pointer to high water mark (max queued) */
   ll_ptr->area_ptr = space_ptr;  /* caller needs this later to free space */
   ll_ptr->free_ptr = space_ptr;  /* the free list will be here initially */
   ll_ptr->head_ptr = NULL;       /* and the active list will be empty */

   *hwm_ptr = 0;                  /* zero high water mark */

   /* run through the allocated space and thread the free list */
   for (prev_next_addr = &(ll_ptr->free_ptr), ptr = space_ptr, num_elem = 0;
        num_elem < max_elem;
        prev_next_addr = &(ptr->next), ptr = ptr->next, num_elem++)
   {
      ptr->next = (sll_elem_ptr_t)((ulong)ptr + elem_size);
   }
   ll_ptr->limt_ptr = *prev_next_addr; /* remember address just past end */
   *prev_next_addr = NULL;             /* mark end of list */
   return;
} /* end sll_init_list */

/*****************************************************************************/
/*
 * NAME:     sll_alloc_elem
 *
 * FUNCTION: allocate a singly-linked list element by unlinking first free item
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  pointer to list element (NULL if none available)
 *
 */
/*****************************************************************************/
sll_elem_ptr_t sll_alloc_elem (
   register s_link_list_t *ll_ptr) /* pointer to list control structure */
{
   register sll_elem_ptr_t ptr;
   register int            saved_intr_level;

   DISABLE_INTERRUPTS(saved_intr_level);
   ptr = ll_ptr->free_ptr;         /* we're going to allocate the first one*/
   if (ptr != NULL)
   {
      ll_ptr->free_ptr = ptr->next;      /* head now points to next */
      ll_ptr->num_free--;                /* reduce count of free elements */
   }
   ENABLE_INTERRUPTS(saved_intr_level);
   if (ptr != NULL)
      bzero (ptr, ll_ptr->elem_size);    /* zero the element */
   return(ptr);
} /* end sll_alloc_elem */

/*****************************************************************************/
/*
 * NAME:     sll_free_elem
 *
 * FUNCTION: free a singly-linked list element by linking to front of free list
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void sll_free_elem  (
   register s_link_list_t  *ll_ptr,   /* pointer to list control structure */
   register sll_elem_ptr_t  elem_ptr) /* pointer to element to be freed */
{
   register int saved_intr_level;

   DISABLE_INTERRUPTS(saved_intr_level);
   elem_ptr->next = ll_ptr->free_ptr; /* freed element's next is old head */
   ll_ptr->free_ptr = elem_ptr;       /* put freed item at head */
   ll_ptr->num_free++;                /* increase count of free elements */
   ENABLE_INTERRUPTS(saved_intr_level);
   return;
} /* end sll_free_elem */

/*****************************************************************************/
/*
 * NAME:     sll_link_last
 *
 * FUNCTION: link a singly-linked list element to the end of the active list
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void sll_link_last (
   register s_link_list_t  *ll_ptr,   /* pointer to list control structure */
   register sll_elem_ptr_t  elem_ptr) /* pointer to element to add to list */
{
   register sll_elem_ptr_t *head_ptr_ptr;
   register sll_elem_ptr_t  ptr;
   register int             saved_intr_level;

   DISABLE_INTERRUPTS(saved_intr_level);
   /* find the end of the list */
   for (head_ptr_ptr = &(ll_ptr->head_ptr), ptr = *head_ptr_ptr;
        ptr != NULL;
        head_ptr_ptr = &(ptr->next), ptr = ptr->next)
      ; /* NULL statement */
   elem_ptr->next = NULL;    /* new element's next marks end of list */
   *head_ptr_ptr = elem_ptr; /* previous element's next is this new element */
   ll_ptr->num_elem++;       /* increase number of elements linked */
   if (*(ll_ptr->hwm_ptr) < ll_ptr->num_elem ) /* update high water mark */
       *(ll_ptr->hwm_ptr) = ll_ptr->num_elem;
   ENABLE_INTERRUPTS(saved_intr_level);
   return;
} /* end sll_link_last */

/*****************************************************************************/
/*
 * NAME:     sll_unlink_first
 *
 * FUNCTION: unlink the first singly-linked list element from the active list
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  pointer to list element (NULL if list empty)
 *
 */
/*****************************************************************************/
sll_elem_ptr_t sll_unlink_first (
   register s_link_list_t  *ll_ptr) /* pointer to list control structure */
{
   register sll_elem_ptr_t  ptr;
   register int saved_intr_level;

   DISABLE_INTERRUPTS(saved_intr_level);
   ptr = ll_ptr->head_ptr;
   if (ptr != NULL)
   {
      ll_ptr->head_ptr = ptr->next; /* head now points to next */
      ll_ptr->num_elem--;           /* reduce count of linked elements */
      ptr->next = NULL;             /* removed element now points nowhere */
   }
   ENABLE_INTERRUPTS(saved_intr_level);
   return(ptr);
} /* end sll_unlink_first */

/*****************************************************************************/
/*
 * NAME:     que_stat_blk
 *
 * FUNCTION: queue a status block (for user process only)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void que_stat_blk (
   register dds_t          *p_dds,      /* pointer to dds structure */
   register open_elem_t    *open_ptr,     /* pointer to open structure */
   register cio_stat_blk_t *stat_blk_ptr) /* pointer to block to queue */
{
   sta_elem_t *sta_ptr;

  /*
   * DEBUGTRACE5 ("QSBb", (ulong)p_dds, (ulong)open_ptr, (ulong)stat_blk_ptr,
   *   (ulong)stat_blk_ptr->code);
   */
   /* que_stat_blk begin */

   sta_ptr =
      (sta_elem_t *) sll_alloc_elem ((s_link_list_t *)(&(open_ptr->sta_que)));
   if (sta_ptr != NULL)
   {
      sta_ptr->stat_blk = *stat_blk_ptr;
      sll_link_last ((s_link_list_t *)(&(open_ptr->sta_que)),
         (sll_elem_ptr_t)sta_ptr);

      /* notify any waiting user process there is status available */
      if (open_ptr->selectreq & POLLPRI)
      {
         open_ptr->selectreq &= ~POLLPRI;
         selnotify (open_ptr->devno, open_ptr->chan, POLLPRI);
      }
   }
   else
   {
      TRACE2 ("QSB1", (ulong)p_dds); /* que_stat_blk ERROR (stat que full) */
      open_ptr->sta_que_ovrflw = TRUE;
      RAS.ds.sta_que_overflow++;
   }

   return;
} /* end que_stat_blk */

/*****************************************************************************/
/*
 * NAME:     report_status
 *
 * FUNCTION: report status to kernel or user process
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void 
report_status (
   register dds_t          *p_dds,      /* pointer to dds structure */
   register open_elem_t    *open_ptr,     /* pointer to open structure */
   register cio_stat_blk_t *stat_blk_ptr) /* pointer to block to report */
{
   if (open_ptr->devflag & DKERNEL) /* data is for a kernel process */
   {
      /* notify kernel process */
      DEBUGTRACE1 ("KSFc"); /* kernel proc status function call */
      (*(open_ptr->sta_fn)) (open_ptr->open_id, stat_blk_ptr);
      DEBUGTRACE1 ("KSFr"); /* kernel proc status function returned */
   }
   else /* data is for a user process */
   {
      /* add the status block to the status queue */
      que_stat_blk (p_dds, open_ptr, stat_blk_ptr);
   }
   return;
}

