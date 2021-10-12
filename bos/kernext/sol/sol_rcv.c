static char sccsid[] = "@(#)47	1.8  src/bos/kernext/sol/sol_rcv.c, sysxsol, bos411, 9428A410j 6/6/94 15:40:19";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_get_rcv, sol_rcv_notify
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
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
#include <sys/soluser.h>
#include "soldd.h"

extern struct sol_ddi	sol_ddi;

/*
 *  			GENERAL DESCRIPTION OF RECEIVE
 *
 *  The SLA will interrupt when another processor wishes to send to us.  At
 *  that time, IMCS will look and see if there are any buffers ready for
 *  the requested subchannel.  There won't be because we haven't given him
 *  any.  So the "slih extension" sol_get_header will be called to get
 *  the buffers.  Here's a description of all the types of buffers.
 *
 *	super-header - this is a 256 byte mbuf that contains an imcs_header,
 *		misc. info for transmit or receive, and the off-level
 *		interrupt structure.  The m_hdr field is left alone, so
 *		that it won't have to be "fixed-up" later.
 *		NOTE:  an super_header is never transmitted, and data is
 *		never received into one.
 *
 *	imcs_header - contains all the information needed by imcs.
 *		The most important thing is that it includes the tags
 *		(real addresses) of all the buffers that are available
 *		and have been mapped for this dma.  
 *
 *	m_save - this is part of the rcv_info of the super_header.
 *		When we get the second interrupt after the receive
 *		takes place, the offlevel (sol_rcv_notify) will
 *		be called with the header as a parameter.  From this
 *		we can get the save array (m_save).  This buffer
 *		contains pointers to all the mbufs that have been mapped
 *		for the receive.  In the case of clusters, the save_array
 *		points to the small mbuf, and the small mbuf points to
 *		the cluster.  The reason for all this is that when we
 *		receive data into mbufs, we receive the ENTIRE mbuf from
 *		the other side, including the m_next pointers, which are
 *		meaningless to us.  So the save_array is needed to link
 *		all the mbufs back together again.
 *
 *	cluster_descripter - if clusters are transmitted, there will always
 *		be a cluster descripter on the front end.  This is
 *		a small mbuf, but it has been modified to contain the 
 *		offset of the data, and the length of the data for each
 *		cluster.  Since the small mbufs are not transmitted with
 *		the clusters, this information is needed.  NOTE:  the
 *		cluster descripter is always transmitted, and there must
 *		be a small mbuf ready at the receive end for it.
 *
 *  The data received will be in one of two "shapes", depending on the
 *  subchannel the data comes in on.  An even subchannel indicates a
 *  cluster chain.  This means there will be a small mbuf with the
 *  cluster descripter, followed by up to 15 4K clusters.  An odd
 *  subchannel indicates a small mbuf chain, which will be up to 4
 *  small mbufs (all with data).  Since we get an interrupt before we
 *  receive the data, we will always know what the subchannel is and
 *  we can set up the receive buffers for the correct "shape" of data.
 *  So, when sol_get_header is called, it will return an imcs_header
 *  with the correct types of buffers mapped and ready to go.
 */

/*
 * NAME: sol_get_rcv
 *
 * FUNCTION: Gets first receive element for read routine (sleeps if necessary)
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the process environment, but it can
 *	not page fault because interrupts must be disabled.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0		- successful completion                        
 *	EINTR		- the sleep was interrupted by a signal
 */

int
sol_get_rcv(
struct sol_open_struct	*open_ptr,	/* ptr to open structure	*/
struct sol_recv_que	*recv_que,	/* ptr to head of receive queue	*/
struct mbuf		**mptr)	/* destination for queu element	*/
{
	int	oldpri, rc;

	/*
	 *  Sleep until there is somthing on the receive queue.  Interrupts
	 *  must be disabled around the check.
	 */
	oldpri = i_disable(SOL_SLIH_LEVEL);
	while (recv_que->head == NULL) {
		rc = e_sleep(&open_ptr->recv_event, EVENT_SIGRET);
		if (rc == EVENT_SIG) {
			i_enable(oldpri);
			return EINTR;
		}
	}
	/*
	 *  At this point there is a receive element (mbuf chain) on the
	 *  head of the list.  Copy this mbuf pointer for the caller and
	 *  bump the head pointer to the next element in the list (via
	 *  the nextpkt field in the mbuf.
	 */
	*mptr = recv_que->head;
	recv_que->head = recv_que->head->m_nextpkt;
	if (recv_que->head == NULL) {
		recv_que->tail = NULL;
	}
	open_ptr->recv_count--;
	i_enable(oldpri);
	return 0;
}

/*
 * NAME: sol_rcv_notify
 *
 * FUNCTION: Schedules receive off-level routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called by the interrupt routine, and can not
 *	page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	None.
 */
void
sol_rcv_notify(
struct imcs_header	*hdr)	/* IMCS header that just completed	*/
{
	struct super_header	*super_header;
	struct mbuf		**freelist, *m;
	short			first_unused, nummbufs, havecluster, n;

	/*
	 *  Before scheduling the off-level, go ahead and free the unused
	 *  mbufs.  Since most of the packets are small, this could prevent
	 *  us from running out of mbufs prematurely.
	 *
	 *  Set up first_unused = first unused mbuf in the received chain, and
	 *  first_index = the first mbuf with data.
	 */
	super_header = (struct super_header *)((uint)hdr-sizeof(struct m_hdr));
	SYS_SOL_TRACE(RCV_NOTIFY_ENTRY, super_header, 0, 0);
	if (!(hdr -> IMCS_SUBCHANNEL & 0x1)) {	/* cluster chain */
		nummbufs = SOL_CMBUF_LEN + 1;
		/*
		 * Compute position of first unused mbuf.
		 */
		first_unused = 1 + (hdr->IMCS_MSGLEN+CLBYTES-1-MSIZE)/CLBYTES;
		if (hdr -> outcome ) {
			first_unused = 1;
		}
		havecluster = 1;
		freelist = &sol_ddi.freeclus;
	} else {				/* small mbuf chain */
		nummbufs = SOL_MMBUF_LEN;
		/*
		 * Compute position of first unused mbuf.
		 */
		first_unused = (hdr -> IMCS_MSGLEN)/MSIZE;
		if (hdr -> outcome ) {
			first_unused = 0;        
		}
		havecluster = 0;
		freelist = &sol_ddi.freembuf;
	}

	/*
	 * Save unused mbufs in private freelist which buildchain will
	 * try first.  This is because these mbufs have already been
	 * cache-flushed and tagwords built.  No need to disable
	 * since we're still on the interrupt thread.
	 */
	for (n = first_unused; n < nummbufs; n++) {
		m = super_header->rcv_tx.rcv_info.msave[n];
		((struct super_header *)m)->rcv_tx.rcv_info.tag_save =
		    hdr->IMCS_TAG(n);
		m -> m_next = *freelist;
		*freelist = m;
	}

	/*
	 *  Schedule sol_rcv_offlevel to do the rest of the work.
	 */
	super_header->ourintr.super_header = super_header;
	INIT_OFFL1(&super_header->ourintr.intr,sol_rcv_offlevel,0);
	i_sched(&super_header->ourintr.intr);
	SYS_SOL_TRACE(RCV_NOTIFY_EXIT, super_header, 0, 0);
	return;
}

/*
 * NAME: sol_rcv_offlevel
 *
 * FUNCTION: Receive off-level routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is scheduled by the interrupt routine, and can not
 *	page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	None.
 */
int
sol_rcv_offlevel(
struct intr	*iptr)		/* intr structure filled in by rcv_notify */
{
	
	struct 	mbuf		*firstm, *m, *m0, **freelist;
	struct sol_open_struct	*open_ptr;
	struct imcs_header	*h, *hdr;
	struct super_header	*super_header;
	cio_read_ext_t		rd_ext;
	cio_stat_blk_t		stat_blk;
	short			first_index, first_unused, n;
	struct cl_desc		*cl_desc;
	int			type, oldpri, total_bytes;
	short			nummbufs, havecluster;
	netid_t			netid;

	super_header = ((struct ourintr *)iptr)->super_header;
	h = &super_header->imcs_header;
	SYS_SOL_TRACE(RCV_OFFLEVEL_ENTRY, super_header, h->outcome, 0);

	/*
	 *  Set up first_unused = first unused mbuf in the received chain, and
	 *  first_index = the first mbuf with data.
	 */
	if (!(h -> IMCS_SUBCHANNEL & 0x1)) {	/* cluster chain */
		nummbufs = SOL_CMBUF_LEN + 1;
		first_index = 1;			/* first real mbuf */
		m = super_header->rcv_tx.rcv_info.msave[0];
		cl_desc = (struct cl_desc *) m;
		/*
		 *  Work-around for defect 42545.  For some reason this
	 	 *  buffer does not seem to have been invalidated when
		 *  we reach this point.  So go ahead and invalidate it
		 *  at this point.
		 */
		vm_cinval(m,MSIZE);
		/*
		 * Compute position of first unused mbuf.
		 */
		first_unused = 1 + (h->IMCS_MSGLEN+CLBYTES-1-MSIZE)/CLBYTES;
		if (h -> outcome ) {
			/* update statistics */
			sol_ddi.stats.cc.rx_err_cnt++;
			first_unused = 1;
		}
		havecluster = 1;
		freelist = &sol_ddi.freeclus;
	} else {				/* small mbuf chain */
		nummbufs = SOL_MMBUF_LEN;
		first_index = 0;

		/*
		 * here we need to flush the cache to
		 * avoid picking up bad data
		 */

		m = super_header->rcv_tx.rcv_info.msave[0];
		vm_cinval(m,MSIZE);

		/*
		 * Compute position of first unused mbuf.
		 */
		first_unused = (h -> IMCS_MSGLEN)/MSIZE;
		if (h -> outcome ) {
			/* update statistics */
			sol_ddi.stats.cc.rx_err_cnt++;
			first_unused = 0;        
		}
		havecluster = 0;
		freelist = &sol_ddi.freembuf;
	}

	if (!(h->outcome)) {
		/*
		 *  Use the save array (msave) to link all the mbufs back
		 *  together again.  All the m_next pointers were over-written
		 *  when we received the data.  For clusters, use the cluster
		 *  descripter (the first mbuf received) to set the length and
		 *  offset fields.
		 */
		firstm = m = super_header->rcv_tx.rcv_info.msave[first_index];
		if (havecluster) {
			m->m_data = (caddr_t) ((uint) m->m_data +
			    cl_desc->offsets[first_index-1]);
			m->m_len = cl_desc->lengths[first_index-1];
		} else {
			/*  Convert the offset back to a direct pointer */
			m->m_data = (caddr_t)((uint)m + (uint)m->m_data);
			m->m_type = MT_DATA;
			m->m_flags = 0;
			m->m_nextpkt = NULL;
		}
		total_bytes = m->m_len;
		for (n = first_index+1; n < first_unused; n++) {
			m -> m_next = super_header->rcv_tx.rcv_info.msave[n];
			m = m -> m_next;
			if (havecluster) {
				m->m_data = (caddr_t) ((uint) m->m_data +
				    cl_desc->offsets[n-1]);
				m -> m_len  = cl_desc -> lengths[n-1];
			} else {
				/* Convert the offset back to a direct ptr */
				m->m_data = (caddr_t)((uint)m+(uint)m->m_data);
				m->m_type = MT_DATA;
				m->m_flags = 0;
				m->m_nextpkt = NULL;
			}
			total_bytes += m->m_len;
		}
		m -> m_next = NULL;

		/*
		 *  Get the netid from the data, and look-up in the netid table.
		 */
		netid = (ushort) *((caddr_t)((int)MTOD(firstm, uchar *) +
		    SOL_NETID_OFFSET));
		/*
		 * Check for group netid (odd)
		 */
		if (netid % 2) {
			open_ptr = NULL;
		} else {
			open_ptr = sol_ddi.netid_table[netid >> 1];
		}

		/*
		 *  If there was an error or the data was for an un-registered
		 *  netid, free the mbufs.
		 */

		if (open_ptr == NULL) {
			m_freem(firstm);
		} else {	/* no error */

			/*
			 * update the standard counters
			 */
			if (ULONG_MAX == sol_ddi.stats.cc.rx_frame_lcnt) {
				/* record overflow in msh of counter */
				sol_ddi.stats.cc.rx_frame_mcnt++;
			}
			sol_ddi.stats.cc.rx_frame_lcnt++;
			if ((ULONG_MAX - total_bytes) <
			    sol_ddi.stats.cc.rx_byte_lcnt) {
				/* record overflow in msh of counter */
				sol_ddi.stats.cc.rx_byte_mcnt++;
			}
			sol_ddi.stats.cc.rx_byte_lcnt += total_bytes;
			
			if (open_ptr->devflag & DKERNEL) { /* kernel mode */
				/*
				 *  For kernel-mode, call the receive function
				 *  directly.
				 */
				rd_ext.status = CIO_OK;
				rd_ext.netid = netid;
				(*(open_ptr->recv_fn)) (open_ptr->open_id,
				    &rd_ext, firstm);
			} else { /* user-mode */
				/*
				 *  Check if we've reached the size limit
				 *  for the receive queue.
				 */
				if (open_ptr->recv_count ==
				    sol_ddi.ops_info.rec_que_size) {
					/*
					 *  Build a lost data status block,
					 *  and drop the packet.
					 */
					stat_blk.code = CIO_ASYNC_STATUS;
					stat_blk.option[0] = CIO_LOST_DATA;
					sol_report_status(open_ptr, &stat_blk);
					m_freem(firstm);
					/*
					 *  Update the statistics counter
					 */
					sol_ddi.stats.ds.rec_que_overflow++;
				} else {
					/*
					 *  For user-mode, link this mbuf
					 *  chain on the receive queue,
					 *  using the m_act field.
					 */
					firstm->m_nextpkt = NULL;
					open_ptr->recv_count++;
					/*
					 *  Update stats if necessary
					 */
					if (open_ptr->recv_count >
					    sol_ddi.stats.cc.rec_que_high) {
						sol_ddi.stats.cc.rec_que_high++;
					}
					if (open_ptr->recv_que.head == NULL) {
						open_ptr->recv_que.head=firstm;
						open_ptr->recv_que.tail=firstm;
					} else {
						open_ptr->recv_que.tail->
						    m_nextpkt = firstm;
						open_ptr->recv_que.tail=firstm;
					}
					e_wakeup(&open_ptr->recv_event);
					if (open_ptr->select_req & POLLIN) {
						open_ptr->select_req &= ~POLLIN;
						selnotify(sol_ddi.devno,
						    open_ptr->chan, POLLIN);
					}
				}
			}
		}
	}

	/*
	 *  Now free the cluster descripter.
	 */
	if (havecluster) {
		m = super_header->rcv_tx.rcv_info.msave[0];
		m->m_type = MT_DATA;
		m->m_flags = 0;
		m->m_nextpkt = NULL;
		m->m_next = NULL;
		m->m_len = 0;
		m_free(m);
	}

	/*
	 *  Free the header.
	 */

	m_free((struct mbuf *) super_header);

	/*
	 *  Replenish the headers and receive buffers for the next receive.
	 */
	while (sol_ddi.num_mbuf < SOL_MAX_MMBUFS) {
		super_header = sol_buildchain((short)FALSE);
		if (super_header == NULL) {
			break;
		} else {
			oldpri = i_disable(SOL_SLIH_LEVEL);
			sol_ddi.num_mbuf++;
			super_header->imcs_header.imcs_chain_word =
			    (struct imcs_header *)sol_ddi.hdrmlist;
			sol_ddi.hdrmlist = super_header;
			i_enable(oldpri);
		}
	}
	while (sol_ddi.num_clus < SOL_MAX_CMBUFS) {
		super_header = sol_buildchain((short)TRUE);
		if (super_header == NULL) {
			break;
		} else {
			oldpri = i_disable(SOL_SLIH_LEVEL);
			sol_ddi.num_clus++;
			super_header->imcs_header.imcs_chain_word =
			    (struct imcs_header *)sol_ddi.hdrclist;
			sol_ddi.hdrclist = super_header;
			i_enable(oldpri);
		}
	}
	SYS_SOL_TRACE(RCV_OFFLEVEL_EXIT, super_header, 0, 0);
	return 0;
}
/*
 * NAME: sol_buildchain
 *
 * FUNCTION: Builds a chain of mbufs for receive data.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the interrupt environment, and it can
 *	not page fault.
 *
 * NOTES:
 *	This routine is called either from the interrupt handler, if there
 *	are no buffers available when a receive interrupt occurs, or from
 *	the receive off-level handler to replenish the buffers after a
 *	receive operation.  The routine attempts to get mbufs from the
 *	chain of pre-mapped and invalidated buffers.  If there are none
 *	available, m_get is called and the buffer is mapped and invalidated.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	struct imcs_header	- pointer to imcs_header for the chain.
 *	NULL			- couldn't get mbuf or cluster
 */

struct super_header *
sol_buildchain(
short	wantclus)		/* small mbufs or clusters needed */
{

	struct mbuf		*h, *m;
	struct imcs_header	*hdr;
	struct super_header	*super_header;
	short			n, nummbufs;
	int			oldpri, tag;

	SYS_SOL_TRACE(BUILDCHAIN_ENTRY, wantclus, 0, 0);

	nummbufs = SOL_MMBUF_LEN;
	tag = 0;
	/*
	 *  Get mbuf for the super_header
	 */
	h = m_get(M_DONTWAIT,MT_DATA);		/* for the header */
	if (h==NULL) {
		SYS_SOL_TRACE(BUILDCHAIN_EXIT, NULL, 0, 0);
		return NULL;
	}
	
	super_header = (struct super_header *)h;
	hdr = &super_header->imcs_header;
	hdr->IMCS_PROTOCOL = IMCS_RTS_RCV_CODE;
	hdr->notify_address = (void (*) ()) sol_rcv_notify;
	
	/*
	 *  For a cluster chain, we only need one small mbuf for the 
	 *  cluster descripter, otherwise we will need SOL_MMBUFS_LEN.
	 */
	if (wantclus) {
		nummbufs = 1;
	}

	for (n = 0; n < nummbufs; n++) {
		oldpri = i_disable(SOL_SLIH_LEVEL);
		/*
		 *  If we can, we want to get the mbuf from our freelist,
		 *  because we already have the tag for it.  Otherwise,
		 *  call m_get and then tagwords to map the buffer.
		 *  The save array is built during this loop.
		 */
		if (sol_ddi.freembuf != NULL) {
			m = sol_ddi.freembuf;
			sol_ddi.freembuf = m -> m_next;
			i_enable(oldpri);
			hdr -> IMCS_TAG(n) = ((struct super_header *) m)->
			    rcv_tx.rcv_info.tag_save;
			
			/*
			 *  Unfortunately, we have to invalidate this small
			 *  mbuf, because we just read it to get the tag.
			 */
			vm_cinval(m,MSIZE);	/* clear out cache before I/O */
			tag++;  
		} else {
			i_enable(oldpri);
			m = m_get(M_DONTWAIT, MT_DATA);
			if (m==NULL) {
				oldpri = i_disable(SOL_SLIH_LEVEL);
				for (n=n-1; n>= 0; n--) {
					/* return the chain to free list */
					m = super_header->rcv_tx.rcv_info.
					    msave[n];
					((struct super_header *)m)->rcv_tx.
					    rcv_info.tag_save =
					    hdr->IMCS_TAG(n);
					m -> m_next = sol_ddi.freembuf;
					sol_ddi.freembuf = m;
				}
				i_enable(oldpri);

				/* free the imcs header and save array */
				m_free(h);

				SYS_SOL_TRACE(BUILDCHAIN_EXIT, NULL, 0, 0);
				return NULL;
			}
			tagwords(m, MSIZE, hdr, &tag);
			/*
			 *  Note:  by doing a cache invalidate here on this
			 *  mbuf, the values in the mbuf (i.e. m_type) will
			 *  probabaly be wrong if we don't dma into this
			 *  buffer.  This is OK, though because we will
			 *  set up all the important fields before we return.
			 */
			vm_cinval(m,MSIZE);
		}
		super_header->rcv_tx.rcv_info.msave[n] = m;
	}

	/*
	 * Get clusters if requested.
	 */
		
	if (wantclus) {
		for (n = 1; n < SOL_CMBUF_LEN + 1; n++) {
			oldpri = i_disable(SOL_SLIH_LEVEL);
			/*
			 *  Get the clusters from the freelist if possible,
			 *  otherwise call m_getclust and tagwords to map
			 *  the cluster.
			 */
			if ((m = sol_ddi.freeclus) != NULL) {
				ASSERT(m -> m_type != MT_FREE);
				sol_ddi.freeclus = m -> m_next;
				m -> m_next = NULL;
				i_enable(oldpri);
				hdr->IMCS_TAG(n) = ((struct super_header *)m)->
				    rcv_tx.rcv_info.tag_save;
				/*
				 * We have to set this field back to zero
				 * sometime before it is freed.
				 */
				tag++;  
			} else {
				i_enable(oldpri);
				if (!(m = m_getclust(M_DONTWAIT,MT_DATA))) {
					oldpri = i_disable(SOL_SLIH_LEVEL);
					for (n=n-1; n>0; n--) {
						/*
						 *  Return the clusters to the
						 *  free list.
						 */
						m = super_header->rcv_tx.
						    rcv_info.msave[n];
						((struct super_header *)m)->
						    rcv_tx.rcv_info.tag_save =
						    hdr->IMCS_TAG(n);
						m -> m_next = sol_ddi.freeclus;
						sol_ddi.freeclus = m;
					}
					/*
					 *  Return the small mbuf to the
					 *  free list.
					 */
					m = super_header->rcv_tx.rcv_info.
					    msave[0];
					((struct super_header *)m)->rcv_tx.
					    rcv_info.tag_save =
					    hdr->IMCS_TAG(0);
					m -> m_next = sol_ddi.freembuf;
					sol_ddi.freembuf = m;
					i_enable(oldpri);
					/*
					 *  Free the imcs header.
					 */
					m_free(h);

					SYS_SOL_TRACE(BUILDCHAIN_EXIT, NULL,
					    0, 0);
					return NULL;
				}
				tagwords(SOL_MTOCL(m),CLBYTES,hdr,&tag);
				vm_cinval(SOL_MTOCL(m),CLBYTES);	
			}
			super_header->rcv_tx.rcv_info.msave[n] = m;
		}
	}
	for ( ; tag<NUM_HDR_TCWS ; tag++) {
		hdr -> IMCS_TAG(tag) = LAST_TCW;
	}
	SYS_SOL_TRACE(BUILDCHAIN_EXIT, super_header, 0, 0);
	return super_header;
}
/*
 * NAME: sol_get_header
 *
 * FUNCTION: Sets up the chain of mbufs for receive data when the request to
 *	     send comes in.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the interrupt environment, and it can
 *	not page fault.
 *
 * NOTES:
 *	This routine is called when a RTS interrupt occurs. It will either
 *	return an imcs header that has already been set up, or call
 *	sol_buildchain to build one.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	struct imcs_header	- pointer to imcs_header for the chain.
 *	NULL			- couldn't get mbuf or cluster
 */
caddr_t 
sol_get_header(
uint	subch)			/* subchannel that needs buffers	*/
{
	struct super_header	*super_header;

	SYS_SOL_TRACE(GET_HEADER_ENTRY, subch, 0, 0);
	/*
	 *  Hopefully, most of the time we will have a header and receive
	 *  buffers all set up, and all we have to do is get it off the
	 *  list and return.  If not, call buildchain to try and get
	 *  the buffers.  If this fails (it can't wait for mbufs), then
	 *  we're out of luck.
	 */
	if (subch & 1) {	/* if odd, these are mbufs not clusters */
		if ((super_header = sol_ddi.hdrmlist) == NULL) {
			super_header = sol_buildchain((short)FALSE);
			if (super_header == NULL) {
				SYS_SOL_TRACE(GET_HEADER_EXIT, NULL, 0, 0);
				return NULL;
			}
		} else {
			sol_ddi.hdrmlist = (struct super_header *)
			    super_header->imcs_header.imcs_chain_word;
			sol_ddi.num_mbuf--;
		}
	} else {		/* clusters */
		if ((super_header = sol_ddi.hdrclist) == NULL) {
			super_header = sol_buildchain((short)TRUE);
			if (super_header == NULL) {
				SYS_SOL_TRACE(GET_HEADER_EXIT, NULL, 0, 0);
				return NULL;
			}
		} else {
			sol_ddi.hdrclist = (struct super_header *)
			    super_header->imcs_header.imcs_chain_word;
			sol_ddi.num_clus--;
		}
	}
		
	super_header->imcs_header.IMCS_SUBCHANNEL = subch;
	SYS_SOL_TRACE(GET_HEADER_EXIT, super_header, 0, 0);
	return (caddr_t)(&super_header->imcs_header);
}
