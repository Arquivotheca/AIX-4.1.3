static char sccsid[] = "@(#)47  1.20.1.5  src/bos/kernext/ientdiag/i_ciodd.c, diagddient, bos411, 9428A410j 5/17/94 15:15:14";
/*
 * COMPONENT_NAME: SYSXIENT -- Common Communications Code Model
 *
 * FUNCTIONS: cioconfig, ciompx,   cioopen,  cioclose,
 *            cioread,   ciowrite, cioioctl, cioselect,
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <net/spl.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include "ient_comio_errids.h"
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/ioacc.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/except.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/ciouser.h>
#include <sys/entuser.h> 
#include "i_entddi.h"
#include "i_cioddi.h"
#include "i_cioddhi.h"
#include "i_entdslo.h"
#include "i_entdshi.h"
#include "i_ciodds.h"
#include "i_ddctl.h"

/*****************************************************************************/
/*
 * NAME: ciodd.c
 *
 * FUNCTION: common code device driver for producing ethdd.c
 *
 * EXECUTION ENVIRONMENT:
 *    This module becomes part of an AIX device driver for communications
 *    devices.  It must be combined with xxxds.c to form the complete
 *    device driver.
 *
 * NOTES:
 *    This module provides all of the AIX3 entry points for the Ethernet
 *    multiplexed communications device driver.
 *
 *    Roughly speaking, this common module provides the interface to the AIX3
 *    kernel and associated control structure management.  It functions as a
 *    "device head" while the device-specific module provides all the actual
 *    i/o and other adapter-specific functions.
 *
 *    The device-specific module is expected to provide the functions defined
 *    below under "external functions required".
 *
 *    The prototypes for the utility routines in this module are defined in
 *    ciodslo.h so any changes to these functions' entry points should also
 *    be updated in that include file.
 *
 */


/*****************************************************************************/
/* external functions required to be in device-specific code                 */
/*****************************************************************************/
/* one-time device driver initialization */
extern void xxx_init (void);

/* device-specific initialization of dds, set POS registers */
extern int xxx_initdds (dds_t *dds_ptr);

/* replace ciointr() with xxx_intr() */
extern int xxx_intr (struct intr  *ihsptr);

/* detect invalid file name extension at ddmpx time */
extern int xxx_badext (char *channame);

/* activate ("open" and/or connect) the adapter */
extern int xxx_act (dds_t *dds_ptr);

/* inactivate ("close" and/or disconnect) the adapter */
extern int xxx_inact (dds_t *dds_ptr);

/* device specific ioctl's */
extern int xxx_ioctl (dds_t       *dds_ptr,
open_elem_t *open_ptr,
int cmd, int arg, ulong devflag, int ext);

/* start a transmit if appropriate */
extern void xxx_xmit (dds_t *dds_ptr);

/* fill in the START_DONE (CONNECT_DONE) status block */
extern void xxx_startblk (dds_t          *dds_ptr,
netid_elem_t   *netid_elem_ptr,
cio_stat_blk_t *stat_blk_ptr);

/* process a halt (usually just build HALT_DONE status block and report it) */
extern void xxx_halt (dds_t          *dds_ptr,
open_elem_t    *open_ptr,
cio_sess_blk_t *sess_blk_ptr);

/* allow device specific code to clean up if abnormal close */
extern void xxx_close (dds_t          *dds_ptr,
open_elem_t    *open_ptr);

/* watchdog timer interrupt handler */
extern void wdt_intr (struct	watchdog *wds_ptr );

extern int ent_fastwrt_entry(
open_elem_t             *p_open,
cio_get_fastwrt_t       *p_arg,
ulong                   devflag);


/*****************************************************************************/
/* fixed storage area                                                        */
/*****************************************************************************/

dd_ctrl_t dd_ctrl = { 
	1,0,0 };

/*  valid states */
#define CIO_NOSTATE     0x1
#define CIO_CONFIGURED  0x2
#define CIO_OPENED      0x4
#define CIO_ACTIVE      0x8



typedef volatile struct {
	ulong nexthole;                 /* index into table for next add          */
	ulong table[TRACE_TABLE_SIZE];  /* ring buffer of trace data              */
} tracetable_t;

static tracetable_t tracetable;    /* this is the trace table                */

/* enough for trace table and, for each adap, the dds, open structs, and ds  */
#define MAX_CDT_ELEMS (1 + (MAX_ADAPTERS * (1 + MAX_OPENS + MAX_CDT)))

typedef struct {
	struct cdt_head  header;
	struct cdt_entry entry[MAX_CDT_ELEMS];
} cdt_t;

static cdt_t ciocdt;               /* this is the component dump table       */


/*
 * NAME:     cio_save_trace
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
void cio_save_trace (num, str1, arg2, arg3, arg4, arg5)
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
	#ifdef DEBUG
	ulong  trcdata[MAXTRACEARGS];
	#else
	ulong  trcdata[MAXTRACEARGS];
	#endif

	/* first argument is supposed to be a character string */
	s = (char *)trcdata;

	for (ndx=0; ndx<4; ndx++,s++)
		*s = str1[ndx];

	switch (num)
	{
	case 5:  
		trcdata[4] = arg5;
	case 4:  
		trcdata[3] = arg4;
	case 3:  
		trcdata[2] = arg3;
	case 2:  
		trcdata[1] = arg2;
	case 1:

		/* now, for in-memory trace table, move the data */
		for (ndx = 0; ndx < num; ndx++)
		{
			tracetable.table[tracetable.nexthole] = trcdata[ndx];
			if (++tracetable.nexthole > TRACE_TABLE_SIZE)
				tracetable.nexthole = 0;
		}
		/* mark end of table data */
		tracetable.table[tracetable.nexthole] = 0x2A2A2A2A; /* "****" */
	}

	/* for AIX trace, make the call */
	TRCHKGT (TRACE_HOOKWORD | HKTY_GT | num, trcdata[0],
	    arg2, arg3, arg4, arg5);

	return;
} /* end cio_save_trace */


/*
 * NAME:     cio_add_cdt
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
void cio_add_cdt (
register char *name,  /* label string for area dumped */
register char *ptr, /* pointer to area to be dumped */
register int   len)   /* amount of data to be dumped */
{
	struct cdt_entry temp_entry;
	int num_elems;
	int rc;

	TRACE4 ("ACDb", (ulong)name, (ulong)ptr,(ulong)len);

	strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
	temp_entry.d_len = len;
	temp_entry.d_ptr = ptr;
	temp_entry.d_xmemdp = (ulong)NULL;

	num_elems = (ciocdt.header._cdt_len - sizeof(ciocdt.header)) /
	    sizeof(struct cdt_entry);
	if (rc = (num_elems < MAX_CDT_ELEMS))
	{
		ciocdt.entry[num_elems] = temp_entry;
		ciocdt.header._cdt_len += sizeof(struct cdt_entry);
	}

	TRACE2 ("ACDe",(ulong)(!rc)); /* cio_add_cdt end */
	return;
} /* end cio_add_cdt */


/*
 * NAME:     cio_del_cdt
 *
 * FUNCTION: delete an entry from the component dump table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
void cio_del_cdt (
register char *name,  /* label string for area dumped */
register char *ptr,   /* area to be dumped */
register int   len)   /* amount of data to be dumped */
{
	struct cdt_entry temp_entry;
	int num_elems;
	int rc;
	int ndx;

	TRACE4 ("DCDb", (ulong)name, (ulong)ptr,(ulong)len); /* cio_del_cdt begin */

	strncpy (temp_entry.d_name, name, sizeof(temp_entry.d_name));
	temp_entry.d_len = len;
	temp_entry.d_ptr = ptr;
	temp_entry.d_xmemdp = (ulong)NULL;

	num_elems = (ciocdt.header._cdt_len - sizeof(ciocdt.header)) /
	    sizeof(struct cdt_entry);

	/* find the element in the array (match only the memory pointer) */
	for (ndx = 0;
	    (ndx < num_elems) &&
	    (temp_entry.d_ptr != ciocdt.entry[ndx].d_ptr);
	    ndx++)
		; /* NULL statement */

	/* re-pack the array to remove the element if it is there */
	if (ndx < num_elems)
	{
		for (ndx++ ; ndx < num_elems; ndx++)
			ciocdt.entry[ndx-1] = ciocdt.entry[ndx];
		bzero (&ciocdt.entry[ndx-1], sizeof(struct cdt_entry));
		ciocdt.header._cdt_len -= sizeof(struct cdt_entry);
		rc = 0;
	}
	else /* item not in table */
	{
		rc = 1;
	}

	TRACE2 ("DCDe",(ulong)(!rc)); /* cio_del_cdt end */
	return;
} 


/*
 * NAME:     remove_netid
 *
 * FUNCTION: remove specified netid from table
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  TRUE iff item was in table (else FALSE)
 *
 */
static int remove_netid (
register dds_t   *dds_ptr, /* pointer to dds structure */
register chan_t   chan,    /* channel for this user    */
register netid_t  netid)   /* netid to be removed */
{
	int rc;
	int ndx;
	int saved_intr_level;

	TRACE3 ("REMb", (ulong)dds_ptr, (ulong)netid); /* remove_netid begin */
	DISABLE_CL2_INTRS (saved_intr_level);

	/* find the netid element in the array */
	for (ndx = 0; ndx < CIO.num_netids; ndx++) {
		if( ( netid == CIO.netid_table_ptr[ndx].netid) &&
		    ( chan == CIO.netid_table_ptr[ndx].chan))
			break;
	}

	/* re-pack the array to remove the element if it is there */
	if (ndx < CIO.num_netids) {
		for (ndx++ ; ndx < CIO.num_netids; ndx++)
			CIO.netid_table_ptr[ndx-1] = CIO.netid_table_ptr[ndx];
		bzero (&CIO.netid_table_ptr[ndx-1], sizeof(netid_elem_t));
		CIO.num_netids--;
		rc = TRUE;
	}
	else /* item not in table */
	{
		rc = FALSE;
	}
	ENABLE_INTERRUPTS (saved_intr_level);
	TRACE2 ("REMe", (ulong)rc); /* remove_netid end */
	return (rc);
} /* end remove_netid */


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
static int remove_netid_for_chan (
register dds_t   *dds_ptr,  /* pointer to dds structure */
register chan_t   chan)     /* channel for which last netid is to be remvd*/
{
	int rc;
	int ndx;
	int saved_intr_level;

	TRACE3 ("RMCb",(ulong)dds_ptr,(ulong)chan); /* Trace-point 'begin'. */
	DISABLE_CL2_INTRS (saved_intr_level);

	/* find the netid element in the array */
	for (ndx = CIO.num_netids - 1; (ndx >= 0) &&
	    (chan != CIO.netid_table_ptr[ndx].chan); ndx--)
		; /* NULL statement */

	/* re-pack the array to remove the element if it is there */
	if (ndx >= 0)
	{
		for (ndx++ ; ndx < CIO.num_netids; ndx++)
			CIO.netid_table_ptr[ndx-1] = CIO.netid_table_ptr[ndx];
		bzero (&CIO.netid_table_ptr[ndx-1], sizeof(netid_elem_t));
		CIO.num_netids--;
		rc = TRUE;
	}
	else /* item not in table */
	{
		rc = FALSE;
	}
	ENABLE_INTERRUPTS (saved_intr_level);
	TRACE2 ("RMCe", (ulong)rc); /* remove_netid_for_chan end */
	return (rc);
} /* end remove_netid_for_chan */


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

	ll_ptr->num_free = max_elem;/* initially there will be this many free */
	ll_ptr->num_elem = 0;          /* and none will be on the active list */
	ll_ptr->max_elem = max_elem;/* we need this only for consistency */
	ll_ptr->elem_size = elem_size; /* keep size of an element */
	ll_ptr->hwm_ptr = hwm_ptr; /* pointer to high water mark (max queued) */
	ll_ptr->area_ptr = space_ptr;/* caller needs this later to free space */
	ll_ptr->free_ptr = space_ptr; /* the free list will be here initially */
	ll_ptr->head_ptr = NULL;       /* and the active list will be empty */

	*hwm_ptr = 0;                  /* zero high water mark */

	/* run through the allocated space and thread the free list */
	for (prev_next_addr = (sll_elem_ptr_t *)&(ll_ptr->free_ptr), 
	    ptr = space_ptr, num_elem = 0;
	    num_elem < max_elem;
	    prev_next_addr = (sll_elem_ptr_t *)&(ptr->next), ptr = ptr->next, 
	    num_elem++)
	{
		ptr->next = (struct sll_elem_tag *)((ulong)ptr + elem_size);
	}
	ll_ptr->limt_ptr = *prev_next_addr; /* remember address just past end */
	*prev_next_addr = NULL;             /* mark end of list */
	return;
} /* end sll_init_list */


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
sll_elem_ptr_t sll_alloc_elem (
register s_link_list_t *ll_ptr) /* pointer to list control structure */
{
	register sll_elem_ptr_t ptr;

	ptr = ll_ptr->free_ptr;       /* we're going to allocate the first one*/
	if (ptr != NULL) {
		ll_ptr->free_ptr = ptr->next;      /* head now points to next */
		ll_ptr->num_free--;          /* reduce count of free elements */
	}
	if (ptr != NULL)
		bzero (ptr, ll_ptr->elem_size);    /* zero the element */
	return(ptr);
} /* end sll_alloc_elem */


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
void sll_free_elem  (
register s_link_list_t  *ll_ptr,   /* pointer to list control structure */
register sll_elem_ptr_t  elem_ptr) /* pointer to element to be freed */
{
	/* freed element's next is old head */
	elem_ptr->next = (struct sll_elem_tag *)ll_ptr->free_ptr;
	ll_ptr->free_ptr = elem_ptr;       /* put freed item at head */
	ll_ptr->num_free++;                /* increase count of free elements */
	return;
} /* end sll_free_elem */


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
void sll_link_last (
register s_link_list_t  *ll_ptr,   /* pointer to list control structure */
register sll_elem_ptr_t  elem_ptr) /* pointer to element to add to list */
{
	register sll_elem_ptr_t *head_ptr_ptr;
	register sll_elem_ptr_t  ptr;

	/* find the end of the list */
	for (head_ptr_ptr = (sll_elem_ptr_t *)&(ll_ptr->head_ptr), 
	    ptr = *head_ptr_ptr; ptr != NULL;
	    head_ptr_ptr = (sll_elem_ptr_t *)&(ptr->next), 
	    ptr = (sll_elem_ptr_t)ptr->next)
		; /* NULL statement */
	elem_ptr->next = NULL;    /* new element's next marks end of list */
	*head_ptr_ptr = elem_ptr; /* previous element's next is this new element */
	ll_ptr->num_elem++;       /* increase number of elements linked */
	if (*(ll_ptr->hwm_ptr) < ll_ptr->num_elem ) /* update high water mark */
		*(ll_ptr->hwm_ptr) = ll_ptr->num_elem;
	return;
} 


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
sll_elem_ptr_t sll_unlink_first (
register s_link_list_t  *ll_ptr) /* pointer to list control structure */
{
	register sll_elem_ptr_t  ptr;

	ptr = ll_ptr->head_ptr;
	if (ptr != NULL) {
		ll_ptr->head_ptr = ptr->next; /* head now points to next */
		ll_ptr->num_elem--;        /* reduce count of linked elements */
		ptr->next = NULL;       /* removed element now points nowhere */
	}
	return(ptr);
}


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
static void que_stat_blk (
register dds_t          *dds_ptr,      /* pointer to dds structure */
register open_elem_t    *open_ptr,     /* pointer to open structure */
register cio_stat_blk_t *stat_blk_ptr) /* pointer to block to queue */
{
	sta_elem_t *sta_ptr;
	register int            saved_intr_level;

	TRACE5 ("QSBb", (ulong)dds_ptr, (ulong)open_ptr, (ulong)stat_blk_ptr,
	    (ulong)stat_blk_ptr->code); /* que_stat_blk begin */

	DISABLE_CL2_INTRS(saved_intr_level);
	sta_ptr = (sta_elem_t *) sll_alloc_elem((s_link_list_t *)
			(&(open_ptr->sta_que)));
	ENABLE_INTERRUPTS(saved_intr_level);
	if (sta_ptr != NULL) {
		sta_ptr->stat_blk = *stat_blk_ptr;
		DISABLE_CL2_INTRS(saved_intr_level);
		sll_link_last ((s_link_list_t *)(&(open_ptr->sta_que)),
		    (sll_elem_ptr_t)sta_ptr);
		ENABLE_INTERRUPTS(saved_intr_level);

		/* notify any waiting user process there is status available */
		if (open_ptr->selectreq & POLLPRI) {
			open_ptr->selectreq &= ~POLLPRI;
			selnotify (open_ptr->devno, open_ptr->chan, POLLPRI);
		}
	}
	else {
		TRACE2 ("QSB1", (ulong)dds_ptr); /* que_stat_blk ERROR (stat que full) */
		open_ptr->sta_que_ovrflw = TRUE;
		RAS.ds.sta_que_overflow++;
	}

	TRACE1 ("QSBe"); /* que_stat_blk end */
	return;
} /* end que_stat_blk */


/*
 * NAME:     cio_report_status
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
void cio_report_status (
register dds_t          *dds_ptr,      /* pointer to dds structure */
register open_elem_t    *open_ptr,     /* pointer to open structure */
register cio_stat_blk_t *stat_blk_ptr) /* pointer to block to report */
{
	TRACE5 ("STAb", (ulong)dds_ptr, (ulong)open_ptr, (ulong)stat_blk_ptr,
	    (ulong)stat_blk_ptr->code); /* cio_report_status begin */

	if (open_ptr->devflag & DKERNEL) /* data is for a kernel process */
	{
		/* notify kernel process */
		TRACE1 ("KSFc"); /* kernel proc status function call */
		(*(open_ptr->sta_fn)) (open_ptr->open_id, stat_blk_ptr);
		TRACE1 ("KSFr"); /* kernel proc status function returned */
	}
	else /* data is for a user process */
	{
		/* add the status block to the status queue */
		que_stat_blk (dds_ptr, open_ptr, stat_blk_ptr);
	}

	TRACE1 ("STAe"); /* cio_report_status end */
	return;
} /* end cio_report_status */


/*
 * NAME:     cio_proc_recv
 *
 * FUNCTION: process a receive element
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
void cio_proc_recv (
register dds_t       *dds_ptr,  /* pointer to dds structure */
register open_elem_t *open_ptr, /* pointer to open structure */
register rec_elem_t  *rec_ptr)  /* pointer to receive element structure */
{
#ifdef	DEBUG
	TRACE4 ("RECb", (ulong)dds_ptr, (ulong)open_ptr,
	    (ulong)rec_ptr); /* cio_proc_recv begin */
#endif	/* DEBUG */
      	AIXTRACE (TRC_RDAT, open_ptr->devno, open_ptr->chan,
                   rec_ptr->mbufp, rec_ptr->bytes);

	/* update the standard counters */
	if (ULONG_MAX == RAS.cc.rx_frame_lcnt)
		RAS.cc.rx_frame_mcnt++; /* record overflow in msh of counter */
	RAS.cc.rx_frame_lcnt++;
	if ((ULONG_MAX - rec_ptr->bytes) < RAS.cc.rx_byte_lcnt)
		RAS.cc.rx_byte_mcnt++;  /* record overflow in msh of counter */
	RAS.cc.rx_byte_lcnt += rec_ptr->bytes;

	if (open_ptr->devflag & DKERNEL) /* data is for a kernel process */
	{
		/* call read_data_available function (freeing mbuf) */
		TRACE1 ("KRFc"); /* kernel proc read function call */
		(*(open_ptr->rec_fn)) (open_ptr->open_id, &(rec_ptr->rd_ext),
		    rec_ptr->mbufp);
		TRACE1 ("KRFr"); /* kernel proc read function returned */
		AIXTRACE (TRC_RNOT, open_ptr->devno, open_ptr->chan,
		    rec_ptr->mbufp, rec_ptr->bytes);
	}
	else { /* data is for a user process */
		rec_elem_t     *new_rec_ptr;
		cio_stat_blk_t stat_blk;
		register int   saved_intr_level;

		/* actions depend on whether there is room in receive que */
		DISABLE_CL2_INTRS(saved_intr_level);
		new_rec_ptr = (rec_elem_t *) sll_alloc_elem((s_link_list_t *)
				(&(open_ptr->rec_que)));
		ENABLE_INTERRUPTS(saved_intr_level);
		if (new_rec_ptr != NULL) {
			*new_rec_ptr = *rec_ptr;

			/* add the element to the receive queue */
			DISABLE_CL2_INTRS(saved_intr_level);
			sll_link_last ((s_link_list_t *)(&(open_ptr->rec_que)),
			    (sll_elem_ptr_t)new_rec_ptr);
			ENABLE_INTERRUPTS(saved_intr_level);

			AIXTRACE (TRC_RQUE, open_ptr->devno, open_ptr->chan,
			    rec_ptr->mbufp, rec_ptr->bytes);

			/* notify any waiting user there is data available */
			if (open_ptr->selectreq & POLLIN) {
				open_ptr->selectreq &= ~POLLIN;
				selnotify (open_ptr->devno, open_ptr->chan, POLLIN);
			}

			/* if user is blocked on read, wake him up */
			if (open_ptr->rec_event != EVENT_NULL) {
				WAKEUP (&open_ptr->rec_event);
			}
		}
		else
		{
			AIXTRACE (TRC_ROVR, open_ptr->devno, open_ptr->chan,
			    rec_ptr->mbufp, rec_ptr->bytes);
			TRACE2("REC1",(ulong)dds_ptr);/* cio_proc_recv ERROR (recv que full)*/
			m_freem (rec_ptr->mbufp); /* free the mbuf -- lose data */
			RAS.ds.rec_que_overflow++;

			/* now attempt to que a status element to report the overflow */
			stat_blk.code = (ulong)CIO_ASYNC_STATUS;
			stat_blk.option[0] = (ulong)CIO_LOST_DATA;
			que_stat_blk (dds_ptr, open_ptr, &stat_blk);
		}
	}

#ifdef	DEBUG
	TRACE1 ("RECe"); /* cio_proc_recv */
#endif	/* DEBUG */
	return;
} /* end cio_proc_recv */


/*
 * NAME:     cio_xmit_done
 *
 * FUNCTION: process a completed transmission
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
void cio_xmit_done (
register dds_t      *dds_ptr,  /* pointer to dds structure */
register xmt_elem_t *xmt_ptr,  /* pointer to transmit element structure */
ulong       status,   /* status word for status block option[0] */
ulong       stat2)    /* status word for status block option[3] */
{
	register open_elem_t *open_ptr;
	cio_stat_blk_t        stat_blk;
	chan_t                chan;
	int                   saved_intr_level;

#ifdef	DEBUG
	TRACE5 ("XMTb", (ulong)dds_ptr, (ulong)xmt_ptr, (ulong)status,
	    (ulong)stat2); /* cio_xmit_done begin */
#endif	/* DEBUG */

	if (status == CIO_OK) {
		/* update the standard counters */
		if (ULONG_MAX == RAS.cc.tx_frame_lcnt)
			RAS.cc.tx_frame_mcnt++; /* overflow in msh of counter */
		RAS.cc.tx_frame_lcnt++;
		if ((ULONG_MAX - xmt_ptr->bytes) < RAS.cc.tx_byte_lcnt)
			RAS.cc.tx_byte_mcnt++;  /* overflow in msh of counter */
		RAS.cc.tx_byte_lcnt += xmt_ptr->bytes;
	}

       AIXTRACE(TRC_WEND,CIO.devno,xmt_ptr->chan,xmt_ptr->mbufp,xmt_ptr->bytes);

	if ( (xmt_ptr->chan > 0) && 
	    (open_ptr = CIO.open_ptr[xmt_ptr->chan - 1]) != NULL) {
		if ((CIO.chan_state[xmt_ptr->chan - 1] == CHAN_OPENED) &&
		    (xmt_ptr->wr_ext.flag & CIO_ACK_TX_DONE)) {

			stat_blk.code = (ulong)CIO_TX_DONE;
			stat_blk.option[0] = status;
			stat_blk.option[1] = xmt_ptr->wr_ext.write_id;
			stat_blk.option[2] = (ulong) (xmt_ptr->mbufp);
			stat_blk.option[3] = stat2;

			cio_report_status (dds_ptr, open_ptr, &stat_blk);
		}
	}

	if ((xmt_ptr->mbufp ) &&
	    (xmt_ptr->chan != 0 && CIO.chan_state[xmt_ptr->chan - 1] != CHAN_OPENED))
	{
		m_freem (xmt_ptr->mbufp); /* free the mbuf */
	}


	/* since there's definitely an element available now, notify kernel procs */
	DISABLE_CL2_INTRS (saved_intr_level);

	if ( ( xmt_ptr->chan > 0 ) && CIO.xmt_fn_needed ) {
	/* there's at least one kernel process to notify */
		CIO.xmt_fn_needed = FALSE;
		for (chan = 0; chan < MAX_OPENS; chan++) {
			if ((CIO.chan_state[chan] == CHAN_OPENED)     &&
			    ((open_ptr = CIO.open_ptr[chan]) != NULL) &&
			    (open_ptr->xmt_fn_needed)) {
				open_ptr->xmt_fn_needed = FALSE;
				ENABLE_INTERRUPTS (saved_intr_level);
				TRACE1 ("KWFc"); /* kernel proc write function call */
				(*(open_ptr->xmt_fn)) (open_ptr->open_id);
				TRACE1 ("KWFr"); /* kernel proc write function returned */
				DISABLE_CL2_INTRS (saved_intr_level);
			}
		}
	}
	ENABLE_INTERRUPTS (saved_intr_level);

	/* if anyone is blocked on a write, wake them up */
	for (chan = 0; chan < MAX_OPENS; chan++) {
		if ((CIO.chan_state[chan] == CHAN_OPENED)     &&
		    ((open_ptr = CIO.open_ptr[chan]) != NULL) &&
		    (open_ptr->xmt_event != EVENT_NULL)          )
		{
			WAKEUP (&open_ptr->xmt_event);
		}
	}

#ifdef	DEBUG
	TRACE1 ("XMTe"); /* cio_xmit_done end */
#endif	/* DEBUG */
	return;
} /* end cio_xmit_done */


/*
 * NAME:     cio_conn_done
 *
 * FUNCTION: notify all callers in nedid table that start is complete or failed
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  nothing
 *
 */
void cio_conn_done (
register dds_t *dds_ptr) /* pointer to dds structure */
{
	register open_elem_t *open_ptr;
	int                   saved_intr_level;
	int                   ndx;
	chan_t                chan;
	cio_stat_blk_t        stat_blk;
	int                   start_ok;

	TRACE2 ("CONb", (ulong)dds_ptr); /* cio_conn_done begin */

	DISABLE_CL2_INTRS (saved_intr_level);
	start_ok = TRUE;

	/* we need to notify every caller with netid currently in table */
	for (ndx = 0; (ndx < CIO.num_netids); ndx++) {
		chan = CIO.netid_table_ptr[ndx].chan;
		open_ptr = CIO.open_ptr[chan-1];

		if (CIO.chan_state[chan - 1] == CHAN_OPENED) {
			ENABLE_INTERRUPTS (saved_intr_level);
			/* let ds code build status block */
			xxx_startblk (dds_ptr, &CIO.netid_table_ptr[ndx], &stat_blk);

			/* "asynchronous" notification to caller that start complete */
			cio_report_status (dds_ptr, open_ptr, &stat_blk);

			if (stat_blk.option[0] != CIO_OK)
				start_ok = FALSE;
			DISABLE_CL2_INTRS (saved_intr_level);
		}
	}
	if (start_ok)
		CIO.device_state = DEVICE_CONNECTED;
	else {
		CIO.device_state = DEVICE_NOT_CONN;
		for (ndx = 0; (ndx < CIO.num_netids); ndx++) {
			chan = CIO.netid_table_ptr[ndx].chan;
			while(remove_netid_for_chan (dds_ptr, chan));
		}
	}
	ENABLE_INTERRUPTS (saved_intr_level);

	/* if cioclose is blocked on close, wake him up */
	if (CIO.cls_event != EVENT_NULL)
		WAKEUP (&CIO.cls_event);

	TRACE2 ("CONe", CIO.device_state); /* cio_conn_done end */
	return;
} /* end cio_conn_done */


/*
 * NAME:     cio_cdt_func
 *
 * FUNCTION: process component dump table interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  pointer to this driver's component dump table
 *
 */
static cdt_t *cio_cdt_func (void)
{
	return (&ciocdt);
}


/*
 * NAME:     ciompx
 *
 * FUNCTION: mpx entry point from kernel (before open and after close)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns allocated channel number
 *
 */
static int ciompx (
dev_t  devno,     /* major and minor number */
int   *chanp,     /* address for returning allocated channel number */
char  *channame)  /* pointer to special file name suffix */
{
	register dds_t *dds_ptr;
	register int    adap;
	int             saved_intr_level;
	int             ndx;
	int             rc;

	TRACE4 ("MPXb", (ulong)devno, (ulong)(*chanp),
	    (ulong)channame); /* ciompx begin */

	if ( devno != dd_ctrl.devno )
		rc = ENODEV;
	else {
	  if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		rc = ENXIO;
	  else {
		rc = 0;
		if (channame == NULL) { /* this is de-allocate request */
			if (CIO.chan_state[(*chanp)-1] == CHAN_AVAIL)
				rc = ENOMSG;
			else {
				CIO.chan_state[(*chanp)-1] = CHAN_AVAIL;
				CIO.num_allocates--;
			}
		}
		else { /* this is allocate request */
			/* filename extensions are requests for exclusive use */
			/* make sure it's ok to let user get a channel & open */
		  if (((*channame != '\0') && (CIO.num_allocates != 0)) ||
		    (CIO.num_allocates >= MAX_OPENS) || ((*channame =='D') &&
			    (CIO.num_allocates > 0)))
				rc = EBUSY;
		  else {
		    for (ndx=0; (ndx < MAX_OPENS) && (CIO.chan_state[ndx] != 
			CHAN_AVAIL); ndx++) ; /* NULL statement */
		    if (ndx == MAX_OPENS)
		      rc = EBUSY;
		    else {
			CIO.num_allocates++;
			CIO.chan_state[ndx] = CHAN_ALLOCATED;
		    }
		}
		if (rc == 0) { /* allocate was successful */
			if ((*channame != '\0') && (xxx_badext(channame))) {
				/* undo the allocation */
				CIO.chan_state[ndx] = CHAN_AVAIL;
				CIO.num_allocates--;
				rc = ENXIO;
			}
		else {
		  /* save "mode" for device specific code's use */
		  CIO.mode = *channame; /* note this is '\0' or a letter */
		  /* tell kernel the channel number we allocated */
		   *chanp = ndx + 1; /* NOTE WELL! channel number is 1 based */
		}
	      }
	    }
	  }
	}

	TRACE2 ("MPXe", (ulong)rc); /* ciompx end */
	return (rc);
} /* end ciompx */


/*
 * NAME:     cioopen
 *
 * FUNCTION: open entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
static int 
cioopen ( devno, devflag, chan,extptr )
dev_t	devno;   			/* major and minor number */
ulong	devflag; 			/* DKERNEL and DNDELAY flags */
chan_t  chan;    			/* channel number */
cio_kopen_ext_t	*extptr;
{
	int saved_intr_level;
	int alloc_size;
	int rc;
	int dds_size;
	register 	int adap;
	register 	dds_t *dds_ptr;
	open_elem_t 	*open_ptr;

	/*
	 *  sanity checking
	 */
	if (( devno != dd_ctrl.devno ) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		return(ENODEV);
	if (CIO.chan_state[chan-1] != CHAN_ALLOCATED)
		return(ENXIO);

	rc = 0;
	if ( dd_ctrl.state == CIO_CONFIGURED ) {
		/* first open, pin driver and DDS */

		/* compute size needed for entire dds */
		dds_size = sizeof(dds_t);
		dds_size += MAX_NETID * sizeof(netid_elem_t);
		pin( dd_ctrl.dds_ptr, dds_size );	/* pin the DDS */
		pincode( cioopen );			/* pin the driver */
		dd_ctrl.state = CIO_OPENED;
		WRK.sysmem = 0;
  		/* add DDS to dump table */
		DISABLE_CL2_INTRS (saved_intr_level);
		cio_add_cdt ("DDS", (char *)dds_ptr, dds_size);
		ENABLE_INTERRUPTS (saved_intr_level);
		dmp_add (cio_cdt_func); 	 /* register for dump */
	}

	TRACE5 ("OPNb", (ulong)devno, (ulong)devflag, (ulong)chan,
	    (ulong)extptr); /* cioopen begin */

	/* make sure we're not in process of shutting down adapter */
	if (CIO.device_state == DEVICE_DISC_IN_PROG) {
		rc = ENOTREADY; /* disconnect in progress */
	}
	else {
		if (((devflag & DKERNEL) && ((extptr == NULL) ||
			 (extptr->rx_fn == NULL) || (extptr->tx_fn == NULL) ||
		    (extptr->stat_fn == NULL)))) {
			rc = EINVAL;
		}
		else {
			/* allocate memory for open structure */
			alloc_size = sizeof(open_elem_t);
			if (!(devflag & DKERNEL)) {
				/* need space for user process queues */
				alloc_size += DDI.cc.rec_que_size * 
						sizeof(rec_elem_t);
				alloc_size += DDI.cc.sta_que_size * 
						sizeof(sta_elem_t);
			}
			if ((open_ptr = (open_elem_t *) KMALLOC (alloc_size))
					 == NULL) {
				rc = ENOMEM;
			}
			else {
				bzero (open_ptr, alloc_size); 
				open_ptr->alloc_size = alloc_size;
				open_ptr->devno = CIO.devno;
				open_ptr->chan = chan;
				open_ptr->devflag = devflag;
				open_ptr->xmt_event = EVENT_NULL;
				open_ptr->rec_event = EVENT_NULL;
				if (devflag & DKERNEL) {
				  open_ptr->rec_fn = 
					 ((cio_kopen_ext_t *)extptr)->rx_fn;
				  open_ptr->xmt_fn =
					  ((cio_kopen_ext_t *)extptr)->tx_fn;
				  open_ptr->sta_fn = 
					 ((cio_kopen_ext_t *)extptr)->stat_fn;
				  open_ptr->open_id = 
					((cio_kopen_ext_t *)extptr)->open_id;
				}
				else {
					/* initialize receive queue right after open structure */
					sll_init_list((s_link_list_t *)(&(open_ptr->rec_que)),
					    (sll_elem_ptr_t) ((int)open_ptr + sizeof(open_elem_t)),
					    DDI.cc.rec_que_size,
					    (int) sizeof(rec_elem_t),
					    (ulong *)(&(RAS.cc.rec_que_high)));

					/* initialize status queue right after receive queue */
					sll_init_list ((s_link_list_t *)(&(open_ptr->sta_que)),
					    (sll_elem_ptr_t) (open_ptr->rec_que.limt_ptr),
					    DDI.cc.sta_que_size,
					    (int) sizeof(sta_elem_t),
					    (ulong *)(&(RAS.cc.sta_que_high)));
				}

				DISABLE_CL2_INTRS (saved_intr_level);
				CIO.open_ptr[chan-1] = open_ptr;
				CIO.num_opens++;
				CIO.chan_state[chan-1] = CHAN_OPENED;
				ENABLE_INTERRUPTS (saved_intr_level);


				if (!CIO.timr_registered) {
				/* add watchdog timer routine to kernel list */
					w_init ((struct watchdog *)(&(WDT)));
					CIO.timr_registered = TRUE;
				}

				/* add open structure to component dump table */
				DISABLE_CL2_INTRS (saved_intr_level);
				cio_add_cdt ("OpenStrc",(char *)open_ptr,
						open_ptr->alloc_size);
				ENABLE_INTERRUPTS (saved_intr_level);

			} /* end else for bad open pointer */
		} /* end else for bad kernel arguments */
	} /* end else for disconnect in progress */

	TRACE2 ("OPNe", (ulong)rc); /* cioopen end */
	return (rc);
} /* end cioopen */


/*
 * NAME:     cioclose
 *
 * FUNCTION: close entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
static int cioclose (
dev_t  devno,  /* major and minor number */
chan_t chan)   /* channel number */
{
	int             rc;
	register dds_t *dds_ptr;
	register int    adap;
	open_elem_t    *open_ptr;
	int             saved_intr_level;
	int             enter_state;
	rec_elem_t     *rec_ptr;
	xmt_elem_t     *xmt_ptr;

	TRACE3 ("CLOb", (ulong)devno, (ulong)chan); /* cioclose begin */

	/* sanity checking */
	if (( devno != dd_ctrl.devno ) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		return(ENODEV);
	if ((CIO.chan_state[chan-1] != CHAN_OPENED)  ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL))
		return(ENXIO);

	rc = 0;

	/* make sure nobody else uses this channel while we're closing */
	DISABLE_CL2_INTRS (saved_intr_level);
	CIO.open_ptr[chan-1] = NULL;
	ENABLE_INTERRUPTS (saved_intr_level);

	/* this may be abnormal close w/o shutting down ds ioctl's */
	xxx_close (dds_ptr, open_ptr);

	/* this may be abnormal close w/o halts -- make sure all netid's gone */
	/* break out when no more entries for this channel found */
	while(remove_netid_for_chan(dds_ptr, chan));

	if (!(open_ptr->devflag & DKERNEL)) /* this is user task */
	{
		/* free the mbuf's for all elements on receive que */
		/* new ones cannot appear because there's no netid's for this user */
		DISABLE_CL2_INTRS (saved_intr_level);
		while ((rec_ptr = (rec_elem_t *) sll_unlink_first (
		    (s_link_list_t *)(&(open_ptr->rec_que)))) != NULL) {
			m_freem (rec_ptr->mbufp);
		}
		ENABLE_INTERRUPTS (saved_intr_level);
	}

	/* delete open structure from component dump table */
	DISABLE_CL2_INTRS (saved_intr_level);
	cio_del_cdt ("OpenStrc", (char *)open_ptr, open_ptr->alloc_size);
	ENABLE_INTERRUPTS (saved_intr_level);

	/* free the open structure (and user process rec que and sta que) */
	if (KMFREE(open_ptr) != 0)
		TRACE2 ("CLO1", (ulong)open_ptr); /* ERROR free failed */

	DISABLE_CL2_INTRS (saved_intr_level);
	CIO.chan_state[chan-1] = CHAN_CLOSED;

	if (--CIO.num_opens == 0) { /* this is last close */
		enter_state = CIO.device_state;
		CIO.device_state = DEVICE_DISC_IN_PROG;  /* block open() */

		if(enter_state == DEVICE_CONN_IN_PROG) {
			if (SLEEP (&CIO.cls_event) != EVENT_SUCC) {
				TRACE2 ("CLO4",
				    (ulong)EINTR); /* sleep interrupted */
			}
		}

		CIO.device_state = DEVICE_NOT_CONN;
		ENABLE_INTERRUPTS (saved_intr_level);

		if (CIO.timr_registered) {
			w_stop (&(WDT));
			w_clear (&(WDT));
			CIO.timr_registered = FALSE;
		}


		/* delete dds from component dump table */
		DISABLE_CL2_INTRS (saved_intr_level);
		cio_del_cdt ("DDS", (char *)dds_ptr, CIO.alloc_size);
		ENABLE_INTERRUPTS (saved_intr_level);
		dmp_del (cio_cdt_func); 	/* un-register for dump */

		/* be sure to unregister interrupt before inactivating device */
		(void) xxx_inact (dds_ptr); /* allow shutdown and cleanup */

		/* consistency check - trace points indicates cleanup problem */
		if (CIO.num_netids != 0)
			TRACE2 ("CLO2", (ulong)CIO.num_netids); /* Error. */
		dd_ctrl.state = CIO_CONFIGURED;
	}
	else {
		ENABLE_INTERRUPTS (saved_intr_level);
	}

	TRACE2 ("CLOe", (ulong)rc); /* cioclose end */
	return (rc);
} /* end cioclose */


/*
 * NAME:     cioread
 *
 * FUNCTION: read entry point from kernel (user processes only)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns indirectly the number of bytes
 *           read through the updating of uiop->uio_resid by uiomove
 *
 */
static int cioread (
dev_t           devno,  /* major and minor number */
struct uio     *uiop,   /* pointer to uio structure */
chan_t          chan,   /* channel number */
cio_read_ext_t *extptr) /* optional pointer to read extension structure */
{
	register dds_t *dds_ptr;
	register int    adap;
	open_elem_t    *open_ptr;
	rec_elem_t     *rec_ptr;
	cio_read_ext_t  local_rd_ext;
	int             rc;
	int             saved_intr_level;
	int             ndx;
	struct mbuf    *tempmbufp;
	int             total_bytes;
	int             bytes_to_move;

	TRACE5 ("REAb", (ulong)devno, (ulong)uiop, (ulong)chan,
	    (ulong)extptr); /* cioread begin */


	/*
         *  sanity checking
         */
	if (( devno != dd_ctrl.devno ) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		return(ENODEV);
	if ((CIO.chan_state[chan-1] != CHAN_OPENED)  ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL))
		return(ENXIO);


	if (open_ptr->devflag & DKERNEL) /* illegal call from kernel process */
	{
		TRACE2 ("REA2", (ulong)EACCES); /* cioread end (call from kernel proc) */
		return (EACCES);
	}

	/* don't allow reads unless a connection has been completed in the past */
	if (CIO.device_state != DEVICE_CONNECTED)
	{
		TRACE2 ("REA3", (ulong)ENOCONNECT); /* cioread end (not connected) */
		return (ENOCONNECT);
	}

	DISABLE_CL2_INTRS (saved_intr_level);
	/* get receive element if there is any data to be read */
	while ((rec_ptr = (rec_elem_t *) sll_unlink_first (
	    (s_link_list_t *)(&(open_ptr->rec_que)))) == (rec_elem_t *)NULL)
	{
		/* there's no data -- maybe because there's no netid in table for us */
		rc = EIO;
		for (ndx = 0; ndx < CIO.num_netids; ndx++)
		{
			if (CIO.netid_table_ptr[ndx].chan == chan)
			{
				rc = 0;
				break;
			}
		}
		if (rc == EIO)
		{
			ENABLE_INTERRUPTS (saved_intr_level);
			/* handle problem of read without start currently in effect */
			local_rd_ext.status = (ulong)CIO_NOT_STARTED;
			(void) COPYOUT (open_ptr->devflag, &local_rd_ext, 
			    extptr, sizeof(local_rd_ext));
			TRACE2 ("REA4", (ulong)EIO); /* cioread end (no active netid) */
			return (EIO);
		}

		/* well, there's simply no data so action depends on DNDELAY */
		if (uiop->uio_fmode & DNDELAY) /* user doesn't want to block */
		{
			ENABLE_INTERRUPTS (saved_intr_level);
			TRACE2 ("REA5", (ulong)0); /* cioread end (no data with DNDELAY) */
			return (0); /* user will see 0 bytes read */
		}

		/* block by sleeping until there's data (wakeup in cio_proc_recv) */
		if (SLEEP (&open_ptr->rec_event) != EVENT_SUCC)
		{
			ENABLE_INTERRUPTS (saved_intr_level);
			TRACE2 ("REA6",
			    (ulong)EINTR); /* cioread end (blocking sleep interrupted) */
			return (EINTR);
		}

		/* now we'll loop back up to while and try again to get rec element */
	}
	ENABLE_INTERRUPTS (saved_intr_level);

	/* at this point, we have data for user and will pass ds rd_ext */
	rc = 0;

	/* calculate total bytes available */
	for (tempmbufp = rec_ptr->mbufp, total_bytes = 0;
	    tempmbufp != NULL;
	    tempmbufp = tempmbufp->m_next)
	{
		total_bytes += tempmbufp->m_len;
	}

	/* if we have too much data, action depends on existence of extptr */
	if (total_bytes > uiop->uio_resid)
	{
		if (extptr == NULL) /* there's no other way to report error */
			rc = EMSGSIZE;
		else /* we can supply part of data and report overflow in *extptr */
		{
			rec_ptr->rd_ext.status = (ulong)CIO_BUF_OVFLW;
			total_bytes = uiop->uio_resid;
		}
	}

	/* if user has room or a rd_ext was provided for status then move the data*/
	for (tempmbufp = rec_ptr->mbufp;
	    (rc == 0) && (total_bytes > 0);
	    total_bytes -= bytes_to_move, tempmbufp = tempmbufp->m_next)
	{
		bytes_to_move = tempmbufp->m_len;
		if (bytes_to_move > total_bytes) /* limit data from this mbuf */
			bytes_to_move = total_bytes;
		if (uiomove (MTOD(tempmbufp,uchar *), bytes_to_move, UIO_READ, uiop))
			rc = EFAULT;
	}

	AIXTRACE (TRC_REND, devno, chan, rec_ptr->mbufp, rec_ptr->bytes);

	/* free the mbuf(s) */
	m_freem (rec_ptr->mbufp);

	/* if no error, update caller's read extension if one is provided */
	if (rc == 0)
		(void) COPYOUT (open_ptr->devflag, &(rec_ptr->rd_ext), 
		    extptr, sizeof(rec_ptr->rd_ext));

	/* free the rec element */
	DISABLE_CL2_INTRS(saved_intr_level);
	sll_free_elem ((s_link_list_t *)(&(open_ptr->rec_que)),
	    (sll_elem_ptr_t)rec_ptr);
	ENABLE_INTERRUPTS(saved_intr_level);

	TRACE2 ("REAe", (ulong)rc); /* cioread end */
	return (rc);
} /* end cioread */


/*
 * NAME:     cioselect
 *
 * FUNCTION: select entry point from kernel (user processes only)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns bits for requested events that
 *           are found to be true
 *
 */
static int cioselect (
dev_t   devno,   /* major and minor number */
ushort  events,  /* requested events */
ushort *reventp, /* address for returning detected events */
int     chan)    /* channel number */
{
	register dds_t *dds_ptr;
	register int adap;
	open_elem_t *open_ptr;
	ulong	ipri;

	TRACE5 ("SELb", (ulong)devno, (ulong)events, (ulong)*reventp,
	    (ulong)chan); /* cioselect begin */


	/*
         *  sanity checking
         */
	if (( devno != dd_ctrl.devno ) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		return(ENODEV);
	if ((CIO.chan_state[chan-1] != CHAN_OPENED)  ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL))
		return(ENXIO);


	if (open_ptr->devflag & DKERNEL) /* illegal call from kernel process */
	{
		TRACE2 ("SEL2",(ulong)EACCES); /* cioselect end (call from kernel proc)*/
		return (EACCES);
	}

	ipri = i_disable( DDI.cc.intr_priority );

	*reventp = 0;                   /* initialize return value */
	if ((events & ~POLLSYNC) == 0) /* no events requested */
	{
		i_enable( ipri );
		return (0);
	}

	/* set return status for all requested events that are true */
	if (events & POLLOUT)
		if (CIO.xmt_que.num_elem < DDI.cc.xmt_que_size)
			*reventp |= POLLOUT;
	if (events & POLLIN)
		if (open_ptr->rec_que.num_elem > 0)
			*reventp |= POLLIN;
	if (events & POLLPRI)
		if (open_ptr->sta_que.num_elem > 0)
			*reventp |= POLLPRI;

	/*
   If there were any device-specific select events defined, each driver would
   have to implement a function to make this call valid, returning zero if
   there were no device-specific events defined or available
   *reventp |= xxx_select (dds_ptr, open_ptr, events);
*/

	/* if no requested event was found, then see if async notification wanted */
	if (*reventp == 0)
	{
		if (!(events & POLLSYNC)) /* this is asynchronous request */
		{
			/* set flags so later notification with selnotify will be done */
			open_ptr->selectreq |= events;
		}
	}

	i_enable( ipri );
	TRACE2 ("SELe", (ulong)0); /* cioselect end */
	return (0);
} /* end cioselect */


/*
 * NAME:     cioioctl
 *
 * FUNCTION: ioctl entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */

static int cioioctl (
dev_t   devno,    /* major and minor number */
int     cmd,      /* ioctl operation desired */
int     arg,      /* argument for this command (usually a structure ptr) */
ulong   devflag,  /* flags including DKERNEL */
chan_t  chan,     /* channel number */
int     ext)      /* optional additional argument */
{
	register dds_t   *dds_ptr;
	cio_stat_blk_t    stat_blk;
	sta_elem_t       *sta_ptr;
	cio_sess_blk_t    sess_blk;
	open_elem_t      *open_ptr;
	int               rc;
	int               rc_sv;
	netid_elem_t      netid_elem;
	int               ndx;
	int               saved_intr_level;
	cio_query_blk_t   qparms;
	int               bytes_to_move;
	int               just_activated;
	ent_query_stats_t *temp_ras_ptr;
	tracetable_t      *tracetable_ptr;

	TRACE5 ("IOCb", (ulong)devno, (ulong)cmd, (ulong)arg,
	    (ulong)devflag); /* cioioctl begin */
	TRACE3 ("IOC+", (ulong)chan, (ulong)ext); /* cioioctl additional args */


	/*
         *  sanity checking
         */
	if (( devno != dd_ctrl.devno ) || (chan <= 0))
		return(ENXIO);
	if (chan > MAX_OPENS)
		return(EBUSY);
	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		return(ENODEV);
	if ((CIO.chan_state[chan-1] != CHAN_OPENED)  ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL))
		return(ENXIO);


	/* handle standard ioctl's */
	switch (cmd)
	{
	case IOCINFO: /* let device-specific code handle this */
		rc = xxx_ioctl (dds_ptr, open_ptr, cmd, arg, devflag, ext);
		break;

	case CIO_START: /* kernel or user process */
		/* get parameter block */
		if (rc = COPYIN (devflag, arg, &sess_blk, sizeof(sess_blk)))
		{
			break;
		}
		netid_elem.chan = chan;
		netid_elem.netid = sess_blk.netid;
		netid_elem.length = sess_blk.length;

		DISABLE_CL2_INTRS (saved_intr_level);

		if (CIO.num_netids >= MAX_NETID)
		{
			ENABLE_INTERRUPTS (saved_intr_level);
			sess_blk.status = CIO_NETID_FULL;
			if (!COPYOUT (devflag, &sess_blk, arg, sizeof(sess_blk)))
				rc = ENOSPC; /* netid table full */
			break;
		}

		for (ndx = 0, rc = 0; (ndx < CIO.num_netids) && (rc == 0); ndx++)
		{
			if (netid_elem.netid == CIO.netid_table_ptr[ndx].netid)
			{
				sess_blk.status = CIO_NETID_DUP;
				rc = EADDRINUSE; /* duplicate netid */
			}
		}
		if (rc) /* then netid was a duplicate */
		{
			ENABLE_INTERRUPTS (saved_intr_level);
			/* save rc, because COPYOUT macro either set rc equal to */
			/* errno or 0.                                           */
			rc_sv = rc;
			if (rc = COPYOUT (devflag, &sess_blk, arg, sizeof(sess_blk)))
				break;
			rc = rc_sv;
			break;
		}

		/* at this point, the net id is acceptable and will go into table */
		CIO.netid_table_ptr[CIO.num_netids] = netid_elem;
		CIO.num_netids++;
		if (RAS.cc.nid_tbl_high < CIO.num_netids) /* update hi water mark */
			RAS.cc.nid_tbl_high = CIO.num_netids;

		just_activated = FALSE; /* dsact could cause conn_done very quickly */
		if (CIO.device_state == DEVICE_NOT_CONN)
		{
			just_activated = TRUE;
			CIO.device_state = DEVICE_CONN_IN_PROG;

			/* activate ("open" or "start" or "connect") the adapter */
			ENABLE_INTERRUPTS (saved_intr_level);
			if (rc = xxx_act (dds_ptr))
			{
				DISABLE_CL2_INTRS (saved_intr_level);
				CIO.device_state = DEVICE_NOT_CONN;
				ENABLE_INTERRUPTS (saved_intr_level);

				/* return sess_blk.status to user */
				sess_blk.status = CIO_NOT_STARTED;
				/* save rc, because COPYOUT macro either set rc equal to */
				/* errno or 0.                                           */
				rc_sv = rc;
				if (rc = COPYOUT (devflag, &sess_blk, arg, sizeof(sess_blk)))
					break;
				rc = rc_sv;

				(void) remove_netid (dds_ptr, open_ptr->chan, netid_elem.netid);
				break;
			}
		}
		else
		{
			if( WRK.pos_request )
				entsetpos( dds_ptr );
			WRK.adapter_state = STARTED;
			ENABLE_INTERRUPTS (saved_intr_level);
		}

		/* Careful! We may have just activated the adapter and it might     */
		/* have finished very quickly.  It that is the case, then           */
		/* cio_conn_done has already processed the START_DONE for this one. */
		if ((CIO.device_state == DEVICE_CONNECTED) && (!just_activated))
		{
			/* let ds code build status block */
			xxx_startblk (dds_ptr, &netid_elem, &stat_blk);

			/* "asynchronous" notification to caller that start complete */
			cio_report_status (dds_ptr, open_ptr, &stat_blk);

		}

		/* return sess_blk.status to user */
		sess_blk.status = CIO_OK;
		rc = COPYOUT (devflag, &sess_blk, arg, sizeof(sess_blk));

		break;

	case CIO_HALT: /* kernel or user process */
		if (CIO.num_netids == 0)
		{
			rc = EINVAL; /* no netid's at all */
			break;
		}
		if (rc = COPYIN (devflag, arg, &sess_blk, sizeof(sess_blk)))
		{
			break;
		}
		if (remove_netid (dds_ptr, open_ptr->chan, sess_blk.netid))
		{
			/* let device specific code perform halt functions */
			xxx_halt (dds_ptr, open_ptr, &sess_blk);

			sess_blk.status = CIO_OK;
			rc = COPYOUT (devflag, &sess_blk, arg, sizeof(sess_blk));
		}
		else
		{
			/* return sess_blk.status to user */
			sess_blk.status = CIO_NETID_INV;
			if(!COPYOUT (devflag, &sess_blk, arg, sizeof(sess_blk)))
				rc = EINVAL; /* netid not in table */
		}
		break;

	case CIO_QUERY: /* expect this to be restricted to priviledged procs */
		/* get the caller's parameters */
		if (rc = COPYIN (devflag, arg, &qparms, sizeof(qparms)))
			break;

		/* get a temporary area for ras so read and/or clear can be sync'ed */
		if ((temp_ras_ptr = (ent_query_stats_t *) KMALLOC(sizeof(RAS)))
			 == NULL) {
			rc = ENOMEM;
			break;
		}

		/* let ds code to look at the data on adapter first */
		rc = xxx_ioctl (dds_ptr, open_ptr, cmd, arg, devflag, ext);

		/* get and/or clear RAS area */
		DISABLE_CL2_INTRS(saved_intr_level);
		bcopy (&RAS, temp_ras_ptr, sizeof(RAS));
		if (qparms.clearall == CIO_QUERY_CLEAR)
			bzero (&RAS, sizeof(RAS));
		ENABLE_INTERRUPTS(saved_intr_level);

		/* limit bytes moved */
		bytes_to_move = sizeof(RAS);
		if (bytes_to_move > qparms.buflen) {
			qparms.status = CIO_BUF_OVFLW;
			bytes_to_move = qparms.buflen;
		}
		else {
			qparms.status = CIO_OK;
		}

		/* copy data to caller's buffer */
		rc = COPYOUT (devflag, temp_ras_ptr,
		    qparms.bufptr, bytes_to_move);

		/* de-allocate the temporary area */
		if (KMFREE(temp_ras_ptr) != 0) {
			TRACE2 ("QRY1", (ulong)temp_ras_ptr); /* free failed */
		}

		/* if the copyout failed, return */
		if (rc)
			break;

		/* return parameter block to caller */
		rc = COPYOUT (devflag, &qparms, arg, sizeof(qparms)); 

		/* if all is well (except caller didn't want all the data) set EIO */
		if ((rc == 0) && (qparms.status == CIO_BUF_OVFLW))
			rc = EIO;
		break;

	case CIO_GET_STAT: /* user process only */
		if (devflag & DKERNEL) /* illegal call from kernel process */
		{
			rc = EACCES;
			break;
		}

		DISABLE_CL2_INTRS (saved_intr_level);
		if (open_ptr->sta_que_ovrflw)
		{
			open_ptr->sta_que_ovrflw = FALSE;
			stat_blk.code = (ulong)CIO_LOST_STATUS;
			ENABLE_INTERRUPTS (saved_intr_level);
		}
		else
		{
			/* get the next status que element */
			sta_ptr=(sta_elem_t *)sll_unlink_first(&(open_ptr->sta_que));
			ENABLE_INTERRUPTS (saved_intr_level);
			if (sta_ptr == NULL)
			{
				stat_blk.code = (ulong)CIO_NULL_BLK;
			}
			else
			{
				/* get useful stuff for user */
				stat_blk = sta_ptr->stat_blk;

				/* free the status que element */
				DISABLE_CL2_INTRS(saved_intr_level);
				sll_free_elem ((s_link_list_t *)
				(&(open_ptr->sta_que)),(sll_elem_ptr_t)sta_ptr);
				ENABLE_INTERRUPTS(saved_intr_level);
			}
		}
		rc = COPYOUT (devflag, &stat_blk, arg, sizeof(stat_blk));
		break;

	case CCC_GET_VPD: /* kernel or user process */
		/* return whole structure with status, length, (data) */
		VPD.length = sizeof(VPD);
		VPD.status = 0;
		rc = COPYOUT (devflag, &VPD, arg, sizeof(VPD));
		break;

	case CCC_TRCTBL: /* return tracetable information */
		tracetable_ptr = &tracetable;
		rc = COPYOUT (devflag, &tracetable_ptr, arg, sizeof(tracetable_ptr));
		break;

	case CIO_GET_FASTWRT:
		rc = ent_fastwrt_entry(open_ptr, arg, devflag);
		break;

	default:
		if ((cmd & 0xFF00) == CIO_IOCTL) /* must be device-specific ioctl */
			rc = xxx_ioctl (dds_ptr, open_ptr, cmd, arg, devflag, ext);
		else
			rc = EINVAL;
		break;
	} /* end switch (cmd) */

	TRACE2 ("IOCe", (ulong)rc); /* cioioctl end */
	return (rc);
} /* end cioioctl */


/*
 * NAME:     config_init
 *
 * FUNCTION: process cioconfig entry with cmd of INIT
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
static int config_init (
dev_t       devno, /* major and minor number */
struct uio *uiop)  /* pointer to uio structure */
{
	register dds_t *dds_ptr;
	int    adap;
	ddi_t  tempddi;
	int    dds_size;
	int    saved_intr_level;

	TRACE3 ("CFIb", (ulong)devno, (ulong)uiop); /* config_init begin */

	if ( devno == dd_ctrl.devno )
	{  /* dds already supplied */
		TRACE2 ("CFI1", (ulong)EEXIST); /* config_init end (DDS already exists)*/
		return (EEXIST);
	}

	/* make sure length is sensical */
	if (uiop->uio_resid != sizeof(ddi_t))
	{
		TRACE2 ("CFI2", (ulong)EINVAL); /* config_init end (wrong length DDI) */
		return (EINVAL);
	}

	/* get temporary copy of ddi (it's normally in user space) */
	if (uiomove ((caddr_t)&tempddi, (int)sizeof(tempddi), UIO_WRITE, uiop))
	{
		TRACE2 ("CFI3",
		    (ulong)EFAULT); /* config_init end (uiomove of DDI failed) */
		return (EFAULT);
	}

        /* ensure that the xmt_que_size is a multiple of eight */
        if( (tempddi.cc.xmt_que_size % 8) != 0 )
                tempddi.cc.xmt_que_size += 8 - (tempddi.cc.xmt_que_size % 8);

	/* compute size needed for entire dds (depends on ddi contents) */
	dds_size = sizeof(dds_t);
	dds_size += MAX_NETID * sizeof(netid_elem_t);

	/* get memory for dds */
	if ((dds_ptr = (dds_t *) xmalloc( dds_size, 12, kernel_heap)) == NULL)
	{
		TRACE2 ("CFI4",
		    (ulong)ENOMEM); /* config_init end (malloc DDS area failed) */
		return (ENOMEM);
	}
	bzero (dds_ptr, dds_size); /* xmalloc does NOT zero storage provided */

	/* now move the temporary copy used for checking to the right place */
	bcopy (&tempddi, &(DDI), sizeof(ddi_t));

	/* finish initialization of DDS */
	CIO.alloc_size = dds_size;   /* save this for use with cio_del_cdt */
	CIO.devno = devno;           /* save this for use with selnotify */
	CIO.cls_event = EVENT_NULL;  /* inital value of the close event */

	/* Because we have a number of data bytes in the rfd we'll have
	 * to offset the type_offset field for the netid. */
	DDI.ds.type_field_off -= NBR_DATA_BYTES_IN_RFD;
	DDI.ds.net_id_offset  -= NBR_DATA_BYTES_IN_RFD;

	/* set up the interrupt control structure section */
	IHS.next = (struct intr *) NULL;
	IHS.handler = xxx_intr;
	IHS.bus_type = DDI.cc.bus_type;
	IHS.flags = 0;
	IHS.level = DDI.cc.intr_level;
	IHS.priority = DDI.cc.intr_priority;
	DDI.cc.bus_id &= (0xFFFFFFBF);
	IHS.bid = DDI.cc.bus_id;

	/* set up the watchdog timer control structure section  */

	WDT.func = wdt_intr;
	WDT.restart = 10;
	WDT.count = 0;
	WDT.next = 0;
	WDT.prev = 0;

	/* initialize the netid table address just past the DDS */
	CIO.netid_table_ptr = (netid_elem_t *) ((int)dds_ptr + sizeof(dds_t));

	/* perform any device-specific init of dds and set POS registers*/
	/* if we return non-zero, then the device cannot be configured */
	if (xxx_initdds (dds_ptr)) {
		/* give back the dds memory */
		if (KMFREE (dds_ptr) != 0)
			TRACE2 ("CFI5", (ulong)dds_ptr); /* free dds failed */
		TRACE2 ("CFI6", (ulong)ENXIO); /* xxx_initdds failed */
		return (ENXIO);
	}

	/* update device driver controls */
	DISABLE_CL2_INTRS (saved_intr_level);
	adap = minor(devno);
	dd_ctrl.dds_ptr = dds_ptr;
	dd_ctrl.devno = devno;
	ENABLE_INTERRUPTS (saved_intr_level);

	TRACE2 ("CFIe", (ulong)0); /* config_init end */
	return (0);
} /* end config_init */


/*
 * NAME:     config_term
 *
 * FUNCTION: process cioconfig entry with cmd of TERM
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
static int config_term (
	int adap) 
{
	dds_t 	*dds_ptr;
	int    	saved_intr_level;
	ulong	alloc_size;
	ulong	num_allocs;

	TRACE2 ("CFTb", (ulong)adap); 	 /* config_term begin */

	dds_ptr = dd_ctrl.dds_ptr;
	dd_ctrl.dds_ptr = NULL;		 /* serializing with ciompx */

	TRACE2 ("CFTd", (ulong)dds_ptr); /* config_term dds_ptr */

	if (dds_ptr == NULL) 		 /* don't have dds to terminate */
	{
		dd_ctrl.dds_ptr = dds_ptr;
		ENABLE_INTERRUPTS(saved_intr_level);
		TRACE2 ("CFT1", (ulong)ENOENT); /* config_term end (no DDS) */
		return (ENOENT);
	}

	alloc_size = CIO.alloc_size;
	num_allocs = CIO.num_allocates;

	DISABLE_CL2_INTRS(saved_intr_level);

	if ( num_allocs != 0) /* adapter is in use */
	{
		dd_ctrl.dds_ptr = dds_ptr;
		ENABLE_INTERRUPTS(saved_intr_level);
		TRACE2 ("CFT2", (ulong)EBUSY); 		/* (DDS is busy) */
		return (EBUSY);
	}

	/* update states and counters */
	dd_ctrl.devno = (dev_t)NULL;
	ENABLE_INTERRUPTS(saved_intr_level);

	/* give back the dds memory */
	if ( xmfree (dds_ptr, kernel_heap ) != 0)
		TRACE2 ("CFT3",
		    (ulong)dds_ptr); /* config_term ERROR (free dds_ptr failed) */

	TRACE2 ("CFTe", (ulong)0); /* config_term end */
	return (0);
} /* end config_term */


/*
 * NAME:     config_qvpd
 *
 * FUNCTION: process cioconfig entry with cmd of QVPD
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
static int config_qvpd (
register dds_t      *dds_ptr, /* pointer to dds structure */
struct uio *uiop)    /* pointer to uio structure */
{
	TRACE3 ("CFQb", (ulong)dds_ptr, (ulong)uiop); /* config_qvpd begin */

	if (dds_ptr == NULL) /* don't have dds */
	{
		TRACE2 ("CFQ1", (ulong)ENOENT); /* config_qvpd end (no DDS) */
		return (ENOENT);
	}

	/* make sure length is sensical */
	if (uiop->uio_resid != sizeof(VPD))
	{
		TRACE2 ("CFQ2",
		    (ulong)EINVAL); /* config_qvpd end (wrong length data area) */
		return (EINVAL);
	}

	/* move vital product data section of DDS to user space) */
	if (uiomove ((caddr_t)&VPD, (int)sizeof(VPD), UIO_READ, uiop))
	{
		TRACE2 ("CFQ3",
		    (ulong)EFAULT); /* config_qvpd end (uiomove of VPD failed) */
		return (EFAULT);
	}

	TRACE2 ("CFQe", (ulong)0); /* config_qvpd end */
	return (0);
} /* end config_qvpd */


/*
 * NAME:     cioconfig
 *
 * FUNCTION: cioconfig entry point from kernel
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *
 */
int cioconfig (
dev_t       devno,  /* major and minor number */
int         cmd,    /* operation desired (INIT, TERM, QVPD) */
struct uio *uiop)   /* pointer to uio structure */
{
	int 	adap;
	int 	rc, first_time;
	ulong	ipri;

	/* cioconfig begin*/
	TRACE4 ("CFGb", (ulong)devno, (ulong)cmd, (ulong)uiop);



	switch (cmd)
	{
	case CFG_INIT:
		/* first config INIT must initialize driver */
		first_time = FALSE;
		if ( dd_ctrl.state != CIO_NOSTATE )
			break;
		else
		{
			first_time = TRUE;
			/* try to add ourselves to switch table */
			if (rc = local_devswadd(devno))
				break;

			/* initialize component dump table */
			ciocdt.header._cdt_magic = DMP_MAGIC;
			strncpy (ciocdt.header._cdt_name, DD_NAME_STR,
			    sizeof(ciocdt.header._cdt_name));
			ciocdt.header._cdt_len = sizeof(ciocdt.header);

			/* allow any device specific initialization */
			xxx_init ();


			/* try to add a DDS */
			if ( (rc = config_init(devno, uiop)) && first_time)
			{
				devswdel(devno);
				dd_ctrl.state = CIO_NOSTATE;
			}
			else
			{
				dd_ctrl.state = CIO_CONFIGURED;
			}
		}
		break;

	case CFG_TERM:
		/*
			 *  unconfigure device
			 *  ensure that it was previously configured
			 */
		if ( dd_ctrl.state == CIO_NOSTATE )
		{
			rc = EACCES;
			break;
		}

		/* possible errors are ENOENT and EBUSY */
		if( (rc = config_term (adap)) != 0 )
			break;

		/* try to remove ourselves from switch table */
		/* possible errors are EEXIST, ENODEV, and EINVAL */
		if (!(rc = devswdel(devno)))
		{
			/* delete trace table from component dump table */
			ipri = i_disable(INTCLASS2);
			cio_del_cdt ("TraceTbl", (char *)(&tracetable),
			    (int)sizeof(tracetable));
			i_enable(ipri);

			/* un-register for dump */
			dmp_del (cio_cdt_func);

			dd_ctrl.state = CIO_NOSTATE; /* allow re-initialization */

			/* this is NOT an error, but a confirmation we're gone */
			rc = 0;
		}
		break;

	case CFG_QVPD:
		/* we can't do this unless there has been a CFG_INIT */
		/*
			 *  query VPD
			 *  the device must have been previously configured
			 */
		if ( dd_ctrl.state == CIO_NOSTATE )
		{
			rc = EACCES;
			break;
		}

		/* try to move vpd */
		/* possible errors are ENOENT, EINVAL, and EFAULT */
		rc = config_qvpd (dd_ctrl.dds_ptr, uiop);
		break;

	default:
		rc = EINVAL;
	} /* end switch (cmd) */

	TRACE2 ("CFGe", (ulong)rc); /* cioconfig end */
	return (rc);
} /* end cioconfig */


/*
 * NAME:     ciowrite
 *
 * FUNCTION: write entry point from kernel
 *	temp spot, will reside in ciodd
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns indirectly the number of bytes
 *           written through the updating of uiop->uio_resid by uiomove
 *
 */
static int 
ciowrite ( 
dev_t	devno,  		/* major and minor number */
struct	uio    *uiop,    	/* pointer to uio structure */
chan_t	chan,    		/* channel number */
cio_write_ext_t	*extptr )	/* optional ptr to wrt ext struct */
{
	register dds_t	*dds_ptr;
	open_elem_t     *open_ptr;
	struct mbuf     *mbufp, *m;
	cio_write_ext_t wr_ext;
	xmt_elem_t      *xmt_ptr;
	ulong	total_bytes, bytes;
	ulong	total_areas;
	ulong	ipri;			/* saved interrupt level */
	ulong	i,k;			/* counter */
	ulong	ioaddr;
	ushort  tcb_cnt;
	ushort  cnt;
	ushort	cmd_value;
	ushort	stat;
	int     rc;


	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		return( ENODEV );

	if ((CIO.chan_state[chan-1] != CHAN_OPENED) ||
	    ((open_ptr = CIO.open_ptr[chan-1]) == NULL))
		return( ENXIO );

	if (WRK.adapter_state != STARTED)  	/* adapter has been opened */
		return( ENXIO );
#ifdef	DEBUG
	AIXTRACE (TRC_WQUE, WRK.readyq_in, WRK.readyq_out, 3, total_bytes);
	TRACE5("cxS",WRK.readyq_out, WRK.readyq_in,
	     	     WRK.waitq_out, devno);
#endif	/* DEBUG */

	/* 
	 *  if given an extended parameter, use it.
	 *  otherwise, roll our own.
	 */
	if (extptr == NULL)
	{
		/* build default structure*/
		wr_ext.status = (ulong)CIO_OK;
		wr_ext.flag = 0;        	/* don't notify on complete,free mbuf */
		wr_ext.write_id = 0;    	/* doesn't matter since no notify */
		wr_ext.netid = 0;       	/* default netid for this write */

		/* 
		 *  note: default netid is first netid in table for 
		 *  this open (if any) 
	 	 */
		for (i = 0; i < CIO.num_netids; i++)
		{
			if (CIO.netid_table_ptr[i].chan == chan)
			{
				wr_ext.netid = CIO.netid_table_ptr[i].netid;
				break;
			}
		}
	}
	else
	{
		if (rc=COPYIN(open_ptr->devflag,extptr,&wr_ext,sizeof(wr_ext)))
		{
			return (rc);
		}
		wr_ext.status = (ulong)CIO_OK;

		/* 
		 *  in user mode, mbuf is always freed by the driver
		 *  CIO_NOFREE_MBUF flag should not be set by user mode caller
		 */
		if (!(open_ptr->devflag & DKERNEL) &&
		    (wr_ext.flag & CIO_NOFREE_MBUF))
		{
			return(EINVAL);
		}
	}

	/*
	 *  add the transmit element to the transmit queue
	 */
	if ( XMITQ_FULL )
	{
		TRACE5("cxm1",WRK.readyq_out, WRK.readyq_in,
				WRK.waitq_out, WRK.waitq_in);
		if (uiop->uio_fmode & DNDELAY)
		{
			/* 
			 *  user does not want to block, return EAGAIN 
			 */
			if (open_ptr->devflag & DKERNEL)
			{
				/* call xmt_fn when resources are available */
				CIO.xmt_fn_needed = TRUE;
				open_ptr->xmt_fn_needed = TRUE;
			}
			wr_ext.status = (ulong)CIO_TX_FULL;
			if ( extptr != NULL )
			{
				(void)COPYOUT (open_ptr->devflag, &wr_ext,
				    extptr, sizeof(wr_ext));
			}
			return (EAGAIN);
		}

		/* 
		 *  block until resources have been freed up
		 *  wakeup in cio_xmit_done 
		 */
		if (SLEEP (&open_ptr->xmt_event) != EVENT_SUCC)
		{
			return (EINTR);
		}
	}

	ATT_MEMORY();				/* attach to IO space */

	/*
	 *  if its a user-mode caller, copy data from user
	 *  buffer now, can't move later when disabled
	 */
	if ( !(open_ptr->devflag & DKERNEL))
	{
		rc = get_user_data (dds_ptr, uiop, &mbufp,
				    &total_bytes, &total_areas);
		if (rc)
		{
			if ( mbufp != NULL )
				m_freem (mbufp);
			DET_MEMORY();
			return (rc);
		}
	}

	ipri = i_disable( DDI.cc.intr_priority );
	/*
	 * available room on the queue, check
	 * for unused buffers
	 */
	if ( XMIT_BUFFERS_FULL ) {
		TRACE5("cxm2",WRK.readyq_out, WRK.readyq_in,
				WRK.waitq_out, WRK.waitq_in);
		/*
		 *  tac transmit onto waitq
		 *  update waitq pointers
		 */
		if ( WRK.xmits_queued == 0 )
		{
			WRK.waitq_out = WRK.readyq_in;
			WRK.waitq_in = WRK.waitq_out;
		}

		if (open_ptr->devflag & DKERNEL)
		{
			/* 
			 *  call is from a kernel process,
			 *  get the information about the
			 *  data buffer and build a xmit element
			 *  data is in an mbuf, keep it there 
			 *  till a buffer becomes available. 
			 */
			mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);
			if (mbufp == 0)
			{
				DET_MEMORY();
				i_enable( ipri );
				return (EINVAL);
			}

			bytes = 0;
			if ( mbufp->m_next == NULL )
				bytes = mbufp->m_len;
			else 
			{
				m = mbufp;
				while ( m->m_next != NULL )
				{
					bytes += m->m_len;
					m = m->m_next;
				}
				bytes += m->m_len;
			}

			/* ensure mbuf length is valid */
			if (( bytes < ent_MIN_PACKET ) || 
			    ( bytes > ent_MAX_PACKET))
			{
                        	if ( bytes > ent_MAX_PACKET )
                                	RAS.ds.too_long++; 
				DET_MEMORY();
				i_enable( ipri );
				return(EINVAL);
			}

			/* build the transmit element */
			WRK.xmit_elem[WRK.waitq_in].mbufp = mbufp;
			WRK.xmit_elem[WRK.waitq_in].chan = chan;
			WRK.xmit_elem[WRK.waitq_in].wr_ext = wr_ext;
			WRK.xmit_elem[WRK.waitq_in].bytes = bytes;
			WRITE_LONG(WRK.xcbl_ptr[WRK.waitq_in].xmit.csc, CB_EL|CB_SUS|CB_INT);
		}
		else
		{
			/*  
			 *  caller was a user process. keep data in the 
			 *  mbuf until resources become available.
			 *  the mbuf is necessary because we can't copy 
			 *  from user space on the interrupt level and
			 *  the xmitq will be processed on that level.
			 *  build the transmit element 
			 */
			WRK.xmit_elem[WRK.waitq_in].mbufp = mbufp;
			WRK.xmit_elem[WRK.waitq_in].bytes = total_bytes;
			WRK.xmit_elem[WRK.waitq_in].chan = chan;
			WRK.xmit_elem[WRK.waitq_in].wr_ext = wr_ext;
		}

		WRK.xmits_queued++;
		WRK.waitq_in =bump_que(WRK.waitq_in,DDI.cc.xmt_que_size);
		WRK.readyq_in =bump_que(WRK.readyq_in,DDI.cc.xmt_que_size);
		if (WRK.xmits_queued > RAS.cc.xmt_que_high)
			RAS.cc.xmt_que_high = WRK.xmits_queued;

	}
	else {
#ifdef DEBUG
TRACE2 ("cxba", WRK.buffer_in);
#endif
		/* 
		 *  transmit buffers are available, prepare
		 *  data and tac it on the WRK.readyq_in
		 */
		if (open_ptr->devflag & DKERNEL)
		{
			/* 
			 *  call is from a kernel process,
			 */
			mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);
			if (mbufp == 0)
			{
				DET_MEMORY();
				i_enable( ipri );
				return (EINVAL);
			}

			bytes = 0;
			if ( mbufp->m_next == NULL )
				bytes = mbufp->m_len;
			else 
			{
				m = mbufp;
				while ( m->m_next != NULL )
				{
					bytes += m->m_len;
					m = m->m_next;
				}
				bytes += m->m_len;
			}

			/* ensure mbuf length is valid */
			if (( bytes < ent_MIN_PACKET ) || 
			    ( bytes > ent_MAX_PACKET))
			{
                        	if ( bytes > ent_MAX_PACKET )
                                	RAS.ds.too_long++; 
				DET_MEMORY();
				i_enable( ipri );
				return(EINVAL);
			}

			m_copydata(mbufp,0,bytes,&WRK.xmit_buf_ptr[WRK.buffer_in].buf);
			/* build the transmit element */
			WRK.xmit_elem[WRK.readyq_in].mbufp = mbufp;
			WRK.xmit_elem[WRK.readyq_in].bytes = bytes;
			WRK.xmit_elem[WRK.readyq_in].chan = chan;
			WRK.xmit_elem[WRK.readyq_in].wr_ext = wr_ext;

			/*  free the mbuf if necessary */
			if ( !(wr_ext.flag & CIO_NOFREE_MBUF ))
			{
				m_freem( mbufp );
				WRK.xmit_elem[WRK.readyq_in].mbufp = NULL;
			}

			/*
			 *  put the # of bytes to xfer in xcbl tcb_cnt
 			 */
			tcb_cnt =  (ushort)bytes;

		}
		else
		{
			/* user process */

			/* build the transmit element */
			WRK.xmit_elem[WRK.readyq_in].mbufp = NULL;
			WRK.xmit_elem[WRK.readyq_in].bytes = total_bytes;
			WRK.xmit_elem[WRK.readyq_in].wr_ext = wr_ext;
			WRK.xmit_elem[WRK.readyq_in].chan = chan;

			tcb_cnt =  (ushort)total_bytes;  /* get length */

			m_copydata(mbufp,0,total_bytes,&WRK.xmit_buf_ptr[WRK.buffer_in].buf);
			m_freem( mbufp );

		}

		if ( ! SALMON() )
		  vm_cflush( &WRK.xmit_buf_ptr[WRK.buffer_in].buf, tcb_cnt );

		WRITE_LONG_REV(WRK.xcbl_ptr[WRK.readyq_in].xmit.tbd,
		    &WRK.tbd_addr[WRK.buffer_in]);

		cnt =  ( tcb_cnt & 0x00FF) << 8;
		tcb_cnt = ((tcb_cnt & 0xFF00) >> 8) | cnt;

		WRITE_LONG(WRK.tbd_ptr[WRK.buffer_in].control ,
		    ((ulong)tcb_cnt << 16) | XMIT_EOF);

		WRITE_LONG(WRK.xcbl_ptr[WRK.readyq_in].xmit.csc,
		    CB_EL | CB_SUS | CB_INT | CB_SF | XMIT_CMD);

		WRK.buffer_in = bump_que( WRK.buffer_in,
		    WRK.xmit_buffers_allocd );
		WRK.xmit_buffers_used++;

		/*
		 *  put the bus address of the transmit CBL into the SCB
		 */
		if (!WRK.xmits_pending) {
		       /*
		 	*  ensure the CU is not in the acceptance phase
		 	*/
			COMMAND_QUIESCE();
			WRITE_LONG_REV(WRK.scb_ptr->cbl_addr, &WRK.xcbl_addr[WRK.readyq_in]);
			START_CU();	/*  if CU is not active, start it */
			WRK.xmits_pending = TRUE;
#ifdef DEBUG
TRACE2("cxCU", WRK.readyq_in);
#endif
		}

		WRK.readyq_in = bump_que( WRK.readyq_in, DDI.cc.xmt_que_size );
           	AIXTRACE (TRC_WQUE, devno, chan, mbufp, tcb_cnt);

	}

#ifdef	DEBUG
	TRACE5("cxmE",WRK.readyq_out, WRK.readyq_in,
			WRK.waitq_out, WRK.waitq_in );
#endif	/* DEBUG */
	DET_MEMORY();
	i_enable( ipri );
	return( 0 );
}

/*
 * NAME:     fastwrite
 *
 * FUNCTION: fast write entry point for the kernel
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns indirectly the number of bytes
 *           written through the updating of uiop->uio_resid by uiomove
 *
 */
int 
fastwrite ( devno, mbufp )
dev_t	devno;  			/* major and minor number */
struct	mbuf     *mbufp;		/* pointer to a mbuf */
{
	register dds_t	*dds_ptr;
	cio_write_ext_t wr_ext;
	struct mbuf     *m;
	int     rc = 0;
	ulong	ioaddr;
	ulong	bytes;
	ulong	ipri;
	ulong	i,k;
	ushort	cmd_value;
	ushort	stat;

	if ((dds_ptr = dd_ctrl.dds_ptr) == NULL)
		rc = ENODEV;

	ipri = i_disable( DDI.cc.intr_priority );
#ifdef	DEBUG
	TRACE5("fxS",WRK.readyq_out, WRK.readyq_in,
	     	     WRK.waitq_out, devno);
#endif	/* DEBUG */

	if (mbufp == 0)
		rc = EINVAL;

	if (WRK.adapter_state != STARTED)  	/* adapter has been opened */
		rc = ENODEV;

	if ( rc != 0 )
	{
		i_enable( ipri );
		return( rc );
	}

#ifdef	DEBUG
	AIXTRACE (TRC_WQUE, WRK.readyq_in, WRK.readyq_out, 3, devno);
	trace(0,0,WRK.xmits_pending,WRK.readyq_in,WRK.readyq_out,WRK.waitq_in,WRK.waitq_out );
#endif  /* DEBUG */
	/* build default structure*/
	wr_ext.status = (ulong)CIO_OK;
	wr_ext.flag = 0;       		/* don't notify on complete,free mbuf */
	wr_ext.write_id = 0;   		/* doesn't matter since no notify */
	wr_ext.netid = 0;      		/* default netid for this write */

	/*
	 *  add the transmit element to the transmit queue
	 */
	if ( XMITQ_FULL )
	{
		TRACE5("fxM1",WRK.readyq_out, WRK.readyq_in,
	     	     WRK.waitq_out, WRK.waitq_in);
		/* 
		 *  caller does not want to block, return EAGAIN 
		 *  call xmt_fn when resources are available
		 */
		wr_ext.status = (ulong)CIO_TX_FULL;
		i_enable( ipri );
		return (EAGAIN);
	}

	/* 
	 *  this call is always from a kernel process,
	 *  calculate total bytes to transmit 
	 */
	for ( m = mbufp, bytes = 0; m != NULL; m = m->m_next)
	{
		bytes += m->m_len;
	}

	if (( bytes < ent_MIN_PACKET ) || ( bytes > ent_MAX_PACKET))
	{
            	if ( bytes > ent_MAX_PACKET )
       			RAS.ds.too_long++; 
		i_enable( ipri );
		return(EINVAL);
	}
	ATT_MEMORY();

	/* available room on the queue, check for unused buffers */
	if ( XMIT_BUFFERS_FULL ) {
		TRACE5("fxM2",WRK.readyq_out, WRK.readyq_in,
	     	     WRK.waitq_out, WRK.waitq_in);
		/* 
		 *  build a xmit element using info from mbuf
		 *  data is in an mbuf, keep it there till
		 *  till a buffer becomes available. 
		 */
		if ( WRK.xmits_queued == 0 ) {
			WRK.waitq_out = WRK.readyq_in;
			WRK.waitq_in = WRK.waitq_out;
		}

		/* build the transmit element */
		WRK.xmit_elem[WRK.waitq_in].mbufp = mbufp;
		WRK.xmit_elem[WRK.waitq_in].bytes = bytes;
		WRK.xmit_elem[WRK.waitq_in].chan = 0;
		WRK.xmit_elem[WRK.waitq_in].wr_ext = wr_ext;
		WRITE_LONG(WRK.xcbl_ptr[WRK.waitq_in].xmit.csc,
		    		CB_EL | CB_SUS | CB_INT | CB_SF | XMIT_CMD);

		/*  tac transmit onto waitq increment indices */
		WRK.xmits_queued++;
		WRK.waitq_in = bump_que(WRK.waitq_in,DDI.cc.xmt_que_size);
		WRK.readyq_in = WRK.waitq_in;
		if (WRK.xmits_queued > RAS.cc.xmt_que_high)
			RAS.cc.xmt_que_high = WRK.xmits_queued;
	}
	else {
		/* 
		 *  transmit buffers are available, prepare
		 *  data and tac it on the end of the readyq queue
		 */

		m_copydata(mbufp,0,bytes,&WRK.xmit_buf_ptr[WRK.buffer_in].buf);

		if ( ! SALMON() )
		  vm_cflush( &WRK.xmit_buf_ptr[WRK.buffer_in].buf, bytes );

		/* build the transmit element */
		WRK.xmit_elem[WRK.readyq_in].mbufp = NULL;
		WRK.xmit_elem[WRK.readyq_in].bytes = bytes;
		WRK.xmit_elem[WRK.readyq_in].chan = 0;
		WRK.xmit_elem[WRK.readyq_in].areas = 1;
		WRK.xmit_elem[WRK.readyq_in].wr_ext = wr_ext;

		/*  free the mbuf unconditionally */
		m_freem( mbufp );

		/*
		 *  link the Transmit Control Block to the TBD and
		 *  fill out the TBD
		 */
		WRITE_LONG_REV(WRK.xcbl_ptr[WRK.readyq_in].xmit.tbd,
		    			&WRK.tbd_addr[WRK.buffer_in]);
		bytes = (( bytes & 0x00FF ) << 8 ) | (( bytes & 0xFF00 ) >> 8 );
		WRITE_LONG(WRK.tbd_ptr[WRK.buffer_in].control, bytes << 16 | 
								XMIT_EOF);
		WRITE_LONG(WRK.xcbl_ptr[WRK.readyq_in].xmit.csc,
		    		CB_EL | CB_SUS | CB_INT | CB_SF | XMIT_CMD);
		WRK.buffer_in = bump_que(WRK.buffer_in,WRK.xmit_buffers_allocd);
		WRK.xmit_buffers_used++;

		if (!WRK.xmits_pending) {
			/*
		 	*   if the CU is not in the acceptance phase, start the CU
		 	*/
			COMMAND_QUIESCE();  
			/*
		 	*  put the bus address of the transmit CBL into the SCB
		 	*/
			WRITE_LONG_REV(WRK.scb_ptr->cbl_addr,&WRK.xcbl_addr[WRK.readyq_in]);
			START_CU();
			WRK.xmits_pending = TRUE;
#ifdef DEBUG
TRACE2 ("fxCU", WRK.readyq_in);
#endif
		}

		WRK.readyq_in = bump_que( WRK.readyq_in, DDI.cc.xmt_que_size );

         	AIXTRACE (TRC_WQUE, devno, 0, mbufp,
                          ((bytes & 0x00ff) << 8) | ((bytes & 0xff00) >> 8));

                if ( WRK.wdt_setter == WDT_INACTIVE )
		{
                	WRK.wdt_setter = WDT_XMIT;
                	w_start (&(WDT));
		}
	}
#ifdef	DEBUG
	trace(1,0,WRK.xmits_pending,WRK.readyq_in,WRK.readyq_out,WRK.waitq_in,WRK.waitq_out);
	TRACE5("fxE",WRK.readyq_out,WRK.readyq_in,WRK.waitq_out,WRK.waitq_in);
#endif  /* DEBUG */

	DET_MEMORY();
	i_enable( ipri );
	return( 0 );
}



/*
 * NAME:     local_devswadd
 *
 * FUNCTION: add ourselves to the device switch table (once for device driver)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno as returned by devswadd
 *
 */
int local_devswadd (
dev_t devno)             /* major and minor number */
{
	extern int nodev();

	struct devsw ciodevsw;
	int          rc;

	TRACE2 ("DSAb", (ulong)devno); /* local_devswadd begin */

	/* define entry points */
	ciodevsw.d_open     = (int(*)())cioopen;
	ciodevsw.d_close    = (int(*)())cioclose;
	ciodevsw.d_read     = (int(*)())cioread;
	ciodevsw.d_write    = (int(*)())ciowrite;
	ciodevsw.d_ioctl    = (int(*)())cioioctl;
	ciodevsw.d_strategy = nodev;
	ciodevsw.d_ttys     = NULL;
	ciodevsw.d_select   = (int(*)())cioselect;
	ciodevsw.d_config   = (int(*)())cioconfig;
	ciodevsw.d_print    = nodev;
	ciodevsw.d_dump     = nodev;
	ciodevsw.d_mpx      = (int(*)())ciompx;
	ciodevsw.d_revoke   = nodev;
	ciodevsw.d_dsdptr   = NULL;
	ciodevsw.d_selptr   = NULL;
	ciodevsw.d_opts     = 0;

	rc = devswadd(devno, &ciodevsw);

	TRACE2 ("DSAe", (ulong)rc); /* local_devswadd end */
	return (rc);
} /* end local_devswadd */

/*
 * NAME:     get_user_data
 *
 * FUNCTION: get the user process's data into an mbuf chain
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns mbuf chain pointer,
 *           total number of bytes, and total areas in mbuf chain
 *
 */
int 
get_user_data ( dds_ptr, uiop, mbufph, len, areas )
dds_t	*dds_ptr;		/* pointer to dds structure */
struct  uio  *uiop;             /* pointer to uio structure */
struct  mbuf *mbufph[];         /* address for returning mbuf chain pointer */
ulong   *len; 			/* address for returning total bytes */
ulong   *areas; 		/* address for returning total areas */
{
	struct	mbuf	*mbufp;         /* current one */
	struct  mbuf 	*mbufpprev;     /* previous one */
	int     bytes_to_move; 		/* amount to write */


	*mbufph = NULL;
	*areas = 0;
	*len = uiop->uio_resid;

	/* 
	 *  check length and total areas to gather-write
	 */
	if ((*len < ent_MIN_PACKET  ) || (*len > ent_MAX_PACKET  ) )
	{
                if ( *len > ent_MAX_PACKET )
			RAS.ds.too_long++; 
		return (EINVAL);
	}

	while ((bytes_to_move = uiop->uio_resid) > 0)
	{
		/* get an mbuf */
		mbufp = m_gethdr( M_DONTWAIT, MT_DATA );

		if (mbufp == NULL)
		{
			return (ENOMEM);
		}

		if (*mbufph == NULL)		/* this is first time */
			*mbufph = mbufp;        /* return beginning of chain */
		else
			mbufpprev->m_next = mbufp;    /* ln this to prev one */

		mbufpprev = mbufp;             /* save for nxt time thru loop */


		if ((bytes_to_move > MHLEN) || 0 )
		{
			m_clget(mbufp);
			if ( !M_HASCL(mbufp) )
			{
				return (ENOMEM);
			}
			if (bytes_to_move > CLBYTES)
				/* will need another mbuf in chain */
				bytes_to_move = CLBYTES;
		}

		/* 
	 	 *  move the data into the mbuf - uiop->uio_resid will 
		 *  be decremented) 
		 */
		if (uiomove (MTOD(mbufp,uchar *),bytes_to_move,UIO_WRITE,uiop))
		{
			return (EFAULT);
		}

		/* change the mbuf len to reflect actual data size */
		mbufp->m_len = bytes_to_move;

		/* increment number of pieces to gather for write */
		(*areas)++;
	}

	TRACE5 ("WGUe", (ulong)uiop->uio_resid, (ulong)(*mbufph),
	    (ulong)(*len),
	    (ulong)(*areas)); /* get_user_data end */
	return (0);
} /* end get_user_data */

