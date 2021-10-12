static char sccsid[] = "@(#)52	1.6  src/bos/kernext/sol/solhqm.c, sysxsol, bos411, 9428A410j 7/16/91 11:57:05";

/*
 * COMPONENT_NAME: (SYSXSOLDD) Serial Optical Link Device Driver 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/syspest.h>	/* for ASSERT */
#include <sys/malloc.h>		/* for xmalloc */
#include <sys/m_intr.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/timer.h>

#include "soldd.h"
#include "sol_proto.h"
#include "sol_extrn.h"

extern  vm_cflush();

extern struct mblks {
			int used;
			char *adrs[4];
		} mblks[2];

struct hdr_ctl {
	long *first_hdr;	/* address of first header available */
	short hdrs_left;	/* number of headers that can be requested
				   before any new pages must be pinned */
	short hdrs_avail;	/* number of headers that can be used to
				receive, i. e., number that has been requested
				but is not yet enqueued to imcs */
};

struct hdr_ctl hdr_ctl;

/**/
/*
 * NAME: imcs_declare
 *                                                                    
 * FUNCTION: assign a queue id with the specified description
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 */  

int
imcs_declare(queue_idp, slih_ext, type, num_hdrs)
ushort *queue_idp;
void (*slih_ext)();
int type;
int num_hdrs;
{
	uint qid;
	int rc;
	struct irq_block * irq;

	irq = NULL_IRQ;
	rc = 0;

	/* check parameters */

	switch(type) {
	case DCL_STRICT_Q :
		if (*queue_idp < 0x0040) rc = DCL_TOO_SMALL;
		else if (rqctl_find(*queue_idp) != NULL_IRQ)
			rc = DCL_IN_USE;
		else
			qid = *queue_idp;
		break;
	case DCL_LIBERAL_Q :
		if (*queue_idp < 0x0080) rc = DCL_TOO_SMALL;
		else if (*queue_idp & 0x007f) rc = DCL_NO_RANGE;
		else if (rqctl_find(*queue_idp) != NULL_IRQ)
			rc = DCL_IN_USE;
		else
			qid = *queue_idp;
		break;
	default: /* unknown type */
		rc = DCL_TYPE;
	}

	if (rc) goto error_out;

	irq = rqctl_get(qid);   /* this also initializes it */
	if (NULL_IRQ == irq) {
		rc = DCL_OUT_BLOCK;
		goto error_out;
	}

	irq -> queue_id = qid;
	irq -> imcs_slih_ext = slih_ext;

	switch(type) {
	case DCL_STRICT_Q :
		/* allocate the headers */
		if (rc = req_imcs_hdr(num_hdrs)) goto error_out;
		irq -> max_num_hdr = (short) num_hdrs;
		irq -> type = IMCS_STRICT_Q;
		break;
	case DCL_LIBERAL_Q :
		irq -> type = IMCS_LIBERAL_Q;
		break;
	}

	rqctl_hash(irq);

	return DCL_DONE;

error_out:
	if (irq != NULL_IRQ) rqctl_put(irq);
	return rc;

}  /* end imcs_declare */


/**/
/*
 * NAME: req_imcs_hdr
 *                                                                    
 * FUNCTION: request a specified number of headers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS:	 0 if number of headers requested available
 *		-1 otherwise
 */  


int req_imcs_hdr(short num_hdr)
{
	int processor_priority;
	short hdr_needed, page_needed;
	char *p, *q, *start;

	if (hdr_ctl.hdrs_left < num_hdr) {
		hdr_needed = num_hdr - hdr_ctl.hdrs_left;

		/* allocate more headers */
		page_needed = hdr_needed >> L2HDRS_IN_PAGE;
		if (hdr_needed & (HDRS_IN_PAGE - 1)) page_needed++;
		page_needed = 
			(page_needed<MIN_HDR_PAGES)?MIN_HDR_PAGES : page_needed;

		start=(char *)xmalloc(page_needed*PAGESIZE,PGSHIFT,pinned_heap);
		bzero((caddr_t) start, page_needed*PAGESIZE);
		ASSERT(mblks[0].used < MAX_NUM_SLA);
		mblks[0].adrs[mblks[0].used++] = start;

		if (NULL == start) return -1;

		/* chain headers */
		for (q = start, p = start + IMCS_RTSI_HDR_SIZE;
		    p < start + PAGESIZE * page_needed;
		    q = p, p = q + IMCS_RTSI_HDR_SIZE)
			*(long *)q = (long *)p;

		hdr_ctl.hdrs_left += (page_needed << L2HDRS_IN_PAGE);

		/* put headers in list */
		processor_priority = i_disable(IMCS_INT_PRIORITY);
		*(long *)q = hdr_ctl.first_hdr;
		hdr_ctl.first_hdr = (long *) start;
	}
	else
		processor_priority = i_disable(IMCS_INT_PRIORITY);

	hdr_ctl.hdrs_avail += num_hdr;
	i_enable(processor_priority);

	hdr_ctl.hdrs_left -= num_hdr;

	return 0;

}


/**/
/*
 * NAME: get_imcs_hdr 
 *                                                                    
 * FUNCTION:  takes one header from the pool and returns its virtual address
 *  to the caller.  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) This routine can only be called when interrupts are masked
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: address of header
 */  


caddr_t
get_imcs_hdr()
{
	caddr_t vaddr;

	ASSERT(hdr_ctl.hdrs_avail > 0);
	vaddr = (caddr_t) hdr_ctl.first_hdr;

	hdr_ctl.first_hdr = (long *)  *hdr_ctl.first_hdr;
	hdr_ctl.hdrs_avail--;

	return vaddr;

}  /* end get_imcs_hdr */

/**/
/*
 * NAME: put_imcs_hdr 
 *                                                                    
 * FUNCTION: returns a header to the pool
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
put_imcs_hdr(vaddr)
caddr_t vaddr;
{
	long *p;
	int processor_priority;

	p = (long *) vaddr;

	processor_priority = i_disable(IMCS_INT_PRIORITY);

	*p = hdr_ctl.first_hdr;
	hdr_ctl.first_hdr = p;

	hdr_ctl.hdrs_avail++;

	i_enable(processor_priority);

}


/**/
/*
 * NAME: count_imcs_hdr 
 *                                                                    
 * FUNCTION: returns the number of headers reclaimed but not in use
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *  	it only makes sense to call this routine at int. level
 *                                                                 
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: number of available headers
 */  


short
count_imcs_hdr()
{
	return hdr_ctl.hdrs_avail;

}

/**/
/*
 * NAME: init_imcs_hdr 
 *                                                                    
 * FUNCTION:  initializes the header area
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
init_imcs_hdr()
{
	hdr_ctl.first_hdr = NULL;
	hdr_ctl.hdrs_avail = 0;
	hdr_ctl.hdrs_left = 0;

}

/**/
/*
 * NAME: rqctl_get 
 *                                                                    
 * FUNCTION: allocates a ctl block and initializes it to "innocent" values
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *      If the free list is empty then a page is allocated in the kernel
 *	segment.  Since once an imcs queue is declared (i. e., an account 
 *	is open), it cannot be "undeclared", the control blocks are 
 *	allocated in the portion of the kernel heap that is always pinned.  
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 	pointer to control block 
 *		NULL_PTR if xmalloc fails
 */  

struct irq_block *
rqctl_get()
{

	struct irq_block * irq;

	if (NULL_IRQ == irq_tbl.free_list) {
		/* allocate more space */
		irq_tbl.free_list = (struct irq_block *) 
				xmalloc(PAGESIZE, PGSHIFT, pinned_heap);
		bzero((caddr_t) irq_tbl.free_list,PAGESIZE);
		ASSERT(mblks[1].used < MAX_NUM_SLA);
		mblks[1].adrs[mblks[1].used++] = irq_tbl.free_list;
		if (NULL_IRQ == irq_tbl.free_list) return NULL_IRQ;

		for (irq = irq_tbl.free_list;
		    irq<irq_tbl.free_list+(PAGESIZE/sizeof(struct irq_block)-1);
		    irq++)
			irq -> next = (struct irq_block *) (irq + 1);

		irq -> next = NULL_IRQ;
	}

	irq = irq_tbl.free_list;
	irq_tbl.free_list = irq -> next;

	irq -> waiting = NULL_IRQ;
	irq -> imcs_slih_ext = NULL;
	irq -> rcv_count = 0;
	irq -> queue_id = 0;
	irq -> max_num_hdr = 0;
	irq -> hdr_in_use = 0;
	irq -> num_buf = 0;
	irq -> status = (uchar) 0;
	irq -> type = (uchar) 0;
	bzero(irq->pids, IMCS_PROC_LIMIT);

	return irq;

}  /* end rqctl_get */


/**/
/*
 * NAME: rqctl_hash 
 *                                                                    
 * FUNCTION: inserts a control block in the appropriate hash chain
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  

void
rqctl_hash(irq)
struct irq_block *irq;
{
	long qa;

	qa = HASH_IRQ(irq -> queue_id);
	irq -> next = irq_tbl.anchor[qa];
	irq_tbl.anchor[qa] = irq;
	/* now the new control block is visible by imcs, but is inactive */

}  /* end rqctl_hash */


/**/
/*
 * NAME: rqctl_put
 *                                                                    
 * FUNCTION: returns a ctl block to the free list.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *       This is only called when imcs declare finds an error after it has
 *       allocated a control block.  Since all the imcs declares serialize
 *       there can be at most one header returned to the pool at any time.
 *       Therefore it is not worthwhile to write code that unpins the control
 *       blocks.
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: NONE
 */  


void
rqctl_put(irq)
struct irq_block *irq;
{
	irq -> next = irq_tbl.free_list;
	irq_tbl.free_list = irq;

}  /* end rqctl_put */



/**/
/*
 * NAME: rqctl_find 
 *                                                                    
 * FUNCTION: finds the address of the control block for the specified queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: pointer to control block
 */  


struct irq_block *
rqctl_find(queue_id)
uint queue_id;
{
	struct irq_block * irq;
	long qa;

	qa = HASH_IRQ(queue_id);

	for (irq = irq_tbl.anchor[qa];
	    irq != NULL_IRQ && irq -> queue_id != queue_id;
	    irq = irq -> next);

	return irq;

} /* end rqctl_find */

/**/
/*
 * NAME: rqctl_unhash
 *                                                                    
 * FUNCTION: unhash the control block for the specified queue
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 */  

struct irq_block *
rqctl_unhash(queue_id)
uint queue_id;
{
	struct irq_block * irq, *pirq;
	long qa;

	qa = HASH_IRQ(queue_id);

	pirq = NULL_IRQ;
	for (irq = irq_tbl.anchor[qa];
	    irq != NULL_IRQ && irq -> queue_id != queue_id;
	    pirq = irq, irq = irq -> next);

	if (irq != NULL_IRQ) {
		if (pirq == NULL_IRQ)
			irq_tbl.anchor[qa] = irq -> next;
		else
			pirq -> next = irq -> next;
	}

	return irq;

} /* end rqctl_unhash */


/**/
/*
 * NAME: tagwords 
 *                                                                    
 * FUNCTION: fills in the imcs tagword area
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) 
 *
 * (RECOVERY OPERATION:) 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 
 *	    0 - OK
 *	   -1 - bad length
 *	   -2 - bad address
 *	   -3 - msg too long
 *	   -4 - address not in real memory
 */  


int
tagwords(waddr,length,hdr_vaddr,starttag)
caddr_t waddr;
long length;
caddr_t hdr_vaddr;
int *starttag;
{

	long len, len1;
	ulong vad;
	int i;
	struct imcs_header *header;

	header = (struct imcs_header *) hdr_vaddr;

	if (*starttag == 0)
		header -> IMCS_MSGLEN = 0;

	vad = (ulong) waddr;

	/*  check on length - .le. 64k and multiple of 64 */
	if (length > 64  * 1024) return -3;  /* length > 64K */
	if ((length & IMCS_LINE_SIZE - 1) != 0) return -1; 
						/* length not multiple */

	/* make sure on line boundary */
	if ((vad & IMCS_LINE_SIZE - 1) != 0) return -2;


	/* do the conversions  */
	len = length;                       /* length remaining  */
	len1 = PAGESIZE - (vad & PAGESIZE - 1);
	len1 = len1 < length ? len1 : length;
	/* len1 is now the length of next chunk */

	for (i = (*starttag); (len > 0) && (i < NUM_HDR_TCWS); i++) {
		header -> IMCS_TAG(i) = imcslra(vad);
		if (header -> IMCS_TAG(i) == 0xffffffff)
			return -4;
		header -> IMCS_COUNT(i) = len1 / IMCS_LINE_SIZE;
		header -> IMCS_MSGLEN += len1;
		len -= len1;
		vad += len1;
		len1 = PAGESIZE > len ? len : PAGESIZE;
	}
	(*starttag) = i;
	if (len > 0) return -3;              /* message too long  */

	return 0;

}

cdd_qdelete(going_away_q)
int going_away_q;
{
	int processor_priority;
        int q_go;
        struct imcs_header * anchor;

        q_go = SLA_OUTQ + going_away_q;
	
	processor_priority = i_disable(IMCS_INT_PRIORITY);
        anchor = cddq.anchor[q_go];
        cddq.anchor[q_go] = NULL_HDR;
	i_enable(processor_priority);

        cdd_requeue(anchor);

}  /* cdd_qdelete */

cdd_requeue(q_anchor)
struct imcs_header * q_anchor;
{
        struct imcs_header *this;
        struct imcs_header *next_in_cdd_chain;

        for (this = q_anchor;
             this != NULL_HDR;
             this = next_in_cdd_chain) {
          next_in_cdd_chain = this -> cdd_chain_word;
          ASSERT(this -> outcome == IMCS_CDD_ENQ);
          cdd_enqueue(this);
        }

}  /* end cdd_requeue */


